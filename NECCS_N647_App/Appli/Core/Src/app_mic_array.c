#include "app_mic_array.h"

#include <stddef.h>

static const AppMicArrayMic_t s_main_scheme_mics[APP_MIC_ARRAY_PHYSICAL_MIC_COUNT] =
{
  {  1U,  150,   10, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U1, 0U,  0U, 0U, 1U },
  {  2U,  158,   86, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U1, 1U,  1U, 1U, 1U },
  {  3U,   82,  152, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U2, 0U,  8U, 4U, 1U },
  {  4U,   -8,  157, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U2, 1U,  9U, 5U, 1U },
  {  5U, -153,   71, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U3, 0U,  0U, 0U, 1U },
  {  6U, -156,   -8, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U3, 1U,  1U, 1U, 1U },
  {  7U,  -83, -148, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U4, 0U,  8U, 4U, 1U },
  {  8U,   15, -154, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U4, 1U,  9U, 5U, 1U },
  {  9U,  255,  105, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U1, 2U,  2U, 2U, 1U },
  { 10U,  130,  275, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U1, 3U,  3U, 3U, 1U },
  { 11U, -110,  260, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U2, 2U, 10U, 6U, 1U },
  { 12U, -275,  110, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U2, 3U, 11U, 7U, 1U },
  { 13U, -250, -115, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U3, 2U,  2U, 2U, 1U },
  { 14U, -135, -280, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U3, 3U,  3U, 3U, 1U },
  { 15U,  105, -265, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U4, 2U, 10U, 6U, 1U },
  { 16U,  270, -120, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U4, 3U, 11U, 7U, 1U },
  { 17U,   50,  460, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U1, 4U,  4U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 18U,  190,  470, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U1, 5U,  5U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 19U,  360,  445, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U1, 6U,  6U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 20U,  460,  280, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U1, 7U,  7U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 21U,  470,  100, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U4, 4U, 12U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 22U,  430, -110, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U4, 5U, 13U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 23U,  310, -310, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U4, 6U, 14U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 24U,  130, -460, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U4, 7U, 15U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 25U,  -80, -470, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U3, 4U,  4U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 26U, -260, -450, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U3, 5U,  5U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 27U, -430, -330, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U3, 6U,  6U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 28U, -470, -120, APP_MIC_ARRAY_BUS_B, APP_MIC_ARRAY_CHIP_U3, 7U,  7U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 29U, -470,   90, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U2, 4U, 12U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 30U, -400,  300, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U2, 5U, 13U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 31U, -240,  460, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U2, 6U, 14U, APP_MIC_ARRAY_INVALID_SLOT, 0U },
  { 32U,  -30,  440, APP_MIC_ARRAY_BUS_A, APP_MIC_ARRAY_CHIP_U2, 7U, 15U, APP_MIC_ARRAY_INVALID_SLOT, 0U }
};

static const uint8_t s_core16_physical_index[APP_MIC_ARRAY_CORE16_MIC_COUNT] =
{
  0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U,
  8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U
};

static uint8_t App_MicArray_IsValidMode(AppMicArrayMode_t mode)
{
  return ((mode == APP_MIC_ARRAY_MODE_WIDE32_48K) ||
          (mode == APP_MIC_ARRAY_MODE_CORE16_192K)) ? 1U : 0U;
}

static uint8_t App_MicArray_GetMicSlot(AppMicArrayMode_t mode,
                                       const AppMicArrayMic_t *mic)
{
  if (mic == NULL)
  {
    return APP_MIC_ARRAY_INVALID_SLOT;
  }

  if (mode == APP_MIC_ARRAY_MODE_WIDE32_48K)
  {
    return mic->tdm_slot_48k;
  }

  if (mode == APP_MIC_ARRAY_MODE_CORE16_192K)
  {
    return mic->tdm_slot_192k_core;
  }

  return APP_MIC_ARRAY_INVALID_SLOT;
}

static const int16_t *App_MicArray_SelectBus(const int16_t *bus_a_interleaved,
                                             const int16_t *bus_b_interleaved,
                                             AppMicArrayBus_t bus)
{
  return (bus == APP_MIC_ARRAY_BUS_A) ? bus_a_interleaved : bus_b_interleaved;
}

static AppMicArrayStatus_t App_MicArray_PrepareSources(AppMicArrayMode_t mode,
                                                       AppMicArraySource_t *sources,
                                                       uint32_t source_capacity,
                                                       uint32_t *channel_count,
                                                       uint32_t *slots_per_bus)
{
  uint32_t local_channel_count = App_MicArray_GetModeMicCount(mode);
  uint32_t local_slots_per_bus = App_MicArray_GetSlotsPerBus(mode);

  if ((sources == NULL) ||
      (channel_count == NULL) ||
      (slots_per_bus == NULL) ||
      (App_MicArray_IsValidMode(mode) == 0U) ||
      (local_channel_count == 0U) ||
      (local_slots_per_bus == 0U) ||
      (source_capacity < local_channel_count))
  {
    return APP_MIC_ARRAY_INVALID_ARGUMENT;
  }

  for (uint32_t channel = 0U; channel < local_channel_count; channel++)
  {
    if (App_MicArray_GetSource(mode, channel, &sources[channel]) != APP_MIC_ARRAY_OK)
    {
      return APP_MIC_ARRAY_INVALID_ARGUMENT;
    }

    if ((sources[channel].bus >= APP_MIC_ARRAY_BUS_COUNT) ||
        (sources[channel].slot >= local_slots_per_bus))
    {
      return APP_MIC_ARRAY_INVALID_ARGUMENT;
    }
  }

  *channel_count = local_channel_count;
  *slots_per_bus = local_slots_per_bus;

  return APP_MIC_ARRAY_OK;
}

