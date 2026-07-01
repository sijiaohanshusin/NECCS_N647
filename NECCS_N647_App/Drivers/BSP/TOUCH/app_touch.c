#include "TOUCH/app_touch.h"

#include "app_i2c2_bus.h"
#include "main.h"
#include "stm32n6xx_hal.h"

#include <stddef.h>

#ifndef APP_TOUCH_FT_SWAP_XY
#define APP_TOUCH_FT_SWAP_XY 1U
#endif

#ifndef APP_TOUCH_GT_SWAP_XY
#define APP_TOUCH_GT_SWAP_XY 0U
#endif

#ifndef APP_TOUCH_FT_INVERT_X
#define APP_TOUCH_FT_INVERT_X 0U
#endif

#ifndef APP_TOUCH_FT_INVERT_Y
#define APP_TOUCH_FT_INVERT_Y 0U
#endif

#ifndef APP_TOUCH_GT_INVERT_X
#define APP_TOUCH_GT_INVERT_X 0U
#endif

#ifndef APP_TOUCH_GT_INVERT_Y
#define APP_TOUCH_GT_INVERT_Y 0U
#endif

#define APP_TOUCH_I2C_TIMEOUT_MS        50U

#define APP_TOUCH_FT_ADDR7              0x38U
#define APP_TOUCH_FT_REG_MODE           0x00U
#define APP_TOUCH_FT_REG_POINTS         0x02U
#define APP_TOUCH_FT_REG_TP1            0x03U
#define APP_TOUCH_FT_REG_THGROUP        0x80U
#define APP_TOUCH_FT_REG_PERIOD_ACTIVE  0x88U
#define APP_TOUCH_FT_REG_LIB_VERSION    0xA1U
#define APP_TOUCH_FT_REG_G_MODE         0xA4U

#define APP_TOUCH_GT_ADDR7_LOW          0x14U
#define APP_TOUCH_GT_ADDR7_HIGH         0x5DU
#define APP_TOUCH_GT_REG_CTRL           0x8040U
#define APP_TOUCH_GT_REG_PID            0x8140U
#define APP_TOUCH_GT_REG_STATUS         0x814EU
#define APP_TOUCH_GT_REG_TP1            0x8150U

#define APP_TOUCH_ERR_NONE              0U
#define APP_TOUCH_ERR_ACK               1U
#define APP_TOUCH_ERR_NO_DEVICE         2U
#define APP_TOUCH_ERR_BAD_STATUS        3U
#define APP_TOUCH_ERR_BAD_ARGUMENT      4U

static AppTouchSnapshot_t g_touch;
static uint8_t g_pins_ready;
static uint8_t g_gt_address7;
static uint32_t g_next_init_retry_ms;

static void touch_pins_init(void)
{
  GPIO_InitTypeDef gpio = {0};

  if (g_pins_ready != 0U)
  {
    return;
  }

  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOQ_CLK_ENABLE();

  gpio.Pin = CTP_RST_Pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CTP_RST_GPIO_Port, &gpio);

  gpio.Pin = CTP_INT_Pin;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CTP_INT_GPIO_Port, &gpio);

  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_SET);
  g_pins_ready = 1U;
}

static void touch_int_input(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin = CTP_INT_Pin;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CTP_INT_GPIO_Port, &gpio);
}

static void touch_int_output(GPIO_PinState state)
{
  GPIO_InitTypeDef gpio = {0};

  HAL_GPIO_WritePin(CTP_INT_GPIO_Port, CTP_INT_Pin, state);

  gpio.Pin = CTP_INT_Pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CTP_INT_GPIO_Port, &gpio);

  HAL_GPIO_WritePin(CTP_INT_GPIO_Port, CTP_INT_Pin, state);
}

static void touch_reset_plain(void)
{
  touch_int_input();
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10U);
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(80U);
}

static void touch_reset_with_int(GPIO_PinState int_state)
{
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_RESET);
  touch_int_output(int_state);
  HAL_Delay(10U);
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(10U);
  touch_int_input();
  HAL_Delay(80U);
}

