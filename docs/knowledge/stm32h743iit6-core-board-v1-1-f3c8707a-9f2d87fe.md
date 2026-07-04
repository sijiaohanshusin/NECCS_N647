<!-- page: 1 -->
## Visual Summary (Page 1)

- page_class: front_matter
- confidence: 0.95
- reason_codes: title_page, mixed_visual, primary_image

Title page for STM32H743IIT6 Core Board V1.1 User Manual by Dongguan Huikun Zhiyuan Technology Co., Ltd. Features a large photograph of the development board with visible microcontroller chip, USB ports, and pin headers.

<!-- page: 2 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

前言

慧勤智远STM32H743IIT6 小系统板采用了STM32H743IIT6 作为主控芯片，该芯
片是STM32 MCU 家族主流的32 位微控制器，主频480MHz，Arm®Cortex®-M7 内核，
LQFP176pin 封装。

STM32H743IIT6 小系统板长为85mm，宽为55mm，厚为1.6mm，体型小巧，采用
稳定可靠的4 层沉金工艺，经久耐用。本手册对STM32H743IIT6 小系统板的硬件资源

进行阐述，以及介绍开发板软件工具的使用，让读者快速上手开发板，进行实践应用。

2

<!-- page: 3 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

更新记录

版本 日期 更新说明

V1.0 2024-12-31 初始版本

V1.1 2025-04-09 优化内容

3

<!-- page: 4 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

目录

前言 .... 2

更新记录 .... 3

目录 .... 4

1. STM32H743IIT6 小系统板配置 .... 5

1.1 STM32H743IIT6 小系统板视图 .... 5

1.2 STM32H743IIT6 小系统板位号图 .... 7

1.3 STM32H743IIT6 小系统板硬件资源总述 .... 9

1.4 STM32H743IIT6 小系统板硬件资源说明 .... 12

1.5 STM32H743IIT6 小系统板引脚分配 .... 15

2. STM32H743IIT6 小系统板硬件电路介绍 .... 20

3. 开发板软件使用方法 .... 29

3.1 MDK5 安装与使用 .... 29

3.1.1 MDK5 安装 .... 29

3.1.2 MDK5 打开工程 .... 34

3.1.3 MDK5 相关设置 .... 35

3.1.4 MDK5 仿真调试设置 .... 37

3.2 串口程序下载（STM32CubeProgrammer） .... 40

3.3 串口调试（SSCOM） .... 50

3.4 DFU 程序下载（STM32CubeProgrammer） .... 52

4. STM32H743IIT6 小系统板例程列表 .... 58

4

<!-- page: 5 -->
## Visual Summary (Page 5)

- page_class: mixed
- confidence: 0.95
- reason_codes: heuristic_page_type_text, heuristic_content_type_text, mixed_visual

Page contains a title section for 'STM32H743IIT6 Core Board V1.1 User Manual' and a figure caption 'Figure 1.1 STM32H743IIT6 Small System Board Top View'. The main visual is a photograph of the PCB board with red dimension lines indicating a width of 85mm and height of 55mm.

<!-- page: 6 -->
## Visual Summary (Page 6)

- page_class: block_diagram
- confidence: 0.95
- reason_codes: diagram_dominant, text_sufficient_for_summary

Page displays a block diagram of the STM32H743IIT6 Core Board V1.1 showing its physical dimensions (79mm x 49mm) and pin pitch specifications, with a caption identifying it as the bottom-layer view.

<!-- page: 7 -->
## Visual Summary (Page 7)

- page_class: block_diagram
- confidence: 0.95
- reason_codes: visual_content_dominant, schematic_diagram, policy_visual_summary_override

Page displays a schematic block diagram of the STM32H743IIT6 Core Board V1.1, showing component placement including USB, KEY0, WK_UP, RESET buttons, LCD interface (LCD1), and various capacitors/resistors on both top and bottom connector rows.

<!-- page: 8 -->
## Visual Summary (Page 8)

- page_class: block_diagram
- confidence: 0.95
- reason_codes: visual_content_dominant, diagram_structure_required, policy_visual_summary_override

Page displays a PCB layout diagram of the STM32H743IIT6 Core Board V1.1 back side, showing component placement including U2 (MCU), TF CARD slot, LCD2 connector, SWD debug interface, and various resistors/capacitors labeled with reference designators.

<!-- page: 9 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

1.3 STM32H743IIT6 小系统板硬件资源总述

STM32H743IIT6 小系统板采用STM32H743IIT6 作为主控芯片，STM32H743IIT6 小系统板的资源分
布如图1.5 所示。

串口芯片 16MB FLASH GPIO 口

3.3V 引出口

USB-C 转串口

过流保险丝

USB-C 通信接口 STM32H743IIT6

2 个用户按键
LCD1 接口-RGB

2 个用户LED

复位按键

电源指示灯 5V 引出口 EEPROM GPIO 口

a) STM32H743IIT6 小系统板顶层资源

9

<!-- page: 10 -->
## Visual Summary (Page 10)

- page_class: block_diagram
- confidence: 0.95
- reason_codes: visual_content_dominant, diagram_based, policy_visual_summary_override

Page displays a block diagram of an STM32H743IIT6 Core Board V1.1 showing component layout including M3 mechanical holes, power chip, TF card slot, LCD2 SPI interface, 32MB SDRAM, and SWD debug interface.

<!-- page: 11 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

STM32H743IIT6 小系统板的资源汇总如下表：

资源 数量 说明

MCU 1 个 STM32H743IIT6，内置FLASH: 2048KB，内置SRAM: 1060KB

3.3V 引出口 1 个 用于接入3.3V 电源或对外提供3.3V 电压

USB-C 转串口 1 个 用于USB 转TTL 串口通信

