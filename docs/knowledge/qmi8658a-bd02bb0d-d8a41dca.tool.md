**==> picture [45 x 52] intentionally omitted <==**

**==> picture [25 x 23] intentionally omitted <==**

QMI8658A DATASHEET JUNE 20, 2022

Security Level: 3

**Document No.** ： **13-52-25 Title:  QMI8658A Datasheet**

**Rev: A**

**Originator: Chunlei Dai**

**==> picture [60 x 47] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

## **QMI8658A Low Noise, Wide Bandwidth 6D Inertial Measurement Unit with Motion Co-Processor**

## **Features**

- Low 13 mdps/ `√` Hz gyroscope noise, low-latency, and wide bandwidth

- Low Noise 150µg/ `√` Hz accelerometer noise

- Host (slave) interface supports MIPI™ I3C, I[2] C, and 3-wire or 4-wire SPI

- Accelerometer and gyroscope sensors feature signal processing paths with digitally programmable data rates and filtering

- 3-axis gyroscope and 3-axis accelerometer in a small 2.5 x 3.0 x 0.86 mm 14-pin LGA package

- Large 1536-byte FIFO can be used to buffer sensor data to lower system power dissipation

- Integrated Pedometer, Tap, Any-Motion, NoMotion, Significant-Motion detection

- Large sensor dynamic ranges from ±16°/s to ±2048°/s for gyroscope and ±2 g to ±16 g for accelerometer

- Low power modes for effective power management

- Digitally programmable sampling rate and filters

- Embedded temperature sensor

- Wide extended operating temperature range (-40°C to 85°C)

## **Description**

The QMI8658A is a complete 6D MEMS inertial measurement unit (IMU). With tight board-level gyroscope sensitivity of `±` 3%, gyroscope noise density of 13 mdps/ `√` Hz, and low latency, the QMI8658A is ideal for consumer and industrial applications.

The QMI8658A incorporates a 3-axis gyroscope and a 3- axis accelerometer. It provides a host-processor interface supporting I3C, I[2] C and 3-wire or 4-wire SPI.

With its built-in digital functionality, low power, and small size, the QMI8658A is the ideal part for applications requiring motion-based functionality.

## **Applications**

- Smartphones

- Game controllers, remote controls and pointing devices

- Robotic vacuums

- E-bikes and scooters

- Bluetooth headsets

- Automotive security systems

- Toys

- Portrait-landscape display control

**==> picture [197 x 166] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 1

**==> picture [506 x 686] intentionally omitted <==**

**----- Start of picture text -----**<br>
|||
|---|---|
|Table of Contents|
|1|General Information ...................................................................................................... 5|
|1.1|Ordering Information .........................................................................................................5|
|1.2|Marking Information ..........................................................................................................5|
|1.3|Internal Block Diagram .......................................................................................................6|
|1.4|Interface Operating ............................................................................................................7|
|1.5|Application Diagrams .........................................................................................................8|
|1.6|Package & Pin Information ............................................................................................... 10|
|1.7|Recommended External Components ............................................................................... 12|
|2|QMI8658A Chip Orientation Coordinate System ........................................................... 13|
|3|System, Electrical and Electro-Mechanical Characteristics ............................................. 14|
|3.1|Absolute Maximum Ratings ............................................................................................. 14|
|3.2|Recommended Operating Conditions ............................................................................... 14|
|3.3|Power On Sequence of VDDIO and VDD ............................................................................ 15|
|3.3.1|Power-On Reset(POR) ....................................................................................................... 15|
|3.3.2|VDDIO and VDD Are Driven by Single Power .................................................................... 15|
|3.3.3|VDDIO and VDD Are Driven by Separate Power Lines ...................................................... 16|
|3.4|Electro-Mechanical Specifications..................................................................................... 17|
|3.5|Accelerometer Programmable Characteristics ................................................................... 19|
|3.6|Gyroscope Programmable Characteristics ......................................................................... 21|
|3.7|Electrical Characteristics ................................................................................................... 22|
|3.8|Current Consumption ....................................................................................................... 23|
|3.9|Temperature Sensor ........................................................................................................ 24|
|4|Register Map Overview ................................................................................................ 25|
|4.1|UI Register Map Overview ................................................................................................ 25|
|5|UI Sensor Configuration Settings and Output Data ........................................................ 28|
|5.1|Typical Sensor Mode Configuration and Output Data ........................................................ 28|
|5.2|Chip Information Register ................................................................................................ 29|
|5.3|Configuration Registers .................................................................................................... 30|
|5.4|FIFO Registers .................................................................................................................. 34|
|5.5|Status and Time Stamp Registers ...................................................................................... 35|
|5.6|Sensor Data Output Registers ........................................................................................... 37|
|5.7|Calibration-On-Demand (COD) Status Register .................................................................. 38|
|5.8|Activity Detection Output Registers .................................................................................. 39|
|5.9|Reset Register .................................................................................................................. 40|
|5.10|CTRL 9 Functionality (Executing Pre-defined Commands) .................................................. 41|
|5.10.1|CTRL9 Protocol Description .............................................................................................. 41|
|5.10.2|CTRL9 Command List ........................................................................................................ 42|
|5.10.3|CAL Registers ..................................................................................................................... 42|
|5.10.4|WCtrl9 (Write – CTRL9 Protocol) ...................................................................................... 43|
|5.10.5|Ctrl9R (CTRL9 Protocol - Read) ......................................................................................... 43|
|5.10.6|CTRL9 Commands in Details ............................................................................................. 43|
|6|Interrupts .................................................................................................................... 46|
|6.1|SyncSample mode ............................................................................................................ 46|
|6.2|Non-SyncSample mode .................................................................................................... 47|
|6.3|DRDY(Data Ready) ........................................................................................................... 47|

**----- End of picture text -----**<br>


© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

2

**==> picture [506 x 686] intentionally omitted <==**

**----- Start of picture text -----**<br>
|||
|---|---|
|7|Operating Modes ......................................................................................................... 49|
|7.1|Operating Modes Descriptions ......................................................................................... 49|
|7.2|General Mode Transitioning ............................................................................................. 49|
|7.3|Transition Time ................................................................................................................ 50|
|7.4|Chip Reset Process ........................................................................................................... 51|
|8|FIFO Description ........................................................................................................... 52|
|8.1|FIFO Structure .................................................................................................................. 52|
|8.2|FIFO Size .......................................................................................................................... 52|
|8.3|Configure FIFO Mode ....................................................................................................... 52|
|8.4|FIFO Sample Count ........................................................................................................... 53|
|8.5|FIFO Watermark Interrupt ................................................................................................ 53|
|8.6|FIFO Full .......................................................................................................................... 53|
|8.7|FIFO Read Mode .............................................................................................................. 53|
|8.8|Read FIFO Data ................................................................................................................ 53|
|8.9|FIFO Data Pattern ............................................................................................................ 54|
|8.10|Reset FIFO ....................................................................................................................... 54|
|9|Motion Detection ......................................................................................................... 55|
|9.1|Motion Detection Principle .............................................................................................. 55|
|9.1.1|Any-Motion Detection Principle ....................................................................................... 55|
|9.1.2|No-Motion Detection Principle ......................................................................................... 55|
|9.1.3|Significant-Motion Detection Principle ............................................................................. 56|
|9.2|Motion Detection Flow .................................................................................................... 57|
|9.3|Parameters of Motion Detection ...................................................................................... 58|
|9.4|Config Motion Detection .................................................................................................. 59|
|9.5|Enabling Motion Detection ............................................................................................... 59|
|9.6|Motion Interrupt.............................................................................................................. 59|
|10|Tap .............................................................................................................................. 60|
|10.1|Tap Detection Principle .................................................................................................... 60|
|10.2|Tap Detection Parameters ................................................................................................ 61|
|10.3|Configure Tap .................................................................................................................. 62|
|10.4|Enable Tap Detection ....................................................................................................... 62|
|10.5|Tap Interrupt ................................................................................................................... 62|
|10.6|Tap Detection Output ...................................................................................................... 62|
|11|Pedometer ................................................................................................................... 64|
|11.1|Pedometer Parameters .................................................................................................... 64|
|11.2|Configure Pedometer ....................................................................................................... 65|
|11.3|Enable Pedometer ........................................................................................................... 65|
|11.4|Pedometer Interrupt ........................................................................................................ 66|
|11.5|Read Step Count .............................................................................................................. 66|
|11.6|Reset Step Count ............................................................................................................. 66|
|12|Wake on Motion (WoM) .............................................................................................. 67|
|12.1|Wake on Motion Principle ................................................................................................ 67|
|12.2|Wake on Motion Parameters ........................................................................................... 67|
|12.3|Accelerometer Configuration ........................................................................................... 67|
|12.4|Wake on Motion Event .................................................................................................... 67|
|12.5|Configuration Procedure .................................................................................................. 68|
|12.6|Exit Wake on Motion Mode .............................................................................................. 68|

**----- End of picture text -----**<br>


Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

© 2022 QST Corporation

www. qstcorp.com

3

**==> picture [506 x 686] intentionally omitted <==**

**----- Start of picture text -----**<br>
|||||
|---|---|---|---|
|13|Locking Mechanism ...................................................................................................... 69|
|13.1|Locking Mechanism Principle ........................................................................................... 69|
|13.2|Locking Mechanism Data Reading Process ........................................................................ 69|
|13.2.1|Disable/Enable AHB Clock Gating ..................................................................................... 70|
|13.2.2|Enable Locking Mechanism ............................................................................................... 70|
|13.2.3|Reading Sensor Data ......................................................................................................... 70|
|13.3|Data_Lock_Delay ............................................................................................................. 70|
|13.4|Exit Locking Mechanism ................................................................................................... 71|
|13.5|On-The-Fly ODR Change in Locking Mechanism ................................................................. 71|
|14|Calibration-On-Demand (COD) ..................................................................................... 72|
|14.1|COD Principle ................................................................................................................... 72|
|14.2|Run COD .......................................................................................................................... 72|
|14.3|COD Status ...................................................................................................................... 72|
|14.4|Save and Restore the New Gain Parameters ..................................................................... 72|
|15|Self-Test (Check-Alive) ................................................................................................. 73|
|15.1|Accelerometer Self-Test ................................................................................................... 73|
|15.2|Gyroscope Self-Test ......................................................................................................... 73|
|16|Host Serial Interface ..................................................................................................... 75|
|16.1|Address Auto Increment .................................................................................................. 75|
|16.2|Serial Peripheral Interface (SPI) ........................................................................................ 75|
|16.2.1|SPI Features ...................................................................................................................... 75|
|16.2.2|SPI Interface Connection................................................................................................... 76|
|16.2.3|SPI Transaction Protocol ................................................................................................... 77|
|16.2.4|SPI Timing Characteristics ................................................................................................. 81|
|16.3|I|[2]|C Interface ..................................................................................................................... 83|
|16.3.1|I|[2]|C Slave Address Selection ............................................................................................... 83|
|16.3.2|I|[2]|C Interface Characteristics .............................................................................................. 83|
|16.4|I|[3]|C Interface ..................................................................................................................... 84|
|17|Package and Handling .................................................................................................. 85|
|17.1|Package Drawing ............................................................................................................. 85|
|17.2|Reflow Specification......................................................................................................... 86|
|17.3|Storage Specifications ...................................................................................................... 86|
|18|Document Information ................................................................................................. 87|
|18.1|Revision History ............................................................................................................... 87|

**----- End of picture text -----**<br>


© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 4

## **1 General Information**

## **1.1 Ordering Information**

**Table 1. Ordering Information Part Number Package Packing Method** QMI8658A LGA14 Tape & Reel

## **1.2 Marking Information**

**ROW EXAMPLE CODE/EXPLANATION** 1 **`8658`** DDDD  – Device code 2 **`2113`** YWLL – Y (Year code), W (1-digit, biweekly code), LL (Lot indication) 3 **`●  RB`** CR – C (Assembly location), R (Product revision)

**==> picture [145 x 103] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 1. Top Mark<br>**----- End of picture text -----**<br>


**==> picture [228 x 228] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 5

## **1.3 Internal Block Diagram**

**==> picture [436 x 438] intentionally omitted <==**

**==> picture [151 x 10] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 2.  Internal Block Diagram<br>**----- End of picture text -----**<br>


Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

© 2022 QST Corporation

www. qstcorp.com 6

## **1.4 Interface Operating**

The QMI8658A can operate in below mode, as shown in the _Figure 3_ . The QMI8658A is a slave device to a host processor that communicates to it using one of the following interfaces: I[2] C, I[3] C, and SPI (3-wire or 4-wire modes). This slave relationship to the host is the same for all operating modes.

**==> picture [303 x 235] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 3.  Operating Mode<br>**----- End of picture text -----**<br>


**==> picture [228 x 228] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 7

## **1.5 Application Diagrams**

The typical application diagrams are shown in this section.

**==> picture [468 x 411] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 4.  4-Wire SPI-UI mode<br>Figure 5.  3-Wire SPI-UI mode<br>**----- End of picture text -----**<br>


**==> picture [85 x 92] intentionally omitted <==**

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

© 2022 QST Corporation

www. qstcorp.com 8

**==> picture [468 x 183] intentionally omitted <==**

**Figure 6. I3C/I2C-UI mode**

There is internal 200Kohm pull-up resistor on the SCL, SDA, CS, SDx, SCx, RESV (Pin10). By default, all those pullup resistors are enabled. All the resistors can be disabled by CTRL9 command, refer to _1.6 Package & Pin Information_ and _5.10.6 CTRL_CMD_SET_RPU_ for details.

There is internal 200Kohm pull-up resistor on SDO/SA0 pin, and is enabled during Power-On Reset or Soft-Reset, is automatically disabled after detecting the I2C slave address during the Reset Process. Therefore, in I2C/I3C mode leave the SDO/SA0 float or connect it to High level (recommended, to provide a stable level), will set the I2C slave address/I3C static address to 0x6A. And connect it to Low level, will set the I2C slave address/ I3C static address to 0x6B.

In 3-wire SPI mode, leave the SDO/SA0 pin float.

SCx and SDx can be connected to VDDIO or Logic High, GND or Logic Low, or be left float if internal pull-up resistors are enabled.

RESV (Pin 10) should NOT be connected to GND or Logic Low. It can be connected to VDDIO or Logic High, or leave it externally float and enable the internal pull-up resistor(by default the pull-up resistor is enabled). Connecting it to VDDIO is preferred, which can provide a stable High level.

RESV-NC (Pin 11) is by default an output pin, should be float (no connection). In case of the necessity to connect it to High or Low level, the RESV(Pin 10) should be firmly connected to VDDIO, providing a stable High level, to disable the output of Pin 11.

Table 2 describes the names for the pins in different functions. The later descriptions will directly use the function name in different scenarios instead of the pin name.

**Table 2. Pin Name Mapped to Function Name**

|Pin Number|Type|Pin Name|Function Name in<br>4-wire SPI|Function Name in<br>3-wire SPI|Function Name in<br>Host I2C / I3C|
|---|---|---|---|---|---|
|1|O|SDO/SA0|SDO||SA0|
|12|I|CS|CS|CS||
|13|IO|SCL|SPC|SPC|SCL|
|14|IO|SDA|SDI|SDIO|SDA|



© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 9

## **1.6 Package & Pin Information**

The pinout of the QMI8658A is shown in Figure 7 and Figure 8. The pin names and functionality are detailed in Table 3. The pin functionality is dictated by the QMI8658A’s operating mode, as described in 1.5 Application Diagrams.

**==> picture [460 x 241] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 7.  Pins Face Down (Top View) Figure 8. Pins Face Up (Bottom View)<br>**----- End of picture text -----**<br>


**==> picture [250 x 250] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 10

## **Table 3. Pin Definitions**

**==> picture [455 x 543] intentionally omitted <==**

**----- Start of picture text -----**<br>
Pin  Type  Pin Name  Function<br>Number<br>1  O  SDO/SA0 [(1)(3)] SPI-UI Data Out (SDO) in SPI-UI  4-Wire Mode<br>SA0, I [2] C Slave address and I [3] C Static address select:<br>        If SA0 = 0, I [2] C / I [3] C address = 0x6B<br>        If SA0 = 1, I [2] C / I [3] C address = 0x6A<br>2  IO  SDx [(1)(2)] Reserved. Connect to VDDIO, GND or No Connection<br>3  IO  SCx [(1)(2)] Reserved. Connect to VDDIO, GND or No Connection<br>4  O  INT1  Programmable Interrupt 1<br>5  I  VDDIO  Power Supply for IO Pins<br>6  I  GND  Ground (0 V supply)<br>7  I  GND  Ground (0 V supply)<br>8  I  VDD   Power supply<br>9  O  INT2   Programmable Interrupt 2 (INT2) / Data Ready (DRDY)<br>10  IO  RESV [(1)(2)] Reserved. Connect to VDDIO or Logic High, or No Connection and<br>enable (by default) internal pull up resistor.<br>Refer to 1.5  Application Diagrams.<br>11  I  RESV-NC   Reserved. Connect to VDDIO or Logic High, GND or Logic Low.<br>Refer to 1.5  Application Diagrams.<br>12  I  CS [(1)(2)] I [2] C/ I [3] C /SPI-UI selection Pin.<br>(If 1: I [2] C-UI Mode: I [2] C/I [3] C communication enabled, SPI idle mode)<br>(If 0: SPI-UI mode: I [2] C/I [3] C disabled)<br>13  IO  SCL [(1)(2)] I [2] C/I [3] C-UI Data (SDA) in I [2] C/ I [3] C mode<br>SPI-UI Serial Clock (SPC) [ (3)]   in SPI mode<br>14  IO  SDA [(1)(2)] I [2] C/I [3] C-UI Data (SDA)<br>SPI-UI Data In (SDI) [ (3)]  in 4 wire Mode<br>SPI-UI Data IO (SDIO) [ (3)]  in 3 Wire Mode<br>**----- End of picture text -----**<br>


## **Notes:**

1. This pin has an internal 200K pull up resistor.

2. The internal pull-up resistor can be disabled by CTRL9 command (CTRL_CMD_SET_RPU). Refer to _**5.10.6** CTRL_CMD_SET_RPU_ for details.

3. Refer to Section 16 for detailed configuration information.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

11

## **1.7 Recommended External Components**

## **Table 4. Recommended External Components**

|**Component**|**Description**|**Parameter**|**Typical**|
|---|---|---|---|
|Cp1|Capacitor|Capacitance|100 nF|
|Cp2|Capacitor|Capacitance|100 nF|
|Rpu(4)|Resistor|Resistance|2K 10 k|



## **Note:**

4. Rpu resistors are only needed when the Host Serial Interface is configured for I[2] C (see I2C Interface section). They are not needed when the Host Serial Interface is configured for SPI or I3C. If pull-up resistors are used on SCL and SDA, then SPI, I3C and I[2] C Modes are all possible. If a pull-down resistor is used on SA0, an alternate slave address is used for I[2] C. SPI and I3C modes will be unaltered with the use of pull-up resistors for I[2] C. Additionally, a suitable pull up resistance (Rpu) value should be selected, accounting for the tradeoff between current consumption and rise time.

**==> picture [166 x 241] intentionally omitted <==**

**==> picture [188 x 10] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 9.  Typical Electrical Connections<br>**----- End of picture text -----**<br>


© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 12

## **2 QMI8658A Chip Orientation Coordinate System**

The QMI8658A uses a right-handed coordinate system as the basis for the sensor frame of reference. Acceleration (ax, ay, az) are given with respect to the X-Y-Z coordinate system shown above. Increasing accelerations along the positive X-Y-Z axes are considered positive. Angular Rate (x, y, z) in the counterclockwise direction around the respective axis are considered positive.

Figure 10 shows the various frames of reference and conventions for using the QMI8658A.

**==> picture [333 x 257] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 10. Chip Orientation Coordinate System<br>**----- End of picture text -----**<br>


**==> picture [197 x 186] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 13

## **3 System, Electrical and Electro-Mechanical Characteristics**

## **3.1 Absolute Maximum Ratings**

Stresses exceeding the absolute maximum ratings may damage the device. The device may not function or be operable above the recommended operating conditions. Stressing the parts to these levels is not recommended. In addition, extended exposure to stresses above the recommended operating conditions may affect device reliability. The absolute maximum ratings are stress ratings only.

## **Table 5. Absolute Maximum Ratings**

| | | | | | | |
|---|---|---|---|---|---|
|**Symbol**|**Parameter**||**Min.**|**Max.**|**Unit**|
|TSTG|Storage Temperature||-40|+125|°C|
|TPmax|Lead SolderingTemperature, 10 Seconds|||+260|°C|
|VDD|SupplyVoltage||-0.3|3.6|V|
|VDDIO|I/O Pins SupplyVoltage||-0.3|3.6|V|
|Sg(5)|Accelerationgfor 0.2 ms(Un-powered)|||10,000|g|
|ESD(6)|Electrostatic Discharge<br>Protection Level|Human BodyModelper JES001-2014|±2000||V|
|||Charged Device Modelper JESD22-C101|±500|||
|**Notes:**||||||



## **Notes:**

5. This is a mechanical shock (g) sensitive device. Proper handling is required to prevent damage to the part. 6. This is an ESD-sensitive device. Proper handling is required to prevent damage to the part.

