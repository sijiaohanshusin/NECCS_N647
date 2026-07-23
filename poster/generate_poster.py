"""Generate the two-sided A4 competition poster for the N647 project."""

from __future__ import annotations

import argparse
import math
import os
import re
from pathlib import Path
from typing import Iterable, Sequence

import fitz
from reportlab.lib import colors
from reportlab.lib.colors import Color, HexColor
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfgen import canvas

try:
    from poster.content import (
        APPLICATIONS,
        BREAKTHROUGHS,
        CORE_METRICS,
        FOOTNOTE,
        INTRO,
        PIPELINE_STAGES,
        PROJECT_KICKER,
        PROJECT_TITLE,
        SYSTEM_LAYERS,
        TAGLINE,
        VALIDATION_STAMP,
        Breakthrough,
        Metric,
    )
except ModuleNotFoundError:  # Allows: py poster/generate_poster.py
    from content import (  # type: ignore[no-redef]
        APPLICATIONS,
        BREAKTHROUGHS,
        CORE_METRICS,
        FOOTNOTE,
        INTRO,
        PIPELINE_STAGES,
        PROJECT_KICKER,
        PROJECT_TITLE,
        SYSTEM_LAYERS,
        TAGLINE,
        VALIDATION_STAMP,
        Breakthrough,
        Metric,
    )


REPO_ROOT = Path(__file__).absolute().parents[1]
DEFAULT_OUTPUT = (
    REPO_ROOT / "output" / "pdf" / "neccs_n647_acoustic_imager_a4_duplex.pdf"
)
DEFAULT_PREVIEW_DIR = REPO_ROOT / "output" / "pdf" / "previews"

PAGE_W, PAGE_H = A4
MARGIN = 12 * mm
CONTENT_W = PAGE_W - (2 * MARGIN)

FONT_REGULAR = "N647Sans"
FONT_BOLD = "N647SansBold"
FONT_MONO = "Courier"

BG = HexColor("#07111B")
SURFACE = HexColor("#0C1B29")
SURFACE_RAISED = HexColor("#102434")
SURFACE_DEEP = HexColor("#081722")
BLUE = HexColor("#3D7EFF")
BLUE_DIM = HexColor("#1C3A6E")
CYAN = HexColor("#31D9CE")
GREEN = HexColor("#2EC27E")
AMBER = HexColor("#F5B84A")
RED = HexColor("#E5484D")
TEXT = HexColor("#F2F5F9")
MUTED = HexColor("#8CA0B4")
MUTED_DARK = HexColor("#597084")
LINE = HexColor("#1A3A53")
LINE_SOFT = HexColor("#122C3F")


def _font_candidates() -> tuple[list[Path], list[Path]]:
    regular_override = os.environ.get("N647_POSTER_FONT_REGULAR")
    bold_override = os.environ.get("N647_POSTER_FONT_BOLD")
    regular = [
        Path(regular_override) if regular_override else Path(),
        Path(r"C:\Windows\Fonts\Deng.ttf"),
        Path(r"C:\Windows\Fonts\msyh.ttc"),
        Path("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"),
        Path("/System/Library/Fonts/PingFang.ttc"),
    ]
    bold = [
        Path(bold_override) if bold_override else Path(),
        Path(r"C:\Windows\Fonts\Dengb.ttf"),
        Path(r"C:\Windows\Fonts\msyhbd.ttc"),
        Path("/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc"),
        Path("/System/Library/Fonts/PingFang.ttc"),
    ]
    return regular, bold


def _first_existing(paths: Iterable[Path]) -> Path:
    for path in paths:
        if str(path) and path.is_file():
            return path
    raise FileNotFoundError(
        "No Chinese font found. Set N647_POSTER_FONT_REGULAR and "
        "N647_POSTER_FONT_BOLD to embeddable TTF/TTC files."
    )


def register_fonts() -> tuple[Path, Path]:
    """Register and return the embedded regular/bold Chinese font files."""

    regular_candidates, bold_candidates = _font_candidates()
    regular = _first_existing(regular_candidates)
    bold = _first_existing(bold_candidates)

    registered = set(pdfmetrics.getRegisteredFontNames())
    if FONT_REGULAR not in registered:
        pdfmetrics.registerFont(TTFont(FONT_REGULAR, str(regular)))
    if FONT_BOLD not in registered:
        pdfmetrics.registerFont(TTFont(FONT_BOLD, str(bold)))
    return regular, bold


