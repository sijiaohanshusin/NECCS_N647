# NECCS N647 Migration Context

## 0. Quick Resume (ASCII)

Use this section first if terminal encoding makes Chinese text unreadable.

- Repo now being prepared: `D:\Project\NECCS\N647_BaseBoard\firmware\NECCS_N647`
- Old project source of truth: `D:\Project\NECCS\H7_Original\firmware\NECCS`
- Current target: `ALIENTEK` / 正点原子 `ATK-CNN647B` (`STM32N647`)
- Common shorthand used in chat: `CNN-647B`
  - treat it as the same board as `ATK-CNN647B`
- Current authoritative local board references:
  - firmware examples: `D:\Project\NECCS\N647_BaseBoard\firmware\NECCS_N647\_vendor_software_ref`
  - hardware files: `D:\Project\NECCS\N647_BaseBoard\hardware`
- Previous board package path was under the deprecated `Program\NECCS_N647` tree and is no longer the active workspace path.
- Future target: self-developed `STM32N657` board, not current phase
- Migration is not done until old project capabilities are fully restored
- New toolchain stack:
  - `STM32CubeIDE`
  - `STM32CubeMX`
  - `STM32CubeProgrammer`
  - `STEdgeAI-Core`
- First milestone:
  - project skeleton
  - clock
  - startup
  - build
  - download
  - debug
- Current task list:
  - map old H7 resources to N647 resources
  - create N647 base project in CubeIDE
  - make startup/clock/download/debug work
  - finish board peripheral configuration
  - learn N6 workflow
  - learn and implement `EML / FSBL` related flow
- User is not familiar with:
  - `CubeIDE`
  - `STM32N6` workflow
  - `EML`
  - `FSBL`
- Confirmed old board:
  - third-party `STM32H743` board
  - MCU `STM32H743IIT6`
  - external memory `32MB`
- Confirmed new board:
  - `ATK-CNN647B` from 正点原子 / ALIENTEK
  - external memory `32MB`
  - has LCD, touch, external memory, camera, audio hw, uart debug
- Board package also includes another hardware variant:
  - `ATK-DNN647`
- Online doc shortcut in the package points to:
  - `https://wiki.alientek.com/docs/Boards/STM32/DNN647/TOC/`
- Audio path assumption:
  - microphone array stays
  - `2 x PCMD3180` stays
  - main change is MCU H7 -> N647
- Important new caveat:
  - stock ALIENTEK BSP is not built around `PCMD3180`
  - vendor BSP includes `ES8388` audio codec instead
  - keep user migration intent and stock board BSP as two separate things
- Priority after base project is ready:
  1. external memory
  2. audio capture
  3. localization algorithm
  4. LCD / touch
  5. camera
- Must restore core capabilities:
  - startup / download / debug
  - LCD
  - touch
  - audio capture
  - `SRP-PHAT`
  - camera
- Do not shrink scope to audio only. Old project app layer is larger than expected.
- Old project key entry files:
  - `START\Core\Src\main.c`
  - `START\Core\Src\freertos.c`
- Old project key hardware/software facts:
  - `SAI1 + DMA` audio capture path
  - `2 x PCMD3180`
  - I2C addresses `0x4C` and `0x4D`
  - one-time init task brings up soft I2C, touch, stream, tasks, DMA, PCMD3180, camera
- Old project has more modules than the simplified README suggests:
  - touch
  - camera
  - storage / SD
  - recorder
  - spectrum / UI
  - profile / perf
  - anomaly / tracker / trigger / laser
- N6 specific caution:
  - do not think like H7 boot flow
  - N6 boots from on-chip ROM first
  - flash/serial/development boot exist
  - FSBL matters
  - EML is currently assumed to mean External Memory Loader
- Vendor package facts now confirmed:
  - vendor BSP source package exists: `4，程序源码\SoftwarePackage.zip`
  - vendor external loader exists:
    - `SoftwarePackage/External_Loader/MX25UM25645G_ATK-CNN647B/Binary/MX25UM25645G_ATK-CNN647B_ExtMemLoader.stldr`
  - the provided external loader matches the confirmed board variant name `ATK-CNN647B`
  - vendor FSBL example exists:
    - `SoftwarePackage/FSBL/MX25UM25645G_W958D8NBYA5I_Example`
  - vendor FSBL postbuild signs the FSBL with `STM32_SigningTool_CLI`
- Custom EML build note:
  - CubeMX/CubeIDE generated the EML sources, but Release/Debug build metadata initially missed middleware sources.
  - The local fix changed the EML subproject to include middleware sources and then manually added `stm32n6xx_hal_xspi.c` to generated `subdir.mk` / `objects.list`.
  - Without `stm32n6xx_hal_xspi.o`, Release link fails with undefined `HAL_XSPI_Init`, `HAL_XSPI_Command`, `HAL_XSPI_Receive`, `HAL_XSPI_Abort`, etc.
  - Verified local Release main-build succeeds and exports `StorageInfo`, `Init`, `Write`, `SectorErase`, `MassErase`, `Verify`.
  - Current custom loader candidate:
    - `NECCS_N647_ExtMemLoader/NECCS_N647_ExtMemLoader/ExtMemLoader/Release/MX25UM25645G_ATK-CNN647B_NECCS.stldr`
  - 2026-05-31 runtime test result:
    - Official vendor loader can read `0x70000000` successfully in STM32CubeProgrammer, so board wiring, flash, ST-LINK, and CubeProgrammer are OK.
    - First custom loader parsed correctly but failed at read time and dropped the device connection.
    - Custom loader v2 was rebuilt at `ExtMemLoader/Release/MX25UM25645G_ATK-CNN647B_NECCS_v2.stldr`.
    - v2 source changes copied from the working vendor loader: use `PWR_SMPS_SUPPLY`, configure `VDDIO3` to `1V8`, enable `PN1/XSPIM_P2_NCS1`, and set `HAL_XSPI_CSSEL_OVR_NCS1`.
