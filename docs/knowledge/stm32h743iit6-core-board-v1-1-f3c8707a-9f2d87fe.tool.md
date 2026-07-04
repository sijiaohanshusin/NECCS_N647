STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## STM32H743IIT6 Core Board V1.1

## 用户手册

**==> picture [401 x 314] intentionally omitted <==**

**==> picture [111 x 16] intentionally omitted <==**

1

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## 前言

慧勤智远 STM32H743IIT6 小系统板采用了 STM32H743IIT6 作为主控芯片，该芯 片是 STM32 MCU 家族主流的 32 位微控制器，主频 480MHz，Arm[®] Cortex[®] -M7 内核， LQFP176pin 封装。

STM32H743IIT6 小系统板长为 85mm，宽为 55mm，厚为 1.6mm，体型小巧，采用 稳定可靠的 4 层沉金工艺，经久耐用。本手册对 STM32H743IIT6 小系统板的硬件资源 进行阐述，以及介绍开发板软件工具的使用，让读者快速上手开发板，进行实践应用。

**==> picture [111 x 16] intentionally omitted <==**

2

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## 更新记录

|版本|日期|更新说明|
|---|---|---|
|V1.0|2024-12-31|初始版本|
|V1.1|2025-04-09|优化内容|
||||
||||
||||
||||
||||
||||
||||
||||
||||
||||



**==> picture [111 x 16] intentionally omitted <==**

3

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## 目录

前言 ........................................................................................................................................... 2 更新记录 ................................................................................................................................... 3 目录 ........................................................................................................................................... 4 1. STM32H743IIT6 小系统板配置 .......................................................................................... 5 1.1 STM32H743IIT6 小系统板视图 ........................................................................ 5 1.2 STM32H743IIT6 小系统板位号图 .................................................................... 7 1.3 STM32H743IIT6 小系统板硬件资源总述 ........................................................ 9 1.4 STM32H743IIT6 小系统板硬件资源说明 ...................................................... 12 1.5 STM32H743IIT6 小系统板引脚分配 .............................................................. 15 2. STM32H743IIT6 小系统板硬件电路介绍 ........................................................................ 20 3. 开发板软件使用方法 ........................................................................................................ 29 3.1 MDK5 安装与使用 ........................................................................................... 29 3.1.1 MDK5 安装 ............................................................................................ 29 3.1.2 MDK5 打开工程 .................................................................................... 34 3.1.3 MDK5 相关设置 .................................................................................... 35 3.1.4 MDK5 仿真调试设置 ............................................................................ 37 3.2 串口程序下载（STM32CubeProgrammer） .................................................. 40 3.3 串口调试（SSCOM） ..................................................................................... 50 3.4 DFU 程序下载（STM32CubeProgrammer） .................................................. 52 4. STM32H743IIT6 小系统板例程列表 ................................................................................ 58

**==> picture [111 x 16] intentionally omitted <==**

4

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **1. STM32H743IIT6** 小系统板配置

## **1.1 STM32H743IIT6** 小系统板视图

**==> picture [461 x 497] intentionally omitted <==**

**----- Start of picture text -----**<br>
85mm<br>55mm<br>**----- End of picture text -----**<br>


## 图 **1.1 STM32H743IIT6** 小系统板顶层视图

**==> picture [111 x 16] intentionally omitted <==**

5

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [461 x 530] intentionally omitted <==**

**----- Start of picture text -----**<br>
79mm<br>P2.54*24=60.96mm<br>49mm  18=45.72mm<br>*<br>P2.54<br>**----- End of picture text -----**<br>


## 图 **1.2 STM32H743IIT6** 小系统板底层视图

**==> picture [111 x 16] intentionally omitted <==**

6

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **1.2 STM32H743IIT6** 小系统板位号图

**==> picture [451 x 293] intentionally omitted <==**

## 图 **1.3 STM32H743IIT6** 小系统板正面位号图

**==> picture [111 x 16] intentionally omitted <==**

7

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [451 x 295] intentionally omitted <==**

## 图 **1.4 STM32H743IIT6** 小系统板背面位号图

**==> picture [111 x 16] intentionally omitted <==**

8

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **1.3 STM32H743IIT6** 小系统板硬件资源总述

STM32H743IIT6 小系统板采用 STM32H743IIT6 作为主控芯片，STM32H743IIT6 小系统板的资源分 布如图 1.5 所示。

**==> picture [540 x 508] intentionally omitted <==**

**----- Start of picture text -----**<br>
串口芯片  16MB FLASH  GPIO 口<br>3.3V 引出口<br>USB-C 转串口<br>过流保险丝<br>USB-C 通信接口  STM32H743IIT6<br>2 个用户按键<br>LCD1 接口-RGB<br>2 个用户LED<br>复位按键<br>电源指示灯  5V 引出口  EEPROM  GPIO 口<br>**----- End of picture text -----**<br>


**a) STM32H743IIT6** 小系统板顶层资源

**==> picture [111 x 16] intentionally omitted <==**

9

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [97 x 100] intentionally omitted <==**

**----- Start of picture text -----**<br>
TF 卡座<br>LCD2 接口-SPI<br>**----- End of picture text -----**<br>


**==> picture [169 x 168] intentionally omitted <==**

**----- Start of picture text -----**<br>
M3 机械孔<br>电源芯片<br>32MB SDRAM<br>SWD 调试接口<br>**----- End of picture text -----**<br>


## **b) STM32H743IIT6** 小系统板底层资源

## 图 **1.5 STM32H743IIT6** 小系统板资源分布图

**==> picture [111 x 16] intentionally omitted <==**

10

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## STM32H743IIT6 小系统板的资源汇总如下表：

