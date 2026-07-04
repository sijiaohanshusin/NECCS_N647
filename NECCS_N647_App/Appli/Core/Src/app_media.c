/**
  ******************************************************************************
  * @file    app_media.c
  * @brief   Board SD media filesystem, screenshot, and recording service.
  ******************************************************************************
  */

#include "app_media.h"

#include "app_media_jpeg.h"
#include "main.h"
#include "./SD_NAND/sd_nand.h"
#include "fx_api.h"

#include <stdio.h>
#include <string.h>

#define APP_MEDIA_THREAD_STACK_SIZE       12288U
#define APP_MEDIA_THREAD_PRIORITY         12U
#define APP_MEDIA_QUEUE_LENGTH            8U
#define APP_MEDIA_FILEX_CACHE_SIZE        (16U * SD_NAND_BLOCK_SIZE)
#define APP_MEDIA_FORMAT_SECTORS_CLUSTER  32U

#define APP_MEDIA_FB_WIDTH                1024U
#define APP_MEDIA_FB_HEIGHT               600U
#define APP_MEDIA_FB_BYTES                (APP_MEDIA_FB_WIDTH * APP_MEDIA_FB_HEIGHT * 2U)
#define APP_MEDIA_FB_BASE                 0x90000000UL

#define APP_MEDIA_RECORD_WIDTH            512U
#define APP_MEDIA_RECORD_HEIGHT           304U
#define APP_MEDIA_RECORD_FPS              5U
#define APP_MEDIA_RECORD_PERIOD_TICKS     (TX_TIMER_TICKS_PER_SECOND / APP_MEDIA_RECORD_FPS)
#define APP_MEDIA_JPEG_MAX_BYTES          (384U * 1024U)
#define APP_MEDIA_JPEG_DECODE_MAX_BYTES   (APP_MEDIA_PREVIEW_WIDTH * APP_MEDIA_PREVIEW_HEIGHT)
#define APP_MEDIA_JPEG_TIMEOUT_MS         1000U
#define APP_MEDIA_MAX_VIDEO_FRAMES        1800U
#define APP_MEDIA_BMP_ROWS_PER_WRITE      64U
#define APP_MEDIA_MIN_FREE_AFTER_WRITE    (1024ULL * 1024ULL)
#define APP_MEDIA_RECORD_MIN_FREE_BYTES   (4ULL * 1024ULL * 1024ULL)

#define APP_MEDIA_SCREEN_DIR              "NECCS/SCREEN"
#define APP_MEDIA_VIDEO_DIR               "NECCS/VIDEO"
#define APP_MEDIA_ROOT_DIR                "NECCS"

typedef enum
{
  APP_MEDIA_CMD_REFRESH = 1,
  APP_MEDIA_CMD_SCREENSHOT = 2,
  APP_MEDIA_CMD_RECORD_START = 3,
  APP_MEDIA_CMD_RECORD_STOP = 4,
  APP_MEDIA_CMD_SELECT_NEXT = 5,
  APP_MEDIA_CMD_READ_SELECTED = 6
} AppMediaCommand_t;

typedef struct
{
  uint32_t offset;
  uint32_t size;
} AppMediaAviIndex_t;

typedef struct
{
  uint32_t width;
  uint32_t height;
  uint32_t components;
} AppMediaJpegInfo_t;

static TX_THREAD s_media_thread;
static TX_QUEUE s_media_queue;
static TX_MUTEX s_media_status_mutex;
static ULONG s_media_thread_stack[APP_MEDIA_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG s_media_queue_storage[APP_MEDIA_QUEUE_LENGTH];
static uint8_t s_status_mutex_ready = 0U;
static uint8_t s_filex_ready = 0U;

static FX_MEDIA s_media;
static FX_FILE s_work_file;
static FX_FILE s_record_file;
static uint8_t s_filex_cache[APP_MEDIA_FILEX_CACHE_SIZE] __attribute__((aligned(32)));
static uint8_t s_boot_sector[SD_NAND_BLOCK_SIZE] __attribute__((aligned(32)));
static uint8_t s_bmp_row[APP_MEDIA_FB_WIDTH * 3U * APP_MEDIA_BMP_ROWS_PER_WRITE] __attribute__((aligned(32)));
static uint8_t s_jpeg_buffer[APP_MEDIA_JPEG_MAX_BYTES] __attribute__((aligned(32)));
static uint8_t s_jpeg_decode_buffer[APP_MEDIA_JPEG_DECODE_MAX_BYTES] __attribute__((section(".EXTRAM"), aligned(32)));
static uint16_t s_preview_buffer[APP_MEDIA_PREVIEW_WIDTH * APP_MEDIA_PREVIEW_HEIGHT] __attribute__((section(".EXTRAM"), aligned(32)));
static AppMediaAviIndex_t s_avi_index[APP_MEDIA_MAX_VIDEO_FRAMES];

static AppMediaStatus_t s_status;
static JPEG_HandleTypeDef s_jpeg_handle;
static uint32_t s_media_mounted = 0U;
static uint32_t s_jpeg_ready = 0U;
static uint32_t s_recording = 0U;
static uint32_t s_next_screenshot = 1U;
static uint32_t s_next_video = 1U;
static uint32_t s_video_play_frame = 0U;
static uint32_t s_video_play_frame_count = 0U;
static uint32_t s_record_file_pos = 0U;
static uint32_t s_record_started_tick = 0U;
static uint32_t s_record_last_frame_tick = 0U;
static uint32_t s_avi_riff_size_offset = 0U;
static uint32_t s_avi_avih_frames_offset = 0U;
static uint32_t s_avi_strh_frames_offset = 0U;
static uint32_t s_avi_movi_size_offset = 0U;
static uint32_t s_avi_movi_data_start = 0U;
static char s_video_play_path[APP_MEDIA_FILE_NAME_LEN];

static void AppMedia_ThreadEntry(ULONG thread_input);
static void AppMedia_FileXDriver(FX_MEDIA *media_ptr);

static void status_lock(void)
{
  if (s_status_mutex_ready != 0U)
  {
    (void)tx_mutex_get(&s_media_status_mutex, TX_WAIT_FOREVER);
  }
}

static void status_unlock(void)
{
  if (s_status_mutex_ready != 0U)
  {
    (void)tx_mutex_put(&s_media_status_mutex);
  }
}

static void status_set_error(uint32_t error)
{
  status_lock();
  s_status.last_error = error;
  status_unlock();
}

static void status_set_busy(uint32_t busy)
{
  status_lock();
  if (busy != 0U)
  {
    s_status.flags |= APP_MEDIA_FLAG_BUSY;
  }
  else
  {
    s_status.flags &= ~APP_MEDIA_FLAG_BUSY;
  }
  status_unlock();
}

static void status_set_file(char *target, const char *source)
{
  if (source == NULL)
  {
    source = "";
  }

  (void)strncpy(target, source, APP_MEDIA_FILE_NAME_LEN - 1U);
  target[APP_MEDIA_FILE_NAME_LEN - 1U] = '\0';
}

static uint32_t media_min_u32(uint32_t a, uint32_t b)
{
  return (a < b) ? a : b;
}

static uint16_t gray_to_rgb565(uint8_t value)
{
  return (uint16_t)(((uint16_t)(value & 0xF8U) << 8) |
                    ((uint16_t)(value & 0xFCU) << 3) |
                    ((uint16_t)value >> 3));
}

static void clean_preview_cache(void)
{
  SCB_CleanDCache_by_Addr((void *)s_preview_buffer, (int32_t)sizeof(s_preview_buffer));
  __DSB();
}

static void reset_video_playback(void)
{
  s_video_play_frame = 0U;
  s_video_play_frame_count = 0U;
  status_set_file(s_video_play_path, "");
}

static void preview_clear(void)
{
  memset(s_preview_buffer, 0, sizeof(s_preview_buffer));
  clean_preview_cache();

  status_lock();
  s_status.flags &= ~APP_MEDIA_FLAG_PREVIEW_VALID;
  s_status.preview_generation++;
  s_status.preview_type = APP_MEDIA_SELECTED_NONE;
  s_status.preview_width = 0U;
  s_status.preview_height = 0U;
  s_status.preview_frame_index = 0U;
  s_status.preview_frame_count = 0U;
  status_unlock();
}

static void preview_commit(uint32_t type, uint32_t frame_index, uint32_t frame_count)
{
  clean_preview_cache();

  status_lock();
  s_status.flags |= APP_MEDIA_FLAG_PREVIEW_VALID;
  s_status.preview_generation++;
  s_status.preview_type = type;
  s_status.preview_width = APP_MEDIA_PREVIEW_WIDTH;
  s_status.preview_height = APP_MEDIA_PREVIEW_HEIGHT;
  s_status.preview_frame_index = frame_index;
  s_status.preview_frame_count = frame_count;
  status_unlock();
}

static UINT file_read_exact(FX_FILE *file, void *buffer, ULONG bytes, uint32_t *bytes_read)
{
  ULONG actual = 0U;
  UINT status = fx_file_read(file, buffer, bytes, &actual);

  if (bytes_read != NULL)
  {
    *bytes_read += actual;
  }

  if (status != FX_SUCCESS)
  {
    return status;
  }

  return (actual == bytes) ? FX_SUCCESS : FX_END_OF_FILE;
}

static uint32_t post_command(AppMediaCommand_t command)
{
  ULONG message = (ULONG)command;

  if (tx_queue_send(&s_media_queue, &message, TX_NO_WAIT) != TX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_QUEUE_FULL);
    return APP_MEDIA_ERROR_QUEUE_FULL;
  }

  status_lock();
  s_status.flags |= APP_MEDIA_FLAG_BUSY;
  s_status.last_error = APP_MEDIA_ERROR_NONE;
  status_unlock();

  return APP_MEDIA_ERROR_NONE;
}

