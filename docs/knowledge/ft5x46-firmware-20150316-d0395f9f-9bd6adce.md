<!-- page: 1 -->
日期 记录
2014.07.29 1. 工厂模式增加 自容噪声值 0x47 0x48
2014.07.30 1. 增加0x1E Diff增益寄存器
2014.08.01 1. 修改工作模式下, 0xC0寄存器为手套开关寄存器
2014.08.01 1. 修改工作模式下, 0x1F的范围改为 20--200
2014.08.06 1. 待机寄存器A5 写 03值修改
2014.08.07 1. 增加0x57 Win8平台HomeKey GPIO量产测试使能寄存器
2014.08.07 1.增加调试版本寄存器0xAE说明
1.AD为调试版本号，内部用
2014.08.08 2.AE改为 公版版本号高位
3.工厂模式增加 IC流水信息 0x60--0x6C
1.AD为调试版本号，内部用
2014.08.11 2.AE改为 公版版本号高位
3.工厂模式增加 IC流水信息 0x60--0x6C
2014.08.18 1. 工厂模式0x1F 改为 20--150
2014.08.25 1.流水信息校正
1、补充详细数据格式说明
2、修正部分不正确的描述
2014.08.25
3、增加工作模式0xFE寄存器说明
4、工作模式0xFC寄存器改名为ID_G_UPGRADE，避免重名，并与0xFD寄存器互换位
2014.08.26 1. 更正一些说明
2014.09.05 1. 增加Down all.bin命令
2014.09.09 1. 自容检测寄存器，增加防水不检测的标志
2014.09.16 1. 工厂模式增加Offset寄存器
1. 更正工作模式对阈值移位的错误说明
2. 更正工厂模式对互容扫描电压的错误说明
2015.02.27
3. 增加工厂模式自动归一寄存器描述
4. 增加工厂模式自容频点噪声分析寄存器描述
2015.03.11 1.增加TE信号输出信息
1. 修改工厂0xb 寄存器描述，避免误解
2015.03.16
2. 增加自定义手势相关寄存器描述

<!-- page: 2 -->
寄存器页面 工作模式
切换方式 写0x00到寄存器0x00

寄存
器地 读写 寄存器名称 b7 b6 b5 b4 b3 b2 b1 b0
址
Device Mode[2:0]
0x00 读写(RW) Mode_Switch 0 :工作模式
4 :工厂模式

Gesture ID [7:0];
0x10: 向上滑动;
0x18: 向下滑动;
0x1c: 向左滑动;
0x01 只读(RO) Guesture
0x14: 向右滑动;
0x48: 缩小;
0x49: 放大;
0x00: 无手势.

0x02 只读(RO) Cur Point Number of touch points[7:0]
1st Event Flag
0: DOWN_EVENT 1st Touch X
0x03 只读(RO) TOUCH1_XH 1: UP_EVENT 2: CONTACT_EVENT Position[11:8]
3: NO_EVENT
0x04 只读(RO) TOUCH1_XL 1st Touch X Position[7:0]
0x05 只读(RO) TOUCH1_YH 1st Touch ID[3:0] 1st Touch Y
0x06 只读(RO) TOUCH1_YL 1st Touch Y Position[7:0]
0x07 只读(RO) TOUCH1_WEIGHT 1st Touch Weight[7:0]
0x08 只读(RO) TOUCH1_MISC 1th Touch Area[3:0] 1th Touch 1th Touch
0x09 只读(RO) TOUCH2_XH 2nd Event Flag 2nd Touch X
0x0A 只读(RO) TOUCH2_XL 2nd Touch X Position[7:0]
0x0B 只读(RO) TOUCH2_YH 2nd Touch ID[3:0] 2nd Touch Y
0x0C 只读(RO) TOUCH2_YL 2nd Touch Y Position[7:0]
0x0D 只读(RO) TOUCH2_WEIGHT 2nd Touch Weight[7:0]
0x0E 只读(RO) TOUCH2_MISC 2th Touch Area[3:0] 2th Touch 2th Touch

