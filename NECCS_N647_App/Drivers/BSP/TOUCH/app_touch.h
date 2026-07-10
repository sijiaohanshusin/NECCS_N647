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
  uint8_t address7;
  uint8_t reserved[3];
  AppTouchIc_t ic;
  uint16_t raw_x;
  uint16_t raw_y;
  uint16_t x;
  uint16_t y;
  uint32_t init_attempts;
  uint32_t sample_count;
  uint32_t error_count;
  uint32_t last_error;
  uint32_t last_hal_status;
  uint32_t consecutive_error_count;
  uint32_t cooldown_count;
  uint32_t reinit_count;
  /* Diagnostics split: bus-mutex wait timeouts (benign contention with the
   * camera) vs real wire-level failures (NACK/arbitration = flaky panel
   * link). Only wire errors should trigger reinit. */
  uint32_t lock_timeout_count;
  uint32_t wire_error_count;
  /* GT911 config-table head (regs 0x8047..0x804E) read back at init:
   * [0]=cfg version, [1..2]=X resolution LE, [3..4]=Y resolution LE,
   * [5]=touch number, [6]=module switch 1, [7]=module switch 2.
   * cfg_read_ok=1 when the readback itself succeeded. An all-zero head on
   * a chip that ACKs = the controller's NVM config failed to load and it
   * will never report touches (root cause of the dead 7" panel). */
  uint8_t cfg_head[8];
  uint8_t cfg_read_ok;
} AppTouchSnapshot_t;

uint8_t AppTouch_Init(void);
uint8_t AppTouch_Sample(uint16_t *x, uint16_t *y);
void AppTouch_GetSnapshot(AppTouchSnapshot_t *snapshot);
const char *AppTouch_IcName(AppTouchIc_t ic);

#ifdef __cplusplus
}
#endif

#endif /* APP_TOUCH_H */