static void put_u16_le(uint8_t *buffer, uint16_t value)
{
  buffer[0] = (uint8_t)value;
  buffer[1] = (uint8_t)(value >> 8);
}

static uint16_t get_u16_le(const uint8_t *buffer)
{
  return (uint16_t)(((uint16_t)buffer[1] << 8) | (uint16_t)buffer[0]);
}

static void put_u32_le(uint8_t *buffer, uint32_t value)
{
  buffer[0] = (uint8_t)value;
  buffer[1] = (uint8_t)(value >> 8);
  buffer[2] = (uint8_t)(value >> 16);
  buffer[3] = (uint8_t)(value >> 24);
}

static uint32_t get_u32_le(const uint8_t *buffer)
{
  return (((uint32_t)buffer[3]) << 24) |
         (((uint32_t)buffer[2]) << 16) |
         (((uint32_t)buffer[1]) << 8) |
         ((uint32_t)buffer[0]);
}

static uint32_t app_media_visible_blocks(void)
{
  const uint32_t total_blocks = sd_nand_get_block_count();

  if (total_blocks <= SD_NAND_FONT_RESERVED_BLOCKS)
  {
    return 0U;
  }

  return total_blocks - SD_NAND_FONT_RESERVED_BLOCKS;
}

static uint32_t app_media_boot_sector_matches(uint32_t media_blocks)
{
  static const uint8_t volume_label[11] = {'N', 'E', 'C', 'C', 'S', ' ', ' ', ' ', ' ', ' ', ' '};
  uint32_t total_sectors;
  uint8_t sectors_per_cluster;

  if (sd_nand_read_disk(s_boot_sector, 0U, 1U) != SD_NAND_OK)
  {
    return 0U;
  }

  if ((s_boot_sector[510] != 0x55U) || (s_boot_sector[511] != 0xAAU))
  {
    return 0U;
  }
  if ((s_boot_sector[0] != 0xEBU) || (s_boot_sector[2] != 0x90U))
  {
    return 0U;
  }
  if (get_u16_le(&s_boot_sector[FX_BYTES_SECTOR]) != SD_NAND_BLOCK_SIZE)
  {
    return 0U;
  }
  if ((s_boot_sector[FX_NUMBER_OF_FATS] != 1U) ||
      (get_u16_le(&s_boot_sector[FX_ROOT_DIR_ENTRIES]) != 0U) ||
      (s_boot_sector[FX_BOOT_SIG_32] != 0x29U) ||
      (get_u32_le(&s_boot_sector[FX_SECTORS_PER_FAT_32]) == 0U))
  {
    return 0U;
  }

  sectors_per_cluster = s_boot_sector[FX_SECTORS_CLUSTER];
  if ((sectors_per_cluster != 8U) && (sectors_per_cluster != APP_MEDIA_FORMAT_SECTORS_CLUSTER))
  {
    return 0U;
  }

  total_sectors = get_u16_le(&s_boot_sector[FX_SECTORS]);
  if (total_sectors == 0U)
  {
    total_sectors = get_u32_le(&s_boot_sector[FX_HUGE_SECTORS]);
  }
  if (total_sectors != media_blocks)
  {
    return 0U;
  }

  if (memcmp(&s_boot_sector[FX_VOLUME_LABEL_32], volume_label, sizeof(volume_label)) != 0)
  {
    return 0U;
  }

  return 1U;
}

static void update_space_status(void)
{
  ULONG available_clusters = 0U;
  uint64_t free_bytes = 0U;

  if (s_media_mounted != 0U)
  {
    if (fx_media_space_available(&s_media, &available_clusters) == FX_SUCCESS)
    {
      free_bytes = (uint64_t)available_clusters *
                   (uint64_t)s_media.fx_media_sectors_per_cluster *
                   (uint64_t)s_media.fx_media_bytes_per_sector;
    }
  }

  status_lock();
  s_status.free_clusters = available_clusters;
  s_status.free_bytes = free_bytes;
  status_unlock();
}

static uint32_t has_free_space(uint64_t required_bytes)
{
  uint64_t free_bytes;

  update_space_status();

  status_lock();
  free_bytes = s_status.free_bytes;
  status_unlock();

  return (free_bytes >= required_bytes) ? 1U : 0U;
}

static UINT ensure_directory(const CHAR *path)
{
  UINT status = fx_directory_create(&s_media, (CHAR *)path);

  if (status == FX_ALREADY_CREATED)
  {
    status = FX_SUCCESS;
  }

  return status;
}

static void make_screenshot_path(uint32_t index, char *path, uint32_t path_len)
{
  (void)snprintf(path, path_len, APP_MEDIA_SCREEN_DIR "/SCR%05lu.BMP", (unsigned long)index);
}

static void make_video_path(uint32_t index, char *path, uint32_t path_len)
{
  (void)snprintf(path, path_len, APP_MEDIA_VIDEO_DIR "/VID%05lu.AVI", (unsigned long)index);
}

static uint32_t file_exists(const char *path)
{
  UINT status = fx_file_open(&s_media, &s_work_file, (CHAR *)path, FX_OPEN_FOR_READ);

  if (status == FX_SUCCESS)
  {
    (void)fx_file_close(&s_work_file);
    return 1U;
  }

  return 0U;
}

static uint32_t count_sequence(uint32_t is_video)
{
  char path[APP_MEDIA_FILE_NAME_LEN];
  uint32_t count = 0U;

  for (uint32_t i = 1U; i <= 99999U; ++i)
  {
    if (is_video != 0U)
    {
      make_video_path(i, path, sizeof(path));
    }
    else
    {
      make_screenshot_path(i, path, sizeof(path));
    }

    if (file_exists(path) == 0U)
    {
      break;
    }
    count = i;
  }

  return count;
}

static void select_latest(void)
{
  char path[APP_MEDIA_FILE_NAME_LEN];

  status_lock();
  if (s_status.videos != 0U)
  {
    s_status.selected_type = APP_MEDIA_SELECTED_VIDEO;
    s_status.selected_index = s_status.screenshots + s_status.videos - 1U;
    make_video_path(s_status.videos, path, sizeof(path));
    status_set_file(s_status.selected_file, path);
  }
  else if (s_status.screenshots != 0U)
  {
    s_status.selected_type = APP_MEDIA_SELECTED_SCREENSHOT;
    s_status.selected_index = s_status.screenshots - 1U;
    make_screenshot_path(s_status.screenshots, path, sizeof(path));
    status_set_file(s_status.selected_file, path);
  }
  else
  {
    s_status.selected_type = APP_MEDIA_SELECTED_NONE;
    s_status.selected_index = 0U;
    status_set_file(s_status.selected_file, "");
  }
  status_unlock();
}

static void scan_media_files(void)
{
  const uint32_t screenshots = count_sequence(0U);
  const uint32_t videos = count_sequence(1U);

  status_lock();
  s_status.screenshots = screenshots;
  s_status.videos = videos;
  status_unlock();

  s_next_screenshot = screenshots + 1U;
  s_next_video = videos + 1U;
  select_latest();
  reset_video_playback();
  preview_clear();
}

