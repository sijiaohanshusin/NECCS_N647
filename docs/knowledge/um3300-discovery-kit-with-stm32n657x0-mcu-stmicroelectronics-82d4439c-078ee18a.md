<!-- page: 1 -->
UM3300

User manual

Discovery kit with STM32N657X0 MCU

Introduction

The STM32N6570-DK Discovery kit is a complete demonstration and development platform for the Arm® Cortex®‑M55
core‑based STM32N657X0H3Q microcontroller.
The STM32N6570-DK Discovery kit includes a full range of hardware features that help the user evaluate many peripherals,
such as USB Type-C®, Octo‑SPI flash memory and Hexadeca‑SPI PSRAM devices, Ethernet, camera module, LCD,
microSD™, audio codec, digital microphones, ADC, flexible extension connectors, and user button. The four flexible extension
connectors feature easy and unlimited expansion capabilities for specific applications such as wireless connectivity, analog
applications, and sensors.
The STM32N657X0H3Q microcontroller features one USB 2.0 high‑speed/full‑speed Device/Host/OTG controller, one USB 2.0
high‑speed/full‑speed Device/Host/OTG controller with UCPD (USB Type-C® Power Delivery), one Ethernet with TSN (time-
sensitive networking), four I2Cs, two I3Cs, six SPIs (of which four I2S‑capable), two SAIs, with four DMIC support, five USARTs,
five UARTs (ISO78916 interface, LIN, IrDA, up to 12.5 Mbit/s), one LPUART, two SDMMCs (MMC version 4.0, CE-ATA version
1.0, and SD version 1.0.1), three CAN FD with TTCAN capability, JTAG and SWD debugging support, and Embedded Trace
Macrocell™ (ETM).

The STM32N6570-DK Discovery kit integrates an STLINK-V3EC embedded in-circuit debugger and programmer for the STM32
MCU, with a USB Virtual COM port bridge and the comprehensive MCU Package.

Figure 1. STM32N6570-DK top view Figure 2. STM32N6570-DK bottom view

Pictures are not contractual.

UM3300 - Rev 1 - December 2024 www.st.com
For further information contact your local STMicroelectronics sales office.

<!-- page: 2 -->
UM3300
Features

1 Features

- STM32N657X0H3Q Arm® Cortex®‑M55‑based microcontroller featuring ST Neural-ART Accelerator, H264
encoder, NeoChrom 2.5D GPU, and 4.2 Mbytes of contiguous SRAM, in a VFBGA264 package
- 5" LCD module with capacitive touch panel
- USB Type-C® with USB 2.0 HS interface, dual‑role‑power (DRP)
- USB Type-A with USB 2.0 HS interface, host, 0.5 A max
- 1‑Gbit Ethernet with TSN (time-sensitive networking) compliant with IEEE‑802.3‑2002
- SAI audio codec
- One MEMS digital microphone
- 1‑Gbit Octo‑SPI flash memory
- 256-Mbit Hexadeca‑SPI PSRAM
- Two user LEDs
- User, tamper, and reset push-buttons
- Board connectors:
– USB Type-C®
– USB Type-A
– Ethernet RJ45
– Camera
– microSD™ card
– LCD
– Stereo headset jack including analog microphone input
– Audio MEMS daughterboard expansion connector
– ARDUINO® Uno R3 expansion connector
– STMod+ expansion connector
- On-board STLINK-V3EC debugger/programmer with USB re-enumeration capability: Virtual COM port, and
debug port
- Comprehensive free software libraries and examples available with the STM32CubeN6 MCU Package
- Support of a wide choice of Integrated Development Environments (IDEs) including IAR Embedded
Workbench®, MDK-ARM, and STM32CubeIDE
- Handled by STM32CubeMonitor-UCPD (STM32CubeMonUCPD) software tool.

Note: Arm is a registered trademark of Arm Limited (or its subsidiaries) in the US and/or elsewhere.

UM3300 - Rev 1 page 2/49

<!-- page: 3 -->
UM3300
Ordering information

2 Ordering information

To order the STM32N6570-DK Discovery kit, refer to Table 1. Additional information is available from the
datasheet and reference manual of the target STM32.

Table 1. Ordering information

Order code Board references Target STM32

- MB1280(1)
- MB1854(2)
STM32N6570-DK STM32N657X0H3Q
- MB1860(3)
- MB1939(4)

1. STMod+ fan-out expansion board
2. Camera module daughterboard
3. LCD daughterboard
4. Main board

2.1 Codification
The meaning of the codification is explained in Table 2.

Table 2. Codification explanation

STM32XXYYZ-DK Description Example: STM32N6570-DK

XX MCU series in STM32 32-bit Arm Cortex MCUs STM32N6 series

YY MCU product line in the series STM32N657

STM32 flash memory size:
Z 0 Kbyte
- 0 for 0-1 Kbyte

Toolkit type:
-DK Discovery kit
- Discovery kit

UM3300 - Rev 1 page 3/49

<!-- page: 4 -->
UM3300
Development environment

3 Development environment

3.1 System requirements
- Multi‑OS support: Windows® 10, Linux® 64-bit, or macOS®
- USB Type-A or USB Type-C® to USB Type-C® cable

Note: macOS® is a trademark of Apple Inc., registered in the U.S. and other countries and regions.
Linux® is a registered trademark of Linus Torvalds.
Windows is a trademark of the Microsoft group of companies.

3.2 Development toolchains
- IAR Systems® - IAR Embedded Workbench®(1)
- Keil® - MDK-ARM(1)
- STMicroelectronics - STM32CubeIDE

1. On Windows® only.

3.3 Demonstration software
The demonstration software, included in the STM32Cube MCU Package corresponding to the on-board
microcontroller, is preloaded in the on-board flash memory for easy demonstration of the device peripherals in
standalone mode. The latest versions of the demonstration source code and associated documentation can be
downloaded from www.st.com.

3.4 CAD resources
All board design resources, including schematics, CAD databases, manufacturing files, and the bill of materials,
are available from the STM32N6570-DK product page at www.st.com.

UM3300 - Rev 1 page 4/49

<!-- page: 5 -->
UM3300
Conventions

4 Conventions

Table 3 provides the conventions used for the ON and OFF settings in the present document.

Table 3. ON/OFF convention

Convention Definition

Jumper JPx ON Jumper fitted

Jumper JPx OFF Jumper not fitted

Jumper JPx [1-2] Jumper fitted between pin 1 and pin 2

Solder bridge SBx ON SBx connections closed by 0 Ω resistor

Solder bridge SBx OFF SBx connections left open

Resistor Rx ON Resistor soldered

Resistor Rx OFF Resistor not soldered

Capacitor Cx ON Capacitor soldered

Capacitor Cx OFF Capacitor not soldered

UM3300 - Rev 1 page 5/49

<!-- page: 6 -->
UM3300
Safety recommendations

5 Safety recommendations

5.1 Targeted audience
This product targets users with at least basic electronics or embedded software development knowledge such as
engineers, technicians, or students. This board is not a toy and is not suited for use by children.

5.2 Handling the board
This product contains a bare printed circuit board and like all products of this type, the user must be careful about
the following points:
- The connection pins on the board might be sharp. Be careful when handling the board to avoid hurting
yourself
- This board contains static‑sensitive devices. To avoid damaging it, handle the board in an ESD‑proof
environment.
- While powered, do not touch the electric connections on the board with your fingers or anything conductive.
The board operates at a voltage level that is not dangerous, but components might be damaged when
shorted.
- Do not put any liquid on the board and avoid operating the board close to water or at a high humidity level.
- Do not operate the board if dirty or dusty.

UM3300 - Rev 1 page 6/49

<!-- page: 7 -->
UM3300
Quick start

6 Quick start

This section describes how to start development quickly using the STM32 Discovery board.
Before installing and using the product, accept the evaluation product license agreement from the www.st.com/
epla webpage.
For more information on the STM32N6570-DK Discovery kit and demonstration software, visit the STM32N6570-
DK webpage.

6.1 Getting started
Follow the sequence below to configure STM32N6570-DK and launch the demonstration application (refer to
Figure 4 and Figure 5 for component location):
1. To identify correctly all device interfaces from the host PC, install the discovery USB driver available on the
STM32N6570-DK webpage, before connecting the board.
2. Check that the jumper JP2 is set on STLK (shunt [1-2]). Set SW1 and SW2 to 'L'.
3. Connect a USB Type-A(1) or USB Type-C® to USB Type-C® cable (not included) from the STM32N6570-DK
Discovery board (CN6) to a PC to power the board. The 5V power LED (LD5) lights up.
4. The LCD daughterboard displays a welcome menu, indicating the demonstration application software startup.
5. The demonstration application software and its user manual, as well as other software examples for exploring
STM32N6 features are available at STM32N6570-DK.
6. Develop an application using the available examples.

1. When using Type-A to USB Type-C® cable, the power supply is limited to around 550 mA, very close to the consumption of
the board. Used with the MB1854 camera module, the board fails to start because the power supply is not enough through
the Type-A to USB Type-C® cable.

UM3300 - Rev 1 page 7/49

