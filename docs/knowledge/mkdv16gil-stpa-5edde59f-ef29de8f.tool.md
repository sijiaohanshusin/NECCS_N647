**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **MK SD NAND Product Datasheet Industrial Grade**

## **Product List**

MKDV08GIL-STPA MKDV16GIL-STPA MKDV32GIL-STPA

www.mkfounder.com

1

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **Revision History**

|**Version**|**Date**|**Description**|
|---|---|---|
|Rev 1.0|2023/5/10|Original version|



## **Notice :**

The datasheet is prepared and approved by MK Founder Technology Co., LTD MK Founder reserves the right to change products or specifications without notice.

2

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **Table Of Contents**

1 Introduction .................................................................................................................................... 4 2 Product List .................................................................................................................................... 5 3 Features ........................................................................................................................................ 5 4 Physical Characteristics ................................................................................................................. 5 4.1 Temperature ......................................................................................................................................... 5 5 Pin Assignments(SD Mode& SPI Mode) ......................................................................................... 6 6 Usage ............................................................................................................................................ 7 6.1 SD Bus Mode protocol ......................................................................................................................... 7 6.2 Card Initialize ....................................................................................................................................... 9 6.3 DC Characteristics ............................................................................................................................. 12 7 Internal Information ...................................................................................................................... 13 7.1 Registers ............................................................................................................................................ 13 7.1.1 OCR Register ..................................................................................................................................... 14 7.1.2 CID Register ....................................................................................................................................... 15 7.1.3 CSD Register ..................................................................................................................................... 16 7.1.4 RCA Register ..................................................................................................................................... 17 7.1.5 DSR Register ..................................................................................................................................... 17 8 Power Scheme ............................................................................................................................. 17 8.1 Power Up ........................................................................................................................................... 17 8.2 Power Up Time .................................................................................................................................. 18 8.2.1 Power On or Power Cycle .................................................................................................................. 19 8.2.2 Power Supply Ramp Up ..................................................................................................................... 19 8.2.3 Power Supply Ramp Up ..................................................................................................................... 19 9 Package Dimensions ................................................................................................................... 20 10 Reference Design ...................................................................................................................... 21

3

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **1 Introduction**

MK SD NAND is an embedded storage solution designed in a LGA package form. The operation of SD is similar to an SD card which is an Industrial standard.

SD NAND consists of NAND flash and a high performance controller. 3.3V supply voltage is required for the NAND area (VCC).

SD NAND is fully compliant with SD2.0 interface, which allows most of general CPU to utilize.

SD NAND has high performance, high quality and low power consumption.

MK’s SD NAND family includes an on-board intelligent controller which manages interface protocols; security algorithms for content protection; data storage and retrieval, as well as Error Correction Code (ECC) algorithms; defect handling; power management; wear leveling and clock control.

**==> picture [392 x 228] intentionally omitted <==**

4

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## 2 **Product List**

|**Part Number**|**Flash Type**|**Capacity**|**Actual Capacity**|**Package**|**Size**|
|---|---|---|---|---|---|
|MKDV08GIL-STPA|pSLC|8Gbit|960MByte|LGA-8|6.6x8.0mm|
|MKDV16GIL-STPA|pSLC|16Gbit|1850MByte|LGA-8|6.6x8.0mm|
|MKDV32GIL-STPA|pSLC|32Gbit|3696MByte|LGA-8|6.6x8.0mm|



## **3 Features**

- ⚫ Support up to 50MHz clock frequency

- ⚫ Supports SPI Mode

- ⚫ Built-in HW ECC Engine and highly reliable NAND management mechanism

- ⚫ C10, U1, V10

- ⚫ Smaller package LGA-8

- ⚫ Advanced thermal management features to maximize performance and data protection at extended temperatures Static, dynamic, and global wear leveling

- ⚫ Bad block management, intelligent garbage collection and support for interleaving, cache, and multi-plane programming

