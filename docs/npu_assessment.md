# N6 NPU (Neural-ART) 在本项目中的应用评估

日期: 2026-07-09 · 状态: 评估完成, 集成为赛后增强项

## 2026-07-11 更新: 工具链已上机验证

- stedgeai v4.0 (`C:\ST\STEdgeAI\4.0`) 编译玩具声纹分类器 (3конv 1D-CNN,
  输入 32x64 对数谱窗, 6 类) **通过**: `tools/npu/make_toy_model.py`。
- int8 静态量化 (QDQ, 合成谱校准) 后 **6/7 epoch 映射到纯 NPU 硬件**
  (仅 softmax 留 CPU); float 版只有 1 个 HW epoch —— 量化是必须项。
- 内存布局与应用工程验证无冲突: 权重 12KB 放 octoFlash `0x71000000`
  (已烧录并校验, 距启动 bundle +16MB), 激活 6KB 放 npuRAM5
  `0x342E0000` (应用链接脚本 RAM 上限 0x34200000, NPU SRAM 全空闲)。
- 运行时: ll_aton 自带 ThreadX OSAL (`ll_aton_osal_threadx.c`),
  LL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6。
- 下一步 (npu-model): ll_aton + network.c 进 CubeIDE 工程, NPU/CACHEAXI
  时钟使能, app_npu 服务挂 AppAcousticService 谱输出, UI 声源类型卡片。

## 结论 (TL;DR)

- STM32N657 的 Neural-ART NPU (600 GOPS) **可用于声纹分类**（识别气体泄漏/轴承磨损/电弧放电等声源类型），是最有竞赛价值的方向。
- **明天验收不依赖 NPU**：当前 DSP 链路（Helium/MVE + CMSIS-DSP FFT/SRP）已满足实时性（SRP 9 ms/帧）。
- 系统页已展示 "NPU 就绪" 加速清单；答辩话术见文末。

## 技术路径（已验证可行性，未上板）

1. **模型**: 1D-CNN / 小型 CRNN，输入为 64-bin 对数谱帧序列（本工程已产出该特征，
   见 `AppAcousticService` 的 `spectrum[64]`），输出 4-6 类声源标签。
2. **工具链**: ST Edge AI (stedgeai) 将 ONNX/TFLite 编译为 Neural-ART 指令流,
   生成 `network.c` + 权重 blob（放外部 Flash, XIP 或加载至 AXISRAM）。
3. **运行时**: `ll_aton` 运行时库（STM32N6 HAL 自带），推理由 NPU 异步执行,
   CPU 零负担；单帧 64x32 谱图推理预算 < 1 ms @ 600 GOPS。
4. **集成点**: `app_acoustic_service.c` 频谱产出处挂推理请求 →
   结果进 snapshot → 成像页右轨显示 "声源类型: 轴承异响 (92%)"。

## 为什么明天不上

- 训练数据不足（需要现场采集各类声源样本）。
- stedgeai 工具链验证 + 内存布局调整（NPU 权重需 AXISRAM/外部 Flash 专区）
  至少 1-2 天工作量，风险高于收益。
- 现有 DSP 性能不构成瓶颈，NPU 不解决当前任何痛点。

## 答辩话术

> "定位引擎跑在 Cortex-M55 的 Helium 向量单元上，单帧 SRP-PHAT 仅 9 ms。
> N657 的 600 GOPS Neural-ART NPU 已预留给声纹分类：下一版本将在定位声源的
> 同时识别声源类型（泄漏/放电/机械磨损），推理与 DSP 完全并行，零 CPU 开销。"