## **3.2 Recommended Operating Conditions**

The Recommended Operating Conditions table defines the conditions for device operation. Recommended operating conditions are specified to ensure optimal performance. It is not forbidden to design beyond to Absolute Maximum Ratings.

## **Table 6. Recommended Operating Conditions**

|**Table 6.**|**Recommended Operating Conditions**|||||
|---|---|---|---|---|---|
|**Symbol**|**Parameter**|**Min**|**Typ**|**Max**|**Unit**|
|VDD|SupplyVoltage|1.71|1.8|3.6|V|
|VDDIO|I/O Pins SupplyVoltage|1.71|1.8|3.6|V|
|VIL|Digital Low Level Input Voltage|||0.3 *VDDIO|V|
|VIH|Digital High Level Input Voltage|0.7 *VDDIO||VDDIO + 0.3|V|
|VOL|Digital Low Level Output Voltage|||0.1 *VDDIO|V|
|VOH|Digital High Level Output Voltage|0.9 *VDDIO|||V|
|VPOR_RLS|POR Threshold Voltage||1.1||V|
| | | | | | | |



© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

14

## **3.3 Power On Sequence of VDDIO and VDD**

## 3.3.1 **Power-On Reset(POR)**

Once the VDD & VDDIO are powered from 0V to a certain level, the internal power voltage detector will trigger the Power-On Reset (POR) automatically, and then exit/release the POR mode as the VDD voltage rises over the POR Threshold (about 1.1V). Refer to _Table 6_ .

After POR release, there will be about 200us Startup Delay, followed by the QMI8658A Initialization.

The instability of VDDIO & VDD power lines, especially the power increase/drop with high slew rate, would interfere the QMI8658A Initialization and operation. Therefore, there should be no sudden transients and spikes on power lines after Startup Delay, to make sure the Initialization and later on operations are properly implemented.

Normally it takes within about 15ms (refer to _System Turn On Time_ in _Table 7_ and _Table 8_ ) for QMI8658A to finish the Initialization and during which, there should be no write/configuration to QMI8658A, to prevent possible interference and failure.

Note that the Software Reset is triggered by the reset command that host write to QMI8658A, which means POR is not involved, while the following sequence are similar (Startup Delay and Initialization) to POR. Refer to _7.4_ for more details.

**==> picture [50 x 49] intentionally omitted <==**

## 3.3.2 **VDDIO and VDD Are Driven by Single Power**

As shown in _Figure 11_ , when QMI8658A is driven by single/same power line, the power line should ramp up from POR Threshold (about 1.1V) to Min Operation Voltage(1.71V) with slew rate higher than 40V/s, otherwise the Initialization might fail, and QMI8658A might not work properly.

**==> picture [473 x 158] intentionally omitted <==**

**Figure 11. Power On process When Driven by Single Power Line**

**==> picture [85 x 92] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

15

## 3.3.3 **VDDIO and VDD Are Driven by Separate Power Lines**

As shown in _Figure 12_ , when QMI8658A is driven by separate power lines, the VDDIO should be always powered ahead of (no later than) VDD, which means:

_VDDdelay >= 0_ , VDDIO should be driven no later than VDD

Note that the VDDdelay starts from the point when VDDIO rises over the POR Threshold and ended at the VDD rises over the POR Threshold. It should always be non-negative.

The power lines should ramp up from POR Threshold (about 1.1V) to Min Operation Voltage(1.71V) with slew rate higher than 40V/s, otherwise the Initialization might fail, and QMI8658A might not work properly.

**==> picture [452 x 298] intentionally omitted <==**

**Figure 12. Power On process When Driven by Separate Power Lines**

**==> picture [85 x 92] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

16

## **3.4 Electro-Mechanical Specifications**

VDD = VDDIO = 1.8 V, T = 25°C unless otherwise noted.

## **Table 7. Accelerometer Electro-Mechanical Specifications**

|**Subsystem**|**Parameter**|**Typical**|**Typical**|**Unit**|**Comments**|
|---|---|---|---|---|---|
|**Accelerometer**|Noise Density|150||g/`√`Hz|High-Resolution Mode|
||Sensitivity Scale Factor|Scale Setting|Sensitivity|LSB/g|16-Bit Output|
|||±2g|16,384|||
|||±4g|8,192|||
|||±8g|4,096|||
|||±16g|2,048|||
||Cross-Axis Sensitivity|±1||%||
||Temperature Coefficient of<br>Offset (TCO)|±1||mg/°C|Over-Temperature Range of<br>-40°C to 85°C, at Board<br>Level|
||Temperature Coefficient of<br>Sensitivity (TCS)|±0.04||%/°C|Over-Temperature Range of<br>-40°C to 85°C, at Board<br>Level|
||Initial Offset Tolerance|±100||mg|Board Level|
||Initial SensitivityTolerance|±6||%|Board Level|
||Non-Linearity|±0.75||%|Best Fit Line|
||System Turn On Time(1)|15||ms|From Software Reset, No<br>Power, or Power Down to<br>Power-on Default state = t0<br>in Figure 16|
||Accel Turn On Time|3 ms + 3/ODR||ms|Accel Turn on from Power-<br>On Default state or from Low<br>Power state = t2 + t5 in<br>Figure 16.|



## **Note:**

7. System Turn-On Time defines the initialization duration of QMI8658A, it starts from about 200us later than the release of POR(Power On Reset) or the Software Reset. Refer to _3.3_ and _7.4_ for details.

**==> picture [121 x 121] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 17

**Table 8. Gyroscope Electro-Mechanical Specifications**

|**Subsystem**|**Parameter**|**Typical**|**Typical**|**Unit**|**Comments**|
|---|---|---|---|---|---|
|**Gyroscope**|Sensitivity|Scale<br>Setting|Sensitivity|LSB/dps|16-Bit Output|
|||±16 dps|2048|||
|||±32 dps|1024|||
|||±64 dps|512|||
|||±128 dps|256|||
|||±256 dps|128|||
|||±512 dps|64|||
|||±1024 dps|32|||
|||±2048 dps|16|||
||Natural Frequency|22.42||kHz|Precision +/- 2%(typical)|
||Noise Density|13||mdps/`√`Hz|High-Resolution Mode|
||Non-Linearity|±0.2||%||
||Cross-Axis Sensitivity|±2||%||
||g-Sensitivity|±0.1||dps/g||
||System Turn On Time(1)|15||ms|From Software Reset, No<br>Power, or Power Down to<br>Power-on Default state<br>= t0 in Figure 16|
||Gyro Turn On Time|150 ms + 3/ODR||ms|Gyro Turn on from Power-<br>On Default = t1 + t5 in<br>Figure 16|
||Temperature Coefficient of<br>Offset (TCO)|X/Y: ±0.1<br>Z: ±0.05||dps/°C|Over-Temperature Range of<br>-40°C to 85°C, at Board<br>Level|
||Temperature Coefficient of<br>Sensitivity (TCS)|X/Y: ±0.05<br>Z: ±0.01||%/°C|Over-Temperature Range of<br>-40°C to 85°C, at Board<br>Level|
||Initial Offset Tolerance|±10||dps|Board Level|
||Initial SensitivityTolerance|±3||%|Board Level|
|**Note:**||||||



8. System Turn-On Time defines the initialization duration of QMI8658A, it starts from about 200us later than the release of POR(Power On Reset) or the Software Reset. Refer to _3.3_ and _7.4_ for details.

**==> picture [68 x 70] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 18

## **3.5 Accelerometer Programmable Characteristics**

VDD = VDDIO = 1.8 V, T = 25°C unless otherwise noted. Typical numbers are provided below unless otherwise noted.

If only accelerometer is enabled, the ODR frequency is derived from the internal oscillator. If both accelerometer and gyroscope (6DOF mode) are enabled, the ODR frequency is derived from the natural frequency of gyroscope. _Table 9_ shows the two ODR frequencies, which can be referenced for later descriptions in the datasheet. Refer to section _5.3_ for detailed ODR configuration.

RMS noise can be calculated based on the noise density and the bandwidth.

## **Table 9. Accelerometer Noise Density**

|**Mode**||||**High-Resolution**|**High-Resolution**|**High-Resolution**|||||**Low-Power**|**Low-Power**|**Low-Power**|**Unit**|
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|**ODR**<br>**(Accel**<br>**only) **||||1000|500|250|125|62.5|31.25|128|21|11|3|Hz|
|**ODR**<br>**(Accel +**<br>**Gyro)**|7174.4|3587.2|<br>1793.6|896.8|448.4|224.2|112.1|56.05|28.025|||||Hz|
|**Typical**<br>**Noise**<br>**Density**|150|150|150|150|150|150|150|150|150|125|180|285|700|g/`√`Hz|



**Table 10. Accelerometer Filter Characteristics (Accelerometer only)[(9)]**

|**Mode**||||**High-Resolution**|**High-Resolution**|**High-Resolution**||||**Low-Power**|**Low-Power**|**Low-Power**|**Low-Power**|**Unit**|
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|**ODR**|8000|4000|2000|1000|500|250|125|62.5|31.25|128|21|11|3|Hz|
|**Bandwidth**<br>**(Default, 27.5% of**<br>**ODR)**|<br>NA|NA|NA|275|137.5|68.8|34.4|17.2|8.6|35.2|5.8|3.0|0.8||
|**Bandwidth with**<br>**Low-Pass Filter**<br>**Enabled Mode 00**<br>**(2.66% of ODR)**|NA|NA|NA|26.6|13.3|6.7|3.3|1.7|0.8|3.4|0.6|0.3|0.1||
|**Bandwidth with**<br>**Low-Pass Filter**<br>**Enabled Mode 01**<br>**(3.63% of ODR)**|NA|NA|NA|36.3|18.2|9.1|4.5|2.3|1.1|4.6|0.8|0.4|0.1||
|**Bandwidth with**<br>**Low-Pass Filter**<br>**Enabled Mode 10**<br>**(5.39% of ODR)**|NA|NA|NA|53.9|27|13.5|6.7|3.4|1.7|6.9|1.1|0.6|0.2||
|**Bandwidth with**<br>**Low-Pass Filter**<br>**Enabled Mode 11**<br>**(13.37% of ODR)**|NA|NA|NA|133.7|66.9|33.4|16.7|8.4|4.2|17.1|2.8|1.5|0.4||



## **Note:**

9. When only accelerometer is enabled, the ODR is derived from the internal oscillator, rather than the nature frequency of Gyroscope

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 19

**Table 11. Accelerometer Filter Characteristics (6DOF)[(10)]**

|**Table 11.**|**Accelerometer Filter Characteristics (6DOF)(10)**|**Accelerometer Filter Characteristics (6DOF)(10)**|**Accelerometer Filter Characteristics (6DOF)(10)**|**Accelerometer Filter Characteristics (6DOF)(10)**|**Accelerometer Filter Characteristics (6DOF)(10)**|**Accelerometer Filter Characteristics (6DOF)(10)**|**Accelerometer Filter Characteristics (6DOF)(10)**|**Accelerometer Filter Characteristics (6DOF)(10)**|**Accelerometer Filter Characteristics (6DOF)(10)**||||||
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|**Mode**|**High-Resolution**|||||||||**Low-Power**||||**Unit**|
|**ODR**|7174.4|3587.2|1793.6|896.8|448.4|224.2|112.1|56.05|28.025|128|21|11|3|Hz|
|**Bandwidth**<br>**(Default,**<br>**27.5% of**<br>**ODR)**|1973.0|986.5|493.2|246.6|123.3|61.7|30.8|15.4|7.7|NA|NA|NA|NA||
|**Bandwidth**<br>**with Low-**<br>**Pass Filter**<br>**Enabled**<br>**Mode 00**<br>**(2.66% of**<br>**ODR)**|190.8|95.4|47.7|23.9|11.9|6.0|3.0|1.5|0.7|NA|NA|NA|NA||
|**Bandwidth**<br>**with Low-**<br>**Pass Filter**<br>**Enabled**<br>**Mode 01**<br>**(3.63% of**<br>**ODR)**|260.4|130.2|65.1|32.6|16.3|8.1|4.1|2.0|1.0|NA|NA|NA|NA||
|**Bandwidth**<br>**with Low-**<br>**Pass Filter**<br>**Enabled**<br>**Mode 10**<br>**(5.39% of**<br>**ODR)**|386.7|193.4|96.7|48.3|24.2|12.1|6.0|3.0|1.5|NA|NA|NA|NA||
|**Bandwidth**<br>**with Low-**<br>**Pass Filter**<br>**Enabled**<br>**Mode 11**<br>**(13.37% of**<br>**ODR)**|959.2|479.6|239.8|119.9|60.0|30.0|15.0|7.5|3.7|NA|NA|NA|NA||
|**Note:**|||||||||||||||



10. When both accelerometer and gyroscope are both enabled, all frequencies are synchronized to the nature frequency of gyroscope.

**==> picture [85 x 92] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 20

## **3.6 Gyroscope Programmable Characteristics**

VDD = VDDIO = 1.8 V, T = 25°C, and represent typical numbers unless otherwise noted. All frequencies are synchronized to the gyroscope nature frequency.

_Table 12_ shows the noise density of gyroscope output over different ODR configurations.

The typical bandwidths of gyroscope over different ODR settings are listed in _Table 13_ .

RMS noise can be calculated based on the noise density and the bandwidth.

**Table 12. Gyroscope Noise Density**

|**Table 12. Gyroscope N**|**oise De**|**nsity**|||||||||
|---|---|---|---|---|---|---|---|---|---|---|
|**Mode**||||**High-Resolution**||||||**Unit**|
|**ODR (Gyro and/or Accel)**|7174.4|3587.2|1793.6|896.8|448.4|224.2|112.1|56.05|28.025|Hz|
|**Typical Noise Density**|RSV(11)|RSV(11)|RSV(11)|13|13|13|13|13|13|mdps/`√`Hz|
|**Note:**|||||||||||



## **Note:**

11. The gyroscope noise increases significantly when the ODR is set above 1KHz. It is not recommended to set the ODR beyond 1KHz for the applications that are impacted by gyroscope noise. For more information please contact QST FAE team.

**==> picture [480 x 335] intentionally omitted <==**

**----- Start of picture text -----**<br>
Table 13. Gyroscope Filter Characteristics<br>Mode  High-Resolution  Unit<br>ODR  7174.4  3587.2  1793.6  896.8  448.4  224.2  112.1  56.05  28.025<br>Bandwidth<br>(Default, 27.5% of  1973.0  986.5  493.2  246.6  123.3  61.7  30.8  15.4  7.7<br>ODR)<br>Bandwidth with<br>Low-Pass Filter<br>190.8  95.4  47.7  23.9  11.9  6.0  3.0  1.5  0.7<br>Enabled Mode 00<br>(2.66% of ODR)<br>Bandwidth with<br>Low-Pass Filter<br>260.4  130.2  65.1  32.6  16.3  8.1  4.1  2.0  1.0  Hz<br>Enabled Mode 01<br>(3.63% of ODR)<br>Bandwidth with<br>Low-Pass Filter<br>386.7  193.4  96.7  48.3  24.2  12.1  6.0  3.0  1.5<br>Enabled Mode 10<br>(5.39% of ODR)<br>Bandwidth with<br>Low-Pass Filter<br>959.2  479.6  239.8  119.9  60.0  30.0  15.0  7.5  3.7<br>Enabled Mode 11<br>(13.37% of ODR)<br>**----- End of picture text -----**<br>


© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 21

## **3.7 Electrical Characteristics**

VDD = VDDIO = 1.8 V, T = 25°C unless otherwise noted.

**Table 14. Electrical Subsystem Characteristics**

**==> picture [469 x 128] intentionally omitted <==**

**----- Start of picture text -----**<br>
Symbol  Parameter  Min.  Typ.  Max.  Unit<br>fSPC Host SPI Interface Speed 15  MHz<br>Host I [2] C Interface Speed (standard mode and Fast<br>fSCL Mode are supported)  400  kHz<br>Standard Data Rate<br>fSCL3 Host I [3] C Interface Speed  12.5 MHz<br>(SDR)<br>**----- End of picture text -----**<br>


**==> picture [377 x 377] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 22

## **3.8 Current Consumption**

VDD = VDDIO = 1.8 V, T = 25°C unless otherwise noted. IDD Current refers to the current flowing into the VDD pin. Typical numbers are provided below.

**Table 15. Current Consumption for Accelerometer Only Sensor Mode (Gyroscope Disabled)**

||**Mode**||**High-Resolution**|**High-Resolution**|**High-Resolution**|**High-Resolution**||**Low-Power**|**Low-Power**|**Low-Power**||**Unit**|
|---|---|---|---|---|---|---|---|---|---|---|---|---|
||**ODR**|1000|500|250|125|62.5|31.25|128|21|11|3|Hz|
|**Typical**<br>**Overall IDD**<br>**Current**|**Filters Disabled**<br>**(aLPF=0)**|182|155|142|134|133|132|55|42|35|30|A|
||**Filters Enabled**<br>**(aLPF=1)**|182|155|142|134|133|132|55|42|35|30||



**Table 16. Current Consumption for Gyroscope Only Sensor Mode (Accelerometer Disabled)**

||**Mode**||||**High-Resolution**|**High-Resolution**|**High-Resolution**||||**Unit**|
|---|---|---|---|---|---|---|---|---|---|---|---|
||**ODR**|7174.4|3587.2|1793.6|896.8|448.4|224.2|112.1|56.05|28.025|Hz|
|**Typical**<br>**Overall**<br>**IDD**<br>**Current**|**Filters Disabled**<br>**(gLPF=0)**|908|861|748|689|659|656|654|653|651|A|
||**Filters Enabled**<br>**(gLPF=1)**|916|863|748|689|659|656|654|653|651||



**Table 17. Current Consumption for 6DOF Sensor Mode (Accelerometer and Gyroscope Enabled)**

|**Mode**|**Mode**||||**High-Resolution**|**High-Resolution**|**High-Resolution**||||**Unit**|
|---|---|---|---|---|---|---|---|---|---|---|---|
|**ODR**||7174.4|3587.2|<br>1793.6|896.8|448.4|224.2|112.1|56.05|28.025|<br>Hz|
|**Typical**<br>**Overall IDD**<br>**Current**|**Filters Disabled**<br>**(aLPF=0;**<br>**gLPF=0)**|1004|956|843|786|757|754|752|751|750|A|
||**Filters Enabled**<br>**(aLPF=1;**<br>**gLPF=1)**|1031|970|850|789|758|756|753|751|750||
| | | | | | | |



**==> picture [121 x 121] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 23

## **3.9 Temperature Sensor**

The QMI8658A is equipped with an internal 16-bit embedded temperature sensor that is automatically turned on by default whenever the accelerometer or gyroscope is enabled. The temperature sensor is used internally to correct the temperature dependency of calibration parameters of the accelerometer and gyroscope. The temperature compensation is optimal in the range of -40[°] C to 85[°] C with a resolution of 0.0625[°] C (1/16[°] C) or inversely, 16 LSB/[ °] C.

The QMI8658A outputs the internal chip temperature that the HOST can read. The output is 16 bits, with a (1/256)[°] C per LSB resolution. To read the temperature, the HOST needs to access the TEMP register _(see TEMP_L and TEMP_H in Data Output Registers in Table 25)._

The calculation formular is:

T = TEMP_H + (TEMP_L / 256)

To read the temperature sensor data properly, the Host is expected to follow the guidelines in 13 Locking Mechanism.

**==> picture [469 x 181] intentionally omitted <==**

**----- Start of picture text -----**<br>
Table 18. Temperature Sensor Specifications<br>Subsystem  Parameter  Typical  Unit<br>Range  -40 to +85  °C<br>Internal Resolution  16  Bits<br>Internal Sensitivity 256  LSB/°C<br>Digital Temperature Sensor<br>Output Register Width  16  Bits<br>Output Sensitivity 256  LSB/°C<br>Refresh Rate  8  Hz<br>**----- End of picture text -----**<br>


**==> picture [228 x 228] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 24

## **4 Register Map Overview**

The QMI8658A UI registers enable programming and control of the inertial measurement unit and associated on-chip signal processing. These registers are accessed through the UI interface – either SPI (4 wires or 3 wires) I3C, or I[2] C.

## **4.1 UI Register Map Overview**

UI register map may be classified into the following register categories:

- Chip Information Registers

- Setup and Control Registers: control various aspects of the IMU.

- Host Controlled Calibration Registers: control and configure various aspects of the IMU via the host command interface called CTRL9

