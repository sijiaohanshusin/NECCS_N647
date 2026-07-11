# N647 UI 设计系统(TouchGFX 手写 UI 规范)

> 本项目 **不用 TouchGFX Designer**,全部 UI 手写在
> `Appli/TouchGFX/gui/src/template_screen/TemplateView.cpp`(单 View 多"页")。
> 改 UI 前通读本文;改完必须跑 `n647.ps1 uitour` 逐页截图自查。

## 1. 设计语言(一句话)

深空蓝黑工业风:近黑蓝底 + 高对比白字 + 单一品牌蓝作强调,红/琥珀只给
警报和录制,所有容器用圆角面板,信息密度高但留白规整。风格对齐外壳的
蓝/红/黑配色。**不要引入新颜色、新字号、新圆角值。**

## 2. 调色板(TemplateView.cpp 顶部,rgb() 定义,勿新增)

| 名称 | RGB | 语义(只能这么用) |
|---|---|---|
| ColorBg      | 7,13,20     | 全局背景/文字底色 |
| ColorPanel*  | (见代码)   | 面板填充 |
| ColorBlue    | 61,126,255  | 品牌强调:选中态、数值高亮、进度 |
| ColorBlueDim | 28,58,110   | 蓝的弱化:边框、次要强调、辅助文字 |
| ColorRed     | 229,72,77   | 危险/录制中/高温警报 |
| ColorRedDim  | 96,34,40    | 红的弱化:边框 |
| ColorGreen   | 46,194,126  | 正常/在线状态 |
| ColorAmber   | 245,184,74  | 警告横幅、次级警报 |
| ColorText    | 242,245,249 | 主文字 |
| ColorMuted   | 140,160,180 | 次要文字/标签/图标默认 |

规则:状态语义色(绿=好,琥珀=警告,红=坏)不可混用;强调只用蓝。

## 3. 布局网格(constexpr,TemplateView.cpp L12-22)

```text
ScreenW=1024  ScreenH=600
BarH=44            顶部状态栏(品牌+模式+SD+fps)
NavW=76            左侧导航热区宽(实际按钮画在 ContentX 左侧)
ContentX=92        内容区左边界
CamX=192 CamY=60   相机窗口(640x480,成像页)
RailX=844 RailW=172 右侧信息轨(卡片堆叠)
```

- 间距节奏:卡片间 8-12px,卡片内边距 14px,行高 18-26px。
- 右轨卡片模式:AppRoundedPanel + 标题(scale1, ColorMuted)+ 内容
  (scale2, ColorText)+ 可选进度条(2-5px 高)。新增卡片照抄现有的。
- 所有坐标是绝对像素;新元素必须对齐既有元素的 x/网格,禁止奇数偏移。

## 4. 组件清单(只用这些,不要造新轮子)

| 组件(gui/common/AppUiWidgets) | 用途 |
|---|---|
| AppTextLabel     | 中文文本(自绘字库);setColors/setText 自带变更检测 |
| AppAsciiLabel    | 纯 ASCII 小标签 |
| AppRoundedPanel  | 圆角面板/卡片/按钮底(radius 统一 10) |
| AppRgb565Preview | 相机/媒体像素缓冲显示 |
| AppSpectrumPanel | 频谱条+拖动选带 |
| AppSonarPulse    | 声呐波纹动画 |
| touchgfx::TouchArea | 触摸热区(叠在面板上) |

辅助函数:`setupLabel(label, x, y, w, h, scale, text, color[, align])` —
所有文本一律经它创建。

## 5. 必守工程规则(违者要么白屏要么卡)

1. **中文字形**:任何新中文字符必须加进
   `Appli/TouchGFX/assets/texts.xml` 三个 Typography 的
   `WidgetWildcardCharacters`,否则渲染为空白。改完跑
   `python tools/ui/check_text_glyphs.py` 验证(0 缺字才能提交)。
2. **invalidate 纪律**:任何 setter 若值未变化不得调用 invalidate()
   (参照 AppTextLabel::setColors 的写法);全屏 invalidate 只允许在
   页面切换时。动画只 invalidate 自己的窄矩形(参照 boot 环的
   invalidateRingBand)。
3. **新图片资产**:必须 L8_ARGB8888(≤256 色,先 posterize 再量化,
   参照 tools/touchgfx/prepare_n647_brand_assets.py),并在
   application.config 登记格式;加完看 .map 的 touchgfx_resources
   增量,RAM Debug 区预算紧张。
4. 每 UI 改动后:`n647.ps1 uitour` 逐页截图,肉眼核对:对齐、配色语义、
   中文完整、无元素残影(漏 invalidate 的典型症状)。

## 6. 页面结构速查

单 View 多页,`snapshot.activeScreen` 决定可见性(refreshVisibility):
IMAGE(成像+右轨+快捷栏) / MICS(麦阵) / PERF(系统/性能) /
SETTINGS(参数步进器) / MEDIA(4x2 画廊+查看器) / BOOT(开机动画)。
新增页面:setupXxxPage() + refreshXxxPage() + refreshVisibility 三处,
并在弹出菜单加入口。

## 7. 远程调试 UI(盲改必备)

```powershell
.\tools\debug\n647.ps1 uitour        # 自动翻全部页面并逐页截图
.\tools\debug\n647.ps1 screenshot    # 当前页面截图
```

uitour 原理:GDB 写 `g_app_ui_test_screen`(Model::tick 轮询)切页。
