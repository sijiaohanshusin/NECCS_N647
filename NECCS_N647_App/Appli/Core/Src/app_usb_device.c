/*
 * USB device service: USB1_OTG_HS (embedded HS PHY) + USBX device stack +
 * MSC (mass storage) class bridged to the SD NAND block driver.
 *
 * Clock/PHY bring-up follows the ST reference (STM32N6570-DK Ux_Device_MSC,
 * FW N6 V1.3.0): HSE 48 MHz direct to OTG + PHY, PHY FSEL = 24 MHz via the
 * HSE/2 path. This board has no VBUS sensing (OTG1_VBUS/ID are NC on the
 * bottom board), so the device is forced on and simply waits for a host.
 *
 * Media arbitration: the MSC LUN reports "medium not present" until
 * app_media enters USB mode (fx_media_close'd, SD exclusively ours). The
 * PC side then sees the FAT volume exactly as FileX formatted it: the
 * FileX driver maps logical sector n -> physical LBA n (superfloppy, boot
 * sector at LBA 0), so raw block passthrough IS the volume.
 */

#include "app_usb_device.h"

#include "app_media.h"
#include "main.h"
#include "./SD_NAND/sd_nand.h"

#include "ux_api.h"
#include "ux_dcd_stm32.h"
#include "ux_device_class_storage.h"
#include "ux_device_descriptors.h"
#include "ux_device_stack.h"

#include "stm32n6xx_ll_bus.h"
#include "stm32n6xx_ll_rcc.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* Sizing                                                              */
/* ------------------------------------------------------------------ */

/* ONE UX pool, INTERNAL RAM only. HyperRAM-backed USBX memory failed two
 * different ways on this board (2026-07-20):
 * - pool with class thread stacks in .EXTRAM -> INVSTATE HardFault (PSP
 *   pointed into the pool, registers full of the 0xEF fill pattern);
 * - transfer buffers in a .EXTRAM cache pool -> host received garbage
 *   sectors (8-byte repeats with raw pool ADDRESSES embedded - PC-side
 *   FAT dump contained 0x909A1D90/0x341768A4).
 * Same failure family as the FileX-cache-in-EXTRAM note in app_media.c.
 * 12 KB fits the device stack + MSC instance + 2 KB class thread stack +
 * 1 KB bulk buffers (DK reference ran the same class set in 5 KB; if a
 * future class addition starves it, AppUsbDevice_StackInit fails loudly
 * with last_status=-3/-4/-5 in the snapshot). Internal RAM is fully
 * booked - every KB here trades against UI/DSP statics. */
#define APP_USB_UX_POOL_BYTES        (12U * 1024U)
/* 1.75 KB: deepest path is HAL_PCD_Init/RCC config at startup, then the
 * thread only sleeps and copies the snapshot (internal RAM is 160 B from
 * full - every straw counts). */
#define APP_USB_APP_THREAD_STACK     1792U
/* 12 (media level), NOT below 13: the acoustic thread is a by-design
 * priority-13 CPU hog (SRP grinds in every spare cycle), so anything
 * >13 literally never runs (board 2026-07-20: run_count stayed 0).
 * Same reason UX_THREAD_PRIORITY_CLASS is pinned to 12 in ux_user.h. */
#define APP_USB_APP_THREAD_PRIORITY  12U
/* IRQ urgency below the SAI/GPDMA audio chain; USB tolerates latency. */
#define APP_USB_IRQ_PRIORITY         8U

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */

PCD_HandleTypeDef g_hpcd_usb1_otg_hs;

static UCHAR s_ux_pool[APP_USB_UX_POOL_BYTES] __attribute__((aligned(32)));
static TX_THREAD s_usb_thread;
static ULONG s_usb_thread_stack[APP_USB_APP_THREAD_STACK / sizeof(ULONG)];
static UX_SLAVE_CLASS_STORAGE_PARAMETER s_storage_parameter;
static volatile AppUsbDeviceSnapshot_t s_snapshot;