过流保险丝 1 个 保险丝，起过流保护作用

USB-C 通信接口 1 个 用于USB SLAVE / HOST 通信

用户按键 2 个 KEY0、WK_UP（具备唤醒功能）

用户LED 2 个 LED0（红色），LED1（绿色）

复位按键 1 个 RESET，用于MCU 复位

电源指示灯 1 个 PWR（蓝色）

5V 引出口 1 个 用于接入5V 电源或对外提供5V 电压

EEPROM 1 个 AT24C02，256B

GPIO 口 88 个 共引出88 个IO 口，用于连接外部模块

LCD1 接口-RGB 1 个 40PIN，支持4.3/5.0/7.0 寸RGB 接口屏

16MB FLASH 1 个 W25Q128，16MB，采用Quad-SPI 驱动

串口芯片 1 个 CH340X，用于实现USB 转串口功能

TF 卡座 1 个 用于TF 卡扩展

LCD2 接口-SPI 1 个 20PIN，支持2.4/3.5/4.3/5.0/7.0 寸SPI 接口屏

M3 机械孔 4 个 可装M3 铜柱，使开发板平稳放置

电源芯片 1 个 CJA1117B-3.3，将5V 电压转化为3.3V 电压

32MB SDRAM 1 个 W9825G6KH-6I，32MB

SWD 调试接口 1 个 用于仿真调试、程序下载等

以上就是STM32H743IIT6 小系统板的板载资源，相比于同类型的开发板，
STM32H743IIT6 小系统板具有以下优点：
A. 独有的20 PIN LCD 接口，搭配慧勤智远的SPI 接口触摸显示屏模块，可以驱动
更高分辨率的RGB 接口触摸显示屏（例4.3 寸/5.0 寸/7.0 寸）。
B. LCD 复位引脚采用独立的GPIO 口控制，可以满足不同LCD 驱动IC 复位时序
要求，可控性强，避免LCD 上电瞬间因复位不成功而导致的开机白屏问题。

11

<!-- page: 12 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

1.4 STM32H743IIT6 小系统板硬件资源说明

1. STM32H743IIT6

STM32H743IIT6 是基于Arm Cortex-M7 内核的32 位微控制器，封装为LQFP176，
内置2048KB 的FLASH 及1060KB 的SRAM，主频480MHz。该芯片具有1 个FMC、
140 个IO 口、1 个Quad-SPI、2 个FDCAN、1 个DCMI、2 个SDIO、4 个I2C、3 个16
位ADC、支持JPEG Codec 等等。

2. 3.3V 引出口

开发板引出了一组3.3V 电源输入/输出接口，可以给外部设备供电，也可以从外部

取电源给板子供电。

3. USB-C 转串口

USB-C 转串口接口在开发板上的标号为COM，该接口用于程序下载或串口打印，

还可为开发板供电。

4. 过流保险丝

开发板的电源保险丝，起到过流保护作用。

5. USB-C 通信接口

开发板的USB-C 接口，在开发板上的标号为USB。该接口连接STM32H743IIT6 自
带的USB，可以进行USB 主机或从机通信，也可为开发板供电。

6. 2 个用户按键

开发板的2 个用户按键，在开发板上的标号依次为KEY0、WK_UP。KEY0 是普通
按键；WK_UP 按键用于待机模式下的唤醒，也可当作普通按键使用。

7. 2 个用户LED

开发板的两个用户LED 灯，在开发板上的标号依次为LED0、LED1。LED0 是红
色，LED1 是绿色。在调试代码的时候，可以用LED 灯来指示程序运行状态。

8. 复位按键

开发板的复位按键，在开发板上的标号为RESET，用于MCU 复位。

9. 电源指示灯

开发板的一个蓝色LED 灯，在开发板上的标号为PWR，开发板接通电源就会亮。

10. 5V 引出口

开发板引出了一组5V 电源输入/输出接口，可以给外部设备供电，也可以从外部取

12

<!-- page: 13 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

电源给板子供电。

11. EEPROM

开发板内置EEPROM 芯片，型号为AT24C02，容量为256B。该芯片可用于掉电数

据保存。

12. GPIO 口

开发板引出了88 个GPIO 口，方便用户测试或连接外部模块。

13. LCD1 接口-RGB

开发板的LCD1 接口为40 PIN，支持RGB 接口的LCD 模块，包括4.3/5.0/7.0 寸等
LCD 模块。该接口采用LTDC 驱动RGB 屏，并带有DMA2D（图形加速器）。

14. 16MB FLASH

开发板内置了16MB 的SPI FLASH 芯片，型号为W25Q128，采用Quad-SPI 方式

驱动，读写速度更快。

15. 串口芯片

开发板的串口芯片为CH340X，用于实现USB 转串口功能。

16. TF 卡座

开发板的TF 卡座，可以插入TF 卡，增加数据存储容量。

17. LCD2 接口-SPI

开发板的LCD2 接口为20 PIN 0.5mm pitch 的连接器，采用SPI 接口方式进行显示，
搭配慧勤智远的SPI接口触摸显示模块，可以驱动更高分辨率的RGB接口触摸显示屏，
支持慧勤智远2.4/3.5/4.3/5.0/7.0 寸SPI 接口LCD 模块。

18. M3 机械孔

开发板的4 个角都预留了M3 孔位，可以装上M3 规格的铜柱，使开发板平稳放

置。

19. 电源芯片

开发板的电源芯片是CJA1117B-3.3，该芯片的作用是将5V 的电压转换为3.3V 的

电压，然后给开发板的主控芯片和外设供电。

20. 32MB SDRAM

开发板搭载了一个32MB 的SDRAM 芯片，型号为W9825G6KH-6I，满足大内存

使用的需求。

13

<!-- page: 14 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

21. SWD 调试接口

