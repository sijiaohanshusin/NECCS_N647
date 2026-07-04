**==> picture [184 x 46] intentionally omitted <==**

## FT5446U

## **True Multi-Touch Capacitive Touch Panel Controller**

## **INTRODUCTION**

The FT5446U is single-chip capacitive touch panel controllers with built-in enhanced 32bit Micro-controller unit (MCU).It provides the benefits of full screen common mode scan technology, fast response time and high level of accuracy. It can drive capacitive type touch panel with up to 16 driving and 28 sensing lines.

## **FEATURES**

**==> picture [227 x 215] intentionally omitted <==**

-  Mutual Capacitive Sensing Techniques

-  Full Screen Common Mode Scan Techniques

-  FT5446U Supports up to 16TX + 28 RX

-  Support up to 10 fingers

-  High immunity to inductive power noise

-  Automatic mode switching (Active, Monitor, Sleep)

-  Support 120Hz sampling rate

-  Auto-calibration

-  Support IIC (up to 400kbits/sec) interface

-  Power

   -  2.7 to 3.6V Operating Voltage

   -  IOVCC supports from 1.71V to 3.6V

-  Built-in 64KB Flash

-  Single Channel(TX or RX)resistance: Up to 100K Ω

-  Single Channel (transmit/receive) Capacitance:   40pF

-  12-Bit ADC Accuracy

-  Features “short I/O ” testing for sense pins

-  Supports various type of panels with no ground shielding layer

-  3  Operating Modes

**==> picture [53 x 60] intentionally omitted <==**

   -  Active

   -  Monitor

   -  Sleep

-  Package:

   -  QFN56L 6x6x0.6mm, 0.35mm/pitch

**==> picture [60 x 60] intentionally omitted <==**

**FocalTech Systems Co., Ltd.** · **www.focaltech-electronics.com** · **support@focaltech-electronics.com** Document Number: D-FT5446U-DataSheet-V0.01-Preliminary

F-OI-RD01-03-03-B

|TABLE OF CONTENTS|TABLE OF CONTENTS|TABLE OF CONTENTS||
|---|---|---|---|
|**INTRODUCTION ........................................................................................................................................................... I**||||
|**FEATURES .................................................................................................................................................................... I**||||
|1|**OVERVIEW ........................................................................................................................................................... 3**|||
||1.1|TYPICALAPPLICATIONS................................................................................................................................... 3||
|2|**FUNCTIONAL BLOCK DESCRIPTIONS ......................................................................................................... 3**|||
||2.1|ARCHITECTUREOVERVIEW............................................................................................................................. 3||
||2.2|MCU................................................................................................................................................................ 4||
||2.3|OPERATIONMODES........................................................................................................................................ 4||
||2.4|HOSTINTERFACE............................................................................................................................................ 4||
||2.5|SERIALINTERFACE.......................................................................................................................................... 5||
|3|**ELECTRICAL SPECIFICATIONS ..................................................................................................................... 6**|||
||3.1|ABSOLUTEMAXIMUMRATINGS....................................................................................................................... 6||
||3.2|DC CHARACTERISTICS.................................................................................................................................... 6||
||3.3|AC CHARACTERISTICS.................................................................................................................................... 7||
||3.4|I/O PORTSCIRCUITS....................................................................................................................................... 8||
||3.5|POWER ON/RESETSEQUENCE.................................................................................................................... 8||
|4|PIN|CONFIGURATIONS**..................................................................................................................................... 9**||
|5|PACKAGE INFORMATION**............................................................................................................................... 12**|||
||5.1|PACKAGEINFORMATION OFQFN-6X6-56L PACKAGE................................................................................. 12||
||5.2|ORDERINGINFORMATION.............................................................................................................................. 14||



**==> picture [212 x 213] intentionally omitted <==**

**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 2 of 16**

## 1 **OVERVIEW**

## 1.1 **Typical Applications**

FT5446U is powerful design for Mobile phones.

|**Part Number**|**Package**|**TX**|**RX**|**Total**<br>**Channels**|**Recommended for Smart**<br>**Phone**<br>**TP Size(16:9/18:9/20:9)**|
|---|---|---|---|---|---|
|FT5446U|QFN 56L 6x6x0.6mm<br>Pitch =0.35mm|16|28|44|`≦`6.0”,<br>Sensor Pitch:5mm|



**==> picture [329 x 269] intentionally omitted <==**

