<!-- page: 1 -->
Confidential

Application Note for CTPM

Application Note for CTPM
Project name Touch panel
Document ref [Document ref]
Version 0.4
Release date 18 Jan 2010
Owner Xiaoxu Du
Classification Confidential
Distribution List [Distribution list]
Approval

This document contains information proprietary to FocalTech Systems, Ltd., and may not be reproduced,
disclosed or used in whole or part without the express written permission of FocalTech Systems, Ltd.

Copyright © 2011, FocalTech Systems, Ltd
All rights reserved

R3-B4-A, South Area, Shenzhen Hi-Tech Industrial Park,
Shenzhen, Gungdong, P.R. China
ZIP :518057
T +86 755 26588222
F +86 755 26712499
E support@focaltech-systems.com

www.focaltech-systems.com

Confidential

<!-- page: 2 -->
CTPM Application Note

Revision History
Date Version List of changes Author + Signature
18 Jan, 2010 0.1 Initial draft Xiaoxu Du
17 Mar,2010 0.2 Add raw data protocol Xiaoxu Du
22 Mar,2010 0.3 Add system information protocol Xiaoxu Du
26 Mar,2010 0.4 Add calibration related parameters Xiaoxu Du
08 May,2010 0.5 Add information to operating mode Xinming Wang
07 Jul, 2010 0.6 Change Protocol and add information Yunfeng Yuan

ii
FocalTech Systems, Ltd. Confidential

<!-- page: 3 -->
CTPM Application Note

Table of Contents
1 I2C Interface .... 2
1.1 CTPM interface to Host .... 2
1.2 I2C Read/Write Interface description .... 2
1.3 Interrupt signal from CTPM to Host .... 3
1.4 Wakeup signal from Host to CTPM .... 4
2 CTP Register Mapping .... 4
2.1 Operating Mode .... 4
2.1.1 DEVICE_MODE .... 9
2.1.2 GEST_ID .... 9
2.1.3 TD_STATUS .... 10
2.1.4 TOUCHn_XH (n:1-10) .... 10
2.1.5 TOUCHn_XL (n:1-10) .... 10
2.1.6 TOUCHn_YH (n:1-10) .... 10
2.1.7 TOUCHn_YL (n:1-10) .... 11
2.1.8 TOUCHn_ WEIGHT (n:1-10) .... 11
2.1.9 TOUCHn_ MISC (n:1-10) .... 11
2.1.10 ID_G_THGROUP .... 11
2.1.11 ID_G_THPEAK .... 12
2.1.12 ID_G_ THCAL .... 12
2.1.13 ID_G_ THWATER .... 12
2.1.14 ID_G_ THTEMP .... 12
2.1.15 ID_G_ THDIFF .... 12
2.1.16 ID_G_ CTRL .... 12
2.1.17 ID_G_ TIMEENTERMONITOR .... 12
2.1.18 ID_G_ PERIODACTIVE .... 13
2.1.19 ID_G_ PERIODMONITOR .... 13
2.1.20 ID_G_ HEIGHT_B .... 13
2.1.21 ID_G_ MAX_FRAME .... 13
2.1.22 ID_G_ DIST_MOVE .... 13
2.1.23 ID_G_ DIST_POINT .... 13
2.1.24 ID_G_ FEG_FRAME .... 14
2.1.25 ID_G_ SINGLE_CLICK_OFFSET .... 14
2.1.26 ID_G_ DOUBLE_CLICK_TIME_MIN .... 14
2.1.27 ID_G_ SINGLE_CLICK_TIME .... 14
2.1.28 ID_G_ LEFT_RIGHT_OFFSET .... 14
2.1.29 ID_G_ UP_DOWN_OFFSET .... 14
2.1.30 ID_G_ DISTANCE_LEFT_RIGHT .... 15
2.1.31 ID_G_ DISTANCE_UP_DOWN .... 15
2.1.32 ID_G_ ZOOM_DIS_SQR .... 15
2.1.33 ID_G_ RADIAN_VALUE .... 15
2.1.34 ID_G_ MAX_X_HIGH .... 15
2.1.35 ID_G_ MAX_X_LOW .... 16
2.1.36 ID_G_ MAX_Y_HIGH .... 16
iii
FocalTech Systems, Ltd. Confidential

<!-- page: 4 -->
CTPM Application Note

2.1.37 ID_ G_MAX_Y_LOW .... 16
2.1.38 ID_G_ K_X_HIGH .... 16
2.1.39 ID_G_K_X_LOW .... 16
2.1.40 ID_G_ K_Y_HIGH .... 16
2.1.41 ID_G_ K_Y_LOW .... 16
2.1.42 ID_G_ AUTO_CLB_MODE .... 17
2.1.43 ID_G_ LIB_VERSION_H .... 17
2.1.44 ID_G_ LIB_VERSION_L .... 17
2.1.45 ID_G_ CIPHER .... 17
2.1.46 ID_G_ MODE .... 17
2.1.47 ID_G_ PMODE .... 17
2.1.48 ID_G_ FIRMWARE_ID .... 17
2.1.49 ID_G_ STATE .... 18
2.1.50 ID_G_ FT5201ID .... 18
2.1.51 ID_G_ ERR .... 18
2.1.52 ID_G_ CLB .... 18
2.2 Test Mode .... 18
2.2.1 DEVICE_MODE .... 20
2.2.2 ROW_ADDR .... 20
2.2.3 ROWDATAN_H .... 21
2.2.4 ROWDATAN_L .... 21
2.3 System information Mode .... 22
2.3.1 DEVICE_MODE .... 23
2.3.2 BIST_COMM .... 23
2.3.3 BIST_STAT .... 23
2.3.4 BL_VERH .... 23
2.3.5 BL_VERL .... 24
2.3.6 FTS_IC_VERH .... 24
2.3.7 FTS_IC_VERL .... 24
2.3.8 APP_IDH .... 24
2.3.9 APP_IDL .... 24
2.3.10 APP_VERH .... 25
2.3.11 APP_VERL .... 25
2.3.12 CID_n(n:0-4) .... 25
3 CTPM Application Introduction .... 26
3.1 Standard Application information of FT5X06 .... 26
3.1.1 Standard application circuit of FT5206GE1 .... 26
3.1.2 Standard application circuit of FT5306DE4 .... 27
3.1.3 Standard application circuit of FT5206EE8 .... 28
4 Communication between host and CTPM .... 28
4.1 Communication Contents .... 28
4.2 I2C Example Code .... 29

