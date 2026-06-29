#include "TOUCH/app_touch.h"

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

#define APP_TOUCH_SCL_GPIO_PORT GPIOD
#define APP_TOUCH_SCL_GPIO_PIN  GPIO_PIN_14
#define APP_TOUCH_SDA_GPIO_PORT GPIOD
#define APP_TOUCH_SDA_GPIO_PIN  GPIO_PIN_4

#define APP_TOUCH_FT_CMD_WR             0x70U
#define APP_TOUCH_FT_CMD_RD             0x71U
#define APP_TOUCH_FT_REG_MODE           0x00U
#define APP_TOUCH_FT_REG_POINTS         0x02U
#define APP_TOUCH_FT_REG_TP1            0x03U
#define APP_TOUCH_FT_REG_THGROUP        0x80U
#define APP_TOUCH_FT_REG_PERIOD_ACTIVE  0x88U
#define APP_TOUCH_FT_REG_LIB_VERSION    0xA1U
#define APP_TOUCH_FT_REG_G_MODE         0xA4U

#define APP_TOUCH_GT_CMD_WR             0x28U
#define APP_TOUCH_GT_CMD_RD             0x29U
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
static uint32_t g_next_init_retry_ms;

static void touch_delay(void)
{
  for (volatile uint32_t i = 0; i < 80U; ++i)
  {
    __NOP();
  }
}

