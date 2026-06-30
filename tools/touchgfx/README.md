# NECCS N647 TouchGFX Tooling

The project uses TouchGFX 4.26.1 for formal font, text, image, and simulator assets.

Expected local tool root:

```text
C:\ST\TouchGFX\4.26.1
```

Useful commands:

```powershell
tools\touchgfx\n647_touchgfx_env.ps1
tools\touchgfx\generate_n647_touchgfx_assets.ps1
tools\touchgfx\update_n647_touchgfx_project_links.ps1 -CheckOnly
tools\touchgfx\build_n647_touchgfx_sim.ps1
```

Workflow:

1. Edit static UI resources with TouchGFX Designer 4.26.1, or edit `assets/texts/texts.xml` only when a small text database change is deliberate.
2. Run `tools\touchgfx\generate_n647_touchgfx_assets.ps1`. This runs TouchGFX asset generation and updates CubeIDE linked generated sources.
3. Run `tools\touchgfx\build_n647_touchgfx_sim.ps1` for simulator validation.
4. Run `tools\build_n647_app.ps1 -Configuration Debug` and `tools\build_n647_app.ps1 -Configuration Release` before hardware tests.

Resource layout:

- `assets/fonts/` contains checked-in font source files, currently `NotoSansSC-VF.ttf`.
- `assets/texts/texts.xml` is the TouchGFX generator source of truth.
- `assets/texts/texts.xlsx` is a human-readable mirror for planning and review.
- `generated/` is produced by TouchGFX Generator; do not edit generated files by hand.

Do not place README or placeholder files inside `NECCS_N647_App/Appli/TouchGFX/assets/images`
or `assets/videos`; TouchGFX converters scan those folders as asset inputs.
