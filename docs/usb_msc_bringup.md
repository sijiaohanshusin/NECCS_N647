# N647 USB 设备(MSC 大容量存储)调通记录 — 2026-07-20

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

注:第一次联调时 Windows 曾经由坏传输路径**写入**过 77 个扇区,把卡上
历史文件的 FAT 链写坏(后被 chkdsk /F 截断回收进 FOUND.000)。当时的
测试素材已丢,属一次性损失;修复后的写入路径经跨重挂往返验证无损。

## 四、验收结果(2026-07-20 板上)

- 枚举:`USB\VID_0483&PID_5720\NECCS0000001`,FAT32 卷自动挂载(E:)。
- 读:1.8MB 截图 BMP 整文件拷出,解码 1024×600 无损(视觉核对通过)。
- 写:PC 写 `NECCS/USBTEST.TXT` → 退出 USB 模式(FileX 重挂)→ 再进 →
  内容原样,目录结构完好。
- 退出后板端:`status` 全绿,截图/媒体命令恢复,声学自检 PASS
  (7.0, -9.2),Debug+Release 双构建 0 错 0 警。
- 吞吐:FS 模式实测 **~9 KB/s**(USB IRQ 速率钉在 ~100/s ≈ tick 频率;
  类线程优先级 9 与 12 无差别 → 非线程饥饿,疑似 FS 路径的每传输
  turnaround,与 HS 升级一并追)。小文件够用,大录像建议先用 HS 修复
  或继续走 GDB pull。

## 五、后续(验收另开)

1. **HS 480Mbps**:OTG1 数据线改短直连(去继电器模块串接/跳线),改回
   `PCD_SPEED_HIGH`,吞吐应到 20-35 MB/s 量级,顺带解决 9KB/s 问题。
2. 复合设备:+CDC-ACM(串口遥测/调参)。
3. UAC 2.0:波束音频 = USB 定向麦克风(官方 Ux_Device_Audio_2.0 可抄)。
4. UVC:热图叠加画面 = USB 摄像头(Ux_Device_Video / VENC_USB 参考)。