<!-- page: 8 -->
UM3300
Hardware layout and configuration

7 Hardware layout and configuration

The STM32N6570-DK Discovery kit is designed around the STM32N657X0H3Q microcontroller, in VFBGA264
package. The hardware block diagram in Figure 3 illustrates the connection between the microcontroller and the
peripherals. Figure 4 and Figure 5 help to locate these features on the STM32N6570-DK Discovery board.

7.1 Hardware block diagram and board layout

Figure 3. Hardware block diagram

3.3 V power 1.8 V power SMPS power
supply supply supply

32 KHz crystal SAI1 and Headset LSE/HSE Codec 48 MHz crystal I2C2 connector

MEMs
Octo-SPI flash memory XSPIM_P2 microphone
MDF
MEMs
connector
Hexadeca-SPI PSRAM XSPIM_P1 (MB1671)

TFT LCD with
RGB, GPIOs,
touch panel
microSDTM card SDMMC2 and I2C2 (MB1860)

STM32N657X0H3Q

CSI,
Camera module GPIOs RJ45 connector RGMII PHY and I2C1 connector

USB Type-C® GPIOs, I2C1, USB1 ARDUINO® source and sink ADCs and
connector
SPI5
USB Type-A USB2
GPIOs, I2C1,
STMod+
VCP USART2, ADC STLINK-V3EC connector USART1 and SPI5

SWD and Two LEDs
JTAG GPIOs User, tamper and
reset buttons
MIPI20 Trace DT59548V1

UM3300 - Rev 1 page 8/49

<!-- page: 9 -->
UM3300
Hardware layout and configuration

Figure 4. STM32N6570-DK PCB layout top view

Tamper User Reset COM Power status Microphone daughterboard
BOOT0 (SW2) button button button LED LED connector
BOOT1 (SW1) (B4) (B2) (B1) (LD4) (LD3) (CN5)

STMod+
(CN4)

LCD connector
(CN3)

Current
Measurement
(CN2)

TAG footprint User LEDs:
(CN1) Green (LD1) Red (LD2) DT59549V1

UM3300 - Rev 1 page 9/49

<!-- page: 10 -->
UM3300
Hardware layout and configuration

Figure 5. STM32N6570-DK PCB layout bottom view

MEMs microphone STLINK-V3EC USB Type-C®
(U13) (CN6)

Power supply
select
ARDUINO® (JP2)
connectors
(CN7, CN8, CN11,
and CN12)

MIPI20
(CN10)

Flash memory
(U25)
SD card socket
(CN13) PSRAM
(U26)

Camera connector
(CN14)

Audio jack
(CN15)

USB1 USB2 Ethernet
USB Type-C® USB Type-A RJ45 (CN18) (CN17) (CN16) DT59550V1

UM3300 - Rev 1 page 10/49

<!-- page: 11 -->
UM3300
Hardware layout and configuration

7.2 Mechanical drawing

Figure 6. Board mechanical drawing (in millimeters)

DT59551V1

UM3300 - Rev 1 page 11/49

<!-- page: 12 -->
UM3300
Hardware layout and configuration

7.3 Embedded STLINK-V3EC
STLINK-V3EC is the embedded version of STLINK-V3 included in the design of the Discovery board. It allows
access to the programming, debugging, and monitoring functions of the STM32 through the USB STLK connector
(CN6).
The STLINK-V3EC facility for debugging and programming is integrated into the STM32N6570-DK Discovery kit.
The embedded STLINK-V3E supports JTAG/SWD and VCP for STM32 devices.
Features supported in STLINK-V3EC:
- 5 V power supplied by the USB Type-C® connector (CN6)
- USB 2.0 high-speed-compatible interface
- JTAG and Serial Wire Debug (SWD) interface
- MIPI20 compatible connector (CN10)
- COM status LED (LD4), which blinks during communication with the PC
- Power status LED (LD3), which identifies the status of current output to the board
Table 4 describes the USB Type-C® connector (CN6) pinout.

Table 4. USB Type-C® connector (CN6) pinout

Pin Pin name Signal name STLINK-V3E STM32 pin Function

A4, A9, B4, B9 VBUS VBUS_STLK - VBUS power

A6, B6 DP STLK_USB_P PB15 DP

A7, B7 DM STLK_USB_N PB14 DM

A5 CC1 STLK_UCPD_CC1_C PC3 Pull-down by 5.1 kΩ

B5 CC2 STLK_UCPD_CC2_C PC4 Pull-down by 5.1 kΩ

A1, A12, B1, B12 GND GND GND GND

7.3.1 STLINK-V3E drivers
The installation of drivers is not mandatory since Windows 10® but allocates an ST‑specific name to the ST-LINK
COM port in the system device manager.
For detailed information on the ST-LINK USB drivers, refer to the technical note Overview of ST-LINK derivatives
(TN1235).

7.3.2 STLINK-V3EC firmware upgrade
STLINK-V3EC embeds a firmware upgrade (stsw-link007) mechanism through the USB-C® port. The firmware
might evolve during the lifetime of the STLINK-V3EC product (for example to add new functionalities, fix bugs,
and support new microcontroller families). Therefore, it is recommended to keep the STLINK-V3EC firmware up
to date before using the STM32N6570-DK Discovery board. The latest version of this firmware is available from
the www.st.com website.

UM3300 - Rev 1 page 12/49

<!-- page: 13 -->
UM3300
Hardware layout and configuration

7.4 Power supply
Figure 7 describes the power architecture and the maximum voltage and current limits, under which functions can
be safely used on the STM32N6570-DK product. In any case, ensure the total power budget of the application
always conforms to the selected 5 V power source mode, if not malfunction can occur. For detailed configuration,
refer to the relevant function description and technical application notes.

Figure 7. Power diagram

3.3 V/up to 300 mA ST-LINK VDD1V8 Inductor VDDA1V8PMU LDO VDDA1V8PMU VDDA1V8 MCU
VDD3V3 Switch VDD_SD
VDD_SD SPDT VDDIO_SD
5V
5V SEL
USB 5 V/up to 3 A eFuse
ST-LINK
0.8 V
DCDC VDDCORE
3.3 V
LDO VDDIO
5V 1.8 V
5V LDO VDDA1V8_AON ARDUINO® LDO 5 V/up to 0.8 A
1.8 V
DCDC VDD1V8
3.3 V 1.8 V
DCDC VDD3V3 LDO VDDA1V8

USB1 USB-C 5 V/1.5 A
Type-C® protection
for DRP
5 V/up to 1.5 A

VDDIO VDD STM32N657X0H3Q Hexadeca-SPI
VBAT VDDA1V8 VDD3V3 SD card PSRAM VDD3V3 VDD33USB
VDDA1V8PMU VDDA18PMU
VDD1V8 VDDSMPS INTERNALSMPS
Octo-SPI MEMs
VDDCORE VDDCORE VDDA1V8 VDD3V3 flash microphone
VDDA1V8 VDDIO2
VDDIO3
VDDIO VDDIO4 VDD3V3 5V
VDDIO_SD VDDIO5 Audio Others
VDDA1V8 VDDA18CSI VDDA1V8 VDD3V3
VDDA18USB
VDDA18ADC
VDDA18PLL
VREF+ VDD3V3 Ethernet
VDDA1V8_AON VDDA18AON DT59552V1

UM3300 - Rev 1 page 13/49

<!-- page: 14 -->
UM3300
Hardware layout and configuration

7.4.1 Power source selection
The STM32N6570-DK product is designed to be powered by a 5 V DC power supply. It is possible to configure
the Discovery board with JP2 headers to use any of the four sources described in Table 5 for the 5 V DC power
supply.

Table 5. 5 V power configurations

Reference Jumper(1) Function Comment

- 5 V (+/- 5%)
5 V is supplied from - 3.5 A embedded overcurrent
5V_STLK
USB STLK (CN6) protection
- Up to 3 A capable

- 5 V (+/- 5% at current < 500 mA) JP2 5 V is supplied from
USB_SNK - Up to 3 A (check USB Type-C® VBUS
USB1 (CN18) constraints in that case)

5 V is supplied from - 5 V (+/- 5%)
5V_VIN
ARDUINO® connector (CN8) - Up to 0.8 A capable

1. The default setting is in bold

7.4.2 STLK (for compatibility with legacy USB Host port)
Figure 8 shows the selection of 5 V from 5V_STLK on JP2 [1-2], with a power source connected to USB ST-LINK
(CN6). It is the default setting.
The STLK ST-LINK USB Type-C® connector (CN6) can power the STM32N6570-DK Discovery kit, but the host
PC only provides 100 mA to the ST-LINK circuit until the end of USB enumeration. At the end of the USB
enumeration, the STM32N6570-DK Discovery kit asks for a 2 A current to the host PC.
If the USB enumeration succeeds, a power switch powers the board with up to 3.2 A current. This power switch
also features a 550 mA, 1.66 or 3.2 A current limitation to protect the PC in case of overcurrent on the board.

Figure 8. STLK (JP2) from USB STLK (CN6)

