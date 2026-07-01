/**
  ******************************************************************************
  * @file    app_media_jpeg.h
  * @brief   Small baseline JPEG encoder used by the media recorder.
  ******************************************************************************
  */

#ifndef APP_MEDIA_JPEG_H
#define APP_MEDIA_JPEG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_MEDIA_JPEG_OK          0U
#define APP_MEDIA_JPEG_ERROR       1U
#define APP_MEDIA_JPEG_NO_SPACE    2U
#define APP_MEDIA_JPEG_BAD_PARAM   3U

uint32_t AppMediaJpeg_EncodeGrayFromRgb565(const uint16_t *src,
                                           uint32_t src_width,
                                           uint32_t src_height,
                                           uint32_t out_width,
                                           uint32_t out_height,
                                           uint8_t *out,
                                           uint32_t out_capacity,
                                           uint32_t *out_size);

#ifdef __cplusplus
}
#endif

#endif