|▪<br>Count Register for time stamping the sensor samples<br>▪<br>Sensor data registers<br>▪<br>FIFO Registers: to set up the FIFO and detect data availability and over-run<br>▪<br>Activity Detection status registers<br>▪<br>General Purpose Registers<br>**Table 19. UI Register Overview**|▪<br>Count Register for time stamping the sensor samples<br>▪<br>Sensor data registers<br>▪<br>FIFO Registers: to set up the FIFO and detect data availability and over-run<br>▪<br>Activity Detection status registers<br>▪<br>General Purpose Registers<br>**Table 19. UI Register Overview**|▪<br>Count Register for time stamping the sensor samples<br>▪<br>Sensor data registers<br>▪<br>FIFO Registers: to set up the FIFO and detect data availability and over-run<br>▪<br>Activity Detection status registers<br>▪<br>General Purpose Registers<br>**Table 19. UI Register Overview**|▪<br>Count Register for time stamping the sensor samples<br>▪<br>Sensor data registers<br>▪<br>FIFO Registers: to set up the FIFO and detect data availability and over-run<br>▪<br>Activity Detection status registers<br>▪<br>General Purpose Registers<br>**Table 19. UI Register Overview**|▪<br>Count Register for time stamping the sensor samples<br>▪<br>Sensor data registers<br>▪<br>FIFO Registers: to set up the FIFO and detect data availability and over-run<br>▪<br>Activity Detection status registers<br>▪<br>General Purpose Registers<br>**Table 19. UI Register Overview**|▪<br>Count Register for time stamping the sensor samples<br>▪<br>Sensor data registers<br>▪<br>FIFO Registers: to set up the FIFO and detect data availability and over-run<br>▪<br>Activity Detection status registers<br>▪<br>General Purpose Registers<br>**Table 19. UI Register Overview**|▪<br>Count Register for time stamping the sensor samples<br>▪<br>Sensor data registers<br>▪<br>FIFO Registers: to set up the FIFO and detect data availability and over-run<br>▪<br>Activity Detection status registers<br>▪<br>General Purpose Registers<br>**Table 19. UI Register Overview**|
|---|---|---|---|---|---|---|
|||**Register Address**|||**Default**|**Comment**|
|**Name**|**Type**||||||
|||**Dec**|**Hex**|**Binary**|**Binary**||
| | | | | | | |
|**General Purpose Registers**|||||||
|WHO_AM_I|r|0|00|00000000|00000101|Device Identifier|
|REVISION_ID|r|1|01|00000001|01101000|Device Revision ID|
|**Setup and Control Registers**|||||||
|CTRL1|rw|2|02|00000010|00100000|SPI Interface and Sensor Enable|
|CTRL2|rw|3|03|00000011|00000000|Accelerometer: Output Data Rate, Full Scale, Self-Test|
|CTRL3|rw|4|04|00000100|00000000|Gyroscope: Output Data Rate, Full Scale, Self-Test|
|Reserved|rw|5|05|00000101|00000000|Reserved|
|CTRL5|rw|6|06|00000110|00000000|Lowpass filter setting|
|Reserved|rw|7|07|00000111|00000000|Reserved|
|CTRL7|rw|8|08|00001000|00000000|Enable Sensors|
|CTRL8|rw|9|09|00001001|00000000|Motion Detection Control|
|CTRL9|rw|10|0A|00001010|00000000|Host Commands|
|**Host Controlled Calibration Registers (See CTRL9, Usage is Optional)**|||||||
|CAL1_L|rw|11|0B|00001011|00000000|Calibration Register|
|CAL1_H|rw|12|0C|00001100|00000000|<br>CAL1_L – lower 8 bits. CAL1_H – upper 8 bits.|
|CAL2_L|rw|13|0D|00001101|00000000|Calibration Register|
|CAL2_H|rw|14|0E|00001110|00000000|CAL2_L – lower 8 bits. CAL2_H – upper 8 bits.|
|CAL3_L|rw|15|0F|00001111|00000000|Calibration Register|
|CAL3_H|rw|16|10|00010000|00000000|<br>CAL3_L – lower 8 bits. CAL3_H – upper 8 bits.|
|CAL4_L|rw|17|11|00010001|00000000|Calibration Register|
|CAL4_H|rw|18|12|00010010|00000000|<br>CAL4_L – lower 8 bits. CAL4_H – upper 8 bits.|
|**FIFO Registers**|||||||
|FIFO_WTM_TH|rw|19|13|00010011|00000000|FIFO watermark level, in ODRs|
|FIFO_CTRL|rw|20|14|00010100|00000000|FIFO Setup|



Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

© 2022 QST Corporation

www. qstcorp.com 25

|FIFO_SMPL_CNT|r|21|15|00010101|00000000|FIFO sample count LSBs|
|---|---|---|---|---|---|---|
|FIFO_STATUS|r|22|16|00010110|00000000|FIFO Status|
|FIFO_DATA|r|23|17|00010111|00000000|FIFO Data|
|**Status Registers**|||||||
|||||||Sensor Data Availability with the Locking mechanism,|
|STATUSINT|r|45|2D|00101101|00000000||
|||||||<br>CmdDone (CTRL9protocol bit).|
| | | | | | | |
|STATUS0|r|46|2E|00101110|00000000|Output Data Over Run and Data Availability.|
|||||||Miscellaneous Status: Any Motion, No Motion,|
|STATUS1|r|47|2F|00101111|00000000||
|||||||<br>Significant Motion, Pedometer, Tap.|
| | | | | | | |
|**Timestamp Register**|||||||
|TIMESTAMP_|||||||
||r|48|30|00110000|00000000|Sample Time Stamp|
|LOW|||||||
|||||||TIMESTAMP_LOW – lower 8 bits.|
|TIMESTAMP_MID|r|49|31|00110001|00000000||
|||||||TIMESTAMP_MID – middle 8 bits.|
|TIMESTAMP_HIG|||||||
||r|50|32|00110010|00000000|TIMESTAMP_HIGH – upper 8 bits|
|H|||||||
| | | | | | | |
|**Data Output Registers (16 bits 2’s Complement Except COD Sensor Data)**|||||||
|TEMP_L|r|51|33|00110011|00000000|Temperature Output Data|
|TEMP_H|r|52|34|00110100|00000000|TEMP_L – lower 8 bits. TEMP_H – upper 8 bits|
|AX_L|r|53|35|00110101|00000000|X-axis Acceleration|
|AX_H|r|54|36|00110110|00000000|AX_L – lower 8 bits. AX_H – upper 8 bits|
|AY_L|r|55|37|00110111|00000000|Y-axis Acceleration|
|AY_H|r|56|38|00111000|00000000|AY_L – lower 8 bits. AY_H – upper 8 bits|
|AZ_L|r|57|39|00111001|00000000|Z-axis Acceleration|
|AZ_H|r|58|3A|00111010|00000000|AZ_L – lower 8 bits. AZ_H – upper 8 bits|
|GX_L|r|59|3B|00111011|00000000|X-axis Angular Rate|
|GX_H|r|60|3C|00111100|00000000|<br>GX_L – lower 8 bits. GX_H – upper 8 bits|
|GY_L|r|61|3D|00111101|00000000|Y-axis Angular Rate|
|GY_H|r|62|3E|00111110|00000000|<br>GY_L – lower 8 bits. GY_H – upper 8 bits|
|GZ_L|r|63|3F|00111111|00000000|Z-axis Angular Rate|
|GZ_H|r|64|40|01000000|00000000|<br>GZ_L – lower 8 bits. GZ_H – upper 8 bits|
|**COD Indication and General Purpose Registers**|||||||
|COD_STATUS|r|70|46|01000110|00000000|Calibration-On-Demand status register|
|dQW_L|r|73|49|01001001|00000000|Generalpurpose register|
|dQW_H|r|74|4A|01001010|00000000|Generalpurpose register|
|dQX_L|r|75|4B|01001011|00000000|Generalpurpose register|
|dQX_H|r|76|4C|01001100|00000000|Reserved|
|dQY_L|r|77|4D|01001101|00000000|General purpose register|
|dQY_H|r|78|4E|01001110|00000000|Reserved|
|dQZ_L|r|79|4F|01001111|00000000|Reserved|
|dQZ_H|r|80|50|01010000|00000000|Reserved|
|dVX_L|r|81|51|01010001|00000000|Generalpurpose register|
|dVX_H|r|82|52|01010010|00000000|Generalpurpose register|
|dVY_L|r|83|53|01010011|00000000|Generalpurpose register|
|dVY_H|r|84|54|01010100|00000000|Generalpurpose register|
|dVZ_L|r|85|55|01010101|00000000|Generalpurpose register|
|dVZ_H|r|86|56|01010110|00000000|Generalpurpose register|



Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

© 2022 QST Corporation

www. qstcorp.com 26

|**Activity Detection**|**Output Registers**|**Output Registers**|**Output Registers**||||
|---|---|---|---|---|---|---|
|TAP_STATUS|r|89|59|01011001|00000000|Axis, direction, number of detected Tap|
|STEP_CNT_LOW|r|90|5A|01011010|00000000|Low byte of stepcount of Pedometer|
|STEP_CNT_MIDL|r|91|5B|01011011|00000000|Middle byte of stepcount of Pedometer|
|STEP_CNT_HIGH|<br>r|92|5C|01011100|00000000|High byte of stepcount of Pedometer|
|**Reset Register**|||||||
|RESET|w|96|60|01100000|00000000|Soft Reset Register|



**==> picture [448 x 438] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 27

## **5 UI Sensor Configuration Settings and Output Data**

## **5.1 Typical Sensor Mode Configuration and Output Data**

In Typical sensor mode, QMI8658A outputs raw sensor values. The sensors are configured and read using the registers described below. The accelerometer and gyroscope can be independently configured. _Table 20_ summarizes these pertinent registers.

**Table 20. Typical Sensor Mode Configuration and Output Data**

|**Table 20. Typical Sensor Mode Configuration and Output Data**|**Table 20. Typical Sensor Mode Configuration and Output Data**|**Table 20. Typical Sensor Mode Configuration and Output Data**|**Table 20. Typical Sensor Mode Configuration and Output Data**|
|---|---|---|---|
|||||
|**Typical Sensor Configuration and Output Data**||||
|**Description**|**Registers**|**Unit**|**Comments**|
|Sensor Enable, SPI 3 or 4<br>Wire|CTRL1||Control power states, configure SPI communications|
|Enable Sensor|CTRL7||Individually Enable/Disable the AttitudeEngine, Accelerometer,<br>and Gyroscope UsingsEN, aEN, andgEN bits, respectively.|
|Configure Accelerometer,<br>Enable Self-Test|CTRL2||Configure Full Scale and Output Data Rate; Enable Self-Test|
|Configure Gyroscope,<br>Enable Self-Test|CTRL3||Configure Full Scale and Output Data Rate; Enable Self-Test|
|Sensor Filters|CTRL5||Configure and Enable/Disable Low Pass Filters|
|Status|STATUSINT<br>STATUS0,<br>STATUS1||Data Availability, FIFO Ready to be Read, CTRL9 Protocol Bit|
|Time Stamp|TIMESTAMP[<br>H,M,L]||Sample Time Stamp (Circular Register 0 – 0xFFFFFF)|
|Acceleration|A[X,Y,Z]_[H,L]|<br>g|In Sensor Frame of Reference, Right-handed Coordinate System|
|Angular Rate|G[X,Y,Z]_[H,L]|dps|In Sensor Frame of Reference, Right-handed Coordinate System|
|Temperature|TEMP_[H,L]|`°`C|Temperature of the Sensor|
|FIFO Based Output|FIFO_DATA||1 Byte FIFO Data Outputs|
|||||



**==> picture [143 x 143] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 28

## **5.2 Chip Information Register**

**Table 21. Chip Information Register Description**

|**Register Name**|**Register Name**|||
|---|---|---|---|
|**WHO_AM_I**||**Register Address: 0(0x00)**||
|**Bits**|**Name**|**Default**|**Description**|
|7:0|WHO_AM_I|0x05|Device identifier 0x05 - to identify the device is a QST sensor.<br>Read-only.|
|**REVISION_ID**||**Register Address: 1 (0x01)**||
|**Bits**|**Name**|**Default**|**Description**|
|7:0|REVISION_ID|0x7C|Device Revision ID.<br>Read-only.|
|||||



**==> picture [353 x 352] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 29

## **5.3 Configuration Registers**

This section describes the various operating modes and register configurations of the QMI8658A.

## **Table 22. Configuration Registers Description**

|**Register Name**|**Register Name**|||||||
|---|---|---|---|---|---|---|---|
|**CTRL1**||**Serial Interface and Sensor Enable. Register Address: 2 (0x02)**||||||
|**Bits**|**Name**|**Default**|**Description**|||||
|7|SIM|1’b0|0: Enables 4-wire SPI interface<br>1: Enables 3-wire SPI interface|||||
|6|ADDR_AI|1’b0|0: Serial interface (SPI, I2C, I3C) address non-increment.<br>1: Serial interface (SPI, I2C, I3C) address auto increment|||||
|5|BE|1’b1|0: Serial interface (SPI, I2C, I3C) read data Little-Endian<br>1: Serial interface (SPI, I2C, I3C) read data Big-Endian|||||
|4|INT2_EN|1’b0|0: INT2 pin is high-Z mode<br>1: INT2pin output is enabled|||||
|3|INT1_EN|1’b0|0: INT1 pin is high-Z mode<br>1: INT1pin output is enabled|||||
|2|FIFO_INT_SEL|1’b0|0: FIFO interrupt is mapped to INT2 pin<br>1: FIFO interrupt is mapped to INT1pin|||||
|1|Reserved|1’b0||||||
|0|SensorDisable|1’b0|0: Enable internal high-speed oscillator<br>1: Disable internal high-speed oscillator. Refer to _7.1_.|||||
|**CTRL2**||**Accelerometer Settings: Address: 3 (0x03)**||||||
|**Bits**|**Name**|**Default**|**Description**|||||
|7|aST|1’b0|0: Disable Accelerometer Self-Test;<br>1: Enable Accelerometer Self-Test.|||||
|6:4|aFS<2:0>|3’b0|Set Accelerometer Full-scale (1xx – N/A):<br>000 - Accelerometer Full-scale = ±2 g<br>001 - Accelerometer Full-scale = ±4 g<br>010 - Accelerometer Full-scale = ±8 g<br>011 – Accelerometer Full-scale = ±16g|||||
|3:0|aODR<3:0>(12)(13)|4’b0|Set Accelerometer Output Data Rate (ODR):||||**Duty Cycle**<br>100%<br>100%<br>100%<br>100%<br>100%<br>100%<br>100%<br>100%<br>100%<br>100%<br>58%<br>31%<br>8.5%|
||||**Setting**|**ODR Rate (Hz)**<br>**(Accel only)**|**ODR Rate (Hz)**<br>**(6DOF)(13)**|**Mode**|**Duty Cycle**|
||||0000|N/A|7174.4|Normal|100%|
||||0001|N/A|3587.2|Normal|100%|
||||0010|N/A|1793.6|Normal|100%|
||||0011|1000|896.8|Normal|100%|
||||0100|500|448.4|Normal|100%|
||||0101|250|224.2|Normal|100%|
||||0110|125|112.1|Normal|100%|
||||0111|62.5|56.05|Normal|100%|
||||1000|31.25|28.025|Normal|100%|
||||1001|N/A|N/A|||
||||1010|N/A|N/A|||
||||1011|N/A|N/A|||
||||1100|128|N/A|Low Power|100%|
||||1101|21|N/A|Low Power|58%|
||||1110|11|N/A|Low Power|31%|
||||1111|3|N/A|Low Power|8.5%|



Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

© 2022 QST Corporation

www. qstcorp.com 30

**Table 22 Configuration Register Description (Continued)**

|**Register Name**|**Register Name**||||||
|---|---|---|---|---|---|---|
|**CTRL3**||**Gyroscope Settings: Address 4(0x04)**|||||
|**Bits**|**Name**|**Default**|**Description**||||
|7|gST|1’b0|0: Disable Gyro self-Test<br>1: Enable Gyro Self-Test.||||
|6:4|gFS<2:0>|3’b0|Set Gyroscope Full-scale:<br>000 - ±16 dps<br>001 - ±32 dps<br>010 - ±64 dps<br>011 - ±128 dps<br>100 - ±256 dps<br>101 - ±512 dps<br>110 - ±1024dps<br>111 - ±2048 dps||||
|3:0|gODR<3:0>(13)|4’b0|Set Gyroscope Output Data Rate(ODR):<br>**Setting**<br>**ODR Rate(Hz)**<br>**Mode**<br>**Duty Cycle**<br>0000<br>7174.4<br>Normal<br>100%<br>0001<br>3587.2<br>Normal<br>100%<br>0010<br>1793.6<br>Normal<br>100%<br>0011<br>896.8<br>Normal<br>100%<br>0100<br>448.4<br>Normal<br>100%<br>0101<br>224.2<br>Normal<br>100%<br>0110<br>112.1<br>Normal<br>100%<br>0111<br>56.05<br>Normal<br>100%<br>1000<br>28.025<br>Normal<br>100%<br>1001<br>N/A<br>1010<br>N/A<br>1011<br>N/A<br>1100<br>N/A<br>1101<br>N/A<br>1110<br>N/A<br>1111<br>N/A||||
||||**Setting**|**ODR Rate(Hz)**|**Mode**|**Duty Cycle**|
||||0000|7174.4|Normal|100%|
||||0001|3587.2|Normal|100%|
||||0010|1793.6|Normal|100%|
||||0011|896.8|Normal|100%|
||||0100|448.4|Normal|100%|
||||0101|224.2|Normal|100%|
||||0110|112.1|Normal|100%|
||||0111|56.05|Normal|100%|
||||1000|28.025|Normal|100%|
||||1001|N/A|||
||||1010|N/A|||
||||1011|N/A|||
||||1100|N/A|||
||||1101|N/A|||
||||1110|N/A|||
||||1111|N/A|||
| | | | | | | |



## **Note:**

12. The accelerometer low power mode is only available when the gyroscope is disabled

13. In 6DOF mode (accelerometer and gyroscope are both enabled), the ODR is derived from the nature frequency of gyroscope, refer to section 3.5 for more information.

**==> picture [60 x 47] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 31

## **Table 22 Configuration Register Description (Continued)**

|**Table 22 Configuration**|**Table 22 Configuration**|**Register Description (Continued)**|**Register Description (Continued)**|**Register Description (Continued)**|
|---|---|---|---|---|
|**Register Name**|||||
|**CTRL5**||**Sensor Data Processing Settings. Register Address: 6(0x06)**|||
|**Bits**|**Name**|**Default**||**Description**|
|7|Reserved|1’b0|||
|6:5|gLPF_MODE|2’b0||BW [Hz]<br>2.66% of ODR<br>3.63% of ODR<br>5.39% of ODR<br>13.37% of ODR|
||||gLPF_MODE|BW [Hz]|
||||00|2.66% of ODR|
||||01|3.63% of ODR|
||||10|5.39% of ODR|
||||11|13.37% of ODR|
|4|gLPF_EN|1’b0|0: Disable Gyroscope Low-Pass Filter.<br>1: Enable Gyroscope Low-Pass Filter with the mode given by<br>gLPF_MODE.||
|3|Reserved|1’b0|||
|2:1|aLPF_MODE|2’b0||BW [Hz]<br>2.66% of ODR<br>3.63% of ODR<br>5.39% of ODR<br>13.37% of ODR|
||||aLPF_MODE|BW [Hz]|
||||00|2.66% of ODR|
||||01|3.63% of ODR|
||||10|5.39% of ODR|
||||11|13.37% of ODR|
|0|aLPF_EN|1’b0|0: Disable Accelerometer Low-Pass Filter.<br>1: Enable Accelerometer Low-Pass Filter with the mode given by<br>aLPF_MODE.||
||||||



**==> picture [197 x 186] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 32

**Table 22 Configuration Register Description (Continued)**

**==> picture [480 x 537] intentionally omitted <==**

**----- Start of picture text -----**<br>
Register Name<br>CTRL7  Enable Sensors and Configure Data Reads. Register Address: 8 (0x08)<br>Bits  Name  Default  Description<br>0: Disable SyncSample mode<br>7  SyncSample  1’b0<br>1: Enable SyncSample mode<br>6  Reserved  1’b0<br>0: DRDY(Data Ready) is enabled, is driven to the INT2 pin<br>5  DRDY_DIS  1’b0<br>1: DRDY(Data Ready) is disabled, is blocked from the INT2 pin<br>0: Gyroscope in Full Mode (Drive and Sense are enabled).<br>4  gSN  1’b0  1: Gyroscope in Snooze Mode (only Drive enabled).<br>This bit is effective only when gEN is set to 1. Refer to 7.1 .<br>3:2  Reserved  2’b0<br>0: Disable Gyroscope.<br>1  gEN  1’b0<br>1: Enable Gyroscope.<br>0: Disable Accelerometer.<br>0  aEN  1’b0<br>1: Enable Accelerometer.<br>CTRL8  Motion Detection Control. Register Address: 9 (0x09)<br>Bits  Name  Default  Description<br>7  CTRL9_HandSha 1b’0 0: use INT1 as CTRL9 handshake<br>ke_Type  1: use STATUSINT.bit7 as CTRL9 handshake<br>6  ACTIVITY_INT_S 1b’0  0: INT2 is used for Activity Detection event interrupt<br>EL  1: INT1 is used for Activity Detection event interrupt<br>Note: this bit influences the Any/No/Sig-motion, Pedometer, Tap Detection<br>interrupt<br>5  reserved  1b’0<br>4  Pedo_EN  1b’0  0: disable Pedometer engine<br>1: enable Pedometer engine<br>3  Sig-Motion_EN  1b’0  0: disable Significant Motion engine<br>1: enable Significant Motion engine<br>2  No-Motion_EN  1b’0  0: disable No Motion engine<br>1: enable No Motion engine<br>1  Any-Motion_EN  1b’0  0: disable Any Motion engine<br>1: enable Any Motion engine<br>0  Tap_EN  1b’0  0: disable Tap engine<br>1: enable Tap engine<br>Register Name<br>Host Commands. Register Address: 10 (0x0A), Referred to:<br>CTRL9<br>CTRL 9 Functionality (Executing Pre-defined Commands)<br>**----- End of picture text -----**<br>


© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

33

## **5.4 FIFO Registers**

**Table 23. FIFO Control/Status/Data Registers**

|**Register Name**|**Register Name**|||||
|---|---|---|---|---|---|
|**FIFO_WTM_TH**||**FIFO Watermark Register Address: 19 (0x13)**||||
|**Bits**|**Name**|**Default**|**Description**|||
|7:0|FIFO_WTM|8’h0|Number of ODRs(Samples) needed to trigger FIFO watermark|||
|**FIFO_CTRL**||**FIFO Control Register Address: 20 (0x14)**||||
|**Bits**|**Name**|**Default**|**Description**|||
|7|FIFO_RD_MODE|1’b0|0: FIFO is in Write mode, sensor data (if enabled) can be filled into FIFO<br>1: FIFO is in Read mode, FIFO data can be read via FIFO_DATA register<br>This bit is automatically set by using a CTRL9 command. It must be<br>cleared again (by write 1’b0 to this bit) after the data read is complete so<br>that fillingdata to the FIFO can resume. Refer to_5.10.6.3_.|||
|6:4|Reserved|3’b0||||
|3:2|FIFO_SIZE|2’b0|FIFO_SIZE[1:0]|FIFO Sample Size||
||||00|16 samples||
||||01|32 samples||
||||10|64 samples||
||||11|128 samples||
|1:0|FIFO_MODE|2’b0|FIFO_MODE[1:0]|FIFO Mode||
||||00|Bypass (FIFO disable)||
||||01|FIFO||
||||10|Stream||
||||11|Reserved||
|**FIFO_SMPL_CNT**||**FIFO Sample Count Register Address: 21 (0x15)**||||
|**Bits**|**Name**|**Default**|**Description**|||
|7:0|FIFO_SMPL_CNT_L<br>SB|8’b0|8 LS bits of FIFO Sample Count, in word (2bytes).|||
|**FIFO_STATUS**||**FIFO Status. Register Address 22 (0x16)**||||
|**Bits**|**Name**|**Default**|**Description**|||
|7|FIFO_FULL|1’b0|0: FIFO is not Full<br>1: FIFO is Full|||
|6|FIFO_WTM|1’b0|0: FIFO Water Mark Level not hit.<br>1: FIFO Water Mark Level Hit|||
|5|FIFO_OVFLOW|1’b0|0: FIFO Overflow has not happened<br>1: FIFO Overflow condition has happened (data droppinghappened)|||
|4|FIFO_NOT_EMPTY|1’b0|0: FIFO is Empty<br>1: FIFO is not Empty|||
|3:2|Reserved|2’b0||||
|1:0|FIFO_SMPL_CNT_<br>MSB|2’b0|2 MS bits of FIFO Sample Count in word (2bytes).|||
|**FIFO_DATA**||**FIFO DATA Output Register Address: 23(0x17)**||||
|**Bits**|**Name**|**Default **|**Description**|||
|7:0|FIFO_DATA|8’b0|8 bit FIFO data output.|||



© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 34

## **5.5 Status and Time Stamp Registers**

**Table 24. Status and Time Stamp Registers**

|**Register Name**|**Register Name**|||
|---|---|---|---|
|**STATUSINT**||**Sensor Data Available and Lock Register Address: 45(0x2D)**||
|**Bits**|**Name**|**Default**|**Description**|
|7|Ctrl9 CmdDone|1’b0|Indicates CTRL9 Command was done, as part of CTRL9 protocol<br>0: Not Completed<br>1: Done|
|6:2|Reserved|5’b0||
|1|Locked|1’b0|If syncSmpl (CTRL7.bit7) = 1:<br>0: Sensor Data is not locked.<br>1: Sensor Data is locked.<br>If syncSmpl = 0, this bit shows the same value of INT1 level|
|0|Avail|1’b0|If syncSmpl (CTRL7.bit7)  = 1:<br>0: Sensor Data is not available<br>1: Sensor Data is available for reading<br>If syncSmpl = 0, this bit shows the same value of INT2 level|
|**STATUS0**||**Output Data Status Register Address: 46 (0x2E)**||
|**Bits**|**Name**|**Default**|**Description**|
|7:2|Reserved|6’b0||
|1|gDA|1’b0|Gyroscope new data available<br>0: No updates since last read.<br>1: New data available.|
|0|aDA|1’b0|Accelerometer new data available<br>0: No updates since last read.<br>1: New data available.|
|**STATUS1**||**Miscellaneous Status. Register Address 47 (0x2F)**||
|**Bits**|**Name**|**Default**|**Description**|
|7|Significant Motion|1’b0|0: No Significant-Motion was detected<br>1: Significant-Motion was detected|
|6|No Motion|1’b0|0: No No-Motion was detected<br>1: No-Motion was detected|
|5|Any Motion|1’b0|0: No Any-Motion was detected<br>1: Any-Motion was detected|
|4|Pedometer|1’b0|0: No step was detected<br>1: stepwas detected|
|3|Reserved|1’b0||
|2|WoM|1’b0|0: No WoM was detected<br>1: WoM was detected|
|1|TAP|1’b0|0: No Tap was detected<br>1: Tapwas detected|
|0|Reserved|1’b0||
|**TIMESTAMP**||**3 Bytes Sample Time Stamp – Output Count.**<br>**Register Address: 48 - 50**(**0x30 - 0x32)**||
|**Bits**|**Name**|**Default**|**Description**|



© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 35

|7:0|TIMESTAMP_L<7:<br>0>|0x00|Sample time stamp. Count incremented by one for each sample<br>(x, y, z data set) from sensor with highest ODR (circular register<br>0x0-0xFFFFFF).|
|---|---|---|---|
|7:0|TIMESTAMP_M<1<br>5:8>|0x00||
|7:0|TIMESTAMP_H<2<br>3:16>|0x00||



**==> picture [448 x 438] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 36

## **5.6 Sensor Data Output Registers**

**Table 25. Sensor Data Output Registers Description**

|**Register Name**|**Register Name**|||
|---|---|---|---|
|**TEMP_[H,L]**||**Temp Sensor Output. Register Address: 0x33 – 0x34**||
|**Bits**|**Name**|**Default**|**Description**|
|7:0|TEMP_L|0x00|Temperature output (°C) in two’s complement.<br>T = TEMP_H + (TEMP_L / 256)|
|7:0|TEMP_H|0x00||
|**Register Name**||||
|**A[X,Y,Z]_[H,L]**||**Acceleration Output. Register Address: 0x35 – 0x3A**||
|**Bits**|**Name**|**Default**|**Description**|
|7:0|AX_L<7:0>|0x00|X-axis acceleration in two’s complement**.**<br>AX_L – lower 8 bits. AX_H – upper 8 bits.|
|7:0|AX_H<15:8>|0x00||
|7:0|AY_L<7:0>|0x00|Y-axis acceleration in two’s complement.<br>AY_L – lower 8 bits. AY_H – upper 8 bits.|
|7:0|AY_H<15:8>|0x00||
|7:0|AZ_L<7:0>|0x00|Z-axis acceleration in two’s complement.<br>AZ_L – lower 8 bits. AZ_H – upper 8 bits.|
|7:0|AZ_H<15:8>|0x00||
|**Register Name**||||
|**G[X,Y.Z]_[H,L]**||**Angular Rate Output. Register Address: 0x3B – 0x40**||
|**Bits**|**Name**|**Default**|**Description**|
|7:0|GX_L<7:0>|0x00|X-axis angular rate in two’s complement.<br>GX_L – lower 8 bits. GX_H – upper 8 bits.|
|7:0|GX_H<15:8>|0x00||
|7:0|GY_L<7:0>|0x00|Y-axis angular rate in two’s complement.<br>GY_L – lower 8 bits. GY_H – upper 8 bits.|
|7:0|GY_H<15:8>|0x00||
|7:0|GZ_L<7:0>|0x00|Z-axis angular rate in two’s complement.<br>GZ_L – lower 8 bits. GZ_H – upper 8 bits.|
|7:0|GZ_H<15:8>|0x00||
|||||



**==> picture [121 x 121] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 37

## **5.7 Calibration-On-Demand (COD) Status Register**

|**Register Name**|**Register Name**|||
|---|---|---|---|
|**COD_STATUS**||**Register Address: 70 (0x46)**||
|**Bits**|**Name**|**Default**|**Description**|
|7|X_Limit_L_Fail|1’b0|0: COD passed for checking low sensitivity limit of X axis of gyroscope<br>1: COD failed for checkinglow sensitivitylimit of X axis ofgyroscope|
|6|X_Limit_H_Fail|1’b0|0: COD passed for checking high sensitivity limit of X axis of gyroscope<br>1: COD failed for checkinghigh sensitivitylimit of X axis ofgyroscope|
|5|Y_Limit_L_Fail|1’b0|0: COD passed for checking low sensitivity limit of Y axis of gyroscope<br>1: COD failed for checkinglow sensitivitylimit of Y axis ofgyroscope|
|4|Y_Limit_H_Fail|1’b0|0: COD passed for checking high sensitivity limit of Y axis of gyroscope<br>1: COD failed for checkinghigh sensitivitylimit of Y axis ofgyroscope|
|3|Accel_Check|1’b0|0: Accelerometer checked pass (no significant vibration happened during<br>COD)<br>1: Accelerometer checked failed (significant vibration happened duringCOD)|
|2|Startup_Failed|1’b0|0: Gyroscope startup succeeded<br>1: Gyroscope startupfailure happened when COD was called|
|1|Gyro_Enabled|1’b0|0: COD was called when gyroscope was not enabled<br>1: COD was called whilegyroscope was enabled, COD return failure|
|0|COD_Failed|1’b0|0: COD succeeded, new gain parameters will be applied to GX & GY data<br>1: COD failed; no COD correction applied|



Note the value of this register is only valid after the COD command. Refer to _14 Calibration-On-Demand (COD)._

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 38

## **5.8 Activity Detection Output Registers**

**Table 26. Activity Detection Output Registers**

|**Register Name**|**Register Name**|||
|---|---|---|---|
|**TAP_STATUS**||**Register Address: 89(0x59)**||
|**Bits**|**Name**|**Default**|**Description**|
|7|TAP_POLARITY|1’b0|0: Tap was detected on the positive direction of the Tap axis<br>1: Tapwas detected on the negative direction of the Tapaxis|
|6|Reserved|1’b0||
|5:4|TAP_AXIS|2’b0|0: No Tap was detected<br>1: Tap was detected on X axis<br>2: Tap was detected on Y axis<br>3: Tapwas detected on Z axis|
|3:2|Reserved|2’b0||
|1:0|TAP_NUM|2’b0|0: No Tap was detected<br>1: Single-Tap was detected<br>2: Double-Tap was detected<br>3: NA|
|**STEP_COUNT[23:0]**||||
|**Bits**|**Name**|**Default**|**Description**|
|23:16|STEP_CNT_HIGH|0x00|24-bit Step Count detected by Pedometer engine|
|15:8|STEP_CNT_MID|0x00||
|7:0|STEP_CNT_LOW|0x00||
|||||



**==> picture [197 x 186] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 39

## **5.9 Reset Register**

**Table 27. Reset Register Description**

|**Register Name**|**Register Name**|||
|---|---|---|---|
|**RESET**||**Register Address: 96(0x60)**||
|**Bits**|**Name**|**Default**|**Description**|
|7:0|RESET|0x00|Soft Reset Register - Write 0xB0 to this register from any modes,<br>will trigger the sensor reset process immediately.<br>The register 0x4D will equals to 0x80 if there is a successful reset<br>(Power-on Reset or Soft Reset) process.<br>Refer to_7.4 Chip Reset Process_.|
|||||



**==> picture [377 x 377] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 40

## **5.10 CTRL 9 Functionality (Executing Pre-defined Commands)**

## 5.10.1 **CTRL9 Protocol Description**

The protocol for executing predefined commands from an external host processor on the QMI8658A is facilitated by using the CTRL9 register. The register is available to the host via the UI SPI/I[2] C/I[3] C bus.

It operates by the host writing a pre-defined value (Command, refer to _5.10.2_ CTRL9 Command List) to the CTRL9 register. The firmware of the QMI8658A evaluates this command and if a match is found it executes the corresponding pre-defined function.

Once the function has been executed, the QMI8658A signals the completion by setting STATUSINT.bit7 to 1 , and raising INT1 interrupt if CTRL1.bit3 = 1 & CTRL8.bit7 == 0. The host must acknowledge this by writing CTRL_CMD_ACK (0x00) to CTRL9 register. After receiving the CTRL_CMD_ACK command, the QMI8658A clears the STATUSINT.bit7 to 0 and pulls down the INT1 interrupt if CTRL1.bit3 = 1 & CTRL8.bit7 == 0.

This command presentation from the host to the QMI8658A and the subsequent execution and handshake between the host and the QMI8658A will be referred to as the “ _CTRL9 Protocol_ ”.

There are three types of interactions between the host and QMI8658A that follow the CTRL9 Protocol.

**WCtrl9** : The host needs to supply data to QMI8658A prior to the Ctrl9 protocol. ( **Write – Ctrl9 Protocol** )

**Ctrl9R** : The host gets data from QMI8658A following the Ctrl9 protocol. ( **Ctrl9 protocol – Read** )

**Ctrl9** : No data transaction is required prior to or following the Ctrl9 protocol. ( **Ctrl9** ).

**==> picture [432 x 368] intentionally omitted <==**

**==> picture [172 x 10] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 13.  CTRL9 Protocol Flow Chart<br>**----- End of picture text -----**<br>


© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 41

## 5.10.2 **CTRL9 Command List**

The predefined CTRL9 commands are listed in Table 28 List of CTRL9 Command.

## **Table 28. List of CTRL9 Commands**

|**Table 28. List of CTRL9 Commands**||||
|---|---|---|---|
|**Command Name**|**CTRL9**<br>**Command**<br>**Value**|**Protocol**<br>**Type**|**Description**|
|CTRL_CMD_ACK|0x00|Ctrl9|Acknowledgement. Host acknowledges to<br>QMI8658, to end theprotocol.|
|CTRL_CMD_RST_FIFO|0x04|Ctrl9|Reset FIFO from Host|
|CTRL_CMD_REQ_FIFO|0x05|Ctrl9R|Get FIFO data from Device|
|CTRL_CMD_WRITE_WOM_SETTING|0x08|WCtrl9|Set up and enable Wake on Motion (WoM)|
|CTRL_CMD_ACCEL_HOST_DELTA_OFFSET|0x09|WCtrl9|Change accelerometer offset|
|CTRL_CMD_GYRO_HOST_DELTA_OFFSET|0x0A|WCtrl9|Change gyroscope offset|
|CTRL_CMD_CONFIGURE_TAP|0x0C|WCtrl9|Configure Tap detection|
|CTRL_CMD_CONFIGURE_PEDOMETER|0x0D|WCtrl9|Configure Pedometer|
|CTRL_CMD_CONFIGURE_MOTION|0x0E|WCtrl9|Configure Any Motion / No Motion /<br>Significant Motion detection|
|CTRL_CMD_RESET_PEDOMETER|0x0F|WCtrl9|Reset pedometer count (step count)|
|CTRL_CMD_COPY_USID|0x10|Ctrl9R|Copy USID and FW Version to UI<br>registers|
|CTRL_CMD_SET_RPU|0x11|WCtrl9|Configures IO pull-ups|
|CTRL_CMD_AHB_CLOCK_GATING|0x12|WCtrl9|Internal AHB clock gating switch|
|CTRL_CMD_ON_DEMAND_CALIBRATION|0xA2|WCtrl9|On-Demand Calibration on gyroscope|
|CTRL_CMD_APPLY_GYRO_GAINS|0xAA|WCtrl9|Restore the saved Gyroscope gains|
|||||



## 5.10.3 **CAL Registers**

The set of CAL registers can be used for the parameter transferring, if WCTL9 or CTRL9R commands are implemented. Refer to _Table 28_ and _5.10.6_ for details.

**Table 29. CAL Register Addresses**

|**Table 29. CAL Register Addresses**|||||
|---|---|---|---|---|
||||||
|**Register Name**|**Dec**|**Register Address**||**Hex**|
|CAL1_L|11|||0x0B|
|CAL1_H|12|||0x0C|
|CAL2_L|13|||0x0D|
|CAL2_H|14|||0x0E|
|CAL3_L|15|||0x0F|
|CAL3_H|16|||0x10|
|CAL4_L|17|||0x11|
|CAL4_H|18|||0x12|



© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 42

## 5.10.4 **WCtrl9 (Write – CTRL9 Protocol)**

1. The host needs to provide the required data for this command to the QMI8658A. The host typically does this by placing the data in a set of registers called the CAL registers. Maximum eight CAL registers are used. Refer to _Table 29_ .

2. Write Ctrl9 register 0x0A with the appropriate Command value, refer to _Table 28_ .

3. The Device will set STATUSINT.bit7 to 1, and raise INT1(if CTRL1.bit3 = 1 & CTRL8.bit7 == 0), once it has executed the appropriate function based on the command value.

4. The host must acknowledge this by writing CTRL_CMD_ACK (0x00) to CTRL9 register, STATUSINT.bit7 (CmdDone) will be reset to 0 on receiving the CTRL_CMD_ACK command. And INT1 is pulled low upon the register read if CTRL1.bit3 = 1 & CTRL8.bit7 == 0.

5. If any data is expected from the device, it will be available at this time. The location of the data is specified separately for each of the Commands.

Refer to _5.10.6 for details._

**==> picture [49 x 49] intentionally omitted <==**

## 5.10.5 **Ctrl9R (CTRL9 Protocol - Read)**

1. Write Ctrl9 register 0x0A with the appropriate Command value.

2. The Device will set STATUSINT.bit7 to 1, and raise INT1 (if CTRL1.bit3 = 1 & CTRL8.bit7 == 0), once it has executed the appropriate function based on the command value.

3. The host must acknowledge this by writing CTRL_CMD_ACK (0x00) to CTRL9 register, STATUSINT.bit7 (CmdDone) will be reset to 0 on receiving the CTRL_CMD_ACK command. INT1 is pulled low upon the register read if CTRL1.bit3 = 1 & CTRL8.bit7 == 0.

4. Data is available from the device on the CAL registers. The location of the data is specified separately for each of the Commands.

Refer to _5.10.6 for details._

## 5.10.6 **CTRL9 Commands in Details**

## **5.10.6.1 CTRL_CMD_ACK**

**==> picture [57 x 57] intentionally omitted <==**

Host acknowledges QMI8658A when received the CmdDone information, to end the CTRL9 protocol.

## **5.10.6.2 CTRL_CMD_RST_FIFO**

This CTRL9 command of writing 0x04 to the Ctrl9 register 0x0a allows the host to instruct the device to reset the FIFO. The FIFO data, sample count and flags will be cleared and reset to default status.

## **5.10.6.3 CTRL_CMD_REQ_FIFO**

This CTRL9 Command is issued when the host wants to get data from the FIFO, by writing 0x05 through the CTRL9 process.

After successfully finish the CTRL9 process, the FIFO read mode will be enabled, the device will direct the FIFO data to the FIFO_DATA register(0x17) until the FIFO is empty. After reading the FIFO data, host must set FIFO_CTRL.FIFO_rd_mode to 0 by write the FIFO_CTRL register, which will cause the FIFO_STATUS.FIFO_WTM/FIFO_FULL to be cleared and/or the INT pin (if enabled) be de-asserted. Refer to 错误 _**!**_ 未 找到引用源。 错误 _**!**_ 未找到引用源。 for CTRL9 operation, and refer to _8.8_ for details.

## **5.10.6.4 CTRL_CMD_WRITE_WOM_SETTING**

This CTRL9 Command is issued when the host wants to enable/modify the trigger thresholds or blanking interval of the Wake on Motion Feature of the device. Please refer to _12 Wake on Motion (WoM)_ for details of setting up this feature. Once the specified CAL registers are loaded with the appropriate data, the Command is issued by writing 0x08 to CTRL9 register 0x0A.

## **5.10.6.5 CTRL_CMD_ACCEL_HOST_DELTA_OFFSET**

This CTRL9 Command is issued when the host wants to manually change the accelerometer offset. Each delta offset value should contain 16 bits and the format is signed 4.12 (12 fraction bits, unit is 1 / 2^12). The user must write the offset to the following registers:

Accel_Delta_X : {CAL1_H, CAL1_L} Accel_Delta_Y : {CAL2_H, CAL2_L} Accel_Delta_Z : {CAL3_H, CAL3_L}

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

43

Next, the Command is issued by writing 0x09 to CTRL9 register 0x0A. Note, this offset change is lost when the sensor is power cycled, or the system is reset.

## **5.10.6.6 CTRL_CMD_GYRO_HOST_DELTA_OFFSET**

This CTRL9 Command is issued when the host wants to manually change the gyroscope offset. Each delta offset value should contain 16 bits and the format is signed 11.5 (5 fraction bits, unit is 1 / 2^5). The user must write the offset to the following registers:

Gyro_Delta_X : {CAL1_H, CAL1_L} Gyro_Delta_Y : {CAL2_H, CAL2_L} Gyro_Delta_Z : {CAL3_H, CAL3_L}

Next, the Command is issued by writing 0x0A to CTRL9 register 0x0A. Note, this offset change is lost when the sensor is power cycled, or the system is reset.

## **5.10.6.7 CTRL_CMD_CONFIGURE_TAP**

This CTRL9 command is issued to configure the parameters of Tap detection. Refer to _10.3 Configure Tap for details._

## **5.10.6.8 CTRL_CMD_CONFIGURE_PEDOMETER**

This CTRL9 command is issued to configure the parameters of Pedometer detection. Refer to _11.2 Configure Pedometer_ for details.

## **5.10.6.9 CTRL_CMD_CONFIGURE_MOTION**

This CTRL9 command is issued to configure the parameters of Motion Detection. Refer to _9.4 Config Motion Detection._

## **5.10.6.10 CTRL_CMD_RESET_PEDOMETER**

This CTRL9 command is issued to clear the step count of Pedometer. Refer to _11.6 Reset Step Count_ for details.

**==> picture [47 x 47] intentionally omitted <==**

## **5.10.6.11 CTRL_CMD_COPY_USID**

The USID is the unique ID of every single QMI8658A part.

This CTRL9 Command copies the following data into UI registers. It is initiated by the host writing 0x10 to CTRL9. After issuing the command, the data will be available for the host to read from the registers shown below:

**==> picture [105 x 98] intentionally omitted <==**

FW_Version byte 0  → dQW_L FW_Version byte 1 → dQW_H FW_Version byte 2 → dQX_L USID_Byte_0 → dVX_L USID_Byte_1 → dVX_H USID_Byte_2 → dVY_L USID_Byte_3 → dVY_H USID_Byte_4 → dVZ_L USID_Byte_5 → dVZ_H

Note that after the successful Power-On Reset or Soft-Reset, the FW_Version and USID will be copied automatically to the according registers once for host to read them. Those registers can be changed after enabling the sensors, so afterwards, the CTRL_CMD_COPY_USID command should be implemented to copy the FW_Version and USID to the according registers before reading.

## **5.10.6.12 CTRL_CMD_SET_RPU**

This CTRL9 Command is issued when the host configure the IO pull-up resistors. Each bit controls a combination of resistors as shown in _Table 30_ :

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 44

**Table 30. Pull-Up Resistor Table**

|**Bit**|**Signal Name**|**Pins**|**Activity**|
|---|---|---|---|
|0|aux_rpu_dis|SDx, SCx, RESV(Pin 10)|0: enable pull-up resistors(default)<br>1: disable pull-up resistors|
|1|icm_rpu_dis|SDx|0: enable pull-up resistor(default)<br>1: disable pull-up resistor|
|2|cs_rpu_dis|CS|0: enable pull-up resistor(default)<br>1: disable pull-up resistor|
|3|i2c_rpu_dis|SCL, SDA|0: enable pull-up resistors(default)<br>1: disable pull-up resistors|
|4:7|Reserved|NA||
|rites t|e appropriate C|AL1L bit by issuing a WCtrl9 c||



The host writes the appropriate CAL1_L bit by issuing a WCtrl9 command with 0x11.

By default, all the pull-up resistors are enabled. Write 1 to the bit will disable the pullup resistors accordingly, while write 0 will enable the pull-up resistors.

## **5.10.6.13 CTRL_CMD_AHB_CLOCK_GATING**

When locking Mechanism is set (CTRL7.bit7 == 1(syncSmpl)), the CTRL_CMD_AHB_CLOCK_GATING should be disabled to guarantee the locking mechanism of data reading, to prevent the possible misalignment. Refer to _14 Calibration-On-Demand (COD)_ for details.

## **5.10.6.14 CTRL_CMD_ON_DEMAND_CALIBRATION**

This CTRL9 Command enables host to recalibrate the gyro sensitivity from time to time. Refer to _14 Calibration-OnDemand (COD)._

## **5.10.6.1 CTRL_CMD_ APPLY_GYRO_GAINS**

This CTRL9 Command enables host to restore the saved the gyro gains to QMI8658A, to avoid run the COD again. This is not recommended when there are significant environment changes, like significant PCB stress change. Refer to _14.4 Save and Restore the New Gain Parameters_ .

**==> picture [197 x 166] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 45

## **6 Interrupts**

The QMI8658A has two Interrupt lines, INT1 and INT2.

Both INT1 and INT2 can be configured as High-Z mode or Push-Pull mode by configure the CTRL1.bit3(INT1) or CTRL1.bit4(INT2). If CTRL1.bit3 (CTRL1.bit4) is set to 0, INT1(INT2) will be set in High-Z mode accordingly. While if CTRL1.bit3 (CTRL1.bit4) is set to 1, INT1(INT2) will be set in Push-Pull mode accordingly. By default, INT1 and INT2 are in High-Z mode.

If QMI8658A is configured in Wake on Motion (WoM) mode, there is no sensor data generated. The INT pins behavior follows the configuration of WoM, refer to _12 Wake on Motion (WoM)_ .

If QMI8658A is not in Wake on Motion mode, there are two modes of the interrupt map, as described below. Host can config multiple internal signal/interrupt sources to INT pins (INT1 and/or INT2). If driven to one INT pin, the multiple sources act in LOGIC-OR.

## 6.1 **SyncSample mode**

The SyncSample mode supports locking the values during reading process. Refer to _13 Locking Mechanism._ For details to the sensor data registers.

.

Set CTRL7.bit7(SyncSample) == 1 will enable the SyncSample mode.

As illustrated in _Figure 14_ . In SyncSample mode, the CTRL9 handshake signal will be routed to INT1. Check _5.10_ for details.

The motion event interrupts (Any Motion, No Motion, Significant Motion, Pedometer, Tap) will be routed to INT1 if enabled.

FIFO function is not supported in this mode, and DRDY signal will be routed to INT2.

**==> picture [465 x 286] intentionally omitted <==**

**Figure 14. Interrupt Map of SyncSample Mode**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 46

## 6.2 **Non-SyncSample mode**

This mode supports FIFO function and free interrupts configuration, as illustrated in _Figure 15_ .

If CTRL7.bit7(SyncSample) == 0, then bit 1 of STATUSINT register will have the same value as INT1 and bit 0 of STATUSINT register will have the same value as INT2.

In Non-SyncSample mode, there are two approaches of CTRL9 handshake. Host can check the INT1 pin high level for the handshark if set CTRL8.bit7 = 0; or poll the STATUSINT.bit7 for handshake if set CTRL8.bit7 = 1.

In Non-SyncSample mode, the motion event interrupt(s) can be configured to INT1 by setting CTRL8.bit6 = 1, or to INT2 by setting CTRL8.bit6 = 0. Note that the motion event engines can be enabled by CTRL8.bit[4:0], refer to _Table 22_ for details.

In Non-SycnSample mode, the sensor data can be output through data register or FIFO. Configure the FIFO_CTRL.FIFO_MODE = ‘bypass’ mode, will enable the DRDY function and disable FIFO functionality; configure the FIFO_CTRL.FIFO_MODE = other mode, will enable the FIFO functionality and disable the DRDY function.

If FIFO mode is enabled, the FIFO interrupt can be configured to INT1 pin if CTRL1.bit2 is set to 1, or INT2 pin if CTRL1.bit2 is set to 0. Refer to _8 FIFO Description_ for more details of FIFO interrupt behavior.

If DRDY mode is enabled, the DRDY signal will be routed to INT2 if the CTRL7.bit5(DRDY_DIS) is set to 0, or be blocked from INT2 pin if CTRL7.bit5(DRDY_DIS) is set to 1.

**==> picture [460 x 283] intentionally omitted <==**

**==> picture [226 x 9] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 15.  Interrupt Map of Non-SyncSample Mode<br>**----- End of picture text -----**<br>


## 6.3 **DRDY(Data Ready)**

If DRDY mode is enabled (accelerometer and/or gyroscope are/is enabled, and FIFO mode is set to bypass) and DRDY_DIS == 0, DRDY (Data Ready) signal will be driven to the INT2 in edge-trigger mode, means the DRDY signal can be seen on INT2. The Sensor Data Output Registers(refer to _5.6_ ) are updated at the Output Data Rate (ODR), and DRDY signal is pulsed at the ODR frequency. The new data is updated to the data registers during the low level before the rising edge, and a rising edge on DRDY indicates that data is available for host to read. DRDY is cleared automatically after a short duration. The DRDY pulse width is dependent on the sensor ODR set by CTRL2 and/or CTRL3 registers and enabled sensor(s).

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

47

In Non-SyncSample mode, it is the responsibility of the host to detect the rising edge and to read the data out during the high level of the INT2 pulse. Otherwise, there is the possibility that the updating of the new data happens during the host reading process and causes data mismatch.

In SyncSample mode, it is possible to lock the data in the data registers and read them in an unlimited delay, by following a process. Refer to _6.1 SyncSample mode._

**==> picture [448 x 438] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 48

## **7 Operating Modes**

The QMI8658A offers a large number of operating modes that may be used to operate the device in a power efficient manner. These modes are described in _Table 31_ and are shown in _Figure 16_ ; they may be configured using the control (CTRL) registers.

## **7.1 Operating Modes Descriptions**

**Table 31. Operating Modes**

**==> picture [469 x 459] intentionally omitted <==**

**----- Start of picture text -----**<br>
Mode  Description  Suggested Configuration<br>All sensors off, clock is turned on. The current in this mode  CTRL1 sensorDisable = 0<br>Power-On Default  is typically 15 µA. Note this mode is the default state upon  CTRL7 aEN = 0, gEN = 0,<br>initial power up or after a reset.  CTRL2 aODR =000<br>Same as Power-On Default mode, except in this mode the<br>CTRL1 sensorDisable =0<br>250 kHz clock is turned on instead of the high-speed clock.<br>Low Power  CTRL7 aEN = 0, gEN = 0,<br>The current in this mode is typically 8 µA. To enter this<br>CTRL2 aODR =11xx<br>mode requires host interaction to set CTRL2 aODR=11xx.<br>All QMI8658A functional blocks are switched off to<br>minimize power consumption. Digital interfaces remain on<br>allowing communication with the device. All configuration<br>CTRL1 sensorDisable =1<br>Power-Down  register values are preserved, and output data register<br>CTRL7 aEN = 0, gEN = 0<br>values are maintained. The current in this mode is typically<br>6 µA. The host must initiate this mode by setting<br>sensorDisable=1.<br>CTRL7 aEN =1, gEN =0,<br>Normal Accel Only   Device configured as an accelerometer only.  CTRL2 aODR !=11xx<br>CTRL7 aEN =1, gEN =0,<br>Low Power Accel Only  Device configured in low power accelerometer mode.  CTRL2 aODR =11xx<br>Device configured as gyroscope drive only, the gyroscope<br>MEMS will keep running at resonance frequency. Since the<br>Sensing part is not enabled, there is no data from the  CTRL7 gSN=1, aEN =0, gEN<br>Snooze Gyro  gyroscope in this mode. This mode enables relative lower  =1<br>current consumption than Gyro-Only mode and can quickly<br>generate data from clearing of gSN.<br>CTRL7 gSN=0, aEN =0, gEN<br>Gyro Only  Device configured as a gyroscope only.  =1<br>CTRL7 gSN=0, aEN =1, gEN<br>Accel + Gyro (IMU)  Device configured as an Inertial Measurement Unit, i.e. an  =1<br>accelerometer and gyroscope combination sensors.<br>CTRL2 aODR != 11xx<br>CTRL7 gSN=1, aEN =1, gEN<br>Accel + Snooze Gyro  Accelerometer and gyroscope snooze are enabled. Only accelerometer data is available.  =1<br>CTRL2 aODR != 11xx<br>Very low power mode used to wake-up the host by<br>CTRL7 aEN =1, gEN =0,<br>Wake on Motion (WoM)  providing an interrupt upon detection of device motion.<br>CTRL2 aODR = 11xx<br>Rrefer to 5.10.6.4.<br>Software Reset  Software Reset asserted<br>No Power  VDDIO and VDD low<br>**----- End of picture text -----**<br>


## **7.2 General Mode Transitioning**

Upon exiting the No Power state (i.e. on first applying power to the part) or exiting a Software Reset state, the part will enter the Power-On Default state. From there, the sensor can be configured in the various modes described in Table 31 and as shown in Figure 16. The figure illustrates the timing associated with various mode transitions, and values for these times are given in the section below and in Table 7 and Table 8.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

49

**==> picture [475 x 440] intentionally omitted <==**

**----- Start of picture text -----**<br>
From Any State  From Any State<br>Soft-Reset  No Power<br>Wake-On Motion  Power Down<br>(WOM)  (All sensors off, clock<br>aODR = 15  t2 + t5  t7  SensorDisable = 1 off)<br>t0  t0<br>t2 + t5<br>t6  t6<br>Power-on Default<br>t7  All sensors off, normal  t6<br>clock<br>Low Power Mode  t7  t1 + t5<br>(All sensors off,  Accel + Gyro<br>slow clock)  aODR = 0 - 8<br>aODR = 12-15  t6  gODR = 0 - 7<br>t6<br>t6  t6<br>t4 + t5<br>t2 + t5  t2 + t5<br>t1 + t5<br>Low Power Accel<br>Only  t2 + t5<br>aODR = 12-15<br>Gyro Only<br>gODR = 0 - 7<br>Accel Only<br>aODR = 0 - 8<br>**----- End of picture text -----**<br>


**==> picture [46 x 47] intentionally omitted <==**

**Figure 16. Operating Mode Transition Diagram**

## **7.3 Transition Time**

t0 is the _System Turn On Time_ and is the time to enter the Power-On Default state from Software Reset, No Power, or Power down. Time t0 is maximum 15ms. This time only needs to be done once, upon transitioning from either a No Power or Power Down state, or whenever a reset is issued, which should not be done unless the intent is to have the device to go through its entire boot sequence (see the specification System Turn On Time in both Table 7 and Table 8).

The times t1, t2 and t4, are defined as the time it takes from issuing the enabling-sensor command (when the corresponding sensor is off) to DRDY(INT2) going high (data being present) or STATUSINT.bit0 is set to 1.

The time t5 is the time it takes to have the internal filtered data to output the correct representation of the inertial state. t5 is variable and is associated with the user selected Output Data Rate (ODR). We have defined minimum t5 = (3/ODR) to generally represent that time.

t6 is the time it takes to go from a sensor powered state to a state where the sensors are off. This time depends on the Output Data Rate (ODR) and ranges from 1/ODR to 2/ODR.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 50

t7 is the transition time between various states where the sensors are off.

The Gyro Turn on Time (see Table 8) is comprised of t1 (the gyroscope wakeup time) and t5 (the part’s filter settling time). t1 is typically 150 ms and t5 is defined as 3/ODR, where ODR is the output data rate in Hertz.

The Accel Turn on Time (see Table 7) is comprised of t2 (the accelerometer wakeup time) and t5 (the part’s filter settling time). t2 is typically 3 ms, and t5 is defined as 3/ODR, where ODR is the output data rate in Hertz.

The t7 transition is dependent on data transfer rates and is for I2C at 400 kHz is <100 µs for SPI at 11 Mbps is around 40 µs.

## **7.4 Chip Reset Process**

There are two approaches that can trigger the QMI8658A run the Reset process, Power-On Reset and Software Reset.

Power-On reset is initialized by driving the VDD & VDDIO lines to valid working range from the Power Off status (VDD = 0V, VDDIO = 0V). Refer to _3.2_ for details. The Power-On Reset process starts from the release of POR, refer to _3.3_ for details.

The Software Reset (Soft Reset) is generated by writing the 0x0B to RESET register (0x60). Refer to _5.9_ .

After the Reset is triggered (Power-On Reset and Software Reset), the QMI8658 will run the reset process. The UI registers, internal RAM, FIFO will be set to default values, Analog and digital circuitries will be disabled, refer to _3.3_ for details.

It takes maximum 15ms for the Reset process to be finished. Refer to _Table 7_ and _Table 8_ for details. Note that the VDDIO & VDD power lines are expected to be stable(no sharp pulse) once settled to the Final Value(s), before the System Turn On Time. Otherwise, the QMI8658A initialization may be interfered by the unwanted power pulse, and result in failure of accelerometer or gyroscope startup, refer to _3.3_ for details.

The register 0x4D will present 0x80 if there is a successful Reset (Power-on Reset or Soft Reset) process. Refer to _5.9_ . Note that the content of register 0x4D could be overwritten after later operations, like enabling the sensor(s) (in CTRL7.bit[1:0]) or implementing the CTRL9 command, so host is expected to read the register immediately after POR or Software Reset to check the Reset result.

**==> picture [228 x 228] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 51

## **8 FIFO Description**

## **8.1 FIFO Structure**

The QMI8658A contains a programmable 1536-byte FIFO. The FIFO’s operating mode and configuration are set via the FIFO_CTRL register, refer to _Table 23_ . FIFO data may consist of gyroscope and accelerometer data and is accessible via the serial interfaces (SPI/I2C/I3C), in burst reads. Depending on how many sensors are enabled, the host is expected to read increments of 6, 12 bytes, corresponding to one and two sensors active at the same time. This feature helps reduce overall system power consumption by enabling the host processor to read and process the sensor data in bursts and then enter a low-power mode. The interrupt function may be used to alert host when FIFO watermark level is reached.

**==> picture [308 x 257] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 17.  FIFO Data Flow<br>**----- End of picture text -----**<br>


## **8.2 FIFO Size**

There are four levels of FIFO size: 16 samples, 32 samples, 64 samples, 128 samples. The sample stands for 6 bytes of accelerometer data or 6 bytes of gyroscope data if one of them is enabled, or 6 bytes of accelerometer and 6 bytes of gyroscope data (total 12 bytes) if both are enabled.

The FIFO size is configured using the FIFO_CTRL.FIFO_SIZE[1:0]: 16 samples(0), 32 samples(1), 64 samples(2), 128 samples(3), refer to _Table 23._

When the FIFO is enabled for two sensors (Accelerometer and Gyroscope), the sensors must be set at the same Output Data Rate (ODR), refer to _Table 22_ for CTRL2 and CTRL3 registers.

## **8.3 Configure FIFO Mode**

The FIFO has multiple operating modes: Bypass, FIFO, Stream. The operating modes are set by the FIFO_CTRL.FIFO_MDOE[1:0] bits in the register, refer to _Table 23._

Configure the FIFO_CTRL.FIFO_MODE to _‘FIFO’_ (1) or _‘Stream’_ (2) mode will enable the FIFO functionality. Configure the FIFO_MODE to _‘Bypass’_ (0) mode, will disable the FIFO functionality.

Once FIFO is enabled and the Accelerometer and/or Gyroscope is(are) enabled, the corresponding data will be filled into FIFO.

In ‘ _FIFO_ ’ mode, once FIFO is full, the data filling will stop and new data will be discarded until host reads out the FIFO data and release the space for new data to be written to.

In ‘ _Stream’_ mode, once FIFO is full, the data filling will continue and the oldest data will be discarded, until host reads out the FIFO data and release the space for new data to be written to.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 52

## **8.4 FIFO Sample Count**

The FIFO Sample Count indicates the filled content level of FIFO, which means the quantity of sensor data that was written into FIFO, and to be read out.

The FIFO Sample Count is a 10-bit value and stored in FIFO_SMPL_CNT and FIFO_STATUS[1:0], unit is word(two bytes). Host need to read the two registers, to calculate the number of bytes of FIFO content data by below formular:

FIFO_Sample_Count (in byte) = 2 * (fifo_smpl_cnt_msb[1:0] * 256 + fifo_smpl_cnt_lsb[7:0])

## **8.5 FIFO Watermark Interrupt**

The FIFO_WTM register(0x13) indicates the expected level of FIFO data that host wants to get the FIFO Watermark interrupt. The unit is sample, which means 6 bytes if one of accelerometer and gyroscope is enabled, and 12 bytes if both are enabled.

Note that the configured FIFO watermark should not be higher than the FIFO size configured by FIFO_CTRL.FIFO_SIZE.

If the FIFO_WTM is written with non-zero value, the FIFO watermark function is enabled. The internal FIFO watermark interrupt will be triggered if the FIFO content level reaches or is higher than the configured FIFO watermark level. The FIFO_STATUS.FIFO_WTM flag will be set to 1, and will be cleared to 0 if FIFO content drops lower than the FIFO watermark level.

The internal FIFO watermark interrupt signal can drive the INT1 pin if CTRL1.bit2 = 1, or INT2 pin if CTRL1.bit2 = 0. Note that the CTRL1.bit3(INT1) and CTRL1.bit4(INT2) controls the high-Z or push-pull mode of the INT pins, refer to _6 Interrupts_ for details.

Once the corresponds INT pin is configured to the push-pull mode, the FIFO watermark interrupt can be seen on the corresponds INT pin. It will keep high level as long as the FIFO filled level is equal to or higher than the watermark, will drop to low level as long as the FIFO filled level is lower than the configured FIFO watermark after reading out by host and FIFO_RD_MODE is cleared.

## **8.6 FIFO Full**