- Confirmed memory-related device references in the vendor package:
  - NOR Flash: `MX25UM25645G`
  - HyperRAM: `W958D8NBYA5I`
  - SD NAND docs also exist in the package
- Confirmed BSP/peripheral references in the vendor package:
  - audio: `ES8388`
  - camera: `OV5640`, `IMX335`
  - touch: `FT5206`, `GT9xxx`
  - ethernet PHY: `YT8512C`
- Important missing/unclear items:
  - whether current migration will use stock ALIENTEK peripherals or custom NECCS-side peripherals
  - exact memory map / boot source choice that will be used in the new project
  - how much adaptation is needed because vendor package is based on older CubeN6 versions
- Version drift to remember:
  - vendor package includes `STM32CubeIDE 1.17.0`
  - vendor package includes `STM32CubeMX 6.13.0`
  - vendor package includes `STM32CubeN6 v1.0.0`
  - user local environment already showed `STM32Cube FW_N6 V1.3.0`
  - expect API / project-structure drift between vendor examples and local tools
- Important rule:
  - do not depend on `ocrkb`
  - do not keep writing docs as if `ocrkb` is available
  - do not jump into algorithm port before N6 boot/download/debug path is understood

## 0.1 Reassessment From ALIENTEK Package (ASCII authoritative)

This section supersedes older assumptions made before the ALIENTEK package was copied in.

- The local package is real and directly usable through the reorganized workspace:
  - firmware examples: `D:\Project\NECCS\N647_BaseBoard\firmware\NECCS_N647\_vendor_software_ref`
  - hardware files: `D:\Project\NECCS\N647_BaseBoard\hardware`
- This is not a vague third-party board anymore.
  - It is an `ALIENTEK` / 正点原子 `STM32N647` development-board package.
- The package contains two board variants:
  - `ATK-CNN647B`
  - `ATK-DNN647`
- The actual board variant in hand is now confirmed:
  - `ATK-CNN647B`
- Naming note:
  - user shorthand `CNN-647B` and package name `ATK-CNN647B` refer to the same board
- The online documentation shortcut included in the package points to `DNN647`.
  - Treat this as a package-level mixed reference, not as proof that the physical board is `DNN647`.
- The package already contains board schematics:
  - `3，原理图\ATK-CNN647B V1.0.pdf`
  - `3，原理图\ATK-DNN647 V1.0.pdf`
- The package already contains a full vendor software package:
  - `4，程序源码\SoftwarePackage.zip`
- The package already contains a ready-made external loader:
  - `SoftwarePackage/External_Loader/MX25UM25645G_ATK-CNN647B/Binary/MX25UM25645G_ATK-CNN647B_ExtMemLoader.stldr`
- This loader name matches the confirmed board variant `ATK-CNN647B`.
- Therefore the previous statement "vendor EML not confirmed" is obsolete.
- The package already contains an FSBL example:
  - `SoftwarePackage/FSBL/MX25UM25645G_W958D8NBYA5I_Example`
- The vendor FSBL postbuild script signs the FSBL using `STM32_SigningTool_CLI`, then emits `fsbl.hex`.
- Therefore the previous statement "exact FSBL project flow not confirmed" is too weak now.
  - It is not fully understood yet, but there is already a concrete local example to follow.
- 2026-05-31: extracted the vendor FSBL example for direct reference:
  - `_vendor_fsbl_ref/MX25UM25645G_W958D8NBYA5I_Example.ioc`
  - `_vendor_fsbl_ref/STM32CubeIDE/FSBL/postbuild.sh`
  - `_vendor_fsbl_ref/Binary/fsbl.hex`
  - key observed settings: `ProjectStructure=FSBL true; Appli false; ExtMemLoader false`, `EXTMEM_MANAGER.RefParam_BOOT_enable=true`, `EXTMEM_MANAGER.RefParam_MEMORY_1_Driver_Selection=EXTMEM_USER`, `XSPI2` for `MX25UM25645G`, `XSPI1` for `W958D8NBYA5I`, `VDDIO2/VDDIO3=1V8`.
- 2026-05-31: build issue root cause and fix:
  - Do not import/build the isolated `_vendor_fsbl_ref` copy as the active project. It is useful for reading, but its generated paths miss the original `SoftwarePackage` layout.
  - The working layout is `_vendor_software_ref`, which preserves the needed siblings: `FSBL`, `STM32Cube_FW_N6_V1.0.0`, and `Drivers/BSP`.
  - Headless CubeIDE Release build succeeded for `_vendor_software_ref/FSBL/MX25UM25645G_W958D8NBYA5I_Example/STM32CubeIDE/FSBL`.
  - Build output confirmed: 0 errors, 0 warnings, `fsbl.hex` regenerated at `_vendor_software_ref/FSBL/MX25UM25645G_W958D8NBYA5I_Example/Binary/fsbl.hex`.
- 2026-05-31: first Appli bring-up target:
  - Extracted vendor `Projects/01_LED` into `_vendor_software_ref/Projects/01_LED`.
  - Headless CubeIDE Release build succeeded for `_vendor_software_ref/Projects/01_LED/STM32CubeIDE/Appli`.
  - `Projects/01_LED/STM32CubeIDE/Appli/postbuild.sh` converts `01_LED_Appli.bin` to Intel HEX at address `0x70100400`.
  - Generated application image: `_vendor_software_ref/Projects/01_LED/Binary/appli.hex`.
  - Expected flash layout for this smoke test: FSBL at `0x70000000`, Appli at `0x70100400`.
- The vendor package gives concrete external-memory clues:
  - NOR Flash driver and docs for `MX25UM25645G`
  - HyperRAM driver and docs for `W958D8NBYA5I`
  - SD NAND docs also exist
- This means the previous statement "exact external flash model on N647 board not confirmed" is no longer generally true.
- What is still worth checking is whether the exact example memory map matches the intended NECCS project layout.
- The vendor BSP clearly shows the stock board ecosystem differs from the old NECCS hardware chain:
  - audio codec: `ES8388`
  - cameras: `OV5640`, `IMX335`
  - touch: `FT5206`, `GT9xxx`
  - ethernet PHY: `YT8512C`