<!-- page: 3 -->
寄存器页面 工作模式
0x0F 只读(RO) TOUCH3_XH 3rd Event Flag 3rd Touch X
0x10 只读(RO) TOUCH3_XL 3rd Touch X Position[7:0]
0x11 只读(RO) TOUCH3_YH 3rd Touch ID[3:0] 3rd Touch Y
0x12 只读(RO) TOUCH3_YL 3rd Touch Y Position[7:0]
0x13 只读(RO) TOUCH3_WEIGHT 3rd Touch Weight[7:0]
0x14 只读(RO) TOUCH3_MISC 3th Touch Area[3:0] 3th Touch 3th Touch
0x15 只读(RO) TOUCH4_XH 4th Event Flag 4th Touch X
0x16 只读(RO) TOUCH4_XL 4th Touch X Position[7:0]
0x17 只读(RO) TOUCH4_YH 4th Touch ID[3:0] 4th Touch Y
0x18 只读(RO) TOUCH4_YL 4th Touch Y Position[7:0]
0x19 只读(RO) TOUCH4_WEIGHT 4th Touch Weight[7:0]
0x1A 只读(RO) TOUCH4_MISC 4th Touch Area[3:0] 4th Touch 4th Touch
0x1B 只读(RO) TOUCH5_XH 5th Event Flag 5th Touch X
0x1C 只读(RO) TOUCH5_XL 5th Touch X Position[7:0]
0x1D 只读(RO) TOUCH5_YH 5th Touch ID[3:0] 5th Touch Y
0x1E 只读(RO) TOUCH5_YL 5th Touch Y Position[7:0]
0x1F 只读(RO) TOUCH5_WEIGHT 5th Touch Weight[7:0]
0x20 只读(RO) TOUCH5_MISC 5th Touch Area[3:0] 5th Touch 5th Touch
0x21 只读(RO) TOUCH6_XH 6th Event Flag 6th Touch X
0x22 只读(RO) TOUCH6_XL 6th Touch X Position[7:0]
0x23 只读(RO) TOUCH6_YH 6th Touch ID[3:0] 6th Touch Y
0x24 只读(RO) TOUCH6_YL 6th Touch Y Position[7:0]
0x25 只读(RO) TOUCH6_WEIGHT 6th Touch Weight[7:0]
0x26 只读(RO) TOUCH6_MISC 6th Touch Area[3:0] 6th Touch 6th Touch
0x27 只读(RO) TOUCH7_XH 7th Event Flag 7th Touch X
0x28 只读(RO) TOUCH7_XL 7th Touch X Position[7:0]
0x29 只读(RO) TOUCH7_YH 7th Touch ID[3:0] 7th Touch Y
0x2A 只读(RO) TOUCH7_YL 7th Touch Y Position[7:0]
0x2B 只读(RO) TOUCH7_WEIGHT 7th Touch Weight[7:0]
0x2C 只读(RO) TOUCH7_MISC 7th Touch Area[3:0] 7th Touch 7th Touch
0x2D 只读(RO) TOUCH8_XH 8th Event Flag 8th Touch X
0x2E 只读(RO) TOUCH8_XL 8th Touch X Position[7:0]

