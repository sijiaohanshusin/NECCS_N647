# 项目状态交接 — 2026-07-12

> 给接手的任何人/任何模型:先读根目录 `AGENTS.md` 的操作规则,再读本文。
> 板子操作只用 `tools\debug\n647.ps1`。

## 当前基线

- 分支:`main`(`ec6e31e9` 合并了 pcmd-stable-debug 的全部修复)
- 板上:Debug 构建(RAM),外部 Flash 里是修好锁死问题的 Release bundle
- 一切远程可控:COM3 = H7 继电器/BOOT1,ST-LINK = N6 SWD,J-Link = H7

## 已完成且板级验证(勿回退)

| 领域 | 状态 | 关键提交 |
|---|---|---|
| 麦阵 PCMD | 硬件 I2C2 一次配置成功,31/32 麦有效,187fps 零看门狗 | 692a006f |
| XIP 冷启动 | 锁死根因(main 里 DBGMCU 访问)已除;SystemInit BSEC 解锁使 XIP 可调试 | f1512157 |
| 相机画质 | ISP 链全开 + AE/AWB 伺服;亮场景收敛待实验室验证 | a051467a |
| 图库 | DMA2D 硬件解码、点击即显、解码自愈、扫描 O(logN) | 641b8c51..de163ffb |
| NPU | 真模型上线:6类声纹 CNN 全部 6 epoch 纯硬件,381µs/次,播放实测 5/5 类正确;UI 分类卡由 NPU 输出驱动 | fc751a5b, 7161be94, 80c2aa61 |
| 192k 超声 | Core16@192k 无晶振实现(PLL2/IC7 重配),参数页可切换,切换循环稳定 | a0ae74f4, 03b382b3 |
| 定向录音 | 独立"声源录音"页面(导航第2项):大录制键+自动/点选+方位表盘+电平表+滚动电平史,相机窗点按/拖拽瞄准,准星三态;录制跨页面持续(顶栏蓝点);4kHz 扫角前后比 ~9dB,独录零丢帧,与录像互斥;WAV 可经 `tools/debug/pull_audio.ps1` 拉到 PC | 54db7c68, ea532ca5 |
| 云台/激光预备 | 引脚按网表定稿并同步 .ioc:PE13/PE14=TIM1_CH3/CH4 舵机 PWM、PC7=GIMBAL_PWR_EN(ENB1, 7.4V 轨)、**PC10=LASER_EN(ENB2, U6 3.3V 激光轨门控)**、PD0=GIMBAL_RELAY_EN(均上电低/灭);SetLaser 自动连带抬 7.4V 轨(轨引用计数);配套硬件改动清单见 docs/gimbal_laser_pinmap.md §四;标定与 UI 开关等硬件到货 | 本次 |
| 声源定位重构 | "高速率估计+跟踪器"架构落地:滞后域 GCC 引擎(88M→16M cycles/帧, SRP 4.5→15-20fps@叠加)、互谱跨帧累积、坏麦 M31 掩码、B24 降级修复、SRP 线程 13 级时间片、app_acoustic_tracker(Kalman+关联制置信度+瞬态快路径)取代 Model 双帧锁/2s 冻结、场 EMA 时间常数化+置信度淡入淡出;合成自检 GDB 钩子 PASS(顺带修滞后窗角点尺寸与合成 dy 镜像两个存量 bug);详见 docs/acoustic_refactor.md,现场声源验收清单待跑 | 本次 |
| 频带双模式(仿福禄克) | 根因:泄漏嘶声 88% 能量在 8kHz+ 而分析带只到 7.9k,带内空 bin 经 PHAT 白化后淹没信号 bin(气体泄漏.mp4 定位差的直接原因)。四层修复:1) SRP 按 bin SNR 门控 PHAT(空 bin 降权,带宽混合保单音,绝对电平+streak 迟滞防静音误开);2) 自动频带:服务层监视全谱(显示谱 64→96 bin=18kHz),能量聚集持续 ~0.7s 即把分析带挪过去,静默 ~5s 回落场景预设,追带上限 bin42(>8k 为 94mm 阵列栅瓣区,板证:9.7-16.7k 窗自信地锁死在错误方位,长/短基线对都不行——超声检测走 Core16);3) 手动频带:福禄克手势(拖边缘调宽窄/拖中间整体平移),范围放开到 18kHz,参数页"频带"行点按切换自动/手动;4) 泄漏场景预设 27..64。板验:自检 PASS、静音全黑、泄漏音频 5-8k 带内锁定、2k 单音 (-4,-5)±2° 稳定 | 本次 |
| USB 大容量存储(OTG1) | USBX+MSC 调通:PC 插 OTG1 枚举为「NECCS Acoustic Imager」,系统页「USB 存储」一键把 SD 整卡借给 PC(FileX 关卷、媒体命令拒绝),PC 直读录音/录像/截图,退出即重挂恢复。踩坑修复:应用时钟树没开 HSE(PHY 参考钟死)、HS chirp 过不了继电器链路(暂降 FS 12Mbps)、USBX 内存放 HyperRAM 两种崩法(线程栈 INVSTATE / 传输缓冲吐垃圾扇区,EXTRAM 禁入)、类线程优先级 20 被声学 13 饿死(→12)。供电纪律:USB 5V 继电器(CH341/COM 自动探测)纳入 n647.ps1,烧写断电序列自动断 USB 5V。板验:读 1.8MB BMP 无损、写入跨重挂存活、退出后 status 全绿+自检 PASS;FS 模式 ~9KB/s(HS 待布线整洁后启用)。详见 docs/usb_msc_bringup.md | 本次 |
| 波束实时监听+回放(固件就绪,待上板) | MAX98357A 功放到货,I2S2 播放链路落地:`app_beam_play.c`(I2S2 主发 48k、PLL2→IC8 与采集同 VCO 采样锁定、GPDMA1_Ch2 循环双半缓冲、欠载填零)+ FeedFrame 监听 tap(录音页「监听」开关,指哪听哪,~10ms 延迟)+ 最近 WAV 回放(「回放」键,媒体线程喂流,与录制互斥,USB 模式自动停)。腾内部 RAM:USBX regular 池挪 npuRAM3 0x34220000。接线:P1.17=BCLK/P1.19=LRC/P1.21=DIN(全 AF5)+P6 5V/GND,GAIN/SD 悬空。GDB 钩子 g_app_beam_play_test_request(1=监听 2=回放)。双构建零错零警;**未上板验证**(板子不在),验收清单见 docs/beam_realtime_audio_options.md 顶部 | 本次 |
| 电源板杜邦联调(充电链验收) | 焊好的自研电源板经 H1 排针+杜邦接入(FPC 待正式化):BQ25730 探测 0x40/0xD5、电池 ADC 真值(坑:POR 低功耗 EN_LWPWR 使 ADC 不转换,init 清位)、**12V PD 充电全项通过**(256/512/768mA 闭环 ichg=命令值、4min 浸泡跨 175s 看门狗、停充回 IDLE);5V 轮 VINDPM 极限行为符合预期(ichg=0 是真值非 ADC 故障)。修掉真 bug:ChargerStatus 高低字节接反,STAT_AC 误读为 ACOV 故障 → 插适配器即假 FAULT。工具:power_dbg.ps1(BQ 全状态)、gimbal_test.ps1(激光/舵机/充电一键,含 GPIO ODR 回读)。遗留:BQ I2C 杜邦链路 ~75% NACK(干净 NACK 型,触摸无恙,待查共地/轨压/R16-R20+R29)、F1 1A 限充 ≤640mA、CH224A I2C 控压未实现(硬件 12V 档已够)。见 docs/power_board_bringup.md | 本次 |
| USB 吞吐优化(HS+400 倍) | OTG1 直连后 HS 480Mbps 枚举成功(`PCD_SPEED_HIGH`)。9KB/s 根因:`sd_nand_init()` 每次 MSC 回调都跑,内含 10ms 电源轨稳定延时 = 每 1KB 等 110ms(DWT 实测定罪)→ 加已初始化快路径。再把 MSC 分块 1KB→16KB(`UX_SLAVE_REQUEST_DATA_MAX_LENGTH`),端点缓冲挪进 npuRAM3(0x34200000 片上 AXI SRAM,声纹网络只用 npuRAM4/5;须清 RAMCFG SRAMSD,同 NPU 解锁坑,否则读到随机垃圾)。板验:裸读 16MB **3.7MB/s**、写 8MB **1.7MB/s**、读写 MD5 往返一致、退出重挂全绿;Release XIP 冷启动存活(tick/vsync 前进)。遗留(整体优化候选):写路径 SDMMC IDMA 双缓冲、OTG DMA、status 工具 XIP 下自动选 Release ELF、USB GDB 钩子放开到 Release | 本次 |
| 热力图显示链 | 三大根因修复:1) DCMIPP 帧 ISR 把 P1STM0AR(=刚完成帧)当"正在采集",显示的正是下一帧的采集目标→热图被顶行开始活体覆盖(=闪烁);改为三缓冲轮转(FB 0x903/904/905),完成帧展示期间绝不被采集。2) 热图门控与锁定门解耦:首个 valid 帧即开门(旧:两帧一致才显示),持有期热图跟随 SRP 实时场(~7fps)刷新(旧:只在稀疏 valid 帧冻结拷贝→"1-2 秒才动");q>=2 开门/任意 valid 续门滞回。3) perf 撕裂防护(total<分段和时用分段和,系统页"总计 0.0M"消除) | 本次 |
| 1px 条纹(历史顽疾) | 根因=采集调度与扫描的一帧重叠窗:flip 请求后 CFBAR 影子加载最多滞后一个 LTDC 刷新,三缓冲下 ISR 选"空闲块"仍会打中"刚请求上屏"的帧→细碎覆盖条纹。改四缓冲(FB 0x902-0x905)+显式排除{刚完成,合成中,CFBAR 正扫}三者。A/B 证据:相机缓冲改不可缓存后条纹消失(证明非算法)、混合算法 PC 离线重放像素级干净;顺带测得 uncached=105ms/draw、write-through=92ms/draw、write-back+全帧清=32ms(采用)。验证:音频激励下合成帧异常行 50-60→0 | 本次 |
| 定位校准方法论(重要) | 两个根因闭环:1) **连续正弦单音在实验室(玻璃窗强反射)形成驻波场**——tone bin 幅度跨麦 24 倍散布、SRP 候选全部钉死在搜索边缘(仰角±60-62°伪峰)、q=0;换**宽带噪声**立即 conf=100 稳定锁定(左/右声道各自独立方位,复现可靠)。校准/验收一律用噪声或真实宽带源,别用单音。2) 相机 FOV 扩大(2560x1920 裁剪)后 overlay 常数仍是 77x61.1°→所有准心/热图缩放错一倍;按裁剪光学重算为 50.4x38.9°(app_acoustic_service.h + Model.cpp 两处)。附:接线乱序假设已排除——**PCMD3180 就焊在麦阵板上**(阵列网表:U1/U2→SDOUT1、U3/U4→SDOUT2,DIN 经串阻直连各麦),FPC 只传 TDM+I2C,重插不可能打乱麦序;I2C present=0xf 也证明 FPC 无错位。原始帧相位拟合脚本 _debug_logs/phase_fit.py 可复用。遗留:相机-阵列俯仰安装夹角未定标(热图残影与准心俯仰有出入),需实物参照一次性标定 | 3a585d30 |
| 触摸 | 根因=旧 7 寸屏 GT911 配置丢失(硬件);驱动加了诊断+锁竞争区分 | f08504a5 |
| H7 控制器 | boot1/xipboot/devboot 串口命令(COM3, 921600) | H7 Keil 工程 |

