#ifndef APP_CAMERA_IMX219_H
#define APP_CAMERA_IMX219_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_CAMERA_IMX219_OK                 0
#define APP_CAMERA_IMX219_ERROR_INVALID_ARG -1
#define APP_CAMERA_IMX219_ERROR_I2C_LOCK    -2
#define APP_CAMERA_IMX219_ERROR_I2C         -3
#define APP_CAMERA_IMX219_ERROR_CHIP_ID     -4

extern volatile uint32_t g_app_camera_imx219_last_reg;
extern volatile uint32_t g_app_camera_imx219_last_error;
extern volatile uint32_t g_app_camera_imx219_last_hal_status;
extern volatile uint32_t g_app_camera_imx219_transfer_count;
extern volatile uint32_t g_app_camera_imx219_readback_status;
extern volatile uint32_t g_app_camera_imx219_readback_lane_mode;
extern volatile uint32_t g_app_camera_imx219_readback_data_format0;
extern volatile uint32_t g_app_camera_imx219_readback_data_format1;
extern volatile uint32_t g_app_camera_imx219_readback_test_pattern;
extern volatile uint32_t g_app_camera_imx219_readback_stream;

typedef struct
{
  uint16_t width;
  uint16_t height;
  uint16_t fps;
  uint32_t input_clock_hz;
} AppCameraIMX219Config_t;

int32_t AppCameraIMX219_Init(const AppCameraIMX219Config_t *config, uint16_t *chip_id);
int32_t AppCameraIMX219_SetStream(uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif /* APP_CAMERA_IMX219_H */