7.4.3 User USB1
Figure 9 shows the selection of 5 V DC power from USER USB1 on JP2, with a power source connected to the
USB Type-C® connector (CN18). The LED (LD9) lights up. A power switch with a protective circuit protects the
STM32N6570-DK Discovery kit from overvoltage on VBUS. The USB Type-C® VBUS nominal voltage must be in
the 5 V +/- 5% range.
The following constraints apply to USB Type-C® host types to be used:
- USB 2.0 or USB 3.x legacy hosts might be used if they can provide 500 mA without data communication. In
practice, today nearly all these ports deliver 500 mA without data communication.

It is preferable to use USB ports showing charging port marking in that case ( ), as they enable higher
currents.
- Any other USB Type-C® source type can supply and start up the STM32N6570-DK board, up to 3 A. When
powered through USB1(CN18), LED (LD9) lights up.

UM3300 - Rev 1 page 14/49

<!-- page: 15 -->
UM3300
Hardware layout and configuration

Figure 9. USB1 (JP2) from USB Type-C® (CN18)

7.4.4 5V_IN
Figure 10 shows the selection of 5 V DC power from 5V_IN on JP2, with a power source connected to external
VIN on CN8 pin 8. VIN is then used by an embedded LDO to generate the 5V_VIN power supply.
In this case, the STM32N6570-DK product must be powered by a power supply unit or by auxiliary equipment
complying with the standard EN 62368-1:2014+A11:2017 and be safety extralow voltage (SELV) with limited
power capability.

Note: There is no input current protection in this configuration. The recommended maximum current to be drawn from
this 5V_IN pin is 800 mA (depending on VIN source voltage and capability).

Figure 10. 5V_IN (JP2) from VIN (CN8)

7.4.5 MCU power supply: Internal or external SMPS
Internal SMPS configuration: Using the internal SMPS
External SMPS default configuration: Using an external SMPS. The output power could be configured to two
modes:
- Nominal mode: PWR_LP = 0, VDDCORE = 0.81 V
- Overdrive mode: PWR_LP = 1, VDDCORE = 0.89 V

Table 6. Internal or external SMPS configuration

External SMPS
Solder bridges(1) Internal SMPS
(default configuration)

R12 ON OFF

R21 ON OFF

R173 ON OFF

R22 ON OFF

R179 OFF ON

R30 OFF ON

1. The default setting is in bold.

UM3300 - Rev 1 page 15/49

<!-- page: 16 -->
UM3300
Hardware layout and configuration

7.4.6 Consumption measurement
Using CN2 pins to measure the current of the MCU powers.
- The pin 1/pin 2 pair can measure the input current for an external SMPS (R23 OFF)
- The pin 3/pin 4 pair can measure the current of VDDCORE when in external SMPS mode (R31 OFF)
- The pin 5/pin 6 pair can measure the input current for the internal SMPS (R19 OFF)
- The pin 9/pin 10 pair can measure the current of VDDCORE when in internal SMPS mode (R167 OFF)
- The pin 11/pin 12 pair can measure the current of VDDIO (R79 OFF)
- The pin 13/pin 14 pair can measure the current of VDDA1V8 (R187 OFF)
The user can measure the current of VDD33USB through R159.
The user can measure the current of VDDA18AON through R163.

7.5 Clock sources
Four clock sources are available on the STM32N6570-DK board, as described below:
- X1 24 MHz crystal for the STLINK-V3EC
- X2 48 MHz crystal for the STM32N657X0H3Q HSE system clock
- X3 32.768 kHz crystal for the STM32N657X0H3Q embedded RTC
- X4 25 MHz crystal for Ethernet

7.6 Reset sources
The general reset of the STM32N6570-DK board is active LOW. The reset sources include:
- Reset button (B1)
- Embedded STLINK-V3EC
- ARDUINO® Uno shield board through ARDUINO® connector (CN8 pin 3)
- MIPI20 (CN10)
- TAG connector (CN1)
The general reset is connected to the following peripheral reset functions:
- Octo‑SPI flash memory
- microSD™ power
- Ethernet

7.7 Boot options
BOOT0 and BOOT1 pins determine the boot mode as shown in Table 7. For more details, refer to the RM0486
reference manual available at www.st.com.

Table 7. Boot modes

BOOT0 BOOT1 Boot source #1

- 1 Development boot

0 0 Flash boot

1 0 Serial boot

On the board, BOOT0 and BOOT1 might be configured manually by pushing the mechanical parts: SW1(BOOT1)
and SW2(BOOT0).

UM3300 - Rev 1 page 16/49

<!-- page: 17 -->
UM3300
Hardware layout and configuration

7.8 TFT color LCD 800x480 pixels
The STM32N6570-DK board includes a 5‑inch 800x480 LCD‑TFT board (MB1860), which is connected to the
RGB interface of STM32N657X0H3Q through a 50‑pin connector (CN3). The MB1860 LCD daughterboard uses a
TFT LCD with the driving system, white LED backlight, and capacitive touch panel.
The touch-panel controller interfaces with STM32N657X0H3Q via the bidirectional I2C2 bus: SCL (PD14) and
SDA (PD4). The I2C write‑read address is 0xBA/0xBB.

Warning: LCD_R3 (PB4) and LCD_R5 (PA15) are multiplexed with JTAG.

7.9 USB1 USB Type-C® (HS, DRP)
The STM32N6570-DK board supports a USB HS 2.0 interface on the USB Type-C® receptacle connector (CN18).
It offers compatibility with USB Type-C® revision 1.3, USB PD 3.0, PPS, and USB BC 1.2 on the USB Type-C®
receptacle connector (CN18).
CN18 can be used as a DRP (dual‑role port). Its VBUS can be managed to supply other platforms as a Provider
or be supplied as a Consumer. The embedded UCPD protection manages DRP functions. It is compatible with
VBUS current up to 1.5 A and VBUS 5 V only.
By default, the embedded UCPD protection manages the dead battery (DB) feature of this USB connector. It
communicates with MCU via the bidirectional I2C2 bus: SCL (PD14) and SDA (PD4). The I2C write/read address
is 0x68/0x69.
The red LED (LD10) indicates the USB OCP status.
The green LED (LD9) lights up when one of the following events occurs:
- The source path is open, and STM32N6570-DK provides up to 1.5 A 5 V power to CN18.
- VBUS1 is powered by another USB host when the STM32N6570-DK board works as a Sink device.

7.10 USB2 Type-A (HS, host only)
USB2 is used as a host at high speed, and the current is limited to 0.5 A maximum. The USB2 connector (CN17)
is a USB Type-A connector.
The red LED (LD8) indicates the power fault status of USB2.

7.11 Octo‑SPI flash memory
The Octo‑SPI flash memory has the following characteristics: 1-Gbit, 1.8 V, 200 MHz, DTR, read while writing. It is
connected to the OCTOSPI interface of the STM32N657X0H3Q microcontroller on the STM32N6570-DK board.

Note: Since the NRST (system reset) is at 3.3 V level, while the reset of the flash memory is 1.8 V in this design, a
diode (D4) is used to adapt the reset signals.

7.12 Hexadeca‑SPI PSRAM
The Hexadeca‑SPI PSRAM has the following characteristics: 256 Mbits, 1.8 V, 200 MHz, DDR. It is connected to
the Hexadeca‑SPI interface of the STM32N657X0H3Q microcontroller on the STM32N6570-DK board.

7.13 Ethernet
The STM32N6570-DK board supports 10/100/1000-Mbit Ethernet communication with a PHY and integrates an
RJ45 connector (CN16). The Ethernet PHY is connected to the STM32N657X0H3Q microcontroller via an RGMII
interface.
The 25 MHz clock of the PHY is generated from the X4 crystal.

UM3300 - Rev 1 page 17/49

<!-- page: 18 -->
UM3300
Hardware layout and configuration

7.14 microSD™ card
A slot (CN13) for microSD™ card (UHS-I supported) is available on the STM32N6570-DK board and is connected
to the SDMMC2 interface of STM32N657X0H3Q.
The uSD_Detect signal (PN12) manages the microSD™ card detection. When a microSD™ card is inserted into
the slot, the uSD_Detect signal level is LOW, otherwise, it is HIGH.
The SD_SEL signal (connected to PO5) manages the power selection (1.8/3.3 V).
The system reset (NRST) and PWR_SD_EN(PQ7) signals enable or disable the power switch. PWR_ON pulls up
these two signals.

7.15 Audio
STM32N6570-DK features audio characteristics such as:
- An audio codec with an SAI interface and a 3.5 mm audio jack connector (for stereo earphones and one
analog microphone)
- A digital MEMS microphone and microphone daughterboard connector

7.15.1 Audio codec
An audio codec is driven through the I2C2 interface (100 kHz). The I2C write/read address is 0x34/0x35.
An SAI interface transfers audio data. A dedicated I/O (Audio_INT, PB1, active LOW) is used for the IRQ interrupt.

Warning: The SAI signals are multiplexed with trace signals.