|资源|数量|说明|
|---|---|---|
|MCU|1个|STM32H743IIT6，内置FLASH: 2048KB，内置SRAM: 1060KB|
|3.3V引出口|1个|用于接入3.3V电源或对外提供3.3V电压|
|USB-C转串口|1个|用于USB转TTL串口通信|
|过流保险丝|1个|保险丝，起过流保护作用|
|USB-C通信接口|1个|用于USB SLAVE / HOST通信|
|用户按键|2个|KEY0、WK_UP（具备唤醒功能）|
|用户LED|2个|LED0（红色），LED1（绿色）|
|复位按键|1个|RESET，用于MCU复位|
|电源指示灯|1个|PWR（蓝色）|
|5V引出口|1个|用于接入5V电源或对外提供5V电压|
|EEPROM|1个|AT24C02，256B|
|GPIO口|88个|共引出88个IO口，用于连接外部模块|
|LCD1接口-RGB|1个|40PIN，支持4.3/5.0/7.0寸RGB接口屏|
|16MB FLASH|1个|W25Q128，16MB，采用Quad-SPI驱动|
|串口芯片|1个|CH340X，用于实现USB转串口功能|
|TF卡座|1个|用于TF卡扩展|
|LCD2接口-SPI|1个|20PIN，支持2.4/3.5/4.3/5.0/7.0寸SPI接口屏|
|M3机械孔|4个|可装M3铜柱，使开发板平稳放置|
|电源芯片|1个|CJA1117B-3.3，将5V电压转化为3.3V电压|
|32MB SDRAM|1个|W9825G6KH-6I，32MB|
|SWD调试接口|1个|用于仿真调试、程序下载等|



以上就是 STM32H743IIT6 小系统板的板载资源，相比于同类型的开发板， STM32H743IIT6 小系统板具有以下优点：

- A. 独有的 20 PIN LCD 接口，搭配慧勤智远的 SPI 接口触摸显示屏模块，可以驱动 更高分辨率的 RGB 接口触摸显示屏（例 4.3 寸/5.0 寸/7.0 寸）。

- B. LCD 复位引脚采用独立的 GPIO 口控制，可以满足不同 LCD 驱动 IC 复位时序 要求，可控性强，避免 LCD 上电瞬间因复位不成功而导致的开机白屏问题。

**==> picture [111 x 16] intentionally omitted <==**

11

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **1.4 STM32H743IIT6** 小系统板硬件资源说明

## **1. STM32H743IIT6**

STM32H743IIT6 是基于 Arm Cortex-M7 内核的 32 位微控制器，封装为 LQFP176， 内置 2048KB 的 FLASH 及 1060KB 的 SRAM，主频 480MHz。该芯片具有 1 个 FMC、 140 个 IO 口、1 个 Quad-SPI、2 个 FDCAN、1 个 DCMI、2 个 SDIO、4 个 I2C、3 个 16 位 ADC、支持 JPEG Codec 等等。

## **2. 3.3V** 引出口

开发板引出了一组 3.3V 电源输入/输出接口，可以给外部设备供电，也可以从外部 取电源给板子供电。

## **3. USB-C** 转串口

USB-C 转串口接口在开发板上的标号为 COM，该接口用于程序下载或串口打印， 还可为开发板供电。

**4.** 过流保险丝

开发板的电源保险丝，起到过流保护作用。

## **5. USB-C** 通信接口

开发板的 USB-C 接口，在开发板上的标号为 USB。该接口连接 STM32H743IIT6 自 带的 USB，可以进行 USB 主机或从机通信，也可为开发板供电。

## **6. 2** 个用户按键

开发板的 2 个用户按键，在开发板上的标号依次为 KEY0、WK_UP。KEY0 是普通 按键；WK_UP 按键用于待机模式下的唤醒，也可当作普通按键使用。

## **7. 2** 个用户 **LED**

- 开发板的两个用户 LED 灯，在开发板上的标号依次为 LED0、LED1。LED0 是红

- 色，LED1 是绿色。在调试代码的时候，可以用 LED 灯来指示程序运行状态。

   **8.** 复位按键

开发板的复位按键，在开发板上的标号为 RESET，用于 MCU 复位。

## **9.** 电源指示灯

开发板的一个蓝色 LED 灯，在开发板上的标号为 PWR，开发板接通电源就会亮。

## **10. 5V** 引出口

开发板引出了一组 5V 电源输入/输出接口，可以给外部设备供电，也可以从外部取

**==> picture [111 x 16] intentionally omitted <==**

12

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

电源给板子供电。

## **11. EEPROM**

开发板内置 EEPROM 芯片，型号为 AT24C02，容量为 256B。该芯片可用于掉电数 据保存。

## **12. GPIO** 口

开发板引出了 88 个 GPIO 口，方便用户测试或连接外部模块。

## **13. LCD1** 接口 **-RGB**

开发板的 LCD1 接口为 40 PIN，支持 RGB 接口的 LCD 模块，包括 4.3/5.0/7.0 寸等 LCD 模块。该接口采用 LTDC 驱动 RGB 屏，并带有 DMA2D（图形加速器）。

## **14. 16MB FLASH**

开发板内置了 16MB 的 SPI FLASH 芯片，型号为 W25Q128，采用 Quad-SPI 方式 驱动，读写速度更快。

## **15.** 串口芯片

开发板的串口芯片为 CH340X，用于实现 USB 转串口功能。

## **16. TF** 卡座

开发板的 TF 卡座，可以插入 TF 卡，增加数据存储容量。

## **17. LCD2** 接口 **-SPI**

开发板的 LCD2 接口为 20 PIN 0.5mm pitch 的连接器，采用 SPI 接口方式进行显示， 搭配慧勤智远的 SPI 接口触摸显示模块，可以驱动更高分辨率的 RGB 接口触摸显示屏， 支持慧勤智远 2.4/3.5/4.3/5.0/7.0 寸 SPI 接口 LCD 模块。

## **18. M3** 机械孔

开发板的 4 个角都预留了 M3 孔位，可以装上 M3 规格的铜柱，使开发板平稳放 置。

## **19.** 电源芯片

开发板的电源芯片是 CJA1117B-3.3，该芯片的作用是将 5V 的电压转换为 3.3V 的 电压，然后给开发板的主控芯片和外设供电。

## **20. 32MB SDRAM**

开发板搭载了一个 32MB 的 SDRAM 芯片，型号为 W9825G6KH-6I，满足大内存 使用的需求。

**==> picture [111 x 16] intentionally omitted <==**

13

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **21. SWD** 调试接口

开发板的 SWD 调试接口，在开发板上的标号为 SWD。该接口使用 SWD 模式进行 调试或下载程序。