开发板的SWD 调试接口，在开发板上的标号为SWD。该接口使用SWD 模式进行

调试或下载程序。

14

<!-- page: 15 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

1.5 STM32H743IIT6 小系统板引脚分配

STM32H743IIT6 小系统板的IO 引脚分配如表1.1 所示。

表1.1 STM32H743IIT6 小系统板 IO 引脚分配表

GPIO 引脚编号 引脚功能定义 连接关系说明

PA0 40 WK_UP 按键WK_UP / 待机唤醒脚

PA1 41 KEY0 连接按键KEY0

PA2 42 LCD_R1 LCD1 接口的R1 脚

PA3 47 普通IO 口 /

PA4 50 普通IO 口 /

PA5 51 普通IO 口 /

PA6 52 普通IO 口 /

PA7 53 普通IO 口 /

PA8 119 LCD_B3 LCD 接口的B3 脚

PA9 120 UART1_TX 连接CH340X 的RXD 脚

PA10 121 UART1_RX 连接CH340X 的TXD 脚

PA11 122 USB_D- 连接USB D-引脚

PA12 123 USB_D+ 连接USB D+引脚

PA13 124 SWDIO SWD 仿真接口

PA14 137 SWCLK SWD 仿真接口

PA15 138 普通IO 口 /

PB0 56 LED0 连接LED0（红色）

PB1 57 LED1 连接LED1（绿色）

PB2 58 QSPI_BK1_CLK W25Q128 的CLK 脚

PB3 161 普通IO 口 /

PB4 162 普通IO 口 /

PB5 163 BL_EN LCD1 接口的背光控制脚

PB6 164 QSPI_BK1_CS W25Q128 的CS 脚

PB7 165 普通IO 口 /

PB8 167 普通IO 口 /

PB9 168 普通IO 口 /

PB10 79 普通IO 口 /

PB11 80 TP_MISO LCD1、LCD2 接口的触摸MISO 信号

PB12 92 SPI2_CS LCD2 接口的LT_SCS 脚

15

<!-- page: 16 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

PB13 93 SPI2_SCK LCD2 接口的LT_SCK 脚

PB14 94 SPI2_MISO LCD2 接口的LT_MISO 脚

PB15 95 SPI2_MOSI LCD2 接口的LT_MOSI 脚

PC0 32 FMC_SDNWE 连接SDRAM 的WE 脚

PC1 33 普通IO 口 /

PC2 34 普通IO 口 /

PC3 35 普通IO 口 /

PC4 54 普通IO 口 /

PC5 55 普通IO 口 /

PC6 115 普通IO 口 /

PC7 116 普通IO 口 /

PC8 117 SDIO_D0 TF 卡接口的DAT0 脚

PC9 118 SDIO_D1 TF 卡接口的DAT1 脚

PC10 139 SDIO_D2 TF 卡接口的DAT2 脚

PC11 140 SDIO_D3 TF 卡接口的DAT3 脚

PC12 141 SDIO_CLK TF 卡接口的CLK 脚

PC13 8 普通IO 口 /

PC14 9 OSC32_IN 连接32.768KHz 晶振

PC15 10 OSC32_OUT 连接32.768KHz 晶振

PD0 142 FMC_D2 FMC 总线数据线D2（连接SDRAM 的DQ2 脚）

PD1 143 FMC_D3 FMC 总线数据线D3（连接SDRAM 的DQ3 脚）

PD2 144 SDIO_CMD TF 卡接口的CMD 脚

PD3 145 普通IO 口 /

PD4 146 普通IO 口 /

PD5 147 普通IO 口 /

PD6 150 LCD_B2 LCD1 接口的B2 脚

PD7 151 普通IO 口 /

PD8 96 FMC_D13 FMC 总线数据线的D13（连接SDRAM 的DQ13 脚）

PD9 97 FMC_D14 FMC 总线数据线的D14（连接SDRAM 的DQ14 脚）

PD10 98 FMC_D15 FMC 总线数据线的D15（连接SDRAM 的DQ15 脚）

PD11 99 LCD_RST LCD1、LCD2 接口的复位引脚

PD12 100 TP_SCK LCD1、LCD2 接口的触摸SCK 信号

PD13 101 TP_MOSI LCD1、LCD2 接口的触摸MOSI 信号

PD14 104 FMC_D0 FMC 总线数据线D0（连接SDRAM 的DQ0 脚）

16

<!-- page: 17 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

PD15 105 FMC_D1 FMC 总线数据线D1（连接SDRAM 的DQ1 脚）

PE0 169 FMC_NBL0 FMC 总线NBL0（连接SDRAM 的LDQM 脚）

PE1 170 FMC_NBL1 FMC 总线NBL1（连接SDRAM 的UDQM 脚）

PE2 1 普通IO 口 /

PE3 2 普通IO 口 /

PE4 3 普通IO 口 /

PE5 4 LCD_G0 LCD1 接口的G0 脚

PE6 5 LCD_G1 LCD1 接口的G1 脚

PE7 68 FMC_D4 FMC 总线数据线D4（连接SDRAM 的DQ4 脚）

PE8 69 FMC_D5 FMC 总线数据线D5（连接SDRAM 的DQ5 脚）

PE9 70 FMC_D6 FMC 总线数据线D6（连接SDRAM 的DQ6 脚）

PE10 73 FMC_D7 FMC 总线数据线D7（连接SDRAM 的DQ7 脚）

PE11 74 FMC_D8 FMC 总线数据线D8（连接SDRAM 的DQ8 脚）

PE12 75 FMC_D9 FMC 总线数据线D9（连接SDRAM 的DQ9 脚）

PE13 76 FMC_D10 FMC 总线数据线D10（连接SDRAM 的DQ10 脚）

