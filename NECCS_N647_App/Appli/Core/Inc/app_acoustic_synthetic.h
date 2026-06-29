/*
 * Deterministic synthetic acoustic frames for hardware-independent SRP tests.
 */

#ifndef APP_ACOUSTIC_SYNTHETIC_H
#define APP_ACOUSTIC_SYNTHETIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_acoustic_imaging.h"

AppAcousticImagingStatus_t App_AcousticSynthetic_FillPlaneWave(const AppAcousticImagingConfig_t *config,
                                                               float theta_deg,
                                                               float phi_deg,
                                                               float frequency_hz,
                                                               float amplitude,
                                                               float noise_amplitude,
                                                               uint32_t seq,
                                                               float *dst_planar,
                                                               uint32_t samples_per_channel,
                                                               AppAudioFrame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* APP_ACOUSTIC_SYNTHETIC_H */