- ⚫ Read disturb management and dynamic data refresh

- ⚫ Best-in-class power fail management

- ⚫ 50,000 Program / Erase cycles (Typ)

## **4 Physical Characteristics**

## **4.1 Temperature**

- 1) Operation Conditions Temperature Range:  - 40 ℃ to 85 ℃

- 2) Storage Conditions Temperature Range:  - 40 ℃ to 85 ℃

5

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **5 Pin Assignments(SD Mode& SPI Mode)**

## **TOP VIEW**

**==> picture [190 x 142] intentionally omitted <==**

**==> picture [401 x 170] intentionally omitted <==**

- a. Type Key: S=power supply; I= input; O=output using push-pull drivers; PP=I/O using push-pull drivers.

- b. The extended DAT lines (DAT1-DAT3) are input on power up. They start to operate as DAT lines after the SET_BUS_WIDTH Type Key: S=power supply; I=input; O=output using push-pull drivers; PP=I/O using push-pull drivers.

- c. At power up this line has a 50 kilohm pull-up enabled in the card. This resistor serves two functions: Card detection and Mode Selection. For Mode Selection, the host can drive the line high or let it be pulled high to select SD mode. If the host wants to select SPI mode it should drive the line low. For Card detection, the host detects that the line is pulled high. This pull-up should be disconnected by the user, during regular data transfer, with SET_CLR_CARD_DETECT (ACMD42) command.

6

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **6 Usage**

## **6.1  SD Bus Mode protocol**

The SD bus allows the dynamic configuration of the number of data line from 1 to 4 Bi-directional data signal. After power up by default, the SD card will use only DAT0. After initialization, host can change the bus width.

Multiplied SD cards connections are available to the host. Common VDD, VSS and CLK signal connections are available in the multiple connections. However, Command, Respond and Data lined (DAT0-DAT3) shall be divided for each device from host.

This feature allows easy trade off between hardware cost and system performance. Communication over the SD bus is based on command and data bit stream initiated by a start bit and terminated by stop bit.

## **Command**

Commands are transferred serially on the CMD line. A command is a token to starts an operation from host to the device. Commands are sent to an addressed single card (addressed Command) or to all connected cards (Broad cast command).

## **Response**

Responses are transferred serially on the CMD line.

A response is a token to answer to a previous received command. Responses are sent from an addressed single card or from all connected cards.

## **Data**

Data can be transfer from the card to the host or vice versa. Data is transferred via the data lines.

7

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

**==> picture [280 x 292] intentionally omitted <==**

**----- Start of picture text -----**<br>
SD NAND (A)<br>**----- End of picture text -----**<br>


**==> picture [65 x 275] intentionally omitted <==**

|CLK|Host card Clock signal|
|---|---|
|CMD|Bi-directional Command/Response Signal|
|DAT0 - DAT3|4 Bi-directional data signal|
|VDD|Power supply|
|VSS|GND|



8

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **6.2 Card Initialize**

To initialize the SD NAND, follow the following procedure is recommended example.

- 1) Supply Voltage for initialization

Host System can apply the Operating Voltage from initialization to the card. Apply more than 74 cycles of Dummy-clock to the SD card.

- 2) Select operation mode (SD mode or SPI mode)

In case of SPI mode operation, host should drive 1 pin (CD/DAT3) of SD Card I/F to “Low” level. Then, issue CMD0. In case of SD mode operation, host should drive or detect 1 pin of SD Card I/F (Pull up register of 1 pin is pull up

to “High” normally).

Card maintain selected operation mode except re-issue of CMD0 or power on below is SD mode initialization procedure.

- 3) Send the ACMD41 with Arg = 0 and identify the operating voltage range of the Card.

- 4) Apply the indicated operating voltage to the card.

- Reissue ACMD41 with apply voltage storing and repeat ACMD41 until the busy bit is cleared. (Bit 31 Busy = 1) If response time out occurred, host can recognize not SD Card.