uint32_t App_MicArray_GetPhysicalMicCount(void)
{
  return APP_MIC_ARRAY_PHYSICAL_MIC_COUNT;
}

uint32_t App_MicArray_GetModeMicCount(AppMicArrayMode_t mode)
{
  switch (mode)
  {
  case APP_MIC_ARRAY_MODE_WIDE32_48K:
    return APP_MIC_ARRAY_PHYSICAL_MIC_COUNT;

  case APP_MIC_ARRAY_MODE_CORE16_192K:
    return APP_MIC_ARRAY_CORE16_MIC_COUNT;

  default:
    return 0U;
  }
}

uint32_t App_MicArray_GetModeSampleRateHz(AppMicArrayMode_t mode)
{
  switch (mode)
  {
  case APP_MIC_ARRAY_MODE_WIDE32_48K:
    return 48000U;

  case APP_MIC_ARRAY_MODE_CORE16_192K:
    return 192000U;

  default:
    return 0U;
  }
}

uint32_t App_MicArray_GetSlotsPerBus(AppMicArrayMode_t mode)
{
  switch (mode)
  {
  case APP_MIC_ARRAY_MODE_WIDE32_48K:
    return APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS;

  case APP_MIC_ARRAY_MODE_CORE16_192K:
    return APP_MIC_ARRAY_CORE16_SLOTS_PER_BUS;

  default:
    return 0U;
  }
}

uint8_t App_MicArray_ValidateMode(AppMicArrayMode_t mode)
{
  uint8_t used[APP_MIC_ARRAY_BUS_COUNT][APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS] = { 0U };
  AppMicArraySource_t sources[APP_MIC_ARRAY_PHYSICAL_MIC_COUNT];
  uint32_t channel_count;
  uint32_t slots_per_bus;

  if ((App_MicArray_PrepareSources(mode,
                                   sources,
                                   APP_MIC_ARRAY_PHYSICAL_MIC_COUNT,
                                   &channel_count,
                                   &slots_per_bus) != APP_MIC_ARRAY_OK) ||
      (slots_per_bus > APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS))
  {
    return 0U;
  }

  for (uint32_t channel = 0U; channel < channel_count; channel++)
  {
    if (used[sources[channel].bus][sources[channel].slot] != 0U)
    {
      return 0U;
    }

    used[sources[channel].bus][sources[channel].slot] = 1U;
  }

  return 1U;
}

const AppMicArrayMic_t *App_MicArray_GetPhysicalMic(uint32_t physical_index)
{
  if (physical_index >= APP_MIC_ARRAY_PHYSICAL_MIC_COUNT)
  {
    return NULL;
  }

  return &s_main_scheme_mics[physical_index];
}

const AppMicArrayMic_t *App_MicArray_FindMicById(uint8_t mic_id)
{
  for (uint32_t index = 0U; index < APP_MIC_ARRAY_PHYSICAL_MIC_COUNT; index++)
  {
    if (s_main_scheme_mics[index].mic_id == mic_id)
    {
      return &s_main_scheme_mics[index];
    }
  }

  return NULL;
}

const AppMicArrayMic_t *App_MicArray_GetModeMic(AppMicArrayMode_t mode,
                                                uint32_t logical_index)
{
  if (mode == APP_MIC_ARRAY_MODE_WIDE32_48K)
  {
    return App_MicArray_GetPhysicalMic(logical_index);
  }

  if ((mode == APP_MIC_ARRAY_MODE_CORE16_192K) &&
      (logical_index < APP_MIC_ARRAY_CORE16_MIC_COUNT))
  {
    return App_MicArray_GetPhysicalMic(s_core16_physical_index[logical_index]);
  }

  return NULL;
}

AppMicArrayStatus_t App_MicArray_GetSource(AppMicArrayMode_t mode,
                                           uint32_t logical_index,
                                           AppMicArraySource_t *source)
{
  const AppMicArrayMic_t *mic;
  uint8_t slot;

  if (source == NULL)
  {
    return APP_MIC_ARRAY_INVALID_ARGUMENT;
  }

  mic = App_MicArray_GetModeMic(mode, logical_index);
  slot = App_MicArray_GetMicSlot(mode, mic);
  if ((mic == NULL) || (slot == APP_MIC_ARRAY_INVALID_SLOT))
  {
    return APP_MIC_ARRAY_INVALID_ARGUMENT;
  }

  source->mic_id = mic->mic_id;
  source->logical_index = (uint8_t)logical_index;
  source->bus = mic->bus;
  source->slot = slot;

  return APP_MIC_ARRAY_OK;
}