- This is a major migration note:
  - old NECCS intent still says `16ch mic array + 2 x PCMD3180`
  - stock ALIENTEK N647 board package is centered on a different peripheral set
  - migration planning must distinguish:
    - stock dev-board bring-up path
    - final NECCS feature path
- The package also contains generic CubeIDE application examples such as:
  - `Projects/01_LED`
  - `Projects/05_Serial`
  - sensor examples
- This changes the recommended bootstrap path:
  - using a vendor example as the first bring-up base is now more practical than starting from a totally blank project
- Tool/version drift is now explicit:
  - vendor package bundles `STM32CubeIDE 1.17.0`
  - vendor package bundles `STM32CubeMX 6.13.0`
  - vendor package bundles `STM32CubeN6 v1.0.0`
  - user local environment already showed `STM32Cube FW_N6 V1.3.0`
- Therefore any direct copy from vendor examples must be checked for:
  - middleware layout changes
  - FSBL project structure changes
  - external memory manager/loader API drift
  - linker-script naming drift

## 1. 文档定位

这不是对外文档，也不是正式设计说明。

这份文件主要写给后续继续工作的 AI / Codex 自己看，用来防止上下文丢失。目标是：

- 记录当前已经确认的迁移方向
- 记录旧工程真实范围，避免误判“只迁移主链路就结束”
- 记录接下来应该先做什么
- 记录哪些点还没确认，后续不要擅自脑补

当前迁移工作目录：

- `D:\Project\NECCS\N647_BaseBoard\firmware\NECCS_N647`

旧工程目录：

- `D:\Project\NECCS\H7_Original\firmware\NECCS`

重要约束：

- 不要依赖 `ocrkb`
- 不要在后续说明里继续暗示 `ocrkb` 可用
- 当前用户对 `STM32N6` 新流程、`CubeIDE`、以及 `EML / FSBL` 仍不熟，需要协助

## 2. 当前已确认结论

### 2.1 迁移目标

- 当前第一阶段目标平台是 `STM32N647`
- 使用的是 `第三方 N647 开发板`
- 后续还会迁移到 `自研 STM32N657 板`，但那是后续事项，不属于当前第一阶段主目标

### 2.2 迁移完成的定义

用户的口径不是“做出一个能跑的 demo 就算完成”，而是：

- 迁移工作要一步一步推进
- 最终要把旧工程里应该具备的功能都恢复出来
- 全部功能实现后，才算迁移完成

因此不要把当前工作误收缩为“只把音频算法跑起来”。

### 2.3 新平台开发环境

当前确认的新环境：

- `STM32CubeProgrammer`
- `STM32CubeMX`
- `STM32CubeIDE`
- `STEdgeAI-Core`

当前主要 IDE / 调试构建入口：

- `STM32CubeIDE`

### 2.4 第一阶段目标

第一步先做：

- `基础工程骨架`
- `时钟`
- `启动`
- `编译`
- `下载`
- `调试`

不要一上来就直接做算法移植。

### 2.5 当前阶段任务

用户已经明确的近期任务：

- 梳理 `N647` 板卡硬件资源与旧 `H7` 工程资源的对应关系
- 用 `STM32CubeIDE` 建立 `N647` 基础工程
- 打通启动、时钟、下载、调试
- 同时完成 `N647` 板子的外设配置
- 学习和梳理 `STM32N6` 开发流程
- 补上 `EML / FSBL` 相关认知和落地流程

### 2.6 迁移优先级

在基础工程跑通之后，用户确认的外设迁移顺序是：

1. 外扩内存
2. 音频采集链路
3. 声源定位算法
4. LCD / 触摸
5. 摄像头

注意：这只是“优先顺序”，不是“最终范围”。

## 3. 新旧平台范围认知

### 3.1 旧工程平台

旧工程当前运行在：

- `第三方 STM32H743 开发板`

旧平台核心芯片：

- `STM32H743IIT6`

旧平台外扩内存规格，用户已确认：

- `32MB`

### 3.2 新平台

新平台当前目标：

- `第三方 STM32N647 开发板`

用户已确认该板至少具备：

- `LCD`
- `触摸`
- `外扩内存`
- `摄像头`
- `音频相关硬件`
- `串口调试接口`

新平台外扩内存规格，用户已确认按下列口径处理：

- `32MB`

### 3.3 音频链路假设

用户已确认，当前迁移时音频硬件链路按“基本不变”处理：

- `16 路麦克风阵列` 保留
- `2 x PCMD3180` 音频前端方案继续保留
- 主要变化是 `MCU 从 H7 换到 N647`

后续如果板卡资料显示音频接口分配变了，再修正文档。

## 4. 旧工程真实范围

不要只看最初 README 的简化描述。旧工程实际功能范围比“音频 + 显示 + 摄像头”更大。

### 4.1 旧工程关键目录

- `D:\Project\NECCS\Program\NECCS\START\Core`
- `D:\Project\NECCS\Program\NECCS\START\User\Algorithm`
- `D:\Project\NECCS\Program\NECCS\START\User\App`
- `D:\Project\NECCS\Program\NECCS\START\User\BSP`
- `D:\Project\NECCS\Program\NECCS\START\User\Hardware`
- `D:\Project\NECCS\Program\NECCS\START\MDK-ARM`
- `D:\Project\NECCS\Program\NECCS\START\START.ioc`

### 4.2 已明确存在的应用层模块

旧工程 `START\User\App` 下可以直接确认的模块包括但不限于：