7.15.2 MEMS microphone
A digital MEMS microphone (U13) is ON on the bottom side of the STM32N6570-DK main board.
A microphone daughterboard connector (CN5) is implemented close to U13, to support the STEVAL-
MIC008A MEMS microphone daughterboard. Only two microphones (left and right channels) on these boards can
be supported due to only one data signal on CN5.
By default, U13 is connected to the data signal of SAI_PDM on the STM32N657X0H3Q microcontroller. When a
microphone daughterboard is plugged into CN5, the data signal is switched to the microphone daughterboard
automatically (through U14).
Note that a second reserved data pin (PE12) in CN5 pin5 is multiplexed with the ARDUINO® connector.

7.16 Camera module
STM32N6570-DK features a CSI camera module (with ToF and IMU) (MB1854) through CN14.
Refer to B-CAMS-IMX for more details.
This camera module daughterboard provides a compelling hardware set to handle several computer vision
scenarios and use cases. It features a high-resolution 5‑Mpx CMOS RGB image sensor, an inertial motion unit,
and a ToF sensor. It can be used with any STM32 board featuring a MIPI CSI-2® interface with a 22‑pin FFC
connector to enable full-featured computer vision on STM32 microcontrollers and microprocessors easily.

7.17 ARDUINO®
STM32N6570-DK features ARDUINO® Uno R3.

Note: The analog level limit is 1.8 V while digital I/O is up to 3.3 V in the MCU pins. Thus, to avoid any risk of
damaging the MCU pins, an amplifier is used to down the voltage level for each analog pin.
This implements two functions (analog and digital) in the ARDUINO® pins (A0–A5), connecting two pins (one
analog and one digital I/O) to each ARDUINO® pin (A0–A5). The basic logic is shown in Figure 11.
Note: There is a possibility to get the wake‑up feature for D4 through SB44 (OFF by default).

UM3300 - Rev 1 page 18/49

<!-- page: 19 -->
UM3300
Hardware layout and configuration

Figure 11. ADC ARDUINO® input voltage adaptation

3V3 ADC from Amplifier 1V8 ADC ADC STM32N657X0H3Q ARDUINO®
MCU

DT59556V1

Warning: USB1 ISENSE shares the same ADC with ARDUINO® ADC0 (A0).
STMod+ shares the same ADC (CN4 pin13) with ARDUINO® ADC5 (A5).

7.18 STMod+
STM32N6570-DK features STMod+.

Note: For the ADC pin in the STMod+ connector, the same solution as for ARDUINO® A0 to A5 pins is adapted,
connecting two pins (one analog and one digital I/O) to the ADC pin.

Warning: STMod+ function causes the loss of an SPI on CN4 pin9 due to the lack of available SPI
resources.
STMod+ shares the same ADC (CN4 pin13) with ARDUINO® ADC5 (A5)

7.19 Virtual COM port
The serial interface UART1 (PE5/PE6) that supports the bootloader is directly available as a Virtual COM port of
the PC connected to the STLINK-V3EC USB connector (CN6). The VCP configuration is the following:
- 115200 bit/s
- 8-bit data
- Even parity
- One-stop bit
- No flow control

7.20 TAG
One TAG interface footprint (CN1) is reserved on the STM32N6570-DK board, which can be used for the board
debugging and programming.

Warning: The JNRST(PB4) and MCU.JTDI(PA15) are multiplexed with LCD signals.

7.21 MIPI20
One MIPI20 connector (CN10) is on the STM32N6570-DK board, which can be used for the board debugging and
programming.

Warning: The trace signals (PB0, PB6, PB7, PE3, and PG7) are multiplexed with audio SAI signals.

UM3300 - Rev 1 page 19/49

<!-- page: 20 -->
UM3300
Hardware layout and configuration

7.22 Buttons and LEDs
The black button (B1) on the top side is the reset of the STM32N657X0H3Q microcontroller.
The blue button (B2) on the top side is used as the user1 button (wake-up-alternate function).
The blue button (B4) on the top side is used as a tamper button.

When the black button (B1) is pressed, the logic state is LOW, otherwise, the logic state is HIGH.

The LD2 LED might be used for BOOTFAILEDN signal with LED for boot ROM failure detection.

Table 8 summarizes the different buttons and LEDs of the STM32N6570-DK Discovery kit and their function:

Table 8. Button and LED control ports

Reference Color Function Description

B1 Black Reset button Resets the board.

Supports the wake-up and tamper features
B2 Blue User1 button
(PC13).

B4 Blue Tamper button PE0

LD1 Green User LED Active HIGH (PO1)

Active LOW (PG10)
LD2 Red User LED
(Might indicate BOOTFAILEDN)

Power status:(1)
- OFF: Target not powered by ST-LINK
- Orange: Requested power higher than
USB power budget LD3 Tricolor (orange/green/red) STLINK-V3EC power status
- Green: Requested power lower or
equal to USB power budget
- Red: Overcurrent detected
- Blinking red: Internal error

LD4 Bicolor (red/green) STLINK-V3EC COM Green when communication is ongoing

LD5 Green User 5 V 5 V ON

LD6 Green ARDUINO® D13 Active HIGH (PE15)

LD7 Green Ethernet LED2 Ethernet status

LD8 Red USB2 FAULT USB2 fault

LD9 Green USB1 5V VBUS1 is present

LD10 Red USB1 OVP It indicates USB1 OVP status.

1. For detailed information on the power status LED, refer to the technical note Overview of ST-LINK derivatives (TN1235),
section 7.

UM3300 - Rev 1 page 20/49

<!-- page: 21 -->
UM3300
Board connectors

8 Board connectors

8.1 TAG connector (CN1)
The TAG connector footprint (CN1) is used to connect the STM32N657X0H3Q microcontroller for programming or
debugging the board.

Figure 12. TAG connector (CN1)

Table 9. TAG connector (CN1) pinout

Pin number Description Pin number Description

1 VDDIO (3V3) 10 NRST

2 JTMS (PA13) 9 JNRST (PB4)

3 GND 8 JTDI (PA15)

4 JTCK (PA14) 7 NC

5 GND 6 JTDO (PB5)

8.2 Consumption measurement connector (CN2)

Figure 13. Consumption measurement connector (CN2) top view

The CN2 connector is used to measure the consumption of MCU powers.

Table 10. Consumption measurement connector (CN2) pinout

Pin number Description Pin number Description

1 External SMPS input power_P 2 External SMPS input power_N

3 External SMPS output power (VDDCORE)_P 4 External SMPS output power (VDDCORE)_P

5 Internal SMPS input power_P 6 Internal SMPS input power (VDDCORE)__N

7 GND 8 GND

9 Internal SMPS output power (VDDCORE)_P 10 Internal SMPS output power (VDDCORE)_P

11 VDDIO_P 12 VDDIO_N

13 VDDA1V8_P 14 VDDA1V8_N

UM3300 - Rev 1 page 21/49

<!-- page: 22 -->
UM3300
Board connectors

8.3 TFT LCD connector (CN3)
The CN3 connector is designed to connect the 5-inch TFT LCD touchscreen board.

Figure 14. TFT LCD connector (CN3) top view

Table 11. TFT LCD connector (CN3) pinout

MCU port Signal name Pin number Signal name MCU port

- GND 1 2 GND -

PG0 LCD_R0 3 4 LCD_G0 PG12

PD9 LCD_R1 5 6 LCD_G1 PG1

PD15 LCD_R2 7 8 LCD_G2 PA1

PB4(1) LCD_R3 9 10 LCD_G3 PA0

PH4 LCD_R4 11 12 LCD_G4 PB15

PA15(2) LCD_R5 13 14 LCD_G5 PB12

PG11 LCD_R6 15 16 LCD_G6 PB11

PD8 LCD_R7 17 18 LCD_G7 PG8

- GND 19 20 GND -

PG15 LCD_B0 21 22 LCD_DE PG13

PA7 LCD_B1 23 24 LCD_ON/OFF PQ3

PB2 LCD_B2 25 26 LCD_HSYNC PB14

PG6 LCD_B3 27 28 LCD_VSYNC PE11

PH3 LCD_B4 29 30 GND -

PH6 LCD_B5 31 32 LCD_CLK PB13

PA8 LCD_B6 33 34 GND -

PA2 LCD_B7 35 36 NRST PE1

- GND 37 38 I2C1_SDA PD4

PQ4 CTP_INT 39 40 I2C1_SCL PD14

- NC 41 42 NC -

PQ6 LCD_BL_CTRL 43 44 NC -

- 5V 45 46 NC -

- GND 47 48 NC -

- GND 49 50 3V3 -

1. PB4 is multiplexed with JNRST.
2. PA15 is multiplexed with JTDI.

UM3300 - Rev 1 page 22/49

<!-- page: 23 -->
UM3300
Board connectors

8.4 STMod+ connector (CN4)
The standard 20-pin STMod+ connector is available on the STM32N6570-DK board to increase compatibility with
external boards and modules from the ecosystem of microcontrollers. STMod+ includes UART or SPI interface
signals for communication with the host MCU and the dedicated solder bridges allow configuring the external
board to be controlled by the UART2 or SPI5 serial interface of the STM32N657X0 MCU.