static uint8_t i2c_status_ok(HAL_StatusTypeDef status)
{
  g_touch.last_hal_status = (uint32_t)status;
  return (status == HAL_OK) ? 1U : 0U;
}

static uint8_t touch_i2c_mem_write8(uint8_t address7,
                                    uint8_t reg,
                                    const uint8_t *data,
                                    uint32_t len)
{
  HAL_StatusTypeDef status;

  if ((data == NULL) || (len == 0U) || (len > 0xFFFFU))
  {
    g_touch.last_error = APP_TOUCH_ERR_BAD_ARGUMENT;
    return 0U;
  }

  if (AppI2C2_Lock(APP_TOUCH_I2C_TIMEOUT_MS) == 0U)
  {
    g_touch.last_hal_status = (uint32_t)HAL_BUSY;
    return 0U;
  }

  status = HAL_I2C_Mem_Write(&hi2c2,
                             (uint16_t)(address7 << 1),
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             (uint8_t *)data,
                             (uint16_t)len,
                             APP_TOUCH_I2C_TIMEOUT_MS);
  AppI2C2_Unlock();

  return i2c_status_ok(status);
}

static uint8_t touch_i2c_mem_read8(uint8_t address7,
                                   uint8_t reg,
                                   uint8_t *data,
                                   uint32_t len)
{
  HAL_StatusTypeDef status;

  if ((data == NULL) || (len == 0U) || (len > 0xFFFFU))
  {
    g_touch.last_error = APP_TOUCH_ERR_BAD_ARGUMENT;
    return 0U;
  }

  if (AppI2C2_Lock(APP_TOUCH_I2C_TIMEOUT_MS) == 0U)
  {
    g_touch.last_hal_status = (uint32_t)HAL_BUSY;
    return 0U;
  }

  status = HAL_I2C_Mem_Read(&hi2c2,
                            (uint16_t)(address7 << 1),
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            data,
                            (uint16_t)len,
                            APP_TOUCH_I2C_TIMEOUT_MS);
  AppI2C2_Unlock();

  return i2c_status_ok(status);
}

static uint8_t touch_i2c_mem_write16(uint8_t address7,
                                     uint16_t reg,
                                     const uint8_t *data,
                                     uint32_t len)
{
  HAL_StatusTypeDef status;

  if ((data == NULL) || (len == 0U) || (len > 0xFFFFU))
  {
    g_touch.last_error = APP_TOUCH_ERR_BAD_ARGUMENT;
    return 0U;
  }

  if (AppI2C2_Lock(APP_TOUCH_I2C_TIMEOUT_MS) == 0U)
  {
    g_touch.last_hal_status = (uint32_t)HAL_BUSY;
    return 0U;
  }

  status = HAL_I2C_Mem_Write(&hi2c2,
                             (uint16_t)(address7 << 1),
                             reg,
                             I2C_MEMADD_SIZE_16BIT,
                             (uint8_t *)data,
                             (uint16_t)len,
                             APP_TOUCH_I2C_TIMEOUT_MS);
  AppI2C2_Unlock();

  return i2c_status_ok(status);
}

static uint8_t touch_i2c_mem_read16(uint8_t address7,
                                    uint16_t reg,
                                    uint8_t *data,
                                    uint32_t len)
{
  HAL_StatusTypeDef status;

  if ((data == NULL) || (len == 0U) || (len > 0xFFFFU))
  {
    g_touch.last_error = APP_TOUCH_ERR_BAD_ARGUMENT;
    return 0U;
  }

  if (AppI2C2_Lock(APP_TOUCH_I2C_TIMEOUT_MS) == 0U)
  {
    g_touch.last_hal_status = (uint32_t)HAL_BUSY;
    return 0U;
  }

  status = HAL_I2C_Mem_Read(&hi2c2,
                            (uint16_t)(address7 << 1),
                            reg,
                            I2C_MEMADD_SIZE_16BIT,
                            data,
                            (uint16_t)len,
                            APP_TOUCH_I2C_TIMEOUT_MS);
  AppI2C2_Unlock();

  return i2c_status_ok(status);
}

