/**
  ******************************************************************************
  * @file    app_media.h
  * @brief   Board SD media filesystem, screenshot, and recording service.
  ******************************************************************************
  */

#ifndef APP_MEDIA_H
#define APP_MEDIA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tx_api.h"

#include <stdint.h>

#define APP_MEDIA_FILE_NAME_LEN        32U
#define APP_MEDIA_PREVIEW_WIDTH        512U
#define APP_MEDIA_PREVIEW_HEIGHT       304U

/* Gallery thumbnails: raw RGB565 sidecar files (.THM) written at capture
 * time, loaded page-by-page into slots for the media grid. */
#define APP_MEDIA_THUMB_WIDTH          176U
#define APP_MEDIA_THUMB_HEIGHT         99U
#define APP_MEDIA_THUMB_SLOTS          8U

#define APP_MEDIA_FLAG_CARD_PRESENT    0x00000001UL
#define APP_MEDIA_FLAG_SD_READY        0x00000002UL
#define APP_MEDIA_FLAG_FS_MOUNTED      0x00000004UL
#define APP_MEDIA_FLAG_FORMATTED       0x00000008UL
#define APP_MEDIA_FLAG_RECORDING       0x00000010UL
#define APP_MEDIA_FLAG_BUSY            0x00000020UL
#define APP_MEDIA_FLAG_PREVIEW_VALID   0x00000040UL
#define APP_MEDIA_FLAG_PLAYING         0x00000080UL
#define APP_MEDIA_FLAG_BEAM_RECORDING  0x00000100UL
/* SD handed over to the USB MSC bridge (FileX closed, PC owns the disk). */
#define APP_MEDIA_FLAG_USB_MODE        0x00000200UL
/* Latest AUDnnnnn.WAV streaming to the speaker (app_beam_play sink). */
#define APP_MEDIA_FLAG_AUDIO_PLAYING   0x00000400UL

typedef enum
{
  APP_MEDIA_ERROR_NONE = 0,
  APP_MEDIA_ERROR_QUEUE_FULL = 1,
  APP_MEDIA_ERROR_SD_INIT = 2,
  APP_MEDIA_ERROR_NO_MEDIA_BLOCKS = 3,
  APP_MEDIA_ERROR_MOUNT = 4,
  APP_MEDIA_ERROR_FORMAT = 5,
  APP_MEDIA_ERROR_FILE_CREATE = 6,
  APP_MEDIA_ERROR_FILE_OPEN = 7,
  APP_MEDIA_ERROR_FILE_WRITE = 8,
  APP_MEDIA_ERROR_FILE_READ = 9,
  APP_MEDIA_ERROR_FILE_CLOSE = 10,
  APP_MEDIA_ERROR_NOT_MOUNTED = 11,
  APP_MEDIA_ERROR_ALREADY_RECORDING = 12,
  APP_MEDIA_ERROR_NOT_RECORDING = 13,
  APP_MEDIA_ERROR_JPEG = 14,
  APP_MEDIA_ERROR_AVI_FINALIZE = 15,
  APP_MEDIA_ERROR_NO_SELECTION = 16,
  APP_MEDIA_ERROR_NO_SPACE = 17,
  APP_MEDIA_ERROR_UNSUPPORTED_FORMAT = 18,
  APP_MEDIA_ERROR_INVALID_MEDIA = 19,
  /* Command refused: the SD belongs to the USB host right now. */
  APP_MEDIA_ERROR_USB_MODE = 20
} AppMediaError_t;

typedef enum
{
  APP_MEDIA_SELECTED_NONE = 0,
  APP_MEDIA_SELECTED_SCREENSHOT = 1,
  APP_MEDIA_SELECTED_VIDEO = 2
} AppMediaSelectedType_t;

typedef struct
{
  uint32_t generation;
  uint32_t valid;
  uint32_t type;
  uint32_t width;
  uint32_t height;
  uint32_t frame_index;
  uint32_t frame_count;
} AppMediaPreviewInfo_t;