<!-- page: 4 -->
寄存器页面 工作模式
0x2F 只读(RO) TOUCH8_YH 8th Touch ID[3:0] 8th Touch Y
0x30 只读(RO) TOUCH8_YL 8th Touch Y Position[7:0]
0x31 只读(RO) TOUCH8_WEIGHT 8th Touch Weight[7:0]
0x32 只读(RO) TOUCH8_MISC 8th Touch Area[3:0] 8th Touch 8th Touch
0x33 只读(RO) TOUCH9_XH 9th Event Flag 9th Touch X
0x34 只读(RO) TOUCH9_XL 9th Touch X Position[7:0]
0x35 只读(RO) TOUCH9_YH 9th Touch ID[3:0] 9th Touch Y
0x36 只读(RO) TOUCH9_YL 9th Touch Y Position[7:0]
0x37 只读(RO) TOUCH9_WEIGHT 9th Touch Weight[7:0]
0x38 只读(RO) TOUCH9_MISC 9th Touch Area[3:0] 9th Touch 9th Touch
0x39 只读(RO) TOUCH10_XH 10th Event Flag 10th Touch X
0x3A 只读(RO) TOUCH10_XL 10th Touch X Position[7:0]
0x3B 只读(RO) TOUCH10_YH 10th Touch ID[3:0] 10th Touch Y
0x3C 只读(RO) TOUCH10_YL 10th Touch Y Position[7:0]
0x3D 只读(RO) TOUCH10_WEIGHT 10th Touch Weight[7:0]
0x3E 只读(RO) TOUCH10_MISC 10th Touch Area[3:0] 10th Touch 10th
0x3F~
RESERVED
0x7F
0x80 读写(RW) ID_G_MC_THGROUP 互电容触摸阈值/4
0x81 读写(RW) ID_G_MC_THPEAK 互电容Peak阈值/4
0x82 读写(RW) ID_G_SC_THGROUP 自容触摸阈值/16
0x83 读写(RW) ID_G_SC_TXTHPEAK 自容Tx Peak阈值/16
0x84 读写(RW) ID_G_SC_RXTHPEAK 自容Rx Peak阈值/16
0x87 读写(RW) ID_G_THDIFF 点滤波范围阈值/16
0x86 读写(RW) ID_G_CTRL 是否允许进入monitor模式（1 - 允许，0 - 禁止）
0x87 读写(RW) ID_G_TIMEENTERMONITOR 在指定时间内没有触摸则进入MONITOR状态
0x88 读写(RW) ID_G_PERIODACTIVE 工作模式扫描周期 (用于控制报点率）
0x89 读写(RW) ID_G_PERIODMONITOR MONITOR模式扫描周期 （控制报点率）
0x8A Reserved 保留
0x8B 读写(RW) ID_G_CHARGER_STATE 充电器插入指示标志（1 - 开，0 - 关）
0x8C 只读(RO) ID_G_FACE_DETECT_STATE 接近感应状态（1 - 接近，0 - 离开）, 调试用,主要用于
0x8D 只读(RO) ID_G_FACE_DETECT_HUGE_REDUCE 接近感应自容变化大小, K为单位, 调试用, 将来扩展+G
0x8E 只写(WO) ID_G_FACE_DETECT_G_STATE G sensor状态（0x1E - 大动作，0x5A - 很大动作, 0x0F -

<!-- page: 5 -->
寄存器页面 工作模式
0x8F 读写(RW) ID_G_TOUCH_INT_CNT 中断次数统计, 用于ESD等异常情况下, INT无法正确发出
0x90 Reserved 保留
0x91 只读(RO) REG_CHECK_IF_FLOW_WORK_RUNNING Flow work的循环次数统计, 用于ESD等异常情况下, WDT
0x92-
Reserved 保留
0x98
0x9F 只读(RO) ID_G_CHIPER_LOW IC内核--低字节 0x22
0xA0 只读(RO) ID_G_TYPE IC型号名称
0xA1 只读(RO) ID_G_LIB_VERSION_H App库文件版本号高字节
0xA2 只读(RO) ID_G_LIB_VERSION_L App库文件版本号低字节
0xA3 只读(RO) ID_G_CHIPER_HIGH IC内核--- 高字节 0x54
0xA4 读写(RW) ID_G_MODE 1 - INT trigger模式，0 - INT polling模式
0xA5 读写(RW) ID_G_PMODE Power Mode：0 - Active，1 - Monitor，3 - Sleep
0xA6 只读(RO) ID_G_FIRMID Firmware version number
0xA7 读写(RW) ID_G_STATE FW内部工作状态切换（工作模式和工厂模式切换)，寄
0xA8 只读(RO) ID_G_VENODRID Vendor ID（屏厂）
0xA9 只读(RO) ID_G_DRVLIB_VER_H 驱动库文件版本号高字节
0xAA 只读(RO) ID_G_DRVLIB_VER_L 驱动库文件版本号低字节
0xAB- Reserved 保留
开发或调试阶段的 Firmware版本信息，便于在调试阶段
0xAD 读写(RW) ID_G_FIRM_DEBUG_ID
进行版本记录.可写，但掉电后会恢复到默认值。
0xAE 只读(RO) ID_G_RELEASE_ID_HIGH 公版release code ID 高位
0xAF 只读(RO) ID_G_RELEASE_ID_LOW 公版release code ID 低位
0xB0 读写(RW) ID_G_FACE_DEC_MODE 近距离感应使能 (1 - enable，0 - disable)
0xB1 只读(RO) ID_G_IC_VERSION IC version number: 0x01 - A, 0x02 - B, 0x03 - C, 0x04 - D,
0xB2 只读(RO) ID_G_PRESIZE_EN 压力大小上报使能
0xB3 读写(RW) ID_G_HOVER_MODE 悬浮模式 (1 - enable，0 - disable)
0xB4-
Reserved 保留
0xBD
0xBE 只读(RO) ID_G_SCAN_STATUS Scan扫描状态
0xBF 读写(RW) Reserved Reserved
0xC0 读写(RW) ID_G_GLOVE_MODE_EN 手套模式开关，1：开; 0: 关
0xC1 读写(RW) ID_G_COVER_MODE_EN 皮套模式，1：开; 0: 关
0xC2 读写(RW) ID_G_PEN_SUPPORT_EN 被动笔模式，1：开; 0: 关