def _tokenize(text: str) -> list[str]:
    return re.findall(
        r"\n|[A-Za-z0-9][A-Za-z0-9_.:/+@%×μ–—-]* ?|[ \t]+|.",
        text,
        flags=re.UNICODE,
    )


def wrap_text(text: str, font_name: str, font_size: float, max_width: float) -> list[str]:
    """Wrap mixed Chinese/Latin copy without splitting Latin technical tokens."""

    lines: list[str] = []
    current = ""
    for token in _tokenize(text):
        if token == "\n":
            lines.append(current.rstrip())
            current = ""
            continue
        candidate = current + token
        if current and pdfmetrics.stringWidth(candidate, font_name, font_size) > max_width:
            lines.append(current.rstrip())
            current = token.lstrip()
        else:
            current = candidate
    if current or not lines:
        lines.append(current.rstrip())
    return lines


def draw_wrapped_text(
    c: canvas.Canvas,
    text: str,
    x: float,
    y: float,
    max_width: float,
    *,
    font_name: str = FONT_REGULAR,
    font_size: float = 9,
    leading: float | None = None,
    color: Color = TEXT,
    max_lines: int | None = None,
) -> float:
    leading = leading or font_size * 1.45
    lines = wrap_text(text, font_name, font_size, max_width)
    if max_lines is not None and len(lines) > max_lines:
        lines = lines[:max_lines]
        ellipsis = "…"
        while (
            lines[-1]
            and pdfmetrics.stringWidth(
                lines[-1] + ellipsis, font_name, font_size
            )
            > max_width
        ):
            lines[-1] = lines[-1][:-1]
        lines[-1] += ellipsis

    c.setFont(font_name, font_size)
    c.setFillColor(color)
    cursor = y
    for line in lines:
        c.drawString(x, cursor, line)
        cursor -= leading
    return cursor


def draw_card(
    c: canvas.Canvas,
    x: float,
    y: float,
    width: float,
    height: float,
    *,
    fill: Color = SURFACE,
    stroke: Color = LINE,
    radius: float = 8,
    line_width: float = 0.7,
) -> None:
    c.setFillColor(fill)
    c.setStrokeColor(stroke)
    c.setLineWidth(line_width)
    c.roundRect(x, y, width, height, radius, fill=1, stroke=1)


def draw_section_label(
    c: canvas.Canvas,
    x: float,
    y: float,
    label: str,
    title: str,
    *,
    width: float,
) -> None:
    c.setFillColor(CYAN)
    c.roundRect(x, y - 2, 4, 14, 2, fill=1, stroke=0)
    c.setFont(FONT_MONO, 7)
    c.setFillColor(CYAN)
    c.drawString(x + 12, y + 4, label.upper())
    c.setFont(FONT_BOLD, 13)
    c.setFillColor(TEXT)
    c.drawString(x + 72, y, title)
    c.setStrokeColor(LINE)
    c.setLineWidth(0.6)
    c.line(x + 72, y - 7, x + width, y - 7)


def draw_footer(c: canvas.Canvas, page_number: int, descriptor: str) -> None:
    y = 8.5 * mm
    c.setStrokeColor(LINE_SOFT)
    c.setLineWidth(0.6)
    c.line(MARGIN, y + 8, PAGE_W - MARGIN, y + 8)
    c.setFillColor(MUTED_DARK)
    c.setFont(FONT_REGULAR, 6.7)
    c.drawString(MARGIN, y - 1, descriptor)
    c.setFont(FONT_MONO, 6.8)
    c.drawRightString(
        PAGE_W - MARGIN,
        y - 1,
        f"NECCS / N647 / {page_number:02d}",
    )