iv
FocalTech Systems, Ltd. Confidential

<!-- page: 5 -->
CTPM Application Note

Terminology
CTP – Capacitive touch panel
CTPM – Capacitive touch panel module

1
FocalTech Systems, Ltd. Confidential

<!-- page: 6 -->
CTPM Application Note

1 I2C Interface
1.1 CTPM interface to Host
Figure 1-1 shows how CTPM communicates with the Host，there are three kind of communication between
CTPM and Host，we will introduce each communication in this section.
Transfer the data via I2C
Send interrupt when there is a valid touch
Host send Wakeup signal to CTPM

Serial
Interface TX
CTP CTP MCU Host Controller /INT
RX
/WAKE

Figure 1-1 CTPM and Host connection
The Power Supply voltage of CTPM is 2.8V~3.3V, interface supply voltage is 2.8V~3.3V. There are Control
Interface and Data Interface. As Figure 1-1 demonstrates, Serial interface is the data interface, /INT and
/WAKE are the control interface. For the detail, please refer to Table 1-1.
Table 1-1 Description for TP module and Host interface
Port Name Voltage Polar Description
Serial 2.8~3.3V Serial interface is for data transfer between Host and CTPM.
interface CTPM support both I2C and SPI interface
/INT 2.8~3.3V LOW The interrupt from the CTPM to the Host
/WAKE* 2.8~3.3V LOW Wakeup signal from host to the CTPM

1.2 I2C Read/Write Interface description
Write N bytes to I2C slave
Slave Addr Data Address[X] Data [X] Data [X+N-1]
A A A A A A A R R R R R R R R R D D D D D D D D D D D D D D D D S A A A … A P 6 5 4 3 2 1 0 W 7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0
ACK ACK ACK ACKSTOPSTART WRITE

Set Data Address
Slave Addr Data Address[X]
A A A A A A A R R R R R R R R R S A A P 6 5 4 3 2 1 0 W 7 6 5 4 3 2 1 0
ACK ACKSTOPSTART WRITE

2
FocalTech Systems, Ltd. Confidential

<!-- page: 7 -->
CTPM Application Note

Read X bytes from I2C Slave
Slave Addr Data [N] Data [X+N-1]
A A A A A A A R D D D D D D D D D D D D D D D D S A A … A P 6 5 4 3 2 1 0 W 7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0
ReadACK ACK ACKSTOPSTART

Note: In the I2C Read/Write mode, the Data Address should not be more than 0xFF, it will stays at 0xFF when
reaching 0xFF.
1.3 Interrupt signal from CTPM to Host
As for standard CTPM, host need to use both interrupt control signal and serial data interface to get the touch
data. There are two kind of method to use interrupt: interrupt trigger and interrupt query.
Note: In Interrupt query mode, To avoid missing touch data, the sampling rate should be bigger than the
report rate.
Here is the timing to get touch data.
Touch Start Touch End

/INT

Serial
Data
Data Packet Data Packet Data Packet Blank …… Blank 0 1 N

Figure 1-2 Interrupt query mode

Touch Start Touch End

/INT

Serial
Data
Data Packet Data Packet Data Packet Blank …… Blank 0 1 N

Figure 1-3 Interrupt trigger mode

Host use general I2C protocol to read the touch data or the information from CTPM . CTPM will send host a
interrupt signal when there is a valid touch. Then host can use the serial data interface to get the touch data. If
there is no valid touch detected, the /INT will not be pulled up, the host do not need to read the touch data.
NOTE: “valid touch” may have different definition in various systems. For example, in some systems, the valid
touch is defined as there is one more valid touch point. But in some other systems, the valid touch is defined as
one more valid touch with valid gestures. In usual, /INT will be pulled up when there is a valid touch point, and
to be low when a touch finishes.
As for interrupt trigger mode, /INT signal will be low if there is no touch detected. But for per update of valid
touch data, CTPM will produce a valid pulse for /INT signal, host can read the touch data periodically
according to the frequency of this pulse. In this mode, the pulse frequency is the touch data update frequency.
.

3
FocalTech Systems, Ltd. Confidential

<!-- page: 8 -->
CTPM Application Note

1.4 Wakeup signal from Host to CTPM
Host can use the Wakeup Signal to wakeup the I2C slave device.
This pin should be pulled down to GND when flash programming while in normal running mode it should not
be.

2 CTP Register Mapping
This chapter describes the standard FTS Capacitive Touch Panel products communication registers in address
order for each device mode. The most detailed descriptions of the Standard Products communication registers
are in the Register Definitions section of each chapter. The device modes are listed in the table below, along
with each mode’s register prefix.
Device Mode Prefix Val Description
Operating Op 000b Read touch point and gesture
Test Te 100b Read raw data
System Information Sy 001b Read system information related
Reserved

2.1 Operating Mode
In this mode the CTP is fully functional as a touch screen controller. Read and write access address is just
logical address which is not enforced by hardware or firmware. Here is the operating mode register map.
Operating Mode Register Map
Address Name Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0 Host
Access
Op,00h DEVIDE_MODE Device Mode[2:0] RW
Op,01h GEST_ID Gesture ID[7:0] R
Op,02h TD_STATUS Frame remaining Number of R
touch points[3:0]
Op,03h TOUCH1_XH 1stEvent 1st Touch R
Flag X Position[11:8]
Op,04h TOUCH1_XL 1st Touch X Position[7:0] R
Op,05h TOUCH1_YH 1st Touch ID[3:0] 1st Touch R
Y Position[11:8]
Op,06h TOUCH1_YL 1st Touch Y Position[7:0] R
Op,07h TOUCH1_WEIGHT 1st Touch Weight[7:0] R
Op,08h TOUCH1_MISC 1st Touch Area[3:0] 1stTouch 1stTouch R
Direction Speed
[1:0] [1:0]

4
FocalTech Systems, Ltd. Confidential

<!-- page: 9 -->
CTPM Application Note

