# 项目状态交接 — 2026-07-11

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
| NPU | 全链路跑通:378µs/次,实时喂谱,系统页遥测 | fc751a5b, 7161be94 |
| 触摸 | 根因=旧 7 寸屏 GT911 配置丢失(硬件);驱动加了诊断+锁竞争区分 | f08504a5 |
| H7 控制器 | boot1/xipboot/devboot 串口命令(COM3, 921600) | H7 Keil 工程 |

## 阻塞项(需要人在实验室)

1. **硬件修复**:摘 5V 轨上烧毁的 RT9293(冷启动间歇冻结头号嫌疑)、
   换 7 寸屏总成、查 A 总线槽 14 死麦。修完跑 20 次 `xipboot` 冷启动批量。
2. **采购**:串口二维云台+激光头(见聊天记录 07-11 的选型结论:载激光头
   不载整机,100g 负载规格足够)、24.576MHz TCXO(192k 模式)、
   AMS1117+磁珠(麦阵独立供电)。
3. **NPU 训练数据**:对设备播放各类声音,每类 1-2 分钟:
   `python tools/npu/collect_spectra.py --label <类名> --seconds 90`
   然后 `train_classifier.py` → `export_trained_onnx.py` → 量化 → stedgeai
   → 更新 `network_weights_blob.h` + `network.c`。

## 两周计划位置

计划文件:`~/.cursor/plans/二轮验收两周冲刺_8cbc5754.plan.md`。
第 1-3 天软件项已全部完成;等硬件到货后做:激光云台驱动(PWM 或串口)、
192k 模式(按 docs/core16_192k_design_status.md 顺序)、最后冻结彩排。

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