static void scl_write(uint8_t high)
{
  HAL_GPIO_WritePin(APP_TOUCH_SCL_GPIO_PORT,
                    APP_TOUCH_SCL_GPIO_PIN,
                    high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void sda_write(uint8_t high)
{
  HAL_GPIO_WritePin(APP_TOUCH_SDA_GPIO_PORT,
                    APP_TOUCH_SDA_GPIO_PIN,
                    high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t sda_read(void)
{
  return (HAL_GPIO_ReadPin(APP_TOUCH_SDA_GPIO_PORT, APP_TOUCH_SDA_GPIO_PIN) == GPIO_PIN_SET) ? 1U : 0U;
}

static void i2c_start(void)
{
  sda_write(1U);
  scl_write(1U);
  touch_delay();
  sda_write(0U);
  touch_delay();
  scl_write(0U);
  touch_delay();
}

static void i2c_stop(void)
{
  scl_write(0U);
  sda_write(0U);
  touch_delay();
  scl_write(1U);
  touch_delay();
  sda_write(1U);
  touch_delay();
}

static uint8_t i2c_wait_ack(void)
{
  uint32_t timeout = 0U;

  sda_write(1U);
  touch_delay();
  scl_write(1U);
  touch_delay();

  while (sda_read() != 0U)
  {
    ++timeout;
    if (timeout > 250U)
    {
      scl_write(0U);
      return 0U;
    }
  }

  scl_write(0U);
  touch_delay();
  return 1U;
}

static void i2c_ack(uint8_t ack)
{
  scl_write(0U);
  sda_write(ack ? 0U : 1U);
  touch_delay();
  scl_write(1U);
  touch_delay();
  scl_write(0U);
  sda_write(1U);
  touch_delay();
}

static uint8_t i2c_send_byte(uint8_t value)
{
  for (uint32_t bit = 0U; bit < 8U; ++bit)
  {
    scl_write(0U);
    sda_write((value & 0x80U) ? 1U : 0U);
    value <<= 1;
    touch_delay();
    scl_write(1U);
    touch_delay();
  }

  scl_write(0U);
  return i2c_wait_ack();
}

static uint8_t i2c_read_byte(uint8_t ack)
{
  uint8_t value = 0U;

  sda_write(1U);
  for (uint32_t bit = 0U; bit < 8U; ++bit)
  {
    value <<= 1;
    scl_write(0U);
    touch_delay();
    scl_write(1U);
    touch_delay();
    if (sda_read() != 0U)
    {
      value |= 1U;
    }
  }

  scl_write(0U);
  i2c_ack(ack);
  return value;
}

static void touch_pins_init(void)
{
  GPIO_InitTypeDef gpio = {0};

  if (g_pins_ready != 0U)
  {
    return;
  }

  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOQ_CLK_ENABLE();

  gpio.Pin = APP_TOUCH_SCL_GPIO_PIN | APP_TOUCH_SDA_GPIO_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_OD;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &gpio);

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

  sda_write(1U);
  scl_write(1U);
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_SET);
  g_pins_ready = 1U;
}

static void touch_reset(void)
{
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10U);
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(80U);
}

static uint8_t ft_write_reg(uint8_t reg, const uint8_t *data, uint32_t len)
{
  i2c_start();
  if ((i2c_send_byte(APP_TOUCH_FT_CMD_WR) == 0U) || (i2c_send_byte(reg) == 0U))
  {
    i2c_stop();
    return 0U;
  }

  for (uint32_t i = 0U; i < len; ++i)
  {
    if (i2c_send_byte(data[i]) == 0U)
    {
      i2c_stop();
      return 0U;
    }
  }

  i2c_stop();
  return 1U;
}

static uint8_t ft_read_reg(uint8_t reg, uint8_t *data, uint32_t len)
{
  i2c_start();
  if ((i2c_send_byte(APP_TOUCH_FT_CMD_WR) == 0U) || (i2c_send_byte(reg) == 0U))
  {
    i2c_stop();
    return 0U;
  }

  i2c_start();
  if (i2c_send_byte(APP_TOUCH_FT_CMD_RD) == 0U)
  {
    i2c_stop();
    return 0U;
  }

  for (uint32_t i = 0U; i < len; ++i)
  {
    data[i] = i2c_read_byte((i + 1U) < len);
  }

  i2c_stop();
  return 1U;
}

static uint8_t gt_write_reg(uint16_t reg, const uint8_t *data, uint32_t len)
{
  i2c_start();
  if ((i2c_send_byte(APP_TOUCH_GT_CMD_WR) == 0U) ||
      (i2c_send_byte((uint8_t)(reg >> 8)) == 0U) ||
      (i2c_send_byte((uint8_t)(reg & 0xFFU)) == 0U))
  {
    i2c_stop();
    return 0U;
  }

  for (uint32_t i = 0U; i < len; ++i)
  {
    if (i2c_send_byte(data[i]) == 0U)
    {
      i2c_stop();
      return 0U;
    }
  }

  i2c_stop();
  return 1U;
}

static uint8_t gt_read_reg(uint16_t reg, uint8_t *data, uint32_t len)
{
  i2c_start();
  if ((i2c_send_byte(APP_TOUCH_GT_CMD_WR) == 0U) ||
      (i2c_send_byte((uint8_t)(reg >> 8)) == 0U) ||
      (i2c_send_byte((uint8_t)(reg & 0xFFU)) == 0U))
  {
    i2c_stop();
    return 0U;
  }

  i2c_start();
  if (i2c_send_byte(APP_TOUCH_GT_CMD_RD) == 0U)
  {
    i2c_stop();
    return 0U;
  }

  for (uint32_t i = 0U; i < len; ++i)
  {
    data[i] = i2c_read_byte((i + 1U) < len);
  }

  i2c_stop();
  return 1U;
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

  touch_reset();

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

static uint8_t gt_try_init(void)
{
  uint8_t pid[4] = {0U, 0U, 0U, 0U};
  uint8_t value;

  touch_reset();

  if (gt_read_reg(APP_TOUCH_GT_REG_PID, pid, 4U) == 0U)
  {
    return 0U;
  }

  if (gt_pid_looks_valid(pid) == 0U)
  {
    return 0U;
  }

  value = 0x02U;
  (void)gt_write_reg(APP_TOUCH_GT_REG_CTRL, &value, 1U);
  HAL_Delay(10U);
  value = 0U;
  (void)gt_write_reg(APP_TOUCH_GT_REG_CTRL, &value, 1U);

  g_touch.ic = APP_TOUCH_IC_GT9XXX;
  g_touch.ready = 1U;
  g_touch.last_error = APP_TOUCH_ERR_NONE;
  return 1U;
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
  if ((count == 0U) || (count > 10U))
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