Op,09h TOUCH2_XH 2ndEvent 2ndTouch R
Flag X Position[11:8]
Op,0Ah TOUCH2_XL 2nd touch X Position[7:0] R
Op,0Bh TOUCH2_YH 2nd Touch ID[3:0] 2nd Touch R
Y Position[11:8]
Op,0Ch TOUCH2_YL 2nd Touch Y Position[7:0] R
Op,0Dh TOUCH2_WEIGHT 2nd Touch Weight[7:0] R
Op,0Eh TOUCH2_MISC 2nd Touch Area[3:0] 2ndTouch 2ndTouch R
Direction Speed
[1:0] [1:0]
Op,0Fh TOUCH3_XH 3rdEvent 3rd Touch R
Flag X Position[11:8]
Op,10h TOUCH3_XL 3rd Touch X Position[7:0] R
Op,11h TOUCH3_YH 3rd Touch ID[3:0] 3rd Touch R
Y Position[11:8]
Op,12h TOUCH3_YL 3rd Touch Y Position[7:0] R
Op,13h TOUCH3_WEIGHT 3rd Touch Weight[7:0] R
Op,14h TOUCH3_MISC 3rd Touch Area[3:0] 3rdTouch 3rdTouch R
Direction Speed
[1:0] [1:0]
Op,15h TOUCH4_XH 4thEvent 4th Touch R
Flag X Position[11:8]
Op,16h TOUCH4_XL 4th Touch X Position[7:0] R
Op,17h TOUCH4_YH 4th Touch ID[3:0] 4th Touch R
Y Position[11:8]
Op,18h TOUCH4_YL 4th Touch Y Position[7:0] R
Op,19h TOUCH4_WEIGHT 4th Touch Weight[7:0] R
Op,1Ah TOUCH4_MISC 4th Touch Area[3:0] 4thTouch 4thTouch R
Direction Speed
[1:0] [1:0]
Op,1Bh TOUCH5_XH 5thEvent 5th Touch R
Flag X Position[11:8]
Op,1Ch TOUCH5_XL 5thTouch X Position[7:0] R
Op,1Dh TOUCH5_YH 5th Touch ID[3:0] 5th Touch R
Y Position[11:8]
Op,1Eh TOUCH5_YL 5th Touch Y Position[7:0] R
Op,1Fh TOUCH5_WEIGHT 5th Touch Weight[7:0] R
Op,20h TOUCH5_MISC 5th Touch Area[3:0] 5thTouch 5thTouch R

5
FocalTech Systems, Ltd. Confidential

<!-- page: 10 -->
CTPM Application Note

Direction Speed
[1:0] [1:0]
Op,21h TOUCH6_XH 6thEvent 6th Touch R
Flag X Position[11:8]
Op,22h TOUCH6_XL 6th Touch X Position[7:0] R
Op,23h TOUCH6_YH 6th Touch ID[3:0] 6th Touch R
Y Position[11:8]
Op,24h TOUCH6_YL 6th Touch Y Position[7:0] R
Op,25h TOUCH6_WEIGHT 6th Touch Weight[7:0] R
Op,26h TOUCH6_MISC 6th Touch Area[3:0] 6thTouch 6thTouch R
Direction Speed
[1:0] [1:0]
Op,27h TOUCH7_XH 7thEvent 7th Touch R
Flag X Position[11:8]
Op,28h TOUCH7_XL 7th Touch X Position[7:0] R
Op,29h TOUCH7_YH 7th Touch ID[3:0] 7th Touch R
Y Position[11:8]
Op,2Ah TOUCH7_YL 7th Touch Y Position[7:0] R
Op,2Bh TOUCH7_WEIGHT 7th Touch Weight[7:0] R
Op,2Ch TOUCH7_MISC 7th Touch Area[3:0] 7thTouch 7thTouch R
Direction Speed
[1:0] [1:0]
Op,2Dh TOUCH8_XH 8thEvent 8th Touch R
Flag X Position[11:8]
Op,2Eh TOUCH8_XL 8th Touch X Position[7:0] R
Op,2Fh TOUCH8_YH 8th Touch ID[3:0] 8th Touch R
Y Position[11:8]
Op,30h TOUCH8_YL 8th Touch Y Position[7:0] R
Op,31h TOUCH8_WEIGHT 8th Touch Weight[7:0] R
Op,32h TOUCH8_MISC 8th Touch Area[3:0] 8thTouch 8thTouch R
Direction Speed
[1:0] [1:0]
Op,33h TOUCH9_XH 9thEvent 9th Touch R
Flag X Position[11:8]
Op,34h TOUCH9_XL 9th Touch X Position[7:0] R
Op,35h TOUCH9_YH 9th Touch ID[3:0] 9th Touch R
Y Position[11:8]
Op,36h TOUCH9_YL 9th Touch Y Position[7:0] R

6
FocalTech Systems, Ltd. Confidential

<!-- page: 11 -->
CTPM Application Note

Op,37h TOUCH9_WEIGHT 9th Touch Weight[7:0] R
Op,38h TOUCH9_MISC 9th Touch Area[3:0] 9thTouch 9thTouch R
Direction Speed
[1:0] [1:0]
Op,39h TOUCH10_XH 10thEvent 10th Touch R
Flag X Position[11:8]
Op,3Ah TOUCH10_XL 10th Touch X Position[7:0] R
Op,3Bh TOUCH10_YH 10th Touch ID[3:0] 10th Touch R
Y Position[11:8]
Op,3Ch TOUCH10_YL 10th Touch Y Position[7:0] R
Op,3Dh TOUCH10_WEIGHT 10th Touch Weight[7:0] R
Op,3Eh TOUCH10_MISC 10th Touch Area[3:0] 10thTouch 10thTouch R
Direction Speed
[1:0] [1:0]
Op,3Fh Reserved
… …
Op,7Fh Reserved
Op,80h ID_G_THGROUP valid touching detect threshold. R/W
Op,81h ID_G_THPEAK valid touching peak detect threshold. R/W
Op,82h ID_G_THCAL the threshold when calculating the focus of touching. R/W
Op,83h ID_G_THWATER the threshold when there is surface water. R/W
Op,84h ID_G_THTEMP the threshold of temperature compensation. R/W
Op,85h ID_G_THDIFF the threshold whether the coordinate is different from R/W
the original
Op,86h ID_G_CTRL Power R/W
control
mode[1:0]
Op,87h ID_G_TIME_ENTER The timer of entering monitor status R/W
_MONITOR
Op,88h ID_G_PERIODACTIVE Period Active[3:0] R/W
Op,89h ID_G_PERIOD The timer of entering idle while in monitor status R/W
MONITOR
Op,8Ah ID_G_HEIGHT_B The height of valid touching gesture region R/W
Op,8Bh ID_G_MAX_FRAME The timer of the valid single click gesture R/W
Op,8Ch ID_G_DIST_MOVE Minimum of the valid move left, move right, move up, R/W
move down gesture.
Op,8Dh ID_G_DIST_POINT R/W
Op,8Eh ID_G_FEG_FRAME The timer of the all valid gesture R/W