- 5) Issue the CMD2 and get the Card ID (CID).

- 6) Issue the CMD3 and get the RCA. (RCA value is randomly changed by access, not equal zero)

- 7) Issue the CMD7 and move to the transfer state.

If necessary, Host may issue the ACMD42 and disabled the pull up resistor for Card detect.

- 8) Issue the ACMD13 and poll the Card status as SD Memory Card.Check SD_CARD_TYPE value. If significant 8 bits are “all zero”, that means SD Card. If it is not, stop initialization.

- 9) Issue CMD7 and move to standby state. Issue CMD9 and get CSD. Issue CMD10 and get CID.

- 10) Back to the Transfer state with CMD7.

- 11) Issue ACMD6 and choose the appropriate bus-width.

Then the Host can access the Data between the SD card as a storage device.

9

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **SPI Mode Initialization Flow**

**==> picture [489 x 371] intentionally omitted <==**

10

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **Normal SD initial flow**

**==> picture [469 x 621] intentionally omitted <==**

**SD card Initialize Procedure**

11

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **6.3 DC Characteristics**

## **DC Characteristics**

|Item|Item|Symbol|MIN.|MAX.|Unit|Note|
|---|---|---|---|---|---|---|
|Supply Voltage||VDD|2.7|3.6|V||
|Input Voltage|High Level|VIH|VDD×0.625|VDD+0.3|V||
||Low Level|VIL|VSS-0.3|VDD×0.25|V||
|Output<br>Voltage|High Level|VOH|VDD×0.75|－|V|IOH = -2mA , VDD=VDD min|
||Low Level|VOL|－|VDD×0.125|V|IOL = 2mA , VDD=VDD min|
|Standby Current||ICC1|－|0.25|mA|VDD = 3.6V , Clock 25MHz|
||||－|0.05||VDD = 3.3V, Clock STOP,<br>Ta=25℃|
|Operation<br>Current (*)|Write|ICC2|－|30|mA|3.3V / 25MHz|
||Read||－|28|||
|Input Voltage Setup Time||Vrs|－|250|ms|From 0V to VDD min|



*) Peak Current: RMS value over a 10usec period

## **Peak Voltage and Leak Current**

|Item|Symbol|Min.|Max.|Unit|Note|
|---|---|---|---|---|---|
|Peak voltage on all lines||-0.3|VDD+0.3|V||
|Input Leakage Current<br>for all pins||-10|10|uA||
|Output Leakage<br>Current for all outputs||-10|10|uA||



## **Signal Capacitance**

|Item|Symbol|Min.|Max.|Unit|Note|
|---|---|---|---|---|---|
|Pull up Resistance|RCMD<br>RDAT|10|100|kΩ||
|Total bus capacitance<br>for each signal line|CL|─|40|pF|1 card<br>CHOST+CBUS≦30pF|
|Card capacitance<br>for signal pin|CCAR D|─|10|pF||
|Pull up Resistance<br>inside card ( pin1 )|RDAT3|10|90|kΩ||
|Capacity Conneted to<br>Power line|CC|─|5|uF||



12

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **7 Internal Information**

## **7.1 Registers**

The SD NAND has six registers and SD Status information: OCR, CID, CSD, RCA, DSR, SCR and SD Status. DSR IS NOT SUPPORTED in this card.

There are two types of register groups.

MMC compatible registers: OCR, CID, CSD, RCA, DSR, and SCR SD card Specific: SD Status

## **SD card Registers**