def draw_header_signature(c: canvas.Canvas) -> None:
    """Draw the recurring 32-microphone wavefront signature."""

    cx = PAGE_W - MARGIN - 35
    cy = PAGE_H - MARGIN - 35
    c.saveState()
    c.setLineWidth(0.55)
    for radius, color in (
        (18, LINE),
        (29, BLUE_DIM),
        (41, LINE),
        (53, LINE_SOFT),
    ):
        c.setStrokeColor(color)
        c.circle(cx, cy, radius, fill=0, stroke=1)
    c.setFillColor(BLUE)
    for index in range(32):
        angle = (2 * math.pi * index / 32) - (math.pi / 2)
        radius = 29 if index % 2 else 41
        mx = cx + math.cos(angle) * radius
        my = cy + math.sin(angle) * radius
        c.circle(mx, my, 1.35, fill=1, stroke=0)
    c.setStrokeColor(CYAN)
    c.setLineWidth(1)
    c.line(cx - 8, cy, cx + 8, cy)
    c.line(cx, cy - 8, cx, cy + 8)
    c.restoreState()


def draw_image_contain(
    c: canvas.Canvas,
    path: Path,
    x: float,
    y: float,
    width: float,
    height: float,
    *,
    background: Color = SURFACE_DEEP,
    border: Color = LINE,
) -> None:
    if not path.is_file():
        raise FileNotFoundError(f"Poster asset not found: {path}")
    from PIL import Image

    with Image.open(path) as image:
        image_w, image_h = image.size
    scale = min(width / image_w, height / image_h)
    draw_w = image_w * scale
    draw_h = image_h * scale
    draw_x = x + ((width - draw_w) / 2)
    draw_y = y + ((height - draw_h) / 2)

    c.setFillColor(background)
    c.setStrokeColor(border)
    c.setLineWidth(0.7)
    c.roundRect(x, y, width, height, 7, fill=1, stroke=1)
    c.saveState()
    clip = c.beginPath()
    clip.roundRect(x + 1, y + 1, width - 2, height - 2, 6)
    c.clipPath(clip, fill=0, stroke=0)
    c.drawImage(
        str(path),
        draw_x,
        draw_y,
        draw_w,
        draw_h,
        preserveAspectRatio=True,
        mask="auto",
    )
    c.restoreState()


def draw_acoustic_field(
    c: canvas.Canvas,
    x: float,
    y: float,
    width: float,
    height: float,
) -> None:
    draw_card(c, x, y, width, height, fill=SURFACE_DEEP)
    pad = 10
    gx, gy = x + pad, y + pad
    gw, gh = width - (2 * pad), height - (2 * pad)

    c.saveState()
    c.setStrokeColor(LINE_SOFT)
    c.setLineWidth(0.35)
    for col in range(1, 7):
        xx = gx + (gw * col / 7)
        c.line(xx, gy, xx, gy + gh)
    for row in range(1, 8):
        yy = gy + (gh * row / 9)
        c.line(gx, yy, gx + gw, yy)

    source_x = gx + (gw * 0.68)
    source_y = gy + (gh * 0.61)
    rings = (
        (44, BLUE_DIM),
        (34, HexColor("#235D8B")),
        (25, HexColor("#278FA6")),
        (17, CYAN),
        (10, AMBER),
    )
    for radius, color in rings:
        c.setFillColor(color)
        c.circle(source_x, source_y, radius, fill=1, stroke=0)

    array_x = gx + (gw * 0.29)
    array_y = gy + (gh * 0.35)
    c.setStrokeColor(MUTED_DARK)
    c.setLineWidth(0.45)
    for radius in (18, 30, 41):
        c.circle(array_x, array_y, radius, fill=0, stroke=1)
    for index in range(32):
        angle = (2 * math.pi * index / 32) - (math.pi / 2)
        radius = 30 if index % 2 else 41
        mx = array_x + math.cos(angle) * radius
        my = array_y + math.sin(angle) * radius
        c.setFillColor(CYAN if index % 4 == 0 else MUTED)
        c.circle(mx, my, 1.25, fill=1, stroke=0)

    c.setStrokeColor(TEXT)
    c.setLineWidth(0.8)
    c.circle(source_x, source_y, 7, fill=0, stroke=1)
    c.line(source_x - 12, source_y, source_x + 12, source_y)
    c.line(source_x, source_y - 12, source_x, source_y + 12)
    c.restoreState()

    c.setFillColor(TEXT)
    c.setFont(FONT_BOLD, 10)
    c.drawString(x + 12, y + height - 20, "声场 → 视场")
    c.setFont(FONT_REGULAR, 6.8)
    c.setFillColor(MUTED)
    c.drawString(x + 12, y + 10, "32 麦阵与热力图融合示意")
    c.setFillColor(CYAN)
    c.setFont(FONT_MONO, 6)
    c.drawRightString(x + width - 10, y + 10, "ON-DEVICE")