<!-- page: 6 -->
寄存器页面 工作模式
0xC3 读写(RW) ID_G_HOST_EVENT_MSG HOST 消息通知（如来电通知），0x0：无消息；0x1：
0xC4 读写(RW) ID_G_COVER_WINDOW_LEFT 皮套透明窗口左边坐标，等于（实际坐标*255/分辨率）
0xC5 读写(RW) ID_G_COVER_WINDOW_RIGHT 皮套透明窗口右边坐标，等于（实际坐标*255/分辨率）
0xC6 读写(RW) ID_G_COVER_WINDOW_UP 皮套透明窗口上边坐标，等于（实际坐标*255/分辨率）
0xC7 读写(RW) ID_G_COVER_WINDOW_DOWN 皮套透明窗口下边坐标，等于（实际坐标*255/分辨率）
0xC8--
保留
0xCF
0xD0 读写(RW) ID_G_SPEC_GESTURE_EN 特殊手势模式，1：开; 0: 关
0xD1 读写(RW) ID_G_SPEC_GESTURE_CONFIG1 特殊手势字符开
0xD2 读写(RW) ID_G_SPEC_GESTURE_CONFIG2 特殊手势字符开关,Bit0--7:“o”, "w", "m", "e", "c", "g", "a",
0xD3 读写(RW) ID_G_SPEC_GESTURE_COOR 特殊手势信息输出;写操作:输出游标归0,读操作:信息输出
0xD4 只读(RO) ID_G_SPEC_GESTURE_COOR1 特殊手势信息输出;写操作:无效,读操作:信息输出
0xD5 读写(RW) ID_G_SPEC_GESTURE_CONFIG3 特殊手势字符开关,Bit0:"n", Bit2--Bit7: "b", "q", "L", "p",
0xD6 读写(RW) ID_G_SPEC_GESTURE_CONFIG4 特殊手势字符开关,Bit0:"@", Bit2--Bit5: ">", "^", "v", "△"
0xD7 读写(RW) ID_G_SPEC_GESTURE_CONFIG5 特殊手势字符开关,Bit0--3:"h", "k", "y", "r"
0xD8 读写(RW) ID_G_SPEC_GESTURE_CONFIG6 特殊手势字符开关,Bit0--5:"3", "6", "9", "7", "8", "2"
0xD9 读写(RW) ID_G_SELF_DEFINED_GESTURE_STATUS 特殊手势字符,自定义手势状态读取寄存器
0xDA 读写(RW) ID_G_SELF_DEFINED_GESTURE_OUTPUT 特殊手势字符,自定义手势模板读取寄存器
0xDB 读写(RW) ID_G_SELF_DEFINED_GESTURE_OUTPUT1 特殊手势字符,自定义手势模板读取寄存器
0xDC 读写(RW) ID_G_SELF_DEFINED_GESTURE_INPUT 特殊手势字符,自定义手势模板添加寄存器
0xDD 读写(RW) ID_G_SELF_DEFINED_GESTURE_INPUT1 特殊手势字符,自定义手势模板添加寄存器
0xDE 读写(RW) ID_G_SELF_DEFINED_GESTURE_OPTION 特殊手势字符,自定义手势模板操作寄存器
0xE0 读写(RW) ID_G_PERIOD_WAK_INVALD 唤醒后手势有效延迟时间
0xF0 读写(RW) RegAddrH 内部寄存器地址高8位
0xF1 读写(RW) RegAddrL 内部寄存器地址高8位
0xF2 读写(RW) RegDataH 内部寄存器数据高8位
0xF3 读写(RW) RegDataL 内部寄存器数据高8位
读 - 返回FW工作状态 （见0xA7寄存器说明）
W FC AA ; W FC 55 : 升级命令
0xFC 读写(RW) ID_G_UPGRADE
W FC AA ; W FC 66 : 软件复位命令
W FC AA ; W FC 88 : Download all.bin命令
0xFD 只读(RW) Debug-state 状态寄存器输出

<!-- page: 7 -->
<table><tr><td colspan="2">寄存器页面</td><td colspan="2">工作模式</td></tr><tr><td>0xFE</td><td>只读(RW)</td><td>Work Freq</td><td>返回当前工作频点值</td></tr><tr><td colspan="4">备注：灰色表示预留，未用</td></tr></table>

<!-- page: 8 -->
寄存器页 工厂模式（TEST）
切换方式 写0x40到寄存器0x00