static UINT mount_or_format_media(void)
{
  UINT status;
  uint32_t media_blocks;
  uint32_t preserved_flags;
  uint8_t sd_status;

  status_lock();
  preserved_flags = s_status.flags & (APP_MEDIA_FLAG_BUSY | APP_MEDIA_FLAG_RECORDING);
  s_status.flags = preserved_flags;
  s_status.sd_status = sd_nand_is_inserted();
  s_status.total_blocks = sd_nand_get_block_count();
  s_status.media_blocks = 0U;
  s_status.total_bytes = 0U;
  if (s_status.sd_status != 0U)
  {
    s_status.flags |= APP_MEDIA_FLAG_CARD_PRESENT;
  }
  status_unlock();

  sd_status = sd_nand_init();
  status_lock();
  s_status.sd_status = sd_status;
  status_unlock();
  if (sd_status != SD_NAND_OK)
  {
    status_set_error(APP_MEDIA_ERROR_SD_INIT);
    return FX_IO_ERROR;
  }

  media_blocks = app_media_visible_blocks();
  if (media_blocks == 0U)
  {
    status_set_error(APP_MEDIA_ERROR_NO_MEDIA_BLOCKS);
    return FX_IO_ERROR;
  }

  status_lock();
  s_status.flags |= APP_MEDIA_FLAG_SD_READY;
  s_status.total_blocks = sd_nand_get_block_count();
  s_status.media_blocks = media_blocks;
  s_status.total_bytes = (uint64_t)media_blocks * (uint64_t)SD_NAND_BLOCK_SIZE;
  status_unlock();

  if (app_media_boot_sector_matches(media_blocks) != 0U)
  {
    status = fx_media_open(&s_media,
                           (CHAR *)"NECCS_SD",
                           AppMedia_FileXDriver,
                           FX_NULL,
                           s_filex_cache,
                           sizeof(s_filex_cache));
  }
  else
  {
    status = FX_MEDIA_INVALID;
  }
  status_lock();
  s_status.mount_status = status;
  status_unlock();

  if (status != FX_SUCCESS)
  {
    status = fx_media_format(&s_media,
                             AppMedia_FileXDriver,
                             FX_NULL,
                             s_filex_cache,
                             sizeof(s_filex_cache),
                             (CHAR *)"NECCS",
                             1U,
                             0U,
                             0U,
                             media_blocks,
                             SD_NAND_BLOCK_SIZE,
                             APP_MEDIA_FORMAT_SECTORS_CLUSTER,
                             1U,
                             1U);
    status_lock();
    s_status.format_status = status;
    if (status == FX_SUCCESS)
    {
      s_status.flags |= APP_MEDIA_FLAG_FORMATTED;
    }
    status_unlock();

    if (status != FX_SUCCESS)
    {
      status_set_error(APP_MEDIA_ERROR_FORMAT);
      return status;
    }

    status = fx_media_open(&s_media,
                           (CHAR *)"NECCS_SD",
                           AppMedia_FileXDriver,
                           FX_NULL,
                           s_filex_cache,
                           sizeof(s_filex_cache));
    status_lock();
    s_status.mount_status = status;
    status_unlock();
  }

  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_MOUNT);
    return status;
  }

  s_media_mounted = 1U;
  status_lock();
  s_status.flags |= APP_MEDIA_FLAG_FS_MOUNTED;
  s_status.last_error = APP_MEDIA_ERROR_NONE;
  status_unlock();

  status = ensure_directory((const CHAR *)APP_MEDIA_ROOT_DIR);
  if (status == FX_SUCCESS)
  {
    status = ensure_directory((const CHAR *)APP_MEDIA_SCREEN_DIR);
  }
  if (status == FX_SUCCESS)
  {
    status = ensure_directory((const CHAR *)APP_MEDIA_VIDEO_DIR);
  }

  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_FILE_CREATE);
    return status;
  }

  scan_media_files();
  update_space_status();
  return FX_SUCCESS;
}

static const uint16_t *framebuffer_ptr(void)
{
  uint32_t address = APP_MEDIA_FB_BASE;

#if defined(LTDC_Layer2)
  if (LTDC_Layer2->CFBAR != 0U)
  {
    address = LTDC_Layer2->CFBAR;
  }
#endif

  return (const uint16_t *)address;
}

static void clean_framebuffer_cache(const uint16_t *framebuffer)
{
  SCB_CleanDCache_by_Addr((void *)framebuffer, (int32_t)APP_MEDIA_FB_BYTES);
  __DSB();
}

static uint32_t ensure_jpeg_ready(void)
{
  if (s_jpeg_ready != 0U)
  {
    return APP_MEDIA_ERROR_NONE;
  }

  __HAL_RCC_JPEG_CLK_ENABLE();
  __HAL_RCC_JPEG_FORCE_RESET();
  __HAL_RCC_JPEG_RELEASE_RESET();

  memset(&s_jpeg_handle, 0, sizeof(s_jpeg_handle));
  s_jpeg_handle.Instance = JPEG;
  if (HAL_JPEG_Init(&s_jpeg_handle) != HAL_OK)
  {
    status_set_error(APP_MEDIA_ERROR_JPEG);
    return APP_MEDIA_ERROR_JPEG;
  }

  s_jpeg_ready = 1U;
  return APP_MEDIA_ERROR_NONE;
}

static uint32_t write_bmp_screenshot(void)
{
  char path[APP_MEDIA_FILE_NAME_LEN];
  uint8_t header[54];
  UINT status;
  const uint16_t *framebuffer = framebuffer_ptr();
  const uint32_t row_bytes = APP_MEDIA_FB_WIDTH * 3U;
  const uint32_t pixel_bytes = row_bytes * APP_MEDIA_FB_HEIGHT;
  const uint32_t file_bytes = sizeof(header) + pixel_bytes;

  if (s_media_mounted == 0U)
  {
    status_set_error(APP_MEDIA_ERROR_NOT_MOUNTED);
    return APP_MEDIA_ERROR_NOT_MOUNTED;
  }

  if (has_free_space((uint64_t)file_bytes + APP_MEDIA_MIN_FREE_AFTER_WRITE) == 0U)
  {
    status_set_error(APP_MEDIA_ERROR_NO_SPACE);
    return APP_MEDIA_ERROR_NO_SPACE;
  }

  make_screenshot_path(s_next_screenshot, path, sizeof(path));
  (void)fx_file_delete(&s_media, (CHAR *)path);
  status = fx_file_create(&s_media, (CHAR *)path);
  if ((status != FX_SUCCESS) && (status != FX_ALREADY_CREATED))
  {
    status_set_error(APP_MEDIA_ERROR_FILE_CREATE);
    return APP_MEDIA_ERROR_FILE_CREATE;
  }

  status = fx_file_open(&s_media, &s_work_file, (CHAR *)path, FX_OPEN_FOR_WRITE);
  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_FILE_OPEN);
    return APP_MEDIA_ERROR_FILE_OPEN;
  }

  memset(header, 0, sizeof(header));
  header[0] = 'B';
  header[1] = 'M';
  put_u32_le(&header[2], file_bytes);
  put_u32_le(&header[10], sizeof(header));
  put_u32_le(&header[14], 40U);
  put_u32_le(&header[18], APP_MEDIA_FB_WIDTH);
  put_u32_le(&header[22], APP_MEDIA_FB_HEIGHT);
  put_u16_le(&header[26], 1U);
  put_u16_le(&header[28], 24U);
  put_u32_le(&header[34], pixel_bytes);

  clean_framebuffer_cache(framebuffer);

  status = fx_file_write(&s_work_file, header, sizeof(header));
  if (status == FX_SUCCESS)
  {
    uint32_t rows_in_chunk = 0U;

    for (int32_t row = (int32_t)APP_MEDIA_FB_HEIGHT - 1; row >= 0; --row)
    {
      uint8_t *dst = &s_bmp_row[rows_in_chunk * row_bytes];
      const uint16_t *src = &framebuffer[(uint32_t)row * APP_MEDIA_FB_WIDTH];

      for (uint32_t col = 0U; col < APP_MEDIA_FB_WIDTH; ++col)
      {
        const uint16_t pixel = src[col];
        const uint8_t r5 = (uint8_t)((pixel >> 11) & 0x1FU);
        const uint8_t g6 = (uint8_t)((pixel >> 5) & 0x3FU);
        const uint8_t b5 = (uint8_t)(pixel & 0x1FU);
        *dst++ = (uint8_t)((b5 << 3) | (b5 >> 2));
        *dst++ = (uint8_t)((g6 << 2) | (g6 >> 4));
        *dst++ = (uint8_t)((r5 << 3) | (r5 >> 2));
      }

      rows_in_chunk++;
      if ((rows_in_chunk >= APP_MEDIA_BMP_ROWS_PER_WRITE) || (row == 0))
      {
        status = fx_file_write(&s_work_file, s_bmp_row, rows_in_chunk * row_bytes);
        if (status != FX_SUCCESS)
        {
          break;
        }
        rows_in_chunk = 0U;
      }
    }
  }

  if (status == FX_SUCCESS)
  {
    status = fx_file_close(&s_work_file);
  }
  else
  {
    (void)fx_file_close(&s_work_file);
  }

  (void)fx_media_flush(&s_media);

  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_FILE_WRITE);
    return APP_MEDIA_ERROR_FILE_WRITE;
  }

  status_lock();
  s_status.screenshots = s_next_screenshot;
  s_status.selected_type = APP_MEDIA_SELECTED_SCREENSHOT;
  s_status.selected_index = s_next_screenshot - 1U;
  status_set_file(s_status.last_file, path);
  status_set_file(s_status.selected_file, path);
  s_status.last_error = APP_MEDIA_ERROR_NONE;
  status_unlock();

  reset_video_playback();
  preview_clear();
  s_next_screenshot++;
  update_space_status();
  return APP_MEDIA_ERROR_NONE;
}