## 阻塞项(需要人在实验室)

1. **硬件修复**:摘 5V 轨上烧毁的 RT9293(冷启动间歇冻结头号嫌疑)、
   换 7 寸屏总成、查 A 总线槽 14 死麦。修完跑 20 次 `xipboot` 冷启动批量。
2. **采购**:云台已购(LD-1501MG/LD-3015MG PWM 舵机二维云台,在途;
   接线按 docs/gimbal_laser_pinmap.md);24.576MHz TCXO 不再必须
   (192k 已无晶振实现);AMS1117+磁珠(麦阵独立供电)仍建议。
3. ~~NPU 训练数据~~ **已完成(2026-07-12)**:6 类 × ~150 窗口(PC 播放特征音采集,
   数据在 `tools/npu/dataset/`),CNN 99.6% int8 精度,板上播放实测 5/5 类正确。
   赛前可到实验室用真实声源(真泄漏/真轴承)重采升级。重训流程:
   `collect_all.ps1` → `train_cnn.py` → `quantize_neccs.py` → `eval_onnx.py` →
   `stedgeai generate` → `make_weights_blob.py` + 拷贝 network.c/h → flash-debug。

### NPU 硬件解锁两大坑(2026-07-12 实证,已写入 app_npu.c,勿删)

- **RAMCFG SRAMSD**:AXISRAM3-6 复位态是 shutdown,只开 RCC 时钟远远不够,
  读返回 0、写被丢弃。必须清 `RAMCFG_SRAMx_AXI->CR` 的 SRAMSD 位。
