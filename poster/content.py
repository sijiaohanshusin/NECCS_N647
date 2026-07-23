"""Evidence-backed copy for the N647 competition poster.

Keep claims and their validation conditions together.  The layout module only
decides how to present this content; it must not invent stronger claims.
"""

from __future__ import annotations

from dataclasses import dataclass


PROJECT_TITLE = "基于 STM32N647 的智能声学成像仪"
PROJECT_KICKER = "全国大学生嵌入式芯片与系统设计竞赛 · 端侧智能仪器"
TAGLINE = "让设备在端侧完成「听见 · 定位 · 看见 · 识别 · 记录」闭环"
INTRO = (
    "面向气体泄漏、机械异常与电气放电等巡检场景，系统融合 32 通道宽频麦阵、"
    "IMX219 视觉、Cortex-M55 实时定位与 Neural-ART 声纹识别，"
    "无需云端即可把不可见声源叠加为可交互热力图。"
)
VALIDATION_STAMP = "板级实测基线 · ae9fa3cf · 2026-07-21"


@dataclass(frozen=True)
class Metric:
    value: str
    label: str
    condition: str
    source: str


@dataclass(frozen=True)
class Breakthrough:
    index: str
    title: str
    thesis: str
    details: tuple[str, ...]
    evidence: str
    source: str


@dataclass(frozen=True)
class PipelineStage:
    label: str
    headline: str
    detail: str


CORE_METRICS = (
    Metric(
        value="32 CH",
        label="宽频麦克风阵列",
        condition="4×PCMD3180；31/32 路板测有效",
        source="docs/PROJECT_STATUS.md",
    ),
    Metric(
        value="48 / 192 kHz",
        label="可听声 / 超声双模式",
        condition="Wide32 与 Core16 参数页可切换",
        source="docs/core16_192k_design_status.md",
    ),
    Metric(
        value="15–20 fps",
        label="实时 SRP 定位",
        condition="Wide32 STANDARD，叠加与 markers 同开",
        source="docs/acoustic_refactor.md",
    ),
    Metric(
        value="381 μs",
        label="单次 NPU 推理",
        condition="6 类 int8 CNN，Neural-ART 纯硬件",
        source="docs/PROJECT_STATUS.md",
    ),
    Metric(
        value="99.6%",
        label="int8 数据集评估",
        condition="板上播放实测 5/5 个有声类别正确",
        source="docs/PROJECT_STATUS.md",
    ),
    Metric(
        value="≈9 dB",
        label="定向录音前后比",
        condition="4 kHz 扫角板测；独立录制零丢帧",
        source="docs/PROJECT_STATUS.md",
    ),
)


PIPELINE_STAGES = (
    PipelineStage(
        label="感知",
        headline="32 路声学 + 视觉",
        detail="MEMS 阵列 · IMX219",
    ),
    PipelineStage(
        label="采集",
        headline="同步 TDM 数据链",
        detail="4×PCMD3180 · SAI / DMA",
    ),
    PipelineStage(
        label="定位",
        headline="GCC + SRP-PHAT",
        detail="粗搜 / 精搜 · Kalman 跟踪",
    ),
    PipelineStage(
        label="识别",
        headline="Neural-ART NPU",
        detail="32×64 对数谱 · 6 类 CNN",
    ),
    PipelineStage(
        label="呈现",
        headline="热力图与证据留存",
        detail="TouchGFX · WAV / 录像 · USB",
    ),
)


BREAKTHROUGHS = (
    Breakthrough(
        index="01",
        title="双模式宽频阵列",
        thesis="一套硬件同时覆盖可听声定位与超声近场检测。",
        details=(
            "Wide32：32 通道 @ 48 kHz，面向语音、机械与通用场景",
            "Core16：内圈 16 通道 @ 192 kHz，分析带扩展至约 40 kHz",
            "PLL2 精确合成 24.576 MHz，无需新增外部晶振",
        ),
        evidence="双向切换 3–8 s；Core16 SRP 21–39 ms/帧",
        source="docs/core16_192k_design_status.md",
    ),
    Breakthrough(
        index="02",
        title="高速定位与连续跟踪",
        thesis="把低速跳变的估计器重构为高帧率定位 + 独立跟踪器。",
        details=(
            "滞后域 GCC 复用相关行，粗 9×9 搜索后对 top3 精搜",
            "互谱 Welch EMA 抑制旁瓣，坏麦掩码阻断错误通道",
            "双轴 Kalman + 关联制置信度，热斑与准心同一状态源",
        ),
        evidence="88M → 16M cycles；4.5 → 15–20 fps",
        source="docs/acoustic_refactor.md",
    ),
    Breakthrough(
        index="03",
        title="自适应频带抗噪",
        thesis="不让 PHAT 把空频点白化成强噪声，主动追踪真正有能量的频带。",
        details=(
            "逐 bin SNR 门控与迟滞，静音不误开、窄带信号不过抑",
            "自动模式持续监测 18 kHz 参考谱并移动分析窗",
            "手动模式支持拖边缘调带宽、拖中心整体平移",
        ),
        evidence="静音全黑；泄漏音频 5–8 kHz 频带板测锁定",
        source="docs/PROJECT_STATUS.md",
    ),
    Breakthrough(
        index="04",
        title="端侧异构智能",
        thesis="Cortex-M55 负责确定性 DSP，Neural-ART 并行完成声源类型识别。",
        details=(
            "32×64 对数谱窗口输入 6 类 int8 CNN",
            "6 个计算 epoch 全部映射至 NPU，分类结果直达 UI 卡片",
            "识别、定位、相机叠加与录制均在 ThreadX 设备端闭环",
        ),
        evidence="381 μs/次；int8 评估 99.6%；播放实测 5/5",
        source="docs/PROJECT_STATUS.md",
    ),
)


SYSTEM_LAYERS = (
    ("应用", "实时成像 · 声源录音 · 麦阵诊断 · 媒体中心 · USB 存储"),
    ("融合", "频带管理 · 声源跟踪 · NPU 分类 · 相机热力图合成"),
    ("算法", "窗函数 / FFT · GCC-PHAT · SRP 粗精搜索 · 波束形成"),
    ("运行时", "ThreadX · TouchGFX · FileX · USBX · CMSIS-DSP"),
    ("硬件", "STM32N647 · 4×PCMD3180 · 32×MEMS · IMX219 · SD"),
)

APPLICATIONS = (
    ("气体泄漏", "高频嘶声频带追踪"),
    ("机械诊断", "轴承异响与撞击识别"),
    ("电气巡检", "电弧 / 放电声源定位"),
    ("定向取证", "瞄准录音、录像与导出"),
)

FOOTNOTE = (
    "所有性能数字均来自当前工程文档记录的板级或数据集测试；"
    "测试条件随指标标注，未将单点波动扩写为通用定位精度。"
)
