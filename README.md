# NECCS_N647 工程速查

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