7
FocalTech Systems, Ltd. Confidential

<!-- page: 12 -->
CTPM Application Note

Op,8Fh ID_G_SGL_CLK Minimum of the single click gesture R/W
_OFFSET
Op,90h ID_G_DBL_CLK R/W
_TIME_MIN
Op,91h ID_G_SGL_CLK R/W
_TIME
Op,92h ID_G_L_R_OFFSET Maximum of the distance of X axis of the valid move R/W
up, move down gesture.
Op,93h ID_G_U_D_OFFSET Maximum of the distance of Y axis of the valid move R/W
left, move right gesture.
Op,94h ID_G_DISTANCE Minimum of the distance of X axis of the valid move R/W
_LEFT_RIGHT left, move right gesture.
Op,95h ID_G_DISTANCE Minimum of the distance of Y axis of the valid move R/W
_UP_DOWN up, move down gesture.
Op,96h ID_G_ZOOM The threshold of valid Zoom In, Zoom Out gesture R/W
_DIS_SQR
Op,97h ID_G_RADIAN Minimum of angle of Double Right/Left Rotation R/W
_VALUE gesture
Op,98h ID_G_MAX_X_HIGH maximum resolution of X axis high byte R/W
Op,99h ID_G_MAX_X_LOW maximum resolution of X axis low byte R/W
Op,9Ah ID_G_MAX_Y_HIGH minimum resolution of Y axis high byte R/W
Op,9Bh ID_G_MAX_Y_LOW minimum resolution of Y axis low byte R/W
Op,9Ch ID_G_K_X_HIGH the resolution coefficient of X axis high byte R/W
Op,9Dh ID_G_K_X_LOW the resolution coefficient of X axis low byte R/W
Op,9Eh ID_G_K_Y_HIGH the resolution coefficient of Y axis high byte R/W
Op,9Fh ID_G_K_Y_LOW the resolution coefficient of Y axis low byte R/W
Op,A0h ID_G_AUTO_CLB auto calibration mode R/W
_MODE
Op,A1h ID_G_LIB_ Firmware Library Version H byte R
VERSION_H
Op,A2h ID_G_LIB Firmware Library Version L byte R
_VERSION_L
Op,A3h ID_G_CHIP Chip vendor ID R
Op,A4h ID_G_MODE the interrupt mode to host R
Op,A5h ID_G_PMODE Power Consume Mode
Op,A6h ID_G_FIRMID Firmware ID R
Op,A7h ID_G_STATE Running State
Op,A8h ID_G_FT5201ID CTPM Vendor ID R
8
FocalTech Systems, Ltd. Confidential

<!-- page: 13 -->
## Visual Summary (Page 13)

- page_class: register_table
- confidence: 0.98
- reason_codes: register_table_detected, structured_data_present, high_readability

Page contains three register tables from a CTPM Application Note: ID_G registers (Op.A9h-FFh) defining error codes, calibration modes, and touch thresholds; DEVICE_MODE register (Op.00h) specifying operating states; and GEST_ID register (Op.01h) listing gesture identifiers for single/multi-touch interactions.

<!-- page: 14 -->
CTPM Application Note

0x00 No Gesture

2.1.3 TD_STATUS
This register is the Touch Data status register.
Address Bit Address Register Name Description
Op,02h 3:0 Number of touch How many points detected.
points[3:0] 1-5 is valid.
7:4 Frame remaining Frame remaining after host’s reading
[7:4] Range from 0 to 9

2.1.4 TOUCHn_XH (n:1-10)
This register describes MSB of the X coordinate of the nth touch point and the corresponding event flag.
Address Bit Address Register Name Description
Op,03h 7:6 Event Flag 00b: Put Down
~ 01b: Put Up
Op,39h 10b: Contact
11b: No event
5:4 Reserved
3:0 Touch X Position MSB of Touch X Position in pixels
[11:8]

2.1.5 TOUCHn_XL (n:1-10)
This register describes LSB of the X coordinate of the nth touch point.
Address Bit Address Register Name Description
Op,04h 7:0 Touch X Position LSB of the Touch X Position in pixels
~ [7:0]
Op,3Ah

2.1.6 TOUCHn_YH (n:1-10)
This register describes MSB of the Y coordinate of the nth touch point and corresponding touch ID.
Address Bit Address Register Name Description
Op,05h 7:4 Touch ID[3:0] Touch ID of Touch Point

10
FocalTech Systems, Ltd. Confidential

<!-- page: 15 -->
CTPM Application Note

~ 3:0 Touch X Position MSB of Touch Y Position in pixels
Op,3Bh [11:8]

2.1.7 TOUCHn_YL (n:1-10)
This register describes LSB of the Y coordinate of the nth touch point.
Address Bit Address Register Name Description
Op,06h 7:0 Touch X Position LSB of The Touch Y Position in pixels
~ [7:0]
Op,3Ch

2.1.8 TOUCHn_ WEIGHT (n:1-10)
This register describes weight of the nth touch point.
Address Bit Address Register Name Description
Op,07h 7:4 Touch Area[7:4] The valid touching area
~ 3:2 Touch Direction 0: up
Op,3Dh 1:down
2:left
3:right
1:0 Touch Speed 0: static
1: normal speed
2: high speed

2.1.9 TOUCHn_ MISC (n:1-10)
This register describes the miscellaneous information of the nth touch point.
Address Bit Address Register Name Description
Op,08h 7:0 Touch Weight[7:0] Valid points in X direction×Valid points
~ in Y direction/2
Op,3Eh

2.1.10 ID_G_THGROUP
This register describes valid touching detect threshold.
Address Bit Address Register Name Description
Op,80h 7:0 ID_G_THGROUP The actual value will be 4 times of the register’s
value. Default:280/4

11
FocalTech Systems, Ltd. Confidential

<!-- page: 16 -->
CTPM Application Note

2.1.11 ID_G_THPEAK
This register describes valid touching peak detect threshold.
Address Bit Address Register Name Description
Op,81h 7:0 ID_G_ THPEAK Default:60

2.1.12 ID_G_ THCAL
This register describes threshold when calculating the focus of touching.
Address Bit Address Register Name Description
Op,82h 7:0 ID_G_ THCAL Default:16