PE14 77 FMC_D11 FMC 总线数据线D11（连接SDRAM 的DQ11 脚）

PE15 78 FMC_D12 FMC 总线数据线D12（连接SDRAM 的DQ12 脚）

PF0 16 FMC_A0 FMC 总线地址线A0（连接SDRAM 的A0 脚）

PF1 17 FMC_A1 FMC 总线地址线A1（连接SDRAM 的A1 脚）

PF2 18 FMC_A2 FMC 总线地址线A2（连接SDRAM 的A2 脚）

PF3 19 FMC_A3 FMC 总线地址线A3（连接SDRAM 的A3 脚）

PF4 20 FMC_A4 FMC 总线地址线A4（连接SDRAM 的A4 脚）

PF5 21 FMC_A5 FMC 总线地址线A5（连接SDRAM 的A5 脚）

PF6 24 QSPI_BK1_IO3 W25Q128 的HOLD(IO3)脚

PF7 25 QSPI_BK1_IO2 W25Q128 的WP(IO2)脚

PF8 26 QSPI_BK1_IO0 W25Q128 的DI(IO0)脚

PF9 27 QSPI_BK1_IO1 W25Q128 的DO(IO1)脚

PF10 28 LCD_DEN LCD1 接口的DEN 脚

PF11 59 FMC_SDNRAS 连接SDRAM 的RAS 脚

PF12 60 FMC_A6 FMC 总线地址线A6（连接SDRAM 的A6 脚）

PF13 63 FMC_A7 FMC 总线地址线A7（连接SDRAM 的A7 脚）

PF14 64 FMC_A8 FMC 总线地址线A8（连接SDRAM 的A8 脚）

PF15 65 FMC_A9 FMC 总线地址线A9（连接SDRAM 的A9 脚）

PG0 66 FMC_A10 FMC 总线地址线A10（连接SDRAM 的A10 脚）

17

<!-- page: 18 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

PG1 67 FMC_A11 FMC 总线地址线A11（连接SDRAM 的A11 脚）

PG2 106 FMC_A12 FMC 总线地址线A12（连接SDRAM 的A12 脚）

PG3 107 普通IO 口 /

PG4 108 FMC_BS0 连接SDRAM 的BS0 脚

PG5 109 FMC_BS1 连接SDRAM 的BS1 脚

PG6 110 LCD_R7 LCD1 接口的R7 脚

PG7 111 LCD_PCLK LCD1 接口的DCLK 脚

PG8 112 FMC_SDCLK 连接SDRAM 的CLK 脚

PG9 152 普通IO 口 /

PG10 153 普通IO 口 /

PG11 154 普通IO 口 /

PG12 155 LCD_B1 LCD1 接口的B1 脚

PG13 156 LCD_R0 LCD1 接口的R0 脚

PG14 157 LCD_B0 LCD1 接口的B0 脚

PG15 160 FMC_SDNCAS 连接SDRAM 的CAS 脚

PH0 29 OSC_IN 连接25MHz 晶振

PH1 30 OSC_OUT 连接25MHz 晶振

PH2 43 FMC_SDCKE0 连接SDRAM 的CKE 脚

PH3 44 FMC_SDNE0 连接SDRAM 的CS 脚

PH4 45 EE_SCL 连接AT24C02 的SCL 脚

PH5 46 EE_SDA 连接AT24C02 的SDA 脚

PH6 83 TP_CS LCD1、LCD2 接口的触摸CS 信号

PH7 84 TP_IRQ LCD1、LCD2 接口的触摸IRQ 信号

PH8 85 LCD_R2 LCD1 接口的R2 脚

PH9 86 LCD_R3 LCD1 接口的R3 脚

PH10 87 LCD_R4 LCD1 接口的R4 脚

PH11 88 LCD_R5 LCD1 接口的R5 脚

PH12 89 LCD_R6 LCD1 接口的R6 脚

PH13 128 LCD_G2 LCD1 接口的G2 脚

PH14 129 LCD_G3 LCD1 接口的G3 脚

PH15 130 LCD_G4 LCD1 接口的G4 脚

PI0 131 LCD_G5 LCD1 接口的G5 脚

PI1 132 LCD_G6 LCD1 接口的G6 脚

PI2 133 LCD_G7 LCD1 接口的G7 脚

18

<!-- page: 19 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

PI3 134 普通IO 口 /

PI4 173 LCD_B4 LCD1 接口的B4 脚

PI5 174 LCD_B5 LCD1 接口的B5 脚

PI6 175 LCD_B6 LCD1 接口的B6 脚

PI7 176 LCD_B7 LCD1 接口的B7 脚

PI8 7 普通IO 口 /

PI9 11 LCD_VSYNC LCD1 接口的VSYNC 脚

PI10 12 LCD_HSYNC LCD1 接口的HSYNC 脚

PI11 13 普通IO 口 /

19

<!-- page: 20 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

2. STM32H743IIT6 小系统板硬件电路介绍

1. MCU

开发板采用STM32H743IIT6 作为主控芯片，MCU 原理图如图2.1 所示。

图2.1 STM32H743IIT6 小系统板MCU 原理图

芯片的大部分IO 口连接了外设，查看原理图文档可知其功能定义。芯片的PC14
和PC15 连接了32.768KHz 的晶振，用于实现RTC 功能；芯片的PH0 和PH1 连接了
25MHz 的晶振，用于给系统提供时钟源；芯片采用3.3V 供电。
VBAT 为主控芯片的电池输入端，默认由VDD 供电。图2.1 中的D1（BAT54C）
双向二极管隔离了VDD 和VBAT，VBAT 已在开发板上引出，用户可以外接纽扣电池

为其供电。

VREFP 为主控芯片的参考电压输入端，ADC 采集时，VREFP 必须给定一个参考电
压。VREF 已在开发板上引出，用户可自行选择参考电压，一般选择3.3V。