static UINT avi_write_bytes(const void *data, ULONG size)
{
  UINT status = fx_file_write(&s_record_file, (VOID *)data, size);

  if (status == FX_SUCCESS)
  {
    s_record_file_pos += size;
  }

  return status;
}

static UINT avi_write_tag(const char tag[4])
{
  return avi_write_bytes(tag, 4U);
}

static UINT avi_write_u16(uint16_t value)
{
  uint8_t buffer[2];
  put_u16_le(buffer, value);
  return avi_write_bytes(buffer, sizeof(buffer));
}

static UINT avi_write_u32(uint32_t value)
{
  uint8_t buffer[4];
  put_u32_le(buffer, value);
  return avi_write_bytes(buffer, sizeof(buffer));
}

static UINT avi_patch_u32(uint32_t offset, uint32_t value)
{
  uint8_t buffer[4];
  UINT status;

  put_u32_le(buffer, value);
  status = fx_file_seek(&s_record_file, offset);
  if (status == FX_SUCCESS)
  {
    status = fx_file_write(&s_record_file, buffer, sizeof(buffer));
  }

  return status;
}

static UINT avi_write_header(void)
{
  UINT status;
  uint32_t hdrl_size_offset;
  uint32_t hdrl_payload_start;
  uint32_t strl_size_offset;
  uint32_t strl_payload_start;
  uint32_t payload_end;

#define AVI_CHECK(expr) do { status = (expr); if (status != FX_SUCCESS) { return status; } } while (0)

  AVI_CHECK(avi_write_tag("RIFF"));
  s_avi_riff_size_offset = s_record_file_pos;
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_tag("AVI "));

  AVI_CHECK(avi_write_tag("LIST"));
  hdrl_size_offset = s_record_file_pos;
  AVI_CHECK(avi_write_u32(0U));
  hdrl_payload_start = s_record_file_pos;
  AVI_CHECK(avi_write_tag("hdrl"));

  AVI_CHECK(avi_write_tag("avih"));
  AVI_CHECK(avi_write_u32(56U));
  AVI_CHECK(avi_write_u32(1000000U / APP_MEDIA_RECORD_FPS));
  AVI_CHECK(avi_write_u32(900000U));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(0x10U));
  s_avi_avih_frames_offset = s_record_file_pos;
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(1U));
  AVI_CHECK(avi_write_u32(APP_MEDIA_JPEG_MAX_BYTES));
  AVI_CHECK(avi_write_u32(APP_MEDIA_RECORD_WIDTH));
  AVI_CHECK(avi_write_u32(APP_MEDIA_RECORD_HEIGHT));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(0U));

  AVI_CHECK(avi_write_tag("LIST"));
  strl_size_offset = s_record_file_pos;
  AVI_CHECK(avi_write_u32(0U));
  strl_payload_start = s_record_file_pos;
  AVI_CHECK(avi_write_tag("strl"));

  AVI_CHECK(avi_write_tag("strh"));
  AVI_CHECK(avi_write_u32(56U));
  AVI_CHECK(avi_write_tag("vids"));
  AVI_CHECK(avi_write_tag("MJPG"));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u16(0U));
  AVI_CHECK(avi_write_u16(0U));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(1U));
  AVI_CHECK(avi_write_u32(APP_MEDIA_RECORD_FPS));
  AVI_CHECK(avi_write_u32(0U));
  s_avi_strh_frames_offset = s_record_file_pos;
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(APP_MEDIA_JPEG_MAX_BYTES));
  AVI_CHECK(avi_write_u32(0xFFFFFFFFUL));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u16(0U));
  AVI_CHECK(avi_write_u16(0U));
  AVI_CHECK(avi_write_u16(APP_MEDIA_RECORD_WIDTH));
  AVI_CHECK(avi_write_u16(APP_MEDIA_RECORD_HEIGHT));

  AVI_CHECK(avi_write_tag("strf"));
  AVI_CHECK(avi_write_u32(40U));
  AVI_CHECK(avi_write_u32(40U));
  AVI_CHECK(avi_write_u32(APP_MEDIA_RECORD_WIDTH));
  AVI_CHECK(avi_write_u32(APP_MEDIA_RECORD_HEIGHT));
  AVI_CHECK(avi_write_u16(1U));
  AVI_CHECK(avi_write_u16(24U));
  AVI_CHECK(avi_write_tag("MJPG"));
  AVI_CHECK(avi_write_u32(APP_MEDIA_RECORD_WIDTH * APP_MEDIA_RECORD_HEIGHT * 3U));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_u32(0U));

  payload_end = s_record_file_pos;
  AVI_CHECK(avi_patch_u32(strl_size_offset, payload_end - strl_payload_start));
  AVI_CHECK(fx_file_seek(&s_record_file, s_record_file_pos));

  payload_end = s_record_file_pos;
  AVI_CHECK(avi_patch_u32(hdrl_size_offset, payload_end - hdrl_payload_start));
  AVI_CHECK(fx_file_seek(&s_record_file, s_record_file_pos));

  AVI_CHECK(avi_write_tag("LIST"));
  s_avi_movi_size_offset = s_record_file_pos;
  AVI_CHECK(avi_write_u32(0U));
  AVI_CHECK(avi_write_tag("movi"));
  s_avi_movi_data_start = s_record_file_pos;

#undef AVI_CHECK

  return FX_SUCCESS;
}

static UINT avi_append_frame(void)
{
  uint32_t jpeg_size = 0U;
  uint32_t chunk_start;
  UINT status;
  const uint16_t *framebuffer = framebuffer_ptr();

  if (s_status.record_frames >= APP_MEDIA_MAX_VIDEO_FRAMES)
  {
    return FX_SUCCESS;
  }

  clean_framebuffer_cache(framebuffer);
  if (AppMediaJpeg_EncodeGrayFromRgb565(framebuffer,
                                        APP_MEDIA_FB_WIDTH,
                                        APP_MEDIA_FB_HEIGHT,
                                        APP_MEDIA_RECORD_WIDTH,
                                        APP_MEDIA_RECORD_HEIGHT,
                                        s_jpeg_buffer,
                                        sizeof(s_jpeg_buffer),
                                        &jpeg_size) != APP_MEDIA_JPEG_OK)
  {
    status_lock();
    s_status.dropped_frames++;
    status_unlock();
    status_set_error(APP_MEDIA_ERROR_JPEG);
    return FX_SUCCESS;
  }

  chunk_start = s_record_file_pos;
  status = avi_write_tag("00dc");
  if (status == FX_SUCCESS)
  {
    status = avi_write_u32(jpeg_size);
  }
  if (status == FX_SUCCESS)
  {
    status = avi_write_bytes(s_jpeg_buffer, jpeg_size);
  }
  if ((status == FX_SUCCESS) && ((jpeg_size & 1U) != 0U))
  {
    static const uint8_t pad = 0U;
    status = avi_write_bytes(&pad, 1U);
  }

  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_FILE_WRITE);
    return status;
  }

  s_avi_index[s_status.record_frames].offset = chunk_start - s_avi_movi_data_start;
  s_avi_index[s_status.record_frames].size = jpeg_size;

  status_lock();
  s_status.record_frames++;
  s_status.record_seconds = (tx_time_get() - s_record_started_tick) / TX_TIMER_TICKS_PER_SECOND;
  status_unlock();

  return FX_SUCCESS;
}

static UINT avi_finalize(void)
{
  UINT status = FX_SUCCESS;
  const uint32_t frames = s_status.record_frames;
  const uint32_t movi_end = s_record_file_pos;

  status = avi_write_tag("idx1");
  if (status == FX_SUCCESS)
  {
    status = avi_write_u32(frames * 16U);
  }

  for (uint32_t i = 0U; (i < frames) && (status == FX_SUCCESS); ++i)
  {
    status = avi_write_tag("00dc");
    if (status == FX_SUCCESS)
    {
      status = avi_write_u32(0x10U);
    }
    if (status == FX_SUCCESS)
    {
      status = avi_write_u32(s_avi_index[i].offset);
    }
    if (status == FX_SUCCESS)
    {
      status = avi_write_u32(s_avi_index[i].size);
    }
  }

  if (status == FX_SUCCESS)
  {
    status = avi_patch_u32(s_avi_riff_size_offset, s_record_file_pos - 8U);
  }
  if (status == FX_SUCCESS)
  {
    status = avi_patch_u32(s_avi_avih_frames_offset, frames);
  }
  if (status == FX_SUCCESS)
  {
    status = avi_patch_u32(s_avi_strh_frames_offset, frames);
  }
  if (status == FX_SUCCESS)
  {
    status = avi_patch_u32(s_avi_movi_size_offset, movi_end - (s_avi_movi_size_offset + 4U));
  }
  if (status == FX_SUCCESS)
  {
    status = fx_file_close(&s_record_file);
  }
  else
  {
    (void)fx_file_close(&s_record_file);
  }

  (void)fx_media_flush(&s_media);
  return status;
}

