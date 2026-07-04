|1<br>2<br>3<br>5<br>4<br>SCL<br>GND<br>SDA<br>WP<br>VCC<br>1<br>2<br>3<br>4<br>8<br>7<br>6<br>5<br>A0<br>A1<br>A2<br>GND<br>VCC<br>WP<br>SCL<br>SDA<br>1<br>2<br>3<br>4<br>8<br>7<br>6<br>5<br>VCC<br>WP<br>SCL<br>SDA<br>A0<br>A1<br>A2<br>GND<br>VCC<br>WP<br>SCL<br>SDA<br>A0<br>A1<br>A2<br>GND<br>1<br>2<br>3<br>4<br>8<br>7<br>6<br>5<br>1<br>2<br>3<br>4<br>8<br>7<br>6<br>5<br>A0<br>A1<br>A2<br>GND<br>VCC<br>WP<br>SCL<br>SDA<br>1<br>2<br>3<br>4<br>8<br>7<br>6<br>5<br>A0<br>A1<br>A2<br>GND<br>VCC<br>WP<br>SCL<br>SDA<br>8-lead SOIC<br>8-ball dBGA2<br>8-lead Ultra-Thin<br>Mini-MAP (MLP 2x3)<br>8-lead PDIP<br>5-lead SOT23<br>Bottom View<br>Bottom View<br>8-lead TSSOP<br>**Features**<br>• **Low-voltage and Standard-voltage Operation**<br>**– 1.8 (VCC = 1.8V to 5.5V)**<br>• **Internally Organized 256 x 8 (2K)**<br>• **Two-wire Serial Interface**<br>• **Schmitt Trigger, Filtered Inputs for Noise Suppression**<br>• **Bidirectional Data Transfer Protocol**<br>• **1 MHz (5V), 400 kHz (1.8V, 2.5V, 2.7V) Compatibility**<br>• **Write Protect Pin for Hardware Data Protection**<br>• **8-byte Page (2K) Write Modes**<br>• **Partial Page Writes Allowed**<br>• **Self-timed Write Cycle (5 ms max)**<br>• **High-reliability**<br>**– Endurance: 1 Million Write Cycles**<br>**– Data Retention: 100 Years**<br>• **8-lead PDIP, 8-lead JEDEC SOIC, 8-lead Ultra Thin Mini-MAP (MLP 2x3), 5-lead SOT23,**<br>**8-lead TSSOP and 8-ball dBGA2 Packages**<br>• **Lead-free/Halogen-free**<br>• **Available in Automotive**<br>• **Die Sales: Wafer Form and Tape and Reel**<br>**Description**<br>The AT24C02B provides 2048 bits of serial electrically erasable and programmable<br>read-only memory (EEPROM) organized as 256 words of 8 bits each. The device is<br>optimized for use in many industrial and commercial applications where low-power<br>and low-voltage operation are essential. The AT24C02B is available in space-saving<br>8-lead PDIP, 8-lead JEDEC SOIC, 8-lead Ultra Thin Mini-MAP (MLP 2x3), 5-lead<br>SOT23, 8-lead TSSOP, and 8-ball dBGA2 packages and is accessed via a Two-wire<br>serial interface. In addition, the AT24C02B is available in 1.8V (1.8V to 5.5V) version.<br>Note:<br>For use of 5-lead SOT23, the<br>software A2, A1, and A0 bits in<br>the device address word<br>must be set to zero to prop-<br>erly communicate.<br>**Table 0-1.**<br>Pin Configuration<br>**Pin Name**<br>**Function**<br>A0 - A2<br>Address Inputs<br>SDA<br>Serial Data<br>SCL<br>Serial Clock Input<br>WP<br>Write Protect<br>GND<br>Ground<br>VCC<br>Power Supply|||
|---|---|---|
||||
||||
||**Two-wire**<br>**Serial EEPROM**<br>**2K (256 x 8)**||
||**AT24C02B**<br>**Not**<br>**Recommended**<br>**for New Design**<br>5126H–SEEPR–8/07||



**==> picture [298 x 35] intentionally omitted <==**

**==> picture [301 x 34] intentionally omitted <==**

## **Absolute Maximum Ratings**

Operating Temperature ................................ –55  C to +125  C Storage Temperature.................................... –65  C to +150  C Voltage on Any Pin with Respect to Ground ....................................–1.0V to +7.0V Maximum Operating Voltage .......................................... 6.25V DC Output Current........................................................ 5.0 mA

- *NOTICE: Stresses beyond those listed under “Absolute Maximum Ratings” may cause permanent damage to the device. This is a stress rating only and functional operation of the device at these or any other conditions beyond those indicated in the operational sections of this specification is not implied. Exposure to absolute maximum rating conditions for extended periods may affect device reliability.

## **Figure 0-1.** Block Diagram

**==> picture [331 x 267] intentionally omitted <==**

**----- Start of picture text -----**<br>
VCC<br>GND<br>WP<br>START<br>SCL<br>STOP<br>SDA<br>LOGIC<br>SERIAL<br>EN<br>CONTROL H.V. PUMP/TIMING<br>LOGIC<br>LOAD<br>DEVICE COMP DATA RECOVERY<br>ADDRESS<br>COMPARATOR LOAD INC<br>A2<br>A1 R/W DATA WORD EEPROM<br>A0 ADDR/COUNTER<br>Y DEC SERIAL MUX<br>DIN DOUT/ACK<br>LOGIC<br>DOUT<br>X DEC<br>**----- End of picture text -----**<br>


## **1. Pin Description**

**SERIAL CLOCK (SCL):** The SCL input is used to positive edge clock data into each EEPROM device and negative edge clock data out of each device.

**SERIAL DATA (SDA):** The SDA pin is bidirectional for serial data transfer. This pin is opendrain driven and may be wire-ORed with any number of other open-drain or open-collector devices.

**DEVICE/PAGE ADDRESSES (A2, A1, A0):** The A2, A1 and A0 pins are device address inputs that are hard wired for the AT24C02B. As many as eight 2K devices may be addressed on a single bus system (device addressing is discussed in detail under the Device Addressing section).

**WRITE PROTECT (WP):** The AT24C02B has a write protect pin that provides hardware data protection. The write protect pin allows normal read/write operations when connected to ground

**AT24C02B**

**2**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

(GND). When the write protect pin is connected to VCC, the write protection feature is enabled and operates as shown in Table 1-1.