**----- Start of picture text -----**<br>
AFE INTERFACE<br>TX<br>12bit  Port0<br>LPF SAR  I2C PORT<br>ADC Controller<br>RX Port1<br>High speed Digital processor<br>P ram<br>Flash Enhanced MCU) Data ram<br>CGU OCps DSP ASM<br>EAC<br>POR Power Regulator 50MHz low power internal Oscillator<br>**----- End of picture text -----**<br>


## 2 **FUNCTIONAL BLOCK DESCRIPTIONS**

## **2.1 Architecture Overview**

Figure2-1 shows the architecture of FT5446U.

_Figure 2-1  System Architecture Diagram_

**==> picture [43 x 42] intentionally omitted <==**

The FT5446U has five main functional parts below,

##  Touch Panel Interface Circuits

The main function for the AFE and AFE controller is to interface with the touch panel. It scans the panel by sending AC signals to the panel and processes the received signals from the panel. It includes both Transmit (TX) and Receive (RX) functions. Key parameters to configure this circuit can be sent via serial interfaces.

 Enhanced  MCU with  DSP accelerator

For the Enhanced MCU, larger program and data memories are supported. Furthermore, a Flash memory is implemented to store programs and some key parameters.

Complex signal processing algorithms are implemented by MCU and DSP accelerator to detect the touches reliably and efficiently. Communication protocol software is also implemented in this MCU to exchange data and control information with the host processor.

##  External Interface

   -  I2C: an interface for data exchange with host

   -  INT: an interrupt signal to inform the host processor that touch data is ready for read

   -  RSTN: an external low signal reset the chip. The port is also use to wake up the FT5446U from the Sleep mode.

-  A watch dog timer is implemented to ensure the robustness of the chip.

-  A voltage regulator to generate 1.2V for digital circuits from the input VDD3 supply

-  Power On Reset (POR) is active until VDDD is higher than some level and hold decades of μs.

**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 3 of 16**

## **2.2 MCU**

This section describes some critical features and operations supported by the enhanced MCU.

Figure 2-2 shows the overall structure of the MCU block. In addition to the enhanced MCU core, we have added the following circuits,

-  A  DSP accelerator cooperates with  MCU to process the complex algorithms

-  Timer: A number of timers are available to generate different clocks

-  Clock Manager: To control various clocks under different operation conditions of the system

**==> picture [340 x 216] intentionally omitted <==**

**----- Start of picture text -----**<br>
Program  Data<br>Memory Memory<br>Clock Enhanced<br>Manager MCU Core<br>Master Watch<br>Timer<br>Clock dog<br>Figure 2-2  MCU Block Diagram<br>**----- End of picture text -----**<br>


## **2.3 Operation Modes**

FT5446U offers following three modes:

##  **Active Mode**

In active mode,the frame scan rate is 0~120Hz.The host processor can configure it to speed up or to slow down.

##  **Monitor Mode**

In this mode, most algorithms are stopped. A simpler algorithm is being executed to determine if there is a touch or not. When a touch is detected, FT5446U shall enter the Active mode immediately. During this mode, the serial port is closed and no data shall be transferred with the host processor.

##  **Sleep Mode**

In Sleep mode, it shall only respond to the “RESET” , “INT” signal from the host processor.

## **2.4 Host Interface**

**Figure 2-3** shows the interface between a host processor and FT5446U. This interface consists of the following three sets of signals:

-  Serial Interface

-  Interrupt from FT5446U to the Host

-  Reset Signal from the Host to FT5446U

**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 4 of 16**

**==> picture [353 x 127] intentionally omitted <==**

**----- Start of picture text -----**<br>
TP Module<br>Serial<br>Interface<br>TX<br>TP FT5446U Host<br>RX<br>/INT<br>/RST<br>**----- End of picture text -----**<br>


_Figure 2-3  Host Interface Diagram_

The serial interface of FT5446U is I2C. The detail of the interface is described in detail in Section 2.5. The interrupt signal (/INT) is used for FT5446U to inform the host that data are ready for the host to receive. The /RST signal is used for the host to wake up FT5446U from the Sleep mode. After resetting, FT5446U shall enter the Active mode.

## 2.5 Serial Interface

FT5446U supports the I2C interfaces, which can be used by a host processor or other devices. The I2C is always configured in the Slave mode. The data transfer format is shown in **Figure 2-4.**

**==> picture [440 x 315] intentionally omitted <==**