The FIFO_STATUS.FIFO_FULL flag is set if the FIFO filled level (samples) equals to the FIFO size configured by FIFO_CTRL.FIFO_SIZE. And is cleared if FIFO filled level is lower than the FIFO size.

## **8.7 FIFO Read Mode**

To read out the FIFO data, host need to set the FIFO into Read Mode, by issuing CTRL_CMD_REQ_FIFO command through CTRL9 process, refer to _5.10 CTRL 9 Functionality (Executing Pre-defined Commands)_ .

Once FIFO Read Mode is enabled, FIFO data will be directed to the FIFO_DATA register, then host can possibly read data from the FIFO_DATA register.

Note that once FIFO Read Mode is enabled, the new data won’t be filled into FIFO, and will be discarded. Host is expected to read out the FIFO data and disable the FIFO Read Mode before the new data come. Otherwise, data dropping will happen. The time for the new data come is defined by the sensor ODR, which is 1 / ODR.

## **8.8 Read FIFO Data**

The FIFO data is read through the I[2] C/I[3] C/SPI interface by reading the FIFO_DATA register. Any time the  FIFO_DATA register is read, data is erased from the FIFO memory, corresponds to First-In-First-Out concept.

Host is expected to read out the FIFO data, following below sequence:

1. Got FIFO watermark interrupt by INT pin or polling the FIFO_STATUS register (FIFO_WTM and/or FIFO_FULL).

2. Read the FIFO_SMPL_CNT and FIFO_STATUS registers, to calculate the level of FIFO content data, refer to 8.4 FIFO Sample Count.

3. Send CTRL_CMD_REQ_FIFO (0x05) by CTRL9 command, to enable FIFO read mode. Refer to _CTRL_CMD_REQ_FIFO_ for details.

4. Read from the FIFO_DATA register per FIFO_Sample_Count.

5. Disable the FIFO Read Mode by setting FIFO_CTRL.FIFO_rd_mode to 0. New data will be filled into FIFO afterwards.

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

© 2022 QST Corporation

www. qstcorp.com 53

## **8.9 FIFO Data Pattern**

Note that when only the accelerometer or gyroscope is enabled, the sensor data format at the host interface is:

AX_L[0]AX_H[0]AY_L[0]AY_H[0]AZ_L[0]AZ_H[0]AX_L[1]… or

GX_L[0]GX_H[0]GY_L[0]GY_H[0]GZ_L[0]GZ_H[0]GX_L[1]…

When both accelerometer and gyroscope are enabled, the sensor data format is:

AX_L[0]AX_H[0]AY_L[0]AY_H[0]AZ_L[0]AZ_H[0] GX_L[0]GX_H[0]GY_L[0]GY_H[0]GZ_L[0]GZ_H[0] AX_L[1]AX_H[1]…

## **8.10 Reset FIFO**

The FIFO content can be cleared/emptied by inserting CTRL_CMD_RST_FIFO through CTRL9 process, refer to 5.10 CTRL 9 Functionality (Executing Pre-defined Commands) for details.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 54

## **9 Motion Detection**

The calculation of the Motion Detection (No-, Any-, or Significant-Motion) is based on the accelerometer ODR defined by _CTRL2.aODR_ , refer to _Table 22_ for details.

The Motion Detection can only work in Non-SyncSample mode, refer to _6.2 Non-SyncSample mode_ for details.

No-, Any-, or Significant-Motion interrupts can be issued to host. No-Motion interrupt indicates that the device is in idle/quiet status, host can run into sleep or low-power mode. Any-Motion interrupt indicates that the device is in movement, host can be awakened from sleep/low-power mode. Significant-Motion indicates host that the device is in significant and continuous movement.

## **9.1 Motion Detection Principle**

Any-/No-/Significant-Motion detection is calculating on the slope of the acceleration of enabled axis:

Slope(n) = Acc(n) – Acc(n-1), for enabled axis/axes of accelerometer

## 9.1.1 **Any-Motion Detection Principle**

Any-Motion Detection is detecting the absolute of slope that is higher than defined threshold ( _AnyMotionXThr, AnyMotionYThr, AnyMotionZThr_ ), and last consecutively for _AnyMotionWindow_ or more samples. As shown in _Figure 18_ . As long as the conditions (threshold and duration) are fulfilled, Any-Motion event flag is set, and can be configured to drive the interrupt pin (INT1 or INT2), synced to the DRDY. When the slope falls within the range of _(-Threshold, +Threshold),_ the Any-Motion conditions are not fulfilled, and the corresponding interrupt is reset.

**==> picture [415 x 255] intentionally omitted <==**

**Figure 18. Any-Motion Detection Interrupt**

## 9.1.2 **No-Motion Detection Principle**

No-Motion Detection is detecting the absolute of slope that is lower than defined threshold ( _NoMotionXThr, NoMotionYThr, NoMotionZThr_ ), and last consecutively for _NoMotionWindow_ or more samples. As shown in _Figure 19_ . If the conditions (threshold and duration) are fulfilled, No-Motion event flag is set, and can be configured to drive the interrupt pin (INT1 or INT2), synced to the DRDY. When the slope falls out of the range of _(-Threshold, +Threshold),_ the No-Motion conditions are not fulfilled, and the corresponding interrupt is reset.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

55

**==> picture [443 x 270] intentionally omitted <==**

**Figure 19. No-Motion interrupt detection**

## 9.1.3 **Significant-Motion Detection Principle**

Significant-Motion Detection works based on Any-Motion and No-Motion events, so both Any-Motion and No-Motion should be configured and enabled to make sure Significant-Motion Detection run properly. As shown in _Figure 20_ .

After valid Any-Motion is detected, if a further Any-Motion is detected after _SigMotionWaitWindow_ and before the _SigMotionConfirmWindow_ time, the Significant-Motion event flag will be raised to the interrupt path. The SignificantMotion interrupt is cleared when No-Motion is detected.

**==> picture [329 x 222] intentionally omitted <==**

**Figure 20. Significant-Motion interrupt detection**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 56

## **9.2 Motion Detection Flow**

The flow chart of No-Motion Detection is show in Figure 21 No-Motion Detection Flow.

The flow chart of Any-Motion Detection is show in Figure 22 Any-Motion Detection Flow.

The flow chart of Significant-Motion Detection is show in Figure 23 Significant-Motion Detection Flow.

**==> picture [490 x 62] intentionally omitted <==**

## **Figure 21. No-Motion Detection Flow**

**==> picture [486 x 61] intentionally omitted <==**

**Figure 22. Any-Motion Detection Flow**

**==> picture [483 x 164] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 23.  Significant-Motion Detection Flow<br>**----- End of picture text -----**<br>


**==> picture [121 x 121] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 57

## **9.3 Parameters of Motion Detection**

**Table 32. Motion Detection Parameters**

**==> picture [486 x 499] intentionally omitted <==**

**----- Start of picture text -----**<br>
Parameter Name  Format   Resolution Description<br>0: Logic-OR between events of enabled axes for No-Motion<br>detection<br>NoMotionAxisLogic  1-bit integer  1<br>1: Logic-AND between events of enabled axes for No-Motion<br>detection<br>NoMotionEnX<br>0: the corresponding axis is not involved for calculation<br>NoMotionEnY  1-bit integer  1<br>1: the corresponding axis data is calculated for No-Motion detection<br>NoMotionEnZ<br>NoMotionXThr  1-byte format<br>unsigned,   0.03125g  Defines the slope threshold of the corresponding axis for No-Motion<br>NoMotionYThr<br>5-bits  (1 / 32)   detection<br>NoMotionZThr   fraction<br>Defines the minimum number of consecutive samples (duration) that<br>1-byte<br>NoMotionWindow  1 sample  the absolute of the slope of the enabled axis/axes data should keep<br>integer<br>lower than the threshold<br>0: Logic-OR between events of enabled axes for Any-Motion<br>detection<br>AnyMotionAxisLogic  1-bit integer  1<br>1: Logic-AND between events of enabled axes for Any-Motion<br>detection<br>AnyMotionEnX<br>0: the corresponding axis is not involved for calculation<br>AnyMotionEnY  1-bit integer  1<br>1: the corresponding axis data is calculated for Any-Motion detection<br>AnyMotionEnZ<br>AnyMotionXThr  1-byte format<br>unsigned,   0.03125g  Defines the slope threshold of the corresponding axis for Any-Motion<br>AnyMotionYThr<br>5-bits  (1 / 32)  detection<br>AnyMotionZThr  fraction<br>1-byte format<br>unsigned,   1 sample  Defines the minimum number of consecutive samples (duration) that<br>AnyMotionWindow  the absolute of the slope of the enabled axis/axes data should keep<br>5-bits  higher than the threshold<br>fraction<br>Defines the wait window (idle time) starts from the first Any-Motion<br>SigMotionWaitWindow  [2-bytes ] 1 sample  event until starting to detecting another Any-Motion event for<br>integer<br>confirmation<br>Defines the maximum duration for detecting the other Any-Motion<br>SigMotionConfirmWind 2-bytes<br>1 sample  event to confirm Significant-Motion, starts from the first Any-Motion<br>ow  integer<br>event<br>**----- End of picture text -----**<br>


**Note** : Once the Logic-OR is selected, the calculation on any enabled axis and reported one valid event, will trigger the corresponding Motion Detection(Any-Motion or No-Motion). Once the Logic-AND is selected, the calculation on all enabled axes should report valid event, to trigger the corresponding Motion Detection(Any-Motion or No-Motion).

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

58

The MOTION_MODE_CTRL byte consists of the parameters shown in _Table 33_ .

## **Table 33. MOTION_MODE_CTRL Content**

|7|6|5|4|3|2|1|0|
|---|---|---|---|---|---|---|---|
|NoMotionAxisLogic|NoMotionEnZ|NoMotionEnY|NoMotionEnX|AnyMotionAxisLogic|AnyMotionEnZ|AnyMotionEnY|AnyMotionEnX|



## **9.4 Config Motion Detection**

The Motion Detection parameters are divided into two sets and can be passed to the QMI8658A internal algorithm through two callings of CTRL9 command. As shown in _Table 34_ .

Host should write the parameters to the corresponding registers, according to _Table 34_ . Especially, write 0x01 to CAL4_H register for the first set of parameters, while write 0x02 to CAL4_H for the second set of parameters. Refer to _Table 29_ for address of CAL registers.

Then trigger the CTRL9 command with 0x0E(CTRL_CMD_CONFIGURE_MOTION). Refer to _Table 28_ for details.

**Table 34. Write Motion Detection Parameters to QMI8658A**

|**Table 34. Write**|**Motion Detection Parameters to QMI865**|**8A**|
|---|---|---|
|Register (bits)|First CTRL9 Command|Second CTRL9 Command|
|CAL1_L (7:0)|AnyMotionXThr.|AnyMotionWindow.|
|CAL1_H (7:0)|AnyMotionYThr.|NoMotionWindow|
|CAL2_L (7:0)|AnyMotionZThr.|SigMotionWaitWindow[7:0]|
|CAL2_H (7:0)|NoMotionXThr.|SigMotionWaitWindow [15:8]|
|CAL3_L (7:0)|NoMotionYThr.|SigMotionConfirmWindow[7:0]|
|CAL3_H (7:0)|NoMotionZThr.|SigMotionConfirmWindow[15:8]|
|CAL4_L (7:0)|MOTION_MODE_CTRL|NA|
|CAL4_H (7:4)|0x01(means 1st command)|0x02(means 2nd command)|
|CTRL9 code|0x0E(CTRL_CMD_CONFIGURE_MOTION)|0x0E(CTRL_CMD_CONFIGURE_MOTION)|



**Note** : Configuration should be done when accelerometer and gyroscope are disabled(CTRL7.aEN = CTRL7.gEN =0).

## **9.5 Enabling Motion Detection**

After successfully passing the parameters to QMI8658A Motion Detection engine, host need to enable the Any-Motion Detection engine by setting CTRL8.bit1 to 1, enable the No-Motion Detection engine by setting CTRL8.bit2 to 1, enable the Significant-Motion Detection engine by setting CTRL8.bit3 to 1. If the accelerometer is configured and enabled too, the enabled engines will be started to detect the corresponding events.

On contrary, set the bit(s) in CTRL8.bit[3:1] to 0, will disable the corresponding engine. This can be done when accelerometer and/or gyroscope are(is) enabled or disabled. Refer to _5.3 Configuration_ Registers for the details of CTRL8.

## **9.6 Motion Interrupt**

Once the Any-Motion, No-Motion and/or Significant-Motion event(s) is/are reported, the corresponding Motion Detection Event will be generated.

The Motion Event (internal signal) can be selected to drive INT1(CTRL8.bit6 = 1) or INT2(CTRL8.bit6 = 0). And once the corresponding INT pin is enabled (by CTRL1.bit3 for INT1, or CTRL1.bit4 for INT2), the Motion interrupt can be seen on the INT pin (synced with DRDY). Refer to _5.3_ Configuration Registers for details.

At the meantime, the STATUS1.bit[7:5] is updated and host can read these bits to confirm the Motion interrupt status. Note that the STATUS1.bit[5] = 1 represents the Any-Motion was detected, STATUS1.bit[6] = 1 represents the NoMotion was detected, STATUS1.bit[7] = 1 represents the Significant-Motion was detected. Refer to _5.3_ Configuration Registers for details.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 59

## **10 Tap**

The Tap engine detects the Single-Tap or Double-Tap, if enabled.

The calculation of the Tap Detection is based on the accelerometer ODR defined by _CTRL2.aODR_ , refer to _Table 22_ for details.

The Tap detection can only work in Non-SyncSample mode, refer to for details 6.2 Non-SyncSample mode.

## **10.1 Tap Detection Principle**

_Figure 24_ shows the principle of Tap detecting, includes Single Tap and Double Tap.

The acceleration data of the three axes (x, y, z) is dynamically averaged to get the Average of Acceleration. Alpha parameter defines the ratio/weight of the averaging calculation.

The Linear Acceleration is calculated: Linear Acceleration = Acceleration – Average of Acceleration.

The Average Movement Magnitude indicates the movement energy level, is used to detect the Quiet status. It is calculated with _Gamma._

If the square sum of the Linear Acceleration of three axes is higher than the _PeakMagThr_ , the peak detecting is started. If later at the end of _PeakWindow_ , the Average of the Movement Magnitude is lower than the _UDMThr (Undefined Motion Threshold)_ , it means the vibration is low and return to Quiet status, it is considered a valid Peak is detected.

If a valid Peak is detected, and no further significant vibration within the _TapWindow_ (be quiet after the Peak, no further Tap), a valid Tap is detected.

Once a valid Tap (first Tap) is detected, the second Tap of the Double-Tap should be detected after the TapWindow (Quiet time after first Tap) and before the _DTapWindow_ (timeout of the Double Tap detection), and the Double-Tap event will be reported.

If the second Tap is detected within the _TapWindow_ , it is considered as the Undefined Motion, and will reset the full Tap process, no Tap event will be reported.

If no further Tap is detected within the _DTapWindow_ , Single-Tap event will be reported. The Tap detected after _DTapWindow_ , is considered as the First Tap of the new round of detecting.

**==> picture [477 x 191] intentionally omitted <==**

**Figure 24. Tap Detect Principle**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 60

## **10.2 Tap Detection Parameters**

The parameters listed in _Table 35_ are used to configure the Tap Engine.

**Table 35. Tap Parameters**

**==> picture [496 x 481] intentionally omitted <==**

**----- Start of picture text -----**<br>
Parameter Name Format   Resolution  Description<br>Priority definition between the x, y, z axes of acceleration. Only<br>Priority[2:0] bits are used.<br>The axis that output the first peak of Linear Acceleration in a valid<br>Priority  1-byte integer  NA  Tap detection, will be consider as the Tap axis. However, there is<br>possibility that two or three of the axes shows same Linear<br>Acceleration at exactly same time when reach (or be higher than)<br>the PeakMagThr. In this case, the defined priority is used to judge<br>and select the axis as Tap axis.<br>Defines the maximum duration (in sample) for a valid peak. In a<br>valid peak, the linear acceleration should reach or be higher than<br>PeakWindow  1-byte integer  1 sample  the  PeakMagThr  and should return to quiet (no significant<br>movement) within  UDMThr , at the end of  PeakWindow .<br>E.g., 20 @500Hz ODR<br>Defines the minimum quiet time before the second Tap happen.<br>After the first Tap is detected, there should be no significant<br>movement (defined by  UDMThr ) during the  TapWindow . The valid<br>TapWindow     2-bytes integer  1 sample  second tap should be detected after  TapWindow  and before<br>DTapWindow .<br>E.g., 50 @500Hz ODR<br>Defines the maximum time for a valid second Tap for Double Tap,<br>DTapWindow  2-bytes integer  1 sample  count start from the first peak of the valid first Tap.<br>E.g., 250 @500Hz ODR<br>Defines the ratio for calculation the average of the acceleration.<br>1-byte unsigned,  0.0078<br>Alpha     The bigger of  Alpha , the bigger weight of the latest data.<br>7-bits fraction  (1/128)<br>E.g., 0.0625<br>Defines the ratio for calculating the average of the movement<br>1-byte unsigned,  0.0078  magnitude. The bigger of  Gamma , the bigger weight of the latest<br>Gamma  data.<br>7-bits fraction  (1/128)<br>E.g., 0.25<br>2-bytes<br>0.001g [2] Threshold for peak detection.<br>PeakMagThr  unsigned,<br>(1/ 1024)  E.g, 0.8g [2]  (0x0320)<br>10-bits fraction<br>2-bytes  Undefined Motion threshold. This defines the threshold of the<br>0.001g<br>UDMThr  unsigned,   Linear Acceleration for quiet status.<br>(1/ 1024)<br>10-bits fraction  E.g., 0.4g [2]  (0x0190)<br>**----- End of picture text -----**<br>


The _Priority[2:0]_ is defined as below: 0: (X > Y> Z) 1: (X > Z > Y) 2: (Y > X > Z) 3: (Y > Z > X) 4: (Z > X > Y) 5: (Z > Y > X) 6, 7: (Same as 0)

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 61

## **10.3 Configure Tap**

The Tap parameters are divided into two sets and can be passed to the QMI8658A internal algorithm through two callings of CTRL9 command. As shown in _Table 36_ .

Host should write the parameters to the corresponding registers, according to _Table 36_ . Especially, write 0x01 to CAL4_H register for the first set of parameters, while write 0x02 to CAL4_H for the second set of parameters. Refer to _Table 29_ for address of CAL registers.

Then trigger the CTRL9 comment with 0x0C(CTRL_CMD_CONFIGURE_TAP). Refer to _5.10_ for details.

**Table 36. Write Tap Parameters to QMI8658A**

|**Table 36. Write**|**Tap Parameters to QMI8658A**||
|---|---|---|
|Register (bits)|First Command Set|Second Command Set|
|CAL1_L (7:0)|PeakWindow[7:0]|Alpha[7:0]|
|CAL1_H (7:0)|Priority[7:0] (actually only [2:0] is used)|Gamma[7:0]|
|CAL2_L (7:0)|TapWindow[7:0]|PeakMagThr[7:0]|
|CAL2_H (7:0)|TapWindow[15:8]|PeakMagThr[15:8]|
|CAL3_L (7:0)|DTapWindow[7:0]|UDMThr[7:0]|
|CAL3_H (7:0)|DTapWindow[15:8]|UDMThr[15:8]|
|CAL4_L (7:0)|NA|NA|
|CAL4_H (7:4)|0x01(means 1st command)|0x02(means 2nd command)|
|CTRL9 code|0x0C(CTRL_CMD_CONFIGURE_TAP)|0x0C(CTRL_CMD_CONFIGURE_TAP)|



**Note** : Configuration should be done when accelerometer and gyroscope are disabled(CTRL7.aEN = CTRL7.gEN =0).

## **10.4 Enable Tap Detection**

After successfully passing the parameters to QMI8658A Tap engine, host need to enable the Tap engine by setting CTRL8.bit0 to 1. If the accelerometer is properly configured and enabled (CTRL7.aEN = 1), Tap engine will be started to detect the taps. To detecting the Tap activity, it is recommended to set accelerometer ODR to higher than 200Hz (defined by CTRL2.aODR).

If CTRL8.bit0 is set to 0, or CTRL7.aEN = 0, Tap engine will be stopped to detect the taps.

## **10.5 Tap Interrupt**

Once the Tap event is reported (Single- or Double-Tap), it can generate the Tap interrupt.

The Tap event (internal signal) can be selected to drive INT1(CTRL8.bit6 = 1) or INT2(CTRL8.bit6 = 0). And once the corresponding INT pin is enabled (by CTRL1.bit3 for INT1, or CTRL1.bit4 for INT2), the Tap interrupt will be seen on the INT pin (synced with DRDY). Refer to to _5.3 Configuration_ Registers for details.

At the meantime, the STATUS1.bit1 is set and host can read this bit to confirm the Tap (Single- or Double-Tap) interrupt is generated.

## **10.6 Tap Detection Output**

When the Tap (Single- or Double-Tap) is detected, the information of the Tap is presented in TAP_STATUS (register 0x59). The TAP_STATUS is updated and valid after the Tap event is detected (STATUSINT.bit1 = 1).