**Table 1-1. Write Protect**

|**Table 1-1.**|**Write Protect**|
|---|---|
|**WP Pin**<br>**Status**|**Part of the Array Protected**|
||**24C02B**|
|At VCC|Full (2K) Array|
|At GND|Normal Read/Write Operations|



## **2. Memory Organization**

**AT24C02B, 2K SERIAL EEPROM:** Internally organized with 32 pages of 8 bytes each, the 2K requires an 8-bit data word address for random word addressing.

**3**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **Table 2-1.** Pin Capacitance[(1)]

Applicable over recommended operating range from TA = 25  C, f = 1.0 MHz, VCC = +1.8V

|**Symbol**|**Test Condition**|**Max**|**Units**|**Conditions**|
|---|---|---|---|---|
|CI/O|Input/Output Capacitance (SDA)|8|pF|VI/O= 0V|
|CIN|Input Capacitance (A0, A1, A2, SCL)|6|pF|VIN= 0V|



Note: 1. This parameter is characterized and is not 100% tested.

**Table 2-2.** DC Characteristics

Applicable over recommended operating range from: TAI = –40  C to +85  C, VCC = +1.8V to +5.5V, VCC = +1.8V to +5.5V (unless otherwise noted)

|**Symbol**|**Parameter**|**Test Condition**|**Min**|**Typ**|**Max**|**Units**|
|---|---|---|---|---|---|---|
|VCC1|Supply Voltage||1.8||5.5|V|
|VCC2|Supply Voltage||2.5||5.5|V|
|VCC3|Supply Voltage||2.7||5.5|V|
|VCC4|Supply Voltage||4.5||5.5|V|
|ICC|Supply Current VCC= 5.0V|READ at 100 kHz||0.4|1.0|mA|
|ICC|Supply Current VCC= 5.0V|WRITE at 100 kHz||2.0|3.0|mA|
|ISB1|Standby Current VCC= 1.8V|VIN= VCCor VSS||0.6|3.0|µA|
|ISB2|Standby Current VCC= 2.5V|VIN= VCCor VSS||1.4|4.0|µA|
|ISB3|Standby Current VCC= 2.7V|VIN= VCCor VSS||1.6|4.0|µA|
|ISB4|Standby Current VCC= 5.0V|VIN= VCCor VSS||8.0|18.0|µA|
|ILI|Input Leakage Current|VIN= VCCor VSS||0.10|3.0|µA|
|ILO|Output Leakage Current|VOUT= VCCor VSS||0.05|3.0|µA|
|VIL|Input Low Level(1)||–0.6||VCCx 0.3|V|
|VIH|Input High Level(1)||VCCx 0.7||VCC+ 0.5|V|
|VOL2|Output Low Level VCC= 3.0V|IOL= 2.1 mA|||0.4|V|
|VOL1|Output Low Level VCC= 1.8V|IOL= 0.15 mA|||0.2|V|



Note: 1. VIL min and VIH max are reference only and are not tested.

## **Table 2-3.** AC Characteristics

Applicable over recommended operating range from TAI = –40  C to +85  C, VCC = +1.8V to +5.5V, CL = 1 TTL Gate and 100 pF (unless otherwise noted)

|**Symbol**|**Parameter**|**1.8, 2.5, 2.7**|**1.8, 2.5, 2.7**|**5.0-volt**|**5.0-volt**|**Units**<br>kHz<br>µs<br>µs<br>ns<br>µs|
|---|---|---|---|---|---|---|
|||**Min**|**Max**|**Min**|**Max**||
|fSCL|Clock Frequency, SCL||400||1000||
|tLOW|Clock Pulse Width Low|1.2||0.4|||
|tHIGH|Clock Pulse Width High|0.6||0.4|||
|tI|Noise Suppression Time||50||40||
|tAA|Clock Low to Data Out Valid|0.1|0.9|0.05|0.55||



**AT24C02B**

**4**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **Table 2-3.** AC Characteristics

Applicable over recommended operating range from TAI = –40  C to +85  C, VCC = +1.8V to +5.5V, CL = 1 TTL Gate and 100 pF (unless otherwise noted)

|**Symbol**|**Parameter**|**1.8, 2.5, 2.7**|**1.8, 2.5, 2.7**|**5.0-volt**|**5.0-volt**|**Units**|
|---|---|---|---|---|---|---|
|||**Min**|**Max**|**Min**|**Max**||
|tBUF|Time the bus must be free before a new transmission can start|1.2||0.5||µs|
|tHD.STA|Start Hold Time|0.6||0.25||µs|
|tSU.STA|Start Setup Time|0.6||0.25||µs|
|tHD.DAT|Data In Hold Time|0||0||µs|
|tSU.DAT|Data In Setup Time|100||100||ns|
|tR|Inputs Rise Time(1)||0.3||0.3|µs|
|tF|Inputs Fall Time(1)||300||100|ns|
|tSU.STO|Stop Setup Time|0.6||.25||µs|
|tDH|Data Out Hold Time|50||50||ns|
|tWR|Write Cycle Time||5||5|ms|
|Endurance(1)|5.0V, 25C, Byte Mode|1 Million||||Write<br>Cycles|



Note: 1. This parameter is ensured by characterization only.

**5**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **3. Device Operation**

**CLOCK and DATA TRANSITIONS:** The SDA pin is normally pulled high with an external device. Data on the SDA pin may change only during SCL low time periods (see Figure 5-2 on page 8). Data changes during SCL high periods will indicate a start or stop condition as defined below.

**START CONDITION:** A high-to-low transition of SDA with SCL high is a start condition which must precede any other command (see Figure 5-3 on page 8).

**STOP CONDITION:** A low-to-high transition of SDA with SCL high is a stop condition. After a read sequence, the stop command will place the EEPROM in a standby power mode (see Figure 5-3 on page 8).

**ACKNOWLEDGE:** All addresses and data words are serially transmitted to and from the EEPROM in 8-bit words. The EEPROM sends a zero to acknowledge that it has received each word. This happens during the ninth clock cycle.

**STANDBY MODE:** The AT24C02B features a low-power standby mode which is enabled: (a) upon power-up and (b) after the receipt of the STOP bit and the completion of any internal operations.

**2-Wire Software Reset:** After an interruption in protocol, power loss or system reset, any 2-wire part can be reset by following these steps: (a) Create a start bit condition, (b) clock 9 cycles, (c) create another start bit followed by stop bit condition as shown below. The device is ready for next communication after above steps have been completed.