typedef struct
{
  uint32_t generation;      /* bumped whenever slot contents change */
  uint32_t total_items;     /* screenshots + videos */
  uint32_t page;            /* 0-based page currently loaded */
  uint32_t page_count;
  uint8_t slot_used[APP_MEDIA_THUMB_SLOTS];   /* item exists on this page */
  uint8_t slot_valid[APP_MEDIA_THUMB_SLOTS];  /* thumb pixels loaded */
  uint8_t slot_type[APP_MEDIA_THUMB_SLOTS];   /* AppMediaSelectedType_t */
  uint32_t slot_index[APP_MEDIA_THUMB_SLOTS]; /* 1-based file index */
} AppMediaThumbInfo_t;

typedef struct
{
  uint32_t flags;
  uint32_t last_error;
  uint32_t sd_status;
  uint32_t mount_status;
  uint32_t format_status;
  uint32_t total_blocks;
  uint32_t media_blocks;
  uint32_t free_clusters;
  uint32_t screenshots;
  uint32_t videos;
  uint32_t selected_index;
  uint32_t selected_type;
  uint32_t record_frames;
  uint32_t dropped_frames;
  uint32_t record_seconds;
  uint32_t beam_seconds;
  uint32_t audio_clips;
  uint32_t audio_play_index;    /* 1-based clip number while playing */
  uint32_t audio_play_seconds;  /* playback position */
  uint32_t last_read_bytes;
  uint32_t preview_generation;
  uint32_t preview_type;
  uint32_t preview_width;
  uint32_t preview_height;
  uint32_t preview_frame_index;
  uint32_t preview_frame_count;
  uint64_t total_bytes;
  uint64_t free_bytes;
  char last_file[APP_MEDIA_FILE_NAME_LEN];
  char selected_file[APP_MEDIA_FILE_NAME_LEN];
} AppMediaStatus_t;

#ifdef DEBUG
/* Per-stage timing of the last recorded frame (ms), for perf HUD/GDB. */
extern volatile uint32_t g_app_media_perf_compose_ms;
extern volatile uint32_t g_app_media_perf_encode_ms;
extern volatile uint32_t g_app_media_perf_write_ms;
#endif

UINT AppMedia_Init(VOID *memory_ptr);
uint32_t AppMedia_RequestScreenshot(void);
uint32_t AppMedia_RequestRecordStart(void);
uint32_t AppMedia_RequestRecordStop(void);
/* Beamformed mono WAV recording (NECCS/AUDIO/AUDnnnnn.WAV). */
uint32_t AppMedia_RequestBeamStart(void);
uint32_t AppMedia_RequestBeamStop(void);
/* Toggle speaker playback of the most recent audio clip (refused while
 * beam recording; beam recording start preempts a running playback). */
uint32_t AppMedia_RequestAudioPlayToggle(void);
uint32_t AppMedia_RequestRefresh(void);
uint32_t AppMedia_RequestSelectNext(void);
uint32_t AppMedia_RequestReadSelected(void);
/* Toggle timed AVI playback of the selected video (5 fps frame stepping). */
uint32_t AppMedia_RequestPlayToggle(void);
/* Load one gallery page of thumbnails (newest first) into the slot buffers. */
uint32_t AppMedia_RequestThumbPage(uint32_t page);
/* Select a specific file (type/1-based index) and read it for preview. */
uint32_t AppMedia_RequestSelectItem(uint32_t type, uint32_t index);
void AppMedia_GetStatus(AppMediaStatus_t *status);
const uint16_t *AppMedia_GetPreviewBuffer(AppMediaPreviewInfo_t *info);
/* Slot pixel buffer (THUMB_W x THUMB_H RGB565); info optionally filled. */
const uint16_t *AppMedia_GetThumbBuffer(uint32_t slot, AppMediaThumbInfo_t *info);

/* ---- USB mass-storage handover ----
 * enable=1: media thread stops recording/playback, closes the FileX
 * volume and flags the SD as PC-owned (MSC reports media present).
 * enable=0: remounts and rescans. The transition is asynchronous; poll
 * AppMedia_UsbModeActive() (it flips only after the handover completed). */
void AppMedia_RequestUsbMode(uint8_t enable);
uint8_t AppMedia_UsbModeActive(void);
uint8_t AppMedia_UsbModeRequested(void);
/* Raw device capacity for the MSC LUN (0 until the SD came up). Matches
 * the FileX view: logical sector n == physical LBA n, boot sector at 0. */
uint32_t AppMedia_UsbBlockCount(void);

#ifdef __cplusplus
}
#endif

#endif