20

<!-- page: 21 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

BOOT0 接地，开发板默认从芯片的FLASH 地址0x0800 0000 启动。

2. TF CARD

TF CARD 的电路如图2.2 所示。

图2.2 TF CARD 原理图

开发板的TF 卡接口采用SDIO 模式驱动，读写速度比SPI 模式更快。
SDIO_D0/SDIO_D1/SDIO_D2/SDIO_D3/SDIO_CLK/SDIO_CMD 连接在主控芯片的
PC8/PC9/PC10/PC11/PC12/PD2。

3. QSPI FLASH

SPI FLASH 芯片型号为W25Q128，容量为16MB，该部分电路如图2.3 所示。

图2.3 QSPI FLASH 原理图

W25Q128 使用了主控芯片的Quad-SPI 方式驱动，读写速度比普通SPI 更快。

4. EEPROM

EEPROM 芯片型号为AT24C02，容量为256B。该部分电路如图2.4 所示。

21

<!-- page: 22 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

图2.4 EEPROM 原理图

AT24C02 的A0~A2 均接地，A0、A1 和A2 为0，则读操作地址为0XA1，写操作
地址为0XA0。EE_SCL 连接在主控芯片的PH4 上，EE_SDA 连接在PH5 上，这两根信
号线都接了4.7KΩ 上拉电阻，保证通信的稳定。

5. KEY

开发板按键的电路如图2.5 所示。

图2.5 KEY 原理图

WK_UP 连接在主控芯片的PA0 上，KEY0 连接在PA1 上。WK_UP 用作开发板的
待机唤醒输入或普通按键输入，KEY0 用作普通按键输入。NRST 连接在主控芯片的复
位引脚，当按下RESET 按键，系统复位。该NRST 引脚是系统专用的复位引脚，并未
用于LCD 屏的复位。

22

<!-- page: 23 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

6. USB SLAVE/HOST

USB SLAVE/HOST 电路如图2.6 所示。

图2.6 USB SLAVE/HOST 原理图

USB_D+/USB_D-连接在主控芯片的PA12/PA11 上，该接口可以实现USB 主/从机

通信，还可为开发板供电。

7. USB UART

USB UART 的电路如图2.7 所示。

图2.7 USB UART 原理图

该电路使用了CH340X 芯片实现USB 转串口功能，可进行串口通信或程序下载。

23

<!-- page: 24 -->
## Visual Summary (Page 24)

- page_class: mixed_visual
- confidence: 0.95
- reason_codes: heuristic_page_type_text, heuristic_content_type_text, mixed_visual, sparse_text

Page contains two circuit diagrams: Figure 2.8 showing the SDRAM (W9825G6KH-6I, 32MB) interface via FMC controller with pin mapping and power supply; Figure 2.9 showing three LED indicators (red/green/blue) connected to GPIO pins PB0/PB1 with current-limiting resistors.

<!-- page: 25 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

PWR 是开发板的电源指示灯，为蓝色。LED0 连接在主控芯片的PB0 上，LED1 连
接在PB1 上。LED0 为红色，LED1 为绿色。

10. SWD

SWD 调试接口的电路如图2.10 所示。

图2.10 SWD 原理图

SWDIO/SWCLK 连接在主控芯片的PA13/PA14 上，该接口使用SWD 模式进行调

试和下载程序。

11. GPIO

开发板引出了两排排针，为P1 和P2，该部分电路如图2.11 所示。

图2.11 GPIO 原理图

图2.11 中，引出了一组3.3V 和一组5V 电源输入/输出口。引出了88 个IO 口，同
时引出了芯片的VBAT、 BOOT0、VREF、NRST，方便测试。

25

<!-- page: 26 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

12. POWER

系统的电源电路如图2.12 所示。

图2.12 POWER 原理图

开发板使用USB 进行供电，电源芯片采用了CJA1117B-3.3，作用是将5V 电压转
换成3.3V，给开发板的芯片和外设供电。

13. LCD1-RGB INTERFACE

LCD1 接口的电路如图2.13 所示。

图2.13 LCD1 接口原理图

LCD1 接口为40 PIN，支持RGB 接口的LCD 模块，支持565/888 模式显示。BL_EN
连接在主控芯片的PB5 上，用于控制LCD 的背光。LCD_RST 连接在主控芯片的PD11
上，用单独的IO 口控制屏幕复位。该接口采用LTDC 驱动RGB 屏，并带有DMA2D

（图形加速器）。

26

<!-- page: 27 -->
STM32H743IIT6 Core Board V1.1 用户手册

东莞市慧勤智远科技有限公司

14. LCD2-SPI INTERFACE

LCD2接口为20PIN，采用SPI+I2C方式进行图像显示及触摸，该部分电路如图2.14所示。

LCD2-SPI interface

<table><tr><td>SPI2_CS</td><td>PB12</td></tr><tr><td>SPI2_SCK</td><td>PB13</td></tr><tr><td>SPI2_MISO</td><td>PB14</td></tr><tr><td>SPI2_MOSI</td><td>PB15</td></tr><tr><td>LCD_RST</td><td>PD11</td></tr><tr><td>TP_IRQ</td><td>PH7</td></tr><tr><td>TP_CS</td><td>PH6</td></tr><tr><td>TP_SCK</td><td>PD12</td></tr><tr><td>TP_MISO</td><td>PB11</td></tr><tr><td>TP_MOSI</td><td>PD13</td></tr></table>

图2.14 LCD2接口原理图