|||**SD card Registers**|
|---|---|---|
|Resister Name|Bit Width|Description|
|OCR|32|Operation Conditions (VDU Voltage Profile and Busy Status|
|CID|128|Card Identification information|
|CSD|128|Card specific information|
|RCA|16|Relative Card Address|
|DSR|16|Not Implemented (Programmable Card Driver): Driver Stage Register|
|SCR|64|SD Memory Card‟s special features|
|SD Status|512|Status bits and Card features|



13

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **7.1.1 OCR Register**

This 32-bit register describes operating voltage range and status bit in the power supply.

## **OCR register definition**

|OCR bit|VDD voltage window|Initial|
|---|---|---|
|31|Card power up status bit(busy)|“0”= busy|
|30|Card Capacity Status|“0”= SD Memory Card|
|29-25|reserved|All 0|
|24|Switching to 1.8V Accepted(S18A)|0|
|23|3.6 - 3.5|1|
|22|3.5 - 3.4|1|
|21|3.4 - 3.3|1|
|20|3.3 - 3.2|1|
|19|3.2 - 3.1|1|
|18|3.1 - 3.0|1|
|17|3.0 - 2.9|1|
|16|2.9 - 2.8|1|
|15|2.8 - 2.7|1|
|14|Reserved|0|
|13|Reserved|0|
|12|Reserved|0|
|11|Reserved|0|
|10|Reserved|0|
|9|Reserved|0|
|8|Reserved|0|
|7|Reserved for Low Voltage Range|0|
|6|Reserved|0|
|5|Reserved|0|
|4|Reserved|0|
|3-0|reserved|All 0|



bit 23-4: Describes the SD Card Voltage

bit 31 indicates the card power up status. Value “1” is set after power up and initialization procedure has been completed.

14

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **7.1.2 CID Register**

The CID (Card Identification) register is 128-bit width. It contains the card identification information. (Refer Appendix 3. for the detail)

The Value of CID Register is vender specific.

## **CID Register**

|Field|Width|CID-slice|Initial Value|
|---|---|---|---|
|MID|8|[127:120]|0x22|
|OID|16|[119:104]|0x2345|
|PNM|40|[103:64]|MK|
|PRV|8|[63:56]|0x06|
|PSN|32|[55:24]|150C0415|
|reserved|4|[23:20]|0x0|
|MDT|12|[19:8]|0x21C|
|CRC|7|[7:1]|CRC7|
|reserved|1|[0:0]|0x1|



- a. Depends on the SD Card. Controlled by Production Lot.

- b. Depends on the CID Register

15

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **7.1.3 CSD Register**

CSD is Card-Specific Data register provides information on 128bit width. Some field of this register can writable by PROGRAM_CSD (CMD27).

## **CSD Register**

||||**CSD Register**||
|---|---|---|---|---|
|Field|Width|Cell Type|<br>CSD Slice|Initial Value|
|CSD_STRUCTURE|2|R|[127:126]|01b|
|reserved|6|R|[125:120]|000000b|
|TAAC|8|R|[119:112]|00001110b|
|NSAC|8|R|[111:104]|00000000b|
|TRAN_SPEED|8|R|[103:96]|00110010b|
|CCC|12|R|[95:84]|010110110101b|
|READ_BL_LEN|4|R|[83:80]|1001b|
|READ_BL_PARTIAL|1|R|[79:79]|0b|
|WRITE_BLK_MISALIG|1|R|[78:78]|0b|
|READ_BLK_MISALIGN|1|R|[77:77]|0b|
|DSR_IMP|1|R|[76:76]|0b|
|reserved|6|R|[75:70]|000000b|
|C_SIZE|22|R|[69:48]|0000000001110011011111b|
|reserved|1|R|[47:47]|0b|
|ERASE_BLK_EN|1|R|[46:46]|1b|
|SECTOR_SIZE|7|R|[45:39]|1111111b|
|WP_GRP_SIZE|7|R|[38:32]|0000000b|
|WP_GRP_ENABLE|1|R|[31:31]|0b|
|reserved|2|R|[30:29]|00b|
|R2W_FACTOR|3|R|[28:26]|010b|
|WRITE_BL_LEN|4|R|[25:22]|1001b|
|WRITE_BL_PARTIAL|1|R|[21:21]|0b|
|reserved|5|R|[20:16]|00000b|
|FILE_FORMAT_GRP|1|R/W(1)|[15:15]|0b|
|COPY|1|R/W(1)|[14:14]|0b|
|PERM_WRITE_PROTE|1|R/W(1)|[13:13]|0b|
|TMP_WRITE_PROTEC|1|R/W|[12:12]|0b|
|FILE_FORMAT|2|R|[11:10]|00b|
|reserved|2|R|[9:8]|00b|
|CRC|7|R/W|[7:1]|1101011b|
|not used, always'1'|1|-|[0:0]|1b|



Cell Type:R: Read Only, R/W: Writable and Readable, R/W(1): One-time Writable / Readable Note: Erase of one data block is not allowed in this card. This information is indicated by “ERASE_BLK_EN”.

Host System should refer this value before one data block size erase.

16

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **7.1.4 RCA Register**

The writable 16bit relative card address register carries the card address in SD Card mode.

## **7.1.5 DSR Register**

This register is not implemented on this car.

## **8 Power Scheme**

## **8.1 Power Up**

**==> picture [358 x 216] intentionally omitted <==**

'Power up time' is defined as voltage rising time from 0 volt to VDD min.

'Supply ramp up time' provides the time that the power is built up to the operating level (Host Supply Voltage) and the time to wait until the SD NAND can accept the first command, The host shall supply power to the card so that the voltage is reached to Vdd_min within 250ms and start to supply at least 74 SD clocks to the SD NAND with keeping CMD line to high.

17

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **8.2 Power Up Time**

**==> picture [384 x 181] intentionally omitted <==**

Host needs to keep power line level less than 0.5V and more than 1ms before power ramp up.

18

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **8.2.1 Power On or Power Cycle**

Followings are requirements for Power on and Power cycle to assure a reliable Tailor™ SD hard reset.

- (1) Voltage level shall be below 0.5V

- (2) Duration shall be at least 1ms.

## **8.2.2 Power Supply Ramp Up**

The power ramp up time is defined from 0.5V threshold level up to the operating supply voltage which is stable between VDD(min.) and VDD(max.) and host can supply SDCLK.

Followings are recommendation of Power ramp up:

- (1) Voltage of power ramp up should be monotonic as much as possible.

- (2) The minimum ramp up time should be 0.1ms.

- (3) The maximum ramp up time should be 35ms for 2.7-3.6V power supply.

## **8.2.3 Power Supply Ramp Up**

When the host shuts down the power, the VDD shall be lowered to less than 0.5Volt for a minimum period of 1ms. During power down, DAT, CMD, and CLK should be disconnected or driven to logical 0 by the host to avoid a situation that the operating current is drawn through the signal lines.

If the host needs to change the operating voltage, a power cycle is required. Power cycle means the power is turned off and supplied again. Power cycle is also needed for accessing cards that are already in Inactive State. To create a power cycle the host shall follow the power down description before power up the card (i.e. the VDD shall be once lowered to less than 0.5Volt for a minimum period of 1ms).

19

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **9 Package Dimensions**

**==> picture [283 x 222] intentionally omitted <==**

**==> picture [281 x 228] intentionally omitted <==**

**==> picture [186 x 193] intentionally omitted <==**

**==> picture [516 x 172] intentionally omitted <==**

20

**SD NAND**

**==> picture [39 x 31] intentionally omitted <==**

## **10 Reference Design**

**==> picture [439 x 287] intentionally omitted <==**

RDAT and RCMD (10K~100 kΩ) are pull-up resistors protecting the CMD and the DAT lines against bus floating when SD NAND is in a high-impedance mode.

The host shall pull-up all DAT0-3 lines by RDAT, even if the host uses the SD NAND as 1 bit mode-only in SD mode. It is recommended to have 2.2uF capacitance on VDD. Rclk reference 0~120 Ω.

21
