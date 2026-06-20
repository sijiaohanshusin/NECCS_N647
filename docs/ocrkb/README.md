# NECCS OCR Knowledge

This directory is stored inside the N647 firmware project so developers and AI
collaborators can find the hardware knowledge base without relying on an
external workspace path.

The OCR batch:

1. Scans PDF files under `D:\Project\NECCS`.
2. Excludes generated/build directories and this output directory.
3. Deduplicates identical PDFs by SHA-1 while preserving every source path.
4. Ingests each unique PDF into the central OCRKB.
5. Exports reviewed text, page images, manifests, and structured register data.
6. Produces a searchable index and quality report.

Useful files:

- `INDEX.md`: human-readable document index.
- `source_catalog.csv`: every source PDF and duplicate relationship.
- `unique_documents.json`: canonical source selected for each unique PDF.
- `run_state.json`: live progress and current document.
- `reports/quality_report.md`: OCR and validation warnings.
- `logs/`: per-document ingest/export logs.

The batch can be resumed safely:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File D:\Project\NECCS\N647_BaseBoard\firmware\NECCS_N647\docs\ocrkb\run_ocrkb_all.ps1
```

Generated page images, exported documents, logs, and machine-readable state are
kept locally under this directory but ignored by Git. The compact index,
quality report, runner, and usage notes remain suitable for version control.