static uint8_t ft_write_reg(uint8_t reg, const uint8_t *data, uint32_t len)
{
  return touch_i2c_mem_write8(APP_TOUCH_FT_ADDR7, reg, data, len);
}

static uint8_t ft_read_reg(uint8_t reg, uint8_t *data, uint32_t len)
{
  return touch_i2c_mem_read8(APP_TOUCH_FT_ADDR7, reg, data, len);
}

static uint8_t gt_write_reg_at(uint8_t address7, uint16_t reg, const uint8_t *data, uint32_t len)
{
  return touch_i2c_mem_write16(address7, reg, data, len);
}

static uint8_t gt_read_reg_at(uint8_t address7, uint16_t reg, uint8_t *data, uint32_t len)
{
  return touch_i2c_mem_read16(address7, reg, data, len);
}

static uint8_t gt_write_reg(uint16_t reg, const uint8_t *data, uint32_t len)
{
  return gt_write_reg_at(g_gt_address7, reg, data, len);
}

static uint8_t gt_read_reg(uint16_t reg, uint8_t *data, uint32_t len)
{
  return gt_read_reg_at(g_gt_address7, reg, data, len);
}

static uint16_t clamp_coord(uint32_t value, uint16_t limit)
{
  if (value >= limit)
  {
    return (uint16_t)(limit - 1U);
  }
  return (uint16_t)value;
}

static uint16_t maybe_invert(uint16_t value, uint16_t limit, uint8_t invert)
{
  if (invert == 0U)
  {
    return value;
  }
  return (uint16_t)((limit - 1U) - clamp_coord(value, limit));
}

static void store_touch(uint16_t raw_x,
                        uint16_t raw_y,
                        uint8_t swap_xy,
                        uint8_t invert_x,
                        uint8_t invert_y,
                        uint8_t count)
{
  uint16_t mapped_x = swap_xy ? raw_y : raw_x;
  uint16_t mapped_y = swap_xy ? raw_x : raw_y;

  mapped_x = maybe_invert(mapped_x, (uint16_t)APP_TOUCH_DISPLAY_WIDTH, invert_x);
  mapped_y = maybe_invert(mapped_y, (uint16_t)APP_TOUCH_DISPLAY_HEIGHT, invert_y);

  g_touch.raw_x = raw_x;
  g_touch.raw_y = raw_y;
  g_touch.x = clamp_coord(mapped_x, (uint16_t)APP_TOUCH_DISPLAY_WIDTH);
  g_touch.y = clamp_coord(mapped_y, (uint16_t)APP_TOUCH_DISPLAY_HEIGHT);
  g_touch.touch_count = count;
  g_touch.down = 1U;
  g_touch.last_error = APP_TOUCH_ERR_NONE;
}

static uint8_t ft_try_init(void)
{
  uint8_t value;
  uint8_t version[2] = {0U, 0U};

  touch_reset_plain();
  g_touch.address7 = APP_TOUCH_FT_ADDR7;
  g_gt_address7 = 0U;

  value = 0U;
  if (ft_write_reg(APP_TOUCH_FT_REG_MODE, &value, 1U) == 0U)
  {
    return 0U;
  }

  value = 0U;
  (void)ft_write_reg(APP_TOUCH_FT_REG_G_MODE, &value, 1U);
  value = 22U;
  (void)ft_write_reg(APP_TOUCH_FT_REG_THGROUP, &value, 1U);
  value = 12U;
  (void)ft_write_reg(APP_TOUCH_FT_REG_PERIOD_ACTIVE, &value, 1U);

  if (ft_read_reg(APP_TOUCH_FT_REG_LIB_VERSION, version, 2U) == 0U)
  {
    return 0U;
  }

  g_touch.ic = APP_TOUCH_IC_FT5X06;
  g_touch.ready = 1U;
  g_touch.last_error = APP_TOUCH_ERR_NONE;
  (void)version;
  return 1U;
}