def draw_metric_card(
    c: canvas.Canvas,
    metric: Metric,
    x: float,
    y: float,
    width: float,
    height: float,
    accent: Color,
) -> None:
    draw_card(c, x, y, width, height, fill=SURFACE)
    c.setFillColor(accent)
    c.roundRect(x, y, 3.3, height, 1.6, fill=1, stroke=0)
    c.setFont(FONT_BOLD, 15.5)
    c.setFillColor(TEXT)
    c.drawString(x + 12, y + height - 24, metric.value)
    c.setFont(FONT_REGULAR, 7.7)
    c.setFillColor(accent)
    c.drawString(x + 12, y + height - 39, metric.label)
    draw_wrapped_text(
        c,
        metric.condition,
        x + 12,
        y + height - 53,
        width - 22,
        font_size=6.7,
        leading=8.3,
        color=MUTED,
        max_lines=2,
    )


def draw_pipeline(c: canvas.Canvas, x: float, y: float, width: float, height: float) -> None:
    count = len(PIPELINE_STAGES)
    gap = 8
    stage_w = (width - (gap * (count - 1))) / count
    accent_colors: Sequence[Color] = (CYAN, BLUE, AMBER, GREEN, CYAN)
    for index, (stage, accent) in enumerate(zip(PIPELINE_STAGES, accent_colors)):
        sx = x + (index * (stage_w + gap))
        draw_card(c, sx, y, stage_w, height, fill=SURFACE_DEEP)
        c.setFillColor(accent)
        c.roundRect(sx + 9, y + height - 18, 23, 10, 5, fill=1, stroke=0)
        c.setFillColor(BG)
        c.setFont(FONT_BOLD, 5.8)
        c.drawCentredString(sx + 20.5, y + height - 15.2, stage.label)
        draw_wrapped_text(
            c,
            stage.headline,
            sx + 9,
            y + height - 35,
            stage_w - 18,
            font_name=FONT_BOLD,
            font_size=7.4,
            leading=9.2,
            color=TEXT,
            max_lines=2,
        )
        draw_wrapped_text(
            c,
            stage.detail,
            sx + 9,
            y + 15,
            stage_w - 18,
            font_size=5.9,
            leading=7.2,
            color=MUTED,
            max_lines=2,
        )
        if index < count - 1:
            ax = sx + stage_w + 1.5
            ay = y + (height / 2)
            c.setStrokeColor(BLUE)
            c.setFillColor(BLUE)
            c.setLineWidth(0.8)
            c.line(ax, ay, ax + 5, ay)
            path = c.beginPath()
            path.moveTo(ax + 5, ay + 2.5)
            path.lineTo(ax + 8, ay)
            path.lineTo(ax + 5, ay - 2.5)
            path.close()
            c.drawPath(path, fill=1, stroke=0)