**==> picture [111 x 16] intentionally omitted <==**

14

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **1.5 STM32H743IIT6** 小系统板引脚分配

STM32H743IIT6 小系统板的 IO 引脚分配如表 1.1 所示。

表 **1.1 STM32H743IIT6** 小系统板 **IO** 引脚分配表

|GPIO|引脚编号|引脚功能定义|连接关系说明|
|---|---|---|---|
|PA0|40|WK_UP|按键WK_UP /待机唤醒脚|
|PA1|41|KEY0|连接按键KEY0|
|PA2|42|LCD_R1|LCD1接口的R1脚|
|PA3|47|普通IO口|/|
|PA4|50|普通IO口|/|
|PA5|51|普通IO口|/|
|PA6|52|普通IO口|/|
|PA7|53|普通IO口|/|
|PA8|119|LCD_B3|LCD接口的B3脚|
|PA9|120|UART1_TX|连接CH340X的RXD脚|
|PA10|121|UART1_RX|连接CH340X的TXD脚|
|PA11|122|USB_D-|连接USB D-引脚|
|PA12|123|USB_D+|连接USB D+引脚|
|PA13|124|SWDIO|SWD仿真接口|
|PA14|137|SWCLK|SWD仿真接口|
|PA15|138|普通IO口|/|
|PB0|56|LED0|连接LED0（红色）|
|PB1|57|LED1|连接LED1（绿色）|
|PB2|58|QSPI_BK1_CLK|W25Q128的CLK脚|
|PB3|161|普通IO口|/|
|PB4|162|普通IO口|/|
|PB5|163|BL_EN|LCD1接口的背光控制脚|
|PB6|164|QSPI_BK1_CS|W25Q128的CS脚|
|PB7|165|普通IO口|/|
|PB8|167|普通IO口|/|
|PB9|168|普通IO口|/|
|PB10|79|普通IO口|/|
|PB11|80|TP_MISO|LCD1、LCD2接口的触摸MISO信号|
|PB12|92|SPI2_CS|LCD2接口的LT_SCS脚|



**==> picture [111 x 16] intentionally omitted <==**

15

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

|PB13|93|SPI2_SCK|LCD2接口的LT_SCK脚|
|---|---|---|---|
|PB14|94|SPI2_MISO|LCD2接口的LT_MISO脚|
|PB15|95|SPI2_MOSI|LCD2接口的LT_MOSI脚|
|PC0|32|FMC_SDNWE|连接SDRAM的WE脚|
|PC1|33|普通IO口|/|
|PC2|34|普通IO口|/|
|PC3|35|普通IO口|/|
|PC4|54|普通IO口|/|
|PC5|55|普通IO口|/|
|PC6|115|普通IO口|/|
|PC7|116|普通IO口|/|
|PC8|117|SDIO_D0|TF卡接口的DAT0脚|
|PC9|118|SDIO_D1|TF卡接口的DAT1脚|
|PC10|139|SDIO_D2|TF卡接口的DAT2脚|
|PC11|140|SDIO_D3|TF卡接口的DAT3脚|
|PC12|141|SDIO_CLK|TF卡接口的CLK脚|
|PC13|8|普通IO口|/|
|PC14|9|OSC32_IN|连接32.768KHz晶振|
|PC15|10|OSC32_OUT|连接32.768KHz晶振|
|PD0|142|FMC_D2|FMC总线数据线D2（连接SDRAM的DQ2脚）|
|PD1|143|FMC_D3|FMC总线数据线D3（连接SDRAM的DQ3脚）|
|PD2|144|SDIO_CMD|TF卡接口的CMD脚|
|PD3|145|普通IO口|/|
|PD4|146|普通IO口|/|
|PD5|147|普通IO口|/|
|PD6|150|LCD_B2|LCD1接口的B2脚|
|PD7|151|普通IO口|/|
|PD8|96|FMC_D13|FMC总线数据线的D13（连接SDRAM的DQ13脚）|
|PD9|97|FMC_D14|FMC总线数据线的D14（连接SDRAM的DQ14脚）|
|PD10|98|FMC_D15|FMC总线数据线的D15（连接SDRAM的DQ15脚）|
|PD11|99|LCD_RST|LCD1、LCD2接口的复位引脚|
|PD12|100|TP_SCK|LCD1、LCD2接口的触摸SCK信号|
|PD13|101|TP_MOSI|LCD1、LCD2接口的触摸MOSI信号|
|PD14|104|FMC_D0|FMC总线数据线D0（连接SDRAM的DQ0脚）|



**==> picture [111 x 16] intentionally omitted <==**

16

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

|PD15|105|FMC_D1|FMC总线数据线D1（连接SDRAM的DQ1脚）|
|---|---|---|---|
|PE0|169|FMC_NBL0|FMC总线NBL0（连接SDRAM的LDQM脚）|
|PE1|170|FMC_NBL1|FMC总线NBL1（连接SDRAM的UDQM脚）|
|PE2|1|普通IO口|/|
|PE3|2|普通IO口|/|
|PE4|3|普通IO口|/|
|PE5|4|LCD_G0|LCD1接口的G0脚|
|PE6|5|LCD_G1|LCD1接口的G1脚|
|PE7|68|FMC_D4|FMC总线数据线D4（连接SDRAM的DQ4脚）|
|PE8|69|FMC_D5|FMC总线数据线D5（连接SDRAM的DQ5脚）|
|PE9|70|FMC_D6|FMC总线数据线D6（连接SDRAM的DQ6脚）|
|PE10|73|FMC_D7|FMC总线数据线D7（连接SDRAM的DQ7脚）|
|PE11|74|FMC_D8|FMC总线数据线D8（连接SDRAM的DQ8脚）|
|PE12|75|FMC_D9|FMC总线数据线D9（连接SDRAM的DQ9脚）|
|PE13|76|FMC_D10|FMC总线数据线D10（连接SDRAM的DQ10脚）|
|PE14|77|FMC_D11|FMC总线数据线D11（连接SDRAM的DQ11脚）|
|PE15|78|FMC_D12|FMC总线数据线D12（连接SDRAM的DQ12脚）|
|PF0|16|FMC_A0|FMC总线地址线A0（连接SDRAM的A0脚）|
|PF1|17|FMC_A1|FMC总线地址线A1（连接SDRAM的A1脚）|
|PF2|18|FMC_A2|FMC总线地址线A2（连接SDRAM的A2脚）|
|PF3|19|FMC_A3|FMC总线地址线A3（连接SDRAM的A3脚）|
|PF4|20|FMC_A4|FMC总线地址线A4（连接SDRAM的A4脚）|
|PF5|21|FMC_A5|FMC总线地址线A5（连接SDRAM的A5脚）|
|PF6|24|QSPI_BK1_IO3|W25Q128的HOLD(IO3)脚|
|PF7|25|QSPI_BK1_IO2|W25Q128的WP(IO2)脚|
|PF8|26|QSPI_BK1_IO0|W25Q128的DI(IO0)脚|
|PF9|27|QSPI_BK1_IO1|W25Q128的DO(IO1)脚|
|PF10|28|LCD_DEN|LCD1接口的DEN脚|
|PF11|59|FMC_SDNRAS|连接SDRAM的RAS脚|
|PF12|60|FMC_A6|FMC总线地址线A6（连接SDRAM的A6脚）|
|PF13|63|FMC_A7|FMC总线地址线A7（连接SDRAM的A7脚）|
|PF14|64|FMC_A8|FMC总线地址线A8（连接SDRAM的A8脚）|
|PF15|65|FMC_A9|FMC总线地址线A9（连接SDRAM的A9脚）|
|PG0|66|FMC_A10|FMC总线地址线A10（连接SDRAM的A10脚）|



