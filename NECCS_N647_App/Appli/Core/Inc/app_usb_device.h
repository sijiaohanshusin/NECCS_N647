/*
 * USB device (USB1_OTG_HS + USBX) service: exposes the SD card as a USB
 * mass-storage device while app_media is in USB mode.
 *
 * Board facts this module bakes in (bottom board netlist 2026-07-20):
 * - OTG1 Type-C has NO VBUS/ID sensing (both NC) -> forced device mode,
 *   vbus_sensing disabled, PCD started as soon as the SD capacity is known.
 * - USB 5V feeds the board's VCC5 through a fuse; a serial relay in the
 *   cable normally keeps it CUT (tools/debug/n647.ps1 usb5v-*). Data works
 *   without VBUS, so "cable plugged, 5V cut" is the normal lab state.
 */

#ifndef APP_USB_DEVICE_H
#define APP_USB_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "tx_api.h"

/* Service state, published for UI/GDB. */
typedef struct
{
  uint8_t initialized;     /* USBX + PCD up, D+ pullup live            */
  uint8_t configured;      /* host enumerated + configured the device  */
  uint8_t media_exposed;   /* app_media in USB mode, LUN reports ready */
  uint32_t read_blocks;    /* MSC sector reads served                  */
  uint32_t write_blocks;   /* MSC sector writes served                 */
  uint32_t io_errors;
  int32_t last_status;     /* last init/step status (0 = OK)           */
  /* Throughput instrumentation (DWT us): total SD time vs read-callback
   * count isolates "SD is slow" from "host/USB pacing is slow". */
  uint32_t read_calls;     /* StorageRead invocations                  */
  uint32_t sd_read_us_total;
  uint32_t sd_read_us_max;
} AppUsbDeviceSnapshot_t;

/* Create the USB device thread (call from App_ThreadX_Init). The thread
 * waits for the SD capacity, then brings up USBX + the OTG PHY. */
UINT AppUsbDevice_Init(VOID);

void AppUsbDevice_GetSnapshot(AppUsbDeviceSnapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* APP_USB_DEVICE_H */