def draw_front_page(c: canvas.Canvas) -> None:
    c.setFillColor(BG)
    c.rect(0, 0, PAGE_W, PAGE_H, fill=1, stroke=0)
    draw_header_signature(c)

    top = PAGE_H - MARGIN
    c.setFillColor(CYAN)
    c.setFont(FONT_MONO, 7.2)
    c.drawString(MARGIN, top - 2, "NECCS · ACOUSTIC VISION / 01")
    c.setFillColor(MUTED)
    c.setFont(FONT_REGULAR, 6.8)
    c.drawString(MARGIN + 158, top - 2, PROJECT_KICKER)

    c.setFont(FONT_BOLD, 24)
    c.setFillColor(TEXT)
    c.drawString(MARGIN, top - 38, PROJECT_TITLE)
    c.setFillColor(BLUE)
    c.roundRect(MARGIN, top - 54, 52, 3, 1.5, fill=1, stroke=0)

    c.setFont(FONT_BOLD, 12)
    c.setFillColor(CYAN)
    c.drawString(MARGIN, top - 76, TAGLINE)
    draw_wrapped_text(
        c,
        INTRO,
        MARGIN,
        top - 97,
        CONTENT_W - 55,
        font_size=8.1,
        leading=11.2,
        color=MUTED,
        max_lines=2,
    )

    hero_y = 430
    hero_h = 223
    image_w = 356
    field_x = MARGIN + image_w + 10
    field_w = CONTENT_W - image_w - 10
    draw_image_contain(
        c,
        REPO_ROOT / "docs" / "ui_reference" / "ref_image.png",
        MARGIN,
        hero_y,
        image_w,
        hero_h,
    )
    c.setFillColor(TEXT)
    c.setFont(FONT_BOLD, 8)
    c.drawString(MARGIN + 10, hero_y + 11, "实时成像界面")
    c.setFont(FONT_MONO, 5.8)
    c.setFillColor(CYAN)
    c.drawRightString(MARGIN + image_w - 10, hero_y + 11, "TOUCHGFX / LIVE")
    draw_acoustic_field(c, field_x, hero_y, field_w, hero_h)

    pipeline_y = 317
    draw_section_label(
        c,
        MARGIN,
        pipeline_y + 93,
        "SYSTEM LOOP",
        "完全端侧的五段闭环",
        width=CONTENT_W,
    )
    draw_pipeline(c, MARGIN, pipeline_y, CONTENT_W, 75)

    metrics_y = 95
    draw_section_label(
        c,
        MARGIN,
        metrics_y + 195,
        "MEASURED",
        "评委一眼可核验的硬指标",
        width=CONTENT_W,
    )
    gap = 8
    card_w = (CONTENT_W - (2 * gap)) / 3
    card_h = 76
    colors_by_card: Sequence[Color] = (CYAN, BLUE, GREEN, AMBER, CYAN, BLUE)
    for index, (metric, accent) in enumerate(zip(CORE_METRICS, colors_by_card)):
        col = index % 3
        row = index // 3
        x = MARGIN + (col * (card_w + gap))
        y = metrics_y + ((1 - row) * (card_h + 8))
        draw_metric_card(c, metric, x, y, card_w, card_h, accent)

    draw_footer(c, 1, VALIDATION_STAMP)
    c.showPage()


def draw_bullet_list(
    c: canvas.Canvas,
    bullets: Sequence[str],
    x: float,
    y: float,
    width: float,
    *,
    font_size: float = 6.7,
    leading: float = 8.4,
    gap: float = 3.1,
) -> float:
    cursor = y
    for bullet in bullets:
        c.setFillColor(CYAN)
        c.circle(x + 2.2, cursor + 2.2, 1.35, fill=1, stroke=0)
        lines = wrap_text(bullet, FONT_REGULAR, font_size, width - 12)
        c.setFont(FONT_REGULAR, font_size)
        c.setFillColor(MUTED)
        for line in lines:
            c.drawString(x + 9, cursor, line)
            cursor -= leading
        cursor -= gap
    return cursor


def draw_breakthrough_card(
    c: canvas.Canvas,
    item: Breakthrough,
    x: float,
    y: float,
    width: float,
    height: float,
    accent: Color,
) -> None:
    draw_card(c, x, y, width, height, fill=SURFACE)
    c.setFillColor(accent)
    c.roundRect(x, y + height - 4, width, 4, 2, fill=1, stroke=0)
    c.setFillColor(BLUE_DIM)
    c.setFont(FONT_BOLD, 24)
    c.drawRightString(x + width - 12, y + height - 29, item.index)
    c.setFont(FONT_BOLD, 11.6)
    c.setFillColor(TEXT)
    c.drawString(x + 13, y + height - 26, item.title)
    draw_wrapped_text(
        c,
        item.thesis,
        x + 13,
        y + height - 45,
        width - 26,
        font_size=7.2,
        leading=9,
        color=accent,
        max_lines=2,
    )
    draw_bullet_list(
        c,
        item.details,
        x + 13,
        y + height - 72,
        width - 26,
        font_size=6.2,
        leading=7.5,
        gap=2.0,
    )
    c.setFillColor(SURFACE_RAISED)
    c.roundRect(x + 10, y + 10, width - 20, 21, 5, fill=1, stroke=0)
    c.setFont(FONT_BOLD, 6.5)
    c.setFillColor(accent)
    c.drawString(x + 18, y + 18, item.evidence)


