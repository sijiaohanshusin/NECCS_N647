# Microphone Array References

Recovered on 2026-06-26 from branch `codex/pcmd3180-screen-freeze`.

Files:

- `array_32ch_design.html`: main 32-channel handheld acoustic imaging array
  scheme.
- `array_32ch_coords.csv`: microphone coordinate and ownership table. The core
  16 microphones are marked with `is_core16=1`; the 192 kHz core-mode slots are
  listed in `tdm_slot_192k_core`.
- `pcmd3180_mic_mapper.html`: interactive PCMD3180 microphone/channel mapper.

Original source on branch `codex/pcmd3180-screen-freeze`:

- Top-level microphone-array main-scheme directory:
  `array_32ch_design.html` from blob `207d38eafe2840d2959cf02ec5dbbf2479a306ce`.
- Top-level microphone-array main-scheme directory:
  `array_32ch_coords.csv` from blob `45f8dadaebf944fcfd276778d4f03b35f55c5f1b`.
- `docs/tools/pcmd3180_mic_mapper.html` from blob
  `cce646adcdc6eba82b6fe17f05fdb30f2fc1fdc5`.

This directory is documentation only. Do not treat these files as the active
PCMD firmware configuration until the hardware wiring and slot ownership are
confirmed during bring-up.

Firmware-side passive mapping layer:

- `NECCS_N647_App/Appli/Core/Inc/app_mic_array.h`
- `NECCS_N647_App/Appli/Core/Src/app_mic_array.c`

The mapping layer keeps the current main-scheme Bus A/Bus B slot ownership and
can copy received TDM samples into logical microphone order. It intentionally
does not start SAI, DMA, I2C, PCMD3180 initialization, or debug tasks.