- `app_audio_task.*`
- `app_display.*`
- `app_touch.*`
- `app_camera.*`
- `app_capture.*`
- `app_data_stream.*`
- `app_data_output.*`
- `app_runtime.*`
- `app_perf.*`
- `app_ui_cli.*`
- `app_lvgl_ui.*`
- `app_ui_screens.*`
- `app_ui_spectrum_panel.*`
- `app_storage_task.*`
- `app_sd.*`
- `app_recorder.*`
- `app_profile.*`
- `app_spectrum.*`
- `app_noise_floor.*`
- `app_sound_level.*`
- `app_tracker.*`
- `app_trigger.*`
- `app_laser.*`
- `app_anomaly.*`

结论：

- 旧工程后期已经不是单一声源定位 demo
- 迁移范围中需要保留“以后还要迁”的意识
- 不能只围绕 `SRP-PHAT` 一条链路建工程

### 4.3 已明确存在的硬件/BSP模块

`START\User\Hardware`：

- `pcmd3180.*`
- `soft_i2c.*`
- `camera_ov2640.*`

`START\User\BSP`：

- `sdram.*`
- `LCD\lcd.*`
- `LCD\ltdc.*`
- `LCD\dma2d_accel.*`
- `LCD\tft_spi.*`
- `TOUCH\touch.*`
- `TOUCH\touch_gt9xxx.*`
- `TOUCH\touch_ft5206.*`
- `TOUCH\touch_i2c.*`
- `SD\bsp_sd.*`
- `SD\diskio_sd.c`

### 4.4 已明确存在的算法模块

`START\User\Algorithm`：

- `ai_config.*`
- `ai_preprocess.*`
- `ai_beamforming.*`
- `ai_bandpass.*`
- `ai_beamsteer.*`
- `ai_srp_lut.*`

### 4.5 已定位的参考资料入口

旧 `H7` 板资料入口：

- `D:\Project\NECCS\Program\NECCS\这是这个开发板的官方资料，需要可以取用\1. 用户手册\STM32H743IIT6 Core Board V1.1_用户手册.pdf`
- `D:\Project\NECCS\Program\NECCS\这是这个开发板的官方资料，需要可以取用\2. 原理图\STM32H743IIT6 V1.1_SCH.pdf`

当前仓库内已定位到的 `N6` 通用资料入口：

- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\rm0486-stm32n647657xx-armbased-32bit-mcus-stmicroelectronics.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\es0620-stm32n6xxxx-device-errata-stmicroelectronics.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\pm0273-stm32-cortexm55-mcus-programming-manual-stmicroelectronics.pdf`

当前仓库内已定位到的 `N6` 官方评估板原理图：

- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\官方n6评估板\mb1280-3v3-c01-schematic.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\官方n6评估板\mb1854-csi-b01-schematic.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\官方n6评估板\mb1854-csi-b02-schematic.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\官方n6评估板\mb1860-rk050hr18c-b01-schematic.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\官方n6评估板\mb1939-n6570-c02-schematic.pdf`

说明：

- 用户口中的当前目标板是“第三方 `N647` 开发板”
- 但当前我还没有从仓库文件名中明确定位到它的专属资料文件
- 后续如果找到其板卡手册/原理图，应把路径补到本文件中

## 5. 旧工程关键实现事实

以下结论直接来自旧工程代码和工程文件，后续移植时应优先参考，而不是只看口头印象。

### 5.1 旧工程入口与初始化顺序

主入口文件：

- `D:\Project\NECCS\Program\NECCS\START\Core\Src\main.c`
- `D:\Project\NECCS\Program\NECCS\START\Core\Src\freertos.c`

从代码可直接确认的旧工程启动链路：

1. 配置 MPU
2. `HAL_Init()`
3. 应用自定义 MPU / Cache 配置
4. 系统时钟配置
5. 初始化 `SDRAM`
6. 初始化 `GPIO / DMA / USART1`
7. 初始化显示链路
8. 初始化 `SAI1`
9. 运行启动自检（当前有 `SDRAM` 冒烟测试）
10. 启动 `FreeRTOS`
11. 在一次性初始化任务中继续完成软 I2C、触摸、数据流、任务创建、音频前端、摄像头等初始化

### 5.2 旧工程音频初始化关键点

旧工程不是在 `main()` 里一次性把音频前端全配完，而是通过 `PCMD3180InitTask` 完成：

- 初始化 `Soft_I2C`
- 初始化 `Touch`
- 初始化数据流 / 算法运行态
- 创建应用任务
- 启动 `SAI DMA`
- 等待时钟稳定
- 依次初始化两片 `PCMD3180`
- 初始化并启动摄像头

这个顺序很关键，后续迁移到 `N647` 时不要随意打散。

### 5.3 旧工程音频硬件口径

从代码和文档可直接确认：

- `2 x PCMD3180`
- 地址分别是 `0x4C` 和 `0x4D`
- 一片对应 `slot 0-7`
- 一片对应 `slot 8-15`
- `SAI1 + DMA` 是主采集链路

### 5.4 旧工程调试与交互

可直接确认的旧工程调试能力：

- `USART1`
- 串口日志
- `CLI`
- `VOFA+` 调试输出
- 启动阶段诊断
- 运行时参数/性能相关模块

虽然用户这次没有把 CLI 单独列为“第一优先恢复项”，但它在旧工程里实际是很重要的辅助能力，不应彻底遗忘。

## 6. 用户认定的必须恢复能力

用户已经明确认定以下能力属于必须恢复的核心范围：

- 基本启动、下载、调试
- `LCD`
- `触摸`
- 音频采集链路
- `SRP-PHAT` 声源定位算法
- 摄像头相关

另一个重要口径：

- “旧工程里出现过的模块，后面都可能纳入迁移目标”

所以后续执行时应把“核心范围”和“完整范围”区分开：

- 核心范围：优先迁移、优先验证
- 完整范围：不要因为第一轮没做就忘记

## 7. N6 特有认知与当前风险

### 7.1 当前认知状态

用户对以下内容还不熟：

- `STM32CubeIDE` 新开发环境
- `STM32N6` 开发流程
- `EML`
- `FSBL`

这不是边缘问题，而是当前迁移工作的关键风险点。

### 7.2 当前对 N6 启动链路的保守理解

