/*
 * Neural-ART (ATON NPU) service: runs the sound-classifier network on
 * 32x64 log-spectrum windows. Bring-up stage: synthetic-input self test
 * driven over SWD, latency measured with DWT cycles.
 */

#ifndef APP_NPU_H
#define APP_NPU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_NPU_CLASS_COUNT 6U

typedef struct
{
  uint8_t initialized;
  uint8_t last_status;      /* 0 = ok */
  uint32_t inference_count;
  uint32_t last_cycles;     /* CPU cycles for the last inference */
  uint32_t last_us;         /* microseconds, derived from SystemCoreClock */
  int8_t output_q[APP_NPU_CLASS_COUNT]; /* raw int8 logits */
  uint8_t top_class;
  uint32_t in_bytes;
  uint32_t out_bytes;
  uint32_t in_addr;
  uint32_t out_addr;
} AppNpuSnapshot_t;

/* One-time runtime + network init (safe to call again; no-op when done). */
int32_t AppNpu_Init(void);

/* Run one inference on the given 32x64 int8 spectrum window (row-major,
 * 2048 bytes). Pass NULL to use a synthetic ramp test pattern. */
int32_t AppNpu_RunInference(const int8_t *spectrogram_2048);

/* Live path: push one 64-bin display spectrum frame (uint8). A 32-frame
 * rolling window is kept internally; every 8th frame triggers one NPU
 * inference (~0.4 ms in the caller's context). */
void AppNpu_FeedSpectrum(const uint8_t *spectrum_64);

void AppNpu_GetSnapshot(AppNpuSnapshot_t *snapshot);

/* Bring-up poll hook: services g_app_npu_test_request (SWD-driven). */
void AppNpu_Poll(void);

extern volatile uint32_t g_app_npu_test_request;

#ifdef __cplusplus
}
#endif

#endif /* APP_NPU_H */