## **Figure 3-1.** Software reset

**==> picture [476 x 122] intentionally omitted <==**

**----- Start of picture text -----**<br>
Start bit Dummy Clock Cycles Start bit Stop b<br>SCL 1 2 3 8 9<br>SDA<br>**----- End of picture text -----**<br>


**AT24C02B**

**6**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **4. Bus Timing**

**Figure 4-1.** SCL: Serial Clock, SDA: Serial Data I/O[®]

**==> picture [475 x 420] intentionally omitted <==**

**----- Start of picture text -----**<br>
tF tHIGH tR<br>SCL tLOW tLOW<br>tSU.STA tHD.STA tHD.DAT tSU.DAT tSU.STO<br>SDA IN<br>tAA tDH tBUF<br>SDA OUT<br>Write Cycle Timing<br>SCL: Serial Clock, SDA: Serial Data I/O<br>SCL<br>SDA 8th BIT ACK<br>WORDn<br>(1)<br>t<br>wr<br>STOP START<br>CONDITION CONDITION<br>**----- End of picture text -----**<br>


## **5. Write Cycle Timing**

**Figure 5-1.** SCL: Serial Clock, SDA: Serial Data I/O

Note: 1. The write cycle time tWR is the time from a valid stop condition of a write sequence to the end of the internal clear/write cycle.

**7**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **Figure 5-2.** Data Validity

**==> picture [264 x 147] intentionally omitted <==**

**----- Start of picture text -----**<br>
SDA<br>SCL<br>DATA STABLE DATA STABLE<br>DATA<br>CHANGE<br>**----- End of picture text -----**<br>


**Figure 5-3.** Start and Stop Definition

**==> picture [313 x 366] intentionally omitted <==**

**----- Start of picture text -----**<br>
SDA<br>SCL<br>START STOP<br>SCL 1 8 9<br>DATA IN<br>DATA OUT<br>START ACKNOWLEDGE<br>**----- End of picture text -----**<br>


**Figure 5-4.** Output Acknowledge

**AT24C02B**

**8**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **6. Device Addressing**

The 2K EEPROM device requires an 8-bit device address word following a start condition to enable the chip for a read or write operation (refer to Figure 8-1).

The device address word consists of a mandatory one, zero sequence for the first four most significant bits as shown. This is common to all the EEPROM devices.

The next 3 bits are the A2, A1 and A0 device address bits for the 2K EEPROM. These 3 bits must compare to their corresponding hard-wired input pins.

The eighth bit of the device address is the read/write operation select bit. A read operation is initiated if this bit is high and a write operation is initiated if this bit is low.

Upon a compare of the device address, the EEPROM will output a zero. If a compare is not made, the chip will return to a standby state.

## **7. Write Operations**

**BYTE WRITE:** A write operation requires an 8-bit data word address following the device address word and acknowledgment. Upon receipt of this address, the EEPROM will again respond with a zero and then clock in the first 8-bit data word. Following receipt of the 8-bit data word, the EEPROM will output a zero and the addressing device, such as a microcontroller, must terminate the write sequence with a stop condition. At this time the EEPROM enters an internally timed write cycle, tWR, to the nonvolatile memory. All inputs are disabled during this write cycle and the EEPROM will not respond until the write is complete (see Figure 8-2 on page 11).

**PAGE WRITE:** The 2K EEPROM is capable of an 8-byte page write.

A page write is initiated the same as a byte write, but the microcontroller does not send a stop condition after the first data word is clocked in. Instead, after the EEPROM acknowledges receipt of the first data word, the microcontroller can transmit up to seven data words. The EEPROM will respond with a zero after each data word received. The microcontroller must terminate the page write sequence with a stop condition (see Figure 8-3 on page 11).

The data word address lower three bits are internally incremented following the receipt of each data word. The higher data word address bits are not incremented, retaining the memory page row location. When the word address, internally generated, reaches the page boundary, the following byte is placed at the beginning of the same page. If more than eight data words are transmitted to the EEPROM, the data word address will “roll over” and previous data will be overwritten.

**ACKNOWLEDGE POLLING:** Once the internally timed write cycle has started and the EEPROM inputs are disabled, acknowledge polling can be initiated. This involves sending a start condition followed by the device address word. The read/write bit is representative of the operation desired. Only if the internal write cycle has completed will the EEPROM respond with a zero allowing the read or write sequence to continue.

**9**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **8. Read Operations**

Read operations are initiated the same way as write operations with the exception that the read/write select bit in the device address word is set to one. There are three read operations: current address read, random address read and sequential read.

**CURRENT ADDRESS READ:** The internal data word address counter maintains the last address accessed during the last read or write operation, incremented by one. This address stays valid between operations as long as the chip power is maintained. The address “roll over” during read is from the last byte of the last memory page to the first byte of the first page. The address “roll over” during write is from the last byte of the current page to the first byte of the same page.

Once the device address with the read/write select bit set to one is clocked in and acknowledged by the EEPROM, the current address data word is serially clocked out. The microcontroller does not respond with an input zero but does generate a following stop condition (see Figure 8-4 on page 11).

**RANDOM READ:** A random read requires a “dummy” byte write sequence to load in the data word address. Once the device address word and data word address are clocked in and acknowledged by the EEPROM, the microcontroller must generate another start condition. The microcontroller now initiates a current address read by sending a device address with the read/write select bit high. The EEPROM acknowledges the device address and serially clocks out the data word. The microcontroller does not respond with a zero but does generate a following stop condition (see Figure 8-5 on page 12).

**SEQUENTIAL READ:** Sequential reads are initiated by either a current address read or a random address read. After the microcontroller receives a data word, it responds with an acknowledge. As long as the EEPROM receives an acknowledge, it will continue to increment the data word address and serially clock out sequential data words. When the memory address limit is reached, the data word address will “roll over” and the sequential read will continue. The sequential read operation is terminated when the microcontroller does not respond with a zero but does generate a following stop condition (see Figure 8-6 on page 12).

**Figure 8-1.** Device Address

**==> picture [158 x 33] intentionally omitted <==**

**----- Start of picture text -----**<br>
MSB LSB<br>**----- End of picture text -----**<br>


**AT24C02B**

**10**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

**Figure 8-2.** Byte Write

**==> picture [42 x 71] intentionally omitted <==**