寄存 寄存
寄存器
模块 器地 读写 寄存器名称 器 寄存器描述
长度
址 默认
寄存器页面模式切换
Bit[0]~Bit[3]: 保留
0x00 1 Byte 读写(RW) Mode_Switch Bit[4~6]: 0 工作模式，4 工厂模式
Bit[7]: 写1时FW开始采集RawData，一帧数据采集完成后FW
自动清零
RawData类型切换，切换时RawData地址寄存器偏移量会相应
的清零
AA: Mc RawData, RawData地址寄存器指向互容
RawData
0x01 1 Byte 读写(RW) AB: Sc Nomal, RawData地址寄存器指向自容非防水
Line Addr
AC: Sc Water, RawData地址寄存器指向自容防水
AD: RawData地址寄存器偏移量清零
AE: 设置RawData地址寄存器偏移量，见0x2c(高8位)，
0x02 1 Byte 只读(RO) TxNum 当前使用Tx数量
0x03 1 Byte 只读(RO) RxNum 当前使用Rx数量

写 - 触发校准和保存参数
0x04: 自容自动校准
0x05：保存校准后的参数到flash
0x06：互容自动归一计算
读 - 返回校准状态
0x04 1 Byte 读写(RW) CLB cmd&status
0x00：未校准初始状态
0x01：校准进行中
0x02：校准成功
0x03：校准失败
0xFF：校准失败

支持的点数
0x05 1 Byte 读写(RW) MaxPointNum
最大10点，默认5点

工厂模式下显示数据类型选择
0x06 1 Byte 读写(RW) DataSelect 0：显示固件采集的RawData系统
1：显示固件原始Diff(带环境跟踪)

<!-- page: 9 -->
寄存器页 工厂模式（TEST）

工厂模式下短路检测使能
写：使能短路检测
1: enable
0: disable
0x07 1 Byte 读写(RW) ShortTestEn

读：返回短路检测状态
1: 短路检测中
0: 短路检测完成

0x08 1 Byte 只写(WO) IntTest INT脚测试，INT脚波形反转
防水模式：
Bit6: 0 : 检测防水Rx+Tx； 1：只检测一个通道
Bit5: 0：检测防水模式; 1：不检测防水模式.
Bit2: 0: 只检测防水Tx; 1: 只检测防水Rx
Water_Channel 有效性判断, 先Bit5 --> Bit6 --> Bit2
0x09 1 Byte 只读(RO)
_Select 普通模式：
Bit7: 0 普通模式检测； 1：普通模式不检测;
Bit1,Bit0: 00:普通模式Tx; 01: 普通模式Rx; 10: 普通模式
Rx+Tx
有效性判断, 先Bit7 --> Bit1,0

跳频开启时,切换工作频点
0--3为对应的跳频频点表中的索引;
0x0A 1 Byte 读写(RW) FreList 0x00
0x80: 切换到跳频频点表中的最低频点;
0x81: 切换到跳频频点表中的最高频点;
仅供部分较早期项目使用
0x0B 1 Byte 读写(RW) OffsetSelect 0x00 1: 读取归一后Rawdata；
0: 读取调均匀后的Rawdata；
0x0C 19Byte RESERVED 保留
互容Rawdata类型
(如果打开自动归一化, 则默认值为1, 即按行列自动归一后的
0x16 1 Byte 读写(RW) McRawDataType 0
rawdata类型;
如果不开, 则默认值为0, 即原始(整体归一化)的rawdata).
0x17 1 Byte 读写(RW) McEQLimitH 0 互容自动归一化的最大限制(百分比值)
0x18 1 Byte 读写(RW) McEQLimitL 0 互容自动归一化的最小限制(百分比值)
互容读取自动归一化参数的地址地址 (0 ~
0x19 1 Byte 读写(RW) McEQAddrR 0
RX_NUM+2*TX_NUM)

<!-- page: 10 -->
寄存器页 工厂模式（TEST）

0x1A 1 Byte 读写(RW) McTxOffsetAddrR 0 互容读TX OFFSET起始地址 （0--34）