**==> picture [111 x 16] intentionally omitted <==**

17

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

|PG1|67|FMC_A11|FMC总线地址线A11（连接SDRAM的A11脚）|
|---|---|---|---|
|PG2|106|FMC_A12|FMC总线地址线A12（连接SDRAM的A12脚）|
|PG3|107|普通IO口|/|
|PG4|108|FMC_BS0|连接SDRAM的BS0脚|
|PG5|109|FMC_BS1|连接SDRAM的BS1脚|
|PG6|110|LCD_R7|LCD1接口的R7脚|
|PG7|111|LCD_PCLK|LCD1接口的DCLK脚|
|PG8|112|FMC_SDCLK|连接SDRAM的CLK脚|
|PG9|152|普通IO口|/|
|PG10|153|普通IO口|/|
|PG11|154|普通IO口|/|
|PG12|155|LCD_B1|LCD1接口的B1脚|
|PG13|156|LCD_R0|LCD1接口的R0脚|
|PG14|157|LCD_B0|LCD1接口的B0脚|
|PG15|160|FMC_SDNCAS|连接SDRAM的CAS脚|
|PH0|29|OSC_IN|连接25MHz晶振|
|PH1|30|OSC_OUT|连接25MHz晶振|
|PH2|43|FMC_SDCKE0|连接SDRAM的CKE脚|
|PH3|44|FMC_SDNE0|连接SDRAM的CS脚|
|PH4|45|EE_SCL|连接AT24C02的SCL脚|
|PH5|46|EE_SDA|连接AT24C02的SDA脚|
|PH6|83|TP_CS|LCD1、LCD2接口的触摸CS信号|
|PH7|84|TP_IRQ|LCD1、LCD2接口的触摸IRQ信号|
|PH8|85|LCD_R2|LCD1接口的R2脚|
|PH9|86|LCD_R3|LCD1接口的R3脚|
|PH10|87|LCD_R4|LCD1接口的R4脚|
|PH11|88|LCD_R5|LCD1接口的R5脚|
|PH12|89|LCD_R6|LCD1接口的R6脚|
|PH13|128|LCD_G2|LCD1接口的G2脚|
|PH14|129|LCD_G3|LCD1接口的G3脚|
|PH15|130|LCD_G4|LCD1接口的G4脚|
|PI0|131|LCD_G5|LCD1接口的G5脚|
|PI1|132|LCD_G6|LCD1接口的G6脚|
|PI2|133|LCD_G7|LCD1接口的G7脚|



**==> picture [111 x 16] intentionally omitted <==**

18

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

|PI3|134|普通IO口|/|
|---|---|---|---|
|PI4|173|LCD_B4|LCD1接口的B4脚|
|PI5|174|LCD_B5|LCD1接口的B5脚|
|PI6|175|LCD_B6|LCD1接口的B6脚|
|PI7|176|LCD_B7|LCD1接口的B7脚|
|PI8|7|普通IO口|/|
|PI9|11|LCD_VSYNC|LCD1接口的VSYNC脚|
|PI10|12|LCD_HSYNC|LCD1接口的HSYNC脚|
|PI11|13|普通IO口|/|



**==> picture [111 x 16] intentionally omitted <==**

19

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **2. STM32H743IIT6** 小系统板硬件电路介绍

## **1. MCU**

开发板采用 STM32H743IIT6 作为主控芯片，MCU 原理图如图 2.1 所示。

**==> picture [451 x 391] intentionally omitted <==**

图 **2.1 STM32H743IIT6** 小系统板 **MCU** 原理图

芯片的大部分 IO 口连接了外设，查看原理图文档可知其功能定义。芯片的 PC14 和 PC15 连接了 32.768KHz 的晶振，用于实现 RTC 功能；芯片的 PH0 和 PH1 连接了 25MHz 的晶振，用于给系统提供时钟源；芯片采用 3.3V 供电。

VBAT 为主控芯片的电池输入端，默认由 VDD 供电。图 2.1 中的 D1（BAT54C） 双向二极管隔离了 VDD 和 VBAT，VBAT 已在开发板上引出，用户可以外接纽扣电池 为其供电。

VREFP 为主控芯片的参考电压输入端，ADC 采集时，VREFP 必须给定一个参考电 压。VREF 已在开发板上引出，用户可自行选择参考电压，一般选择 3.3V。

**==> picture [111 x 16] intentionally omitted <==**

20

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

BOOT0 接地，开发板默认从芯片的 FLASH 地址 0x0800 0000 启动。

## **2. TF CARD**

TF CARD 的电路如图 2.2 所示。

**==> picture [451 x 176] intentionally omitted <==**

图 **2.2 TF CARD** 原理图

