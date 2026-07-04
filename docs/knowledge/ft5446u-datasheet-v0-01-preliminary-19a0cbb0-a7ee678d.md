<!-- page: 1 -->
FT5446U

True Multi-Touch
Capacitive Touch Panel Controller

INTRODUCTION

The FT5446U is single-chip capacitive touch panel controllers with built-in enhanced 32bit Micro-controller
unit (MCU).It provides the benefits of full screen common mode scan technology, fast response time and
high level of accuracy. It can drive capacitive type touch panel with up to 16 driving and 28 sensing lines.

FEATURES

- Mutual Capacitive Sensing Techniques
- Full Screen Common Mode Scan Techniques
- FT5446U Supports up to 16TX + 28 RX
- Support up to 10 fingers
- High immunity to inductive power noise
- Automatic mode switching (Active, Monitor, Sleep)
- Support 120Hz sampling rate
- Auto-calibration
- Support IIC (up to 400kbits/sec) interface
- Power
 2.7 to 3.6V Operating Voltage
 IOVCC supports from 1.71V to 3.6V

- Built-in 64KB Flash
- Single Channel(TX or RX)resistance: Up to 100K Ω
- Single Channel (transmit/receive) Capacitance: 40pF
- 12-Bit ADC Accuracy
- Features “short I/O ” testing for sense pins
- Supports various type of panels with no ground shielding layer
- 3 Operating Modes
 Active
 Monitor
 Sleep
- Package:
 QFN56L 6x6x0.6mm, 0.35mm/pitch

FocalTech Systems Co., Ltd. · www.focaltech-electronics.com · support@focaltech-electronics.com

Document Number: D-FT5446U-DataSheet-V0.01-Preliminary
F-OI-RD01-03-03-B

<!-- page: 2 -->
TABLEOFCONTENTS

INTRODUCTION .... I

FEATURES .... I
1 OVERVIEW .... 3

1.1 TYPICAL APPLICATIONS .... 3

2 FUNCTIONAL BLOCK DESCRIPTIONS .... 3

2.1 ARCHITECTURE OVERVIEW .... 3
2.2 MCU .... 4
2.3 OPERATION MODES .... 4
2.4 HOST INTERFACE .... 4
2.5 SERIAL INTERFACE .... 5

3 ELECTRICAL SPECIFICATIONS .... 6

3.1 ABSOLUTE MAXIMUM RATINGS .... 6
3.2 DC CHARACTERISTICS .... 6
3.3 AC CHARACTERISTICS .... 7
3.4 I/O PORTS CIRCUITS .... 8
3.5 POWER ON/RESET SEQUENCE .... 8

4 PIN CONFIGURATIONS .... 9
5 PACKAGE INFORMATION .... 12

5.1 PACKAGE INFORMATION OF QFN-6X6-56L PACKAGE .... 12
5.2 ORDERING INFORMATION .... 14

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 2 of 16

<!-- page: 3 -->
## Visual Summary (Page 3)

- page_class: block_diagram
- confidence: 0.95
- reason_codes: timing_diagram_heuristic, block_diagram_detected, visual_summary_preferred, policy_visual_summary_override

Page displays the system architecture diagram for FT5446U, a touch panel controller. The block diagram illustrates five main functional parts: AFE (Touch Panel Interface Circuits), Enhanced MCU with DSP accelerator, External Interface (I2C, INT, RSTN), Power regulator, and Power On Reset (POR). Text below the diagram describes each component's function.

<!-- page: 4 -->
2.2 MCU

This section describes some critical features and operations supported by the enhanced MCU.

Figure 2-2 shows the overall structure of the MCU block. In addition to the enhanced MCU core, we have
added the following circuits,
- A DSP accelerator cooperates with MCU to process the complex algorithms
- Timer: A number of timers are available to generate different clocks
- Clock Manager: To control various clocks under different operation conditions of the system

Program Data
Memory Memory

Clock Enhanced
Manager MCU Core

Master Watch
Timer
Clock dog