AppMicArrayStatus_t App_MicArray_CopyInterleavedI16(AppMicArrayMode_t mode,
                                                    const int16_t *bus_a_interleaved,
                                                    const int16_t *bus_b_interleaved,
                                                    uint32_t frame_count,
                                                    int16_t *dst_interleaved,
                                                    uint32_t dst_channel_count)
{
  AppMicArraySource_t sources[APP_MIC_ARRAY_PHYSICAL_MIC_COUNT];
  uint32_t channel_count;
  uint32_t slots_per_bus;

  if ((bus_a_interleaved == NULL) ||
      (bus_b_interleaved == NULL) ||
      (dst_interleaved == NULL) ||
      (App_MicArray_PrepareSources(mode,
                                   sources,
                                   APP_MIC_ARRAY_PHYSICAL_MIC_COUNT,
                                   &channel_count,
                                   &slots_per_bus) != APP_MIC_ARRAY_OK) ||
      (dst_channel_count < channel_count))
  {
    return APP_MIC_ARRAY_INVALID_ARGUMENT;
  }

  for (uint32_t frame = 0U; frame < frame_count; frame++)
  {
    for (uint32_t channel = 0U; channel < channel_count; channel++)
    {
      const int16_t *src_bus;

      src_bus = App_MicArray_SelectBus(bus_a_interleaved,
                                       bus_b_interleaved,
                                       sources[channel].bus);
      dst_interleaved[(frame * dst_channel_count) + channel] =
          src_bus[(frame * slots_per_bus) + sources[channel].slot];
    }
  }

  return APP_MIC_ARRAY_OK;
}

AppMicArrayStatus_t App_MicArray_DeinterleavePlanarI16(AppMicArrayMode_t mode,
                                                       const int16_t *bus_a_interleaved,
                                                       const int16_t *bus_b_interleaved,
                                                       uint32_t frame_count,
                                                       int16_t *dst_planar,
                                                       uint32_t dst_samples_per_channel)
{
  AppMicArraySource_t sources[APP_MIC_ARRAY_PHYSICAL_MIC_COUNT];
  uint32_t channel_count;
  uint32_t slots_per_bus;

  if ((bus_a_interleaved == NULL) ||
      (bus_b_interleaved == NULL) ||
      (dst_planar == NULL) ||
      (App_MicArray_PrepareSources(mode,
                                   sources,
                                   APP_MIC_ARRAY_PHYSICAL_MIC_COUNT,
                                   &channel_count,
                                   &slots_per_bus) != APP_MIC_ARRAY_OK) ||
      (dst_samples_per_channel < frame_count))
  {
    return APP_MIC_ARRAY_INVALID_ARGUMENT;
  }

  for (uint32_t channel = 0U; channel < channel_count; channel++)
  {
    const int16_t *src_bus;

    src_bus = App_MicArray_SelectBus(bus_a_interleaved,
                                     bus_b_interleaved,
                                     sources[channel].bus);

    for (uint32_t frame = 0U; frame < frame_count; frame++)
    {
      dst_planar[(channel * dst_samples_per_channel) + frame] =
          src_bus[(frame * slots_per_bus) + sources[channel].slot];
    }
  }

  return APP_MIC_ARRAY_OK;
}

AppMicArrayStatus_t App_MicArray_DeinterleavePlanarF32(AppMicArrayMode_t mode,
                                                       const int16_t *bus_a_interleaved,
                                                       const int16_t *bus_b_interleaved,
                                                       uint32_t frame_count,
                                                       float *dst_planar,
                                                       uint32_t dst_samples_per_channel,
                                                       float scale)
{
  AppMicArraySource_t sources[APP_MIC_ARRAY_PHYSICAL_MIC_COUNT];
  uint32_t channel_count;
  uint32_t slots_per_bus;

  if ((bus_a_interleaved == NULL) ||
      (bus_b_interleaved == NULL) ||
      (dst_planar == NULL) ||
      (App_MicArray_PrepareSources(mode,
                                   sources,
                                   APP_MIC_ARRAY_PHYSICAL_MIC_COUNT,
                                   &channel_count,
                                   &slots_per_bus) != APP_MIC_ARRAY_OK) ||
      (dst_samples_per_channel < frame_count))
  {
    return APP_MIC_ARRAY_INVALID_ARGUMENT;
  }

  for (uint32_t channel = 0U; channel < channel_count; channel++)
  {
    const int16_t *src_bus;

    src_bus = App_MicArray_SelectBus(bus_a_interleaved,
                                     bus_b_interleaved,
                                     sources[channel].bus);

    for (uint32_t frame = 0U; frame < frame_count; frame++)
    {
      dst_planar[(channel * dst_samples_per_channel) + frame] =
          (float)src_bus[(frame * slots_per_bus) + sources[channel].slot] * scale;
    }
  }

  return APP_MIC_ARRAY_OK;
}