0x1B 1 Byte 读写(RW) McTxOffsetAddrW 0 互容写TX OFFSET起始地址 （0--34）
0x1C 1 Byte 读写(RW) McRxOffsetAddrR 0 互容读RX OFFSET起始地址 （0--27）
0x1D 1 Byte 读写(RW) McRxOffsetAddrW 0 互容写RX OFFSET起始地址 （0--27）
0x1E 1 Byte 读写(RW) ucMcGain 64 Diff增益 = gain/64, 默认1倍
归一目标值 = Reg[0x1F]*100
0x1F 1 Byte 读写(RW) McClbVal 90
范围： 20--150
扫描电压（自互容共用）(0~7) , 每档0.5V , 默认7.5V
0x20 1 Byte 读写(RW) ChpVol 4
0=5.5v; 1=6.0v; 2=6.5v; 3=7.0v; 4=7.5v; 5=8.0v; 6=8.5v
0x21 1 Byte 读写(RW) VK mul 64 虚拟按键归一系数（1~255）
0x22 1 Byte 读写(RW) Va mul 64 VA区归一系数（1~255）
0x23 1 Byte 读写(RW) Shift 0 互电容归一移位数（0~6）
0x24 1 Byte 读写(RW) McTxCapAddrR 0 互容TxCap读地址寄存器（0~34）
0x25 1 Byte 读写(RW) McTxCapAddrW 0 互容TxCap写地址寄存器（0~34）
0x26 1 Byte 读写(RW) McRxCapAddrR 0 互容RxCap读地址寄存器（0~27）
0x27 1 Byte 读写(RW) McRxCapAddrW 0 互容RxCap写地址寄存器（0~27）
互容 0x28 1 Byte 读写(RW) McTxOrderAddrR 0 互容TxOrder读地址寄存器（0~34）
0x29 1 Byte 读写(RW) McTxOrderAddrW 0 互容TxOrder写地址寄存器（0~34）
0x2A 1 Byte 读写(RW) RxOrderAddrR 0 互容RxOrder读地址寄存器（0~27）
0x2B 1 Byte 读写(RW) RxOrderAddrW 0 互容RxOrder写地址寄存器（0~27）
0x2C 1 Byte 读写(RW) RawAddrH 0 RawData地址寄存器偏移量高8位
0x2D 1 Byte 读写(RW) RawAddrL 0 RawData地址寄存器偏移量低8位 ， 必须先设高位，然后再设
0x2E 1 Byte 读写(RW) McOverFlag 0
0x2F 1 Byte 只读(RO) McFreMinH 0x01 基准频率高8位，
0x30 1 Byte 只读(RO) McFreMinL 0xF4 基准频率低8位，0.1KHz为单位，0x1F4即500表示50KHz
0x31 1 Byte 读写(RW) McFreStep 25 频率步长，0.1KHz为单位，默认2.5KHz
互容扫描频率实际可用(0~136), 即最高工作频点390KHz
互容实际扫描频率= ((Reg[McFreMinH]<<8 | Reg[McFreMinL]) 0x32 1 Byte 读写(RW) SCAN_FREQ 0
+ Reg[SCAN_FREQ]*Reg[McFreStep])/10(KHz),
实际调屏时，不超过360KHz
0x33 1 Byte 读写(RW) TX_CLK_NUM 32 TX的时钟数量，16，32，48，64四档
0x34 1 Byte 读写(RW) ADC_THR 85 Adc溢出的阈值 （10～100）

<!-- page: 11 -->
寄存器页 工厂模式（TEST）
互容采样倍数
0: 4倍
0x35 1 Byte 读写(RO) SAMPLE_MOD 1
1: 8倍
2: 16倍
0x36 1 Byte 只读(RO) RawBuf0 0 Rawdata数据寄存器0
0x37 1 Byte 只读(RO) RawBuf1 0 Rawdata数据寄存器1
0x38 1 Byte 读写(RW) McTxCapBuf0 0 互容TxCap数据寄存器0
0x39 1 Byte 读写(RW) McTxCapBuf1 0 互容TxCap数据寄存器1
0x3A 1 Byte 读写(RW) McRxCapBuf0 0 互容RxCap数据寄存器0
0x3B 1 Byte 读写(RW) McRxCapBuf1 0 互容RxCap数据寄存器1
0x3C 1 Byte 读写(RW) TxOrderBuf0 0 TxOrder数据寄存器0
0x3D 1 Byte 读写(RW) TxOrderBuf1 0 TxOrder数据寄存器1
公用
0x3E 1 Byte 读写(RW) RxOrderBuf0 0 RxOrder数据寄存器0
0x3F 1 Byte 读写(RW) RxOrderBuf1 0 RxOrder数据寄存器1
0x40 1 Byte 读写(RW) K1Delay 0x10 自电容K1周期 (1-128)
0x41 1 Byte 读写(RW) K2Delay 0x10 自电容K2周期 (10-128)
自电容扫描周期0：1次样, (1-62) = 8*(n)
0: 1 samples
1: 8 samples
0x42 1 Byte 读写(RW) SCSampleCycle 3
2: 16 samples
3: 24 samples
...
0x43 1 Byte 读写(RW) SCChannelCf 0x80 自电容通道CF值 （0~255）， 默认值0x80
自容 自容工作模式选择:
0x44 1 Byte 读写(RW) ScWorkMode 1: 防水模式
0: 非防水模式
0x45 1 Byte 读写(RW) ScCbAddrR 自容CB读地址寄存器
0x46 1 Byte 读写(RW) ScCbAddrW 自容CB写地址寄存器
0x47- 2Byte 只读(RO) usScNoise 当前自容的噪声值(需要关自容跳频)
0x47 5Byte RESERVED 保留
0x4E 1 Byte 读写(RW) ScCbBuf0 自容CB数据寄存器0
0x4F 1 Byte 读写(RW) ScCbBuf1 自容CB数据寄存器1

