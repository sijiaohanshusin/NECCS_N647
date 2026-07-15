# 云台 + 激光引脚定稿与驱动契约

日期: 2026-07-15 · 状态: 引脚已定稿并同步进 .ioc, 驱动骨架已上板验证(无外设空跑)

## 一、定稿引脚(新板走线依据, .ioc 中已用标签固定)

经新板 HC-USB3.0-C34-P 连接器引出(USB2 D+/D- 走真 USB, SuperSpeed 4 根
差分线复用为 GPIO 拉到云台舱):

| 连接器线 | MCU 引脚 | .ioc 标签 | 功能 | 说明 |
|---|---|---|---|---|
| GPIO1 (SSRX-) | **PE13** | `GIMBAL_PAN_PWM` | 水平舵机 PWM | TIM1_CH3, AF1, 50 Hz / 500-2500 µs |
| GPIO2 (SSRX+) | **PE14** | `GIMBAL_TILT_PWM` | 俯仰舵机 PWM | TIM1_CH4, AF1, 同上 |
| GPIO3 (SSTX-) | **PA12** | `LASER_EN` | 激光使能 | 推挽, 高有效, 驱 MOSFET 栅极, 上电默认低 |
| GPIO4 (SSTX+) | **PB1** | `GIMBAL_RELAY_EN` | 继电器 | 推挽, 高有效, 上电默认低 |
| VBUS | - | - | 7.4 V 舵机/激光电源 | 电池直供, 与主板共地, 不过 MCU 板 |

依据: PE13/PE14 的 TIM1_CH3/CH4 AF1 映射对照 ST 官方
`NUCLEO-N657X0-Q/Examples/TIM/TIM_PWMOutput`(N657 与 N647 同 die)。
四个引脚都在现有底板 EXP 排针上, 新板到货前可飞线实测。

- 上电安全态: 四脚均在 `MX_GPIO_Init` 配成推挽输出并写低
  (激光/继电器绝不上电闪亮; PWM 脚无脉冲 = 舵机不锁轴)。
- RIF: 四脚原本就在 Appli CID1 SEC 白名单里, 无需改动。

## 二、驱动契约(`app_gimbal.c/h`, 已编译上板)

- `AppGimbal_Init()`: TIM1 1 MHz 计数(CCR 即微秒), 20 ms 帧;
  时钟用 `HAL_RCCEx_GetTIMGFreq()` 实时读(N6 无经典 2xAPB 规则)。
  惰性调用 -- 任何 Set* 入口都会自动 init, 开机不占启动时序。
- `AppGimbal_SetEnabled(on)`: 启/停两路 PWM(停 = 舵机松轴)。
- `AppGimbal_PointAt(theta, phi)`: 声学角(度) -> 脉宽, 每轴
  `center_us + deg * us_per_deg`(带符号可反装), 双重夹紧
  (标定限位 + 绝对 500-2500)。标定表 `s_cal_pan/s_cal_tilt`
  到货后按实测填(指向已知角度调 center/scale/limit + 视差补偿)。
- `AppGimbal_Poll()`: 挂在 UI tick(60 Hz), 800 µs/s 转速限幅后写 CCR,
  扫摆平滑且限制舵机浪涌电流。
- `AppGimbal_SetLaser/SetRelay(on)`: 电平控制。
- 联动(Model::tick 已接): 声源录音页在哪儿指哪儿(beam 目标),
  其余页面跟随声学锁定(sourceDisplayValid)。UI 的激光开关走
  `toggleLaser()`(快捷键位后续到货再放)。

## 三、GDB 试指向钩子(DEBUG 构建)

```
set 'app_gimbal.c'::g_app_gimbal_test_theta = 30
set 'app_gimbal.c'::g_app_gimbal_test_phi = -10
set 'app_gimbal.c'::g_app_gimbal_test_request = 1   # 使能+指向
set 'app_gimbal.c'::g_app_gimbal_test_request = 2   # 激光翻转
set 'app_gimbal.c'::g_app_gimbal_test_request = 3   # 继电器翻转
set 'app_gimbal.c'::g_app_gimbal_test_request = 4   # PWM 使能翻转
```

验证读数: `s_live_pan_us/s_live_tilt_us`(慢速逼近目标即转速限幅在工作)、
TIM1 寄存器(ARR=19999, CCR3/CCR4=脉宽)、GPIOA ODR bit12(激光)。

## 四、到货后待办

1. 飞线接舵机, 示波器/舵机实测 50 Hz 帧与 500-2500 µs 行程。
2. 标定 `s_cal_*`(center/scale/invert/limit)+ 云台在阵列上方 ~6 cm
   的视差补偿。
3. UI 放"激光指向"开关(建议声源录音页控制列或系统页)。
4. 演示形态: 定位锁定 -> 云台转向 -> 激光点打在声源 -> 同步定向录音。