Figure 2-2 MCU Block Diagram
2.3 Operation Modes
FT5446U offers following three modes:

- Active Mode
In active mode,the frame scan rate is 0~120Hz.The host processor can configure it to speed up or to slow
down.

- Monitor Mode
In this mode, most algorithms are stopped. A simpler algorithm is being executed to determine if there is a
touch or not. When a touch is detected, FT5446U shall enter the Active mode immediately. During this mode,
the serial port is closed and no data shall be transferred with the host processor.
- Sleep Mode
In Sleep mode, it shall only respond to the “RESET” , “INT” signal from the host processor.

2.4 Host Interface
Figure 2-3 shows the interface between a host processor and FT5446U. This interface consists of the follow-
ing three sets of signals:
- Serial Interface
- Interrupt from FT5446U to the Host
- Reset Signal from the Host to FT5446U

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 4 of 16

<!-- page: 5 -->
## Visual Summary (Page 5)

- page_class: timing_diagram
- confidence: 0.95
- reason_codes: timing_diagram_heuristic, visual_content_dominant, policy_visual_summary_override

Page contains a Host Interface Diagram (Figure 2-3) showing the FT5446U module connected to a TP and Host via I2C, /INT, and /RST signals. Below is an I2C Serial Data Transfer Format timing diagram illustrating SDA/SCL waveforms with ACK/STOP states. Further down are two block diagrams (Figures 2-5 and 2-6) depicting I2C master write/slave read and master read/slave write sequences respectively, followed by a Mnemonics Description table defining signal codes.

<!-- page: 6 -->
R/ W READ/WRITE bit, ‘1’ for read, ‘0’for write

A(N) ACK(NACK) bit
STOP: the indication of the end of a packet (if this bit is missing, S will indicate
P
the end of the current packet and the beginning of the next packet)

I2C Interface Timing Characteristics is shown in Table 2-2.

Table 2-2 I2C Timing Characteristics

Parameter Min Max Unit

SCL frequency 0 400 KHz

Bus free time between a STOP and START condition 1.3 us

Hold time (repeated) START condition 0.6 us

Data setup time 100 ns

Setup time for a repeated START condition 0.6 us

Setup time for STOP condition 0.6 us

3 ELECTRICAL SPECIFICATIONS

3.1 Absolute Maximum Ratings

Table 3-1 Absolute Maximum Ratings

Item Symbol Value Unit Note

Power Supply Voltage VDD3 – VSS TBD V 1

I/O Digital Voltage IOVCC TBD V 1

Operating Temperature Topr TBD ℃ 1

Storage Temperature Tstg TBD ℃ 1

Notes

1. If used beyond the absolute maximum ratings, FT5446U may be permanently damaged. It is strongly
recommended that the device be used within the electrical characteristics in normal operations. If exposed
to the condition not within the electrical characteristics, it may affect the reliability of the device.
2. Make sure VDD3 (high) ≥VSSLF (low)

3.2 DC Characteristics

Table 3-2 DC Characteristics

Item Symbol Unit Test Condition Min. Typ. Max. Note

Input high-level voltage VIH V 0.7 x IOVCC -- IOVCC

0.3 x
Input low –level voltage VIL V -0.3 --
IOVCC

Output high –level voltage VOH V 0.7 x IOVCC -- --

0.3 x
Output low –level voltage VOL V -- --
IOVCC

I/O leakage current ILI uA -1 -- 1

Current consumption
Iopr mA -- TBD --
（Normal operation mode）

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 6 of 16

<!-- page: 7 -->
Current consumption TBD Imon mA -- --
（Monitor mode）
Current consumption TBD
Islp uA -- --
（Sleep mode）

Step-up output voltage VDD5 V TBD

Step-up output voltage VDD10 V TBD

Output voltage VDD5 V TBD TBD

Output voltage VDD10 V TBD TBD

Power Supply voltage VDD3 V TBD TBD TBD
Notes: This sample data is intended for design guidance only. Values shown are typical for a 16Tx × 28Rx
sensor configured at 80 Hz report rate.Actual current will depend on the particular sensor design and firm-
ware options.