<!-- page: 12 -->
寄存器页 工厂模式（TEST）
设置读取FW 配置信息buf地址偏移量
0: I2C从地址;
1: I2C从地址校验码，按位取反;
2: I/O接口电压驱动，0为1.8V，1为VCC;
3: I/O接口电压校验码，按位取反;
4: 厂家ID; 0x50 1 Byte 读写(RW) FwCnt 0x00
5: 厂家ID校验码，按位取反;
6~31: 保留未用;
32: FW固件版本号;
33: FW固件版本号校验码，按位取反;
34~46: 客户代码，不超过13个字符;
47: 保留为零，留作字符串分隔符用
0x51 1Byte 只读(RO) FwBuf0 FW 信息buf(Host读接口)
量产 0x52 1Byte 只读(RO) FwBuf1 FW 信息buf (固件内部自锁用)
相关 0x53 1Byte 只读(RO) PatternType 屏体类型 1: V3 屏体.
屏体通道是否mapping(只对V3屏体有效).
V3PatternNoMapp
0x54 1Byte 读写(RW) 0 0: 表示mapping后.
ing
1: 表示mapping前
0x55 1Byte 只读(RO) TxNumScan 实际使用扫描Tx个数(只对V3屏体有效).
0x56 1Byte 只读(RO) RxNumScan 实际使用扫描Rx个数(只对V3屏体有效).
Win8 HomeKey Win8 HomeKey GPIO脚测试，
0x57 1Byte 只写(WO)
GPIO Pin Test HomeKey GPIO脚波形反转
0x58 1Byte 读写(RW) McTxOffsetBuf 0 互容Tx Offset数据寄存器
0x59 1Byte 读写(RW) McTxOffsetBuf- 0 互容Tx Offset数据寄存器-bak
0x5A 1Byte 读写(RW) McRxOffsetBuf 0 互容Rx Offset数据寄存器
0x5B 1Byte 读写(RW) McRxOffsetBuf- 0 互容Rx Offset数据寄存器-bak
0x5C 1Byte 读写(RO) RX/TX归一化参数 0 互容自动归一参数 数据寄存器0
0x5D 1Byte 读写(RO) RX/TX归一化参数 0 互容自动归一参数 数据寄存器1
0：没有检测到TE信号
0x5F 1Byte 只读(RO) TE信号输出信息 0 1：有TE信号，firmware正在使用
2：有TE信号，firmware没有使用
0x60-
8Byte 只读(RO) Wafer lot Number XX Wafer 的编码，使用 ASCII 码表示
-0x67
IC 0x68 1Byte 只读(RO) Wafer 刻号 XX 每片Wafer的编号
OTP 0x69 1Byte 只读(RO) Wafer X 轴座标 XX IC 在 Wafer X 轴坐标值
信息 0x6A 1Byte 只读(RO) Wafer Y 轴座标 XX IC 在 Wafer Y 轴坐标值