根据仓库内已有 `RM0486` 摘要信息，可先保守记住这些点：

- `STM32N6` 的 `Cortex-M55` 在应用复位后总是先从片上 `ROM` 启动
- `N6` 存在 `Flash boot / Serial boot / Development boot`
- `Flash boot` 会从外部 Flash 加载镜像
- 在安全启动场景下，启动镜像会涉及 `FSBL`
- `FSBL` 可以理解为第一阶段引导镜像/第一阶段启动固件

对当前任务最重要的不是一次性讲全安全启动体系，而是明确：

- `N6` 的启动/烧录/外部 Flash 组织方式，不能沿用 `H7` 的思维
- 基础工程建立前，必须先把 `N6` 的启动链路认知梳理清楚

### 7.3 关于 EML 的当前处理口径

用户提到的 `EML`，当前先按 `External Memory Loader` 理解。

在没有进一步板卡资料之前，先假设后续需要处理以下问题：

- `CubeProgrammer` / `CubeIDE` 是否需要匹配该板的外部 Flash Loader
- 第三方 `N647` 板是否已提供现成 Loader
- 如果没有现成 Loader，是否需要自行适配
- Loader、`FSBL`、外部 Flash 镜像布局之间如何协同

如果后续确认这里的 `EML` 指的不是 `External Memory Loader`，再修正。

### 7.4 建议重点参考的 N6 文档

当前仓库内已能直接确认的 N6 相关资料：

- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\rm0486-stm32n647657xx-armbased-32bit-mcus-stmicroelectronics.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\es0620-stm32n6xxxx-device-errata-stmicroelectronics.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\pm0273-stm32-cortexm55-mcus-programming-manual-stmicroelectronics.pdf`

`RM0486` 中已出现的一个需要后续补资料的文档线索：

- `UM3234 "How to proceed with boot ROM on STM32N6 MCUs"`

当前仓库里未确认该文档是否已存在。后续如果缺失，应补入。

### 7.5 当前 N6 风险提示

迁移时要主动关注这些差异，不要默认“H7 上能这么做，N6 也一样”：

- 启动链路不同
- 外部 Flash / 外部存储装载方式不同
- Cache / AXI / 启动后默认状态不同
- 安全启动约束可能影响调试与烧录
- 外设初始化顺序可能需要重排
- `CubeIDE` 工程组织与旧 `Keil` 工程不同

## 8. 当前最合理的实施路线

### 8.1 第 0 阶段：先别急着搬代码

先完成：

- 确认 `N647` 第三方开发板资料位置
- 梳理开发板的 `时钟源 / 外部 Flash / 外扩内存 / LCD / 触摸 / 摄像头 / 音频接口`
- 明确 `CubeIDE + CubeMX + CubeProgrammer` 下的最小工作流
- 梳理 `N6` 的 `FSBL / 外部 Flash / Loader` 最小闭环

如果这一步不清楚，后面会在下载、启动、调试阶段反复返工。

### 8.2 第 1 阶段：建立最小 N647 工程

目标：

- `CubeIDE` 可以建立和打开工程
- 工程可编译
- 能下载
- 能稳定连接调试
- 能跑最小启动代码

此阶段不要急着引入业务代码。

### 8.3 第 2 阶段：完成板级外设骨架

优先完成：

- 时钟
- GPIO
- 串口调试
- 外部存储相关配置
- 基础中断/缓存/MPU 认知

### 8.4 第 3 阶段：按用户指定顺序推进

顺序：

1. 外扩内存
2. 音频采集链路
3. 算法
4. LCD / 触摸
5. 摄像头

注意事项：

- 不要为了“尽快出画面”擅自把显示提前到音频前面
- 当前顺序是用户明确拍板过的

## 9. 后续 AI 接手时的优先阅读路径

如果后续上下文丢失，优先按下面顺序恢复认知：

1. 先读本文件
2. 再读旧工程 README 和项目说明
3. 再读旧工程入口文件
4. 再读 N6 参考手册中与启动/Boot 相关部分

建议阅读文件：

- `D:\Project\NECCS\Program\NECCS\README.md`
- `D:\Project\NECCS\Program\NECCS\project_doc.html`
- `D:\Project\NECCS\Program\NECCS\START\Core\Src\main.c`
- `D:\Project\NECCS\Program\NECCS\START\Core\Src\freertos.c`
- `D:\Project\NECCS\Program\NECCS\START\User\App\APP_RUNTIME_NOTES.md`
- `D:\Project\NECCS\Program\NECCS\START\User\App\APP_DISPLAY_MEMORY_NOTES.md`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\rm0486-stm32n647657xx-armbased-32bit-mcus-stmicroelectronics.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\es0620-stm32n6xxxx-device-errata-stmicroelectronics.pdf`
- `D:\Project\NECCS\Program\NECCS\stm32n6资料\stm32n6资料\pm0273-stm32-cortexm55-mcus-programming-manual-stmicroelectronics.pdf`

## 10. 待确认项

下面这些点目前不要擅自下结论：

- 第三方 `N647` 开发板的具体型号/厂商命名
- 该板外部 Flash 的具体型号与连接方式
- `EML` 是否已由板厂提供
- `FSBL` 的工程模板来源和推荐工作流
- 新板 LCD 的分辨率、控制链路、触摸控制器具体型号
- 新板摄像头模组/接口与旧工程 `OV2640` 是否完全对应
- 是否需要在第一轮就恢复 `CLI / VOFA+ / SD / Recorder / Tracker / Laser / Anomaly` 等扩展模块

写代码前如遇到这些点，优先查板卡资料或重新问用户，不要脑补。

## 11. 当前行动清单

从这份文档落地后，最优先的下一批动作应是：

1. 在 `NECCS_N647` 目录下建立基础工程结构
2. 记录并导入 `N647` 板卡资料位置
3. 梳理 `N647` 的启动、下载、调试最小闭环
4. 单独整理一份 `N6 / FSBL / EML` 工作流笔记
5. 再进入外扩内存和外设配置阶段

