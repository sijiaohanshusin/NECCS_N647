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

#define APP_MEDIA_FLAG_CARD_PRESENT    0x00000001UL
#define APP_MEDIA_FLAG_SD_READY        0x00000002UL
#define APP_MEDIA_FLAG_FS_MOUNTED      0x00000004UL
#define APP_MEDIA_FLAG_FORMATTED       0x00000008UL
#define APP_MEDIA_FLAG_RECORDING       0x00000010UL
#define APP_MEDIA_FLAG_BUSY            0x00000020UL

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
  APP_MEDIA_ERROR_NO_SPACE = 17
} AppMediaError_t;

typedef enum
{
  APP_MEDIA_SELECTED_NONE = 0,
  APP_MEDIA_SELECTED_SCREENSHOT = 1,
  APP_MEDIA_SELECTED_VIDEO = 2
} AppMediaSelectedType_t;

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
  uint32_t last_read_bytes;
  uint64_t total_bytes;
  uint64_t free_bytes;
  char last_file[APP_MEDIA_FILE_NAME_LEN];
  char selected_file[APP_MEDIA_FILE_NAME_LEN];
} AppMediaStatus_t;

UINT AppMedia_Init(VOID *memory_ptr);
uint32_t AppMedia_RequestScreenshot(void);
uint32_t AppMedia_RequestRecordStart(void);
uint32_t AppMedia_RequestRecordStop(void);
uint32_t AppMedia_RequestRefresh(void);
uint32_t AppMedia_RequestSelectNext(void);
uint32_t AppMedia_RequestReadSelected(void);
void AppMedia_GetStatus(AppMediaStatus_t *status);

#ifdef __cplusplus
}
#endif

#endif