static uint32_t start_recording(void)
{
  char path[APP_MEDIA_FILE_NAME_LEN];
  UINT status;

  if (s_media_mounted == 0U)
  {
    status_set_error(APP_MEDIA_ERROR_NOT_MOUNTED);
    return APP_MEDIA_ERROR_NOT_MOUNTED;
  }

  if (s_recording != 0U)
  {
    status_set_error(APP_MEDIA_ERROR_ALREADY_RECORDING);
    return APP_MEDIA_ERROR_ALREADY_RECORDING;
  }

  if (has_free_space(APP_MEDIA_RECORD_MIN_FREE_BYTES) == 0U)
  {
    status_set_error(APP_MEDIA_ERROR_NO_SPACE);
    return APP_MEDIA_ERROR_NO_SPACE;
  }

  make_video_path(s_next_video, path, sizeof(path));
  (void)fx_file_delete(&s_media, (CHAR *)path);
  status = fx_file_create(&s_media, (CHAR *)path);
  if ((status != FX_SUCCESS) && (status != FX_ALREADY_CREATED))
  {
    status_set_error(APP_MEDIA_ERROR_FILE_CREATE);
    return APP_MEDIA_ERROR_FILE_CREATE;
  }

  status = fx_file_open(&s_media, &s_record_file, (CHAR *)path, FX_OPEN_FOR_WRITE);
  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_FILE_OPEN);
    return APP_MEDIA_ERROR_FILE_OPEN;
  }

  s_record_file_pos = 0U;
  status_lock();
  s_status.record_frames = 0U;
  s_status.dropped_frames = 0U;
  s_status.record_seconds = 0U;
  status_unlock();

  status = avi_write_header();
  if (status != FX_SUCCESS)
  {
    (void)fx_file_close(&s_record_file);
    status_set_error(APP_MEDIA_ERROR_FILE_WRITE);
    return APP_MEDIA_ERROR_FILE_WRITE;
  }

  s_recording = 1U;
  s_record_started_tick = tx_time_get();
  s_record_last_frame_tick = 0U;

  status_lock();
  s_status.flags |= APP_MEDIA_FLAG_RECORDING;
  status_set_file(s_status.last_file, path);
  status_set_file(s_status.selected_file, path);
  s_status.selected_type = APP_MEDIA_SELECTED_VIDEO;
  s_status.selected_index = s_status.screenshots + s_next_video - 1U;
  s_status.last_error = APP_MEDIA_ERROR_NONE;
  status_unlock();

  reset_video_playback();
  preview_clear();
  return APP_MEDIA_ERROR_NONE;
}

static uint32_t stop_recording(void)
{
  UINT status;

  if (s_recording == 0U)
  {
    status_set_error(APP_MEDIA_ERROR_NOT_RECORDING);
    return APP_MEDIA_ERROR_NOT_RECORDING;
  }

  status = avi_finalize();
  s_recording = 0U;

  status_lock();
  s_status.flags &= ~APP_MEDIA_FLAG_RECORDING;
  status_unlock();

  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_AVI_FINALIZE);
    return APP_MEDIA_ERROR_AVI_FINALIZE;
  }

  status_lock();
  s_status.videos = s_next_video;
  s_status.last_error = APP_MEDIA_ERROR_NONE;
  status_unlock();

  reset_video_playback();
  preview_clear();
  s_next_video++;
  update_space_status();
  return APP_MEDIA_ERROR_NONE;
}

static void process_record_tick(void)
{
  const uint32_t now = tx_time_get();

  if (s_recording == 0U)
  {
    return;
  }

  if ((s_record_last_frame_tick == 0U) ||
      ((now - s_record_last_frame_tick) >= APP_MEDIA_RECORD_PERIOD_TICKS))
  {
    s_record_last_frame_tick = now;
    if (avi_append_frame() != FX_SUCCESS)
    {
      (void)stop_recording();
    }
  }
}

static uint32_t select_next_file(void)
{
  char path[APP_MEDIA_FILE_NAME_LEN];
  uint32_t screenshots;
  uint32_t videos;
  uint32_t total;
  uint32_t next;

  status_lock();
  screenshots = s_status.screenshots;
  videos = s_status.videos;
  total = screenshots + videos;
  if (total == 0U)
  {
    s_status.selected_type = APP_MEDIA_SELECTED_NONE;
    s_status.selected_index = 0U;
    status_set_file(s_status.selected_file, "");
    status_unlock();
    status_set_error(APP_MEDIA_ERROR_NO_SELECTION);
    return APP_MEDIA_ERROR_NO_SELECTION;
  }

  next = (s_status.selected_index + 1U) % total;
  s_status.selected_index = next;
  if (next < screenshots)
  {
    s_status.selected_type = APP_MEDIA_SELECTED_SCREENSHOT;
    make_screenshot_path(next + 1U, path, sizeof(path));
  }
  else
  {
    s_status.selected_type = APP_MEDIA_SELECTED_VIDEO;
    make_video_path((next - screenshots) + 1U, path, sizeof(path));
  }
  status_set_file(s_status.selected_file, path);
  s_status.last_error = APP_MEDIA_ERROR_NONE;
  status_unlock();

  reset_video_playback();
  preview_clear();
  return APP_MEDIA_ERROR_NONE;
}

static uint32_t jpeg_parse_info(const uint8_t *data, uint32_t size, AppMediaJpegInfo_t *info)
{
  uint32_t pos = 2U;

  if ((data == NULL) || (info == NULL) || (size < 4U) ||
      (data[0] != 0xFFU) || (data[1] != 0xD8U))
  {
    return APP_MEDIA_ERROR_INVALID_MEDIA;
  }

  memset(info, 0, sizeof(*info));

  while ((pos + 4U) <= size)
  {
    uint8_t marker;
    uint32_t segment_len;

    while ((pos < size) && (data[pos] != 0xFFU))
    {
      pos++;
    }
    while ((pos < size) && (data[pos] == 0xFFU))
    {
      pos++;
    }
    if (pos >= size)
    {
      break;
    }

    marker = data[pos++];
    if ((marker == 0xD8U) || (marker == 0xD9U))
    {
      continue;
    }
    if ((marker >= 0xD0U) && (marker <= 0xD7U))
    {
      continue;
    }
    if ((pos + 2U) > size)
    {
      return APP_MEDIA_ERROR_INVALID_MEDIA;
    }

    segment_len = ((uint32_t)data[pos] << 8) | (uint32_t)data[pos + 1U];
    if ((segment_len < 2U) || ((pos + segment_len) > size))
    {
      return APP_MEDIA_ERROR_INVALID_MEDIA;
    }

    if (marker == 0xC0U)
    {
      if (segment_len < 8U)
      {
        return APP_MEDIA_ERROR_INVALID_MEDIA;
      }
      info->height = ((uint32_t)data[pos + 3U] << 8) | (uint32_t)data[pos + 4U];
      info->width = ((uint32_t)data[pos + 5U] << 8) | (uint32_t)data[pos + 6U];
      info->components = data[pos + 7U];
      return APP_MEDIA_ERROR_NONE;
    }

    if (marker == 0xDAU)
    {
      break;
    }

    pos += segment_len;
  }

  return APP_MEDIA_ERROR_INVALID_MEDIA;
}