- **RIF 主设备属性**:NPU 是总线主设备,不配 RIMC 时发出非安全事务,
  RISAF 静默拦截一切内存访问——推理"正常完成"但从不碰内存,输出读到的
  是输入缓冲的旧字节。必须 `HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_NPU, CID1+SEC+PRIV)`。
- CACHEAXI 保持 bypass(mpool 配置 bypass_enable=1,生成代码无 NPU 缓存维护)。

## 两周计划位置

计划文件:`~/.cursor/plans/二轮验收两周冲刺_8cbc5754.plan.md`。
第 1-3 天软件项已全部完成;192k 模式已上线;云台驱动已预备(PWM 版,
引脚定稿见 docs/gimbal_laser_pinmap.md);等硬件到货后做标定+UI 开关、
最后冻结彩排。

## 已知问题(按优先级)

1. 录像偶发写坏 JPEG 帧(录制侧;解码端已能跳过自愈;修录制路径时
   从 `media_hw_encode_color` 的输出尺寸/写入时序查起;铁证:VID00011
   帧 2 PC 端 PIL 也解不开,DHT 段位置是垃圾数据)
2. 冷启动间歇冻结(SysTick 停摆,连 FSBL 也会;疑电源;等 RT9293 摘除后复测)
3. 64KB EXTRAM FileX 缓存导致重启后文件扫不到(已回滚为 16 扇区内部
   SRAM;根因未查明,注释在 app_media.c)
4. 触摸 y 轴映射在 4.3 寸临时屏上恒为 0(7 寸屏参数不适配小屏,换正式
   屏后无需处理)

## 快速自检(接手第一件事)

```powershell
cd D:\Project\NECCS\Program\NECCS_N647
.\tools\debug\n647.ps1 status      # 期望: uptime 增长, pcmd 0xf/0xf, wdog=0
.\tools\debug\n647.ps1 screenshot  # 期望: UI 正常渲染
```
