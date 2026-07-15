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
| 云台/激光预备 | 引脚定稿并同步 .ioc:PE13/PE14=TIM1_CH3/CH4 舵机 PWM、PA12=LASER_EN、PB1=GIMBAL_RELAY_EN(均上电低);`app_gimbal` 50Hz PWM 驱动+标定表+转速限幅+GDB 试指向钩子,Model 已接自动跟随;标定与 UI 开关等硬件到货(docs/gimbal_laser_pinmap.md) | 本次 |
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