<!-- page: 13 -->
寄存器页 工厂模式（TEST）
0x6B 1Byte 只读(RO) CP 测试厂识别 XX 对测试厂的编码
0x6C 1Byte 只读(RO) CP 测试结果记录 XX 记录CP测试结果OK还是NG
0xD0 1Byte 读写(RW) 扫频K1起始频率 10 扫频K1起始频率
0xD1 1Byte 读写(RW) 扫频K1总长度 1 扫频K1总长度
0xD2 1Byte 读写(RW) 扫频K2起始频率 10 扫频K2起始频率
0xD3 1Byte 读写(RW) 扫频K2总长度 1 扫频K2总长度
0xD4 1Byte 只读(RO) 噪声检测状态 0 0： 空闲状态 1： 启动噪声检测 2：噪声检测中 3： 噪声检测
自容
0xD5 1Byte 读写(RW) 噪声数据频率起始 设置读出噪声数据的频率起始
频点
0xD6 1Byte 只读(RO) 噪声数据的高8位 噪声数据的高8位噪声
0xD7 1Byte 只读(RO) 噪声数据的低8位 噪声数据的低8位分析
0xD8 1Byte 只读(RO) 每个频点测试帧数 16 每个频点测试帧数
0xD9 1Byte 只读(RO) 当前扫描完频点 当前扫描完的频点K1
0xDA 1Byte 只读(RO) 当前扫描完频点 当前扫描完的频点K2
0：取防水模式噪声
0xDB 1Byte 只读(RO) 自容噪声扫描模式
1：取非防水模式噪声
0xE0 1 Byte 读写(RW) McFrePolBegin 0 噪声检测的起始频率(0~199)，频率意义同0x32寄存器
0xE1 1 Byte 读写(RW) McFrePolEnd 0 噪声检测的结束频率(0~199), 频率意义同0x32寄存器
噪声检测状态
互容 0：空闲状态
频点 0xE2 1 Byte 读写(RW) McFrePolState 0 1：启动噪声检测
噪声 2：噪声检测中
分析 3：噪声检测完成
0xE3 1 Byte 读写(RW) McFreAddrBegin 0 设置读噪声数据的起始频率，频率意义同0x32寄存器
0xE4 1 Byte 只读(RO) McFreNoiseDataH 0 噪声数据寄存器高8位
0xE5 1 Byte 只读(RO) McFreNoiseDataL 0 噪声数据寄存器低8位
0xF0 1 Byte 读写(RW) RegAddrH 内部通用寄存器地址高8位
0xF1 1 Byte 读写(RW) RegAddrL 内部通用寄存器地址低8位
0xF2 1 Byte 读写(RW) RegDataH 内部通用寄存器数据高8位
0xF3 1 Byte 读写(RW) RegDataL 内部通用寄存器数据低8位

<!-- page: 14 -->
寄存器页 工厂模式（TEST）

短路检测ValL数据寄存器0
数据总长度 :(TxNum+RxNum)*4 + 6 Bytes, 偏移顺序及字节长
度说明如下：
Offset高8位数据：1 Bytes
Offset低8位数据：1 Bytes
校正通道0对地短路高8位数据：1 Bytes
校正通道0对地短路低8位数据： 1 Bytes
Tx1~TxNum通道对地短路数据(高8位在前，低8位在后)：
0xF4 1 Byte 只读(RO) ValLBuf0 0 (TxNum*2) Bytes
Rx1~RxNum通道对地短路数据(高8位在前，低8位在后)：
(RxNum*2) Bytes
校正通道0通道互短路高八位数据：1 Bytes
DEB 校正通道0通道互短路低八位数据：1 Bytes
UG Tx1~TxNum通道互短路数据(高8位在前，低8位在后)：
(TxNum*2) Bytes
Rx1~RxNum通道互短路数据(高8位在前，低8位在后)：
(RxNum*2) Bytes

0xF5 1 Byte 只读(RO) ValLBuf1 0 短路检测ValL数据寄存器1
AFE参数组切换工厂测试寄存器，
0xF6 1 Byte 读写(RW) ID_G_SWITCH_AFE 0
0x1 高信噪比模式，0x0 普通模式
0xF7 1 Byte 读写(RW) ID_G_LCD_NOISE_ 80 LCD噪声阈值工厂测试寄存器
0xF8 1Byte 只读(RO) RESERVED 预留单通道对地短路测试异常通道上报
0xF9 1Byte 只读(RO) RESERVED 预留单通道对地短路测试异常通道上报
0xFA 1Byte RESERVED 保留
FIR使能
0xFB 1Byte 读写(RW) Fir开关 1: 使能FIR
0: 不使能FIR
0xFC 1 Byte RESERVED
0xFD 1 Byte 读写(RW) I2C_DEBUG_LEN I2C Debug 字节长度
0xFF 1 Byte 读写(RW) I2C_DEBUG_DATA I2C Debug 数据

备注：
1、灰色表示预留;
2、写16位寄存器时, 必需先写高8位，然后写低8位

<!-- page: 15 -->
| | | | | | |
|---|---|---|---|---|---|
| 0x00 | 0x01 | 0x02 | 0x03 | 0x04 | 0x05 |
| Slave_address | Slave_address取反 | iovoltage | Iovoltage 取反 | 屏厂ID | 屏厂ID取反 |
| **0x20** | ... | **0x3F** |
| Project Code(ASCII) 32字节 | |
| **0x40** | **0x41** |
| FW Version | |
| **0x42** | ...... | **0x4F** |
| Customer code(ASCII) 14字节 | |