static uint32_t decode_jpeg_preview(uint32_t jpeg_size, uint32_t frame_index, uint32_t frame_count)
{
  AppMediaJpegInfo_t info;
  uint32_t padded_size;
  uint32_t mcu_cols;
  uint32_t mcu_rows;
  uint32_t output_bytes;
  uint32_t dst_x0;
  uint32_t dst_y0;
  const uint8_t *src;

  if (jpeg_parse_info(s_jpeg_buffer, jpeg_size, &info) != APP_MEDIA_ERROR_NONE)
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_INVALID_MEDIA);
    return APP_MEDIA_ERROR_INVALID_MEDIA;
  }

  if ((info.components != 1U) || (info.width == 0U) || (info.height == 0U) ||
      (info.width > APP_MEDIA_PREVIEW_WIDTH) || (info.height > APP_MEDIA_PREVIEW_HEIGHT))
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_UNSUPPORTED_FORMAT);
    return APP_MEDIA_ERROR_UNSUPPORTED_FORMAT;
  }

  padded_size = (jpeg_size + 3U) & ~3UL;
  if (padded_size > APP_MEDIA_JPEG_MAX_BYTES)
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_INVALID_MEDIA);
    return APP_MEDIA_ERROR_INVALID_MEDIA;
  }
  if (padded_size > jpeg_size)
  {
    memset(&s_jpeg_buffer[jpeg_size], 0xFF, padded_size - jpeg_size);
  }

  mcu_cols = (info.width + 7U) / 8U;
  mcu_rows = (info.height + 7U) / 8U;
  output_bytes = mcu_cols * mcu_rows * 64U;
  if ((output_bytes == 0U) || (output_bytes > sizeof(s_jpeg_decode_buffer)))
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_UNSUPPORTED_FORMAT);
    return APP_MEDIA_ERROR_UNSUPPORTED_FORMAT;
  }

  if (ensure_jpeg_ready() != APP_MEDIA_ERROR_NONE)
  {
    preview_clear();
    return APP_MEDIA_ERROR_JPEG;
  }

  memset(s_jpeg_decode_buffer, 0, output_bytes);
  if (HAL_JPEG_Decode(&s_jpeg_handle,
                      s_jpeg_buffer,
                      padded_size,
                      s_jpeg_decode_buffer,
                      output_bytes,
                      APP_MEDIA_JPEG_TIMEOUT_MS) != HAL_OK)
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_JPEG);
    return APP_MEDIA_ERROR_JPEG;
  }

  if ((s_jpeg_handle.Conf.ColorSpace != JPEG_GRAYSCALE_COLORSPACE) ||
      (s_jpeg_handle.Conf.ImageWidth != info.width) ||
      (s_jpeg_handle.Conf.ImageHeight != info.height))
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_UNSUPPORTED_FORMAT);
    return APP_MEDIA_ERROR_UNSUPPORTED_FORMAT;
  }

  memset(s_preview_buffer, 0, sizeof(s_preview_buffer));
  dst_x0 = (APP_MEDIA_PREVIEW_WIDTH - info.width) / 2U;
  dst_y0 = (APP_MEDIA_PREVIEW_HEIGHT - info.height) / 2U;
  src = s_jpeg_decode_buffer;
  for (uint32_t mcu_y = 0U; mcu_y < mcu_rows; ++mcu_y)
  {
    for (uint32_t mcu_x = 0U; mcu_x < mcu_cols; ++mcu_x)
    {
      for (uint32_t y = 0U; y < 8U; ++y)
      {
        const uint32_t dst_y = (mcu_y * 8U) + y;
        if (dst_y >= info.height)
        {
          continue;
        }

        for (uint32_t x = 0U; x < 8U; ++x)
        {
          const uint32_t dst_x = (mcu_x * 8U) + x;
          if (dst_x < info.width)
          {
            s_preview_buffer[((dst_y0 + dst_y) * APP_MEDIA_PREVIEW_WIDTH) + dst_x0 + dst_x] =
              gray_to_rgb565(src[(y * 8U) + x]);
          }
        }
      }
      src += 64U;
    }
  }

  preview_commit(APP_MEDIA_SELECTED_VIDEO, frame_index, frame_count);
  return APP_MEDIA_ERROR_NONE;
}

static uint32_t load_bmp_preview(FX_FILE *file, uint32_t *bytes_read)
{
  uint8_t header[54];
  uint32_t data_offset;
  uint32_t dib_size;
  uint32_t width;
  uint32_t height;
  uint32_t bpp;
  uint32_t compression;
  uint32_t row_stride;
  uint32_t draw_width;
  uint32_t draw_height;
  uint32_t y_offset;
  UINT status;

  status = fx_file_seek(file, 0U);
  if (status == FX_SUCCESS)
  {
    status = file_read_exact(file, header, sizeof(header), bytes_read);
  }
  if (status != FX_SUCCESS)
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_FILE_READ);
    return APP_MEDIA_ERROR_FILE_READ;
  }

  data_offset = get_u32_le(&header[10]);
  dib_size = get_u32_le(&header[14]);
  width = get_u32_le(&header[18]);
  height = get_u32_le(&header[22]);
  bpp = get_u16_le(&header[28]);
  compression = get_u32_le(&header[30]);
  if ((header[0] != 'B') || (header[1] != 'M') ||
      (dib_size < 40U) || (width == 0U) || (height == 0U) ||
      ((height & 0x80000000UL) != 0U) ||
      (bpp != 24U) || (compression != 0U) ||
      (width > APP_MEDIA_FB_WIDTH) ||
      ((width * 3U) > sizeof(s_bmp_row)))
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_UNSUPPORTED_FORMAT);
    return APP_MEDIA_ERROR_UNSUPPORTED_FORMAT;
  }

  row_stride = (width * 3U + 3U) & ~3UL;
  if (row_stride > sizeof(s_bmp_row))
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_UNSUPPORTED_FORMAT);
    return APP_MEDIA_ERROR_UNSUPPORTED_FORMAT;
  }

  draw_width = APP_MEDIA_PREVIEW_WIDTH;
  draw_height = (height * draw_width) / width;
  if (draw_height > APP_MEDIA_PREVIEW_HEIGHT)
  {
    draw_height = APP_MEDIA_PREVIEW_HEIGHT;
    draw_width = (width * draw_height) / height;
  }
  if ((draw_width == 0U) || (draw_height == 0U))
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_INVALID_MEDIA);
    return APP_MEDIA_ERROR_INVALID_MEDIA;
  }

  memset(s_preview_buffer, 0, sizeof(s_preview_buffer));
  y_offset = (APP_MEDIA_PREVIEW_HEIGHT - draw_height) / 2U;
  for (uint32_t dst_y = 0U; dst_y < draw_height; ++dst_y)
  {
    const uint32_t src_y_top = (dst_y * height) / draw_height;
    const uint32_t file_y = (height - 1U) - src_y_top;
    const uint32_t x_offset = (APP_MEDIA_PREVIEW_WIDTH - draw_width) / 2U;
    uint16_t *dst = &s_preview_buffer[((y_offset + dst_y) * APP_MEDIA_PREVIEW_WIDTH) + x_offset];

    status = fx_file_seek(file, (ULONG64)data_offset + ((ULONG64)file_y * (ULONG64)row_stride));
    if (status == FX_SUCCESS)
    {
      status = file_read_exact(file, s_bmp_row, row_stride, bytes_read);
    }
    if (status != FX_SUCCESS)
    {
      preview_clear();
      status_set_error(APP_MEDIA_ERROR_FILE_READ);
      return APP_MEDIA_ERROR_FILE_READ;
    }

    for (uint32_t dst_x = 0U; dst_x < draw_width; ++dst_x)
    {
      const uint32_t src_x = (dst_x * width) / draw_width;
      const uint8_t *pixel = &s_bmp_row[src_x * 3U];
      const uint8_t b = pixel[0];
      const uint8_t g = pixel[1];
      const uint8_t r = pixel[2];
      dst[dst_x] = (uint16_t)(((uint16_t)(r & 0xF8U) << 8) |
                              ((uint16_t)(g & 0xFCU) << 3) |
                              ((uint16_t)b >> 3));
    }
  }

  preview_commit(APP_MEDIA_SELECTED_SCREENSHOT, 0U, 1U);
  return APP_MEDIA_ERROR_NONE;
}

static uint32_t avi_read_chunk_header(FX_FILE *file,
                                      ULONG64 offset,
                                      uint8_t tag[4],
                                      uint32_t *chunk_size,
                                      uint32_t *bytes_read)
{
  uint8_t header[8];
  UINT status = fx_file_seek(file, offset);

  if (status == FX_SUCCESS)
  {
    status = file_read_exact(file, header, sizeof(header), bytes_read);
  }
  if (status != FX_SUCCESS)
  {
    return APP_MEDIA_ERROR_FILE_READ;
  }

  memcpy(tag, header, 4U);
  *chunk_size = get_u32_le(&header[4]);
  return APP_MEDIA_ERROR_NONE;
}