/* ------------------------------------------------------------------ */
/* PHY / PCD bring-up (ST reference sequence)                          */
/* ------------------------------------------------------------------ */

void HAL_PCD_MspInit(PCD_HandleTypeDef *pcd_handle)
{
  if (pcd_handle->Instance == USB1_OTG_HS)
  {
    RCC_PeriphCLKInitTypeDef periph_clk = {0};

    /* VDDUSB power domain. */
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_EnableVddUSBVMEN();
    while (__HAL_PWR_GET_FLAG(PWR_FLAG_USB33RDY))
    {
    }
    HAL_PWREx_EnableVddUSB();

    /* This application's clock tree never starts HSE (everything runs off
     * HSI/MSI + PLLs), but the USB HS PHY reference REQUIRES the 48 MHz
     * HSE (board 2026-07-20: CSRST never cleared, RCC_CR HSEON=0). Start
     * it here; crystal mode first, bypass as fallback in case the core
     * board fits an active oscillator. Nothing else sources from HSE, so
     * this does not disturb the running system clocks. */
    {
      RCC_OscInitTypeDef osc = {0};

      osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
      osc.HSEState = RCC_HSE_ON;
      if (HAL_RCC_OscConfig(&osc) != HAL_OK)
      {
        osc.HSEState = RCC_HSE_BYPASS;
        if (HAL_RCC_OscConfig(&osc) != HAL_OK)
        {
          s_snapshot.last_status = -12;
          return;
        }
      }
    }

    /* OTG + PHY kernel clocks: HSE (48 MHz) direct. */
    periph_clk.PeriphClockSelection = RCC_PERIPHCLK_USBOTGHS1;
    periph_clk.UsbOtgHs1ClockSelection = RCC_USBOTGHS1CLKSOURCE_HSE_DIRECT;
    if (HAL_RCCEx_PeriphCLKConfig(&periph_clk) != HAL_OK)
    {
      s_snapshot.last_status = -10;
      return;
    }
    periph_clk.PeriphClockSelection = RCC_PERIPHCLK_USBPHY1;
    periph_clk.UsbPhy1ClockSelection = RCC_USBPHY1CLKSOURCE_HSE_DIRECT;
    if (HAL_RCCEx_PeriphCLKConfig(&periph_clk) != HAL_OK)
    {
      s_snapshot.last_status = -11;
      return;
    }

    /* Reset OTG + PHY, select HSE/2 (24 MHz) as the PHY reference. */
    LL_AHB5_GRP1_ForceReset(0x00800000U);
    __HAL_RCC_USB1_OTG_HS_FORCE_RESET();
    __HAL_RCC_USB1_OTG_HS_PHY_FORCE_RESET();
    LL_RCC_HSE_SelectHSEDiv2AsDiv2Clock();
    LL_AHB5_GRP1_ReleaseReset(0x00800000U);

    __HAL_RCC_USB1_OTG_HS_CLK_ENABLE();
    HAL_Delay(1); /* PHY register access needs a few clock cycles */

    /* PHY control: FSEL = 24 MHz (0x2), OTGDISABLE0, CMN, RETENABLEN1
     * (bit layout per the ST reference example). */
    USB1_HS_PHYC->USBPHYC_CR &= ~(0x7U << 4);
    USB1_HS_PHYC->USBPHYC_CR |= (0x1U << 16) |
                                (0x2U << 4) |
                                (0x1U << 2) |
                                 0x1U;

    __HAL_RCC_USB1_OTG_HS_PHY_RELEASE_RESET();
    HAL_Delay(1);
    __HAL_RCC_USB1_OTG_HS_RELEASE_RESET();
    __HAL_RCC_USB1_OTG_HS_PHY_CLK_ENABLE();

    HAL_NVIC_SetPriority(USB1_OTG_HS_IRQn, APP_USB_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(USB1_OTG_HS_IRQn);
  }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef *pcd_handle)
{
  if (pcd_handle->Instance == USB1_OTG_HS)
  {
    HAL_NVIC_DisableIRQ(USB1_OTG_HS_IRQn);
    __HAL_RCC_USB1_OTG_HS_CLK_DISABLE();
    __HAL_RCC_USB1_OTG_HS_PHY_CLK_DISABLE();
  }
}

static int32_t AppUsbDevice_PcdInit(void)
{
  memset(&g_hpcd_usb1_otg_hs, 0, sizeof(g_hpcd_usb1_otg_hs));
  g_hpcd_usb1_otg_hs.Instance = USB1_OTG_HS;
  g_hpcd_usb1_otg_hs.Init.dev_endpoints = 9;
  /* FULL speed on the HS PHY for now: the OTG1 data path runs through
   * the USB relay module + jumper wires, and the 480 MHz chirp did not
   * survive it (board 2026-07-20: host saw the attach but "port reset
   * failed", device never received USBRST). FS (12 Mbps) is wiring-
   * tolerant; switch back to PCD_SPEED_HIGH once OTG1 has a clean path. */
  g_hpcd_usb1_otg_hs.Init.speed = PCD_SPEED_HIGH_IN_FULL;
  g_hpcd_usb1_otg_hs.Init.dma_enable = DISABLE;
  g_hpcd_usb1_otg_hs.Init.phy_itface = USB_OTG_HS_EMBEDDED_PHY;
  g_hpcd_usb1_otg_hs.Init.Sof_enable = DISABLE;
  g_hpcd_usb1_otg_hs.Init.low_power_enable = DISABLE;
  g_hpcd_usb1_otg_hs.Init.lpm_enable = DISABLE;
  /* OTG1_VBUS is NC on this board: never sense, never gate on it. */
  g_hpcd_usb1_otg_hs.Init.vbus_sensing_enable = DISABLE;
  g_hpcd_usb1_otg_hs.Init.use_dedicated_ep1 = DISABLE;
  g_hpcd_usb1_otg_hs.Init.use_external_vbus = DISABLE;

  if (HAL_PCD_Init(&g_hpcd_usb1_otg_hs) != HAL_OK)
  {
    return -2;
  }
  if (s_snapshot.last_status != 0)
  {
    return s_snapshot.last_status; /* MspInit clock failure */
  }

  /* FIFO split per the ST reference: 512-word RX, EP0 64, bulk-IN 256. */
  HAL_PCDEx_SetRxFiFo(&g_hpcd_usb1_otg_hs, 0x200);
  HAL_PCDEx_SetTxFiFo(&g_hpcd_usb1_otg_hs, 0, 0x40);
  HAL_PCDEx_SetTxFiFo(&g_hpcd_usb1_otg_hs, 1, 0x100);
  return 0;
}

/* ------------------------------------------------------------------ */
/* MSC storage callbacks: raw block passthrough, gated on USB mode     */
/* ------------------------------------------------------------------ */

#define APP_USB_SENSE_NOT_READY \
  UX_DEVICE_CLASS_STORAGE_SENSE_STATUS(UX_SLAVE_CLASS_STORAGE_SENSE_KEY_NOT_READY, 0x3AU, 0x00U)

static VOID AppUsbDevice_StorageActivate(VOID *storage_instance)
{
  UX_PARAMETER_NOT_USED(storage_instance);
  s_snapshot.configured = 1U;
}

static VOID AppUsbDevice_StorageDeactivate(VOID *storage_instance)
{
  UX_PARAMETER_NOT_USED(storage_instance);
  s_snapshot.configured = 0U;
}

static UINT AppUsbDevice_StorageStatus(VOID *storage_instance, ULONG lun,
                                       ULONG media_id, ULONG *media_status)
{
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_id);

  if (AppMedia_UsbModeActive() != 0U)
  {
    *media_status = 0U;
    s_snapshot.media_exposed = 1U;
    return UX_SUCCESS;
  }

  /* Card-reader-without-card semantics while the firmware owns the SD. */
  *media_status = APP_USB_SENSE_NOT_READY;
  s_snapshot.media_exposed = 0U;
  return UX_ERROR;
}

