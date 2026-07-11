# NECCS N647 声学成像仪 — Agent 工作手册

嵌入式竞赛项目(STM32N647 + TouchGFX + ThreadX)。板子通过 ST-LINK(SWD)+
H7 继电器控制器(COM3)全远程可控。**按本手册操作,不要自由发挥。**

## 一、绝对规则(违反会破坏已验证的系统)

1. **所有板子操作只用一条命令入口**:`.\tools\debug\n647.ps1 <action>`
   (在 `D:\Project\NECCS\Program\NECCS_N647` 下运行)。不要手写 OpenOCD/GDB
   /CubeProgrammer 命令行,不要发明新流程。
2. **绝不用 GDB `load` 烧 Release/XIP**。Release 只走
   `n647.ps1 flash-release`(内部用 CubeProgrammer + 官方外部 loader)。
3. **绝不在 CPU 代码里访问 DBGMCU 寄存器**(`DBGMCU->CR` 等)。冷启动时
   BootROM 锁着调试域,该访问总线错误 → 启动锁死(2026-07-10 实证,
   BFAR=0x54001004)。调试口解锁已在 `SystemInit()` 顶部用 BSEC 实现,勿动。
4. **勿改这些已板级验证的常量**(除非有新的上板证据):
   - `app_pcmd_capture.c` 的 RESET/SETTLE/CLOCK 时序常量(改快过一次,
     32 通道 TDM 直接哑掉);`defer_power_up=0`、`verify_writes=1`、
     `enable_micbias=0`(金标准 7e12d5da 流程)。
   - PCMD 配置后**绝不停 SAI DMA**(BCLK 断 → PCMD3180 PLL 失锁全哑)。
   - I2C2 timing `0x10707DBC`(100kHz;总线现有 4.7K 外部上拉)。
   - `MX_I2C2` 总线由相机+触摸+PCMD 共享,PCMD 配置窗口必须持有
     `AppI2C2_Lock`。
   - FileX 缓存必须留在内部 SRAM(64KB EXTRAM 版本会导致重启后文件
     扫不到,已回滚,原因未明,见 app_media.c 注释)。
5. **每次重新编译后符号地址会变**。读内存一律用 GDB 符号名(带文件限定,
   如 `'app_pcmd_capture.c'::s_snapshot`),或先用 `arm-none-eabi-nm` 查地址。
   绝不复用旧地址。
6. RAM Debug 构建断电即失;继电器断电重启后必须重新 `flash-debug`。
7. 编译前先杀掉残留 CubeIDE 进程:`Get-Process java,javaw | Stop-Process -Force`
   (n647.ps1 build 已内置)。
8. 提交信息里写清"板上验证了什么";没上板验证的改动要在提交里注明。

## 二、标准工作循环

```powershell
cd D:\Project\NECCS\Program\NECCS_N647

# 开发循环(编译+烧+跑,一条命令)
.\tools\debug\n647.ps1 flash-debug            # 含编译
.\tools\debug\n647.ps1 flash-debug -SkipBuild # ELF 已是最新时

# 看板子活没活、各子系统健康(必须学会读这个输出)
.\tools\debug\n647.ps1 status

# 截屏看 UI(输出 PNG 路径,用读图工具看)
.\tools\debug\n647.ps1 screenshot

# Release 验证
.\tools\debug\n647.ps1 flash-release          # 烧外部 Flash
.\tools\debug\n647.ps1 xipboot                # 冷启动进 Release
.\tools\debug\n647.ps1 status                 # XIP 下也能读(SWD 已解锁)
.\tools\debug\n647.ps1 devboot                # 回调试模式
```

`status` 健康判据:`uptime2 > uptime`(活着)、pcmd `present=0xf cfg_ok=0xf
started=1 wdog=0`、camera frames 增长、npu `status=0`、media `err=0`。

## 三、板子/硬件事实