static uint32_t avi_find_regions(FX_FILE *file,
                                 ULONG64 file_size,
                                 ULONG64 *movi_data_start,
                                 ULONG64 *movi_end,
                                 ULONG64 *idx1_start,
                                 uint32_t *idx1_size,
                                 uint32_t *bytes_read)
{
  uint8_t header[12];
  ULONG64 offset = 12U;
  UINT status;

  *movi_data_start = 0U;
  *movi_end = 0U;
  *idx1_start = 0U;
  *idx1_size = 0U;

  status = fx_file_seek(file, 0U);
  if (status == FX_SUCCESS)
  {
    status = file_read_exact(file, header, sizeof(header), bytes_read);
  }
  if (status != FX_SUCCESS)
  {
    return APP_MEDIA_ERROR_FILE_READ;
  }
  if ((memcmp(&header[0], "RIFF", 4U) != 0) || (memcmp(&header[8], "AVI ", 4U) != 0))
  {
    return APP_MEDIA_ERROR_INVALID_MEDIA;
  }

  while ((offset + 8U) <= file_size)
  {
    uint8_t tag[4];
    uint32_t chunk_size;
    ULONG64 payload_start;
    ULONG64 payload_end;
    ULONG64 next_offset;
    uint32_t error = avi_read_chunk_header(file, offset, tag, &chunk_size, bytes_read);

    if (error != APP_MEDIA_ERROR_NONE)
    {
      return error;
    }

    payload_start = offset + 8U;
    payload_end = payload_start + chunk_size;
    next_offset = payload_end + (chunk_size & 1U);
    if ((payload_end > file_size) || (next_offset < offset))
    {
      return APP_MEDIA_ERROR_INVALID_MEDIA;
    }

    if ((memcmp(tag, "LIST", 4U) == 0) && (chunk_size >= 4U))
    {
      uint8_t list_type[4];
      status = fx_file_seek(file, payload_start);
      if (status == FX_SUCCESS)
      {
        status = file_read_exact(file, list_type, sizeof(list_type), bytes_read);
      }
      if (status != FX_SUCCESS)
      {
        return APP_MEDIA_ERROR_FILE_READ;
      }

      if (memcmp(list_type, "movi", 4U) == 0)
      {
        *movi_data_start = payload_start + 4U;
        *movi_end = payload_end;
      }
    }
    else if (memcmp(tag, "idx1", 4U) == 0)
    {
      *idx1_start = payload_start;
      *idx1_size = chunk_size;
    }

    offset = next_offset;
  }

  return (*movi_data_start != 0U) ? APP_MEDIA_ERROR_NONE : APP_MEDIA_ERROR_INVALID_MEDIA;
}

static uint32_t avi_build_index_from_idx1(FX_FILE *file,
                                          ULONG64 file_size,
                                          ULONG64 movi_data_start,
                                          ULONG64 idx1_start,
                                          uint32_t idx1_size,
                                          uint32_t *frame_count,
                                          uint32_t *bytes_read)
{
  const uint32_t entries = idx1_size / 16U;
  uint8_t entry[16];
  uint32_t frames = 0U;

  for (uint32_t i = 0U; (i < entries) && (frames < APP_MEDIA_MAX_VIDEO_FRAMES); ++i)
  {
    UINT status = fx_file_seek(file, idx1_start + ((ULONG64)i * 16ULL));
    if (status == FX_SUCCESS)
    {
      status = file_read_exact(file, entry, sizeof(entry), bytes_read);
    }
    if (status != FX_SUCCESS)
    {
      return APP_MEDIA_ERROR_FILE_READ;
    }

    if ((memcmp(entry, "00dc", 4U) == 0) || (memcmp(entry, "00db", 4U) == 0))
    {
      const uint32_t offset = get_u32_le(&entry[8]);
      const uint32_t size = get_u32_le(&entry[12]);
      const ULONG64 chunk_offset = movi_data_start + offset;

      if ((size != 0U) && (size <= APP_MEDIA_JPEG_MAX_BYTES) &&
          ((chunk_offset + 8ULL + (ULONG64)size) <= file_size) &&
          (chunk_offset <= 0xFFFFFFFFULL))
      {
        s_avi_index[frames].offset = (uint32_t)chunk_offset;
        s_avi_index[frames].size = size;
        frames++;
      }
    }
  }

  *frame_count = frames;
  return (frames != 0U) ? APP_MEDIA_ERROR_NONE : APP_MEDIA_ERROR_INVALID_MEDIA;
}

static uint32_t avi_build_index_from_movi(FX_FILE *file,
                                          ULONG64 movi_data_start,
                                          ULONG64 movi_end,
                                          uint32_t *frame_count,
                                          uint32_t *bytes_read)
{
  ULONG64 offset = movi_data_start;
  uint32_t frames = 0U;

  while (((offset + 8U) <= movi_end) && (frames < APP_MEDIA_MAX_VIDEO_FRAMES))
  {
    uint8_t tag[4];
    uint32_t chunk_size;
    uint32_t error = avi_read_chunk_header(file, offset, tag, &chunk_size, bytes_read);

    if (error != APP_MEDIA_ERROR_NONE)
    {
      return error;
    }
    if ((offset + 8ULL + (ULONG64)chunk_size) > movi_end)
    {
      return APP_MEDIA_ERROR_INVALID_MEDIA;
    }

    if (((memcmp(tag, "00dc", 4U) == 0) || (memcmp(tag, "00db", 4U) == 0)) &&
        (chunk_size != 0U) && (chunk_size <= APP_MEDIA_JPEG_MAX_BYTES) &&
        (offset <= 0xFFFFFFFFULL))
    {
      s_avi_index[frames].offset = (uint32_t)offset;
      s_avi_index[frames].size = chunk_size;
      frames++;
    }

    offset += 8ULL + chunk_size + (chunk_size & 1U);
  }

  *frame_count = frames;
  return (frames != 0U) ? APP_MEDIA_ERROR_NONE : APP_MEDIA_ERROR_INVALID_MEDIA;
}

static uint32_t load_avi_preview(FX_FILE *file, const char *path, uint32_t *bytes_read)
{
  ULONG64 file_size = file->fx_file_current_file_size;
  ULONG64 movi_data_start;
  ULONG64 movi_end;
  ULONG64 idx1_start;
  uint32_t idx1_size;
  uint32_t frame_count = 0U;
  uint32_t frame_index;
  uint8_t tag[4];
  uint32_t chunk_size;
  UINT status;
  uint32_t error;

  if (file_size < 20U)
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_INVALID_MEDIA);
    return APP_MEDIA_ERROR_INVALID_MEDIA;
  }

  if ((s_video_play_frame_count == 0U) || (strncmp(s_video_play_path, path, APP_MEDIA_FILE_NAME_LEN) != 0))
  {
    error = avi_find_regions(file,
                             file_size,
                             &movi_data_start,
                             &movi_end,
                             &idx1_start,
                             &idx1_size,
                             bytes_read);
    if (error == APP_MEDIA_ERROR_NONE)
    {
      if ((idx1_start != 0U) && (idx1_size >= 16U))
      {
        error = avi_build_index_from_idx1(file,
                                          file_size,
                                          movi_data_start,
                                          idx1_start,
                                          idx1_size,
                                          &frame_count,
                                          bytes_read);
      }
      else
      {
        error = avi_build_index_from_movi(file, movi_data_start, movi_end, &frame_count, bytes_read);
      }
    }
    if (error != APP_MEDIA_ERROR_NONE)
    {
      preview_clear();
      status_set_error(error);
      return error;
    }

    s_video_play_frame = 0U;
    s_video_play_frame_count = frame_count;
    status_set_file(s_video_play_path, path);
  }

  frame_count = s_video_play_frame_count;
  frame_index = (frame_count == 0U) ? 0U : (s_video_play_frame % frame_count);
  error = avi_read_chunk_header(file,
                                (ULONG64)s_avi_index[frame_index].offset,
                                tag,
                                &chunk_size,
                                bytes_read);
  if (error != APP_MEDIA_ERROR_NONE)
  {
    preview_clear();
    status_set_error(error);
    return error;
  }
  if ((memcmp(tag, "00dc", 4U) != 0) && (memcmp(tag, "00db", 4U) != 0))
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_INVALID_MEDIA);
    return APP_MEDIA_ERROR_INVALID_MEDIA;
  }

  chunk_size = media_min_u32(chunk_size, s_avi_index[frame_index].size);
  if ((chunk_size == 0U) || (chunk_size > APP_MEDIA_JPEG_MAX_BYTES))
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_INVALID_MEDIA);
    return APP_MEDIA_ERROR_INVALID_MEDIA;
  }

  status = fx_file_seek(file, (ULONG64)s_avi_index[frame_index].offset + 8ULL);
  if (status == FX_SUCCESS)
  {
    status = file_read_exact(file, s_jpeg_buffer, chunk_size, bytes_read);
  }
  if (status != FX_SUCCESS)
  {
    preview_clear();
    status_set_error(APP_MEDIA_ERROR_FILE_READ);
    return APP_MEDIA_ERROR_FILE_READ;
  }

  error = decode_jpeg_preview(chunk_size, frame_index, frame_count);
  if (error == APP_MEDIA_ERROR_NONE)
  {
    s_video_play_frame = (frame_index + 1U) % frame_count;
  }
  return error;
}

