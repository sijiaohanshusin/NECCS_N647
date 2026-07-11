# N647 UI 设计系统(TouchGFX 手写 UI 指南)

> 本项目 **不用 TouchGFX Designer**,全部 UI 手写在
> `Appli/TouchGFX/gui/src/template_screen/TemplateView.cpp`(单 View 多"页")。
> 这份文档的目的不是限制你,而是把"这套 UI 为什么好看"讲清楚,
> 让你的改动天然融入整体。想突破默认约定是允许的——突破前先理解
> 原则,突破后用 `n647.ps1 uitour` 截图和 `docs/ui_reference/` 基准图
> 并排对比,自己判断:**新元素看起来像同一个 App 的吗?**

## 1. 设计原则(为什么这套 UI 好看)

理解了这五条,大部分决策不用查表也能做对:

1. **每页只有一个主角。** 成像页的主角是相机画面,媒体页是缩略图网格,
   参数页是设置列表。其余一切(卡片、标签、按钮)都是配角——配角用
   更暗的颜色、更小的字号、更靠边的位置,绝不和主角抢注意力。
2. **层级靠亮度,颜色只表达含义。** 主文字亮(ColorText)、次要文字暗
   (ColorMuted),这就是全部层级手段。彩色是稀缺资源:蓝=交互/强调,
   绿=正常,琥珀=警告,红=危险/录制。彩色一多,专业感立刻消失——
   这是工业 UI 和玩具 UI 的分水岭。
3. **对齐产生秩序。** 每条边都应该和某个已有元素的边对齐;间距只用
   8/12/14px 这一个节奏。人眼说不出"哪里不对",但 3px 的错位会让整页
   显得廉价。新元素落位前,先找它对齐谁。
4. **克制的动效。** 只在信息变化处动(声呐波纹、进度条、录制红点),
   装饰性动画一个都没有。动得少,动的地方才有意义。
5. **模仿是最快的融入方式。** 加新东西时,找最像的现有元素(右轨卡片、
   快捷按钮、参数行),照抄它的结构和数值,只换内容。自由发挥当然可以,
   但要接受第 0 条检验:和基准图并排看,像不像同一个 App。

## 2. 基准参考图(docs/ui_reference/)

五页出厂状态截图,是"好看"的具象定义。任何 UI 改动后,和对应页面的
基准图并排对比自查:

| 文件 | 页面 |
|---|---|
| ref_image.png | 成像页(相机+右轨卡片+快捷栏+频谱) |
| ref_mics.png | 麦阵页 |
| ref_perf.png | 系统/性能页 |
| ref_settings.png | 参数页(步进器列表) |
| ref_media.png | 媒体页(4x2 画廊) |

改动让页面变得更好 → 更新基准图;变得说不清好坏 → 多半是变差了,回退。

## 3. 调色板(TemplateView.cpp 顶部)

| 名称 | RGB | 角色 |
|---|---|---|
| ColorBg      | 7,13,20     | 全局背景 |
| ColorBlue    | 61,126,255  | 品牌强调:选中、数值高亮、进度 |
| ColorBlueDim | 28,58,110   | 蓝的弱化:边框、次要强调 |
| ColorRed     | 229,72,77   | 危险/录制中 |
| ColorRedDim  | 96,34,40    | 红的弱化:边框 |
| ColorGreen   | 46,194,126  | 正常/在线 |
| ColorAmber   | 245,184,74  | 警告 |
| ColorText    | 242,245,249 | 主文字 |
| ColorMuted   | 140,160,180 | 次要文字/图标默认 |

需要新颜色时先自问:现有 9 个的组合真的表达不了吗?通常加一档
Dim 变体(降饱和降亮度)比引入全新色相安全得多。

## 4. 布局骨架(constexpr,TemplateView.cpp L12-22)

```text
ScreenW=1024  ScreenH=600
BarH=44            顶部状态栏(品牌+模式+SD+fps)
ContentX=92        内容区左边界(左侧 0-91 是导航列)
CamX=192 CamY=60   相机窗口 640x480(成像页主角)
RailX=844 RailW=172 右侧信息轨(卡片自上而下堆叠)
```

节奏:卡片间 8-12px、卡片内边距 14px、行高 18-26px。
右轨卡片的标准结构:AppRoundedPanel + 小标题(scale1, ColorMuted)+
内容(scale2, ColorText)+ 可选细进度条。

## 5. 组件与工具

| 组件(gui/common/AppUiWidgets) | 用途 |
|---|---|
| AppTextLabel     | 中文文本(自绘字库,setter 自带变更检测) |
| AppAsciiLabel    | 纯 ASCII 小标签 |
| AppRoundedPanel  | 圆角面板/卡片/按钮底(radius 统一 10) |
| AppRgb565Preview | 相机/媒体像素缓冲显示 |
| AppSpectrumPanel | 频谱条+拖动选带 |
| AppSonarPulse    | 声呐波纹动画 |
| touchgfx::TouchArea | 触摸热区(叠在面板上) |

文本一律经 `setupLabel(label, x, y, w, h, scale, text, color[, align])`。
现有控件不够用时可以写新控件,放 gui/common/,参考 AppRoundedPanel
的写法(尤其 invalidate 纪律)。

## 6. 工程硬约束(这些不是审美问题,是会白屏/卡顿的物理事实)

1. **中文字形**:新增中文字符必须存在于
   `assets/texts/texts.xml` 各 Typography 的 `WidgetWildcardCharacters`,
   否则**不报错、直接渲染成空白**。改完跑
   `python tools/ui/check_text_glyphs.py`,0 缺字再烧录。
   **更大的坑**:普通编译**不会**重新生成 generated/texts 下的字库
   (2026-07-11 实证:texts.xml 加了字、编译通过、板上仍渲染 `?`)。
   重新生成需要 TouchGFX Designer。没有 Designer 环境时,**只用字库里
   已有的字符**,新状态优先用颜色/已有词汇表达。
2. **invalidate 纪律**:值没变不要 invalidate(参照 AppTextLabel 的
   setter);动画只 invalidate 自己的窄矩形(参照 boot 环的
   invalidateRingBand);全屏 invalidate 只在页面切换时。违反的症状是
   帧率跳水或残影。
3. **图片资产**:L8_ARGB8888(≤256 色,先 posterize 再量化,参照
   tools/touchgfx/prepare_n647_brand_assets.py),在 application.config
   登记格式,加完看 .map 的 touchgfx_resources 增量(RAM Debug 区紧张)。

## 7. 页面结构与远程验证

单 View 多页,`snapshot.activeScreen` 决定可见性(refreshVisibility):
IMAGE / MICS / PERF / SETTINGS / MEDIA / BOOT。
新增页面三件套:setupXxxPage() + refreshXxxPage() + refreshVisibility,
并在弹出菜单加入口。

```powershell
.\tools\debug\n647.ps1 uitour        # 自动翻全部页面并逐页截图
.\tools\debug\n647.ps1 screenshot    # 当前页面截图
```

改完 UI 的自查五问(对着 uitour 截图):
1. 这页的主角还是主角吗?
2. 新元素的每条边和谁对齐?
3. 用到的彩色都在表达含义吗?
4. 和基准图并排看,像同一个 App 吗?
5. 中文都显示出来了吗(没有空白块)?