## 12. 明确不要做的事

- 不要再假设 `ocrkb` 可用
- 不要默认旧 `Keil` 工程能平移到 `CubeIDE`
- 不要把迁移范围误缩成“只迁 SRP-PHAT”
- 不要跳过 `N6` 启动链路认知就直接写外设代码

## 13. Immediate Consequences Of The Package Reassessment (ASCII)

The ALIENTEK package changes the practical strategy for the next steps.

### 13.1 Bring-up base choice

Old suggestion:

- create everything from scratch in CubeMX/CubeIDE

Updated suggestion:

- still learn how to create a clean CubeMX project
- but for the first real bring-up, strongly consider starting from a vendor example

Best first candidates from the vendor package:

- `SoftwarePackage/Projects/01_LED`
- `SoftwarePackage/Projects/05_Serial`

Reason:

- they already match the board family
- they already include CubeIDE project layout
- they already include N6-specific linker-script variants
- they reduce first-stage risk when learning N6 boot/debug flow

### 13.2 External loader strategy

Old uncertainty:

- maybe a custom `ExtMemLoader` must be created immediately

Updated strategy:

- first use the vendor-provided loader as the baseline reference
- then optionally create a custom loader as a learning exercise

This keeps the migration unblocked while still allowing EML learning.

### 13.3 FSBL learning strategy

Old uncertainty:

- FSBL flow exists in theory but local example was not pinned down

Updated strategy:

- use the local vendor FSBL example as the concrete study object
- inspect how it is built
- inspect how postbuild signing is done
- inspect how memory configuration is wired into the example

### 13.4 Hardware-mapping strategy

Do not collapse these two questions into one:

- "What does the stock ALIENTEK N647 board support?"
- "What does the final NECCS migration need?"

They overlap, but they are not identical.

Examples:

- stock board audio path appears centered on `ES8388`
- NECCS target audio path still assumes `16ch mic array + 2 x PCMD3180`
- stock board cameras in package include `OV5640` and `IMX335`
- old NECCS camera code is `OV2640`

Therefore migration notes must explicitly separate:

- board-native bring-up path
- NECCS target-feature adaptation path

### 13.5 Practical next step after this document update

The most sensible next actions are now:

1. inspect the vendor `01_LED` and `05_Serial` examples for `ATK-CNN647B`
2. inspect the vendor FSBL example
3. inspect the vendor external loader example
4. align the new project with the `ATK-CNN647B` memory devices and boot flow
5. only then decide whether the NECCS base should start from:
   - a blank CubeMX project
   - or a vendor example fork

## 14. 2026-06-02 Boot Image Rebuild And Cold-Boot Notes

Do not rely on Debug build output for cold boot. The working cold-boot images must come from Release builds:

- FSBL Release binary is signed by `STM32_SigningTool_CLI` with header magic `STM2` and converted to Intel HEX at `0x70000000`.
- 01_LED Appli Release binary is converted to Intel HEX at `0x70100400`.
- Unified flash images are stored in `_flash_images`:
  - `_flash_images/fsbl.hex`
  - `_flash_images/appli_01_LED.hex`

Use this PowerShell script to prepare both images without depending on Git Bash or `postbuild.sh`:

```powershell
.\tools\rebuild_n647_boot_images.ps1
```

As of 2026-06-03, the script still defaults to the vendor prebuilt FSBL as the safe baseline:

- `_flash_images/fsbl.hex` is copied from `_vendor_fsbl_ref/Binary/fsbl.hex`.
- `_flash_images/appli_01_LED.hex` is rebuilt from the local `01_LED` Release application.

Confirmed root cause for the locally rebuilt FSBL cold-boot failure:

- The current local FSBL source and `.ioc` match the vendor original source.
- The board can cold-boot with the vendor prebuilt `fsbl.hex`, so hardware, external NOR, EML, boot pins, application image address, and flashing process are good.
- With STM32 Signing Tool v2.22.0, `-hv 2.3` does not align the FSBL payload to offset `0x400` unless `-align` is explicitly passed.
- Bad local signed image: vector table appeared at signed-image offset `0x240`.
- Good vendor image: vector table appears at signed-image offset `0x400`.
- Fix applied: add `-align` to FSBL signing in `tools/rebuild_n647_boot_images.ps1` and the CubeIDE FSBL `postbuild.sh`.

To intentionally test the locally rebuilt FSBL, run:

```powershell
.\tools\rebuild_n647_boot_images.ps1 -FsblSource Rebuild
```

The script copies the final images into `_flash_images`, and checks:

- `fsbl.hex` starts at `0x70000000` and contains `53544D32` (`STM2`) at the start.
- For rebuilt FSBL, the signed image must contain the FSBL vector table at payload offset `0x400`; in flash this is `0x70000400`.
- `appli_01_LED.hex` starts at `0x70100400`.

Current cold-boot rule for ATK-CNN647B:

- Development/debug boot: `BOOT0 = GND`, `BOOT1 = 3.3V`.
- External flash boot: `BOOT0 = GND`, `BOOT1 = GND`.
- After programming, disconnect the debugger/programmer if needed, set both boot pins for flash boot, then press `nRST` or power-cycle the board.

Important diagnostic conclusion:

- If FSBL external-memory initialization is disabled, debug may still appear to work because the loader/debugger has already configured external memory.
- That is not a valid cold-boot fix.
- Final FSBL must keep `MX_XSPI2_Init()`, `MX_EXTMEM_MANAGER_Init()`, HyperRAM init, and `BOOT_Application()` enabled so the board can boot without debugger help.

## 15. 2026-06-03 Next Stage: HyperRAM And RGB LCD Bring-Up

Audio acquisition hardware is not fully soldered yet, so the next migration stage should focus on board-native memory and display bring-up first.

Use the local ALIENTEK software package as the concrete reference, not assumptions:

- RAM/display reference example:
  - `【正点原子】N647开发板资料盘(A盘)/4，程序源码/SoftwarePackage/Projects/15_RGBLCD`