static uint8_t gt_pid_looks_valid(const uint8_t *pid)
{
  uint8_t valid = 0U;

  for (uint32_t i = 0U; i < 4U; ++i)
  {
    if ((pid[i] >= (uint8_t)'0') && (pid[i] <= (uint8_t)'9'))
    {
      valid = 1U;
    }
    else if ((pid[i] != 0U) && (pid[i] != 0xFFU))
    {
      valid = 1U;
    }
  }

  return valid;
}

static uint8_t gt_try_init_at(uint8_t address7, GPIO_PinState int_state)
{
  uint8_t pid[4] = {0U, 0U, 0U, 0U};
  uint8_t value;

  touch_reset_with_int(int_state);
  g_touch.address7 = address7;

  if (gt_read_reg_at(address7, APP_TOUCH_GT_REG_PID, pid, 4U) == 0U)
  {
    return 0U;
  }

  if (gt_pid_looks_valid(pid) == 0U)
  {
    return 0U;
  }

  value = 0x02U;
  (void)gt_write_reg_at(address7, APP_TOUCH_GT_REG_CTRL, &value, 1U);
  HAL_Delay(10U);
  value = 0U;
  (void)gt_write_reg_at(address7, APP_TOUCH_GT_REG_CTRL, &value, 1U);

  g_gt_address7 = address7;
  g_touch.address7 = address7;
  g_touch.ic = APP_TOUCH_IC_GT9XXX;
  g_touch.ready = 1U;
  g_touch.last_error = APP_TOUCH_ERR_NONE;
  return 1U;
}

static uint8_t gt_try_init(void)
{
  if (gt_try_init_at(APP_TOUCH_GT_ADDR7_LOW, GPIO_PIN_RESET) != 0U)
  {
    return 1U;
  }

  if (gt_try_init_at(APP_TOUCH_GT_ADDR7_HIGH, GPIO_PIN_SET) != 0U)
  {
    return 1U;
  }

  g_gt_address7 = 0U;
  return 0U;
}

static uint8_t ft_sample(uint16_t *x, uint16_t *y)
{
  uint8_t count = 0U;
  uint8_t point[4] = {0U, 0U, 0U, 0U};
  uint16_t raw_x;
  uint16_t raw_y;

  if (ft_read_reg(APP_TOUCH_FT_REG_POINTS, &count, 1U) == 0U)
  {
    g_touch.last_error = APP_TOUCH_ERR_ACK;
    ++g_touch.error_count;
    return 0U;
  }

  count &= 0x0FU;
  if ((count == 0U) || (count > 5U))
  {
    g_touch.down = 0U;
    g_touch.touch_count = 0U;
    return 0U;
  }

  if (ft_read_reg(APP_TOUCH_FT_REG_TP1, point, 4U) == 0U)
  {
    g_touch.last_error = APP_TOUCH_ERR_ACK;
    ++g_touch.error_count;
    return 0U;
  }

  raw_x = (uint16_t)((((uint16_t)point[0] & 0x0FU) << 8) | point[1]);
  raw_y = (uint16_t)((((uint16_t)point[2] & 0x0FU) << 8) | point[3]);

  store_touch(raw_x,
              raw_y,
              APP_TOUCH_FT_SWAP_XY,
              APP_TOUCH_FT_INVERT_X,
              APP_TOUCH_FT_INVERT_Y,
              count);
  *x = g_touch.x;
  *y = g_touch.y;
  return 1U;
}