static UINT AppUsbDevice_StorageRead(VOID *storage_instance, ULONG lun,
                                     UCHAR *data_pointer, ULONG number_blocks,
                                     ULONG lba, ULONG *media_status)
{
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);

  if (AppMedia_UsbModeActive() == 0U)
  {
    *media_status = APP_USB_SENSE_NOT_READY;
    return UX_ERROR;
  }
  if (sd_nand_read_disk(data_pointer, (uint32_t)lba, (uint32_t)number_blocks) != SD_NAND_OK)
  {
    s_snapshot.io_errors++;
    *media_status = UX_DEVICE_CLASS_STORAGE_SENSE_STATUS(0x03U, 0x11U, 0x00U);
    return UX_ERROR;
  }
  s_snapshot.read_blocks += (uint32_t)number_blocks;
  return UX_SUCCESS;
}

static UINT AppUsbDevice_StorageWrite(VOID *storage_instance, ULONG lun,
                                      UCHAR *data_pointer, ULONG number_blocks,
                                      ULONG lba, ULONG *media_status)
{
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);

  if (AppMedia_UsbModeActive() == 0U)
  {
    *media_status = APP_USB_SENSE_NOT_READY;
    return UX_ERROR;
  }
  if (sd_nand_write_disk(data_pointer, (uint32_t)lba, (uint32_t)number_blocks) != SD_NAND_OK)
  {
    s_snapshot.io_errors++;
    *media_status = UX_DEVICE_CLASS_STORAGE_SENSE_STATUS(0x03U, 0x0CU, 0x00U);
    return UX_ERROR;
  }
  s_snapshot.write_blocks += (uint32_t)number_blocks;
  return UX_SUCCESS;
}