- Image/display stress reference:
  - `【正点原子】N647开发板资料盘(A盘)/4，程序源码/SoftwarePackage/Projects/41_Picture_Show`
- BSP drivers:
  - `【正点原子】N647开发板资料盘(A盘)/4，程序源码/SoftwarePackage/Drivers/BSP/HyperRAM`
  - `【正点原子】N647开发板资料盘(A盘)/4，程序源码/SoftwarePackage/Drivers/BSP/RGBLCD`

Recommended order:

1. Bring up HyperRAM only.
2. Run a small memory self-test at the HyperRAM mapped address.
3. Add LTDC + DMA2D + RGBLCD only after RAM is proven stable.
4. Put the LCD framebuffer into `.EXTRAM`.
5. Test screen with solid colors or color bars before adding touch, images, LVGL, or audio UI.

Key HyperRAM facts from `15_RGBLCD`:

- Device: `W958D8NBYA5I`
- Interface: `XSPI1`, Port1, HyperBus mode.
- Mapped address used by the linker scripts: `0x90000000`.
- Clock source in the reference `.ioc`: `RCC_XSPI1CLKSOURCE_IC4`.
- Reference XSPI1 frequency: `200 MHz`.
- Reference XSPI1 settings:
  - `MemorySize = HAL_XSPI_SIZE_256MB`
  - `ChipSelectHighTimeCycle = 2`
  - `ClockPrescaler = 1 - 1`
  - `AccessTimeCycle = 7`
  - `RWRecoveryTimeCycle = 7`
  - `LatencyMode = HAL_XSPI_FIXED_LATENCY`
  - `WriteZeroLatency = HAL_XSPI_LATENCY_ON_WRITE`
  - `WrapSize = HAL_XSPI_WRAP_32_BYTES`

Important initialization order from the reference app:

```c
MX_XSPI1_Init();
HyperRAM_Init(&HyperRAMObject, &hxspi1);
HyperRAM_EnableMemoryMappedMode(&HyperRAMObject);
MX_GPIO_Init();
MX_DMA2D_Init();
MX_LTDC_Init();
rgblcd_init();
```

Key RGB LCD facts from `15_RGBLCD`:

- Reference panel timing starts with `480 x 272`, RGB565.
- LTDC clock is about `9 MHz` for the 480 x 272 panel ID path.
- `DMA2D` is configured as memory-to-memory with pixel format conversion, output RGB565.
- Backlight GPIO is `PA3`, active high in the BSP macro `RGBLCD_BL(1)`.
- The BSP framebuffer is:

```c
uint16_t g_ltdc_lcd_framebuf[1280 * 800] __attribute__((section(".EXTRAM")));
```

Therefore the application linker script must define `.EXTRAM` in HyperRAM. The reference linker script `STM32N647X0HXQ_LRUN_RAMxspi1.ld` maps it as:

```ld
EXTRAM (rw) : ORIGIN = 0x90000000, LENGTH = 32M

.EXTRAM (NOLOAD) :
{
  *(.EXTRAM)
} >EXTRAM
```

Do not judge LCD bring-up until these three things are all true:

- HyperRAM self-test passes from software after cold boot.
- `.EXTRAM` is mapped to `0x90000000`, not internal SRAM.
- `rgblcd_init()` can turn on the backlight and set the LTDC framebuffer address.

Expected first acceptance tests:

- RAM stage passes a deterministic pattern test over a small range, then a larger range.
- LCD stage shows stable full-screen red/green/blue/white/black fills.
- Only after that, move to picture display, touch, or UI framework integration.

## 16. 2026-06-08 N657 IOC To N647 APP IOC Baseline

Target IOC:

- `D:\Project\NECCS\N647_BaseBoard\firmware\NECCS_N647\NECCS_N647_App\NECCS_N647_App.ioc`

Reference IOC:

- `D:\Project\NECCS\STM32N6开发板\CubeMX\CubeMX.ioc`

This stage migrates the N657 application peripheral intent into the N647 APP IOC, but only where the N647 pinout/examples make the configuration confirmable.

Kept from the already verified N647 APP baseline:

- APP-only project structure: `Appli=true`, `FSBL=false`, `ExtMemLoader=false`.
- `XSPI1` HyperRAM on Port1 HyperBus, mapped by linker scripts to `0x90000000`.
- `LTDC` + `DMA2D` display path, RGB565.
- `PA3` LCD backlight, `PE10`/`PG10` LEDs.
- No `XSPI2` in APP. External NOR flash remains FSBL-owned.

New IOC configuration added for the formal APP baseline:

- `SAI1` follows the N657 microphone-array design exactly:
  - `PB0 = SAI1_FS_A`
  - `PB6 = SAI1_SCK_A`
  - `PB7 = SAI1_SD_A`
  - `PE3 = SAI1_SD_B`
  - `SAI_A` and `SAI_B` are both asynchronous slave blocks.
- `USART1`: `PE5 = TX`, `PE6 = RX`, asynchronous mode.
- `I2C2`: `PD14 = SCL`, `PD4 = SDA`, timing `0x10707DBC`, matching the N647 camera reference.
- `SDMMC2`: `PC0/PC3/PC4/PC5/PD2/PE4`, 4-bit bus, clock divider `4`, hardware flow control enabled.
- `CSI` + `DCMIPP`: enabled using the N647 MIPI camera reference topology, with `DCMIPP_CSI` and Pipe 1 virtual signals.
- `GPDMA1` remains present in the APP context as preparation for audio/camera data paths, but no concrete DMA request/channel is bound yet.
- N657 GPIO labels were migrated when the same physical pin is free on the N647 APP baseline. This includes functional GPIOs such as `USB1_EN`, `BOOT1`, `MIC_SHDNZ`, `CAM_EN_MODULE`, `UCPD1_INT`, `USER1`, `LCD_NRST`, `USER2`, `CAM_LED_EN`, `CTP_RST`, `LCD_BL_PWM`, `SPI2_CS0`, `EXT_SMPS_MODE`, `SD_DET`, `UCPD1_VSENSE`, `LED1`, `CTP_INT`, `PWR_SD_EN`, and the non-conflicting `EXP_GPIO_*` / `EXP_ALT_ETH_*` expansion labels.
- `PG10` is kept as a GPIO output and labeled `LED2` to match the N657 naming.