Figure 15. STMod+ connector (CN4) front view

Table 12. STMod+ connector (CN4) configuration

Solder bridge Setting(1) Description

SB10, SB12, SB14, SB17 ON UART2 connected to STMod+

SB11, SB13, SB15, SB18 OFF SPI5 disconnected to STMod+

SB10, SB12, SB14, SB17 OFF UART7 disconnected to STMod+

SB11, SB13, SB15, SB18 ON SPI5 connected to STMod+

SB16 ON SB16 ON to enable the ADC (Share ARDUINO® A5)

1. The default configuration is in bold.

By default, it is designed to support an ST-dedicated fan-out board to connect different modules or board
extensions from different manufacturers.
The STMod+ fan-out expansion board also embeds a 3.3 V regulator and I2C level shifters. For more detailed
information on the fan-out board, refer to the user manual STMod+ fan-out expansion board for STM32 Discovery
kits and Evaluation boards (UM2695).
For details about the STMod+ interface, refer to the technical note STMod+ interface specification (TN1238).

Table 13. STMod+ connector (CN4) pinout

Pin Pin
Description Description
number number

1 SPI5_CS (PA3)/USART2_CTS (PG5) 11 INT (PC11)

2 SPI5_MOSI (PG2)/USART2_TX (PD5) 12 RESET (PB3)

ADC (PB10) (share ARDUINO® A5) or I/O(PC9)
3 SPI5_MISO (PH8)/USART2_RX (PF6) 13 (1)(2)

4 SPI5_SCK (PE15)/USART2_RTS (PG14) 14 PWM (PC7)

5 GND 15 +5 V

6 +5 V 16 GND

7 I2C1_SCL (PH9) 17 GPIO (PD13)

8 SPI5_MOSIs (PH7) 18 GPIO (PF1)

9 I/O (PC6) (missing MISO signal)(3) 19 GPIO (PB8)

10 I2C1_SDA (PC1) 20 GPIO (PG9)

1. Since for ADC, the level of MCU is 1.8 V, while for digital I/O the level of MCU pins is 3.3 V.
2. STMod+ shares the same ADC (CN4 pin13) with ARDUINO® ADC5 (A5).
3. Losing one SPI for CN4 pin9 due to no spare SPI resource.

Note: On the STM32N6570-DK board, signals on STMod+ are shared with ARDUINO® connectors. The user must
make sure that nothing is connected to ARDUINO® connectors.

UM3300 - Rev 1 page 23/49

<!-- page: 24 -->
UM3300
Board connectors

8.5 Microphone connector (CN5)
The microphone connector (CN5) connects the STEVAL-MIC008A MEMS microphone daughterboard.

Figure 16. Microphone connector (CN5)

Table 14. Microphone connector (CN5) pinout

Pin number Description Pin number Description

1 GND 2 VDD_MIC (3.3 V)

3 NC 4 PDM clock (CK-PE2)

5 PDM data reserved (PE12)(1) 6 PDM data (D1-PE8)

7 NC 8 NC

9 NC 10 Microphone board detection

11 NC 12 VDD_MIC (3.3 V)

13 NC 14 NC

15 NC 16 NC

17 NC 18 NC

19 VDD_MIC (3.3 V) 20 GND

1. PE12 is a second data, reserved through solder bridge SB41 (OFF by default).
PE12 is multiplexed with ARDUINO® A2 pin.

UM3300 - Rev 1 page 24/49

<!-- page: 25 -->
UM3300
Board connectors

8.6 STLINK-V3EC USB Type-C® connector (CN6)
The USB connector (CN6) is used to connect the embedded STLINK-V3EC to the PC for programming and
debugging purposes.

Figure 17. STLINK-V3EC connector (CN6) front view

Table 15. STLINK-V3EC connector (CN6) pinout

Pin number Description Pin number Description

A1 GND B1 GND

A2 NC B2 NC

A3 NC B3 NC

A4 VBUS_STLK B4 VBUS_STLK

A5 CC1 (STLINK MCU PC3) B5 CC2 (STLINK MCU PC4)

A6 D+ (STLINK MCU PB15) B6 D+ (STLINK MCU PB15)

A7 D- (STLINK MCU PB14) B7 D- (STLINK MCU PB14)

A8 NC B8 NC

A9 VBUS_STLK B9 VBUS_STLK

A10 NC B10 NC

A11 NC B11 NC

A12 GND B12 GND