2.1.13 ID_G_ THWATER
This register describes threshold when there is surface water.
Address Bit Address Register Name Description
Op,83h 7:0 ID_G_ THWATER Default:60

2.1.14 ID_G_ THTEMP
This register describes threshold of temperature compensation.
Address Bit Address Register Name Description
Op,84h 7:0 ID_G_ THTEMP Default:10

2.1.15 ID_G_ THDIFF
This register describes threshold whether the coordinate is different from the original.
Address Bit Address Register Name Description
Op,85h 7:0 ID_G_ THDIFF The actual value must be 32timers of
the register’s value. Default :20

2.1.16 ID_G_ CTRL
This register describes the run mode of microcontroller controlled by host
Address Bit Address Register Name Description
Op,86h 0 ID_G_ CTRL 0: not auto jump 1:auto jump

2.1.17 ID_G_ TIMEENTERMONITOR
This register describes the time delay value when entering monitor status.
Address Bit Address Register Name Description
Op,87h 7:0 ID_G_TIME Default :2
ENTERMONITOR

12
FocalTech Systems, Ltd. Confidential

<!-- page: 17 -->
CTPM Application Note

2.1.18 ID_G_ PERIODACTIVE
This register describes the period of active status, it should not less than 12
Address Bit Address Register Name Description
Op,88h 4:0 ID_G_ PERIOD Range form 3 to 14,default 12
ACTIVE
7:4

2.1.19 ID_G_ PERIODMONITOR
This register describes period of monitor status, it should not less than 30.
Address Bit Address Register Name Description
Op,89h 7:0 ID_G_ PERIOD Default:40
MONITOR

2.1.20 ID_G_ HEIGHT_B
This is the height of gesture B area register, it is now obsolete.
Address Bit Address Register Name Description
Op,8Ah 7:0 ID_G_ HEIGHT Default:125
_B

2.1.21 ID_G_ MAX_FRAME
This register is only used in the mode of reporting gesture to host once after lifting up.
Address Bit Address Register Name Description
Op,8Bh 7:0 ID_G_ MAX The maximum of timer to produce
_FRAME Single Click gesture. Default:120

2.1.22 ID_G_ DIST_MOVE
This register is only used in the mode of reporting gesture to host once after lifting up.
Address Bit Address Register Name Description
Op,8Ch 7:0 ID_G_ DIST The minimum distance to produce pan up, pan
_MOVE down, pan right, pan left gesture.Default: 60

2.1.23 ID_G_ DIST_POINT
This register describes maximum distance to produce point gesture.
Address Bit Address Register Name Description
Op,8Dh 7:0 ID_G_ DIST reserved
_POINT

13
FocalTech Systems, Ltd. Confidential

<!-- page: 18 -->
CTPM Application Note

2.1.24 ID_G_ FEG_FRAME
This register is only used in the mode of reporting gesture to host once after lifting up.
Address Bit Address Register Name Description
Op,8Eh 7:0 ID_G_ The maximum timer to produce Left, Right
FEG_FRAME Rotation gesture. Default:120

2.1.25 ID_G_ SINGLE_CLICK_OFFSET
This register is only used in the mode of reporting gesture to host once after lifting up.
Address Bit Address Register Name Description
Op,8FH 7:0 ID_G_SINGLE The maximum distance to produce Single
_CLICK_OFFSET Click. Default: 50

2.1.26 ID_G_ DOUBLE_CLICK_TIME_MIN
This register is predefined, and now no use
Address Bit Address Register Name Description
Op,90H 7:0 ID_G_ DOUBLE
_CLICK_TIME_MIN

2.1.27 ID_G_ SINGLE_CLICK_TIME
This register is predefined, and now no use
Address Bit Address Register Name Description
Op,91h 7:0 ID_G_ SINGLE
_CLICK_TIME

2.1.28 ID_G_ LEFT_RIGHT_OFFSET
This register is only used in the mode of continuous reporting gesture to host while valid gesture
produced.
Address Bit Address Register Name Description
Op,92h 7:0 ID_G_ LEFT_RIGHT The maximum distance on X axis to produce
_OFFSET Up, Down gesture. Default: 20

2.1.29 ID_G_ UP_DOWN_OFFSET
This register is only used in the mode of continuous reporting gesture to host while valid gesture
produced.
Address Bit Address Register Name Description

14
FocalTech Systems, Ltd. Confidential

<!-- page: 19 -->
CTPM Application Note

Op,93h 7:0 ID_G_ UP_DOWN The maximum distance on Y axis to produce
_OFFSET Left, Right gesture. Default: 20

2.1.30 ID_G_ DISTANCE_LEFT_RIGHT
This register is only used in the mode of continuous reporting gesture to host while valid gesture
produced.
Address Bit Address Register Name Description
Op,94h 7:0 ID_G_DISTANCE The minimum distance on X axis to produce
_LEFT_RIGHT Left, Right gesture. Default: 50

2.1.31 ID_G_ DISTANCE_UP_DOWN
This register is only used in the mode of continuous reporting gesture to host while valid gesture
produced.
Address Bit Address Register Name Description
Op,95h 7:0 ID_G_DISTANCE The minimum distance on Yaxis to produce
_UP_DOWN Up, Down gesture. Default: 50

2.1.32 ID_G_ ZOOM_DIS_SQR
This register describes minimum square of distance while zoom in or out used in both reporting mode..
Address Bit Address Register Name Description
Op,96h 7:0 ID_G_ ZOOM The minimum distance to produce Zoom In or
_DIS_SQR Out used in both reporting mode.

2.1.33 ID_G_ RADIAN_VALUE
This register is only used in the mode of continuous reporting gesture to host while valid gesture
produced.
Address Bit Address Register Name Description
Op,97h 7:0 ID_G_ RADIAN The minimum angle to produce Double Left
_VALUE or Right Rotation or

2.1.34 ID_G_ MAX_X_HIGH
This register describes the resolution of X axis high byte.
Address Bit Address Register Name Description

15
FocalTech Systems, Ltd. Confidential

<!-- page: 20 -->
CTPM Application Note

Op, 98h 7:0 ID_G_ MAX_X_HIGH MSB of the resolution of X axis.

2.1.35 ID_G_ MAX_X_LOW
This register describes the resolution of X axis low byte.
Address Bit Address Register Name Description
Op, 99h 7:0 ID_G_ MAX_X_LOW LSB of the resolution of X axis.