**==> picture [99 x 48] intentionally omitted <==**

**Figure 8-3.** Page Write

**==> picture [42 x 69] intentionally omitted <==**

**==> picture [169 x 40] intentionally omitted <==**

**Figure 8-4.** Current Address Read

**==> picture [42 x 70] intentionally omitted <==**

**11**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **Figure 8-5.** Random Read

**==> picture [170 x 97] intentionally omitted <==**

## **Figure 8-6.** Sequential Read

**==> picture [51 x 32] intentionally omitted <==**

**AT24C02B**

**12**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **AT24C02B Ordering Information**

|**AT24C02B Ordering Information**||||
|---|---|---|---|
|**Ordering Code**|**Voltage**|**Package**|**Operation Range**|
|AT24C02B-PU (Bulk form only)|1.8|8P3|Lead-free/Halogen-free/<br>Industrial Temperature<br>(–40C to 85C)|
|AT24C02BN-SH-B(1) (NiPdAu Lead Finish)|1.8|8S1||
|AT24C02BN-SH-T(2) (NiPdAu Lead Finish)|1.8|8S1||
|AT24C02B-TH-B(1) (NiPdAu Lead Finish)|1.8|8A2||
|AT24C02B-TH-T(2) (NiPdAu Lead Finish)|1.8|8A2||
|AT24C02BY6-YH-T(2) (NiPdAu Lead Finish)|1.8|8Y6||
|AT24C02B-TSU-T(2)|1.8|5TS1||
|AT24C02BU3-UU-T(2)|1.8|8U3-1||
|AT24C02B-W-11(3)|1.8|Die Sale|Industrial Temperature<br>(–40C to 85C)|



Notes: 1. “-B” denotes bulk.

2. “-T” denotes tape and reel. SOIC = 4K per reel. TSSOP, Ultra Thin Mini-MAP, SOT23, and dBGA2 = 5K per reel.

3. Available in tape and reel and wafer form; order as SL788 for inkless wafer form. Please contact Serial Interface Marketing.

|||
|---|---|
|**Package Type**||
|**8P3**|8-lead, 0.300" Wide, Plastic Dual Inline Package (PDIP)|
|**8S1**|8-lead, 0.150" Wide, Plastic Gull Wing Small Outline (JEDEC SOIC)|
|**8A2**|8-lead, 4.4 mm Body, Plastic Thin Shrink Small Outline Package (TSSOP)|
|**8Y6**|8-lead, 2.00 mm x 3.00 mm Body, 0.50 mm Pitch, Ultra Thin Mini-MAP, Dual No Lead Package (DFN), (MLP 2x3 mm)|
|**5TS1**|5-lead, 2.90 mm x 1.60 mm Body, Plastic Thin Shrink Small Outline Package (SOT23)|
|**8U3-1**|8-ball, die Ball Grid Array Package (dBGA2)|
|**Options**||
|–**1.8**|Low-voltage (1.8V to 5.5V)|



**13**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **9. Part Marking Scheme**

## **8-PDIP**

## Seal Year

TOP MARK                 |  Seal Week |   |   | |---|---|---|---|---|---|---|---| A   T   M   L   U   Y   W   W |---|---|---|---|---|---|---|---| 0   2   B       1 |---|---|---|---|---|---|---|---| *   Lot Number |---|---|---|---|---|---|---|---| | Pin 1 Indicator (Dot)

U = Material Set

Y = Seal Year

WW = Seal Week

02B = Device

1 = Voltage Indicator

*Lot Number to Use ALL Characters in Marking

BOTTOM MARK No Bottom Mark

**AT24C02B**

**14**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **8-SOIC**

## Seal Year

TOP MARK                 |  Seal Week |   |   | |---|---|---|---|---|---|---|---| A   T   M   L   H   Y   W   W |---|---|---|---|---|---|---|---| 0   2   B       1 |---|---|---|---|---|---|---|---| *   Lot Number |---|---|---|---|---|---|---|---| | Pin 1 Indicator (Dot)

H = Material Set

Y = Seal Year

WW = Seal Week

02B = Device

1 = Voltage Indicator

*Lot Number to Use ALL Characters in Marking

BOTTOM MARK

No Bottom Mark

**15**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **8-TSSOP**

## TOP MARK

Pin 1 Indicator (Dot) | |---|---|---|---| * H Y W W |---|---|---|---|---| 0 2 B 1 |---|---|---|---|---| H = Material Set Y = Seal Year WW = Seal Week 02B = Device

V = Voltage Indicator

BOTTOM MARK

|---|---|---|---|---|---|---| X   X |---|---|---|---|---|---|---| A   A   A   A   A   A   A |---|---|---|---|---|---|---| <- Pin 1 Indicator Lot Number

XX = Country of Origin

AAAAAA = Lot Number

**AT24C02B**

**16**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **SOT23**

TOP MARK |---|---|---|---|---| Line 1 ----------->    2   B   1   W   U |---|---|---|---|---| * |

2B = Device

1 = Voltage Indicator

W = Write Protect Feature

U = Material Set

Pin 1 Indicator (Dot)

BOTTOM MARK

|---|---|---|---| Y   M   T   C |---|---|---|---|

## Y = One Digit Year Code

M = Seal Month

TC = Trace Code

## **ULTRA THIN MINI MAP**

TOP MARK |---|---|---| 0   2   B |---|---|---| H       1 |---|---|---| Y   T   C |---|---|---| * |

02B = Device

H = Material Set

1 = Voltage Indicator

Y = Year of Assembly

TC = Trace Code

Pin 1 Indicator (Dot)

**17**

5126H–SEEPR–8/07

**dBGA2**

**==> picture [301 x 34] intentionally omitted <==**

TOP MARK LINE 1------->    02BU LINE 2------->    YMTC |<--  Pin 1 This Corner 02B = Device U = Material Set Y = One Digit Year Code M = Seal Month TC = Trace Code

**AT24C02B**

**18**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **10. Packaging Information**

## **8P3 – PDIP**

**==> picture [84 x 120] intentionally omitted <==**

**----- Start of picture text -----**<br>
1<br>N<br>Top View<br>**----- End of picture text -----**<br>


**==> picture [99 x 171] intentionally omitted <==**

**----- Start of picture text -----**<br>
E<br>E1<br>c<br>eA<br>End View<br>**----- End of picture text -----**<br>