static UINT AppUsbDevice_StorageFlush(VOID *storage_instance, ULONG lun,
                                      ULONG number_blocks, ULONG lba,
                                      ULONG *media_status)
{
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(number_blocks);
  UX_PARAMETER_NOT_USED(lba);
  UX_PARAMETER_NOT_USED(media_status);
  /* sd_nand_write_disk is synchronous - nothing to flush. */
  return UX_SUCCESS;
}

/* Windows probes removable media with GET EVENT STATUS NOTIFICATION;
 * report "no event" instead of leaving the class to stall EP-IN. */
static UINT AppUsbDevice_StorageNotification(VOID *storage_instance, ULONG lun,
                                             ULONG media_id, ULONG notification_class,
                                             UCHAR **media_notification,
                                             ULONG *media_notification_length)
{
  static UCHAR s_no_event[2] = {0U, 0U};

  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_id);
  UX_PARAMETER_NOT_USED(notification_class);

  *media_notification = s_no_event;
  *media_notification_length = sizeof(s_no_event);
  return UX_SUCCESS;
}

/* ------------------------------------------------------------------ */
/* Service thread                                                      */
/* ------------------------------------------------------------------ */

static int32_t AppUsbDevice_StackInit(uint32_t block_count)
{
  UCHAR *framework_hs;
  UCHAR *framework_fs;
  ULONG framework_hs_length;
  ULONG framework_fs_length;
  UCHAR *string_framework;
  ULONG string_framework_length;
  UCHAR *language_id_framework;
  ULONG language_id_length;

  if (ux_system_initialize(s_ux_pool, APP_USB_UX_POOL_BYTES, UX_NULL, 0) != UX_SUCCESS)
  {
    return -3;
  }

  framework_hs = USBD_Get_Device_Framework_Speed(USBD_HIGH_SPEED, &framework_hs_length);
  framework_fs = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED, &framework_fs_length);
  string_framework = USBD_Get_String_Framework(&string_framework_length);
  language_id_framework = USBD_Get_Language_Id_Framework(&language_id_length);

  if (ux_device_stack_initialize(framework_hs, framework_hs_length,
                                 framework_fs, framework_fs_length,
                                 string_framework, string_framework_length,
                                 language_id_framework, language_id_length,
                                 UX_NULL) != UX_SUCCESS)
  {
    return -4;
  }

  memset(&s_storage_parameter, 0, sizeof(s_storage_parameter));
  s_storage_parameter.ux_slave_class_storage_instance_activate = AppUsbDevice_StorageActivate;
  s_storage_parameter.ux_slave_class_storage_instance_deactivate = AppUsbDevice_StorageDeactivate;
  s_storage_parameter.ux_slave_class_storage_parameter_number_lun = 1;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_last_lba = block_count - 1U;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_block_length = SD_NAND_BLOCK_SIZE;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_type = 0;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_removable_flag = 0x80U;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_read_only_flag = UX_FALSE;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_read = AppUsbDevice_StorageRead;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_write = AppUsbDevice_StorageWrite;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_flush = AppUsbDevice_StorageFlush;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_status = AppUsbDevice_StorageStatus;
  s_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_notification = AppUsbDevice_StorageNotification;

  if (ux_device_stack_class_register(_ux_system_slave_class_storage_name,
                                     ux_device_class_storage_entry,
                                     USBD_Get_Configuration_Number(CLASS_TYPE_MSC, 0),
                                     USBD_Get_Interface_Number(CLASS_TYPE_MSC, 0),
                                     &s_storage_parameter) != UX_SUCCESS)
  {
    return -5;
  }

  return 0;
}