**----- Start of picture text -----**<br>
SDA<br>MSB ACK from  ACK from<br>slave receiver<br>SCL<br>1 2 3~6 7 8 9 1 2 3~7 8 9<br>START or<br>ACK ACK<br>repeat START Stop<br>Figure 2-4   I2C Serial Data Transfer Format<br>SLV addr Data[n] Data[n+1] Data[n+2]<br>S A[6:0] W A D[7:0] A D[7:0] A D[7:0] A P<br>Figure 2-5  I2C master write, slave read<br>SLV addr Data[n] Data[n+1] Data[n+2]<br>S A[6:0] R A D[7:0] A D[7:0] A D[7:0] N P<br>**----- End of picture text -----**<br>


_Figure 2-6  I2C master read, slave write_

**Table 2-1** lists the meanings of the mnemonics used in the above figures.

**Table 2-1  Mnemonics Description**

|**Mnemonics**|**Description **|
|---|---|
|S|I2C Start or I2C Restart|
|A[6:0]|Slave address|



**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 5 of 16**

|R/ W|READ/WRITE bit, ‘1’ for read, ‘0’for write|
|---|---|
|A(N)|ACK(NACK) bit|
|P|STOP: the indication of the end of a packet (if this bit is missing, S will indicate<br>the end of the currentpacket and the beginningof the nextpacket)|



I2C Interface Timing Characteristics is shown in Table 2-2.

**Table 2-2 I2C Timing Characteristics**

|**Parameter**|**Min**|**Max**|**Unit**|
|---|---|---|---|
|SCL frequency|0|400|KHz|
|Bus free time between a STOP and START condition|1.3||us|
|Hold time (repeated) START condition|0.6||us|
|Data setup time|100||ns|
|Setup time for a repeated START condition|0.6||us|
|Setup time for STOP condition|0.6||us|
|||||



## 3 **ELECTRICAL SPECIFICATIONS**

## **3.1 Absolute Maximum Ratings**

**==> picture [49 x 49] intentionally omitted <==**

**Table 3-1 Absolute Maximum Ratings**

|**Item**|**Symbol**|**Value**|**Unit**|**Note**|
|---|---|---|---|---|
|Power Supply Voltage|VDD3 – VSS|TBD|V|1|
|I/O Digital Voltage|IOVCC|TBD|V|1|
|Operating Temperature|Topr|TBD|℃|1|
|Storage Temperature|Tstg|TBD|℃|1|
||||||



## **Notes**

1. If used beyond the absolute maximum ratings, FT5446U may be permanently damaged. It is strongly recommended that the device be used within the electrical characteristics in normal operations. If exposed to the condition not within the electrical characteristics, it may affect the reliability of the device. 2. Make sure VDD3 (high) ≥VSSLF (low)

## 3.2 DC Characteristics

|3.2<br>DC Characteristics||||||||
|---|---|---|---|---|---|---|---|
|||**Table 3-2  DC Characteristics**||||||
| | | | | | | |
|**Item**|**Symbol**|**Unit**|**Test Condition**|**Min.**|**Typ. **|**Max.**|**Note**|
|Input high-level voltage|VIH|V||0.7 x IOVCC|--|IOVCC||
|Input low –level voltage|VIL|V||-0.3|--|0.3 x<br>IOVCC||
|Output high –level voltage|VOH|V||0.7 x IOVCC|--|--||
|Output low –level voltage|VOL|V||--|--|0.3  x<br>IOVCC||
|I/O leakage current|ILI|uA||-1|--|1||
|Current consumption<br>`（`Normal operation mode`）`|Iopr|mA||--|TBD|--||



**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 6 of 16**

|Current consumption<br>`（`Monitor mode`）`|Imon|mA||--|TBD|--||
|---|---|---|---|---|---|---|---|
|Current consumption<br>`（`Sleep mode`）`|Islp|uA||--|TBD|--||
|Step-up output voltage|VDD5|V|||TBD|||
|Step-up output voltage|VDD10|V|||TBD|||
|Output voltage|VDD5|V||TBD||TBD||
|Output voltage|VDD10|V||TBD||TBD||
|Power Supply voltage|VDD3|V||TBD|TBD|TBD||



Notes: This sample data is intended for design guidance only. Values shown are typical for a 16Tx × 28Rx sensor configured at 80 Hz report rate.Actual current will depend on the particular sensor design and firmware options.

