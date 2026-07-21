# N647 USB 设备(MSC 大容量存储)调通记录 — 2026-07-20(吞吐优化 07-21)

PC 插 OTG1 Type-C 后,设备枚举为「NECCS Acoustic Imager」大容量存储;
进入 USB 模式后 SD 卡整卡以 FAT32 呈现给 PC(录音 WAV / 录像 AVI / 截图
BMP 直接拷贝),退出后板端 FileX 重挂、录制功能恢复。

## 一、硬件事实(底板网表核实)

- OTG1 = `USB1_OTG_HS`(内嵌 HS PHY),D+/D- 在专用球 A5/B5,经底板
  **P8 跳线**(P8-1↔P8-3, P8-2↔P8-4)到 Type-C `USB_SLAVER1`。
- **VBUS/ID 均未接**(NC)→ 固件强制 device 模式、关 VBUS 检测,无法感知
  插拔;PCD 常开,靠主机侧动作。
- USB 5V → 保险丝 → K1 → `VCC5` 直接喂整板。**H7 继电器(5V 轨)+ USB
  串口继电器(VBUS 线)都断才能真正断电**。
- 供电纪律已进 `n647.ps1`:`devboot/xipboot/flash-*` 前自动断 USB 5V
  (LCUS A0 协议, CH341/PID_5523 自动探测);`usb5v-on|off|status` 手动控制。
  H7 控制器同样改为按 PID_7523 自动探测(COM 号漂移免疫)。
- **平时保持 USB 5V 断开**:数据不需要 VBUS,且开着会让断电复位失效。

## 二、固件栈

- USBX(FW N6 V1.3.0 拷入 131 个源文件子集):core + device stack +
  storage class + ST DCD;`.project` 三个文件夹链接,`.cproject` 4 处
  include + `UX_INCLUDE_USER_DEFINE_FILE` define + sourceEntries 白名单
  (**漏加 sourceEntries 时链接器报 _ux_ 全部未定义**)。
- `app_usb_device.c`:PHY/时钟按 DK Ux_Device_MSC 配方;服务线程等 SD
  容量就绪后注册 MSC 并 `HAL_PCD_Start`;快照 `s_snapshot`
  (init/cfg/exposed/rd/wr/ioerr/last)供 GDB/UI。
- MSC 桥:裸块直通 `sd_nand_read/write_disk`——FileX 驱动本就是
  逻辑扇区 n = 物理 LBA n(superfloppy),PC 看到的就是 FileX 卷本体。
- 独占切换(相机式):`AppMedia_RequestUsbMode(1)` → 媒体线程停录/关播/
  `fx_media_flush+close` → LUN 报告在位;期间媒体命令一律拒绝
  (`APP_MEDIA_ERROR_USB_MODE`),record/play/beam tick 停转。退出 →
  重挂+重扫。UI:系统页「USB 存储」按钮(激活时琥珀色「退出 USB」);
  GDB 钩子 `g_app_media_usb_request`(1=进 2=出)。

## 三、上板踩坑实录(都是真实烧板得出的)

| # | 症状 | 根因 | 修复 |
|---|---|---|---|
| 1 | `HAL_PCD_Init` 卡死,GRSTCTL.CSRST 永不清零 | **应用时钟树从未启动 HSE**(全跑 HSI/PLL),USB PHY 参考钟(HSE/2=24MHz)是死的 | MspInit 里补 `HAL_RCC_OscConfig(HSE_ON)`(失败退 BYPASS) |
| 2 | PC 完全看不到设备(板侧 DSTS 一直 suspend) | **480MHz HS chirp 过不了继电器模块+跳线链路** | 降 `PCD_SPEED_HIGH_IN_FULL`(FS 12Mbps);布线整洁后再回 HS |
| 3 | 全系统冻结,只有 storage 线程在跑;后演变为 INVSTATE HardFault,PSP 指进 UX 池 | UX 池(含类线程栈)放在 **EXTRAM/HyperRAM** | UX 池收回内部 RAM(12KB) |
| 4 | 枚举成功但盘上目录乱码,chkdsk 报大量丢失簇;FAT dump 里出现 0x909A1D90 等**池地址** | 传输缓冲放 EXTRAM cache 池同样出错(读出垃圾扇区),与 app_media 注释的 FileX-cache-EXTRAM 疑难同族 | 单一内部池;**EXTRAM 禁止承载 USBX 任何内存** |
| 5 | USBX 类线程 run_count=0 永不运行 | 默认优先级 20 <(数值大于)声学 13 常驻满载线程 | `UX_THREAD_PRIORITY_CLASS=12`(媒体层) |
| 6 | Windows「该设备无法启动」(code 10) | 坑 3/4 崩溃后主机放弃;修复后软断开重连即恢复 | 无需额外代码;调试期可 GDB 置 DCTL.SDIS 触发重枚举 |
| 7 | 吞吐 9 KB/s(FS/HS 完全一样,IRQ 速率钉在 tick 频率) | **`sd_nand_init()` 每次读写回调都执行**,里面 `HAL_Delay(10)` 等 SD 电源轨稳定 = 每 1KB 白等 ~110ms(DWT 实测 SD 平均 109.7ms/次) | 快路径:已初始化直接 return;上电稳定延时只在真正重新 init 时走(读 9.1→1240 KB/s,136 倍) |
| 8 | 提升 MSC 分块到 16KB 后端点缓冲放不下内部 RAM(剩 <5KB) | 每端点缓冲 = `UX_SLAVE_REQUEST_DATA_MAX_LENGTH`,3 端点 ×16KB;EXTRAM 又会坑 4 | **npuRAM3**(AXISRAM3, 0x34200000, 448KB, 片上 AXI SRAM)作 USBX cache-safe 池:声纹网络只占 npuRAM4/5,npuRAM3 空闲 |
| 9 | 切到 npuRAM3 池后主机读到全随机垃圾,MSC 线程死循环 PHASE_ERROR(CBW 全是乱码) | npuRAM3 上电除了时钟门控还带 **RAMCFG SRAMSD(shutdown)**:写丢弃、读随机,D-cache 还会掩蔽到逐出才暴露 | StackInit 里按 app_npu 同款流程唤醒:`AXISRAM3_MEM_CLK_ENABLE` + 清 `RAMCFG_CR_SRAMSD` + 等 SRAMBUSY |