static VOID AppUsbDevice_ThreadEntry(ULONG thread_input)
{
  uint32_t block_count = 0U;
  int32_t status;

  (void)thread_input;

  /* MSC capacity is baked into the class registration, so wait for the
   * media thread to bring the SD up (sd_nand caches the card info). No
   * SD = no USB device; the port stays silent, which reads as "cable to
   * a powered-off device" on the PC. */
  while (1)
  {
    block_count = AppMedia_UsbBlockCount();
    if (block_count != 0U)
    {
      break;
    }
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 5U);
  }

  status = AppUsbDevice_StackInit(block_count);
  if (status != 0)
  {
    s_snapshot.last_status = status;
    return;
  }

  status = AppUsbDevice_PcdInit();
  if (status != 0)
  {
    s_snapshot.last_status = status;
    return;
  }

  if (ux_dcd_stm32_initialize((ULONG)USB1_OTG_HS,
                              (ULONG)&g_hpcd_usb1_otg_hs) != UX_SUCCESS)
  {
    s_snapshot.last_status = -6;
    return;
  }

  if (HAL_PCD_Start(&g_hpcd_usb1_otg_hs) != HAL_OK)
  {
    s_snapshot.last_status = -7;
    return;
  }

  s_snapshot.initialized = 1U;
  s_snapshot.last_status = 0;

  /* Publish the enumeration state for UI/GDB. */
  while (1)
  {
    s_snapshot.configured =
        (_ux_system_slave->ux_system_slave_device.ux_slave_device_state ==
         UX_DEVICE_CONFIGURED) ? 1U : 0U;
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 10U);
  }
}

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

UINT AppUsbDevice_Init(VOID)
{
  memset((void *)&s_snapshot, 0, sizeof(s_snapshot));

  return tx_thread_create(&s_usb_thread,
                          (CHAR *)"app_usb_device",
                          AppUsbDevice_ThreadEntry,
                          0,
                          s_usb_thread_stack,
                          sizeof(s_usb_thread_stack),
                          APP_USB_APP_THREAD_PRIORITY,
                          APP_USB_APP_THREAD_PRIORITY,
                          TX_NO_TIME_SLICE,
                          TX_AUTO_START);
}

void AppUsbDevice_GetSnapshot(AppUsbDeviceSnapshot_t *snapshot)
{
  if (snapshot != NULL)
  {
    *snapshot = s_snapshot;
  }
}
