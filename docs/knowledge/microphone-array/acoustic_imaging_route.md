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
- default firmware profile `Wide32-General-SRP`, `160` selected microphone pairs
- temperature-corrected sound speed, channel health masks, pair/bin weighting,
  confidence gates, and temporal heatmap smoothing

Implemented profile family:

| Algorithm | Firmware profile | Pair selection | Intended use |
| --- | --- | ---: | --- |
| `Wide32-Fast-SRP` | `FAST` | 96 | UI/camera pressure or thermal fallback |
| `Wide32-General-SRP` | `BALANCED` | 160 | default product route |
| `Wide32-Quality-SRP` | `QUALITY` | 240 | enable only after N6 timing proves budget |
| `Wide32-HF-Hint` | `FAST` | 96 short-baseline | high-frequency leakage hint, not main DOA accuracy |
| `Core16-HF-Nearfield` | reserved | 120 | phase-2, 192 kHz near-field route |

The full 496-pair Wide32 set is an offline-only comparison target.

`app_acoustic_srp.*` v1 implements the Wide32 FAST/BALANCED/QUALITY and
HF-Hint F32 path. Core16 remains a reserved entry point until the 192 kHz clock
and capture chain are verified. Large runtime buffers are placed in `.EXTRAM`;
hot scalar state stays in the context. The runtime uses no dynamic allocation.

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

Run the profile robustness matrix:

```powershell
python .\tools\acoustic_imaging\evaluate_srp_profiles.py --random-trials 8
```

The evaluator reuses `array_32ch_coords.csv` and covers single source,
multi-source, near-field distance hints, SNR sweeps, gain/delay errors,
reflections, bad channels, slot swaps, and polarity flips. It reports angular
error percentiles, confidence, miss counts, relative work units, and the
recommended first firmware profile.

Generated assets are written to `docs/knowledge/microphone-array/generated/`.

## No-MIC Firmware Benchmark

Before PCMD/SAI/DMA capture is ready, firmware can exercise the real SRP path
with deterministic synthetic frames through:

- `App_AcousticSrp_RunSelfTest(...)`: validates FAST/BALANCED/QUALITY profiles.
- `App_AcousticSrp_RunSyntheticBenchmark(...)`: returns requested/processed
  frames, average and maximum DWT cycles for preprocess, FFT, GCC, coarse,
  fine, output, total, effective FPS Q8, final visualization frame, and status.

The benchmark path is intended to be called from a board-local diagnostic task
or command hook once the parallel PCMD/CubeIDE baseline is stable. It does not
start or require microphones.

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