def draw_system_stack(
    c: canvas.Canvas,
    x: float,
    y: float,
    width: float,
    height: float,
) -> None:
    draw_card(c, x, y, width, height, fill=SURFACE_DEEP)
    inner_x = x + 12
    inner_w = width - 24
    row_gap = 5
    row_h = (height - 30 - (row_gap * (len(SYSTEM_LAYERS) - 1))) / len(
        SYSTEM_LAYERS
    )
    palette: Sequence[Color] = (CYAN, BLUE, AMBER, GREEN, MUTED)
    for index, ((label, detail), accent) in enumerate(zip(SYSTEM_LAYERS, palette)):
        row_y = y + height - 14 - ((index + 1) * row_h) - (index * row_gap)
        c.setFillColor(SURFACE_RAISED if index % 2 == 0 else SURFACE)
        c.roundRect(inner_x, row_y, inner_w, row_h, 4, fill=1, stroke=0)
        c.setFillColor(accent)
        c.roundRect(inner_x + 7, row_y + 5, 35, row_h - 10, 4, fill=1, stroke=0)
        c.setFillColor(BG)
        c.setFont(FONT_BOLD, 6.5)
        c.drawCentredString(inner_x + 24.5, row_y + (row_h / 2) - 2.2, label)
        c.setFont(FONT_REGULAR, 6.5)
        c.setFillColor(TEXT)
        c.drawString(inner_x + 50, row_y + (row_h / 2) - 2.2, detail)


def draw_evidence_thumbnail(
    c: canvas.Canvas,
    image: Path,
    x: float,
    y: float,
    width: float,
    height: float,
    label: str,
) -> None:
    draw_image_contain(c, image, x, y, width, height)
    c.setFillColor(BG)
    c.roundRect(x + 6, y + 6, 70, 13, 4, fill=1, stroke=0)
    c.setFont(FONT_BOLD, 5.9)
    c.setFillColor(CYAN)
    c.drawString(x + 11, y + 10, label)


def draw_application_row(
    c: canvas.Canvas,
    x: float,
    y: float,
    width: float,
    height: float,
) -> None:
    gap = 7
    card_w = (width - (gap * 3)) / 4
    accents: Sequence[Color] = (CYAN, AMBER, RED, BLUE)
    for index, ((title, detail), accent) in enumerate(zip(APPLICATIONS, accents)):
        card_x = x + (index * (card_w + gap))
        draw_card(c, card_x, y, card_w, height, fill=SURFACE_DEEP)
        c.setFillColor(accent)
        c.circle(card_x + 14, y + height - 15, 3.2, fill=1, stroke=0)
        c.setFont(FONT_BOLD, 8)
        c.setFillColor(TEXT)
        c.drawString(card_x + 23, y + height - 18, title)
        draw_wrapped_text(
            c,
            detail,
            card_x + 10,
            y + height - 39,
            card_w - 20,
            font_size=6.3,
            leading=8,
            color=MUTED,
            max_lines=2,
        )