8.7 ARDUINO® Uno V3 connectors (CN7, CN8, CN11, and CN12)
The ARDUINO® Uno V3 connectors ((CN7, CN8, CN11, and CN12) are female connectors compatible with the
ARDUINO® Uno revision 3 standard. Most shields designed for ARDUINO® Uno V3 fit the STM32N6570-DK
board.

Important: The STM32 microcontroller I/Os are 3.3 V compatible instead of 5 V for ARDUINO® Uno

UM3300 - Rev 1 page 25/49

<!-- page: 26 -->
UM3300- Table 16. ARDUINO® Uno V3 connectors pinout
Rev Left connectors Right connectors
1
Connector Pin Pin name MCU pin SoC function SoC function MCU pin Pin name Pin Connector
number number

I2/3C1_SCL PH9 D15 10

I2/3C1_SDA PC1 D14 9

AVDD - AREF 8

Ground - GND 7

1 - - 5V_IN test SPI5_SCK PE15 D13 6
CN12 Digital
2 IOREF - 3.3 V ref SPI5_MISO PH8 D12 5

3 RESET NRST RESET TIM14_CH1 or SPI5_MOSI PG2 D11 4

4 +3V3 - 3.3 V output TIM16_CH1 or SPI_CS PA3 D10 3
CN8 Power
5 +5V - 5 V output TIM1_CH4 PE14 D9 2

6 GND - Ground - PE7 D8 1

7 GND - Ground

8 VIN - Power input - PD6 D7 8

TIM1_CH3 PE13 D6 7

1 A0 PA5 or PD7(1) ADC12_INP18(PA5) TIM1_CH2N PE10 D5 6

2 A1 PA9 or PC10(1) ADC12_INP10(PA9) - PH5 D4 5

3 A2 PA10 or PE12(1) ADC12_INP11(PA10) TIM1_CH1 PE9 D3 4 CN11 Digital
CN7 Analog 4 A3 PA12 or PD11(1) ADC12_INP13(PA12) - PD0 D2 3

5 A4 PF3/PC12(1) or ADC12_INP16(PF3) or USART2_TX PD5 D1 2
PC1(2) I2C1_SDA(PC1)

6 A5 PB10/PH2(1) or ADC12_INP8(PB10) or USART2_RX PF6 D0 1
PH9(2) I2C1_SCL(PH9)

1. Since for ADC, the level of MCU pins is 1.8 V, while for digital I/Os the level of MCU pins is 3.3 V.
2. By default, the I2C1 function is disabled with the configuration: SB32 and SB34 OFF, SB31 and SB33 ON. To get the I2C1 function, the configuration must be: SB32 and SB34 ON, Board
SB31 and SB33 OFF.
page26/49 connectors UM3300

<!-- page: 27 -->
UM3300
Board connectors

8.8 MIPI20 connector (CN10)
The MIPI20 debug connector (CN10) is implemented to program and debug the STM32N657X0 microcontroller.

Figure 18. MIPI20 connector (CN10) top view

Table 17. MIPI20 connector (CN10) pinout

Pin number Description Pin number Description

1 VDDIO 2 SWDIO(PA13)

3 GND 4 SWCLK(PA14)

5 GND 6 SWO(PB5)

7 - 8 JTDI(PA15)(1)

9 GND 10 NRST

11 GND(2) 12 TRACECLK(PG7)(3)

13 GND(2) 14 TRACED0(PE3)(3)

15 GND 16 TRACED1(PB0)(3)

17 GND 18 TRACED2(PB6)(3)

19 GND 20 TRACED3(PB7)(3)

1. JTDI is multiplexed with LCD signal.
2. Through SB54 to GND. By default, SB54 is OFF.
3. Trace signals are multiplexed with audio SAI signals.

UM3300 - Rev 1 page 27/49

<!-- page: 28 -->
UM3300
Board connectors

8.9 microSD™ card connector (CN13)
microSD™ cards with 4 Gbytes or more capacity can be inserted in the receptacle (CN13). Four data bits of the
SDMMC2 interface, CLK, and CMD signals of the STM32N657X0H3Q are used to communicate with the
microSD™ card. The SD_Detect signal detects the card insertion. When a microSD™ card is inserted, the
SD_Detect level is LOW, otherwise, it is HIGH.

Note: There are also some control signals to manage the power of the SD card: SD_SEL (PO5), PWR_SD_EN (PQ7),
and NRST.

Figure 19. microSD™ connector (CN13) top view

Table 18. microSD™ connector (CN13) pinout

Pin number Description Pin number Description

1 SDMMC2_D2 (PC0) 6 GND

2 SDMMC2_D3 (PE4) 7 SDMMC2_D0 (PC4)

3 SDMMC2_CMD (PC3) 8 SDMMC2_D1 (PC5)

4 VDD_SD 9 GND

5 SDMMC2_CK (PC2) 10 SD_Detect (PN12)

UM3300 - Rev 1 page 28/49

<!-- page: 29 -->
## Visual Summary (Page 29)

- page_class: timing_diagram
- confidence: 0.95
- reason_codes: dense_table_readable, prose_present

Page 29 of UM3300 datasheet describes the Camera module connector (CN14) on the STM32N6570-DK Discovery board, featuring a 22-pin ZIF connector for CSI camera modules with ToF and IMU. Includes Figure 20 showing top view layout and Table 19 detailing pinout assignments including GND, CSI signals, IMU interrupts, NRST_CAM, I2C lines, and VDD_CAM.

<!-- page: 30 -->
UM3300
Board connectors

8.11 Audio jack (CN15)
A 3.5 mm stereo audio jack is available on the STM32N6570-DK board to support stereo earphones and an
analog microphone.

Figure 21. Audio jack (CN15) top view

Table 20. Audio jack (CN15) pinout

Pin number Description Stereo headset with microphone pinning

6 OUT_Left

4 OUT_Right

3 GND

2 MIC_IN

8.12 Ethernet RJ45 connector (CN16)
The STM32N6570-DK board supports 10/100/1000 Mbit/s Ethernet communications with PHY, and the integrated
RJ45 connector (CN16). The Ethernet PHY is connected to the STM32N657X0H3Q microcontroller through an
RGMII interface.
The PHY 25 MHz clock is generated from the X4 crystal.

Figure 22. Ethernet RJ45 connector (CN16) front view

Table 21. Ethernet RJ45 connector (CN16) pinout

Pin number Description Pin number Description

1 TX1+ 8 TX3-

2 TX1- 9 TX4+

3 TX2+ 10 TX4-

4 TX2- 11 GA

5 CT1 12 GC

6 CT2 13 YA

7 TX3+ 14 YC

UM3300 - Rev 1 page 30/49

<!-- page: 31 -->
UM3300
Board connectors

8.13 User USB2 Type-A connector (CN17)
CN17 is a USB Type-A connector (host). For more details, refer to Section 7.10.

Figure 23. User USB2 Type-A connector (CN17) front view

Table 22. User USB2 Type-A connector (CN17) pinout

Pin number Description

1 VBUS2

2 2_HSDM

3 2_HSDP

4 GND

8.14 User USB1 USB Type-C® connector (CN18)
CN18 is a USB Type-C® connector (DRP). For more details, refer to Section 7.9.

Figure 24. User USB1 USB Type-C® connector (CN18) front view

Table 23. User USB1 USB Type-C® connector (CN18) pinout

Pin number Description Pin number Description

A1 GND B1 GND

A2 NC B2 NC

A3 NC B3 NC

A4 VBUS1 B4 VBUS1

A5 CC1 B5 CC2

A6 D+ B6 D+

A7 D- B7 D-

A8 NC B8 NC

A9 VBUS1 B9 VBUS1

A10 NC B10 NC

A11 NC B11 NC

A12 GND B12 GND

UM3300 - Rev 1 page 31/49

<!-- page: 32 -->
UM3300
STM32N6570-DK I/O assignment

9 STM32N6570-DK I/O assignment

Table 24. STM32N6570-DK I/O assignment

Pin Pin name On‑board ARDUINO® signals STMod+ signals number peripheral signals

U10 PA0 LCD_G3 - -

P9 PA1 LCD_G2 - -

T8 PA2 LCD_B7 - -
R14 PA3 - D10‑TIM16_CH1/SPI_CS P1-SPI_CS

U14 PA4 USB1_EN - -

R8 PA5 - A0-ADC2 INP18(1) -

P8 PA6 BOOT1 - -

R12 PA7 LCD_B1 - -

U8 PA8 LCD_B6 - -

P7 PA9 - A1-ADC1/2 INP10(1) -

R7 PA10 - A2-ADC1/2 INP11(1) -

T7 PA11 USB1_OCP - -

U7 PA12 - A3-ADC1/2 INP13(1) -

U6 PA13(JTMS/SWDIO) MCU.SWDIO - -

T6 PA14(JTCK/SWCLK) MCU.SWCLK - -

P12 PA15(JTDI) PA15 - -

E15 PB0 SAI1_FS_A - -

R11 PB1 AUDIO_INT - -

C16 PB2 LCD_B2 - -

F13 PB3 - - P12-RST

T12 PB4(NJTRST) LCD_R3 - -

U12 PB5(JTDO/TRACESWO) MCU.SWO - -

E17 PB6 SAI1_CLK_A - -

E16 PB7 SAI1_SD_A - -

D16 PB8 - - P19-GPIO

D15 PB9 PWR_USB2_EN - -

U11 PB10 - A5-ADC1/2 INP8(1) P13-ADC1/2 INP8(1)

P10 PB11 LCD_G6 - -

R10 PB12 LCD_G5 - -

C15 PB13 LCD_CLK - -

B17 PB14 LCD_HSYNC - -

D17 PB15 LCD_G4 - -

C10 PC0 SD_D2 - -

A9 PC1 - D14-I2C1/I3C1_SDA P10-I2C1_SDA

D9 PC2 SD_CK - -

E10 PC3 SD_CMD - -

A10 PC4 SD_D0 - -

UM3300 - Rev 1 page 32/49

<!-- page: 33 -->
UM3300
STM32N6570-DK I/O assignment

Pin Pin name On‑board ARDUINO® signals STMod+ signals number peripheral signals

B10 PC5 SD_D1 - -

B9 PC6 - - P9-GPIO(2)

C9 PC7 - - P14-TIM3_CH2

D7 PC8 NRST_CAM - -

C7 PC9 - - P13-GPIO(1)

A8 PC10 - A1D(1) -

B8 PC11 - - P11-INT

C8 PC12 - A4D(1) -

F3 PC13 USER1 - -

D1 PC14-OSC32_IN(OSC32_IN) PC14 - -

PC15-
C1 PC15 - -
OSC32_OUT(OSC32_OUT)

D13 PD0 - D2 -

A14 PD1 ETH_MDC - -

C12 PD2 EN_MODULE - -

E13 PD3 ETH_MDINT - -

B15 PD4 I2C2_SDA - -

D14 PD5 - D1-USART2_TX P2-USART2_TX

D12 PD6 - D7 -

E12 PD7 - A0D(1) -

E14 PD8 LCD_R7 - -

H14 PD9 LCD_R1 - -

A13 PD10 UCPD1_INT - -

G14 PD11 - A3D(1) -

C13 PD12 ETH_MDIO - -

C17 PD13 - - P17-GPIO

C14 PD14 I2C2_SCL - -

B14 PD15 LCD_R2 - -

F16 PE0 TAMP - -

F17 PE1 LCD_NRST - -

F15 PE2 CK - -

F14 PE3 SAI1_SD_B - -

D10 PE4 SD_D3 - -

C11 PE5 MCU.VCP_TX - -

D11 PE6 MCU.VCP_RX - -

B13 PE7 - D8 -

A15 PE8 D1 - -

A16 PE9 - D3-TIM1_CH1 -

B16 PE10 - D5-TIM1_CH2N -

E11 PE11 LCD_VSYNC - -

B12 PE12 - A2D(1) -

UM3300 - Rev 1 page 33/49

<!-- page: 34 -->
UM3300
STM32N6570-DK I/O assignment

Pin Pin name On‑board ARDUINO® signals STMod+ signals number peripheral signals

A11 PE13 - D6-TIM1_CH3 -

B11 PE14 - D9-TIM1_CH4 -

A12 PE15 - D13-SPI5_SCK P4-SPI5_SCK

R3 PF0 ETH_GTX_CLK - -

P4 PF1 - - P18-GPIO

N4 PF2 ETH_CLK125 - -

M2 PF3 - A4-ADC1 INP16(1) -

L2 PF4 EXT_SMPS_MODE - -

M3 PF5 ETH_CLK - -

L5 PF6 - D0-USART2RX P3-USART2_RX

M1 PF7 ETH_RX_CLK - -

N3 PF8 ETH_RXD2 - -

P1 PF9 ETH_RXD3 - -

L4 PF10 ETH_RX_DV - -

P3 PF11 ETH_TX_EN - -

T1 PF12 ETH_TXD0 - -

R2 PF13 ETH_TXD1 - -

N2 PF14 ETH_RXD0 - -

N1 PF15 ETH_RXD1 - -

R13 PG0 LCD_R0 - -

R9 PG1 LCD_G1 - -
T11 PG2 - D11‑TIM14_CH1/SPI5_MOSI P2-SPI5_MOSI

R1 PG3 ETH_TXD2 - -

P2 PG4 ETH_TXD3 - -

M4 PG5 - - P1-USART2_CTS

L3 PG6 LCD_B3 - -

L1 PG7 SAI1_MCLK_A - -

T14 PG8 LCD_G7 - -

T13 PG9 - - P20-GPIO

T10 PG10 LED2 - -

U13 PG11 LCD_R6 - -

T9 PG12 LCD_G0 - -

U9 PG13 LCD_DE - -

P6 PG14 - - P4-USART2_RTS

R6 PG15 LCD_B0 - -

A7 PH0-OSC_IN(PH0) PH0 - -

B7 PH1-OSC_OUT(PH1) PH1 - -

E7 PH2 - A5D(1) -

B1 PH3 LCD_B4 - -

F4 PH4 LCD_R4 - -

UM3300 - Rev 1 page 34/49

<!-- page: 35 -->
UM3300
STM32N6570-DK I/O assignment

Pin Pin name On‑board ARDUINO® signals STMod+ signals number peripheral signals

F5 PH5 - D4 -

B2 PH6 LCD_B5 - -

D2 PH7 - - P8-SPI5_MOSI

C2 PH8 - D12-SPI5_MISO P3-SPI5_MISO

D8 PH9 - D15-I2C1/I3C1_SCL P7-I2C1_SCL

P17 PN0 OCTOSPI_DQS - -

R17 PN1 OCTOSPI_NCS - -

T17 PN2 OCTOSPI_IO0 - -

R15 PN3 OCTOSPI_IO1 - -

N17 PN4 OCTOSPI_IO2 - -

R16 PN5 OCTOSPI_IO3 - -

P15 PN6 OCTOSPI_CLK - -

T16 PN7 UCPD1_VSENSE - -

P16 PN8 OCTOSPI_IO4 - -

T15 PN9 OCTOSPI_IO5 - -

U15 PN10 OCTOSPI_IO6 - -

U16 PN11 OCTOSPI_IO7 - -

P14 PN12 SD_DET - -

M17 PO0 HEXASPI_NCS - -

K16 PO1 LED1 - -

K17 PO2 HEXASPI_DQS0 - -

M16 PO3 HEXASPI_DQS1 - -

N16 PO4 HEXASPI_CLK - -

K15 PO5 SD_SEL - -

G17 PP0 HEXASPI_IO0 - -

H16 PP1 HEXASPI_IO1 - -

J17 PP2 HEXASPI_IO2 - -

J16 PP3 HEXASPI_IO3 - -

H15 PP4 HEXASPI_IO4 - -

J14 PP5 HEXASPI_IO5 - -

G16 PP6 HEXASPI_IO6 - -

G15 PP7 HEXASPI_IO7 - -

L16 PP8 HEXASPI_IO8 - -

N14 PP9 HEXASPI_IO9 - -

N15 PP10 HEXASPI_IO10 - -

L15 PP11 HEXASPI_IO11 - -

J15 PP12 HEXASPI_IO12 - -

K14 PP13 HEXASPI_IO13 - -

L17 PP14 HEXASPI_IO14 - -

H17 PP15 HEXASPI_IO15 - -

E5 PQ0 TOF_INT - -

UM3300 - Rev 1 page 35/49

<!-- page: 36 -->
UM3300
STM32N6570-DK I/O assignment

Pin Pin name On‑board ARDUINO® signals STMod+ signals number peripheral signals

E4 PQ1 IMU_INT1 - -

E3 PQ2 IMU_INT2 - -

D3 PQ3 LCD_ON/OFF - -

D4 PQ4 LCD_INT - -

D5 PQ5 TOF_LPn - -

A2 PQ6 LCD_BL_CTRL - -

G5 PQ7 PWR_SD_EN - -

1. ADC is in the 1.8 V power domain. I/O is in the 3.3 V power domain. For ADC pins in ARDUINO® and STMod+ connectors,
each pin is connected to two pins (ADC and I/O) of the MCU.
2. P9 of the STMod+ connector misses the SPI feature. This is a limitation.

UM3300 - Rev 1 page 36/49

<!-- page: 37 -->
UM3300
STM32N6570-DK product information

10 STM32N6570-DK product information

10.1 Product marking
The product and each board composing the product are identified with one or several stickers. The stickers,
located on the top or bottom side of each PCB, provide product information:

- Main board featuring the target device: product order code, product identification, serial number, and board reference
with revision.

Single-sticker example:

Product order code
Product identification
syywwxxxxx
MBxxxx-Variant-yzz DT73589V1

Dual-sticker example:

Product order code MBxxxx-Variant-yzz
and Product identification syywwxxxxx

- Other boards if any: board reference with revision and serial number.

Examples:

MBxxxx-Variant-yzz
or or or syywwxxxxx syywwxxxxx DT76508V1
MBxxxx-Variant-yzz DT76507V1

On the main board sticker, the first line provides the product order code, and the second line the product
identification.
On all board stickers, the line formatted as “MBxxxx-Variant-yzz” shows the board reference “MBxxxx”, the
mounting variant “Variant” when several exist (optional), the PCB revision “y”, and the assembly revision “zz”, for
example B01. The other line shows the board serial number used for traceability.
Products and parts labeled as “ES” or “E” are not yet qualified or feature devices that are not yet qualified.
STMicroelectronics disclaims any responsibility for consequences arising from their use. Under no circumstances
will STMicroelectronics be liable for the customer's use of these engineering samples. Before deciding to use
these engineering samples for qualification activities, contact STMicroelectronics' quality department.
“ES” or “E” marking examples of location:
- On the targeted STM32 that is soldered on the board (for an illustration of STM32 marking, refer to the
STM32 datasheet Package information paragraph at the www.st.com website).
- Next to the ordering part number of the evaluation tool that is stuck, or silk-screen printed on the board.
Some boards feature a specific STM32 device version, which allows the operation of any bundled commercial
stack/library available. This STM32 device shows a “U” marking option at the end of the standard part number
and is not available for sales.
To use the same commercial stack in their applications, the developers might need to purchase a part number
specific to this stack/library. The price of those part numbers includes the stack/library royalties.

UM3300 - Rev 1 page 37/49

<!-- page: 38 -->
UM3300
STM32N6570-DK product information

10.2 STM32N6570-DK product history

Table 25. Product history

Order Product
Product details Product change description Product limitations
code identification

MCU:
- STM32N657X0H3Q
silicon revision "B"

MCU errata sheet:
- STM32N6xxx device
errata (ES0620)

Boards:
- MB1280-3V3-C01
DK32N6570$CR1 Initial revision No limitation
(STMod+ fan-out
expansion board) STM32N6570-DK - MB1854-CSI-B01
(camera module
daughterboard)
- MB1860-
RK050HR18C-B01
(LCD daughterboard)
- MB1939-N6570-C02
(main board)

10.3 Board revision history

Table 26. Board revision history

Board variant and
Board reference Board change description Board limitations
revision

MB1280
(STMod+ fan-out expansion 3V3-C01 Initial revision No limitation
board)

MB1854
CSI-B01 Initial revision No limitation
(camera module)

MB1860
RK050HR18C-B01 Initial revision No limitation
(LCD daughterboard)

MB1939 P9 of the STMod+ connector misses
N6570-C02 Initial revision
(main board) the SPI feature.

UM3300 - Rev 1 page 38/49

<!-- page: 39 -->
UM3300
Federal Communications Commission (FCC) and ISED Canada Compliance Statements

11 Federal Communications Commission (FCC) and ISED Canada
Compliance Statements

11.1 FCC Compliance Statement

Part 15.19

This device complies with Part 15 of the FCC Rules. Operation is subject to the following two conditions: (1)
this device may not cause harmful interference, and (2) this device must accept any interference received,
including interference that may cause undesired operation.

Part 15.21

Any changes or modifications to this equipment not expressly approved by STMicroelectronics may cause
harmful interference and void the user's authority to operate this equipment.

Part 15.105

This equipment has been tested and found to comply with the limits for a Class A digital device, pursuant to part
15 of the FCC Rules. These limits are designed to provide reasonable protection against harmful interference
when the equipment is operated in a commercial environment. This equipment generates, uses, and can radiate
radio frequency energy and, if not installed and used in accordance with the instruction manual, may cause
harmful interference to radio communications. Operation of this equipment in a residential area is likely to cause
harmful interference in which case the user will be required to correct the interference at his own expense.

Responsible party (in the USA)

Francesco Doddo
STMicroelectronics, Inc.
200 Summit Drive | Suite 405 | Burlington, MA 01803
USA
Telephone: +1 781-472-9634

11.2 ISED Compliance Statement

ISED Canada ICES-003 Compliance Label: CAN ICES-3 (A) / NMB-3 (A).

Étiquette de conformité à la NMB-003 d'ISDE Canada : CAN ICES-3 (A) / NMB-3 (A).

UM3300 - Rev 1 page 39/49

<!-- page: 40 -->
UM3300
CE conformity

12 CE conformity

12.1 Warning

EN 55032 / CISPR32 (2012) Class A product

Warning: this device is compliant with Class A of EN55032 / CISPR32. In a residential environment, this
equipment may cause radio interference.
Avertissement : cet équipement est conforme à la Classe A de la EN55032 / CISPR 32. Dans un environnement
résidentiel, cet équipement peut créer des interférences radio.

UM3300 - Rev 1 page 40/49

<!-- page: 41 -->
UM3300
UKCA Compliance Statement

13 UKCA Compliance Statement

SIMPLIFIED UK DECLARATION OF CONFORMITY
Hereby, the manufacturer STMicroelectronics, declares that the radio equipment type “STM32N6570-DK” is in
compliance with the UK Radio Equipment Regulations 2017 (UK S.I. 2017 No. 1206). The full text of the UK
Declaration of Conformity is available at the following internet address: www.st.com.

UM3300 - Rev 1 page 41/49

<!-- page: 42 -->
UM3300
Product disposal

14 Product disposal

Disposal of this product: WEEE (Waste Electrical and Electronic Equipment)

(Applicable in Europe)

This symbol on the product, accessories, or accompanying documents indicates that the
product and its electronic accessories should not be disposed of with household waste at the
end of their working life.

To prevent possible harm to the environment and human health from uncontrolled waste
disposal, please separate these items from other type of waste and recycle them responsibly
to the designated collection point to promote the sustainable reuse of material resources.

Household users:

You should contact either the retailer where you buy the product or your local authority for
further details of your nearest designated collection point.

Business users:

You should contact your dealer or supplier for further information.

UM3300 - Rev 1 page 42/49

<!-- page: 43 -->
UM3300

Revision history

Table 27. Document revision history

Date Revision Changes

13-Dec-2024 1 Initial release.

UM3300 - Rev 1 page 43/49

<!-- page: 44 -->
UM3300
Contents

Contents

1 Features. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .2
2 Ordering information . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .3

2.1 Codification . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 3
3 Development environment . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .4

3.1 System requirements . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4

3.2 Development toolchains . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4

3.3 Demonstration software . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4

3.4 CAD resources . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4
4 Conventions. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .5
5 Safety recommendations . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .6

5.1 Targeted audience. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6

5.2 Handling the board . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6
6 Quick start . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .7

6.1 Getting started. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7
7 Hardware layout and configuration. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .8

7.1 Hardware block diagram and board layout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8

7.2 Mechanical drawing . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11

7.3 Embedded STLINK-V3EC . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .12

7.3.1 STLINK-V3E drivers . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12

7.3.2 STLINK-V3EC firmware upgrade . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12

7.4 Power supply . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .13

7.4.1 Power source selection . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14

7.4.2 STLK (for compatibility with legacy USB Host port). . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14

7.4.3 User USB1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14

7.4.4 5V_IN . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15

7.4.5 MCU power supply: Internal or external SMPS . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15

7.4.6 Consumption measurement . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16

7.5 Clock sources . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .16

7.6 Reset sources . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .16

7.7 Boot options. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .16

7.8 TFT color LCD 800x480 pixels . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .17
7.9 USB1 USB Type-C® (HS, DRP) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .17

7.10 USB2 Type-A (HS, host only) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .17
7.11 Octo‑SPI flash memory . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .17

UM3300 - Rev 1 page 44/49

<!-- page: 45 -->
UM3300
Contents

7.12 Hexadeca‑SPI PSRAM. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .17
7.13 Ethernet . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .17
7.14 microSD™ card . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .18

7.15 Audio. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .18

7.15.1 Audio codec . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 18

7.15.2 MEMS microphone . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 18

7.16 Camera module. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .18
7.17 ARDUINO® . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .18

7.18 STMod+ . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .19

7.19 Virtual COM port . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .19

7.20 TAG. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .19

7.21 MIPI20 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .19

7.22 Buttons and LEDs . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .20
8 Board connectors . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .21

8.1 TAG connector (CN1) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .21

8.2 Consumption measurement connector (CN2) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .21

8.3 TFT LCD connector (CN3). . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .22

8.4 STMod+ connector (CN4) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .23

8.5 Microphone connector (CN5). . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .24
8.6 STLINK-V3EC USB Type-C® connector (CN6) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .25
8.7 ARDUINO® Uno V3 connectors (CN7, CN8, CN11, and CN12) . . . . . . . . . . . . . . . . . . . . . . . 25

8.8 MIPI20 connector (CN10). . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .27
8.9 microSD™ card connector (CN13) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .28

8.10 Camera module connector (CN14) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .29

8.11 Audio jack (CN15). . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .30

8.12 Ethernet RJ45 connector (CN16) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .30

8.13 User USB2 Type-A connector (CN17) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .31
8.14 User USB1 USB Type-C® connector (CN18). . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .31
9 STM32N6570-DK I/O assignment . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .32
10 STM32N6570-DK product information . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .37

10.1 Product marking . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .37

10.2 STM32N6570-DK product history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .38

10.3 Board revision history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .38
11 Federal Communications Commission (FCC) and ISED Canada Compliance
Statements . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .39

UM3300 - Rev 1 page 45/49

<!-- page: 46 -->
UM3300
Contents

11.1 FCC Compliance Statement . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .39

11.2 ISED Compliance Statement . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .39
12 CE conformity . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .40

12.1 Warning . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .40
13 UKCA Compliance Statement . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .41
14 Product disposal . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .42
Revision history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .43
List of tables . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .47
List of figures. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .48

UM3300 - Rev 1 page 46/49

<!-- page: 47 -->
UM3300
List of tables

List of tables

Table 1. Ordering information. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 3
Table 2. Codification explanation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 3
Table 3. ON/OFF convention . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5
Table 4. USB Type-C® connector (CN6) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
Table 5. 5 V power configurations. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
Table 6. Internal or external SMPS configuration . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15
Table 7. Boot modes. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16
Table 8. Button and LED control ports. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 20
Table 9. TAG connector (CN1) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 21
Table 10. Consumption measurement connector (CN2) pinout. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 21
Table 11. TFT LCD connector (CN3) pinout. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 22
Table 12. STMod+ connector (CN4) configuration . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 23
Table 13. STMod+ connector (CN4) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 23
Table 14. Microphone connector (CN5) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 24
Table 15. STLINK-V3EC connector (CN6) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 25
Table 16. ARDUINO® Uno V3 connectors pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 26
Table 17. MIPI20 connector (CN10) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 27
Table 18. microSD™ connector (CN13) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 28
Table 19. Camera module connector (CN14) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 29
Table 20. Audio jack (CN15) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30
Table 21. Ethernet RJ45 connector (CN16) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30
Table 22. User USB2 Type-A connector (CN17) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 31
Table 23. User USB1 USB Type-C® connector (CN18) pinout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 31
Table 24. STM32N6570-DK I/O assignment . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 32
Table 25. Product history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 38
Table 26. Board revision history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 38
Table 27. Document revision history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 43

UM3300 - Rev 1 page 47/49

<!-- page: 48 -->
UM3300
List of figures

List of figures

Figure 1. STM32N6570-DK top view. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 1
Figure 2. STM32N6570-DK bottom view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 1
Figure 3. Hardware block diagram . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8
Figure 4. STM32N6570-DK PCB layout top view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9
Figure 5. STM32N6570-DK PCB layout bottom view. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
Figure 6. Board mechanical drawing (in millimeters) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
Figure 7. Power diagram . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 13
Figure 8. STLK (JP2) from USB STLK (CN6) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
Figure 9. USB1 (JP2) from USB Type-C® (CN18) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15
Figure 10. 5V_IN (JP2) from VIN (CN8) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15
Figure 11. ADC ARDUINO® input voltage adaptation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 19
Figure 12. TAG connector (CN1) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 21
Figure 13. Consumption measurement connector (CN2) top view. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 21
Figure 14. TFT LCD connector (CN3) top view. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 22
Figure 15. STMod+ connector (CN4) front view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 23
Figure 16. Microphone connector (CN5) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 24
Figure 17. STLINK-V3EC connector (CN6) front view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 25
Figure 18. MIPI20 connector (CN10) top view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 27
Figure 19. microSD™ connector (CN13) top view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 28
Figure 20. Camera module connector (CN14) top view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 29
Figure 21. Audio jack (CN15) top view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30
Figure 22. Ethernet RJ45 connector (CN16) front view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30
Figure 23. User USB2 Type-A connector (CN17) front view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 31
Figure 24. User USB1 USB Type-C® connector (CN18) front view . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 31

UM3300 - Rev 1 page 48/49

<!-- page: 49 -->
UM3300

IMPORTANT NOTICE – READ CAREFULLY

STMicroelectronics NV and its subsidiaries (“ST”) reserve the right to make changes, corrections, enhancements, modifications, and improvements to ST
products and/or to this document at any time without notice. Purchasers should obtain the latest relevant information on ST products before placing orders. ST
products are sold pursuant to ST’s terms and conditions of sale in place at the time of order acknowledgment.

Purchasers are solely responsible for the choice, selection, and use of ST products and ST assumes no liability for application assistance or the design of
purchasers’ products.

No license, express or implied, to any intellectual property right is granted by ST herein.

Resale of ST products with provisions different from the information set forth herein shall void any warranty granted by ST for such product.

ST and the ST logo are trademarks of ST. For additional information about ST trademarks, refer to www.st.com/trademarks. All other product or service names
are the property of their respective owners.

Information in this document supersedes and replaces information previously supplied in any prior versions of this document.

© 2024 STMicroelectronics – All rights reserved

UM3300 - Rev 1 page 49/49