The TAP_NUM indicate the Single-Tap (TAP_STATUS.TAP_NUM = 1) or Double-Tap (TAP_STATUS.TAP_NUM = 2) was detected.

The TAP_STATUS.TAP_AXIS indicates the first valid peak of the Tap happens on X axis (TAP_STATUS.TAP_AXIS = 1), or Y axis (TAP_STATUS.TAP_AXIS = 2), or Z axis (TAP_STATUS.TAP_AXIS = 3). Note the TAP_AXIS judgement follows the definition of the Tap Priority. Refer to _Table 35_ for details.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 62

The TAP_STATUS.TAP_POLARITY indicates the direction from which the Tap moves towards the 8658A. Note that, this direction is derived from the value of the linear acceleration of first valid Peak.

**==> picture [448 x 438] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 63

## **11 Pedometer**

The Pedometer Engine detects the steps and output the step count to the UI registers for host to read, as well as generate the Pedometer interrupt.

The calculation of the Pedometer Detection is based on the accelerometer ODR defined by _CTRL2.aODR_ , refer to _Table 22_ for details.

The Pedometer can only work in Non-SyncSample mode, refer to 6.2 Non-SyncSample mode for details.

## **11.1 Pedometer Parameters**

**==> picture [458 x 447] intentionally omitted <==**

**----- Start of picture text -----**<br>
The parameters listed in  Table 37  are used to configure the Pedometer Engine.<br>Table 37. Pedometer Parameters<br>Parameter Name  Format   Resolution  Description<br>Indicates the count of sample batch/window for<br>ped_sample_cnt  2-bytes integer 1 sample<br>calculation<br>2-bytes format  Indicates the threshold of the valid peak-to-peak<br>(1 / 2^10) g<br>ped_fix_peak2peak  unsigned,   detection<br>= 1 mg<br>10-bits fraction  E.g., 0x00CC means 200mg<br>2-bytes format  Indicates the threshold of the peak detection comparing<br>ped_fix_peak  unsigned,   1 mg  to average<br>10-bits fraction  E.g., 0x0066 means 100mg<br>Indicates the maximum duration (timeout window) for a<br>step. Reset counting calculation if no peaks detected<br>ped_time_up  2-bytes integer 1 sample  within this duration.<br>E.g., 80 means 1.6s @ ODR = 50Hz<br>Indicates the minimum duration for a step. The peaks<br>ped_time_low  1-byte integer  1 sample  detected within this duration (quiet time) is ignored.<br>E.g., 12 means 0.25s @ ODR = 50Hz<br>Indicates the minimum continuous steps to start the valid<br>step counting. If the continuously detected steps is lower<br>than this count and timeout, the steps will not be take<br>into account; if yes, the detected steps will all be taken<br>into account and counting is started to count every<br>ped_time_cnt_entry  1-byte integer  1 step<br>following step before timeout. This is useful to screen out<br>the fake steps detected by non-step vibrations.<br>The timeout duration is defined by  ped_time_up .<br>E.g., 10 means 10 steps entry count<br>ped_fix_precision  1-byte integer  1 sample  0 is recommended<br>The amount of steps when to update the pedometer<br>output registers.<br>ped_sig_count  1-byte integer  1 sample<br>E.g.,  ped_sig_count  = 4, every 4 valid steps is detected,<br>update the registers once (added by 4).<br>**----- End of picture text -----**<br>


The parameters listed in _Table 37_ are used to configure the Pedometer Engine.

## **Table 37. Pedometer Parameters**

In the example of parameters listed below, the calculation window is 50 samples (1s duration);

The slowest step that can be detected is 4s (0.25 step per 1 second); the fastest step that can be detected is 0.4s (2.5 steps per 1 second).

The steps detected lower than 10 and timeout (defined by _ped_time_up_ ), will be ignored. The step counting starts if 10 continues steps detected (internal steps: 10, 11, 12, ...), while the step count registers is updated with 10, 14, 18… (update once after every 4 steps detected).

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 64

It is recommended to select the proper _ped_fix_peak2peak_ and _ped_fix_peak_ values depend on the movement amplitude and movement pattern, for example, if places QMI8658A in a watch or bend, running normally shows more significant _peaks than walking._

ODR = 50Hz

ped_sample_cnt:       50 samples (0x0032) ped_fix_peak2peak:  200mg (0x00CC in u6.10 format) ped_fix_peak: 100mg (0x0066 in u6.10 format) ped_time_up: 200 samples (0x00C8, 4s) ped_time_low: 20 samples (0x14, 0.4) ped_time_cnt_entry: 10 samples (0x0A) ped_fix_precision:     0 (0x00) ped_sig_count: 4 samples (0x04)

**==> picture [103 x 86] intentionally omitted <==**

## **11.2 Configure Pedometer**

The Pedometer parameters are divided into two sets and can be passed to the QMI8658A internal algorithm through two callings of CTRL9 command. As shown in _Table 38_ .

Host should write the parameters to the corresponding registers, according to _Table 38_ . Especially, write 0x01 to CAL4_H register for the first CTRL9 command of parameters, while write 0x02 to CAL4_H for the second CTRL9 command of parameters. Then trigger the CTRL9 command with 0x0D(CTRL_CMD_CONFIGURE_PEDOMETER). Refer to _Table 28_ for details.

**Table 38. Write Pedometer Parameters to QMI8658A**

|**Table 38. Write**|**Pedometer Parameters to QMI8658A**||
|---|---|---|
|Register (bits)|First CTRL9 Command|Second CTRL9 Command|
|CAL1_L (7:0)|ped_sample_cnt[7:0]|ped_time_up[7:0]|
|CAL1_H (7:0)|ped_sample_cnt[15:8]|ped_time_up [15:8]|
|CAL2_L (7:0)|ped_fix_peak2peak [7:0]|ped_time_low|
|CAL2_H (7:0)|ped_fix_peak2peak[15:8]|ped_cnt_entry|
|CAL3_L (7:0)|ped_fix_peak [7:0]|ped_fix_precision|
|CAL3_H (7:0)|ped_fix_peak[15:8]|ped_sig_count|
|CAL4_L (7:0)|NA|NA|
|CAL4_H (7:4)|0x01(means 1st command)|0x02(means 2nd command)|
|CTRL9 code|0x0D<br>(CTRL_CMD_CONFIGURE_PEDOMETER)|0x0D<br>(CTRL_CMD_CONFIGURE_PEDOMETER)|



**Note** : Configuration should be done when accelerometer and gyroscope are disabled(CTRL7.aEN = CTRL7.gEN =0).

## **11.3 Enable Pedometer**

After successfully passed the parameters to QMI8658 Pedometer engine, host need to enable the Pedometer engine by setting CTRL8.bit4 to 1. If the accelerometer is enabled (CTRL7.aEN = 1), the Pedometer engine will start to detect the steps.

If CTRL8.bit4 is set to 0, or CTRL7.aEN = 0, Pedometer engine will stop to detect the steps.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 65

## **11.4 Pedometer Interrupt**

Once the detected steps (count from last pedometer event) reach the _ped_sig_count_ defined by host, it will generate the Pedometer interrupt once. For example, if _ped_sig_count_ = 4, then every 4 valid steps detected, there is one Pedometer interrupt (internal signal).

At the meantime, the STATUS1.bit4 is set and host can read this bit to confirm Pedometer event is detected.

The Pedometer interrupt(internal signal) can be selected to drive INT1(CTRL8.bit6 = 1) or INT2(CTRL8.bit6 = 0). And once the corresponding INT pin is enabled(by CTRL1.bit3 for INT1, or CTRL1.bit4 for INT2), the Pedometer interrupt will be seen on the INT pin(synced with DRDY). Refer to to _5.3 Configuration_ Registers for details.

## **11.5 Read Step Count**

When the Pedometer is enabled, and the Pedometer event is detected (STATUS1.bit4 = 1), the step count is updated to the Step Count registers of STEP_CNT_LOW, STEP_CNT_MIDL, STEP_CNT_HIGH, in 24-bits value. Note that the step count is updated per the _ped_sig_count_ definition. For example, if _ped_sig_count_ = 4, the step count is be updated every 4 steps. The 24-bits step count is cycling counted, will return to 0 after 0xFFFFFF.

Host can read from the three registers, to get the latest step count.

## **11.6 Reset Step Count**

The Step Count can be reset by issuing Power-On Reset, Soft Reset, setting CTRL8.bit4 to 1 from 0, and CTRL9 command (CTRL_CMD_RESET_PEDOMETER).

After reset the Step Count registers (STEP_CNT_LOW, STEP_CNT_MIDL, STEP_CNT_HIGH) are cleared to 0.

When CTRL8.bit4 (Pedo_EN)   = 0, set CTRL8.bit4 to 1 will reset the Setp Count registers. Host can simply clear the CTRL8.bit4 and then set it to restart the Pedometer engine and reset the Step Count registers. Refer to _Table 22_ for CTRL8 register.

Send 0x0F (CTRL_CMD_RESET_PEDOMETER) by CTRL9 command protocol will reset the Step Count without reset the QMI8658A. Refer to _Table 28_ for CTRL9 operation.

Note that enalbe and/or disable the accelerometer by configure CTRL7.bit1(aEN), will not influence the content of the Step Counter registers.

**==> picture [197 x 186] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 66

## **12 Wake on Motion (WoM)**

## **12.1 Wake on Motion Principle**

The purpose of the Wake on Motion (WoM) functionality is to allow a system to enter a low power sleep state while the system is static and then to automatically awaken when moved. In this mode the system should use very little power, yet still respond quickly to motion.

It is assumed that the system host processor is responsible for configuring the QMI8658A correctly to place it into Wake on Motion mode, and then reconfigure the QMI8658A as necessary following a WoM interrupt.

Note that, Wake on Motion function works similar to Any-Motion detection but without sensor data output and can be configure more flexibly on interrupt behavior. Besides, once QMI8658A is configured into Wake on Motion mode, there is WoM interrupt to awaken the host if movement is detected.

## **12.2 Wake on Motion Parameters**

Wake on Motion is configured through the CTRL9 command interface (refer to _Table 28_ for details for CTRL_CMD_WRITE_WOM_SETTING ).

**Table 39. Registers used for WoM**

|||||
|---|---|---|---|
|**Register**<br>**(bits)**|**Format**|**Resolution**|**Function**|
|CAL1_L[7:0]|1-byte unsigned<br>integer|1mg|WoM Threshold:<br>Indicate the threshold of absolute of slope of a valid movement<br>0x00 must be used to indicate that WoM mode is disabled|
|CAL1_H[7:6]|2-bits unsigned<br>integer|1|WoM Interrupt Initial Value select:<br>01 – INT2 (with initial value 0)<br>11 – INT2 (with initial value 1)<br>00 – INT1 (with initial value 0)<br>10 – INT1 (with initial value 1)|
|CAL1_H[5:0]|6-bits unsigned<br>integer|1|Interrupt Blanking Time (in number of accelerometer samples), the<br>number of consecutive samples that will be ignored after enabling the<br>WoM, to screen out unwanted fake detection|
|CTRL9[7:0]|1-byte unsigned<br>integer|1|CTRL_CMD_WRITE_WOM_SETTING (0x08): to send this pre-<br>defined command to QMI8658A to enable the WoM mode|
|||||



The WoM Threshold value is configurable to make the amount of motion required to wake QMI8658A and the host. The special threshold value of 0x00 can be used to disable the WoM mode, returning the interrupt pins to their normal functionality. Refer to _6 Interrupts_ .

The Interrupt Initial Value (1 or 0) and the interrupt pin used for signaling (INT1 or INT2) are selectable to make it easy for system integrators to use the WoM motion mode to wake the host processor from its deepest sleep level. The selected INT pin can be configured as High-active or Low-active, if enabled by CTRL1.bit3 for INT1, or CTRL1.bit4 for INT2, the WoM interrupt can be seen on the INT pin and interrupt host.

The Interrupt Blanking Time is a programmable number of accelerometer samples to ignore when starting WoM mode so that no spurious wake-up events are generated by startup transients.

## **12.3 Accelerometer Configuration**

Host is expected to configure the proper ODR and full-scale range of accelerometer by configure CTRL2 register. Refer to _Table 22_ .

## **12.4 Wake on Motion Event**

When a Wake on Motion event is detected the QMI8658A will set STATUS1.WoM = 1 (bit2). Reading STATUS1 by the host will clear the WoM bit and will reset the chosen interrupt line (INT1 or INT2) to the value given by the WoM Interrupt initial value (refer to _12.2_ ).

For each WoM event, the state of the selected interrupt line is toggled. This ensures that while the system is moved, the host processor will receive wakeup interrupts regardless of whether it uses high, low, positive- or negative-edge interrupts.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 67

The QMI8658A stays in WoM mode until commanded to enter a new mode by the host processor.

## **12.5 Configuration Procedure**

To configure QMI8658A into WoM mode, the sequence ilustated in _Figure 25_ should be followed and implemented.

**==> picture [341 x 175] intentionally omitted <==**

**----- Start of picture text -----**<br>
Disable sensors (Write 0 to CTRL7.bit[1:0])<br>Set Accelerometer ODR and Full-scale (Write CTRL2)<br>Set Wake on Motion (WoM) Threshold in CAL1_L;<br>Select interrupt, polarity and blanking time in CAL1_H<br>Execute CTRL9 command to configure WoM mode<br>(CTRL_CMD_WRITE_WOM_SETTING)<br>Set Accelerometer enable bit (CTRL7.aEN = 1)<br>**----- End of picture text -----**<br>


**Figure 25. WoM Configuration Commands and Sequence**

The STATUS1.WoM bit is cleared upon setting the WoM threshold to a non-zero value, and the selected interrupt pin is configured according to the settings. The interrupt blanking time is included that prevents spurious interrupts to propagate after the enabling of WoM.

## **12.6 Exit Wake on Motion Mode**

To exit WoM mode the host must first clear CTRL7[1:0] to disable all sensors, and then write a threshold value of 0x0 for the WoM Threshold (see _Table 39_ , Registers used for WoM) and execute the WoM configuration CTRL9 command (refer to _Table 28_ for details for CTRL_CMD_WRITE_WOM_SETTING). On doing this the interrupt pins will return to their normal function. After zeroing the WoM Threshold the host processor can proceed to reconfigure the QMI8658A as normal.

**==> picture [444 x 251] intentionally omitted <==**

**Figure 26. WoM Example Diagram**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

68

## **13 Locking Mechanism**

## **13.1 Locking Mechanism Principle**

Locking Mechanism function is enabled in SyncSample mode, can lock the sensor data and keep the values in data registers after a proper locking process.

As the concept of “shadow register”, which enables host to read the locked data in unlimited delay without the risk of mixing the two consecutive data if the new data comes and updates to the sensor data registers during the host read the sensor data registers. Refer to 6.1 SyncSample mode.

## **13.2 Locking Mechanism Data Reading Process**

_Figure 27_ shows the process of reading data in Locking Mechanism.

**==> picture [372 x 438] intentionally omitted <==**

**Figure 27. Data Reading Process in Locking Mechanism**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 69

## 13.2.1 **Disable/Enable AHB Clock Gating**

First host need to disable the internal AHB clock gating, this will fully screen out the possibility of misalignment of the two consecutive data samples. Note this is ONLY needed when using SPI (accel only mode with ODR less than 500Hz), I2C or I3C interfaces (in all ODRs).

Host can disable the internal AHB clock gating by applying:

- 1- write 0x01 to CAL1_L register.

- 2- write 0x12 (CTRL_CMD_AHB_CLOCK_GATING) in CTRL9 protocol, refer to 5.9 and 5.10.6.13.

After disabling the sensor, enable back the clock gating by:

- 1- write 0x00 to CAL1_L register.

- 2- write 0x12 (CTRL_CMD_AHB_CLOCK_GATING) in CTRL9 protocol, refer to 5.9 and 5.10.6.13.

**==> picture [49 x 40] intentionally omitted <==**

## 13.2.2 **Enable Locking Mechanism**

The locking Mechanism is enabled when setting CTRL7.bit7 to “1” ( _syncSmpl_ ) and enable accelerometer and/or gyroscope.

- 1- Enable 6DOF in this mode, write 0x83 to CTRL7.

**==> picture [75 x 74] intentionally omitted <==**

- 2- Enable only Accel in this mode, write 0x81 to CTRL7.

- 3- Enable only Gyro in this mode, write 0x82 to CTRL7.

## 13.2.3 **Reading Sensor Data**

When the Locking Mechanism is enabled, the reading to the STATUSINT register when new sensor data is available (STATUSINT.Avail = 1) will trigger the locking of the current sensor data sample. Once the data sample is locked, new data will be dropped, until the release of the locking. The Locking Mechanism is automatically released after host reads GZ_H if gyroscope is enabled or AZ_H if accelerometer only is enabled.

Process of data reading in Locking Mechanism mode:

Read STATUSINT register, if STATUSINT.Avail = 1, the locking mechanism is started to take effect, go to step 2. If STATUSINT.Avail = 0, repeat step 1.

If STATUSINT.Avail = 1 and STATUSINT.Locked = 0, means data looking is in progress, and will be locked within Data_Lock_Delay. If STATUSINT.Locked = 1, go to step 4.

- 1- Wait for Data_Lock_Delay duration (refer to _13.3_ ), go to step 4.

Burst read the sensor data until the last byte of enabled sensor(s) (to release the locking), Refer to _Table 25_ .

- 2- Repeat step 1 – 4, until Exit

## **13.3 Data_Lock_Delay**

When the gyroscope is enabled the duration of Data_Lock_Delay is shown in Table 40.

## **Table 40. Data_Lock_Delay When Gyroscope Is Enabled**

**==> picture [68 x 70] intentionally omitted <==**

|ODR setting|ODR(Hz)|Data_Lock_Delay (usec)|
|---|---|---|
|0|7174.4|2|
|1|3587.2|2|
|2|1793.6|4|
|3|896.8|6|
|4|448.4|12|
|5|224.2|12|
|6|112.1|12|
|7|56.05|12|
|8|28.025|12|



Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

© 2022 QST Corporation

www. qstcorp.com 70

When the gyro is not enabled (accel only mode) the value of _Data_Lock_Delay_ is shown in _Table 41_ .

## **Table 41. Data_Lock_Delay When Gyroscope Is Disabled**

|ODR setting|ODR(Hz)|Data_Lock_Delay (usec)|
|---|---|---|
|3|1000|6|
|4|500|12|
|5|250|24|
|6|125|48|
|7|62.5|48|
|8|31.25|48|
|12|128|40|
|13|21|100|
|14|11|200|
|15|3|270|
|**chanism**|||



## **13.4 Exit Locking Mechanism**

Once pulling sensor data from QMI8658 is finished, it can be configured to exit the Locking Mechanism mode, by disabling the sensors by setting CTRL7.bit[1:0] to 0, and enabling the AHB Clock Gating (refer to _13.2.1_ ).

## **13.5 On-The-Fly ODR Change in Locking Mechanism**

The on-the-fly ODR changing is supported, so host can change the ODR of sensor without disable the sensor.

An example sequence of changing ODR without disabling the sensor in locking mechanism is shown below:

- 1- Write CTRL2/CTRL3 to set the ODR’s of accelerometer and gyroscope and full scales.

- 2- Write 0x81 / 0x82 / 0x83 to CTRL7.

- 3- Read Sensor Data according 13.2.3.

- 4- Changing ODR on-the-fly:

- a- Write 0x01/0x02/0x03 to CTRL7(clear the syncSmpl bit).

- b- Wait 1ms.

- c- Clear the Locking Mechanism in case if the data is still locked from previous ODR by reading GZ_H if gyroscope is enabled or AZ_H if accelerometer only is enabled.

- d- Write CTRL2 / CTRL3 with the new ODR’s.

- e- Write 0x81/0x82/0x83 to CTRL7.

- 5- Start poll and read Sensor Data using the new ODRs based on 13.2.3.

Note that, the new data will be stable in at least 3 samples for filter to settle down, therefore, it is recommended to discard the first several samples at host side.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

71

## **14 Calibration-On-Demand (COD)**

## **14.1 COD Principle**

The Calibration-On-Demand supports the on-demand calibration of Gyro X and Y axes. Based the internal integrated functionality, the QMI8658A can calibrate the internal gain of X & Y axes of gyroscope, result in a more precise sensitivity, and a tighter distribution of the X & Y axes sensitivity over QMI8658A chips.

Note that the Z axis of gyroscope is not influenced by COD.

## **14.2 Run COD**

**==> picture [82 x 72] intentionally omitted <==**

To run the COD, host need to

1. Set CTRL7.aEN = 0 and CTRL7.gEN = 0, to disable the accelerometer and gyroscope.

2. Issue the CTRL_CMD_ON_DEMAND_CALIBRATION (0xA2) by CTRL9 command.

3. And wait about 1.5 seconds for QMI8658A to finish the CTRL9 command.

4. Read the COD_STATUS register (0x46) to check the result/status of the COD implementation.

During the process, it is recommended to place the device in quiet, otherwise, the COD might fail and report error.

If succeeds, the recalibrated gain parameters will be applied to the sensor data afterwards. The updated gains are output to the UI registers and can be read by host, refer to _14.3_ . The recalibrated gain parameters will be lost if a power on reset or soft reset is implemented, QMI8658A will then use the on-chip default gain parameters.