3.3 AC Characteristics
AC Characteristics of Oscillators

Item Symbol Unit Test Condition Min. Typ. Max. Note

OSC clock 1 fosc1 MHz VDD3 = 2.8V; Ta=25℃ TBD TBD TBD

Table 3-3 AC Characteristics of TX & RX

Item Symbol Test Condition Min Typ Max Unit Note

TX acceptable clock ftx 50 150 400 KHz

TX output rise time Ttxr -- 210 -- nS

TX output fall time Ttxf -- 210 -- nS

RX input voltage Trxi 1.2 -- 1.6 V

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 7 of 16

<!-- page: 8 -->
3.4 I/O Ports Circuits

IOVCC IOVCC

2M ohm
VDDA

Input circuit

IOVCC

Floating Sub Output enable
Output data

Figure 3-1 General Purpose In/Out Port Circuit.
The input/output property can be configured via firmware setting. The firmware can also control its output
behavior as push-pull or as open-drain that SDA of I2C interface is required.
IOVCC

50Kohm

300

Figure 3-2 Reset Input Port Circuits

3.5 POWER ON/Reset Sequence

Reset should be pulled down to be low before powering on and powering down. I2C shouldn’t be used by
other devices during Reset time after VDD powering on (Trtp). INT signal will be sent to the host after
initializing all parameters and then start to report points to the host. If Power is down, the voltage of supply
must be below 0.3V and Tpdt is more than 1ms.

Tris

0.9VDD

0.1VDD

Figure 3-3 Power on time

Tpdt

Power

0.3V

Figure 3-4 Power Cycle requirement

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 8 of 16

<!-- page: 9 -->
Trtp Tvdr

Tvdr

VDD3

Reset

INT

I2C

IOVCC Tpio

Figure 3-5 Power on Sequence
Reset time must be enough to guarantee reliable reset, the time of starting to report point after resetting
approach to the time of starting to report point after powering on.

Trsi

Trst

Power

Reset

INT

I2C

Figure 3-6 Reset Sequence

Table 3-5 Power on/Reset Sequence Parameters

Parameter Description Min Max Units

Tris Rise time from 0.1VDD to 0.9VDD -- 5 ms

Tpdt Time of the voltage of supply being below 0.3V 5 -- ms

Trtp Time of resetting to be low before powering on 100 -- μs

Tvdr Reset time after VDD powering on 1 -- ms

Trsi Time of starting to report point after resetting -- 200 ms

Trst Reset time 1 -- ms

Tpio Time of IOVCC to be high before powering on 0 ms

4 PIN CONFIGURATIONS

Pin List of FT5446U

Table 4-1 Pin Definition

Pin No. Type Description
Name
FT5446U
RX28 55 I Receiver input pins
RX27 54 I Receiver input pins
RX26 53 I Receiver input pins
RX25 52 I Receiver input pins
RX24 51 I Receiver input pins
RX23 50 I Receiver input pins
RX22 49 I Receiver input pins
RX21 48 I Receiver input pins
RX20 47 I Receiver input pins
RX19 46 I Receiver input pins
RX18 45 I Receiver input pins

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 9 of 16