2.1.36 ID_G_ MAX_Y_HIGH
This register describes the resolution of Y axis high byte.
Address Bit Address Register Name Description
Op,9Ah 7:0 ID_G_ MAX_Y_HIGH MSB of the resolution of Y axis.

2.1.37 ID_ G_MAX_Y_LOW
This register describes the resolution of Y axis low byte.
Address Bit Address Register Name Description
Op, 9Bh 7:0 ID_G_ MAX_Y_LOW LSB of the resolution of Y axis.

2.1.38 ID_G_ K_X_HIGH
This register describes the resolution coefficient of X axis high byte.
Address Bit Address Register Name Description
Op, 9Ch 7:0 ID_G_ K_X_HIGH MSB of the resolution coefficient of X axis

2.1.39 ID_G_K_X_LOW
This register describes the resolution coefficient of X axis low byte..
Address Bit Address Register Name Description
Op, 9Dh 7:0 ID_G_ K_X_LOW LSB of the resolution coefficient of X axis

2.1.40 ID_G_ K_Y_HIGH
This register describes the resolution coefficient of Y axis high byte.
Address Bit Address Register Name Description
Op, 9Eh 7:0 ID_G_ K_Y_HIGH MSB of the resolution coefficient of Y axis

2.1.41 ID_G_ K_Y_LOW
This register describes the resolution coefficient of Y axis low byte.
Address Bit Address Register Name Description
Op, 9Fh 7:0 ID_G_K_Y_LOW LSB of the resolution coefficient of Y axis

16
FocalTech Systems, Ltd. Confidential

<!-- page: 21 -->
CTPM Application Note

2.1.42 ID_G_ AUTO_CLB_MODE
This register describes auto calibration mode.
Address Bit Address Register Name Description
Op, A0h 7:0 ID_G_ AUTO_ 8’h 00: enable auto calibration
CLB_MODE 8’h ff: disable auto calibration

2.1.43 ID_G_ LIB_VERSION_H
This register describes library version high byte.
Address Bit Address Register Name Description
Op, A1h 7:0 ID_G_LIB_VERSION_H R: xx

2.1.44 ID_G_ LIB_VERSION_L
This register describes library version low byte.
Address Bit Address Register Name Description
Op, A2h 7:0 ID_G_ LIB_VERSION_L R: xx

2.1.45 ID_G_ CIPHER
This register describes vendor’s chip id.
Address Bit Address Register Name Description
OP, A3h 7:0 ID_G_ CIPHER R: xx

2.1.46 ID_G_ MODE
This register describes whether the host is worked in polling mode or whether it is worked in trigger mode,
see details in Section 1.3 Interrupt signals from CTPM to Host.
Address Bit Address Register Name Description
Op,A4h 7:0 ID_G_ MODE 0: host in polling mode
1: host in interrupt trigger mode

2.1.47 ID_G_ PMODE
This register describes the power consumption mode of the TPM when in running status.
Address Bit Address Register Name Description
Op,A5h 7:0 ID_G_ PMODE 0: active
1: monitor
3: hibernate(deep sleep)

2.1.48 ID_G_ FIRMWARE_ID
This register describes the firmware id of the application.
Address Bit Address Register Name Description

17
FocalTech Systems, Ltd. Confidential

<!-- page: 22 -->
## Visual Summary (Page 22)

- page_class: register_table
- confidence: 0.98
- reason_codes: dense_register_tables, structured_data_visuals

Page contains a series of register definition tables for CTPM (Touch Panel) firmware configuration and error reporting. Registers include ID_G_STATE (run mode config), ID_G_FT5201ID (vendor chip ID), ID_G_ERR (error codes like 'chip writing inconsistent' or 'start fail'), and ID_G_CLB (calibration mapping). A section on Test Mode indicates raw touch data and panel configuration are provided.

<!-- page: 23 -->
CTPM Application Note

Address Name Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0 Host
Access
Te,00h DEVIDE_MODE Data Device Mode[2:0] RW
Read
Toggle
Te,01h ROW_ADDR The address of the row to be read RW
Te,02h START_SCAN Start the scan command, the value stands for the scan frequency, RW
will be set to zero when scan finishes
Te,03h ROW_NUM Panel row number RW
Te,04h COL_NUM Panel column number RW
Te,05h DRIVER_VOL Driver voltage of chip RW
Te,06h START_RX Setting the RX start number RW
Te,07h GAIN Control the difference value for touching RW
Te,08h ORIGIN_XH High byte of origin X coordinate RW
Te,09h ORIGIN_XL Low byte of origin X coordinate RW
Te,0Ah ORIGIN_YH High byte of origin Y coordinate RW
Te,0Bh ORIGIN_YL Low byte of origin Y coordinate RW
Te,0Ch RES_WH High byte of width of resolution RW
Te,0Dh RES_WL Low byte of width of resolution RW
Te,0Eh RES_HH High byte of height of resolution RW
Te,0Fh RES_HL Low byte of height of resolution RW
Te,10h RAWDATA0_H High byte of raw data 0 R
Te,11h RAWDATA0_L Low byte of raw data 0 R
Te,12h RAWDATA1_H High byte of raw data 1 R
Te,13h RAWDATA1_L Low byte of raw data 1 R
… … …
Te,4Ah RAWDATA29_H High byte of raw data 29 R
Te,4Bh RAWDATA29_L Low byte of raw data 29 R
Te,4Ch TH_POINT_NUM Touch point number support RW
Te,4Dh Reserved
Te,4Eh Reserved
Te,4Fh Reserved
Te,50h TX_ORDER_0 TX Order, start from zero RW
Te,51h TX_ORDER_1 RW
… … … RW
Te,77h TX_ORDER_39 RW

19
FocalTech Systems, Ltd. Confidential

<!-- page: 24 -->
CTPM Application Note

Te,78h ROW0_CAC Charge Amplifier feedback Capacitance of ROW0 RW
Te,79h ROW1_CAC Charge Amplifier feedback Capacitance of ROW1 RW
… … …
Te,9Fh ROW39_CAC Charge Amplifier feedback Capacitance of ROW39 RW
Te,A0h COL0_CAC Charge Amplifier feedback Capacitance of COL0 RW
… … …
Te,BEh COL29_CAC Charge Amplifier feedback Capacitance of COL29 RW
Te,BFh ROW0_1_OFFSET Offset of ROW1 Offset of ROW0 RW
… … … …
Te,D2h ROW38_39_OFFSET Offset of ROW39 Offset of ROW38 RW
Te,D3h COL0_1_OFFSET Offset of COL1 Offset of COL0 RW
… … … …
Te,E1h COL28_29_OFFSET Offset of COL29 Offset of COL28 RW
… …
Te,FEh LOG_MSG_CNT The log MSG count R
Te,FFh LOG_CUR_CHA Current character of log message, will point to the next character R
when one character is read.