def draw_back_page(c: canvas.Canvas) -> None:
    c.setFillColor(BG)
    c.rect(0, 0, PAGE_W, PAGE_H, fill=1, stroke=0)
    draw_header_signature(c)

    top = PAGE_H - MARGIN
    c.setFillColor(CYAN)
    c.setFont(FONT_MONO, 7.2)
    c.drawString(MARGIN, top - 2, "ENGINEERING EVIDENCE / 02")
    c.setFillColor(MUTED)
    c.setFont(FONT_REGULAR, 6.8)
    c.drawString(MARGIN + 160, top - 2, "从阵列同步、实时 DSP 到 NPU 的完整实现")
    c.setFont(FONT_BOLD, 23)
    c.setFillColor(TEXT)
    c.drawString(MARGIN, top - 38, "四项关键技术突破")
    c.setFont(FONT_REGULAR, 8)
    c.setFillColor(MUTED)
    c.drawString(
        MARGIN,
        top - 59,
        "不是功能堆叠，而是围绕“定位更快、频带更准、端侧更完整”的系统性突破。",
    )

    grid_top = 737
    gap_x = 10
    gap_y = 10
    card_w = (CONTENT_W - gap_x) / 2
    card_h = 145
    accents: Sequence[Color] = (CYAN, BLUE, AMBER, GREEN)
    for index, (item, accent) in enumerate(zip(BREAKTHROUGHS, accents)):
        col = index % 2
        row = index // 2
        x = MARGIN + (col * (card_w + gap_x))
        y = grid_top - ((row + 1) * card_h) - (row * gap_y)
        draw_breakthrough_card(c, item, x, y, card_w, card_h, accent)

    evidence_y = 177
    evidence_h = 223
    stack_w = 324
    thumb_gap = 9
    thumb_x = MARGIN + stack_w + 11
    thumb_w = CONTENT_W - stack_w - 11
    draw_section_label(
        c,
        MARGIN,
        evidence_y + evidence_h + 23,
        "ARCHITECTURE",
        "从硬件到应用的端侧软件栈",
        width=CONTENT_W,
    )
    draw_system_stack(c, MARGIN, evidence_y, stack_w, evidence_h)
    thumb_h = (evidence_h - thumb_gap) / 2
    draw_evidence_thumbnail(
        c,
        REPO_ROOT / "docs" / "ui_reference" / "ref_mics.png",
        thumb_x,
        evidence_y + thumb_h + thumb_gap,
        thumb_w,
        thumb_h,
        "32 路麦阵诊断",
    )
    draw_evidence_thumbnail(
        c,
        REPO_ROOT / "docs" / "ui_reference" / "ref_perf.png",
        thumb_x,
        evidence_y,
        thumb_w,
        thumb_h,
        "板端性能剖析",
    )

    app_y = 91
    draw_section_label(
        c,
        MARGIN,
        app_y + 65,
        "SCENARIOS",
        "面向真实巡检与取证",
        width=CONTENT_W,
    )
    draw_application_row(c, MARGIN, app_y, CONTENT_W, 52)

    draw_wrapped_text(
        c,
        FOOTNOTE,
        MARGIN,
        67,
        CONTENT_W,
        font_size=6.4,
        leading=8,
        color=MUTED_DARK,
        max_lines=2,
    )
    draw_footer(c, 2, "数据来源：PROJECT_STATUS / acoustic_refactor / core16_192k")
    c.showPage()


def render_previews(
    pdf_path: Path,
    preview_dir: Path,
    *,
    dpi: int = 150,
) -> list[Path]:
    preview_dir.mkdir(parents=True, exist_ok=True)
    document = fitz.open(pdf_path)
    matrix = fitz.Matrix(dpi / 72, dpi / 72)
    outputs: list[Path] = []
    for page_index, page in enumerate(document, start=1):
        pixmap = page.get_pixmap(matrix=matrix, alpha=False)
        output = preview_dir / f"neccs_n647_poster_page_{page_index}.png"
        pixmap.save(output)
        outputs.append(output)
    document.close()
    return outputs


def build_poster(
    output_path: str | Path = DEFAULT_OUTPUT,
    *,
    preview_dir: str | Path | None = None,
    preview_dpi: int = 150,
) -> list[Path]:
    """Build the two-page A4 PDF and optionally render page previews."""

    register_fonts()
    output = Path(output_path)
    output.parent.mkdir(parents=True, exist_ok=True)
    c = canvas.Canvas(
        str(output),
        pagesize=A4,
        pageCompression=1,
    )
    c.setTitle(PROJECT_TITLE)
    c.setAuthor("NECCS")
    c.setSubject("STM32N647 智能声学成像仪 A4 双面竞赛海报")
    c.setCreator("poster/generate_poster.py")
    draw_front_page(c)
    draw_back_page(c)
    c.save()

    if preview_dir is None:
        return []
    return render_previews(output, Path(preview_dir), dpi=preview_dpi)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT,
        help="Two-page A4 PDF output path.",
    )
    parser.add_argument(
        "--preview-dir",
        type=Path,
        default=DEFAULT_PREVIEW_DIR,
        help="Directory for rendered page PNG previews.",
    )
    parser.add_argument(
        "--preview-dpi",
        type=int,
        default=150,
        help="Preview rasterization DPI (default: 150).",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    previews = build_poster(
        args.output,
        preview_dir=args.preview_dir,
        preview_dpi=args.preview_dpi,
    )
    print(f"PDF: {args.output}")
    for preview in previews:
        print(f"Preview: {preview}")


if __name__ == "__main__":
    main()