|ware options.|ware options.||||||||
|---|---|---|---|---|---|---|---|---|
|3.3<br>AC Characteristics|||**AC Characteristics of Oscillators**||||||
|**Item**|**Symbol**|**Unit**||**Test Condition**|**Min.**|**Typ.**|**Max.**|**Note**|
|OSC clock 1|fosc1|MHz||VDD3 = 2.8V; Ta=25℃|TBD|TBD|TBD||
| | | | | | | |



## **Table 3-3  AC Characteristics of TX & RX**

| | | | | | | |
|---|---|---|---|---|---|---|---|
|**Item**|**Symbol**|**Test Condition**|**Min**|**Typ**|**Max**|**Unit**|**Note**|
|TX acceptable clock|ftx||50|150|400|KHz||
|TX output rise time|Ttxr||--|210|--|nS||
|TX output fall time|Ttxf||--|210|--|nS||
|RX input voltage|Trxi||1.2|--|1.6|V||
| | | | | | | |



**==> picture [111 x 102] intentionally omitted <==**

**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 7 of 16**

## 3.4 **I/O Ports Circuits**

**==> picture [204 x 147] intentionally omitted <==**

**----- Start of picture text -----**<br>
IOVCC IOVCC<br>2M ohm<br>VDDA<br>Input circuit<br>IOVCC<br>Floating Sub Output enable<br>Output data<br>**----- End of picture text -----**<br>


**==> picture [133 x 122] intentionally omitted <==**

**----- Start of picture text -----**<br>
.<br>**----- End of picture text -----**<br>


_Figure 3-1_ General Purpose _In/Out Port Circuit_ .

The input/output property can be configured via firmware setting. The firmware can also control its output behavior as push-pull or as open-drain that SDA of I2C interface is required.

**==> picture [130 x 70] intentionally omitted <==**

**----- Start of picture text -----**<br>
IOVCC<br>50Kohm<br>300<br>**----- End of picture text -----**<br>


_Figure 3-2 Reset Input Port Circuits_

## 3.5 POWER ON/Reset Sequence

Reset should be pulled down to be low before powering on and powering down.  I2C shouldn’t  be used by other devices during Reset time after VDD powering on (Trtp).  INT signal will be sent to the host after initializing all parameters and then start to report points to the host. If Power is down, the voltage of supply must be below 0.3V and Tpdt is more than 1ms.

**==> picture [111 x 102] intentionally omitted <==**

**==> picture [190 x 132] intentionally omitted <==**

**----- Start of picture text -----**<br>
Tris<br>0.9VDD<br>0.1VDD<br>Figure 3-3 Power on time<br>Tpdt<br>Power<br>0.3V<br>**----- End of picture text -----**<br>


_Figure 3-4 Power Cycle  requirement_

**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 8 of 16**

**==> picture [318 x 106] intentionally omitted <==**

**----- Start of picture text -----**<br>
Trtp Tvdr<br>Tvdr<br>VDD3<br>Reset<br>INT<br>I2C<br>IOVCC Tpio<br>**----- End of picture text -----**<br>


_Figure 3-5 Power on Sequence_

Reset time must be enough to guarantee reliable reset, the time of starting to report point after resetting approach to the time of starting to report point after powering on.

**==> picture [233 x 71] intentionally omitted <==**

**----- Start of picture text -----**<br>
Trsi<br>Trst<br>Power<br>Reset<br>INT<br>I2C<br>**----- End of picture text -----**<br>


**==> picture [47 x 47] intentionally omitted <==**

_Figure 3-6 Reset Sequence_

**==> picture [51 x 43] intentionally omitted <==**

**Table 3-5 Power on/Reset Sequence Parameters**

|**Parameter**|**Description**|**Min**|**Max**|**Units**|
|---|---|---|---|---|
|Tris|Rise time from  0.1VDD to 0.9VDD|--|5|ms|
|Tpdt|Time of the voltage of supply being below 0.3V|5|--|ms|
|Trtp|Time of resetting to be low  before powering on|100|--|μs|
|Tvdr|Reset time after VDD powering on|1|--|ms|
|Trsi|Time of starting to report point after resetting|--|200|ms|
|Trst|Reset time|1|--|ms|
|Tpio|Time of IOVCC to be high  before powering on|0||ms|



## 4 PIN CONFIGURATIONS

Pin List of FT5446U

**Table 4-1  Pin Definition**

**==> picture [60 x 60] intentionally omitted <==**