开发板的 TF 卡接口采用 SDIO 模式驱动，读写速度比 SPI 模式更快。 SDIO_D0/SDIO_D1/SDIO_D2/SDIO_D3/SDIO_CLK/SDIO_CMD 连接在主控芯片的 PC8/PC9/PC10/PC11/PC12/PD2。

## **3. QSPI FLASH**

SPI FLASH 芯片型号为 W25Q128，容量为 16MB，该部分电路如图 2.3 所示。

**==> picture [451 x 160] intentionally omitted <==**

图 **2.3 QSPI FLASH** 原理图

W25Q128 使用了主控芯片的 Quad-SPI 方式驱动，读写速度比普通 SPI 更快。

## **4. EEPROM**

EEPROM 芯片型号为 AT24C02，容量为 256B。该部分电路如图 2.4 所示。

**==> picture [111 x 16] intentionally omitted <==**

21

东莞市慧勤智远科技有限公司

STM32H743IIT6 Core Board V1.1 用户手册

**==> picture [369 x 235] intentionally omitted <==**

图 **2.4 EEPROM** 原理图

AT24C02 的 A0~A2 均接地，A0、A1 和 A2 为 0，则读操作地址为 0XA1，写操作 地址为 0XA0。EE_SCL 连接在主控芯片的 PH4 上，EE_SDA 连接在 PH5 上，这两根信 号线都接了 4.7KΩ 上拉电阻，保证通信的稳定。

## **5. KEY**

开发板按键的电路如图 2.5 所示。

**==> picture [451 x 201] intentionally omitted <==**

## 图 **2.5 KEY** 原理图

WK_UP 连接在主控芯片的 PA0 上，KEY0 连接在 PA1 上。WK_UP 用作开发板的 待机唤醒输入或普通按键输入，KEY0 用作普通按键输入。NRST 连接在主控芯片的复 位引脚，当按下 RESET 按键，系统复位。该 NRST 引脚是系统专用的复位引脚，并未 用于 LCD 屏的复位。

**==> picture [111 x 16] intentionally omitted <==**

22

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **6. USB SLAVE/HOST**

USB SLAVE/HOST 电路如图 2.6 所示。

**==> picture [451 x 160] intentionally omitted <==**

图 **2.6 USB SLAVE/HOST** 原理图

USB_D+/USB_D-连接在主控芯片的 PA12/PA11 上，该接口可以实现 USB 主/从机 通信，还可为开发板供电。

## **7. USB UART**

USB UART 的电路如图 2.7 所示。

**==> picture [451 x 294] intentionally omitted <==**

图 **2.7 USB UART** 原理图

该电路使用了 CH340X 芯片实现 USB 转串口功能，可进行串口通信或程序下载。

**==> picture [111 x 16] intentionally omitted <==**

23

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **8. SDRAM**

SDRAM 芯片型号为 W9825G6KH-6I，容量为 32MB，该电路如图 2.8 所示。

**==> picture [451 x 405] intentionally omitted <==**

图 **2.8 SRAM** 原理图

主控芯片通过 FMC 访问 SDRAM 芯片，读写速度快。

## **9. LED**

- 3 个 LED 指示灯的电路如图 2.9 所示。

**==> picture [367 x 122] intentionally omitted <==**

图 **2.9 LED** 原理图

**==> picture [111 x 16] intentionally omitted <==**

24

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

PWR 是开发板的电源指示灯，为蓝色。LED0 连接在主控芯片的 PB0 上，LED1 连 接在 PB1 上。LED0 为红色，LED1 为绿色。

## **10. SWD**

SWD 调试接口的电路如图 2.10 所示。

**==> picture [426 x 183] intentionally omitted <==**

## 图 **2.10 SWD** 原理图

SWDIO/SWCLK 连接在主控芯片的 PA13/PA14 上，该接口使用 SWD 模式进行调 试和下载程序。

## **11. GPIO**

开发板引出了两排排针，为 P1 和 P2，该部分电路如图 2.11 所示。

**==> picture [451 x 229] intentionally omitted <==**

图 **2.11 GPIO** 原理图

图 2.11 中，引出了一组 3.3V 和一组 5V 电源输入/输出口。引出了 88 个 IO 口，同 时引出了芯片的 VBAT、 BOOT0、VREF、NRST，方便测试。

**==> picture [111 x 16] intentionally omitted <==**

25

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **12. POWER**

系统的电源电路如图 2.12 所示。

**==> picture [451 x 144] intentionally omitted <==**

图 **2.12 POWER** 原理图

开发板使用 USB 进行供电，电源芯片采用了 CJA1117B-3.3，作用是将 5V 电压转 换成 3.3V，给开发板的芯片和外设供电。

## **13. LCD1-RGB INTERFACE**

LCD1 接口的电路如图 2.13 所示。

**==> picture [451 x 227] intentionally omitted <==**

图 **2.13 LCD1** 接口原理图

LCD1 接口为 40 PIN，支持 RGB 接口的 LCD 模块，支持 565/888 模式显示。BL_EN 连接在主控芯片的 PB5 上，用于控制 LCD 的背光。LCD_RST 连接在主控芯片的 PD11 上，用单独的 IO 口控制屏幕复位。该接口采用 LTDC 驱动 RGB 屏，并带有 DMA2D （图形加速器）。

**==> picture [111 x 16] intentionally omitted <==**

26

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **14. LCD2-SPI INTERFACE**

LCD2 接口为 20 PIN，采用 SPI+I2C 方式进行图像显示及触摸，该部分电路如图 2.14 所示。

**==> picture [451 x 207] intentionally omitted <==**

图 **2.14 LCD2** 接口原理图

SPI2_CS/SPI2_SCK/SPI2_MISO/SPI2_MOSI 连 接 在 主 控 芯 片 的 PB12/PB13/PB14/PB15 上，LCD_RST/TP_IRQ/TP_CS/TP_SCK/TP_MISO/TP_MOSI 连 接在主控芯片的 PD11/PH7/PH6/PD12/PB11/PD13 上。该接口支持 3 线 SPI 和 4 线 SPI 的 LCD 模块，可接慧勤智远 2.4/3.5/4.3/5.0/7.0 寸 SPI 接口 LCD 模块，其信号引脚与主 控芯片对应的连接关系如表 2.1 所示。