static uint32_t read_selected_file(void)
{
  char path[APP_MEDIA_FILE_NAME_LEN];
  UINT status;
  uint32_t bytes = 0U;
  uint32_t selected_type;
  uint32_t error;

  status_lock();
  status_set_file(path, s_status.selected_file);
  selected_type = s_status.selected_type;
  status_unlock();

  if (path[0] == '\0')
  {
    status_set_error(APP_MEDIA_ERROR_NO_SELECTION);
    return APP_MEDIA_ERROR_NO_SELECTION;
  }

  if (s_recording != 0U)
  {
    status_set_error(APP_MEDIA_ERROR_ALREADY_RECORDING);
    return APP_MEDIA_ERROR_ALREADY_RECORDING;
  }

  status = fx_file_open(&s_media, &s_work_file, (CHAR *)path, FX_OPEN_FOR_READ);
  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_FILE_OPEN);
    return APP_MEDIA_ERROR_FILE_OPEN;
  }

  if (selected_type == APP_MEDIA_SELECTED_SCREENSHOT)
  {
    error = load_bmp_preview(&s_work_file, &bytes);
  }
  else if (selected_type == APP_MEDIA_SELECTED_VIDEO)
  {
    error = load_avi_preview(&s_work_file, path, &bytes);
  }
  else
  {
    error = APP_MEDIA_ERROR_NO_SELECTION;
    status_set_error(error);
  }
  (void)fx_file_close(&s_work_file);

  status_lock();
  s_status.last_read_bytes = bytes;
  if (error == APP_MEDIA_ERROR_NONE)
  {
    s_status.last_error = APP_MEDIA_ERROR_NONE;
  }
  status_unlock();

  return error;
}

static void process_command(AppMediaCommand_t command)
{
  status_set_busy(1U);

  if (s_media_mounted == 0U)
  {
    if (mount_or_format_media() != FX_SUCCESS)
    {
      status_set_busy(0U);
      return;
    }
  }

  switch (command)
  {
  case APP_MEDIA_CMD_REFRESH:
    scan_media_files();
    update_space_status();
    status_set_error(APP_MEDIA_ERROR_NONE);
    break;
  case APP_MEDIA_CMD_SCREENSHOT:
    (void)write_bmp_screenshot();
    break;
  case APP_MEDIA_CMD_RECORD_START:
    (void)start_recording();
    break;
  case APP_MEDIA_CMD_RECORD_STOP:
    (void)stop_recording();
    break;
  case APP_MEDIA_CMD_SELECT_NEXT:
    (void)select_next_file();
    break;
  case APP_MEDIA_CMD_READ_SELECTED:
    (void)read_selected_file();
    break;
  default:
    break;
  }

  status_set_busy(0U);
}

static void AppMedia_ThreadEntry(ULONG thread_input)
{
  ULONG message = 0U;

  (void)thread_input;
  status_set_busy(1U);
  (void)mount_or_format_media();
  status_set_busy(0U);

  while (1)
  {
    if (tx_queue_receive(&s_media_queue, &message, APP_MEDIA_RECORD_PERIOD_TICKS) == TX_SUCCESS)
    {
      process_command((AppMediaCommand_t)message);
    }

    process_record_tick();
  }
}

static void AppMedia_FileXDriver(FX_MEDIA *media_ptr)
{
  UINT status = FX_SUCCESS;
  uint32_t sector;
  uint32_t sectors;
  uint8_t *buffer;
  const uint32_t media_blocks = app_media_visible_blocks();

  switch (media_ptr->fx_media_driver_request)
  {
  case FX_DRIVER_INIT:
    if ((sd_nand_init() != SD_NAND_OK) || (media_blocks == 0U))
    {
      status = FX_IO_ERROR;
    }
    media_ptr->fx_media_driver_write_protect = FX_FALSE;
    media_ptr->fx_media_driver_free_sector_update = FX_FALSE;
    break;

  case FX_DRIVER_UNINIT:
  case FX_DRIVER_FLUSH:
  case FX_DRIVER_ABORT:
  case FX_DRIVER_RELEASE_SECTORS:
    status = FX_SUCCESS;
    break;

  case FX_DRIVER_BOOT_READ:
    sector = 0U;
    sectors = 1U;
    buffer = media_ptr->fx_media_driver_buffer;
    if (media_blocks == 0U)
    {
      status = FX_IO_ERROR;
    }
    else if (sd_nand_read_disk(buffer, sector, sectors) != SD_NAND_OK)
    {
      status = FX_IO_ERROR;
    }
    break;

  case FX_DRIVER_READ:
    sector = (uint32_t)media_ptr->fx_media_driver_logical_sector;
    sectors = media_ptr->fx_media_driver_sectors;
    if (sectors == 0U)
    {
      sectors = 1U;
    }
    buffer = media_ptr->fx_media_driver_buffer;
    if (((uint64_t)sector + (uint64_t)sectors) > (uint64_t)media_blocks)
    {
      status = FX_IO_ERROR;
    }
    else if (sd_nand_read_disk(buffer, sector, sectors) != SD_NAND_OK)
    {
      status = FX_IO_ERROR;
    }
    break;

  case FX_DRIVER_BOOT_WRITE:
    sector = 0U;
    sectors = 1U;
    buffer = media_ptr->fx_media_driver_buffer;
    if (media_blocks == 0U)
    {
      status = FX_IO_ERROR;
    }
    else if (sd_nand_write_disk(buffer, sector, sectors) != SD_NAND_OK)
    {
      status = FX_IO_ERROR;
    }
    else if (sd_nand_read_disk(s_boot_sector, sector, sectors) != SD_NAND_OK)
    {
      status = FX_IO_ERROR;
    }
    else if (memcmp(s_boot_sector, buffer, SD_NAND_BLOCK_SIZE) != 0)
    {
      status = FX_IO_ERROR;
    }
    break;

  case FX_DRIVER_WRITE:
    sector = (uint32_t)media_ptr->fx_media_driver_logical_sector;
    sectors = media_ptr->fx_media_driver_sectors;
    if (sectors == 0U)
    {
      sectors = 1U;
    }
    buffer = media_ptr->fx_media_driver_buffer;
    if (((uint64_t)sector + (uint64_t)sectors) > (uint64_t)media_blocks)
    {
      status = FX_IO_ERROR;
    }
    else if (sd_nand_write_disk(buffer, sector, sectors) != SD_NAND_OK)
    {
      status = FX_IO_ERROR;
    }
    break;

  default:
    status = FX_IO_ERROR;
    break;
  }

  media_ptr->fx_media_driver_status = status;
}

UINT AppMedia_Init(VOID *memory_ptr)
{
  UINT ret;

  (void)memory_ptr;
  memset(&s_status, 0, sizeof(s_status));
  memset(s_preview_buffer, 0, sizeof(s_preview_buffer));
  s_video_play_path[0] = '\0';
  s_video_play_frame = 0U;
  s_video_play_frame_count = 0U;
  s_jpeg_ready = 0U;

  ret = tx_mutex_create(&s_media_status_mutex, (CHAR *)"media_status", TX_NO_INHERIT);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }
  s_status_mutex_ready = 1U;

  ret = tx_queue_create(&s_media_queue,
                        (CHAR *)"media_cmd",
                        TX_1_ULONG,
                        s_media_queue_storage,
                        sizeof(s_media_queue_storage));
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  if (s_filex_ready == 0U)
  {
    fx_system_initialize();
    s_filex_ready = 1U;
  }

  ret = tx_thread_create(&s_media_thread,
                         (CHAR *)"app_media",
                         AppMedia_ThreadEntry,
                         0U,
                         s_media_thread_stack,
                         APP_MEDIA_THREAD_STACK_SIZE,
                         APP_MEDIA_THREAD_PRIORITY,
                         APP_MEDIA_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);

  return ret;
}

uint32_t AppMedia_RequestScreenshot(void)
{
  return post_command(APP_MEDIA_CMD_SCREENSHOT);
}

uint32_t AppMedia_RequestRecordStart(void)
{
  return post_command(APP_MEDIA_CMD_RECORD_START);
}

uint32_t AppMedia_RequestRecordStop(void)
{
  return post_command(APP_MEDIA_CMD_RECORD_STOP);
}

uint32_t AppMedia_RequestRefresh(void)
{
  return post_command(APP_MEDIA_CMD_REFRESH);
}

uint32_t AppMedia_RequestSelectNext(void)
{
  return post_command(APP_MEDIA_CMD_SELECT_NEXT);
}

uint32_t AppMedia_RequestReadSelected(void)
{
  return post_command(APP_MEDIA_CMD_READ_SELECTED);
}

void AppMedia_GetStatus(AppMediaStatus_t *status)
{
  if (status == NULL)
  {
    return;
  }

  status_lock();
  *status = s_status;
  status_unlock();
}

const uint16_t *AppMedia_GetPreviewBuffer(AppMediaPreviewInfo_t *info)
{
  if (info != NULL)
  {
    status_lock();
    info->generation = s_status.preview_generation;
    info->valid = ((s_status.flags & APP_MEDIA_FLAG_PREVIEW_VALID) != 0U) ? 1U : 0U;
    info->type = s_status.preview_type;
    info->width = s_status.preview_width;
    info->height = s_status.preview_height;
    info->frame_index = s_status.preview_frame_index;
    info->frame_count = s_status.preview_frame_count;
    status_unlock();
  }

  return s_preview_buffer;
}