|D1<br>b3<br>4 PLCS||||D|e|e|e|L<br>~~b~~2<br>b<br>A2 A<br>|**COMMON DIMENSIONS**<br>(Unit of Measure = inches)<br>**SYMBOL**<br>**MIN**<br>**NOM**<br>**MAX**<br>**NOTE**<br>A<br>–<br>–<br>0.210<br>2<br>A2<br>0.115<br>0.130<br>0.195<br>b<br>0.014<br>0.018<br>0.022<br>5<br>b2<br>0.045<br>0.060<br>0.070<br>6<br>b3<br>0.030<br>0.039<br>0.045<br>6<br>c<br>0.008<br>0.010<br>0.014<br>D<br>0.355<br>0.365<br>0.400<br>3<br>D1<br>0.005<br>–<br>–<br>3<br>E<br>0.300<br>0.310<br>0.325<br>4<br>E1<br>0.240<br>0.250<br>0.280<br>3<br>e<br>0.100 BSC<br>eA<br>0.300 BSC<br>4<br>L<br>0.115<br>0.130<br>0.150<br>2|**COMMON DIMENSIONS**<br>(Unit of Measure = inches)<br>**SYMBOL**<br>**MIN**<br>**NOM**<br>**MAX**<br>**NOTE**<br>A<br>–<br>–<br>0.210<br>2<br>A2<br>0.115<br>0.130<br>0.195<br>b<br>0.014<br>0.018<br>0.022<br>5<br>b2<br>0.045<br>0.060<br>0.070<br>6<br>b3<br>0.030<br>0.039<br>0.045<br>6<br>c<br>0.008<br>0.010<br>0.014<br>D<br>0.355<br>0.365<br>0.400<br>3<br>D1<br>0.005<br>–<br>–<br>3<br>E<br>0.300<br>0.310<br>0.325<br>4<br>E1<br>0.240<br>0.250<br>0.280<br>3<br>e<br>0.100 BSC<br>eA<br>0.300 BSC<br>4<br>L<br>0.115<br>0.130<br>0.150<br>2|**COMMON DIMENSIONS**<br>(Unit of Measure = inches)<br>**SYMBOL**<br>**MIN**<br>**NOM**<br>**MAX**<br>**NOTE**<br>A<br>–<br>–<br>0.210<br>2<br>A2<br>0.115<br>0.130<br>0.195<br>b<br>0.014<br>0.018<br>0.022<br>5<br>b2<br>0.045<br>0.060<br>0.070<br>6<br>b3<br>0.030<br>0.039<br>0.045<br>6<br>c<br>0.008<br>0.010<br>0.014<br>D<br>0.355<br>0.365<br>0.400<br>3<br>D1<br>0.005<br>–<br>–<br>3<br>E<br>0.300<br>0.310<br>0.325<br>4<br>E1<br>0.240<br>0.250<br>0.280<br>3<br>e<br>0.100 BSC<br>eA<br>0.300 BSC<br>4<br>L<br>0.115<br>0.130<br>0.150<br>2|**COMMON DIMENSIONS**<br>(Unit of Measure = inches)<br>**SYMBOL**<br>**MIN**<br>**NOM**<br>**MAX**<br>**NOTE**<br>A<br>–<br>–<br>0.210<br>2<br>A2<br>0.115<br>0.130<br>0.195<br>b<br>0.014<br>0.018<br>0.022<br>5<br>b2<br>0.045<br>0.060<br>0.070<br>6<br>b3<br>0.030<br>0.039<br>0.045<br>6<br>c<br>0.008<br>0.010<br>0.014<br>D<br>0.355<br>0.365<br>0.400<br>3<br>D1<br>0.005<br>–<br>–<br>3<br>E<br>0.300<br>0.310<br>0.325<br>4<br>E1<br>0.240<br>0.250<br>0.280<br>3<br>e<br>0.100 BSC<br>eA<br>0.300 BSC<br>4<br>L<br>0.115<br>0.130<br>0.150<br>2|**COMMON DIMENSIONS**<br>(Unit of Measure = inches)<br>**SYMBOL**<br>**MIN**<br>**NOM**<br>**MAX**<br>**NOTE**<br>A<br>–<br>–<br>0.210<br>2<br>A2<br>0.115<br>0.130<br>0.195<br>b<br>0.014<br>0.018<br>0.022<br>5<br>b2<br>0.045<br>0.060<br>0.070<br>6<br>b3<br>0.030<br>0.039<br>0.045<br>6<br>c<br>0.008<br>0.010<br>0.014<br>D<br>0.355<br>0.365<br>0.400<br>3<br>D1<br>0.005<br>–<br>–<br>3<br>E<br>0.300<br>0.310<br>0.325<br>4<br>E1<br>0.240<br>0.250<br>0.280<br>3<br>e<br>0.100 BSC<br>eA<br>0.300 BSC<br>4<br>L<br>0.115<br>0.130<br>0.150<br>2|
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| | | | | | | |
|||||||e||||||||
| | | | | | | |
|||||||||||**MIN**|**NOM**|**MAX**|**NOTE**|
| | | | | | | |
|||||||||||–|–|0.210|2|
|||||||||||0.115|0.130|0.195||
||||Side View|||||||0.014|0.018|0.022|5|
|||||||||||0.045|0.060|0.070|6|
|||||||||||0.030|0.039|0.045|6|
|||||||||||0.008|0.010|0.014||
|||||||||||0.355|0.365|0.400|3|
|||||||||||0.005|–|–|3|
| | | | | | | |
|||||||||||0.300|0.310|0.325|4|
|||||||||||0.240|0.250|0.280|3|
|||||||||||0.100 BSC||||
|||||||||||0.300 BSC|||4|
|||||||||||0.115|0.130|0.150|2|



- Notes: 1. This drawing is for general information only; refer to JEDEC Drawing MS-001, Variation BA, for additional information.

   2. Dimensions A and L are measured with the package seated in JEDEC seating plane Gauge GS-3.

   3. D, D1 and E1 dimensions do not include mold Flash or protrusions. Mold Flash or protrusions shall not exceed 0.010 inch.

   4. E and eA measured with the leads constrained to be perpendicular to datum.

   5. Pointed or rounded lead tips are preferred to ease insertion.

   6. b2 and b3 maximum dimensions do not include Dambar protrusions. Dambar protrusions shall not exceed 0.010 (0.25 mm).