表 **2.1 3** 线和 **4** 线 **SPI LCD** 模块与主控芯片连接关系

|PIN NO.|MCU PIN|3 wire SPI|4 wire SPI|引脚功能说明|
|---|---|---|---|---|
|5|PB12|CS|CS|SPI片选信号|
|6|PB15|NC|MOSI|4线SPI：MOSI|
|7|PB13|SCLK|SCLK|SPI串口时钟信号|
|10|PB14|SDA|MISO|3线SPI：数据输入/输出；4线SPI：MISO|
|9|PD11|RESET|RESET|复位信号|



LCD2 接口的引脚功能定义如表 2.2 所示。

**==> picture [111 x 16] intentionally omitted <==**

27

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

表 **2.2 LCD2** 接口的引脚功能定义

|引脚编号|引脚定义|引脚功能说明|
|---|---|---|
|1|VCC5|5V电源|
|2|VCC5|5V电源|
|3|GND|电源地|
|4|GND|电源地|
|5|SPI2_CS|SPI片选信号|
|6|SPI2_MOSI|3线SPI：NC；4线SPI：MOSI信号|
|7|SPI2_SCK|SPI串口时钟信号|
|8|NC|NC|
|9|LCD_RST|LCD复位信号|
|10|SPI2_MISO|3线SPI：数据输入/输出引脚；4线SPI：MISO信号|
|11|TP_IRQ|电阻触摸屏：笔接触中断引脚；电容触摸屏：INT信号|
|12|TP_SCK|电阻触摸屏：时钟信号输出；电容触摸屏：SCL信号|
|13|TP_MOSI|电阻触摸屏：串行数据输出端；电容触摸屏：SDA信号|
|14|TP_CS|电阻触摸屏：片选信号；电容触摸屏：复位信号|
|15|TP_MISO|电阻触摸屏：串行数据输入端；电容触摸屏：NC|
|16|GND|电源地|
|17~19|NC|NC|
|20|GND|电源地|



**==> picture [111 x 16] intentionally omitted <==**

28

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **3.** 开发板软件使用方法

开发板主要用到三个软件工具，一个是代码编写和下载的工具 MDK5，一个是程序 下载工具 STM32CubeProgrammer，还有一个是串口调试助手 sscom。

目前测试仿真器下载程序的情况如下：

**==> picture [341 x 115] intentionally omitted <==**

建议开发板搭配 DAP-Link 仿真器使用。

## **3.1 MDK5** 安装与使用

## **3.1.1 MDK5** 安装

（1）软件安装包路径“慧勤智远 STM32H743IIT6 小系统板\9. 软件开发工具 \MDK5.zip”，解压文件，双击 打开，点击 Next。

**==> picture [426 x 289] intentionally omitted <==**

图 **3.1 MDK5** 安装界面

**==> picture [111 x 16] intentionally omitted <==**

29

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## （2）勾选“I agree to……”，再点击 Next。

**==> picture [426 x 289] intentionally omitted <==**

图 **3.2**

- （3）选择软件安装路径和支持包安装路径，可以默认，路径不要有中文，点击

## Next。

**==> picture [426 x 290] intentionally omitted <==**

图 **3.3**

**==> picture [111 x 16] intentionally omitted <==**

30

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## （4）填写用户信息（可以随便填写），点击 Next。

**==> picture [426 x 289] intentionally omitted <==**

图 **3.4**

## （5）软件开始安装，等待安装完成。

**==> picture [426 x 290] intentionally omitted <==**

图 **3.5**

**==> picture [111 x 16] intentionally omitted <==**

31

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## （6）软件安装完成，点击 Finish。

**==> picture [426 x 289] intentionally omitted <==**

图 **3.6**

- （7）弹出支持包更新对话框，可直接退出，安装离线包。

**==> picture [451 x 254] intentionally omitted <==**

图 **3.7**

**==> picture [111 x 16] intentionally omitted <==**

32

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

（8）软件安装完之后，电脑桌面出现 图标。打开路径“慧勤智远 STM32H743IIT6 小系统板\9. 软件开发工具\MDK5”，双击

**==> picture [154 x 16] intentionally omitted <==**

安装 STM32H7xx 支持包，点击 Next，开始安装。

**==> picture [341 x 232] intentionally omitted <==**

图 **3.8**

## （9）安装完成，点击 Finish。

**==> picture [341 x 232] intentionally omitted <==**

图 **3.9**

注意：提供的 **MDK** 安装包为官方正版，用户需要激活才能正常使用。

**==> picture [111 x 16] intentionally omitted <==**

33

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **3.1.2 MDK5** 打开工程

- 用 MDK5 软件打开工程有两种方法。

方法 **1** ：

- （1）双击打开 软件，点击“Open…”图标。

**==> picture [195 x 114] intentionally omitted <==**

图 **3.10**

- （2）选择文件类型“Project Files”。

**==> picture [441 x 38] intentionally omitted <==**

图 **3.11**

- （3）找到工程所在路径，找到 xxx.uvprojx 打开即可。

**==> picture [451 x 248] intentionally omitted <==**

图 **3.12**

## 方法 **2** ：

在工程目录下找到 xxx.uvprojx，双击打开即可（默认用 Keil uVision5 打开）。

**==> picture [111 x 16] intentionally omitted <==**

34

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **3.1.3 MDK5** 相关设置

在使用 MDK5 时，通常需要设置一些参数，才能更好地使用该软件。（提示：在 打开工程后才可以设置）

- （1）点击 魔术棒，设置相关参数。

**==> picture [400 x 299] intentionally omitted <==**

图 **3.13**

- （2）首先，选择开发板的芯片型号（根据实际型号选择）。

**==> picture [111 x 16] intentionally omitted <==**

35

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [400 x 298] intentionally omitted <==**

图 **3.14**

## （3）选择 AC5 编译器。

**==> picture [400 x 299] intentionally omitted <==**

图 **3.15**

**==> picture [111 x 16] intentionally omitted <==**

36

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

- （4）勾选“Create HEX File”，编译工程时生成可执行文件（后缀名为 hex）；

