# NECCS V2 正式迁移计划

## Summary

这次迁移不按“临时过渡工程”处理，而是把当前 STM32N6 工程作为 `NECCS V2` 正式架构搭建起来。旧 STM32H7 工程作为 `V1` 参考，目标是功能一致，但实现方式允许升级：更清晰的模块边界、更稳定的板级配置、更好的调试诊断、更适合 N6 的内存、DMA、FSBL、外部 Flash 和算法运行方式。

总体策略是：先搭建完整框架，再按功能块逐个填充。每一块都必须能独立验证，通过后再接入主流程。

## Architecture

- `Board`：固定 N6 板级配置，包含时钟、GPIO、SAI、I2C、XSPI、SDMMC、LTDC、DCMIPP、调试串口、启动配置。后续 N657 作为正式目标板配置维护，不作为临时补丁。
- `Drivers`：底层器件驱动，包括 `PCMD3180`、LCD、触摸、相机、SD、外部 Flash。驱动只负责硬件动作，不直接参与业务逻辑。
- `Services`：系统服务层，包括音频采集服务、帧队列、数据流、存储、显示刷新、性能统计、错误诊断。
- `Algorithm`：迁移旧工程 `ai_*` 算法，但按 N6 优化内存布局、CMSIS-DSP/FPU/MVE、缓存一致性和可测试输入。
- `App`：恢复旧工程功能入口，包括音频成像、频谱、声级、异常检测、UI、相机叠加、录制和数据输出。
- `Diagnostics`：新增正式诊断能力，启动日志、外设自检、DMA/SAI 错误计数、帧率统计、栈水位、heap 状态、版本信息都应可查看。

## Migration Phases

- 第 1 阶段：正式工程骨架。确认 FSBL、Appli、外部 Flash、调试入口、FreeRTOS、UART 日志、错误处理、任务框架和目录结构，形成以后所有功能填充的基底。
- 第 2 阶段：音频硬件链路。优先完成 `PCMD3180 + I2C + SAI1 A/B + GPDMA`，目标是稳定得到连续音频帧，而不是先追求算法效果。
- 第 3 阶段：音频软件接口。把 N6 采集结果整理成统一 `audio_frame`，保持旧功能需要的 `16ch/48k/frame=256` 能力，同时预留 `32ch/48k` 和高采样率模式。
- 第 4 阶段：算法迁移与提升。迁移 SRP-PHAT、beamforming、bandpass、preprocess、spectrum、sound level。先用合成数据验证，再接真实音频。
- 第 5 阶段：显示与交互。恢复 LCD、LVGL/UI、触摸、热图显示、状态页。V2 要加入诊断页，用来显示音频帧率、DMA 状态、PCMD 状态、算法耗时。
- 第 6 阶段：相机、存储和数据输出。恢复相机、SD/FatFs、录制、串口/网络/文件输出等功能，每个模块单独验收后再并入主任务。
- 第 7 阶段：整机联调。完成启动流程、运行模式、异常恢复、性能优化和长期稳定性测试，达到 V1 功能一致且 V2 结构更清楚。

## Fixed Technical Decisions

- RTOS 继续使用 FreeRTOS，不切 ThreadX。N6 性能优化重点放在 DMA、cache、内存区、算法优化和任务调度。
- 音频时钟沿用旧工程思路：MCU 输出 `BCLK/FSYNC`，PCMD3180 作为从机。
- N657 目标引脚固定为：`MIC_FS PB0`、`MIC_BCLK PB6`、`MIC_SDOUT1 PB7`、`MIC_SDOUT2 PE3`、`I2C1_SDA PC1`、`I2C1_SCL PH9`、`MIC_SHDNZ PC6`。
- 旧工程功能必须恢复，但旧代码不强行原样照搬。凡是 H7 相关、临时写法、软 I2C、弱诊断、强耦合任务，都允许在 V2 中重构。
- 所有 DMA buffer、音频帧 buffer、算法大数组必须明确内存位置和 cache 策略，不能靠默认链接脚本碰运气。

## Interfaces To Establish

- `neccs_board_init()`：完成正式板级初始化，并输出启动诊断结果。
- `neccs_audio_init()` / `neccs_audio_start()` / `neccs_audio_stop()`：统一音频采集控制。
- `neccs_audio_get_frame()`：上层只从这里拿音频帧，不直接碰 SAI/DMA buffer。
- `neccs_diag_report()`：统一输出系统状态、外设状态、错误计数和性能统计。
- `neccs_app_start()`：应用层总入口，按配置启动音频、算法、显示、存储、相机等模块。

## Test Plan

- 启动测试：FSBL 能加载 Appli，UART 输出版本、启动阶段、外设初始化结果。
- 调试测试：CubeIDE/J-Link/ST-LINK 能 attach 到运行程序，断点、变量查看、调用栈可用。
- 音频硬件测试：I2C 能识别 PCMD3180，示波器确认 `BCLK/FSYNC` 正确，SAI/GPDMA 半满和满回调稳定。
- 音频数据测试：每通道 RMS、峰值、通道顺序、静音/敲击响应正常。
- 算法测试：合成输入和真实输入都能得到稳定频谱、声级、定位/热图结果。
- UI 测试：显示刷新、触摸响应、诊断页、热图页、相机叠加页逐项恢复。
- 稳定性测试：长时间运行无 DMA error、无 SAI overrun、无任务栈溢出、无 heap 异常。

## Assumptions

- 当前迁移目标定义为 `NECCS V2 on STM32N6`，不是临时 demo。
- V1 的功能作为验收基准，V2 的实现可以更模块化、更可调试、更适合 N6。
- 后续实施时按本计划逐步推进，每次只做一个可验证功能块，避免一次性搬完整个旧工程。