||**Pin No.**|**Type **|**Description**|
|---|---|---|---|
|**Name**|**FT5446U**|||
|RX28|55|I|Receiver inputpins|
|RX27|54|I|Receiver inputpins|
|RX26|53|I|Receiver inputpins|
|RX25|52|I|Receiver inputpins|
|RX24|51|I|Receiver inputpins|
|RX23|50|I|Receiver inputpins|
|RX22|49|I|Receiver inputpins|
|RX21|48|I|Receiver inputpins|
|RX20|47|I|Receiver inputpins|
|RX19|46|I|Receiver inputpins|
|RX18|45|I|Receiver inputpins|



**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 9 of 16**

||RX17|44|I|Receiver inputpins||
|---|---|---|---|---|---|
||RX16|43|I|Receiver inputpins||
||RX15|42|I|Receiver inputpins||
||RX14|41|I|Receiver inputpins||
||RX13|40|I|Receiver inputpins||
||RX12|39|I|Receiver inputpins||
||RX11|38|I|Receiver inputpins||
||RX10|37|I|Receiver inputpins||
||RX9|36|I|Receiver inputpins||
||RX8|35|I|Receiver inputpins||
||RX7|34|I|Receiver inputpins||
||RX6|33|I|Receiver inputpins||
||RX5|32|I|Receiver inputpins||
||RX4|31|I|Receiver inputpins||
||RX3|30|I|Receiver inputpins||
||RX2|29|I|Receiver inputpins||
||RX1|28|I|Receiver inputpins||
||VDD5_IN|56|PWR|internal generated 5V power sup-<br>ply, A 1μF ceramic  capacitor to<br>ground is required.||
||VSSLF|57|PWR|Analog ground||
||VDD5_Out|1|PWR|digital power supply, A 1μF ceramic<br>capacitor toground is required.||
||NC|2|NC|||
||TX10|3|O|Transmit outputpin||
||TX11|4|O|Transmit outputpin||
||TX12|5|O|Transmit outputpin||
||TX13|6|O|Transmit outputpin||
||TX14|7|O|Transmit outputpin||
||TX15|8|O|Transmit outputpin||
||TX16|9|O|Transmit outputpin||
||VDD10|10|PWR|digital power supply, A 1μF ceramic<br>capacitor toground is required.||
||VDD3|11|PWR|digital power supply, A 1μF ceramic<br>capacitor toground is required.||
||VDD15|12|PWR|digital power supply, A 1μF ceramic<br>capacitor toground is required.||
||VDD12|13|PWR|digital power supply, A 1μF ceramic<br>capacitor toground is required.||
||IOVCC|14|PWR|I/Opower supply||
||INT|15|I/O|Interrupt request to the host, or<br>Wakeuprequest from the host.||
||SDA|16|I/O|I2C data input and output||
||SCL|17|I/O|I2C clock input||
||RSTN|18|I|External Reset,Low is active||
||TX1|19|O|Transmit outputpin||
||TX2|20|O|Transmit outputpin||
||TX3|21|O|Transmit outputpin||



**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 10 of 16**

|TX4|22|O|Transmit outputpin|
|---|---|---|---|
|TX5|23|O|Transmit outputpin|
|TX6|24|O|Transmit outputpin|
|TX7|25|O|Transmit outputpin|
|TX8|26|O|Transmit outputpin|
|TX9|27|O|Transmit outputpin|



**==> picture [454 x 443] intentionally omitted <==**

**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 11 of 16**

**==> picture [318 x 261] intentionally omitted <==**

**----- Start of picture text -----**<br>
Vout5 1 42 RX15<br>NC 2 41 RX14<br>TX10 3 40 RX13<br>TX11 4 39 RX12<br>TX12 5 38 RX11<br>TX13 6 37 RX10<br>TX14TX15 78 FT5446U 3635 RX9RX8<br>TX16 9 34 RX7<br>VDD10 10 33 RX6<br>VDD3 11 32 RX5<br>VDD15 12 31 RX4<br>30 RX3<br>VDD12 13<br>29 RX2<br>IOVCC 14<br>FT5446U Package Diagram<br>VSSLF Vin5 RX28 RX27 RX26 RX25 RX24 RX23 RX22 RX21 RX20 RX19 RX18 RX17 RX16<br>57 56 55 54 53 52 51 50 49 48 47 46 45 44 43<br>15 16 17 18 19 20 21 22 23 24 25 26 27 28<br>INT SDA SCL RSTN TX1 TX2 TX3 TX4 TX5 TX6 TX7 TX8 TX9 RX1<br>**----- End of picture text -----**<br>


