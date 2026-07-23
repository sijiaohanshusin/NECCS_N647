# N647 A4 双面海报

本目录保存竞赛海报的可复现源文件。生成结果是两页 A4 竖版 PDF：

- 第 1 页：项目价值、端侧闭环、实际 UI 与核心指标
- 第 2 页：四项技术突破、软件栈、板端证据与应用场景

## 生成环境

推荐使用项目当前的 Windows + Python 3 环境：

```powershell
uv pip install --system reportlab pypdf pillow pymupdf pytest
```

脚本默认嵌入 Windows 的等线字体：

- `C:\Windows\Fonts\Deng.ttf`
- `C:\Windows\Fonts\Dengb.ttf`

其他系统可通过环境变量指定支持中文的 TTF/TTC 字体：

```powershell
$env:N647_POSTER_FONT_REGULAR = "D:\Fonts\NotoSansCJK-Regular.ttc"
$env:N647_POSTER_FONT_BOLD = "D:\Fonts\NotoSansCJK-Bold.ttc"
```

## 生成 PDF 与预览图

在 `Program\NECCS_N647` 仓库根目录运行：

```powershell
py -3 -m poster.generate_poster --preview-dpi 180
```

默认输出：

```text
output/pdf/neccs_n647_acoustic_imager_a4_duplex.pdf
output/pdf/previews/neccs_n647_poster_page_1.png
output/pdf/previews/neccs_n647_poster_page_2.png
```

需要 300 dpi 终检图时：

```powershell
py -3 -m poster.generate_poster `
  --preview-dir output/pdf/qa-300dpi `
  --preview-dpi 300
```

## 修改文案

性能数据、标题与技术说明集中在 `poster/content.py`。每条核心指标都带有
来源文件与测试条件；修改数字时先同步核对：

- `docs/PROJECT_STATUS.md`
- `docs/acoustic_refactor.md`
- `docs/core16_192k_design_status.md`

布局、颜色、图表与 PDF 绘制逻辑位于 `poster/generate_poster.py`。海报直接
引用 `docs/ui_reference/` 下的现有界面截图，不会修改固件或 TouchGFX 资源。

## 验证

```powershell
py -3 -m pytest poster/tests/test_poster.py -q
```

测试会检查双页 A4 尺寸、关键文案、证据来源、PDF 文本与 300 dpi 渲染。

## 双面打印

- 纸张：A4
- 方向：纵向
- 缩放：实际大小 / 100%
- 双面方式：长边翻转
- 建议先打印一份检查深色背景、细字清晰度和打印机无边距能力
