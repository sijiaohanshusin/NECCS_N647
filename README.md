# NECCS_N647 工程速查

## 2026-06-25 PCMD3180 MIC 映射辅助

- 最新硬件结论：前期 PCMD3180 异常包含电源连接问题；PDMCLK/PDCLK 无时钟的根因之一是 FSYNC 虚焊。继续排查前先确认电源、FSYNC、BCLK、PDMCLK 实测正常。
- APP 调试页已在 `MIC RAW16` 区域增加峰值音量条，用现有 SAI DMA 峰值数据显示，不改变 PCMD3180 配置和采样链路。
- MIC 位置匹配工具位于 `docs/tools/pcmd3180_mic_mapper.html`。使用方法：打开 HTML，选择屏幕上响应的 `Mxx`，再点击阵列图中的实际麦克风位置；完成后导出 JSON/CSV，后续把最终匹配结果记录回工程。
- 麦克风物理坐标参考 `麦克风阵列的图（现采用主方案）/array_32ch_coords.csv`，映射时以当前板子实测为准。

## 2026-06-25 PCMD3180 I2C 诊断策略

- `EADDR/ADDR` 地址扫描现在只作为诊断信息，不再阻断 PCMD3180 的真实配置流程。
- 原因：SAI/LTDC 已运行时，16 轮 bit-bang I2C 扫描可能出现漏 ACK；如果把扫描结果作为硬门槛，后续 `Probe -> Configure -> ReadStatus` 根本不会执行，屏幕只能看到一片 `ERR/IO`，无法定位真正失败点。
- 当前排查时应同时看两层信息：`ADDR/16` 反映总线/地址稳定性，设备表里的 `Prb/Cfg/St` 反映真正探测、写寄存器、读状态的结果。
- `Prb` 现在也只是诊断信息，不再阻断 `Wake -> Configure -> ReadStatus`。TI 手册示例本身从写 `P0_R2` 唤醒开始；若睡眠态读探测偶发失败，仍应继续执行真实写配置以定位首个失败寄存器。
- 若 `ADDR/16 BAD` 但 `Cfg/St` 能变绿，优先继续排查 PDMCLK/SAI 数据链路；若 `Cfg` 仍为 `IO`，优先用示波器或逻辑分析仪看 SCL/SDA 的 ACK、拉升沿和是否被其它初始化阶段干扰。
- 若烧录后 `Prb=IO` 但 `Cfg/St=OK`，说明读探测不稳定但写配置链路可用；若 `Cfg=IO` 或 `St=IO`，优先记录屏幕上的 `Fail Ux reg/wr/rd` 和 `I2C:...` 行，用它定位失败发生在唤醒、寄存器表写入还是状态回读阶段。

## 2026-06-25 PCMD3180 / SAI 时钟结论

- 当前 APP-only 工程的 CubeMX `SystemClock_Config()` 属于 FSBL context，APP 冷启动不会执行它。
- PCMD3180 调试前，APP 必须主动确认 `PLL2=245.76 MHz`、`SAI1/IC7=12.288 MHz`，否则 SAI 可能落到 `PLL2=64 MHz / SAIclk=3.2 MHz` 这类错误状态。
- 若屏幕显示 `PLL2:64000000` 或 `SAIclk:3200000`，优先排查 APP 音频时钟链路，不要先改 PCMD3180 寄存器表。

> 新会话 / 压缩上下文恢复提醒：先读本 `README.md`，再根据任务需要读 `AGENTS.md` 和 `MIGRATION_CONTEXT.md`。如果聊天摘要与仓库文件不一致，以当前仓库文件和 `git status` 为准。

本仓库是 NECCS 从 H7 迁移到 STM32N647 的当前工程入口。此文件用于给人和 AI 协作快速恢复上下文，后续遇到新的硬件结论、构建方式或调试结论时可以继续更新。

## 当前工程状态

- 主 APP 工程：`NECCS_N647_App`，CubeMX/CubeIDE 工程位于 `NECCS_N647_App/STM32CubeIDE/Appli`。
- 当前重点：PCMD3180 麦克风阵列调试，以及 APP 外设配置稳定化。
- 已验证基础项：外部 HyperRAM、RGBLCD 显示、APP Debug/Release 构建。
- 当前 PCMD3180 现象：SAI 的 FSYNC 为 48 kHz，BCLK 约 12.288 MHz；PCMD3180 I2C 在 SAI 启动后存在不稳定现象；PCMD3180 PDMCLK 输出仍需继续确认和根因排查。
- 外部 Flash 启动链路：FSBL 放在 `0x70000000`，APP 放在 `0x70100400`。

## 常用命令

在仓库根目录执行：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build_n647_app.ps1 -Configuration Release
```

这个脚本会使用 STM32CubeIDE 自带的 `make.exe` 和 GNU 工具链，直接构建 APP Release，并检查关键产物是否生成。默认会先 `clean` 再 `all`，用于避免残留损坏产物影响结果。

常用产物：

- `NECCS_N647_App/STM32CubeIDE/Appli/Release/NECCS_N647_App_Appli.elf`
- `NECCS_N647_App/STM32CubeIDE/Appli/Release/NECCS_N647_App_Appli.bin`
- `NECCS_N647_App/Binary/appli.hex`

如果需要把最新 APP HEX 同步到烧录目录：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build_n647_app.ps1 -Configuration Release -CopyToFlashImages
```

同步后的 APP 镜像：

- `_flash_images/appli.hex`

如果需要重新整理 FSBL + APP 烧录镜像：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\rebuild_n647_boot_images.ps1
```

## CubeMX/CubeIDE 约定

- 外设初始化和时钟配置必须尽量由 CubeMX/IOC 管理，业务逻辑放到用户代码或独立模块中。
- 手写业务模块需要同时确认 CubeIDE `.cproject` 的 source entries，否则 CubeIDE 重新生成 makefile 后可能漏编译文件。
- 不要把 APP 接管 `XSPI2` 外部 Flash 初始化；外部 Flash 启动链路归 FSBL。
- PCMD3180 的 SAI 配置必须按 N657 麦克风阵列设计迁移，不按 N647 板厂音频例程替换。

## 已知排查点

- 如果构建报 `file format not recognized`，优先关闭调试会话，确认没有 `arm-none-eabi-gdb` 占用，再清理对应 `Debug` 或 `Release` 目录后重建。
- 如果 CubeMX 重新生成后 SAI 模式变成 `SPDIF TX Transmitter`，说明 IOC 中 SAI 模式/引脚配置被恢复逻辑改坏，需要先修 IOC 再生成。
- 如果 PCMD3180 在 SAI 启动后 I2C 不稳定，优先检查 SAI GPIO 速度、I2C GPIO 电气配置、线缆串扰、PCMD3180 时钟/复位/电源时序，不要只靠重复写寄存器掩盖问题。

## 资料入口

- 迁移上下文：`MIGRATION_CONTEXT.md`
- OCR 知识库：`docs/knowledge`
- H7 原工程参考：`D:\Project\NECCS\H7_Original\firmware\NECCS`
- PCMD3180 OCR 手册：`docs/knowledge/ti-pcmd3180-1629184e-801fec58.tool.md`