SPI2_CS/SPI2_SCK/SPI2_MISO/SPI2_MOSI 连接在主控芯片的PB12/PB13/PB14/PB15上，LCD_RST/TP_IRQ/TP_CS/TP_SCK/TP_MISO/TP_MOSI连接在主控芯片的PD11/PH7/PH6/PD12/PB11/PD13上。该接口支持3线SPI和4线SPI的LCD模块，可接慧勤智远2.4/3.5/4.3/5.0/7.0寸SPI接口LCD模块，其信号引脚与主控芯片对应的连接关系如表2.1所示。

表 2.13 线和 4 线 SPI LCD 模块与主控芯片连接关系

<table><tr><td>PIN NO.</td><td>MCU PIN</td><td>3 wire SPI</td><td>4 wire SPI</td><td>引脚功能说明</td></tr><tr><td>5</td><td>PB12</td><td>CS</td><td>CS</td><td>SPI 片选信号</td></tr><tr><td>6</td><td>PB15</td><td>NC</td><td>MOSI</td><td>4 线 SPI: MOSI</td></tr><tr><td>7</td><td>PB13</td><td>SCLK</td><td>SCLK</td><td>SPI 串口时钟信号</td></tr><tr><td>10</td><td>PB14</td><td>SDA</td><td>MISO</td><td>3 线 SPI: 数据输入/输出; 4 线 SPI: MISO</td></tr><tr><td>9</td><td>PD11</td><td>RESET</td><td>RESET</td><td>复位信号</td></tr></table>

LCD2接口的引脚功能定义如表2.2所示。

27

WKS慧勤智远

<!-- page: 28 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

表2.2 LCD2 接口的引脚功能定义

引脚编号 引脚定义 引脚功能说明

1 VCC5 5V 电源

2 VCC5 5V 电源

3 GND 电源地

4 GND 电源地

5 SPI2_CS SPI 片选信号

6 SPI2_MOSI 3 线SPI：NC；4 线SPI：MOSI 信号

7 SPI2_SCK SPI 串口时钟信号

8 NC NC

9 LCD_RST LCD 复位信号

10 SPI2_MISO 3 线SPI：数据输入/输出引脚；4 线SPI：MISO 信号

11 TP_IRQ 电阻触摸屏：笔接触中断引脚；电容触摸屏：INT 信号

12 TP_SCK 电阻触摸屏：时钟信号输出；电容触摸屏：SCL 信号

13 TP_MOSI 电阻触摸屏：串行数据输出端；电容触摸屏：SDA 信号

14 TP_CS 电阻触摸屏：片选信号；电容触摸屏：复位信号

15 TP_MISO 电阻触摸屏：串行数据输入端；电容触摸屏：NC

16 GND 电源地

17~19 NC NC

20 GND 电源地

28

<!-- page: 29 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

3. 开发板软件使用方法

开发板主要用到三个软件工具，一个是代码编写和下载的工具MDK5，一个是程序
下载工具STM32CubeProgrammer，还有一个是串口调试助手sscom。

目前测试仿真器下载程序的情况如下：

建议开发板搭配DAP-Link 仿真器使用。

3.1 MDK5 安装与使用

3.1.1 MDK5 安装

（1）软件安装包路径“慧勤智远 STM32H743IIT6 小系统板\9. 软件开发工具

\MDK5.zip”，解压文件，双击 打开，点击Next。

图3.1 MDK5 安装界面

29

<!-- page: 30 -->
## Visual Summary (Page 30)

- page_class: mixed
- confidence: 0.95
- reason_codes: mixed_content, diagram_dominant

Page 30 of the STM32H743IIT6 Core Board V1.1 User Manual contains installation instructions for ARM Keil MDK-ARM software. It features two screenshots: Figure 3.2 shows the License Agreement screen where the user must check 'I agree to all the terms' and click Next; Figure 3.3 displays the Folder Selection screen, instructing users to select paths for Core and Pack installations (noting that Chinese characters should be avoided in paths) before clicking Next.

<!-- page: 31 -->
## Visual Summary (Page 31)

- page_class: mixed
- confidence: 0.95
- reason_codes: visual_content_dominant, instructional_flow

Page 31 of the STM32H743IIT6 Core Board V1.1 User Manual contains instructions for software installation. It features two screenshots: Figure 3.4 shows the 'Customer Information' input screen in Keil MDK-ARM Setup, and Figure 3.5 displays the progress bar during file installation.

<!-- page: 32 -->
STM32H743IIT6 Core Board V1.1 用户手册

东莞市慧勤智远科技有限公司

（6）软件安装完成，点击Finish。

图3.6

(7) 弹出支持包更新对话框, 可直接退出, 安装离线包。

图3.7

32

WKS慧勤智远

<!-- page: 33 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

（8）软件安装完之后，电脑桌面出现 图标。打开路径“慧勤智远
STM32H743IIT6 小系统板\9. 软件开发工具\MDK5”，双击

安装STM32H7xx 支持包，点击Next，开始安装。

图3.8

（9）安装完成，点击Finish。

图3.9

注意：提供的MDK 安装包为官方正版，用户需要激活才能正常使用。

33

<!-- page: 34 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

3.1.2 MDK5 打开工程

用MDK5 软件打开工程有两种方法。
方法1：

（1）双击打开 软件，点击“Open…”图标。

图3.10

（2）选择文件类型“Project Files”。

图3.11

（3）找到工程所在路径，找到xxx.uvprojx 打开即可。

图3.12

方法2：
在工程目录下找到xxx.uvprojx，双击打开即可（默认用Keil uVision5 打开）。

34

<!-- page: 35 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

3.1.3 MDK5 相关设置

在使用MDK5 时，通常需要设置一些参数，才能更好地使用该软件。（提示：在

打开工程后才可以设置）

（1）点击 魔术棒，设置相关参数。

图3.13

（2）首先，选择开发板的芯片型号（根据实际型号选择）。

35