## 5 PACKAGE INFORMATION

- 5.1 Package Information of QFN-6x6-56L Package

**==> picture [378 x 235] intentionally omitted <==**

**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 12 of 16**

|||||**Millimeter**<br>**Min**<br>**Type **<br>**Max**<br>0.5<br>0.55<br>0.6<br>0<br>0.035<br>0.05<br>----<br>0.4<br>----<br>0.152 REF<br>0.13<br>0.18<br>0.23<br>0.07<br>0.12<br>0.17<br>6BSC<br>6 BSC<br>0.35BSC<br>3.9<br>4<br>4.1<br>3.9<br>4<br>4.1<br>0.35<br>0.4<br>0.45<br>1.45<br>1.55<br>1.65<br>0.1<br>0.1<br>0.08<br>0.1<br>0.1|**Millimeter**<br>**Min**<br>**Type **<br>**Max**<br>0.5<br>0.55<br>0.6<br>0<br>0.035<br>0.05<br>----<br>0.4<br>----<br>0.152 REF<br>0.13<br>0.18<br>0.23<br>0.07<br>0.12<br>0.17<br>6BSC<br>6 BSC<br>0.35BSC<br>3.9<br>4<br>4.1<br>3.9<br>4<br>4.1<br>0.35<br>0.4<br>0.45<br>1.45<br>1.55<br>1.65<br>0.1<br>0.1<br>0.08<br>0.1<br>0.1|**Millimeter**<br>**Min**<br>**Type **<br>**Max**<br>0.5<br>0.55<br>0.6<br>0<br>0.035<br>0.05<br>----<br>0.4<br>----<br>0.152 REF<br>0.13<br>0.18<br>0.23<br>0.07<br>0.12<br>0.17<br>6BSC<br>6 BSC<br>0.35BSC<br>3.9<br>4<br>4.1<br>3.9<br>4<br>4.1<br>0.35<br>0.4<br>0.45<br>1.45<br>1.55<br>1.65<br>0.1<br>0.1<br>0.08<br>0.1<br>0.1|
|---|---|---|---|---|---|---|
||**Item**||**Symbol**|**Millimeter**|||
|||||**Min**|**Type **|**Max**|
||Total Thickness||A|0.5|0.55|0.6|
||Stand Off||A1|0|0.035|0.05|
||MoldThickness||A2|----|0.4|----|
||L/F Thickness||A3|0.152 REF|||
||Lead Width||b|0.13|0.18|0.23|
||||b1|0.07|0.12|0.17|
||Body Size|X|D|6BSC|||
|||Y|E|6 BSC|||
||LeadPitch||e|0.35BSC|||
||EP Size|X|J|3.9|4|4.1|
|||Y|K|3.9|4|4.1|
||LeadLength||L|0.35|0.4|0.45|
||||R|1.45|1.55|1.65|
||PackageEdgeTolerance||aaa|0.1|||
||Mold Flatness||bbb|0.1|||
||CoPlanarity||ccc|0.08|||
||Lead Offset||ddd|0.1|||
||Exposed Pad Offset||eee|0.1|||
| | | | | | | |



**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 13 of 16**

## 5.2 Ordering Information

**==> picture [481 x 131] intentionally omitted <==**

**----- Start of picture text -----**<br>
QFN<br>Package Type 56Pin(6 * 6 )<br>56Pin(0.6 – P0.35)<br>Product Name  Package Type  # TX Pins  # RX Pins<br>FT5446U  QFN-56L  16  28<br>**----- End of picture text -----**<br>


**==> picture [386 x 386] intentionally omitted <==**

**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 14 of 16**

## **Appendix: IC Revision history of FT5446U Specification**

|**Version**|**Change Items**|**Effective Date**|
|---|---|---|
|0.01|1stPreliminary|9-Aug-18|
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
||||



**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 15 of 16**

## END OF DATASHEET

**==> picture [454 x 443] intentionally omitted <==**

**THIS DOCUMENT CONTAINS CONFIDENTIAL AND PRIVILEGED INFORMATION. UNAUTHORIZED USE, COPY OR DISCLOSURE IS PROHIBITED**

**Copyright © 2018, FocalTech Systems Co., Ltd . All rights reserved Version 0.01** `︱` **Page 16 of 16**
