# TouchGFX Fonts

This folder is the source of truth for TouchGFX font generation.

- `NotoSansSC-VF.ttf` is used for the first NECCS N647 UI typography set.
- Keep fonts here, not under `generated/`; `generated/fonts/` is produced by the TouchGFX asset generator.
- The first UI pass intentionally uses a Simplified Chinese subset through TouchGFX text conversion rather than a full CJK bitmap font.

Regenerate assets with:

```powershell
tools\touchgfx\generate_n647_touchgfx_assets.ps1
```
