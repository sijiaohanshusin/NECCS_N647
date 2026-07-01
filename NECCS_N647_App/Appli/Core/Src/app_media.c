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

#define APP_MEDIA_FB_WIDTH                1024U
#define APP_MEDIA_FB_HEIGHT               600U
#define APP_MEDIA_FB_BYTES                (APP_MEDIA_FB_WIDTH * APP_MEDIA_FB_HEIGHT * 2U)
#define APP_MEDIA_FB_BASE                 0x90000000UL

#define APP_MEDIA_RECORD_WIDTH            512U
#define APP_MEDIA_RECORD_HEIGHT           304U
#define APP_MEDIA_RECORD_FPS              5U
#define APP_MEDIA_RECORD_PERIOD_TICKS     (TX_TIMER_TICKS_PER_SECOND / APP_MEDIA_RECORD_FPS)
#define APP_MEDIA_JPEG_MAX_BYTES          (384U * 1024U)
#define APP_MEDIA_MAX_VIDEO_FRAMES        1800U
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
static uint8_t s_bmp_row[APP_MEDIA_FB_WIDTH * 3U] __attribute__((aligned(32)));
static uint8_t s_jpeg_buffer[APP_MEDIA_JPEG_MAX_BYTES] __attribute__((aligned(32)));
static AppMediaAviIndex_t s_avi_index[APP_MEDIA_MAX_VIDEO_FRAMES];

static AppMediaStatus_t s_status;
static uint32_t s_media_mounted = 0U;
static uint32_t s_recording = 0U;
static uint32_t s_next_screenshot = 1U;
static uint32_t s_next_video = 1U;
static uint32_t s_record_file_pos = 0U;
static uint32_t s_record_started_tick = 0U;
static uint32_t s_record_last_frame_tick = 0U;
static uint32_t s_avi_riff_size_offset = 0U;
static uint32_t s_avi_avih_frames_offset = 0U;
static uint32_t s_avi_strh_frames_offset = 0U;
static uint32_t s_avi_movi_size_offset = 0U;
static uint32_t s_avi_movi_data_start = 0U;

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

static void put_u32_le(uint8_t *buffer, uint32_t value)
{
  buffer[0] = (uint8_t)value;
  buffer[1] = (uint8_t)(value >> 8);
  buffer[2] = (uint8_t)(value >> 16);
  buffer[3] = (uint8_t)(value >> 24);
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
    s_status.selected_index = s_status.videos - 1U;
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

  status = fx_media_open(&s_media,
                         (CHAR *)"NECCS_SD",
                         AppMedia_FileXDriver,
                         FX_NULL,
                         s_filex_cache,
                         sizeof(s_filex_cache));
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
                             8U,
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

#if defined(LTDC_Layer1)
  if (LTDC_Layer1->CFBAR != 0U)
  {
    address = LTDC_Layer1->CFBAR;
  }
#endif

  return (const uint16_t *)address;
}

static void clean_framebuffer_cache(const uint16_t *framebuffer)
{
  SCB_CleanDCache_by_Addr((void *)framebuffer, (int32_t)APP_MEDIA_FB_BYTES);
  __DSB();
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
    for (int32_t row = (int32_t)APP_MEDIA_FB_HEIGHT - 1; row >= 0; --row)
    {
      uint8_t *dst = s_bmp_row;
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

      status = fx_file_write(&s_work_file, s_bmp_row, row_bytes);
      if (status != FX_SUCCESS)
      {
        break;
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
  s_status.selected_index = s_next_video - 1U;
  s_status.last_error = APP_MEDIA_ERROR_NONE;
  status_unlock();

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

  return APP_MEDIA_ERROR_NONE;
}

static uint32_t read_selected_file(void)
{
  char path[APP_MEDIA_FILE_NAME_LEN];
  ULONG actual = 0U;
  uint8_t header[512];
  UINT status;
  uint32_t bytes = 0U;

  status_lock();
  status_set_file(path, s_status.selected_file);
  status_unlock();

  if (path[0] == '\0')
  {
    status_set_error(APP_MEDIA_ERROR_NO_SELECTION);
    return APP_MEDIA_ERROR_NO_SELECTION;
  }

  status = fx_file_open(&s_media, &s_work_file, (CHAR *)path, FX_OPEN_FOR_READ);
  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_FILE_OPEN);
    return APP_MEDIA_ERROR_FILE_OPEN;
  }

  status = fx_file_read(&s_work_file, header, sizeof(header), &actual);
  if (status == FX_SUCCESS)
  {
    bytes = actual;
  }
  (void)fx_file_close(&s_work_file);

  if (status != FX_SUCCESS)
  {
    status_set_error(APP_MEDIA_ERROR_FILE_READ);
    return APP_MEDIA_ERROR_FILE_READ;
  }

  status_lock();
  s_status.last_read_bytes = bytes;
  s_status.last_error = APP_MEDIA_ERROR_NONE;
  status_unlock();

  return APP_MEDIA_ERROR_NONE;
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