01/09/02 **TITLE DRAWING NO. REV.** 2325 Orchard Parkway **8P3** , 8-lead, 0.300" Wide Body, Plastic Dual 8P3 B R San Jose, CA  95131 In-line Package (PDIP)

**19**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **8S1 – JEDEC SOIC**

**==> picture [301 x 261] intentionally omitted <==**

**----- Start of picture text -----**<br>
C<br>1<br>E E1<br>L<br>N<br>∅<br>Top View<br>End View<br>**----- End of picture text -----**<br>


|e|||||||||||B|B||||||||||
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|||||||||||||||A||||**COMMON DIMENSIONS**<br>(Unit of Measure = mm)||||
|||||||||||||||||||**SYMBOL**<br>**MIN**|**NOM**|**MAX**|**NOTE**|
|||||||||||||||||A1||A<br>1.35|–|1.75||
|||||||||||||||||||A1<br>0.10|–|0.25||
| | | | | | | |
|||||||||||||||||||b<br>0.31|–|0.51||
|||||||||||||||||||C<br>0.17|–|0.25||
||||||||D|||||||||||D<br>4.80|–|5.00||
|||||||||||||||||||E1<br>3.81|–|3.99||
|||||Side View||||||||||||||E<br>5.79<br>e|–<br>1.27 BSC|6.20||
|||||||||||||||||||L<br>0.40|–|1.27||
|||||||||||||||||||∅<br>0˚|–|8˚||



Note: These drawings are for general information only. Refer to JEDEC Drawing MS-012, Variation AA for proper dimensions, tolerances, datums, etc.

