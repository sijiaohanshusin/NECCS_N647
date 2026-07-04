# NECCS Acoustic Imaging Route

This note captures the current implementation route while PCMD/SAI/DMA hardware
capture is still under investigation.

## Firmware Boundary

The active firmware boundary is:

- `app_mic_array.*`: passive slot-to-logical-microphone mapping.
- `app_audio_frame.*`: hardware-neutral planar audio frame contract.
- `app_acoustic_imaging.*`: imaging profile/configuration, microphone pair
  selection, coarse-grid TDOA LUT generation, and visualization frame contract.
- `app_acoustic_srp.*`: F32 CMSIS-DSP SRP-PHAT runtime and DWT cycle reporting.
- `app_acoustic_synthetic.*`: deterministic planar F32 source for no-hardware
  validation.

None of these modules starts SAI, DMA, I2C, GPIO, PCMD3180 initialization, or
PCMD debug tasks.

## First Runtime Route

The first real-time imaging route is `Wide32 @ 48 kHz`:

- `NFFT=256`
- active bins `3..42`
- coarse grid `9 x 9`, `-60..60 deg`
- fine-search contract `top3 x 4 x 4`, span `10 deg`
- default profile `BALANCED`, `160` selected microphone pairs

Profile pair counts:

| Mode | FAST | BALANCED | QUALITY |
| --- | ---: | ---: | ---: |
| Wide32 @ 48 kHz | 96 | 160 | 240 |
| Core16 @ 192 kHz | 80 | 120 | 120 |

The full 496-pair Wide32 set is an offline-only comparison target.

`app_acoustic_srp.*` v1 implements the Wide32 FAST/BALANCED F32 path. QUALITY
and Core16 remain reserved entry points until the first N6 timing data is
available. Large runtime buffers are placed in `.EXTRAM`; CubeIDE links only the
needed CMSIS-DSP F32 source files from the STM32Cube repository.

## Offline Tools

Tools live under `tools/acoustic_imaging/`.

Generate default Wide32 assets:

```powershell
python .\tools\acoustic_imaging\generate_srp_assets.py --mode wide32 --profile balanced
```

Run synthetic sanity checks:

```powershell
python .\tools\acoustic_imaging\srp_sanity_check.py --mode wide32 --profile balanced
python -m unittest discover .\tools\acoustic_imaging
```

Generated assets are written to `docs/knowledge/microphone-array/generated/`.

## Hardware Gate

Real capture should not feed the SRP pipeline until the following checks pass:

- silent RMS and clip counters are stable,
- single-mic tap order matches `array_32ch_coords.csv`,
- polarity and coarse inter-channel phase are consistent,
- no DMA overrun, SAI error, queue overflow, or ThreadX stack overflow occurs
  during a long run.

`Core16 @ 192 kHz` remains a phase-2 high-frequency/near-field research mode
after Wide32 is stable.

## No-MIC NPU Bring-Up Route

The first STM32N6 NPU step does not depend on PCMD3180, SAI, or DMA capture.
It uses the existing synthetic/offline SRP path as the input source and limits
the NPU scope to the Wide32/BALANCED coarse heatmap projection:

- CPU keeps frame generation, windowing, FFT, GCC-PHAT, fine search, and UI
  publishing.
- NPU receives quantized GCC-PHAT features with shape
  `1 x (160 pairs x 40 bins x 2)` and returns an `int8` 9x9 coarse heatmap.
- Firmware exposes this through `APP_ACOUSTIC_BACKEND_NPU_HEATMAP`; it is
  disabled by default and returns `UNSUPPORTED_MODE` until generated ST Edge AI
  code is linked.

Generate the dependency-free model spec:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\acoustic_imaging\generate_npu_heatmap_model.ps1 -SpecOnly
```

After installing the optional Python `onnx` package in the chosen local Python
environment, generate the ONNX model and run ST Edge AI:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\acoustic_imaging\generate_npu_heatmap_model.ps1
```

The NECCS memory-pool descriptor reserves the NPU RAM banks and starts the AI
HyperRAM pool at `0x90800000` so the first LCD framebuffer region at
`0x90000000` remains untouched. xSPI2 is deliberately disabled in this pool
until the board-specific external Flash address map is re-verified for NPU
constant access.