勾选“Browse Information”，可跳转查看变量或函数的定义。

**==> picture [401 x 298] intentionally omitted <==**

**==> picture [36 x 10] intentionally omitted <==**

**----- Start of picture text -----**<br>
图 3.16<br>**----- End of picture text -----**<br>


以上参数设置完之后，点击“OK”返回。

## **3.1.4 MDK5** 仿真调试设置

通常，我们使用仿真器进行程序下载或调试，下面以 DAP-Link 仿真器为例设置 MDK5 的相关参数。（提示：在打开工程后才可以设置）

（1）点击 魔术棒，打开后，点击 Debug，选择 CMSIS-DAP Link Debugger， 再点击 Settings。

**==> picture [111 x 16] intentionally omitted <==**

37

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [400 x 298] intentionally omitted <==**

图 **3.17**

- （2）设置仿真器参数，选择“DAPLink CMSIS-DAP”，选择 SW 模式，调试速

- 度选择 10MHz。

**==> picture [451 x 290] intentionally omitted <==**

图 **3.18**

**==> picture [111 x 16] intentionally omitted <==**

38

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

（3）继续设置仿真器参数，勾选“Reset and Run”，选择芯片下载算法（根据实 际选取，点击 Add 增加，点击 Remove 移除）。

**==> picture [451 x 290] intentionally omitted <==**

图 **3.19**

以上参数设置完之后，点击“确定”，再点击“OK”返回。

（4）返回工程后，点击 编译代码（已编译则无需再次编译），然后点击 下 载程序。

**==> picture [451 x 222] intentionally omitted <==**

图 **3.20**

**==> picture [111 x 16] intentionally omitted <==**

39

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **3.2** 串口程序下载（ **STM32CubeProgrammer** ）

- （1）安装 CH340 驱动。软件路径“慧勤智远 STM32H743IIT6 小系统板\9. 软件

开发工具\CH340 驱动.ZIP”，解压文件，双击 ，点击安装。

**==> picture [403 x 233] intentionally omitted <==**

图 **3.21**

- （2）安装成功，点击确定返回，再退出安装界面。

**==> picture [143 x 128] intentionally omitted <==**

图 **3.22**

（3）安装 STM32CubeProgrammer。软件路径“慧勤智远 STM32H743IIT6 小系 统板\9. 软件开发工具\en.stm32cubeprg-win64_v2-6-0.zip”，解压文件，双击 开始安装。点击 Next。

**==> picture [111 x 16] intentionally omitted <==**

40

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [361 x 284] intentionally omitted <==**

图 **3.23**

## （4）点击 Next。

**==> picture [361 x 284] intentionally omitted <==**

图 **3.24**

**==> picture [111 x 16] intentionally omitted <==**

41

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

- （5）选择“I accept the terms of this license agreement.”，点击 Next。

**==> picture [362 x 285] intentionally omitted <==**

图 **3.25**

- （6）选择安装路径，可以选择默认路径。点击 Next。

**==> picture [362 x 284] intentionally omitted <==**

图 **3.26**

**==> picture [111 x 16] intentionally omitted <==**

42

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

- （7）选择安装的组件，这里全部选择。点击 Next。

**==> picture [362 x 285] intentionally omitted <==**

图 **3.27**

- （8）软件开始安装，到“2 / 4”时，弹出驱动安装窗口，点击下一页。

**==> picture [362 x 284] intentionally omitted <==**

图 **3.28**

**==> picture [111 x 16] intentionally omitted <==**

43

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [355 x 284] intentionally omitted <==**

图 **3.29**

## （9）驱动安装完成，点击完成。

**==> picture [355 x 284] intentionally omitted <==**

图 **3.30**

**==> picture [111 x 16] intentionally omitted <==**

44

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

- （10）组件安装完成。点击 Next。

**==> picture [362 x 285] intentionally omitted <==**

图 **3.31**

- （11）选择是否创建快捷方式，这里默认选择。点击 Next。

**==> picture [362 x 284] intentionally omitted <==**

图 **3.32**

**==> picture [111 x 16] intentionally omitted <==**

45

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

- （12）软件安装完成。点击 Done。

**==> picture [362 x 285] intentionally omitted <==**

图 **3.33**

（13）电脑连接开发板的 USB-C 转串口接口，将开发板的 BOOT0 引脚与 3.3V 短 接，然后按一次复位键，开发板进入 ISP 模式（此时可将开发板的 BOOT0 与 3.3V 断 开）。打开电脑的设备管理器可以看到 USB 串口。

**==> picture [228 x 146] intentionally omitted <==**

**==> picture [199 x 210] intentionally omitted <==**

图 **3.34**

**==> picture [111 x 16] intentionally omitted <==**

46

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

（14）点击桌面图标 打开 STM32CubeProgrammer。选择“UART”； Port：选择识别到的串口端口（USB-SERIAL CH340）；Baudrate：串口波特率（选择 115200）；Parity：Even。点击“Connect”。

**==> picture [451 x 293] intentionally omitted <==**

图 **3.35**

- （15）连接成功，显示相关信息。点击 。

**==> picture [111 x 16] intentionally omitted <==**

47

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [451 x 294] intentionally omitted <==**

图 **3.36**

（16）点击“Browse”，找到需要下载的 hex 文件。勾选“Verify programming” 和“Run after programming”。点击“Start programming”，程序开始下载。

**==> picture [451 x 293] intentionally omitted <==**

图 **3.37**

**==> picture [111 x 16] intentionally omitted <==**

48

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

- （17）程序下载成功，自动运行。 注意：程序运行后，开发板不工作在 **ISP** 模

- 式，如果想要再次下载程序，请重新按照步骤 **13** 、 **14** 操作。

**==> picture [321 x 107] intentionally omitted <==**

图 **3.38**

**==> picture [111 x 16] intentionally omitted <==**

49

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **3.3** 串口调试（ **SSCOM** ）

   - （1）安装 CH340 驱动。（参考 3.2 节，已安装则忽略）

- （2）电脑连接开发板的 USB-C 转串口接口，打开电脑的设备管理器可以看到

- USB 串口。（参考 3.2 节）

   - （3）打开 sscom 软件。软件路径“慧勤智远 STM32H743IIT6 小系统板\9. 软件

