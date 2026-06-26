#include "app_bringup_thread.h"

#include "./LED/led.h"
#include "./RGBLCD/rgblcd.h"

#include <stdint.h>
#include <stdio.h>

#define APP_COLOR_STEP_TICKS  ((ULONG)TX_TIMER_TICKS_PER_SECOND)

typedef struct
{
  uint16_t color;
  const char *name;
} AppColorStep_t;

static void App_ShowBringUpPage(uint16_t color, const char *color_name, const char *status_line)
{
  const uint16_t text_color = ((color == WHITE) || (color == YELLOW) ||
                               (color == CYAN) || (color == LGRAY)) ? RED : WHITE;

  rgblcd_clear(color);
  rgblcd_show_string(10, 32, 300, 32, 32, "NECCS N647 APP", text_color);
  rgblcd_show_string(10, 76, 300, 24, 24, "ThreadX + RGBLCD", text_color);
  rgblcd_show_string(10, 110, 360, 16, 16, (char *)status_line, text_color);
  rgblcd_show_string(10, 132, 240, 16, 16, "Color:", text_color);
  rgblcd_show_string(80, 132, 240, 16, 16, (char *)color_name, text_color);
}

void App_BringUpThreadEntry(ULONG thread_input)
{
  uint32_t color_index = 0U;
  char status_line[48];
  static const AppColorStep_t color_steps[] =
  {
    { RED,   "RED"   },
    { GREEN, "GREEN" },
    { BLUE,  "BLUE"  },
    { WHITE, "WHITE" },
    { BLACK, "BLACK" },
  };

  (void)thread_input;
  snprintf(status_line, sizeof(status_line), "RAM OK, LCD ID: %04X", rgblcddev.id);

  while (1)
  {
    App_ShowBringUpPage(color_steps[color_index].color,
                        color_steps[color_index].name,
                        status_line);

    color_index++;
    if (color_index >= (sizeof(color_steps) / sizeof(color_steps[0])))
    {
      color_index = 0U;
    }

    LED0_TOGGLE();
    tx_thread_sleep(APP_COLOR_STEP_TICKS);
  }
}