2.2.1 DEVICE_MODE
This register is the device mode register, configure it to determine the current mode of the chip.
Address Bit Address Register Name Description
Te,00h 7 Data Read Toggle This bit is toggled by the Host only when a data
transfer between the Host and TrueTouch
device
requires register based handshaking.
6:4 Device Mode[2:0] 000b Normal operating Mode
001b System Information Mode (Reserved)
100b Test Mode – read raw data (Reserved)

2.2.2 ROW_ADDR
This register is the Touch Data status register.
Address Bit Address Register Name Description
Te,01h 7:0 Row address The address of the row to be read
Please delay for more than 100us, then read the
raw data

20
FocalTech Systems, Ltd. Confidential

<!-- page: 25 -->
CTPM Application Note

2.2.3 ROWDATAN_H
This register is the Touch Data status register.
Address Bit Address Register Name Description
Te,(10+2n)h 7:0 High byte of raw data N High byte of raw data N
If N exceeds the column number will return
0xff

2.2.4 ROWDATAN_L
This register is the Touch Data status register.
Address Bit Address Register Name Description
Te,(10+2n+1)h 7:0 Low byte of raw data N Low byte of raw data N
If N exceeds the column number will return
0xff

21
FocalTech Systems, Ltd. Confidential

<!-- page: 26 -->
CTPM Application Note

2.3 System information Mode
This mode provides access to all of the one-time system information. The system information is either written
by the host to permanently configure the device (for example, power timers), or is written to the device at
compile time for the host to read (for example, application version). To enter BIST (built in self test) mode write
the BIST command required into the BIST_COMM register.
Read and write access is theoretical and is not enforce by hardware or firmware. Words have their MSB at lower
address.

System Information Mode Register Map
Address Name Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0 Host
Access
Sy,00h DEVIDE_MODE Data Device Mode[2:0] RW
Read
Toggle
Sy,01h BIST_COMM BIST Command[7:0] W
Sy,02h BIST_STAT BIST Status[7:0] R
Sy,03h Unused
Sy,04h Unused
Sy,05h Unused
Sy,06h Unused
Sy,07h UID_0 Unique Silicon ID #0[7:0] R
Sy,08h UID_1 Unique Silicon ID #1[7:0] R
Sy,09h UID_2 Unique Silicon ID #2[7:0] R
Sy,0Ah UID_3 Unique Silicon ID #3[7:0] R
Sy,0Bh UID_4 Unique Silicon ID #4[7:0] R
Sy,0Ch UID_5 Unique Silicon ID #5[7:0] R
Sy,0Dh UID_6 Unique Silicon ID #6[7:0] R
Sy,0Eh UID_7 Unique Silicon ID #7[7:0] R
Sy,0Fh BL_VERH Bootloader version[15:8] R
Sy,10h BL_VERL Bootloader version[7:0] R
Sy,11h FTS_IC_VERH Focal Tech IC Version[15:8] R
Sy,12h FTS_IC_VERL Focal Tech IC Version[7:0] R
Sy,13h APP_IDH Application ID[15:8] R
Sy,14h APP_IDL Application ID[7:0] R
Sy,15h APP_VERH Application Version[15:8] R
Sy,16h APP_VERL Application Version[7:0] R
Sy,17h Unused

22
FocalTech Systems, Ltd. Confidential

<!-- page: 27 -->
CTPM Application Note

Sy,18h Unused
Sy,19h Unused
Sy,1Ah Unused
Sy,1Bh CID_0 Custom ID #0[0:7] R
Sy,1Ch CID_1 Custom ID #1[0:7] R
Sy,1Dh CID_2 Custom ID #2[0:7] R
Sy,1Eh CID_3 Custom ID #3[0:7] R
Sy,1Fh CID_4 Custom ID #4[0:7] R
… …
Sy,FEh LOG_MSG_CNT The log MSG count R
Sy,FFh LOG_CUR_CHA Current character of log message, will point to the next character when R
one character is read.

2.3.1 DEVICE_MODE
This register is the device mode register, configure it to determine the current mode of the chip.
Address Bit Address Register Name Description
Sy,00h 6:4 Device Mode[2:0] 000b Normal operating Mode
001b System Information Mode (Reserved)
100b Test Mode – read raw data (Reserved)

2.3.2 BIST_COMM
This register is the BIST command register. The BIST (built in self test) function to perform is set here.
Address Bit Address Register Name Description
Sy,01h 7:0 BIST Command[7:0] BIST command to perform.

2.3.3 BIST_STAT
This register reports the status of BIST (built in self test) functions either in progress or the last function
completed.
Address Bit Address Register Name Description
Sy,02h 7:0 BIST Command[7:0] Status of the last BIST function started.

2.3.4 BL_VERH
This register contains the MSB of the bootloader version specified by the application.
Address Bit Address Register Name Description
Sy,0Fh 7:0 Bootloader version[15:8] R:xx

23
FocalTech Systems, Ltd. Confidential

<!-- page: 28 -->
CTPM Application Note

2.3.5 BL_VERL
This register contains the LSB of the bootloader version specified by the application.
Address Bit Address Register Name Description
Sy,10h 7:0 Bootloader version[7:0] R:xx.

2.3.6 FTS_IC_VERH
This is the FTS IC version register. This register contains the MSB of the FTS IC version. The value is BCD
value, for example
FT5201 – FTS_IC_VERH(0x52), FTS_IC_VERL(0x01)
FT5202 – FTS_IC_VERH(0x52), FTS_IC_VERL(0x02)
FT5206 – FTS_IC_VERH(0x52), FTS_IC_VERL(0x06)
FT5306 – FTS_IC_VERH(0x53), FTS_IC_VERL(0x06)
FT5406 – FTS_IC_VERH(0x54), FTS_IC_VERL(0x06)
Address Bit Address Register Name Description
Sy,11h 7:0 Focal Tech IC version Focal Tech IC Version MSB
[15:8]