Important N657-to-N647 differences:

- `SAI1`: deliberately uses the N657 mic-array pin map, not the N647 ALIENTEK audio-recorder example pin map.
- `LTDC`: N657 used RGB888 intent; N647 APP currently stays on RGB565 because the ATK-CNN647B display/BSP path has already been verified that way.
- LCD resolution is set to `1024 x 600` for the current panel. IOC timing matches the BSP panel path: `hsw=20`, `vsw=3`, `hbp=140`, `vbp=20`, `hfp=160`, `vfp=12`, LTDC clock `40 MHz`.
- `XSPI2`: present in the boot chain but intentionally absent from APP configuration. FSBL initializes external NOR and boots APP from `0x70100400`.
- `USB1_OTG_HS`: not enabled yet. Add it only after Device/Host role and connector wiring are confirmed.
- `I2C1`, `SPI1`, `SPI2`, `USART2`, and `SAI2`: present in the N657 IOC intent but not enabled in the N647 APP IOC until their board-level usage is confirmed.
- Camera configuration keeps the N657 DCMIPP/CSI intent but uses the N647 camera example's MIPI CSI/DCMIPP shape instead of copying N657 labels blindly.
- Newly added peripheral init functions are kept disabled in `ProjectManager.functionlistsort` for now. This prevents SD card, camera, or microphone hardware readiness from breaking the already verified LCD/HyperRAM boot path after code generation.
- N657 GPIO labels not migrated because the same N647 physical pin is already occupied by a higher-priority validated function:
  - `PA10 / EXP_GPIO_PA10`: occupied by `LTDC_B4`.
  - `PA11 / USB1_OCP`: occupied by `LTDC_B3`.
  - `PA3 / EXP_GPIO_PA3`: kept as the N647 LCD backlight GPIO output.
  - `PA5 / UCPD1_ISENSE`: occupied by `LTDC_CLK`.
  - `PB10 / SPI1_CS0`: occupied by `LTDC_G7`.
  - `PB4 / EXP_ALT_ETH_PB4`: same physical pin as `PB4(NJTRST)`, occupied by `LTDC_R3`.
  - `PE10 / EXP_GPIO_PE10`: kept as the N647 LED GPIO output.
  - `PE5 / EXP_GPIO_PE5`: occupied by `USART1_TX`.
  - `PF9 / EXP_GPIO_PF9`: occupied by `LTDC_HSYNC`.
  - `PG13 / EXP_GPIO_PG13`: occupied by `LTDC_DE`.
  - `PG9 / EXP_GPIO_PG9`: occupied by `LTDC_R7`.
  - `PO5 / SD_LDO_SEL`: occupied by `XSPIM_P1_NCLK` for HyperRAM.

Regeneration checklist:

- Open the IOC in CubeMX and confirm there are no red pin conflicts.
- Generate code, then confirm APP code still does not contain `MX_XSPI2_Init()`.
- Rebuild Debug and Release.
- Verify LCD/LED/HyperRAM behavior first before enabling SAI, SDMMC2, CSI/DCMIPP, or USART startup calls.

## 17. 2026-06-20 APP Build Recovery And SAI Baseline Audit

Workspace/build recovery:

- The active repository is `D:\Project\NECCS\N647_BaseBoard\firmware\NECCS_N647`; the old `Program\NECCS_N647` path is no longer the source of truth.
- A stale CubeIDE workspace can retain generated makefiles with the previous absolute project path even though the tracked Eclipse links are relative.
- Use `tools\build_n647_app.ps1` to import/refresh the project and clean-build both configurations from any repository location.
- The script auto-detects the newest `C:\ST\STM32CubeIDE_*` installation. Set `STM32CUBEIDE_ROOT` or pass `-CubeIdeRoot` when CubeIDE is installed elsewhere.
- The Debug linker script now emits separate RX and RW ELF load segments instead of one RWX segment.
- Verified on 2026-06-20:
  - Debug: `0 errors, 0 warnings`.
  - Release: `0 errors, 0 warnings`.
  - Release postbuild regenerated `NECCS_N647_App\Binary\appli.hex`.

SAI baseline audit against the N657 IOC:

- Confirmed equal: `PB0=SAI1_FS_A`, `PB6=SAI1_SCK_A`, `PB7=SAI1_SD_A`, `PE3=SAI1_SD_B`.
- Confirmed equal: Block A and Block B are both configured as asynchronous slaves.
- The N657 IOC contains only the SAI instance/virtual-mode selection. It does not define the final TDM16 data size, frame length, slot mask, clock edge, or DMA channel.
- The computed SAI1 kernel clock differs because the complete board clock trees differ: N657 IOC reports `32 MHz`; the validated N647 display/HyperRAM clock tree reports `200 MHz`.
- Do not replace the complete N647 clock tree merely to copy that computed value. Final SAI framing and DMA must be derived from the microphone-array electrical design and PCMD3180 timing requirements.
- SAI/I2C/USART/SDMMC/DCMIPP init calls remain intentionally disabled in the APP startup path until each peripheral receives an isolated on-board smoke test.

Next bring-up gate:

- First rerun the existing cold-boot LCD/LED/HyperRAM test with the regenerated `appli.hex`.
- After that passes, bring up the microphone control path and SAI DMA as a dedicated step: confirm PCMD3180 clock ownership, define TDM16 framing, bind GPDMA, and verify raw samples before migrating audio algorithms.
- `tools\rebuild_n647_boot_images.ps1` now builds the current `NECCS_N647_App`, not the legacy vendor `01_LED` application, and stages validated images as `_flash_images\fsbl.hex` plus `_flash_images\appli.hex`.