- **启动模式**:BOOT1 由 H7 的 PA13 控制(经 COM3 串口命令)。
  debug boot = BOOT1 高(BootROM 等待,可烧写);XIP boot = BOOT1 低
  (跑外部 Flash 里的 Release)。H7 的 PA13 同时是它自己的 SWDIO——
  正常不用管,若需 J-Link 调 H7,先发 `boot1 hiz`。
- **麦阵**:32 麦 / 4×PCMD3180,I2C 在 PD14/PD4(共享 I2C2,4.7K 上拉),
  A 总线槽 14 那颗麦硬件损坏(恒零,已知,别当固件 bug 查)。
- **屏幕**:原 7 寸屏总成损坏(GT911 配置表丢失 + 背光 RT9293 烧毁,
  等换新);当前接的是 4.3 寸临时屏,显示不全属正常,触摸功能正常。
- **相机**:IMX219,DCMIPP PIPE1,ISP 链(黑电平/gamma/AWB/AE)已开。
- **NPU**:Neural-ART 已跑通,378µs/次推理;权重在 npuRAM5(调试构建,
  blob 编译进固件);工具链见 `tools/npu/`(采集→训练→导出→stedgeai)。
- **已知遗留**:录像偶发写坏帧(录制侧,证据在 de163ffb 提交信息);
  冷启动间歇冻结疑似 5V 轨上烧毁的 RT9293 拖累(摘除后复测)。

## 四、目录地图

- `Program/NECCS_N647/NECCS_N647_App/Appli/Core/Src/` — 应用层
  (app_pcmd_capture 采集、app_acoustic_* 定位、app_camera* 相机、
  app_media 图库、npu/app_npu NPU、app_bringup_thread 启动编排)
- `.../Appli/TouchGFX/gui/` — UI(Model.cpp 数据桥、TemplateView.cpp 全部页面)
- `Program/NECCS_N647/tools/debug/` — 板子操作脚本(n647.ps1 为总入口)
- `Program/NECCS_N647/tools/npu/` — NPU 模型管线
- `Program/NECCS_N647/docs/` — PROJECT_STATUS.md(当前状态)、设计文档
- `Program/NECCS_N647/_debug_logs/` — 日志/截图(git 忽略)
- H7 继电器固件:`D:\Project\NECCS\H7_Original\firmware\NECCS\START`
  (Keil 工程,改完用 `C:\Keil_v5\UV4\UV4.exe -b` 编译,J-Link 烧)

## 五、验证纪律(每个改动都要走)

1. 编译零错误零警告(新警告 = 修掉)。
2. `flash-debug` 后 `status` 全绿。
3. 涉及 UI 的改动:`screenshot` 肉眼确认。
4. 涉及采集/定位的改动:对比改动前后 `status` 里 pcmd 各计数。
5. 提交;`git log` 保持一行式规范(`fix(pcmd): ...`)。
6. 大改动或发布前:`flash-release` + `xipboot` + `status` 冷启动回归。

## 六、UI 改动专项(先读 docs/UI_DESIGN_SYSTEM.md)

UI 全部手写在 `TemplateView.cpp`,没有 Designer。流程:

1. **改前**:读 `docs/UI_DESIGN_SYSTEM.md` 的设计原则(每页一个主角、
   层级靠亮度、彩色只表达含义、对齐节奏)。默认做法是模仿最像的
   现有元素;想创新也可以,创新后用基准图检验。
2. **中文**:新增任何中文字符后跑
   `python tools/ui/check_text_glyphs.py`,0 缺字再烧录
   (缺字不报错,直接在屏幕上渲染成空白)。
3. **改后**:`n647.ps1 uitour` 自动翻全部 5 页逐页截图,和
   `docs/ui_reference/` 基准图并排对比,过"自查五问"
   (主角、对齐、彩色含义、整体感、中文完整)。
   改得更好就更新基准图;说不清好坏就回退。
