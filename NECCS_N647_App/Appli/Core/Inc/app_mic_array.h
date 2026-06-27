/*
 * NECCS microphone array logical mapping.
 *
 * This layer is intentionally hardware-passive. It does not initialize SAI,
 * DMA, I2C, GPIO or PCMD3180 registers; it only describes the main microphone
 * array plan and copies samples from TDM bus slots into logical microphone
 * order for the later audio pipeline.
 */

#ifndef APP_MIC_ARRAY_H
#define APP_MIC_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_MIC_ARRAY_PHYSICAL_MIC_COUNT      32U
#define APP_MIC_ARRAY_CORE16_MIC_COUNT        16U
#define APP_MIC_ARRAY_BUS_COUNT               2U
#define APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS    16U
#define APP_MIC_ARRAY_CORE16_SLOTS_PER_BUS    8U
#define APP_MIC_ARRAY_INVALID_SLOT            0xFFU
#define APP_MIC_ARRAY_Q15_TO_FLOAT_SCALE      (3.0517578125e-5f)

typedef enum
{
  APP_MIC_ARRAY_OK = 0,
  APP_MIC_ARRAY_INVALID_ARGUMENT = -1
} AppMicArrayStatus_t;

typedef enum
{
  APP_MIC_ARRAY_MODE_WIDE32_48K = 0,
  APP_MIC_ARRAY_MODE_CORE16_192K = 1
} AppMicArrayMode_t;

typedef enum
{
  APP_MIC_ARRAY_BUS_A = 0,
  APP_MIC_ARRAY_BUS_B = 1
} AppMicArrayBus_t;

typedef enum
{
  APP_MIC_ARRAY_CHIP_U1 = 0,
  APP_MIC_ARRAY_CHIP_U2 = 1,
  APP_MIC_ARRAY_CHIP_U3 = 2,
  APP_MIC_ARRAY_CHIP_U4 = 3
} AppMicArrayChip_t;

typedef struct
{
  uint8_t mic_id;
  int16_t x_0p1mm;
  int16_t y_0p1mm;
  AppMicArrayBus_t bus;
  AppMicArrayChip_t chip;
  uint8_t chip_slot;
  uint8_t tdm_slot_48k;
  uint8_t tdm_slot_192k_core;
  uint8_t is_core16;
} AppMicArrayMic_t;

typedef struct
{
  uint8_t mic_id;
  uint8_t logical_index;
  AppMicArrayBus_t bus;
  uint8_t slot;
} AppMicArraySource_t;

uint32_t App_MicArray_GetPhysicalMicCount(void);

uint32_t App_MicArray_GetModeMicCount(AppMicArrayMode_t mode);

uint32_t App_MicArray_GetModeSampleRateHz(AppMicArrayMode_t mode);

uint32_t App_MicArray_GetSlotsPerBus(AppMicArrayMode_t mode);

uint8_t App_MicArray_ValidateMode(AppMicArrayMode_t mode);

const AppMicArrayMic_t *App_MicArray_GetPhysicalMic(uint32_t physical_index);

const AppMicArrayMic_t *App_MicArray_FindMicById(uint8_t mic_id);

const AppMicArrayMic_t *App_MicArray_GetModeMic(AppMicArrayMode_t mode,
                                                uint32_t logical_index);

AppMicArrayStatus_t App_MicArray_GetSource(AppMicArrayMode_t mode,
                                           uint32_t logical_index,
                                           AppMicArraySource_t *source);

AppMicArrayStatus_t App_MicArray_CopyInterleavedI16(AppMicArrayMode_t mode,
                                                    const int16_t *bus_a_interleaved,
                                                    const int16_t *bus_b_interleaved,
                                                    uint32_t frame_count,
                                                    int16_t *dst_interleaved,
                                                    uint32_t dst_channel_count);

AppMicArrayStatus_t App_MicArray_DeinterleavePlanarI16(AppMicArrayMode_t mode,
                                                       const int16_t *bus_a_interleaved,
                                                       const int16_t *bus_b_interleaved,
                                                       uint32_t frame_count,
                                                       int16_t *dst_planar,
                                                       uint32_t dst_samples_per_channel);

AppMicArrayStatus_t App_MicArray_DeinterleavePlanarF32(AppMicArrayMode_t mode,
                                                       const int16_t *bus_a_interleaved,
                                                       const int16_t *bus_b_interleaved,
                                                       uint32_t frame_count,
                                                       float *dst_planar,
                                                       uint32_t dst_samples_per_channel,
                                                       float scale);

#ifdef __cplusplus
}
#endif

#endif /* APP_MIC_ARRAY_H */