- 开发工具\串口调试助手.zip”，解压文件，双击 打开。

- （4）设置相关参数。串口号：选择识别到的串口端口（USB-SERIAL CH340）；

- 波特率：要与代码中设置一致（本实验用 115200）。 勾选 **DTR** 。最后打开串口，就 可以进行串口通信。

**==> picture [411 x 326] intentionally omitted <==**

**==> picture [36 x 10] intentionally omitted <==**

**----- Start of picture text -----**<br>
图 3.39<br>**----- End of picture text -----**<br>


- （5）如需向串口发送数据，则要勾选发送新行，然后输入数据，点击发送即可。

**==> picture [111 x 16] intentionally omitted <==**

50

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [411 x 326] intentionally omitted <==**

图 **3.40**

**==> picture [111 x 16] intentionally omitted <==**

51

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **3.4 DFU** 程序下载（ **STM32CubeProgrammer** ）

- （1）安装 DFU 驱动。软件路径“慧勤智远 STM32H743IIT6 小系统板\9. 软件开

- 发工具\STSW_STM32080_V3.0.6.zip”，解压文件，双击

- 打开。点击 Next。

**==> picture [317 x 242] intentionally omitted <==**

图 **3.41**

- （2）填写信息，可以随意填写。

**==> picture [318 x 242] intentionally omitted <==**

图 **3.42**

**==> picture [111 x 16] intentionally omitted <==**

52

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## （3）点击“Install”。

**==> picture [318 x 242] intentionally omitted <==**

图 **3.43**

- （4）等待安装，会弹出一个窗口，选择“是”，继续安装。

**==> picture [318 x 242] intentionally omitted <==**

图 **3.44**

**==> picture [111 x 16] intentionally omitted <==**

53

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## （5）安装完成，点击“Finish”。关闭两个弹窗。

**==> picture [318 x 242] intentionally omitted <==**

图 **3.45**

**==> picture [415 x 312] intentionally omitted <==**

图 **3.46**

- （6）安装 STM32CubeProgrammer。（参考 3.2 节，已安装则忽略）

**==> picture [111 x 16] intentionally omitted <==**

54

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

- （7）电脑连接开发板的 USB-C 通信接口，将开发板的 BOOT0 与 3.3V 短接，然

后按一次复位键，开发板进入 DFU 模式。打开电脑的设备管理器可以看到 DFU 设 备。（电脑识别到 DFU 设备后，可将开发板的 BOOT0 与 3.3V 断开）

**==> picture [228 x 146] intentionally omitted <==**

**==> picture [199 x 174] intentionally omitted <==**

图 **3.47**

（8）点击桌面图标 打开 STM32CubeProgrammer。选择“USB”，会自 动识别到 USB 端口，点击“Connect”。

**==> picture [451 x 294] intentionally omitted <==**

**==> picture [36 x 10] intentionally omitted <==**

**----- Start of picture text -----**<br>
图 3.48<br>**----- End of picture text -----**<br>


**==> picture [111 x 16] intentionally omitted <==**

55

东莞市慧勤智远科技有限公司

STM32H743IIT6 Core Board V1.1 用户手册

**==> picture [251 x 31] intentionally omitted <==**

**----- Start of picture text -----**<br>
（9）连接成功，显示相关信息。点击 。<br>**----- End of picture text -----**<br>


**==> picture [451 x 293] intentionally omitted <==**

图 **3.49**

- （10）点击“Browse”，找到需要下载的 hex 文件。勾选“Verify programming”

- 和“Run after programming”。点击“Start programming”，程序开始下载。

**==> picture [111 x 16] intentionally omitted <==**

56

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

**==> picture [451 x 294] intentionally omitted <==**

图 **3.50**

   - （11）程序下载成功，自动运行。 注意：程序运行后，开发板不工作在 **DFU** 模

- 式，如果想要再次下载程序，请重新按照步骤 **7** 、 **8** 操作。

**==> picture [327 x 106] intentionally omitted <==**

图 **3.51**

**==> picture [111 x 16] intentionally omitted <==**

57

STM32H743IIT6 Core Board V1.1 用户手册 东莞市慧勤智远科技有限公司

## **4. STM32H743IIT6** 小系统板例程列表

STM32H743IIT6 小系统板提供了丰富的例程，可供用户参考学习。

表 **4.1** 基础例程

|基础例程（寄存器版本）||
|---|---|
|1. Template工程模板-新建工程使用|26.内存管理实验|
|2.跑马灯实验|27. SD卡实验|
|3.按键输入实验|28. FATFS实验|
|4.外部中断实验|29.汉字显示实验|
|5.串口通信实验|30.图片显示实验|
|6.独立看门狗实验|31.硬件JPEG解码实验|
|7.窗口看门狗实验|32.视频播放器实验|
|8.定时器中断实验|33. FPU测试（Julia分形）实验|
|9. PWM输出实验|34. DSP测试实验|
|10.输入捕获实验|35.手写识别实验|
|11.内存保护（MPU）实验|36. T9拼音输入法实验|
|12. SDRAM实验|37.串口IAP实验|
|13. LTDC LCD（RGB屏）实验|38. USB读卡器(Slave)实验|
|14. USMART调试实验|39. USB虚拟串口(Slave)实验|
|15. RTC实时时钟实验|40. USB U盘(Host)实验|
|16.硬件随机数实验|41. USB鼠标键盘(Host)实验|
|17.待机模式实验|42. UCOSII实验1-任务调度|
|18. DMA实验|43. UCOSII实验2-信号量和邮箱|
|19. ADC实验|44. UCOSII实验3-消息队列、信号量集和<br>软件定时器|
|20.内部温度传感器实验|45. 7680模块显示实验|
|21. DAC实验|46. 7680模块触摸屏实验|
|22.软件模拟IIC读写EEPROM实验|47. TFTLCD（2.4&3.5寸SPI屏）实验|
|23. QSPI FLASH实验|48.（2.4&3.5寸SPI屏）触摸屏实验|
|24.触摸屏实验|49.综合测试实验（寄存器版本）|
|25. FLASH模拟EEPROM实验||



**==> picture [111 x 16] intentionally omitted <==**

58