2.3.7 FTS_IC_VERL
This is the FTS IC version register. This register contains the MSB of the FTS IC version. The value is BCD
value, for example
FT5201 – FTS_IC_VERH(0x52), FTS_IC_VERL(0x01)
FT5202 – FTS_IC_VERH(0x52), FTS_IC_VERL(0x02)
FT5206 – FTS_IC_VERH(0x52), FTS_IC_VERL(0x06)
FT5306 – FTS_IC_VERH(0x53), FTS_IC_VERL(0x06)
FT5406 – FTS_IC_VERH(0x54), FTS_IC_VERL(0x06)
Address Bit Address Register Name Description
Sy,12h 7:0 Focal Tech IC version [7:0] Focal Tech IC Version LSB

2.3.8 APP_IDH
This is the application ID register. This register contains the MSB of the application ID. This value is set to
designate the individual project.

Address Bit Address Register Name Description
Sy,13h 7:0 Application Version [15:8] R:xx

2.3.9 APP_IDL
This is the application ID register. This register contains the MSB of the application ID. This value is set to
designate the individual project.
24
FocalTech Systems, Ltd. Confidential

<!-- page: 29 -->
CTPM Application Note

Address Bit Address Register Name Description
Sy,14h 7:0 Application Version [15:8] R:xx

2.3.10 APP_VERH
This is the application version register. This register contains the MSB of the application version. This value
should be incremented on each internal or external release of the project.

Address Bit Address Register Name Description
Sy,15h 7:0 Application Version [15:8] R:xx

2.3.11 APP_VERL
This is the application version register. This register contains the LSB of the application version. This value
should be incremented on each internal or external release of the project.

Address Bit Addr. Reg. Name Description
Sy,16h 7:0 Application Version [7:0] R:xx

2.3.12 CID_n(n:0-4)
These are Custom ID registers. These regitsters contain user defined Custom ID identifiers for the FT TPM.

Address Bit Addr. Reg. Name Description
Sy,1Bh~1Fh 7:0 Application Version [7:0] R:xx

25
FocalTech Systems, Ltd. Confidential

<!-- page: 30 -->
CTPM Application Note

3 CTPM Application Introduction

3.1 Standard Application information of FT5X06
Figure3-1,Figure3-2,Figure3-3 demonstrate the typical FT5x06 application schematic. It consists of FT’s
Capacitive Touch Panel(CTP), FT5X06 chip, and some peripheral components. According to the size of CTPM,
you can choose the numbers of TX and RX needed.

3.1.1 Standard application circuit of FT5206GE1

Figure 3-1 FT5206GE1 typical application schematic

26
FocalTech Systems, Ltd. Confidential

<!-- page: 31 -->
## Visual Summary (Page 31)

- page_class: block_diagram
- confidence: 0.95
- reason_codes: visual_content_dominant, schematic_diagram, policy_visual_summary_override

Page displays a standard application circuit schematic for the FT5306DE4 chip (Figure 3-2), showing pinout details, connection blocks (J1-J7), and typical component layout.

<!-- page: 32 -->
CTPM Application Note

3.1.3 Standard application circuit of FT5206EE8

Figure 3-3 FT5406EE8 typical application schematic

4 Communication between host and CTPM

4.1 Communication Contents
The data Host received from the CTPM through serial interface are different depend on the configuration in
Device Mode Register of the CTPM. Please refer to Section 2---CTP Register Mapping.

28
FocalTech Systems, Ltd. Confidential

<!-- page: 33 -->
CTPM Application Note

4.2 I2C Example Code
/////////////////////////////////////////////////////////////////
// I2C write bytes to device.
//
// Arguments: ucSlaveAdr - slave address
// ucSubAdr - sub address
// pBuf - pointer of buffer
// ucBufLen - length of buffer
/////////////////////////////////////////////////////////////////
void i2cBurstWriteBytes(BYTE ucSlaveAdr, BYTE ucSubAdr, BYTE *pBuf, BYTE ucBufLen)
{
BYTE ucDummy; // loop dummy
ucDummy = I2C_ACCESS_DUMMY_TIME;
while(ucDummy--)
{
if (i2c_AccessStart(ucSlaveAdr, I2C_WRITE) == FALSE)
continue;
if (i2c_SendByte(ucSubAdr) == I2C_NON_ACKNOWLEDGE) // check non-acknowledge
continue;
while(ucBufLen--) // loop of writting data
{
i2c_SendByte(*pBuf); // send byte
pBuf++; // next byte pointer
} // while
break;
} // while
i2c_Stop();
}

/////////////////////////////////////////////////////////////////
// I2C read bytes from device.
//
// Arguments: ucSlaveAdr - slave address
// ucSubAdr - sub address
// pBuf - pointer of buffer
// ucBufLen - length of buffer
/////////////////////////////////////////////////////////////////
void i2cBurstReadBytes(BYTE ucSlaveAdr, BYTE ucSubAdr, BYTE *pBuf, BYTE ucBufLen)
{
BYTE ucDummy; // loop dummy

ucDummy = I2C_ACCESS_DUMMY_TIME;
while(ucDummy--)

29
FocalTech Systems, Ltd. Confidential

<!-- page: 34 -->
CTPM Application Note

{
if (i2c_AccessStart(ucSlaveAdr, I2C_WRITE) == FALSE)
continue;
if (i2c_SendByte(ucSubAdr) == I2C_NON_ACKNOWLEDGE) // check non-acknowledge
continue;
if (i2c_AccessStart(ucSlaveAdr, I2C_READ) == FALSE)
continue;
while(ucBufLen--) // loop to burst read
{
*pBuf = i2c_ReceiveByte(ucBufLen); // receive byte
pBuf++; // next byte pointer
} // while
break;
} // while
i2c_Stop();
}

/////////////////////////////////////////////////////////////////
// I2C read current bytes from device.
//
// Arguments: ucSlaveAdr - slave address
// pBuf - pointer of buffer
// ucBufLen - length of buffer
/////////////////////////////////////////////////////////////////
void i2cBurstCurrentBytes(BYTE ucSlaveAdr, BYTE *pBuf, BYTE ucBufLen)
{
BYTE ucDummy; // loop dummy

ucDummy = I2C_ACCESS_DUMMY_TIME;
while(ucDummy--)
{
if (i2c_AccessStart(ucSlaveAdr, I2C_READ) == FALSE)
continue;
while(ucBufLen--) // loop to burst read
{
*pBuf = i2c_ReceiveByte(ucBufLen); // receive byte
pBuf++; // next byte pointer
} // while
break;
} // while
i2c_Stop();
}

30
FocalTech Systems, Ltd. Confidential
