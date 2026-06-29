#ifndef APP_TOUCH_H
#define APP_TOUCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_TOUCH_DISPLAY_WIDTH   1024U
#define APP_TOUCH_DISPLAY_HEIGHT  600U

typedef enum
{
  APP_TOUCH_IC_NONE = 0,
  APP_TOUCH_IC_FT5X06 = 1,
  APP_TOUCH_IC_GT9XXX = 2
} AppTouchIc_t;

typedef struct
{
  uint8_t ready;
  uint8_t down;
  uint8_t touch_count;
  uint8_t int_pin;
  AppTouchIc_t ic;
  uint16_t raw_x;
  uint16_t raw_y;
  uint16_t x;
  uint16_t y;
  uint32_t init_attempts;
  uint32_t sample_count;
  uint32_t error_count;
  uint32_t last_error;
} AppTouchSnapshot_t;

uint8_t AppTouch_Init(void);
uint8_t AppTouch_Sample(uint16_t *x, uint16_t *y);
void AppTouch_GetSnapshot(AppTouchSnapshot_t *snapshot);
const char *AppTouch_IcName(AppTouchIc_t ic);

#ifdef __cplusplus
}
#endif

#endif /* APP_TOUCH_H */
