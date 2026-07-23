from __future__ import annotations

from pathlib import Path

import fitz
from pypdf import PdfReader
from reportlab.lib.pagesizes import A4

from poster.content import (
    BREAKTHROUGHS,
    CORE_METRICS,
    PROJECT_TITLE,
    TAGLINE,
)


REPO_ROOT = Path(__file__).resolve().parents[2]


def test_poster_copy_is_evidence_backed() -> None:
    assert PROJECT_TITLE == "基于 STM32N647 的智能声学成像仪"
    assert "听见" in TAGLINE
    assert "记录" in TAGLINE
    assert len(CORE_METRICS) >= 6
    assert len(BREAKTHROUGHS) == 4

    for metric in CORE_METRICS:
        source = REPO_ROOT / metric.source
        assert source.is_file(), f"missing evidence source: {source}"
        assert metric.value
        assert metric.label
        assert metric.condition

    combined_copy = " ".join(
        [PROJECT_TITLE, TAGLINE]
        + [
            f"{metric.value} {metric.label} {metric.condition}"
            for metric in CORE_METRICS
        ]
    )
    assert "通用精度±2°" not in combined_copy
    assert "板测有效" in combined_copy


def test_default_output_keeps_the_visible_repository_path() -> None:
    from poster import generate_poster

    visible_root = Path(generate_poster.__file__).absolute().parents[1]
    assert generate_poster.REPO_ROOT == visible_root
    assert generate_poster.DEFAULT_OUTPUT.is_relative_to(visible_root)


def test_builds_two_page_a4_pdf_with_key_copy(tmp_path: Path) -> None:
    from poster.generate_poster import build_poster

    output = tmp_path / "poster.pdf"
    previews = tmp_path / "previews"

    generated_previews = build_poster(output, preview_dir=previews, preview_dpi=150)

    assert output.is_file()
    assert output.stat().st_size > 100_000
    assert len(generated_previews) == 2
    assert all(path.is_file() for path in generated_previews)

    reader = PdfReader(str(output))
    assert len(reader.pages) == 2
    expected_width, expected_height = A4
    for page in reader.pages:
        assert abs(float(page.mediabox.width) - expected_width) < 0.1
        assert abs(float(page.mediabox.height) - expected_height) < 0.1

    extracted = "\n".join(page.extract_text() or "" for page in reader.pages)
    for phrase in ("STM32N647", "SRP-PHAT", "Neural-ART", "381", "192 kHz"):
        assert phrase in extracted


def test_pages_render_at_300_dpi_without_blank_output(tmp_path: Path) -> None:
    from poster.generate_poster import build_poster

    output = tmp_path / "poster.pdf"
    build_poster(output)

    document = fitz.open(output)
    assert document.page_count == 2
    scale = 300 / 72
    expected_size = (2481, 3508)

    for page in document:
        pixmap = page.get_pixmap(matrix=fitz.Matrix(scale, scale), alpha=False)
        assert abs(pixmap.width - expected_size[0]) <= 2
        assert abs(pixmap.height - expected_size[1]) <= 2
        samples = memoryview(pixmap.samples)
        assert min(samples) < 230
        assert max(samples) > 245