<!-- page: 10 -->
RX17 44 I Receiver input pins
RX16 43 I Receiver input pins
RX15 42 I Receiver input pins
RX14 41 I Receiver input pins
RX13 40 I Receiver input pins
RX12 39 I Receiver input pins
RX11 38 I Receiver input pins
RX10 37 I Receiver input pins
RX9 36 I Receiver input pins
RX8 35 I Receiver input pins
RX7 34 I Receiver input pins
RX6 33 I Receiver input pins
RX5 32 I Receiver input pins
RX4 31 I Receiver input pins
RX3 30 I Receiver input pins
RX2 29 I Receiver input pins
RX1 28 I Receiver input pins
internal generated 5V power sup-
VDD5_IN 56 PWR ply, A 1μF ceramic capacitor to
ground is required.
VSSLF 57 PWR Analog ground
digital power supply, A 1μF ceramic
VDD5_Out 1 PWR
capacitor to ground is required.
NC 2 NC
TX10 3 O Transmit output pin
TX11 4 O Transmit output pin
TX12 5 O Transmit output pin
TX13 6 O Transmit output pin
TX14 7 O Transmit output pin
TX15 8 O Transmit output pin
TX16 9 O Transmit output pin
digital power supply, A 1μF ceramic
VDD10 10 PWR
capacitor to ground is required.
digital power supply, A 1μF ceramic
VDD3 11 PWR
capacitor to ground is required.
digital power supply, A 1μF ceramic
VDD15 12 PWR
capacitor to ground is required.
digital power supply, A 1μF ceramic
VDD12 13 PWR
capacitor to ground is required.
IOVCC 14 PWR I/O power supply
Interrupt request to the host, or
INT 15 I/O
Wakeup request from the host.
SDA 16 I/O I2C data input and output
SCL 17 I/O I2C clock input
RSTN 18 I External Reset, Low is active
TX1 19 O Transmit output pin
TX2 20 O Transmit output pin
TX3 21 O Transmit output pin

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 10 of 16

<!-- page: 11 -->
TX4 22 O Transmit output pin
TX5 23 O Transmit output pin
TX6 24 O Transmit output pin
TX7 25 O Transmit output pin
TX8 26 O Transmit output pin
TX9 27 O Transmit output pin

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 11 of 16

<!-- page: 12 -->
VSSLF Vin5 RX28 RX27 RX26 RX25 RX24 RX23 RX22 RX21 RX20 RX19 RX18 RX17 RX16

57 56 55 54 53 52 51 50 49 48 47 46 45 44 43

Vout5 1 42 RX15
NC 2 41 RX14
TX10 3 40 RX13
TX11 4 39 RX12
TX12 5 38 RX11
TX13 6 37 RX10
TX14 7

TX15 8 FT5446U 3635 RX9RX8
TX16 9 34 RX7
VDD10 10 33 RX6
VDD3 11 32 RX5
VDD15 12 31 RX4
30 RX3
VDD12 13
29 RX2
IOVCC 14

15 16 17 18 19 20 21 22 23 24 25 26 27 28

INT SDA SCL RSTN TX1 TX2 TX3 TX4 TX5 TX6 TX7 TX8 TX9 RX1

FT5446U Package Diagram

5 PACKAGE INFORMATION

5.1 Package Information of QFN-6x6-56L Package

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 12 of 16

<!-- page: 13 -->
Millimeter
Item Symbol
Min Type Max
Total Thickness A 0.5 0.55 0.6
Stand Off A1 0 0.035 0.05
Mold Thickness A2 ---- 0.4 ----
L/F Thickness A3 0.152 REF
b 0.13 0.18 0.23

Lead Width b1 0.07 0.12 0.17
X D 6 BSC
Body Size
Y E 6 BSC
Lead Pitch e 0.35 BSC
X J 3.9 4 4.1
EP Size
Y K 3.9 4 4.1
Lead Length L 0.35 0.4 0.45
R 1.45 1.55 1.65
Package Edge Tolerance aaa 0.1
Mold Flatness bbb 0.1
Co Planarity ccc 0.08
Lead Offset ddd 0.1
Exposed Pad Offset eee 0.1

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 13 of 16

<!-- page: 14 -->
5.2 Ordering Information

QFN

Package Type 56Pin(6 * 6 )

56Pin(0.6 – P0.35)

Product Name Package Type # TX Pins # RX Pins

FT5446U QFN-56L 16 28

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 14 of 16

<!-- page: 15 -->
Appendix: IC Revision history of FT5446U Specification

Version Change Items Effective Date

0.01 1st Preliminary 9-Aug-18

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 15 of 16

<!-- page: 16 -->
END OF DATASHEET

THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR
DISCLOSURE IS PROHIBITED

Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01︱Page 16 of 16