<!-- page: 36 -->
## STM32H743IIT6 Core Board V1.1 用户手册

**东莞市慧勤智远科技有限公司**

### 图 3.14

（3）选择 AC5 编译器。

![图 3.14](#)

### 图 3.15

36

![WKS 慧勤智远](#)

<!-- page: 37 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

（4）勾选“Create HEX File”，编译工程时生成可执行文件（后缀名为hex）；
勾选“Browse Information”，可跳转查看变量或函数的定义。

图3.16

以上参数设置完之后，点击“OK”返回。

3.1.4 MDK5 仿真调试设置

通常，我们使用仿真器进行程序下载或调试，下面以DAP-Link 仿真器为例设置
MDK5 的相关参数。（提示：在打开工程后才可以设置）

（1）点击 魔术棒，打开后，点击Debug，选择CMSIS-DAP Link Debugger，

再点击Settings。

37

<!-- page: 38 -->
STM32H743IIT6 Core Board V1.1 用户手册

东莞市慧勤智远科技有限公司

图3.17

（2）设置仿真器参数，选择“DAPLink CMSIS-DAP”，选择SW模式，调试速度选择10MHz。

图3.18

38

WKS慧勤智远

<!-- page: 39 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

（3）继续设置仿真器参数，勾选“Reset and Run”，选择芯片下载算法（根据实
际选取，点击Add 增加，点击Remove 移除）。

图3.19

以上参数设置完之后，点击“确定”，再点击“OK”返回。

（4）返回工程后，点击 编译代码（已编译则无需再次编译），然后点击 下

载程序。

图3.20

39

<!-- page: 40 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

3.2 串口程序下载（STM32CubeProgrammer）

（1）安装CH340 驱动。软件路径“慧勤智远 STM32H743IIT6 小系统板\9. 软件

开发工具\CH340 驱动.ZIP”，解压文件，双击 ，点击安装。

图3.21

（2）安装成功，点击确定返回，再退出安装界面。

图3.22

（3）安装STM32CubeProgrammer。软件路径“慧勤智远 STM32H743IIT6 小系
统板\9. 软件开发工具\en.stm32cubeprg-win64_v2-6-0.zip”，解压文件，双击

开始安装。点击Next。

40

<!-- page: 41 -->
## Visual Summary (Page 41)

- page_class: mixed
- confidence: 0.95
- reason_codes: mixed_visual, diagram_dominant

Page contains two screenshots of the STM32CubeProgrammer installation wizard (Step 1 and Step 2) illustrating software features for microcontrollers.

<!-- page: 42 -->
## Visual Summary (Page 42)

- page_class: mixed
- confidence: 0.95
- reason_codes: mixed_content, diagram_heavy

Page 42 of the STM32H743IIT6 Core Board V1.1 User Manual contains installation instructions for the STM32CubeProgrammer software. It features two screenshots: Figure 3.25 showing Step 3 where the user must accept the license agreement, and Figure 3.26 showing Step 4 where the user selects the installation path.

<!-- page: 43 -->
## Visual Summary (Page 43)

- page_class: mixed
- confidence: 0.95
- reason_codes: heuristic_reason_primarily_text_content, route_hint_high_quality_mode_reviews_sparse_pages_before_accepting_native_text

Page 43 of the STM32H743IIT6 Core Board V1.1 User Manual contains installation instructions for software drivers, accompanied by two screenshots (Figures 3.27 and 3.28) showing the 'STM32CubeProgrammer Installation Wizard' interface at different steps.

<!-- page: 44 -->
## Visual Summary (Page 44)

- page_class: mixed
- confidence: 0.95
- reason_codes: mixed_content, diagram_dominant

Page displays two screenshots of a Device Driver Installation Wizard interface (Figures 3.29 and 3.30) illustrating the driver installation process steps.

<!-- page: 45 -->
## Visual Summary (Page 45)

- page_class: mixed
- confidence: 0.95
- reason_codes: heuristic_reason_primarily_text_content, route_hint_high_quality_mode_reviews_sparse_pages_before_accepting_native_text

Page 45 of the STM32H743IIT6 Core Board V1.1 User Manual contains installation wizard screenshots (Figures 3.31 and 3.32) showing steps to complete package installation and configure shortcuts, accompanied by Chinese instructional text.

<!-- page: 46 -->
## Visual Summary (Page 46)

- page_class: mixed
- confidence: 0.95
- reason_codes: heuristic_page_type_text, heuristic_content_type_text, mixed_visual

Page 46 of the STM32H743IIT6 Core Board V1.1 User Manual contains instructional text for software installation and ISP mode setup, accompanied by two figures: Figure 3.33 showing the STM32CubeProgrammer installation completion screen with a 'Done' button highlighted, and Figure 3.34 displaying a development board connected via USB-C to a computer's Device Manager interface highlighting the detected USB-SERIAL CH340 (COM3) port.

<!-- page: 47 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

（14）点击桌面图标 打开STM32CubeProgrammer。选择“UART”；

Port：选择识别到的串口端口（USB-SERIAL CH340）；Baudrate：串口波特率（选择
115200）；Parity：Even。点击“Connect”。

图3.35

（15）连接成功，显示相关信息。点击 。

47

<!-- page: 48 -->
## STM32H743IIT6 Core Board V1.1 用户手册

**东莞市慧勤智远科技有限公司**

### 图 3.36

(16) 点击 “Browse”，找到需要下载的 hex 文件。勾选 “Verify programming” 和 “Run after programming”。点击 “Start programming”，程序开始下载。

### 图 3.37

48

<!-- page: 49 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

（17）程序下载成功，自动运行。注意：程序运行后，开发板不工作在ISP 模
式，如果想要再次下载程序，请重新按照步骤13、14 操作。

图3.38

49

<!-- page: 50 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

3.3 串口调试（SSCOM）

（1）安装CH340 驱动。（参考3.2 节，已安装则忽略）
（2）电脑连接开发板的USB-C 转串口接口，打开电脑的设备管理器可以看到
USB 串口。（参考3.2 节）
（3）打开sscom 软件。软件路径“慧勤智远 STM32H743IIT6 小系统板\9. 软件

开发工具\串口调试助手.zip”，解压文件，双击 打开。

（4）设置相关参数。串口号：选择识别到的串口端口（USB-SERIAL CH340）；
波特率：要与代码中设置一致（本实验用115200）。勾选DTR。最后打开串口，就

可以进行串口通信。

图3.39

（5）如需向串口发送数据，则要勾选发送新行，然后输入数据，点击发送即可。

50

<!-- page: 51 -->
## Visual Summary (Page 51)

- page_class: chart
- confidence: 0.95
- reason_codes: visual_content_dominant, annotated_diagram, policy_visual_summary_override

Figure 3.40 displays a screenshot of the SSCom 3.3 serial communication software interface. The image highlights specific elements: the input field showing '123456', the 'Send' button, and the status bar indicating COM3 is open at 115200bps.

<!-- page: 52 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

3.4 DFU 程序下载（STM32CubeProgrammer）

（1）安装DFU 驱动。软件路径“慧勤智远 STM32H743IIT6 小系统板\9. 软件开

发工具\STSW_STM32080_V3.0.6.zip”，解压文件，双击

打开。点击Next。

图3.41

（2）填写信息，可以随意填写。

图3.42

52

<!-- page: 53 -->
## Visual Summary (Page 53)

- page_class: mixed
- confidence: 0.95
- reason_codes: mixed_content, diagram_heavy

Page 53 of the STM32H743IIT6 Core Board V1.1 User Manual contains installation instructions for DfuSe v3.0.6. It features two screenshots: Figure 3.43 showing the 'Ready to Install' dialog with an highlighted 'Install' button, and Figure 3.44 showing the progress window during installation.

<!-- page: 54 -->
## Visual Summary (Page 54)

- page_class: mixed
- confidence: 0.95
- reason_codes: heuristic_page_type, mixed_visual

Page 54 of the STM32H743IIT6 Core Board V1.1 User Manual contains installation instructions for DfuSe and STM32CubeProgrammer software, accompanied by two screenshots (Figures 3.45 and 3.46) showing the 'InstallShield Wizard Completed' dialog and the main interface of the STM32CubeProgrammer application.

<!-- page: 55 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

（7）电脑连接开发板的USB-C 通信接口，将开发板的BOOT0 与3.3V 短接，然
后按一次复位键，开发板进入DFU 模式。打开电脑的设备管理器可以看到DFU 设
备。（电脑识别到DFU 设备后，可将开发板的BOOT0 与3.3V 断开）

图3.47

（8）点击桌面图标 打开STM32CubeProgrammer。选择“USB”，会自

动识别到USB 端口，点击“Connect”。

图3.48

55

<!-- page: 56 -->
STM32H743IIT6 Core Board V1.1 用户手册

东莞市慧勤智远科技有限公司

(9) 连接成功，显示相关信息。点击 📄 。

图3.49

(10) 点击 “Browse”，找到需要下载的 hex 文件。勾选 “Verify programming” 和 “Run after programming”。点击 “Start programming”，程序开始下载。

56

WKS慧勤智远

<!-- page: 57 -->
STM32H743IIT6 Core Board V1.1 用户手册

东莞市慧勤智远科技有限公司

图3.50

（11）程序下载成功，自动运行。注意：程序运行后，开发板不工作在DFU模式，如果想要再次下载程序，请重新按照步骤7、8操作。

图3.51

57

WKS慧勤智远

<!-- page: 58 -->
STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

4. STM32H743IIT6 小系统板例程列表

STM32H743IIT6 小系统板提供了丰富的例程，可供用户参考学习。

表4.1 基础例程

基础例程（寄存器版本）

1. Template 工程模板-新建工程使用 26. 内存管理实验

2. 跑马灯实验 27. SD 卡实验

3. 按键输入实验 28. FATFS 实验

4. 外部中断实验 29. 汉字显示实验

5. 串口通信实验 30. 图片显示实验

6. 独立看门狗实验 31. 硬件JPEG 解码实验

7. 窗口看门狗实验 32. 视频播放器实验

8. 定时器中断实验 33. FPU 测试（Julia 分形）实验

9. PWM 输出实验 34. DSP 测试实验

10. 输入捕获实验 35. 手写识别实验

11. 内存保护（MPU）实验 36. T9 拼音输入法实验

12. SDRAM 实验 37. 串口IAP 实验

13. LTDC LCD（RGB 屏）实验 38. USB 读卡器(Slave)实验

14. USMART 调试实验 39. USB 虚拟串口(Slave)实验

15. RTC 实时时钟实验 40. USB U 盘(Host)实验

16. 硬件随机数实验 41. USB 鼠标键盘(Host)实验

17. 待机模式实验 42. UCOSII 实验1-任务调度

18. DMA 实验 43. UCOSII 实验2-信号量和邮箱

44. UCOSII 实验3-消息队列、信号量集和
19. ADC 实验
软件定时器

20. 内部温度传感器实验 45. 7680 模块显示实验

21. DAC 实验 46. 7680 模块触摸屏实验

22. 软件模拟IIC 读写EEPROM 实验 47. TFTLCD（2.4&3.5 寸SPI 屏）实验

23. QSPI FLASH 实验 48.（2.4&3.5 寸SPI 屏）触摸屏实验

24. 触摸屏实验 49. 综合测试实验（寄存器版本）

25. FLASH 模拟EEPROM 实验

58