10/7/03 **TITLE DRAWING NO. REV.** 1150 E. Cheyenne Mtn. Blvd. **8S1** , 8-lead (0.150" Wide Body), Plastic Gull Wing 8S1 B R Colorado Springs, CO  80906 Small Outline (JEDEC SOIC)

**AT24C02B**

**20**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **8A2 – TSSOP**

**==> picture [330 x 221] intentionally omitted <==**

**----- Start of picture text -----**<br>
3 2 1<br>Pin 1 indicator<br>this corner<br>E1 E<br>L1<br>N<br>L<br>**----- End of picture text -----**<br>


## Top View

## End View

## **COMMON DIMENSIONS**

(Unit of Measure = mm)

**==> picture [130 x 88] intentionally omitted <==**

**----- Start of picture text -----**<br>
A<br>b<br>e A2<br>D<br>**----- End of picture text -----**<br>


## Side View

|**SYMBOL**|**MIN**|**NOM**|**MAX**|**NOTE**|
|---|---|---|---|---|
|D|2.90|3.00|3.10|2, 5|
|E|6.40 BSC||||
|E1|4.30|4.40|4.50|3, 5|
|A|–|–|1.20||
|A2|0.80|1.00|1.05||
|b|0.19|–|0.30|4|
|e|0.65 BSC||||
|L|0.45|0.60|0.75||
|L1|1.00 REF||||



- Notes: 1. This drawing is for general information only. Refer to JEDEC Drawing MO-153, Variation AA, for proper dimensions, tolerances, datums, etc.

   2. Dimension D does not include mold Flash, protrusions or gate burrs. Mold Flash, protrusions and gate burrs shall not exceed 0.15 mm (0.006 in) per side.

   3. Dimension E1 does not include inter-lead Flash or protrusions. Inter-lead Flash and protrusions shall not exceed 0.25 mm (0.010 in) per side.

   4. Dimension b does not include Dambar protrusion. Allowable Dambar protrusion shall be 0.08 mm total in excess of the b dimension at maximum material condition. Dambar cannot be located on the lower radius of the foot. Minimum space between protrusion and ad acent lead is 0.07 mm.

   5. Dimension D and E1 to be determined at Datum Plane H.

|protrusion and ad acent lead is 0.07 mm.<br>||
|---|---|
|5. Dimension D and E1 to be determined at Datum Plane H.|5/30/02|
|2325 Orchard Parkway<br>San Jose, CA 95131<br>**TITLE**<br>R<br>**8A2**, 8-lead, 4.4 mm Body, Plastic<br>Thin Shrink Small Outline Package (TSSOP)|**DRAWING NO.**<br>**REV.**<br>8A2<br>B|



**21**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **8Y6 - Mini Map**

|10/16/07<br>Notes:<br>1.   This drawing is for general information only. Refer to JEDEC Drawing MO-229, for proper dimensions,<br>tolerances, datums, etc.<br>2.   Dimension b applies to metallized terminal and is measured between 0.15 mm and 0.30 mm from the terminal tip. If the<br>terminal has the optional radius on the other end of the terminal, the dimension should not be measured in that radius area.<br>3.    Soldering the large thermal pad is optional, but not recommended.  No electrical connection is accomplished to the<br>device through this pad, so if soldered it should be tied to ground<br>**COMMON DIMENSIONS**<br>(Unit of Measure = mm)<br>**SYMBOL**<br>**MIN**<br>**NOM**<br>**MAX**<br>**NOTE**<br>D<br>2.00 BSC<br>E<br>3.00 BSC<br>D2<br>1.40<br>1.50<br>1.60<br>E2<br>-<br>-<br>1.40<br>A<br>-<br>-<br>0.60<br>A1<br>0.0<br>0.02<br>0.05<br>A2<br>-<br>-<br>0.55<br>A3<br>0.20 REF<br>L<br>0.20<br>0.30<br>0.40<br>e<br>0.50 BSC<br>b<br>0.20<br>0.25<br>0.30<br>2<br>A2<br>b<br>(8X)<br>Pin 1 ID<br>Pin 1<br>Index<br>Area<br>A1<br>A3<br>D<br>E<br>A<br>L (8X)<br>e (6X)<br>1.50 REF.<br>D2<br>E2|10/16/07<br>Notes:<br>1.   This drawing is for general information only. Refer to JEDEC Drawing MO-229, for proper dimensions,<br>tolerances, datums, etc.<br>2.   Dimension b applies to metallized terminal and is measured between 0.15 mm and 0.30 mm from the terminal tip. If the<br>terminal has the optional radius on the other end of the terminal, the dimension should not be measured in that radius area.<br>3.    Soldering the large thermal pad is optional, but not recommended.  No electrical connection is accomplished to the<br>device through this pad, so if soldered it should be tied to ground<br>**COMMON DIMENSIONS**<br>(Unit of Measure = mm)<br>**SYMBOL**<br>**MIN**<br>**NOM**<br>**MAX**<br>**NOTE**<br>D<br>2.00 BSC<br>E<br>3.00 BSC<br>D2<br>1.40<br>1.50<br>1.60<br>E2<br>-<br>-<br>1.40<br>A<br>-<br>-<br>0.60<br>A1<br>0.0<br>0.02<br>0.05<br>A2<br>-<br>-<br>0.55<br>A3<br>0.20 REF<br>L<br>0.20<br>0.30<br>0.40<br>e<br>0.50 BSC<br>b<br>0.20<br>0.25<br>0.30<br>2<br>A2<br>b<br>(8X)<br>Pin 1 ID<br>Pin 1<br>Index<br>Area<br>A1<br>A3<br>D<br>E<br>A<br>L (8X)<br>e (6X)<br>1.50 REF.<br>D2<br>E2|10/16/07<br>Notes:<br>1.   This drawing is for general information only. Refer to JEDEC Drawing MO-229, for proper dimensions,<br>tolerances, datums, etc.<br>2.   Dimension b applies to metallized terminal and is measured between 0.15 mm and 0.30 mm from the terminal tip. If the<br>terminal has the optional radius on the other end of the terminal, the dimension should not be measured in that radius area.<br>3.    Soldering the large thermal pad is optional, but not recommended.  No electrical connection is accomplished to the<br>device through this pad, so if soldered it should be tied to ground<br>**COMMON DIMENSIONS**<br>(Unit of Measure = mm)<br>**SYMBOL**<br>**MIN**<br>**NOM**<br>**MAX**<br>**NOTE**<br>D<br>2.00 BSC<br>E<br>3.00 BSC<br>D2<br>1.40<br>1.50<br>1.60<br>E2<br>-<br>-<br>1.40<br>A<br>-<br>-<br>0.60<br>A1<br>0.0<br>0.02<br>0.05<br>A2<br>-<br>-<br>0.55<br>A3<br>0.20 REF<br>L<br>0.20<br>0.30<br>0.40<br>e<br>0.50 BSC<br>b<br>0.20<br>0.25<br>0.30<br>2<br>A2<br>b<br>(8X)<br>Pin 1 ID<br>Pin 1<br>Index<br>Area<br>A1<br>A3<br>D<br>E<br>A<br>L (8X)<br>e (6X)<br>1.50 REF.<br>D2<br>E2|10/16/07<br>Notes:<br>1.   This drawing is for general information only. Refer to JEDEC Drawing MO-229, for proper dimensions,<br>tolerances, datums, etc.<br>2.   Dimension b applies to metallized terminal and is measured between 0.15 mm and 0.30 mm from the terminal tip. If the<br>terminal has the optional radius on the other end of the terminal, the dimension should not be measured in that radius area.<br>3.    Soldering the large thermal pad is optional, but not recommended.  No electrical connection is accomplished to the<br>device through this pad, so if soldered it should be tied to ground<br>**COMMON DIMENSIONS**<br>(Unit of Measure = mm)<br>**SYMBOL**<br>**MIN**<br>**NOM**<br>**MAX**<br>**NOTE**<br>D<br>2.00 BSC<br>E<br>3.00 BSC<br>D2<br>1.40<br>1.50<br>1.60<br>E2<br>-<br>-<br>1.40<br>A<br>-<br>-<br>0.60<br>A1<br>0.0<br>0.02<br>0.05<br>A2<br>-<br>-<br>0.55<br>A3<br>0.20 REF<br>L<br>0.20<br>0.30<br>0.40<br>e<br>0.50 BSC<br>b<br>0.20<br>0.25<br>0.30<br>2<br>A2<br>b<br>(8X)<br>Pin 1 ID<br>Pin 1<br>Index<br>Area<br>A1<br>A3<br>D<br>E<br>A<br>L (8X)<br>e (6X)<br>1.50 REF.<br>D2<br>E2|
|---|---|---|---|
|2325 Orchard Parkway<br>San Jose, CA  95131<br>R|**TITLE**<br>**8Y6**, 8-lead 2.0 x 3.0 mm Body, 0.50 mm Pitch, Utlra Thin Mini-Map,<br>Dual No Lead Package (DFN) ,(MLP 2x3)|**DRAWING NO.**<br> <br>8Y6|**REV.**<br>D|



- Notes: 1.   This drawing is for general information only. Refer to JEDEC Drawing MO-229, for proper dimensions, tolerances, datums, etc.

2.   Dimension b applies to metallized terminal and is measured between 0.15 mm and 0.30 mm from the terminal tip. If the terminal has the optional radius on the other end of the terminal, the dimension should not be measured in that radius area.

3.    Soldering the large thermal pad is optional, but not recommended.  No electrical connection is accomplished to the device through this pad, so if soldered it should be tied to ground

10/16/07 **TITLE DRAWING NO. REV.** 2325 Orchard Parkway R San Jose, CA  95131 **8Y6** , 8-lead 2.0 x 3.0 mm Body, 0.50 mm Pitch, Utlra Thin Mini-Map, 8Y6 D Dual No Lead Package (DFN) ,(MLP 2x3)

**AT24C02B**

**22**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **5TS1 – SOT23**

**==> picture [531 x 624] intentionally omitted <==**

**----- Start of picture text -----**<br>
e1<br>5 4 C<br>E1 E C<br>L<br>L1<br>1 2 3<br>Top View End View<br>b<br>A2 A<br>Seating<br>Plane A1<br>e<br>D<br>Side View COMMON DIMENSIONS<br>(Unit of Measure = mm)<br>NOTES: 1. This drawing is for general information only. Refer to JEDEC Drawing SYMBOL MIN NOM MAX NOTE<br>MO-193, Variation AB, for additional information. A –  – 1.10<br>2. Dimension D does not include mold flash, protrusions, or gate burrs.<br>Mold flash, protrusions, or gate burrs shall not exceed 0.15 mm per end. A1 0.00 – 0.10<br>Dimension E1 does not include interlead flash or protrusion. Interlead A2 0.70 0.90 1.00<br>flash or protrusion shall not exceed 0.15 mm per side.<br>3. The package top may be smaller than the package bottom. Dimensions c 0.08  – 0.20 4<br>D and E1 are determined at the outermost extremes of the plastic body D 2.90 BSC 2, 3<br>exclusive of mold flash, tie bar burrs, gate burrs, and interlead flash, but<br>including any mismatch between the top and bottom of the plastic body. E 2.80 BSC 2, 3<br>4. These dimensions apply to the flat section of the lead between 0.08 mm E1 1.60 BSC 2, 3<br>and 0.15 mm from the lead tip.<br>5. Dimension "b" does not include Dambar protrusion. Allowable Dambar L1 0.60 REF<br>protrusion shall be 0.08 mm total in excess of the "b" dimension at e 0.95 BSC<br>maximum material condition. The Dambar cannot be located on the lower<br>radius of the foot. Minimum space between protrusion and an adjacent lead e1 1.90 BSC<br>shall not be less than 0.07 mm. b 0.30 – 0.50 4, 5<br>6/25/03<br>TITLE DRAWING NO. REV.<br>1150 E. Cheyenne Mtn. Blvd. 5TS1,  5-lead, 1.60 mm Body, Plastic Thin Shrink<br>R Colorado Springs, CO  80906 Small Outline Package (SHRINK SOT) PO5TS1            A<br>**----- End of picture text -----**<br>


**23**

5126H–SEEPR–8/07

**==> picture [301 x 34] intentionally omitted <==**

## **8U3-1 – dBGA2**

**==> picture [509 x 588] intentionally omitted <==**

**----- Start of picture text -----**<br>
E<br>D<br>1. b<br>A1<br>PIN 1 BALL PAD CORNER<br>A2<br>Top View<br>A<br>PIN 1 BALL PAD CORNER Side View<br>1 2 3 4<br>(d1)<br>d<br>8 7 6 5<br>e<br>COMMON DIMENSIONS<br>(Unit of Measure = mm)<br>(e1)<br>SYMBOL MIN NOM MAX NOTE<br>Bottom View A  0.71  0.81  0.91<br>8 SOLDER BALLS<br>A1  0.10  0.15  0.20<br>A2  0.40  0.45  0.50<br>b  0.20  0.25  0.30<br>D          1.50 BSC<br>1.   Dimension “b” is measured at the maximum solder ball diameter.<br>E          2.00 BSC<br>This drawing is for general information only.  e          0.50 BSC<br>e1          0.25 REF<br>d          1.00 BSC<br>d1          0.25 REF<br>6/24/03<br>TITLE DRAWING NO. REV.<br>1150 E. Cheyenne Mtn. Blvd. 8U3-1,  8-ball, 1.50 x 2.00 mm Body, 0.50 mm pitch,<br>R Colorado Springs, CO  80906 Small Die Ball Grid Array Package (dBGA2) PO8U3-1           A<br>**----- End of picture text -----**<br>


**AT24C02B**

**24**

5126H–SEEPR–8/07

**AT24C02B**

**==> picture [483 x 45] intentionally omitted <==**

## **11. Revision History**

|**Doc. Rev.**|**Date**|**Comments**|
|---|---|---|
|5126H|8/2007|Updated to new template<br>Updated common graphics<br>Added Part Makring Scheme|
|5126G|4/2007|Removed reference to Waffle Pack on page 1 and Page 13<br>Added lines to Ordering Code table<br>Removed NC row in the table<br>Added note on Pg 1<br>Corrected format on table 5<br>Removed Memory Reset section<br>Added 2-Wire software reset section and figure<br>Corrected Figures 7-11|
|5126F|2/2007|Corrected dBGA2 package code on Pg 13<br>Removed ‘Preliminary’|
|5126E|2/2007|Added Ultra-Thin on Pg 1<br>Modified Ordering Information table|
|5126D|7/2006|Implemented Revision History<br>Added Preliminary status; Added ‘Available in Automotive’ to<br>Features|



**25**

5126H–SEEPR–8/07

**==> picture [89 x 46] intentionally omitted <==**

## **Headquarters**

## **International**

_**Atmel Corporation**_ 2325 Orchard Parkway San Jose, CA 95131 USA

Tel: 1(408) 441-0311 Fax: 1(408) 487-2600

## _**Atmel Asia**_

_**Atmel Europe**_ Le Krebs

## _**Atmel Japan**_

Room 1219 Le Krebs 9F, Tonetsu Shinkawa Bldg. Chinachem Golden Plaza 8, Rue Jean-Pierre Timbaud 1-24-8 Shinkawa 77 Mody Road Tsimshatsui BP 309 Chuo-ku, Tokyo 104-0033 East Kowloon 78054 Saint-Quentin-enJapan Hong Kong Yvelines Cedex Tel: (81) 3-3523-3551 Tel: (852) 2721-9778 France Fax: (81) 3-3523-7581 Fax: (852) 2722-1369 Tel: (33) 1-30-60-70-00 Fax: (33) 1-30-60-71-11

## **Product Contact**

_**Web Site Technical Support**_ www.atmel.com s_eeprom@atmel.com

_**Sales Contact**_ www.atmel.com/contacts

## _**Literature Requests**_

www.atmel.com/literature

**Disclaimer:** The information in this document is provided in connection with Atmel products. No license, express or implied, by estoppel or otherwise, to any intellectual property right is granted by this document or in connection with the sale of Atmel products. **EXCEPT AS SET FORTH IN ATMEL’S TERMS AND CONDITIONS OF SALE LOCATED ON ATMEL’S WEB SITE, ATMEL ASSUMES NO LIABILITY WHATSOEVER AND DISCLAIMS ANY EXPRESS, IMPLIED OR STATUTORY WARRANTY RELATING TO ITS PRODUCTS INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT, CONSEQUENTIAL, PUNITIVE, SPECIAL OR INCIDENTAL DAMAGES (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS, BUSINESS INTERRUPTION, OR LOSS OF INFORMATION) ARISING OUT OF THE USE OR INABILITY TO USE THIS DOCUMENT, EVEN IF ATMEL HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.** Atmel makes no representations or warranties with respect to the accuracy or completeness of the contents of this document and reserves the right to make changes to specifications and product descriptions at any time without notice. Atmel does not make any commitment to update the information contained herein. Unless specifically provided otherwise, Atmel products are not suitable for, and shall not be used in, automotive applications. Atmel’s products are not intended, authorized, or warranted for use as components in applications intended to support or sustain life.

© 2007 Atmel Corporation. **All rights reserved.** Atmel[®] , logo and combinations thereof, and others, are registered trademarks or trademarks of Atmel Corporation or its subsidiaries. Other terms and product names may be trademarks of others.

5126H–SEEPR–8/07