static uint8_t gt_sample(uint16_t *x, uint16_t *y)
{
  uint8_t status = 0U;
  uint8_t point[4] = {0U, 0U, 0U, 0U};
  uint8_t count;
  uint8_t clear = 0U;
  uint16_t raw_x;
  uint16_t raw_y;

  if (gt_read_reg(APP_TOUCH_GT_REG_STATUS, &status, 1U) == 0U)
  {
    g_touch.last_error = APP_TOUCH_ERR_ACK;
    ++g_touch.error_count;
    return 0U;
  }

  if ((status & 0x80U) == 0U)
  {
    return 0U;
  }

  count = status & 0x0FU;
  if (count == 0U)
  {
    (void)gt_write_reg(APP_TOUCH_GT_REG_STATUS, &clear, 1U);
    g_touch.down = 0U;
    g_touch.touch_count = 0U;
    g_touch.last_error = APP_TOUCH_ERR_NONE;
    return 0U;
  }

  if (count > 10U)
  {
    (void)gt_write_reg(APP_TOUCH_GT_REG_STATUS, &clear, 1U);
    g_touch.down = 0U;
    g_touch.touch_count = 0U;
    g_touch.last_error = APP_TOUCH_ERR_BAD_STATUS;
    return 0U;
  }

  if (gt_read_reg(APP_TOUCH_GT_REG_TP1, point, 4U) == 0U)
  {
    (void)gt_write_reg(APP_TOUCH_GT_REG_STATUS, &clear, 1U);
    g_touch.last_error = APP_TOUCH_ERR_ACK;
    ++g_touch.error_count;
    return 0U;
  }

  (void)gt_write_reg(APP_TOUCH_GT_REG_STATUS, &clear, 1U);

  raw_x = (uint16_t)(((uint16_t)point[1] << 8) | point[0]);
  raw_y = (uint16_t)(((uint16_t)point[3] << 8) | point[2]);

  store_touch(raw_x,
              raw_y,
              APP_TOUCH_GT_SWAP_XY,
              APP_TOUCH_GT_INVERT_X,
              APP_TOUCH_GT_INVERT_Y,
              count);
  *x = g_touch.x;
  *y = g_touch.y;
  return 1U;
}

uint8_t AppTouch_Init(void)
{
  touch_pins_init();

  ++g_touch.init_attempts;
  g_touch.ready = 0U;
  g_touch.ic = APP_TOUCH_IC_NONE;
  g_touch.address7 = 0U;
  g_touch.last_error = APP_TOUCH_ERR_NO_DEVICE;

  if (ft_try_init() != 0U)
  {
    g_next_init_retry_ms = 0U;
    return 1U;
  }

  if (gt_try_init() != 0U)
  {
    g_next_init_retry_ms = 0U;
    return 1U;
  }

  g_touch.ready = 0U;
  g_touch.ic = APP_TOUCH_IC_NONE;
  g_next_init_retry_ms = HAL_GetTick() + 1000U;
  ++g_touch.error_count;
  return 0U;
}

uint8_t AppTouch_Sample(uint16_t *x, uint16_t *y)
{
  if ((x == NULL) || (y == NULL))
  {
    g_touch.last_error = APP_TOUCH_ERR_BAD_ARGUMENT;
    ++g_touch.error_count;
    return 0U;
  }

  if (g_touch.ready == 0U)
  {
    if ((g_touch.init_attempts != 0U) && ((int32_t)(HAL_GetTick() - g_next_init_retry_ms) < 0))
    {
      return 0U;
    }

    if (AppTouch_Init() == 0U)
    {
      return 0U;
    }
  }

  ++g_touch.sample_count;
  g_touch.int_pin = (HAL_GPIO_ReadPin(CTP_INT_GPIO_Port, CTP_INT_Pin) == GPIO_PIN_SET) ? 1U : 0U;

  if (g_touch.ic == APP_TOUCH_IC_FT5X06)
  {
    return ft_sample(x, y);
  }

  if (g_touch.ic == APP_TOUCH_IC_GT9XXX)
  {
    return gt_sample(x, y);
  }

  g_touch.down = 0U;
  g_touch.touch_count = 0U;
  g_touch.last_error = APP_TOUCH_ERR_NO_DEVICE;
  return 0U;
}

void AppTouch_GetSnapshot(AppTouchSnapshot_t *snapshot)
{
  if (snapshot == NULL)
  {
    return;
  }

  *snapshot = g_touch;
}

const char *AppTouch_IcName(AppTouchIc_t ic)
{
  switch (ic)
  {
    case APP_TOUCH_IC_FT5X06:
      return "FT5X06";
    case APP_TOUCH_IC_GT9XXX:
      return "GT9XXX";
    default:
      return "NO IC";
  }
}