If failed, there is no influence on the operation of gyroscope, QMI8658A will keep using the previous workable parameters (last successful COD parameters or the on-chip default parameters).

## **14.3 COD Status**

If the COD command is successfully implemented, the COD_STATUS register will output 0x00 for the indication.

The non-zero value of COD_STATUS indicates different modes of failure. Refer to 5.7 Calibration-On-Demand (COD) Status Register for details.

## **14.4 Save and Restore the New Gain Parameters**

After a successful COD, the new gains with COD correction will be applied to the future data of X and Y axes of Gyroscope. At the meantime, the new parameters are updated to the registers below, for host to read and save.

1. Gyro-X gain (16 bits) will be in dVX_L and dVX_H registers (0x51, 0x52)

2. Gyro-Y gain (16 bits) will be in dVY_L and dVY_H registers (0x53, 0x54)

3. Gyro-Z gain (16 bits) will be in dVZ_L and dVZ_H registers (0x55, 0x56)

If the host saved those gain parameters, it is possible to pass them back to the QMI8658A (without invoking again the COD routine), using the CTRL9 command CTRL_CMD_APPLY_GYRO_GAINS (0xAA) as follow:

1. Disable Accelerometer and Gyroscope by setting CTRL7.aEN = 0 and CTRL7.gEN = 0

2. write Gyro-X gain (16 bits) to registers CAL1_L and CAL1_H registers (0x0B, 0x0C)

3. write Gyro-Y gain (16 bits) to registers CAL2_L and CAL2_H registers (0x0D, 0x0E)

4. write Gyro-Z gain (16 bits) to registers CAL3_L and CAL3_H registers (0x0F, 0x10)

5. Write 0xAA to CTRL9 and follow CTRL9 protocol

Once the CTRL9 command is successfully finished, the restored gains will take effects for future data of Gyroscope.

Note that it is always recommended to run the COD from time to time to apply the precise and up-to-date correction of the Gyro-X and Gyro-Y sensitivity. Designer should be careful to restore the out-of-date gain parameters, especially when there is significant change of PCB stress.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 72

## **15 Self-Test (Check-Alive)**

## **15.1 Accelerometer Self-Test**

The accelerometer Self-Test (Check-Alive) is used to determine if the accelerometer is functional and working within acceptable parameters.

It is implemented by applying an electrostatic force to actuate each of the three X, Y, and Z axis of the accelerometer. If the accelerometer mechanical structure responds to this input stimulus by sensing at least 200 mg, then the accelerometer can be considered functional.

The accelerometer Self-Test data is available to be read at registers dVX_L, dVX_H, dVY_L, dVY_H, dVZ_L and dVZ_H. The Host can initiate the Self-Test at any time with the following procedure.

Procedure for accelerometer Self-Test:

- 1-   Disable the sensors  (CTRL7 = 0x00).

- 2-   Set proper accelerometer ODR (CTRL2.aODR) and bit CTRL2.aST (bit7)  to 1 to trigger the Self-Test.

- 3-   Wait for QMI8658A to drive INT2 to High, if INT2 is enabled (CTRL1.bit4 = 1), or STATUSINT.bit0 is set to 1.

**==> picture [74 x 74] intentionally omitted <==**

- 4-   Set CTRL2.aST(bit7) to 0, to clear STATUSINT1.bit0 and/or INT2.

- 5-   Check for QMI8658A drives INT2 back to Low, and sets STATUSINT1.bit0 to 0.

- 6- Read the Accel Self-Test result:

X channel: dVX_L and dVX_H (registers 0x51 and 0x52)

Y channel: dVY_L and dVY_H (registers 0x53 and 0x54)

Z channel: dVZ_L and dVZ_H (registers 0x55 and 0x56)

The results are 16-bits in format signed U5.11, resolution 0.5mg (1 / 2^11 g).

If the absolute results of all three axes are higher than 200mg, the accelerometer can be considered functional. Otherwise, the accelerometer cannot be considered functional.

Note that, the Self-Test function will automatically set the full-scall to 16g and use the aODR set by user (CTRL2.aODR). At the end of Self-Test, QMI8658A will update CTR2 with the original value user set before starting the Check-Alive) routine.

The typical time for Self-Test (from setting aST to 1, until the rising edge of INT2 if enabled, or STATUSINT.bit0 is set to 1) costs about 25 ODRs:

25ms @ 1KHz ODR

800ms @ 32Hz ODR

- 2.2s @ 11Hz ODR

## **15.2 Gyroscope Self-Test**

The gyroscope Self-Test (Check-Alive) is used to determine if the gyroscope is functional.

It is implemented by applying an electrostatic force to actuate each of the three X, Y, and Z axis of the gyroscope and measures the mechanical response on the corresponding X, Y, and Z axis. If the equivalent magnitude of the gyroscope output is greater than 300dps for each axis, the gyroscope can be considered as functional.

The gyroscope Self-Test data is available to be read at output registers dVX_L, dVX_H, dVY_L, dVY_H, dVZ_L & dVZ_H. The Host can initiate the Self-Test anytime with the following procedure. Procedure for gyroscope Self-Test:

- 1-   Disable the sensors  (CTRL7 = 0x00).

- 2-   Set the bit gST to 1.  (CTRL3.bit7 = 1’b1).

- 3-    Wait for QMI8658A to drive INT2 to High, if INT2 is enabled, or STATUSINT.bit0 is set to 1.

- 4-    Set CTRL3.aST(bit7) to 0, to clear STATUSINT1.bit0 and/or INT2.

- 5-    Check for QMI8658A drives INT2 back to Low, or sets STATUSINT1.bit0 to 0.

- 6-     Read the Gyro Self-Test result:

X channel: dVX_L and dVX_H (registers 0x51 and 0x52)

Y channel: dVY_L and dVY_H (registers 0x53 and 0x54)

Z channel: dVZ_L and dVZ_H (registers 0x55 and 0x56)

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 73

Read the 16 bits result in format signed U12.4, resolution is 62.5mdps (1 / 2^4 dps).

If the absolute results of all three axes are higher than 300dps, the gyroscope can be considered functional. Otherwise, the gyroscope cannot be considered functional.

Note that, the Self-Test function will automatically set the full-scale (gFS) and ODR (gODR) of CTRL3. At the end of Self-Test, QMI8658A will update CTR3 with the original value user set before starting the Self-Test routine. The typical time (from writing gST to 1, until the rising edge of INT2 if enabled, or STATUSINT.bit0 set to 1) cost for the Self-Test process is about 400ms.

**==> picture [448 x 438] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 74

## **16 Host Serial Interface**

QMI8658A Host Serial Interface supports slave interfaces of I[2] C, MIPI I[3] C and SPI. For SPI, it supports both 3-wire and 4-wire modes. The basic timing characteristics for the interface are described below. Through the QMI8658A Host Serial Interface, the host can access, setup and control the QMI8658A UI Registers _._

## **16.1 Address Auto Increment**

During burst reads and writes, the target address can be configured in Auto-Increment or Non-Increment, by CTRL1.ADDR_AI(bit 6), refer to _5.3_ .

For example, during burst read(burst write is similar) started from register 0x0B(CAL1_L):

If ADDR_AI = 0, the register address will not increase, and QMI8658A will output the content of CAL1_L, CAL1_L, CAL1_L …, as long as host continues the burst read clock;

If ADDR_AI = 1, the register address will automatically increase, and QMI8658A will output the content of CAL1_L, CAL1_H, CAL2_L …, as long as host continues the burst read clock.

Note that the default value of ADDR_AI is 0, so it is recommended to set it to 1 from beginning, in case of burst read/write is required.

Note that, burst writes to Configuration Registers(refer to _Table 22_ ) are NOT supported. These registers should be written in single cycle mode only.

**==> picture [76 x 76] intentionally omitted <==**

## **16.2 Serial Peripheral Interface (SPI)**

## 16.2.1 **SPI Features**

QMI8658A supports both 3-wire and 4-wire modes in the SPI slave interface. The SPI 4-wire mode uses two control lines (CS, SPC) and two data lines (SDI, SDO). The SPI 3-wire mode uses the same control lines and one bi-directional data line (SDIO). The SDI /SDIO pin is used for both 3- and 4-wire modes and is configured based on the mode selected.

SPI transactions can be done in either Mode 0 (CPOL=0, CPHA=0) or Mode 3 (CPOL=1, CPHA=1).  The QMI8658A interface automatically detects which mode is in use and configures clocking accordingly.

SPI 3- or 4-wire modes are configured by setting CTRL1.bit7. SPI 3-wire mode is selected when CTRL1.bit7 = 1. The default configuration is SPI 4-wire mode, that CTRL1.bit7 is 0.

Figure 28 shows the SPI address and data formats.

## **SPI Features**

- Data is latched on the rising edge of the clock

- Data should change on falling edge of clock

- Maximum frequency is 15 MHz

- Data is delivered MSB first

- Support single read/writes and multi cycle (Burst) read/writes.

- **NOTE** : burst writes to Configuration Registers(refer to Table 22) are NOT supported. These registers should be written in single cycle mode only.

- Supports 7-bit Address format and 8-bit data format

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

75

**==> picture [226 x 117] intentionally omitted <==**

**==> picture [184 x 9] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 28.  SPI Address and Data Format<br>**----- End of picture text -----**<br>


**==> picture [49 x 40] intentionally omitted <==**

## 16.2.2 **SPI Interface Connection**

In a typical SPI Master and Multi-Slave configuration, the SPI master shares the SPI clock (SPC), the serial data input (SDI), and the Serial Data Output (SDO) with all the connected SPI slave devices. Unique Chip Select (CS) lines connect each SPI slave to the master.

**==> picture [191 x 88] intentionally omitted <==**

**Figure 29. Typical SPI 4-Wire Connection**

Figure 29 and Figure 30 show typical multi-slave 4- and 3-wire configurations. The primary difference between the two configurations is that the SDI and SDO lines are replaced by the bi-directional SDIO line. The SDIO line is driven by the master with both address and data when it is configured for write mode. During read mode, the SDIO line is driven by the master with the address, and subsequently driven by the “addressed” slave with data.

**==> picture [191 x 89] intentionally omitted <==**

**Figure 30. Typical SPI 3-Wire Connection**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 76

## 16.2.3 **SPI Transaction Protocol**

Figure 31 and Figure 32 illustrate the waveforms for both 4-wire and 3-wire SPI read and write transactions. Note that CS is active during the entire transaction.

**==> picture [468 x 493] intentionally omitted <==**

**----- Start of picture text -----**<br>
SPI : Mode 0<br>SPI : Mode 3<br>Figure 31.  SPI 4-Wire Single Byte Read and Write (Mode 0 and Mode 3)<br>**----- End of picture text -----**<br>


© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 77

**==> picture [468 x 515] intentionally omitted <==**

**----- Start of picture text -----**<br>
SPI : Mode 0<br>**----- End of picture text -----**<br>


**==> picture [55 x 8] intentionally omitted <==**

**----- Start of picture text -----**<br>
SPI : Mode 3<br>**----- End of picture text -----**<br>


**Figure 32. SPI 4-Wire Multi-Byte Read and Write Transactions**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

78

**==> picture [468 x 146] intentionally omitted <==**

**==> picture [53 x 8] intentionally omitted <==**

**----- Start of picture text -----**<br>
SPI: Mode 0<br>**----- End of picture text -----**<br>


**==> picture [468 x 142] intentionally omitted <==**

SPI : Mode 3

**Figure 33. SPI 3-Wire Single Byte Read and Write Transactions**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 79

**==> picture [468 x 327] intentionally omitted <==**

**----- Start of picture text -----**<br>
SPI : Mode 0<br>SPI : Mode 3<br>**----- End of picture text -----**<br>


**Figure 34. SPI 3-Wire Multi-Byte Read and Write Transactions**

**==> picture [197 x 186] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 80

## 16.2.4 **SPI Timing Characteristics**

The typical operating conditions for the SPI interface are provided in Table 42. Please refer to Table 6 for the VIL, VIH, VOL, VOH definition to define the rising and falling edge condition of the timing symbols.

VDDIO = 1.8 V, T = 25°C unless otherwise noted.

**Table 42. SPI Interface Timing Characteristics**

|**Symbol**|**Parameter**|**Min.**|**Max.**|**Unit**|
|---|---|---|---|---|
|tSPC|SPI Clock Cycle|66.6||ns|
|fSPC|SPI Clock Frequency||15|MHz|
|tsCS|CS SetupTime|6||ns|
|thCS|CS Hold Time|8||ns|
|tsSDI|SDI Input SetupTime|5||ns|
|thSDI|SDI Input Hold Time|15||ns|
|tvSDO|SDO Time for Valid Output||50|ns|
|thSDO|SDO Hold Time for Output|9||ns|
|tdSDO|SDO Disable Time for Output||50|ns|
|tsSDIO|SDIO Address SetupTime|5||ns|
|thSDIO|SDIO Address Hold Time|15||ns|
|tvSDIO|SDIO Time for Valid Data||50|ns|
|tczSDIO|SDIO Time from SPC to High Z||50|ns|
|tzSDIO|SDIO Time from CS to High Z||50|ns|
||||||



**==> picture [228 x 228] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 81

**==> picture [468 x 551] intentionally omitted <==**

**----- Start of picture text -----**<br>
SPI : Mode 0<br>**----- End of picture text -----**<br>


**==> picture [55 x 8] intentionally omitted <==**

**----- Start of picture text -----**<br>
SPI : Mode 3<br>**----- End of picture text -----**<br>


**Figure 35. Timing Characteristics for SPI 3- and 4-Wire Interfaces**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 82

## **16.3 I[2] C Interface**

## 16.3.1 **I[2] C Slave Address Selection**

During the slave device selection phase, the I[2] C master supplies the 7-bit I[2] C slave device address to enable the QMI8658A. When SA0 is pulled down externally, the 7-bit device address becomes 0x6B (0b1101011). The 7-bit device address for the QMI8658A is 0x6A (0b1101010) if SA0 is pulled up or left unconnected. Note that internally there is a weak pull-up of 200K and this pull-up resistor will be automatically disabled after the detection of I2C slave address during the Reset Process, refer to _7.4_ .

## 16.3.2 **I[2] C Interface Characteristics**

Table 43 provides the I[2] C interface timing characteristics while Figure 36 and Figure 37 illustrate the I[2] C timing for both fast and standard modes, respectively. Please refer to Table 6 for the VIL, VIH, VOL, VOH definition to define the rising and falling edge condition of the timing symbols.

For additional technical details about the I[2] C standard, such as pull-up resistor sizing the user is referred to “UM10204 I[2] C-bus specification and user manual,” published by NXP B.V.

|**Table 43.**|**I2C Timing Characteristics**||||||
|---|---|---|---|---|---|---|
|**Symbol**|**Parameter**|**Conditions**|**Min.**|**Typ.**|**Max.**|**Unit**|
|fSCL|SCL Clock Frequency||0||400|KHz|
|tBUF|Bus-Free Time between STOP and<br>START Conditions||1300|||ns|
|tHD;STA|START or Repeated START Hold Time||600|||ns|
|tLOW|SCL LOW Period||1300|||ns|
|tHIGH|SCL HIGH Period||600|||ns|
|tSU;STA|Repeated START SetupTime||600|||ns|
|tSU;DAT|Data SetupTime||100|||ns|
|tHD;DAT|Data Hold Time|Standard Mode|0||3450|ns|
|||Fast Mode|0||900||
|tRCL,tR|SCL Rise Time|Standard Mode|||1000|ns|
|||Fast Mode|20 + 0.1 * CB(14)||300||
|tFCL|SCL Fall Time|Standard Mode|||300|ns|
|||Fast Mode|20 + 0.1 * CB(14)||300||
|tRDA,tRCL1|SDA Rise Time.<br>Rise Time of SCL after a Repeated<br>START Condition and after ACK Bit|Standard Mode|||1000|ns|
|||Fast Mode|20 + 0.1 * CB(14)||300||
|tFDA|SDA Fall Time|Standard Mode|||300|ns|
|||Fast Mode|20 + 0.1 * CB(14)||300||
|tSU;STO|Stop Condition Setup Time||600|||ns|



**Note:**

14. CB is the bus capacitance.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com

83

**==> picture [478 x 108] intentionally omitted <==**

## **Figure 36. I2C Standard Mode Interface Timing**

**==> picture [384 x 183] intentionally omitted <==**

**Figure 37. I2C Fast Mode Interface Timing**

## **16.4 I[3] C Interface**

The QMI8658A is compliant with the MIPI Alliance Basic Specification for I3C, version 1.0.

Note that HDR, IBI are not supported by QMI8658A.

The PID of 8658A is 0x086E00051000.

MIPI Manufacturer ID(VID) for QST Corp. is 0x0437.

The static address of I[3] C follows the descriptions in I[2] C, refer to _16.3.1_ .

**==> picture [85 x 92] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 84

## **17 Package and Handling**

## **17.1 Package Drawing**

**==> picture [469 x 233] intentionally omitted <==**

**Figure 38. 14 Pin LGA 2.5 x 3.0 x 0.86 mm Package**

**==> picture [250 x 250] intentionally omitted <==**

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 85

## **17.2 Reflow Specification**

**==> picture [400 x 440] intentionally omitted <==**

**----- Start of picture text -----**<br>
Note:<br>15. Figure from JEDEC J-STD-020<br>Profile Feature  Pb-Free Assembly Profile<br>Temperature Min. (Tsmin) 150°C<br>Temperature Max. (Tsmax)  200°C<br>Time (tS) from (Tsmin to Tsmax)  60-120 seconds<br>Ramp-up Rate (TL to TP) 3°C/second max.<br>Liquidous Temperature (TL)  217°C<br>Time (tL) Maintained above (TL)  60-150 seconds<br>Peak Body Package Temperature (TP)  260°C +0°C / -5°C<br>Time (tP) within 5°C of 260°C  30 seconds<br>Ramp-down Rate (TP to TL)  6°C/second max.<br>Time 25°C to Peak Temperature  8 minutes max.<br>Figure 39.  Reflow Profile<br>**----- End of picture text -----**<br>


## **17.3 Storage Specifications**

QMI8658A storage specification conforms to IPC/JEDEC J-STD-020D.01 Moisture Sensitivity Level (MSL) 3. Floor life after opening the moisture-sealed bag is 168 hours with storage conditions: Temperature: ambient to ≤30°C and Relative Humidity: 60%RH.

© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 86

## **18 Document Information**

## **18.1 Revision History**

**==> picture [463 x 469] intentionally omitted <==**

**----- Start of picture text -----**<br>
Revision  Revision Date  Description<br>0.4  April 22, 2020  Initial release of Advance Information datasheet<br>0.5  July 7, 2020  Updated CAL Register Addresses, CTRL9 Commands and Descriptions,<br>Current Consumption, Accelerometer and Gyroscope Filter Characteristics,<br>Low Power Mode ODR, Wake on Motion, Magnetometer Sensors supported<br>0.6  Jan 13, 2021  Updated SPI description and diagrams, SPI modes, product performance<br>specifications, and register map and descriptions<br>0.7  17 May, 2021  Updated the pin name and function name map, typical SPI connections, soft<br>reset, Self-Test, On-Demand Calibration, I2C timing parameter table,<br>ADDR_AI, BE, VDDIO rising limitation, recommended I2C pull-up resistance<br>0.8  10 Sep, 2021  Added the maximum limitation of VIH, updated FIFO read command and<br>sequence, updated ODR & filter bandwidth configurations, updated CTRL2,<br>CTRL3, CTRL4, CTRL5 descriptions, deleted descriptions of magnetometer,<br>updated Electro-Mechanical Specifications, updated the internal block<br>diagram & interface operating modes, deleted the specifications, registers,<br>and application diagrams that relative to I2CM interface, updated the<br>Features, Descriptions & Applications, updated the marking information,<br>updated the disabling of the internal pull-up resistors in Ios.<br>0.9  10 Jan, 2022  updated the INT1/INT2 enable bit in CTRL1, updated Wake on Motion, added<br>locking mechanism.<br>0.94  29 Apr, 2022  Updated the parameters of Acceleration and Gyroscope, remove sys_hs,<br>removed AltitudeEngine relative information, updated FIFO relative<br>descriptions, updated Interrupt relative descriptions, updated operating<br>mode and transition diagram, updated CTRL1, CTRL7, CTRL8 register map,<br>added Motion Detection, Tap, Pedometer descriptions.<br>0.95  6 May, 2022  Added the Reset process, updated the time cost for Self-Test process and<br>correct the description of aODR used for Self-Test, added the COD chapter,<br>removed the CTRL_CMD_GYRO_BIAS CTRL9 command, updated the<br>Locking Mechanism, WoM, COD, Self-Test, SPI, I2C, I3C interface<br>descriptions, adjusted the format, remove CTRL4 & CTRL6, remove tr<br>requirement, added power on sequence of VDDIO and VDD, added noise<br>density of gyroscope over ODR.<br>1.0  20 June, 2022  Updated the gyro noise density, changed the name of RESV-NC(pin 10) to<br>RESV, updated the power on sequence.<br>**----- End of picture text -----**<br>


© 2022 QST Corporation

Document#: 13-52-25 ∙ QMI8658A Datasheet ∙ Rev A

www. qstcorp.com 87