注:第一次联调时 Windows 曾经由坏传输路径**写入**过 77 个扇区,把卡上
历史文件的 FAT 链写坏(后被 chkdsk /F 截断回收进 FOUND.000)。当时的
测试素材已丢,属一次性损失;修复后的写入路径经跨重挂往返验证无损。

## 四、验收结果(2026-07-20 首验;2026-07-21 HS+吞吐复验)

- 枚举:`USB\VID_0483&PID_5720\NECCS0000001`,FAT32 卷自动挂载(E:)。
  OTG1 直连(去掉继电器模块串接)后 **HS 480Mbps 枚举成功**
  (`PCD_SPEED_HIGH`,总线报告 High-Speed)。
- 吞吐(2026-07-21,HS + 16KB 分块 + SD 快路径):
  - 裸盘顺序读 16MB:**3.7 MB/s**(优化前 9.1 KB/s,×400)
  - 文件读 1.8MB BMP:**3.6 MB/s**,MD5 与卡上一致
  - 文件写 8MB:**1.7 MB/s**(优化前 128 KB/s),MD5 往返一致
- 读写完整性:8MB 随机数据写→读回 MD5 相同;1.8MB BMP 拷出 MD5 相同。
- 退出后板端:`status` 全绿(pcmd 0xf/0xf、32 槽、npu 正常),媒体重挂
  (mounted=1),录制功能恢复。
- Release:编译 0 错 0 警,bundle 烧写校验通过,XIP 冷启动存活
  (tick 前进、TouchGFX vsync 前进、USB 服务线程 init=1、声学线程满载)。

## 五、遗留与整体优化候选(记录,后续统一做)

1. **写入路径还有 ~2 倍空间**:写 1.7MB/s vs 读 3.7MB/s,瓶颈是
   `HAL_SD_WriteBlocks` 同步轮询 + SD NAND 页编程延迟。候选:SDMMC IDMA
   双缓冲(读 SD 与 USB 发送流水线化)、`UX_DEVICE_CLASS_STORAGE` 双缓冲
   模式。属跨模块改动(sd_nand 驱动被 FileX 共用),放整体优化。
2. **OTG DMA**:当前 `dma_enable=DISABLE`(PIO,每 512B 包进一次 ISR)。
   开 DMA 可把 CPU 从拷包里解放(声学线程受益),但 N6 的 OTG DMA +
   cache 一致性要专门验证,单独开验收。
3. **`n647.ps1 status`/`screenshot` 在 XIP 下读不到**:工具固定用 Debug
   ELF 符号地址,Release 布局不同 → 全零/黑图。候选:按 halt 时 PC 段
   (0x3400xxxx vs 0x7010xxxx)自动选 ELF。本次 Release 验活用的是
   release ELF 手动读 tick/vsync。
4. **GDB USB 钩子 `g_app_media_usb_request` 是 DEBUG-only**:Release 下
   远程验收 MSC 需要人点屏幕。候选:钩子放开到 Release(只是个 volatile
   轮询,无成本)。
5. 复合设备:+CDC-ACM(串口遥测/调参)。
6. UAC 2.0:波束音频 = USB 定向麦克风(官方 Ux_Device_Audio_2.0 可抄)。
7. UVC:热图叠加画面 = USB 摄像头(Ux_Device_Video / VENC_USB 参考)。

## 六、吞吐调优方法论(下次照抄)

1. **先测不猜**:DWT cycle counter 包住可疑段(本次 `StorageRead` 里包
   `sd_nand_read_disk`,`sd_read_us_avg=109722` 一眼定罪 10ms 延时)。
2. 每层单独归因:SD 层(DWT)、USBX 层(read_calls vs read_blocks 推分块
   大小)、主机层(PowerShell 裸盘 FileStream 读,绕过文件系统缓存)。
3. 分块大小 = `UX_SLAVE_REQUEST_DATA_MAX_LENGTH`(它同时是端点缓冲和
   MSC SCSI 分块);16KB 后每命令 SD 耗时 3.2ms(32 块突发),摊薄后
   SD 侧 ~5MB/s,与实测 3.7MB/s(含 USB/调度开销)吻合。
