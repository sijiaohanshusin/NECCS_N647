**==> picture [595 x 94] intentionally omitted <==**

**==> picture [595 x 94] intentionally omitted <==**

**==> picture [595 x 95] intentionally omitted <==**

_( DOC No. HX8696-A-DS )_

HX8696-A 1200CH TFT LCD Gate Driver _Preliminary version 01 August, 2010_

**==> picture [595 x 95] intentionally omitted <==**

**==> picture [595 x 94] intentionally omitted <==**

**==> picture [595 x 95] intentionally omitted <==**

**==> picture [595 x 94] intentionally omitted <==**

HX8696-A 1200CH TFT LCD Gate Driver

_August, 2010_

Preliminary Version 01

## **1. General Description**

The HX8696-A is a 1200-channel outputs gate driver, which is used for driving the gate line of TFT LCD panel. It is designed for 2-level output with 40V LCD driving voltage range.

**==> picture [57 x 45] intentionally omitted <==**

## **2. Features**

- 2-level output

- 1200/1152/1080/960 channel outputs selectable with 2 dummy outputs

- Various scan function can support various dual gate driving sequence

- Maximum 200KHz operation frequency

- Digital supply voltage:2.2V to 3.6V

**==> picture [33 x 33] intentionally omitted <==**

- LCD driving voltage range: 40V

- Bi-directional data shift capability

- High voltage CMOS process technology

- COG/COF package

**==> picture [65 x 80] intentionally omitted <==**

**==> picture [87 x 92] intentionally omitted <==**

**==> picture [33 x 33] intentionally omitted <==**

**==> picture [56 x 56] intentionally omitted <==**

Himax Confidential

_-P.2-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## **3. Block Diagram**

**==> picture [427 x 480] intentionally omitted <==**

**----- Start of picture text -----**<br>
•  •  •  •  •  •<br>Output Buffer<br>VGH  •  •  •  •  •  •<br>VDD<br>VSS<br>Level Shifter<br>VGL<br>•  •  •  •  •  •<br>Bi-directional Shift Register<br>Input Buffer<br>CPV STV1 OE1 OE3 MODE2  SEG2<br>L/R STV2 OE2 MODE1 SEG1  EVEN<br>OUT0 OUT1  OUT2  OUT3<br>(1)<br>OUT1198  OUT1199  OUT1200  OUT1201<br>(1)<br>**----- End of picture text -----**<br>


**Note:** (1) OUT0 and OUT1201 are LCD panel auxiliary pins; these pins always output VGL level.

Himax Confidential

_-P.3-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## **4. Pin Description**

|**Pin Name**|**I/O**|**Function**||**Description**|**Description**|**Description**|**Description**|**Description**|**Description**|
|---|---|---|---|---|---|---|---|---|---|
|CPV|In|Shift clock input||The clock for the internal shift registers.||||||
|**L/R**|**In**|**Shift direction**<br>**control pin**|<br> <br> <br>|The shift direction of device internal shift register is<br>controlled by this pin as shown below: Default L/R=L.<br>L/R =H: STV1�OUT1�OUT2�• • •�OUT1200�STV2.<br>L/R=L: STV2�OUT1200� • • • �OUT2�OUT1�STV1.||||||
|STV1<br>STV2|I/O|Start pulse<br>input/output pin|<br>|L/R=H, STV1 is used for start pulse input;<br>STV2 is used for start pulse output.<br>L/R =L, STV2 is used for start pulse input;<br>STV1 is used for start pulse output.||||||
|OE1<br>OE2<br>OE3|In|Output enable<br>control|<br> <br>|The OE signal controls the output enable.<br>OE1~OE3=H: All driver outputs are fixed to VEE<br>regardless of CPV. However, the content of<br>shift register is not cleared.<br>OE1~OE3=L: Normal operation.||||||
|/XAO|In|Output all-on<br>control|<br> <br> <br> <br>|When /XAO is set to L, all outputs are fixed to VGH. Note<br>that this pin has higher priority than OE. However, the<br>content of shift register is not cleared.<br>/XAO is pulled high to VDD internally. When it is not used,<br>connecting to VDD is recommended.||||||
|**MODE1**<br>**MODE2**|**In**|**Channel mode**<br>**selection**||Channel mode selection, default MODE1=H, MODE2=H.<br>Output<br>channel<br>Disable<br>channel<br>MODE1<br>MODE2<br>960<br>481~720<br>L<br>L<br>1080<br>541~660<br>H<br>L<br>1152<br>577~624<br>L<br>H<br>1200<br>-<br>H<br>H||||||
|||||Output<br>channel|Disable<br>channel||MODE1||MODE2|
|||||960|481~720||L||L|
|||||1080|541~660||H||L|
|||||1152|577~624||L||H|
|||||1200|-||H||H|
|**SEG1**<br>**SEG2**|**In**|**Scan function**<br>**selection**||Scan function|selection, default SEG1=L,||||SEG2=L.<br>SEG2<br>L<br>H<br>L<br>H|
|||||Scan function||SEG1|||SEG2|
|||||Z1||L|||L|
|||||Z1||L|||H|
|||||弓||H|||L|
|||||Z2||H|||H|
|**EVEN**|**In**|**Frame selection**|Frame selection, default EVEN=L.<br>Frame<br>Odd<br>Even|||||||
|||||Frame||||EVEN||
|||||Odd||||L||
|||||Even||||H||
|OUT1~<br>OUT1200|Out|Driver output|<br>|The output is either VGH or<br>TFT LCDpanel.|||VGL for driving the gate line of|||
|OUT0<br>OUT1201|Out|Auxiliary pins||These two pins always output VGL level.||||||
|VDD|In|Power supply||Digitalpower supply.||||||
|VSS|In|Power supply||Groundingfor VDD.||||||
|VGH|In|Power supply||Power supplyfor Gate on output.||||||
|VGL|In|Powersupply||Power supplyfor Gate off output.||||||
|PATH1~3|In|Internal link||Linked together internal.||||||



Himax Confidential This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_-P.4-_

_August, 2010_

HX8696-A 1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## **5. Function Description**

## **5.1 Device operation**

When L/R=H and MODE=H, the STV1 start pulse input is sensed on the rising edge of CPV and stored in the first stage of shift register, which makes the first scan signal output from the OUT1 pin. While stored data is transferred to the next stage shift register on the rising edge of next CPV, new data of STV1 is sensed and stored simultaneously.

The output pin (OUT1 to OUT1200) supplies VGH voltage or VGL voltage to the LCD panel depending on the data stored in the shift register. For normal operation, a VGH voltage is outputted one by one from OUT1 to OUT1200 in synchronization with CPV pulse. According to SEG1, SEG2, and EVEN pins setting, it is possible to get other different output sequence (Described in section 5.2).

After 1200 CPV rising edge are past, the STV2 goes up to high level at the 1200[th] falling edge of CPV and goes down to low level at the 601[st] falling edge of CPV. This STV2 output signal becomes the STV1 start pulse input of next cascaded gate driver.

When OE=H, the corresponding output channels are fixed to VGL level regardless of CPV. The channel output returns to normal status as soon as OE goes back to L.

**==> picture [65 x 80] intentionally omitted <==**

**==> picture [33 x 33] intentionally omitted <==**

**==> picture [86 x 92] intentionally omitted <==**

Himax Confidential

_-P.5-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## Example of input/output timing

## = = = = = = (MODE2 H, MODE1 H, SEG1 L, SEG2 L, EVEN L, L/R H)

**==> picture [469 x 225] intentionally omitted <==**

**----- Start of picture text -----**<br>
1 2 3 4 5 6 7 8 1198 1199 1200 1201 1202<br>CPV<br>STV1<br>OUT1<br>OUT2<br>OUT3<br>OUT4<br>OUT5<br>OUT6<br>OUT7<br>OUT8<br>OUT1199<br>OUT1200<br>STV2<br>**----- End of picture text -----**<br>


## Example of input/output timing = = = = = = (MODE2 H, MODE1 H, SEG1 L, SEG2 L, EVEN H, L/R H)

**==> picture [458 x 237] intentionally omitted <==**

**==> picture [14 x 38] intentionally omitted <==**

Himax Confidential

_-P.6-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

## HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## Example of input/output timing

## = = = = = = (MODE2 H, MODE1 H, SEG1 L, SEG2 H, EVEN L, L/R L)

**==> picture [471 x 232] intentionally omitted <==**

**----- Start of picture text -----**<br>
1 2 3 4 5 6 7 8 1198 1199 1200 1201 1202<br>CPV<br>STV2<br>OUT1200<br>OUT1199<br>OUT1198<br>OUT1197<br>OUT1196<br>OUT1195<br>OUT1194<br>OUT1193<br>OUT2<br>OUT1<br>STV1<br>**----- End of picture text -----**<br>


**==> picture [31 x 31] intentionally omitted <==**

## Example of input/output timing

## = = = = = = (MODE2 H, MODE1 H, SEG1 L, SEG2 H, EVEN H, L/R L)

**==> picture [471 x 229] intentionally omitted <==**

**----- Start of picture text -----**<br>
1 2 3 4 5 6 7 8 1198 1199 1200 1201 1202<br>CPV<br>STV2<br>OUT1200<br>OUT1199<br>OUT1198<br>OUT1197<br>OUT1196<br>OUT1195<br>OUT1194<br>OUT1193<br>OUT2<br>OUT1<br>STV1<br>**----- End of picture text -----**<br>


Himax Confidential

_-P.7-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## Example of input/output timing

## = = = = = = (MODE2 H, MODE1 H, SEG1 H, SEG2 L, EVEN L, L/R H)

**==> picture [5 x 28] intentionally omitted <==**

**==> picture [14 x 28] intentionally omitted <==**

**==> picture [14 x 28] intentionally omitted <==**

**==> picture [14 x 28] intentionally omitted <==**

**==> picture [13 x 27] intentionally omitted <==**

**==> picture [14 x 28] intentionally omitted <==**

**==> picture [13 x 38] intentionally omitted <==**

**==> picture [13 x 38] intentionally omitted <==**

**==> picture [13 x 39] intentionally omitted <==**

**==> picture [13 x 39] intentionally omitted <==**

## Example of input/output timing

## = = = = = = (MODE2 H, MODE1 H, SEG1 H, SEG2 L, EVEN H, L/R H)

**==> picture [470 x 240] intentionally omitted <==**

**----- Start of picture text -----**<br>
1 2 3 4 5 6 7 8 1198 1199 1200 1201 1202<br>CPV<br>STV1<br>OUT1<br>OUT2<br>OUT3<br>OUT4<br>OUT5<br>OUT6<br>OUT7<br>OUT8<br>OUT1199<br>OUT1200<br>STV2<br>**----- End of picture text -----**<br>


Himax Confidential

_-P.8-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

## HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## Example of input/output timing

## (MODE2=H, MODE1=H, SEG1=H SEG2=L, EVEN=L, L/R=L)

**==> picture [471 x 234] intentionally omitted <==**

**----- Start of picture text -----**<br>
1 2 3 4 5 6 7 8 1198 1199 1200 1201 1202<br>CPV<br>STV2<br>OUT1200<br>OUT1199<br>OUT1198<br>OUT1197<br>OUT1196<br>OUT1195<br>OUT1194<br>OUT1193<br>OUT 2<br>OUT 1<br>STV1<br>**----- End of picture text -----**<br>


## Example of input/output timing

## = = = = = = (MODE2 H, MODE1 H, SEG1 H, SEG2 L, EVEN H, L/R L)

**==> picture [471 x 213] intentionally omitted <==**

**----- Start of picture text -----**<br>
1 2 3 4 5 6 7 8 1198 1199 1200 1201 1202<br>CPV<br>STV2<br>OUT1200<br>OUT1199<br>OUT1198<br>OUT1197<br>OUT1196<br>OUT1195<br>OUT1194<br>OUT1193<br>OUT2<br>OUT1<br>STV1<br>**----- End of picture text -----**<br>


Himax Confidential

_-P.9-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## Example of input/output timing

## = = = = = = (MODE2 H, MODE1 H, SEG1 H, SEG2 H, EVEN L, L/R H)

**==> picture [470 x 225] intentionally omitted <==**

**----- Start of picture text -----**<br>
1 2 3 4 5 6 7 8 1198 1199 1200 1201 1202<br>CPV<br>STV1<br>OUT1<br>OUT2<br>OUT3<br>OUT4<br>OUT5<br>OUT6<br>OUT7<br>OUT8<br>OUT1199<br>OUT1200<br>STV2<br>**----- End of picture text -----**<br>


**==> picture [31 x 31] intentionally omitted <==**

## Example of input/output timing

## = = = = = = (MODE2 H, MODE1 H, SEG1 H, SEG2 H, EVEN H, L/R H)

**==> picture [471 x 244] intentionally omitted <==**

**----- Start of picture text -----**<br>
1 2 3 4 5 6 7 8 1198 1199 1200 1201 1202<br>CPV<br>STV1<br>OUT1<br>OUT2<br>OUT3<br>OUT4<br>OUT5<br>OUT6<br>OUT7<br>OUT8<br>OUT1199<br>OUT1200<br>STV2<br>**----- End of picture text -----**<br>


Himax Confidential

_-P.10-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

## HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## Example of input/output timing

## (MODE2=H, MODE1=H, SEG1=H SEG2=H, EVEN=L, L/R=L)

**==> picture [470 x 234] intentionally omitted <==**

**----- Start of picture text -----**<br>
1 2 3 4 5 6 7 8 1198 1199 1200 1201 1202<br>CPV<br>STV2<br>OUT1200<br>OUT1199<br>OUT1198<br>OUT1197<br>OUT1196<br>OUT1195<br>OUT1194<br>OUT1193<br>OUT2<br>OUT1<br>STV1<br>**----- End of picture text -----**<br>


## Example of input/output timing

## = = = = = = (MODE2 H, MODE1 H, SEG1 H, SEG2 H, EVEN H, L/R L)

**==> picture [13 x 38] intentionally omitted <==**

**==> picture [5 x 38] intentionally omitted <==**

**==> picture [5 x 37] intentionally omitted <==**

**==> picture [14 x 38] intentionally omitted <==**

**==> picture [5 x 38] intentionally omitted <==**

**==> picture [13 x 38] intentionally omitted <==**

**==> picture [6 x 38] intentionally omitted <==**

**==> picture [5 x 37] intentionally omitted <==**

**==> picture [13 x 37] intentionally omitted <==**

**==> picture [6 x 37] intentionally omitted <==**

**==> picture [13 x 38] intentionally omitted <==**

**==> picture [13 x 37] intentionally omitted <==**

**==> picture [6 x 37] intentionally omitted <==**

**==> picture [5 x 37] intentionally omitted <==**

**==> picture [13 x 38] intentionally omitted <==**

**==> picture [4 x 38] intentionally omitted <==**

Himax Confidential

_-P.11-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A 1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## **5.2 Device power level**

**==> picture [265 x 98] intentionally omitted <==**

**----- Start of picture text -----**<br>
OUT1 to OUT600<br>VGH<br>Logic level<br>VDD<br>VSS<br>VGL<br>**----- End of picture text -----**<br>


**==> picture [35 x 33] intentionally omitted <==**

The logic levels of CPV, L/R, OE1, OE2, OE3, /XAO, MODE1, MODE2, SEG1, SEG2, EVEN, STV1 and STV2 have to swing between VDD for “H” and VSS for “L”. PATH2 has ESD protection diodes connected to VDD and to VSS, the signal on PATH2 should also swing between VDD and VSS.

## **5.3 Power on/off sequence**

**==> picture [33 x 33] intentionally omitted <==**

HX8696-A has a latch up free design. However, to prevent possible power on display noise or possible power off image residue, the power on/off sequence shown below need to be followed.

When power on, VGH/VGL can start to be turned on after VDD reaches 1.8V. After VGH/VGL start to be turned on, CPV and STV should be not floating, and /XAO should be at VDD level or floating. The other control signals have no timing limitation. If the possible power on display noise is not concerned or the backlight is delayed to turn on to shadow the possible noise, there is no power on sequence limitation.

When power off, VGH/VGL must start to be turned off before VDD drops to 1.8V. If the possible power off image residue is not concerned or the backlight is turned off in advance to shadow the possible image residue, there is no power off sequence limitation.

When power on: VDD → VGH/VGL When power off: VGH → VGL → VDD

**==> picture [308 x 140] intentionally omitted <==**

**----- Start of picture text -----**<br>
VGH<br>VDD<br>VSS  1.8V  1.8V<br>t<br>VGL<br>≧ 0s  ≧ 0s ≧ 0s<br>**----- End of picture text -----**<br>


Himax Confidential

_-P.12-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## **6. DC Characteristics**

## **6.1 Absolute maximum rating** (VSS=0V)

**==> picture [463 x 98] intentionally omitted <==**

**----- Start of picture text -----**<br>
Spec.<br>Parameter  Symbol  Unit<br>Min.  Typ.  Max.<br>Power supply voltage 1  VGH  -0.3  - +42.0  V<br>Power supply voltage 2  VDD  -0.3  - +7.0  V<br>Power supply voltage 3  VGL  VGH-42 - +0.3  V<br>Input voltage VIN -0.3 -  VDD+0.3 V<br>Storage temperature  TSTG -55  -  +125  ℃<br>**----- End of picture text -----**<br>


**Note:** Device will probably be damaged permanently in case that the stresses are over the absolute maximum ratings listed above.

## **6.2 Recommended operating conditions** (VSS=0V)

|||**Spec.**|**Spec.**|**Spec.**||
|---|---|---|---|---|---|
|**Parameter**|**Symbol**||||**Unit**|
|||**Min.**|**Typ. **|**Max.**||
| | | | | | | |
|Power supply voltage 1|VGH|7|-|VGL+40|V|
|Power supply voltage 2|VDD|2.2|3.3|3.6|V|
|Power supply voltage 3|VGL|-20|-|-5|V|
|Powersupplyvoltage4|VGH -VGL|12|-|40|V|
|Operation frequency|FCPV|-|-|200|KHz|
|Operation temperature|TA|-40|-|+95|℃|



## **6.3 Electrical characteristics** (VSS=0V)

**==> picture [31 x 31] intentionally omitted <==**

|||<br>**Applicable**||**Spec.**|**Spec.**|**Spec.**||
|---|---|---|---|---|---|---|---|
|**Pt**|**Sbl**||**Condition**||||**Unit**|
|**arameer**|**ymo**|<br>**pin**||**Min.**|**Typ. **|**Max.**||
| | | | | | | |
|Input H voltage|VIH|All input pins|-|0.7VDD|-|VDD|V|
|Input L voltage|VIL|All input pins|-|VSS|-|0.3VDD||
|Output H voltage|VOH|STV1,2|IOH=40µA|VDD-0.4|-|VDD||
|OutputL voltage|VOL|STV1,2|IOL=40µA|VSS|-|VSS+0.4||
|Output H resistance|ROH|OUT1 ~<br>OUT1200|VOUT=<br>VGH-0.5V|-|-|1000|Ω|
|Output L resistance|ROL|OUT1 ~<br>OUT1200|VOUT=<br>VGL+0.5V|-|-|1000|Ω|
|Inputleakage current|IIN|Note(1)|-|-1.0|-|+1.0|µA|
|Pull high/low resistance|RPU|Note(2)|VDD=3.3V,<br>TA=25℃|70|200|400|kΩ|
|Pull high resistance|RPU|/XAO|VIN=VSS|40|-|200|kΩ|
|VGH Power<br>consumption|IVGH|VGH|Note(3)|-|-|200|µA|
|VDD Power<br>consumption|IVDD|VDD||-|-|100||



**Note:** (1) All input except /XAO.

- (2) MODE1, MODE2, SEG1, SEG2, EVEN. L/R

(3) Power consumption in the following condition:

Output no load, VGH=20V, VGL=-8V, VDD=3.0V, VIH=VDD, VIL=VSS, FCPV=50KHz, OE=VIL, /XAO=VIH.

Himax Confidential

_-P.13-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## **7. AC Characteristics**

**==> picture [463 x 189] intentionally omitted <==**

**----- Start of picture text -----**<br>
Spec.<br>Parameter  Symbol  Condition  Unit<br>Min. Typ.  Max.<br>CPV period  tCPV - 5  - -<br>CPV pulse width  tCPVH, tCPVL 50% duty cycle 2.5  - -<br>OE pulse width  tWOE -  1  -  -<br>/XAO pulse width  tWXAO - 10  - -<br>Data setup time  tSU - 1.0  - -<br>Data hold time  tHD - 1.0  - -<br>EVEN setup time   tMSU - 1.0  - - µs<br>EVEN hold time  tMHD - 1.0  - -<br>CPV to output delay time   tPD1 CL=300pF  - - 1.2<br>Start pulse output delay time tPD2 Loading=30pF -  -  1.2<br>OE to output delay time   tPD3 CL=300pF  - - 0.8<br>/XAO to output delay time   tPD4 CL=300pF  -  -  50<br>**----- End of picture text -----**<br>


**Note:** The measurement point for all of above signals is at 50% of input/output amplitude.

**==> picture [33 x 33] intentionally omitted <==**

**==> picture [65 x 80] intentionally omitted <==**

**==> picture [86 x 92] intentionally omitted <==**

**==> picture [56 x 56] intentionally omitted <==**

Himax Confidential

_-P.14-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

1200CH TFT LCD Gate Driver

## HX8696-A

_DATA SHEET Preliminary V01_

## **8. Waveform**

**==> picture [492 x 648] intentionally omitted <==**

**----- Start of picture text -----**<br>
tCPV tCPVH tCPVL<br>CPV  50%<br>tMSU  tSU tHD<br>STV1  50%<br>EVEN  50%<br>tPD1 tPD1<br>OUT1  50%<br>tPD1 tPD1<br>OUT2 ~  50%<br>OUT600<br>50%<br>CPV<br>OUT600  50%<br>tPD2 tPD2 tMHD<br>STV2  50%<br>EVEN  50%<br>tWOE<br>50%<br>OE<br>OUT1 ~<br>50%<br>OUT600<br>tPD3  tPD3 tPD4<br>/XAO<br>50%<br>tWXAO<br>**----- End of picture text -----**<br>


Himax Confidential

_-P.15-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## **9. Pad Coordinates**

## **9.1 HX8696- A gate driver bump location**

**==> picture [464 x 181] intentionally omitted <==**

**----- Start of picture text -----**<br>
PATH2STV2VGHVDDOE2OE3CPVVGLOE1L/R 13151711192123796 101214161820822245 4 3 2 1 1485148414831482 HX8696-AY(0,0) X 286285284283 282 281 280 279 278 267265263261275259273271269277 274272270268266264262260258276 OE2OE1OE3STV1CPVVGLVGHVDDL/RPATH2<br>VSS VSS<br>25 26 27 28 29 30 31 32 33 249 250 251 252 253 254 255 256 257<br>View from bump side<br>Chip size: 23650µm x 670µm (scribe line included)<br>Scribe line: 80µm<br>Bump height: 12 ± 3µm<br>**----- End of picture text -----**<br>


Chip size: 23650µm x 670µm (scribe line included) Scribe line: 80µm Bump height: 12 ± 3µm Total area of IC bump: 3214120µm[[2]]

**==> picture [462 x 233] intentionally omitted <==**

**----- Start of picture text -----**<br>
Total area of IC bump: 3214120µm [[2]]<br>9.2 Bump outline dimensions<br>23650<br>403 403<br>89 115 199 36 36 36 36 199 115 89<br>57 57<br>54<br>52 115 85 25 25 85 115<br>52<br>32<br>18 291 36 18 36 670<br>80 20<br>52<br>52<br>54 70 70<br>57 57<br>90 70 208 22080 208<br>785<br>785<br>9.3 Alignment mark<br>**----- End of picture text -----**<br>


## **9.2 Bump outline dimensions**

## **9.3 Alignment mark**

**==> picture [9 x 35] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

**==> picture [9 x 35] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

**==> picture [115 x 112] intentionally omitted <==**

**==> picture [10 x 35] intentionally omitted <==**

**==> picture [10 x 35] intentionally omitted <==**

**==> picture [9 x 35] intentionally omitted <==**

**==> picture [10 x 35] intentionally omitted <==**

**==> picture [10 x 35] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

**==> picture [12 x 27] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

**==> picture [115 x 112] intentionally omitted <==**

**==> picture [10 x 36] intentionally omitted <==**

**==> picture [9 x 36] intentionally omitted <==**

**==> picture [10 x 36] intentionally omitted <==**

**==> picture [10 x 36] intentionally omitted <==**

**==> picture [10 x 36] intentionally omitted <==**

**==> picture [11 x 26] intentionally omitted <==**

**==> picture [11 x 26] intentionally omitted <==**

**==> picture [11 x 26] intentionally omitted <==**

**==> picture [11 x 26] intentionally omitted <==**

**==> picture [11 x 26] intentionally omitted <==**

**==> picture [10 x 35] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

**==> picture [10 x 35] intentionally omitted <==**

**==> picture [11 x 27] intentionally omitted <==**

Himax Confidential

_-P.16-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

1200CH TFT LCD Gate Driver

## HX8696-A

_DATA SHEET Preliminary V01_

## **9.4 Bump center coordinates**

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|1|/XAO|-11283|262|70x32||61|VSS|-8000|-243|80x70|
|2|/XAO|-11373|262|70x32||62|VSS|-7900|-243|80x70|
|3|EVEN|-11463|262|70x32||63|VSS|-7800|-243|80x70|
|4|EVEN|-11553|262|70x32||64|VSS|-7700|-243|80x70|
|5|OE1|-11643|262|70x32||65|VSS|-7600|-243|80x70|
|6|OE1|-11733|262|70x32||66|VSS|-7500|-243|80x70|
|7|OE2|-11733|208|70x32||67|VSS|-7400|-243|80x70|
|8|OE2|-11643|208|70x32||68|VSS|-7300|-243|80x70|
|9|OE3|-11733|156|70x32||69|VSS|-7200|-243|80x70|
|10|OE3|-11643|156|70x32||70|VSS|-7100|-243|80x70|
|11|L/R|-11733|104|70x32||71|VSS|-7000|-243|80x70|
|12|L/R|-11643|104|70x32||72|VSS|-6900|-243|80x70|
|13|CPV|-11733|52|70x32||73|VSS|-6800|-243|80x70|
|14|CPV|-11643|52|70x32||74|VSS|-6700|-243|80x70|
|15|PATH2|-11733|0|70x32||75|VSS|-6600|-243|80x70|
|16|PATH2|-11643|0|70x32||76|VSS|-6500|-243|80x70|
|17|STV2|-11733|-52|70x32||77|VSS|-6400|-243|80x70|
|18|STV2|-11643|-52|70x32||78|VSS|-6300|-243|80x70|
|19|VGH|-11733|-104|70x32||79|VSS|-6200|-243|80x70|
|20|VGH|-11643|-104|70x32||80|VSS|-6100|-243|80x70|
|21|VGL|-11733|-156|70x32||81|VSS|-6000|-243|80x70|
|22|VGL|-11643|-156|70x32||82|VSS|-5900|-243|80x70|
|23|VDD|-11733|-208|70x32||83|VSS|-5800|-243|80x70|
|24|VDD|-11643|-208|70x32||84|VSS|-5700|-243|80x70|
|25|VSS|-11733|-262|70x32||85|VSS|-5600|-243|80x70|
|26|VSS|-11643|-262|70x32||86|VSS|-5500|-243|80x70|
|27|MODE2|-11553|-262|70x32||87|VSS|-5400|-243|80x70|
|28|MODE1|-11463|-262|70x32||88|VSS|-5300|-243|80x70|
|29|SEG1|-11373|-262|70x32||89|VSS|-5200|-243|80x70|
|30|SEG2|-11283|-262|70x32||90|VSS|-5100|-243|80x70|
|31|PATH3|-11000|-243|80x70||91|VSS|-5000|-243|80x70|
|32|VSS|-10900|-243|80x70||92|VSS|-4900|-243|80x70|
|33|VSS|-10800|-243|80x70||93|VSS|-4800|-243|80x70|
|34|VSS|-10700|-243|80x70||94|VSS|-4700|-243|80x70|
|35|VSS|-10600|-243|80x70||95|VSS|-4600|-243|80x70|
|36|VSS|-10500|-243|80x70||96|VSS|-4500|-243|80x70|
|37|VSS|-10400|-243|80x70||97|VSS|-4400|-243|80x70|
|38|VSS|-10300|-243|80x70||98|VSS|-4300|-243|80x70|
|39|VSS|-10200|-243|80x70||99|VSS|-4200|-243|80x70|
|40|VSS|-10100|-243|80x70||100|VSS|-4100|-243|80x70|
|41|VSS|-10000|-243|80x70||101|VSS|-4000|-243|80x70|
|42|VSS|-9900|-243|80x70||102|VSS|-3900|-243|80x70|
|43|VSS|-9800|-243|80x70||103|VSS|-3800|-243|80x70|
|44|VSS|-9700|-243|80x70||104|VSS|-3700|-243|80x70|
|45|VSS|-9600|-243|80x70||105|VSS|-3600|-243|80x70|
|46|VSS|-9500|-243|80x70||106|VSS|-3500|-243|80x70|
|47|VSS|-9400|-243|80x70||107|VSS|-3400|-243|80x70|
|48|VSS|-9300|-243|80x70||108|VSS|-3300|-243|80x70|
|49|VSS|-9200|-243|80x70||109|VSS|-3200|-243|80x70|
|50|VSS|-9100|-243|80x70||110|VSS|-3100|-243|80x70|
|51|VSS|-9000|-243|80x70||111|VSS|-3000|-243|80x70|
|52|VSS|-8900|-243|80x70||112|VSS|-2900|-243|80x70|
|53|VSS|-8800|-243|80x70||113|VSS|-2800|-243|80x70|
|54|VSS|-8700|-243|80x70||114|VSS|-2700|-243|80x70|
|55|VSS|-8600|-243|80x70||115|VSS|-2600|-243|80x70|
|56|VSS|-8500|-243|80x70||116|VSS|-2500|-243|80x70|
|57|VSS|-8400|-243|80x70||117|VSS|-2400|-243|80x70|
|58|VSS|-8300|-243|80x70||118|VSS|-2300|-243|80x70|
|59|VSS|-8200|-243|80x70||119|VSS|-2200|-243|80x70|
|60|VSS|-8100|-243|80x70||120|VSS|-2100|-243|80x70|



Himax Confidential

_-P.17-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

## HX8696-A

1200CH TFT LCD Gate Driver

## _DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|121|VSS|-2000|-243|80x70||181|VSS|4000|-243|80x70|
|122|VSS|-1900|-243|80x70||182|VSS|4100|-243|80x70|
|123|VSS|-1800|-243|80x70||183|VSS|4200|-243|80x70|
|124|VSS|-1700|-243|80x70||184|VSS|4300|-243|80x70|
|125|VSS|-1600|-243|80x70||185|VSS|4400|-243|80x70|
|126|VSS|-1500|-243|80x70||186|VSS|4500|-243|80x70|
|127|VSS|-1400|-243|80x70||187|VSS|4600|-243|80x70|
|128|VSS|-1300|-243|80x70||188|VSS|4700|-243|80x70|
|129|VSS|-1200|-243|80x70||189|VSS|4800|-243|80x70|
|130|VSS|-1100|-243|80x70||190|VSS|4900|-243|80x70|
|131|VSS|-1000|-243|80x70||191|VSS|5000|-243|80x70|
|132|VSS|-900|-243|80x70||192|VSS|5100|-243|80x70|
|133|VSS|-800|-243|80x70||193|VSS|5200|-243|80x70|
|134|VSS|-700|-243|80x70||194|VSS|5300|-243|80x70|
|135|VSS|-600|-243|80x70||195|VSS|5400|-243|80x70|
|136|VSS|-500|-243|80x70||196|VSS|5500|-243|80x70|
|137|VSS|-400|-243|80x70||197|VSS|5600|-243|80x70|
|138|VSS|-300|-243|80x70||198|VSS|5700|-243|80x70|
|139|VSS|-200|-243|80x70||199|VSS|5800|-243|80x70|
|140|VSS|-100|-243|80x70||200|VSS|5900|-243|80x70|
|141|VSS|0|-243|80x70||201|VSS|6000|-243|80x70|
|142|VSS|100|-243|80x70||202|VSS|6100|-243|80x70|
|143|VSS|200|-243|80x70||203|VSS|6200|-243|80x70|
|144|VSS|300|-243|80x70||204|VSS|6300|-243|80x70|
|145|VSS|400|-243|80x70||205|VSS|6400|-243|80x70|
|146|VSS|500|-243|80x70||206|VSS|6500|-243|80x70|
|147|VSS|600|-243|80x70||207|VSS|6600|-243|80x70|
|148|VSS|700|-243|80x70||208|VSS|6700|-243|80x70|
|149|VSS|800|-243|80x70||209|VSS|6800|-243|80x70|
|150|VSS|900|-243|80x70||210|VSS|6900|-243|80x70|
|151|VSS|1000|-243|80x70||211|VSS|7000|-243|80x70|
|152|VSS|1100|-243|80x70||212|VSS|7100|-243|80x70|
|153|VSS|1200|-243|80x70||213|VSS|7200|-243|80x70|
|154|VSS|1300|-243|80x70||214|VSS|7300|-243|80x70|
|155|VSS|1400|-243|80x70||215|VSS|7400|-243|80x70|
|156|VSS|1500|-243|80x70||216|VSS|7500|-243|80x70|
|157|VSS|1600|-243|80x70||217|VSS|7600|-243|80x70|
|158|VSS|1700|-243|80x70||218|VSS|7700|-243|80x70|
|159|VSS|1800|-243|80x70||219|VSS|7800|-243|80x70|
|160|VSS|1900|-243|80x70||220|VSS|7900|-243|80x70|
|161|VSS|2000|-243|80x70||221|VSS|8000|-243|80x70|
|162|VSS|2100|-243|80x70||222|VSS|8100|-243|80x70|
|163|VSS|2200|-243|80x70||223|VSS|8200|-243|80x70|
|164|VSS|2300|-243|80x70||224|VSS|8300|-243|80x70|
|165|VSS|2400|-243|80x70||225|VSS|8400|-243|80x70|
|166|VSS|2500|-243|80x70||226|VSS|8500|-243|80x70|
|167|VSS|2600|-243|80x70||227|VSS|8600|-243|80x70|
|168|VSS|2700|-243|80x70||228|VSS|8700|-243|80x70|
|169|VSS|2800|-243|80x70||229|VSS|8800|-243|80x70|
|170|VSS|2900|-243|80x70||230|VSS|8900|-243|80x70|
|171|VSS|3000|-243|80x70||231|VSS|9000|-243|80x70|
|172|VSS|3100|-243|80x70||232|VSS|9100|-243|80x70|
|173|VSS|3200|-243|80x70||233|VSS|9200|-243|80x70|
|174|VSS|3300|-243|80x70||234|VSS|9300|-243|80x70|
|175|VSS|3400|-243|80x70||235|VSS|9400|-243|80x70|
|176|VSS|3500|-243|80x70||236|VSS|9500|-243|80x70|
|177|VSS|3600|-243|80x70||237|VSS|9600|-243|80x70|
|178|VSS|3700|-243|80x70||238|VSS|9700|-243|80x70|
|179|VSS|3800|-243|80x70||239|VSS|9800|-243|80x70|
|180|VSS|3900|-243|80x70||240|VSS|9900|-243|80x70|



Himax Confidential

_-P.18-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

## HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|241|VSS|10000|-243|80x70||301|OUT18|10476|235.5|18x85|
|242|VSS|10100|-243|80x70||302|OUT19|10458|125.5|18x85|
|243|VSS|10200|-243|80x70||303|OUT20|10440|235.5|18x85|
|244|VSS|10300|-243|80x70||304|OUT21|10422|125.5|18x85|
|245|VSS|10400|-243|80x70||305|OUT22|10404|235.5|18x85|
|246|VSS|10500|-243|80x70||306|OUT23|10386|125.5|18x85|
|247|VSS|10600|-243|80x70||307|OUT24|10368|235.5|18x85|
|248|VSS|10700|-243|80x70||308|OUT25|10350|125.5|18x85|
|249|VSS|10800|-243|80x70||309|OUT26|10332|235.5|18x85|
|250|VSS|10900|-243|80x70||310|OUT27|10314|125.5|18x85|
|251|PATH3|11000|-243|80x70||311|OUT28|10296|235.5|18x85|
|252|SEG2|11283|-262|70x32||312|OUT29|10278|125.5|18x85|
|253|SEG1|11373|-262|70x32||313|OUT30|10260|235.5|18x85|
|254|MODE1|11463|-262|70x32||314|OUT31|10242|125.5|18x85|
|255|MODE2|11553|-262|70x32||315|OUT32|10224|235.5|18x85|
|256|VSS|11643|-262|70x32||316|OUT33|10206|125.5|18x85|
|257|VSS|11733|-262|70x32||317|OUT34|10188|235.5|18x85|
|258|VDD|11733|-208|70x32||318|OUT35|10170|125.5|18x85|
|259|VDD|11643|-208|70x32||319|OUT36|10152|235.5|18x85|
|260|VGL|11733|-156|70x32||320|OUT37|10134|125.5|18x85|
|261|VGL|11643|-156|70x32||321|OUT38|10116|235.5|18x85|
|262|VGH|11733|-104|70x32||322|OUT39|10098|125.5|18x85|
|263|VGH|11643|-104|70x32||323|OUT40|10080|235.5|18x85|
|264|STV1|11733|-52|70x32||324|OUT41|10062|125.5|18x85|
|265|STV1|11643|-52|70x32||325|OUT42|10044|235.5|18x85|
|266|PATH2|11733|0|70x32||326|OUT43|10026|125.5|18x85|
|267|PATH2|11643|0|70x32||327|OUT44|10008|235.5|18x85|
|268|CPV|11733|52|70x32||328|OUT45|9990|125.5|18x85|
|269|CPV|11643|52|70x32||329|OUT46|9972|235.5|18x85|
|270|L/R|11733|104|70x32||330|OUT47|9954|125.5|18x85|
|271|L/R|11643|104|70x32||331|OUT48|9936|235.5|18x85|
|272|OE3|11733|156|70x32||332|OUT49|9918|125.5|18x85|
|273|OE3|11643|156|70x32||333|OUT50|9900|235.5|18x85|
|274|OE2|11733|208|70x32||334|OUT51|9882|125.5|18x85|
|275|OE2|11643|208|70x32||335|OUT52|9864|235.5|18x85|
|276|OE1|11733|262|70x32||336|OUT53|9846|125.5|18x85|
|277|OE1|11643|262|70x32||337|OUT54|9828|235.5|18x85|
|278|EVEN|11553|262|70x32||338|OUT55|9810|125.5|18x85|
|279|EVEN|11463|262|70x32||339|OUT56|9792|235.5|18x85|
|280|/XAO|11373|262|70x32||340|OUT57|9774|125.5|18x85|
|281|/XAO|11283|262|70x32||341|OUT58|9756|235.5|18x85|
|282|PATH1|10836|235.5|18x85||342|OUT59|9738|125.5|18x85|
|283|OUT0|10800|235.5|18x85||343|OUT60|9720|235.5|18x85|
|284|OUT1|10782|125.5|18x85||344|OUT61|9702|125.5|18x85|
|285|OUT2|10764|235.5|18x85||345|OUT62|9684|235.5|18x85|
|286|OUT3|10746|125.5|18x85||346|OUT63|9666|125.5|18x85|
|287|OUT4|10728|235.5|18x85||347|OUT64|9648|235.5|18x85|
|288|OUT5|10710|125.5|18x85||348|OUT65|9630|125.5|18x85|
|289|OUT6|10692|235.5|18x85||349|OUT66|9612|235.5|18x85|
|290|OUT7|10674|125.5|18x85||350|OUT67|9594|125.5|18x85|
|291|OUT8|10656|235.5|18x85||351|OUT68|9576|235.5|18x85|
|292|OUT9|10638|125.5|18x85||352|OUT69|9558|125.5|18x85|
|293|OUT10|10620|235.5|18x85||353|OUT70|9540|235.5|18x85|
|294|OUT11|10602|125.5|18x85||354|OUT71|9522|125.5|18x85|
|295|OUT12|10584|235.5|18x85||355|OUT72|9504|235.5|18x85|
|296|OUT13|10566|125.5|18x85||356|OUT73|9486|125.5|18x85|
|297|OUT14|10548|235.5|18x85||357|OUT74|9468|235.5|18x85|
|298|OUT15|10530|125.5|18x85||358|OUT75|9450|125.5|18x85|
|299|OUT16|10512|235.5|18x85||359|OUT76|9432|235.5|18x85|
|300|OUT17|10494|125.5|18x85||360|OUT77|9414|125.5|18x85|



Himax Confidential

_-P.19-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

## HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|361|OUT78|9396|235.5|18x85||421|OUT138|8316|235.5|18x85|
|362|OUT79|9378|125.5|18x85||422|OUT139|8298|125.5|18x85|
|363|OUT80|9360|235.5|18x85||423|OUT140|8280|235.5|18x85|
|364|OUT81|9342|125.5|18x85||424|OUT141|8262|125.5|18x85|
|365|OUT82|9324|235.5|18x85||425|OUT142|8244|235.5|18x85|
|366|OUT83|9306|125.5|18x85||426|OUT143|8226|125.5|18x85|
|367|OUT84|9288|235.5|18x85||427|OUT144|8208|235.5|18x85|
|368|OUT85|9270|125.5|18x85||428|OUT145|8190|125.5|18x85|
|369|OUT86|9252|235.5|18x85||429|OUT146|8172|235.5|18x85|
|370|OUT87|9234|125.5|18x85||430|OUT147|8154|125.5|18x85|
|371|OUT88|9216|235.5|18x85||431|OUT148|8136|235.5|18x85|
|372|OUT89|9198|125.5|18x85||432|OUT149|8118|125.5|18x85|
|373|OUT90|9180|235.5|18x85||433|OUT150|8100|235.5|18x85|
|374|OUT91|9162|125.5|18x85||434|OUT151|8082|125.5|18x85|
|375|OUT92|9144|235.5|18x85||435|OUT152|8064|235.5|18x85|
|376|OUT93|9126|125.5|18x85||436|OUT153|8046|125.5|18x85|
|377|OUT94|9108|235.5|18x85||437|OUT154|8028|235.5|18x85|
|378|OUT95|9090|125.5|18x85||438|OUT155|8010|125.5|18x85|
|379|OUT96|9072|235.5|18x85||439|OUT156|7992|235.5|18x85|
|380|OUT97|9054|125.5|18x85||440|OUT157|7974|125.5|18x85|
|381|OUT98|9036|235.5|18x85||441|OUT158|7956|235.5|18x85|
|382|OUT99|9018|125.5|18x85||442|OUT159|7938|125.5|18x85|
|383|OUT100|9000|235.5|18x85||443|OUT160|7920|235.5|18x85|
|384|OUT101|8982|125.5|18x85||444|OUT161|7902|125.5|18x85|
|385|OUT102|8964|235.5|18x85||445|OUT162|7884|235.5|18x85|
|386|OUT103|8946|125.5|18x85||446|OUT163|7866|125.5|18x85|
|387|OUT104|8928|235.5|18x85||447|OUT164|7848|235.5|18x85|
|388|OUT105|8910|125.5|18x85||448|OUT165|7830|125.5|18x85|
|389|OUT106|8892|235.5|18x85||449|OUT166|7812|235.5|18x85|
|390|OUT107|8874|125.5|18x85||450|OUT167|7794|125.5|18x85|
|391|OUT108|8856|235.5|18x85||451|OUT168|7776|235.5|18x85|
|392|OUT109|8838|125.5|18x85||452|OUT169|7758|125.5|18x85|
|393|OUT110|8820|235.5|18x85||453|OUT170|7740|235.5|18x85|
|394|OUT111|8802|125.5|18x85||454|OUT171|7722|125.5|18x85|
|395|OUT112|8784|235.5|18x85||455|OUT172|7704|235.5|18x85|
|396|OUT113|8766|125.5|18x85||456|OUT173|7686|125.5|18x85|
|397|OUT114|8748|235.5|18x85||457|OUT174|7668|235.5|18x85|
|398|OUT115|8730|125.5|18x85||458|OUT175|7650|125.5|18x85|
|399|OUT116|8712|235.5|18x85||459|OUT176|7632|235.5|18x85|
|400|OUT117|8694|125.5|18x85||460|OUT177|7614|125.5|18x85|
|401|OUT118|8676|235.5|18x85||461|OUT178|7596|235.5|18x85|
|402|OUT119|8658|125.5|18x85||462|OUT179|7578|125.5|18x85|
|403|OUT120|8640|235.5|18x85||463|OUT180|7560|235.5|18x85|
|404|OUT121|8622|125.5|18x85||464|OUT181|7542|125.5|18x85|
|405|OUT122|8604|235.5|18x85||465|OUT182|7524|235.5|18x85|
|406|OUT123|8586|125.5|18x85||466|OUT183|7506|125.5|18x85|
|407|OUT124|8568|235.5|18x85||467|OUT184|7488|235.5|18x85|
|408|OUT125|8550|125.5|18x85||468|OUT185|7470|125.5|18x85|
|409|OUT126|8532|235.5|18x85||469|OUT186|7452|235.5|18x85|
|410|OUT127|8514|125.5|18x85||470|OUT187|7434|125.5|18x85|
|411|OUT128|8496|235.5|18x85||471|OUT188|7416|235.5|18x85|
|412|OUT129|8478|125.5|18x85||472|OUT189|7398|125.5|18x85|
|413|OUT130|8460|235.5|18x85||473|OUT190|7380|235.5|18x85|
|414|OUT131|8442|125.5|18x85||474|OUT191|7362|125.5|18x85|
|415|OUT132|8424|235.5|18x85||475|OUT192|7344|235.5|18x85|
|416|OUT133|8406|125.5|18x85||476|OUT193|7326|125.5|18x85|
|417|OUT134|8388|235.5|18x85||477|OUT194|7308|235.5|18x85|
|418|OUT135|8370|125.5|18x85||478|OUT195|7290|125.5|18x85|
|419|OUT136|8352|235.5|18x85||479|OUT196|7272|235.5|18x85|
|420|OUT137|8334|125.5|18x85||480|OUT197|7254|125.5|18x85|



Himax Confidential

_-P.20-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

1200CH TFT LCD Gate Driver

## HX8696-A

_DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|481|OUT198|7236|235.5|18x85||541|OUT258|6156|235.5|18x85|
|482|OUT199|7218|125.5|18x85||542|OUT259|6138|125.5|18x85|
|483|OUT200|7200|235.5|18x85||543|OUT260|6120|235.5|18x85|
|484|OUT201|7182|125.5|18x85||544|OUT261|6102|125.5|18x85|
|485|OUT202|7164|235.5|18x85||545|OUT262|6084|235.5|18x85|
|486|OUT203|7146|125.5|18x85||546|OUT263|6066|125.5|18x85|
|487|OUT204|7128|235.5|18x85||547|OUT264|6048|235.5|18x85|
|488|OUT205|7110|125.5|18x85||548|OUT265|6030|125.5|18x85|
|489|OUT206|7092|235.5|18x85||549|OUT266|6012|235.5|18x85|
|490|OUT207|7074|125.5|18x85||550|OUT267|5994|125.5|18x85|
|491|OUT208|7056|235.5|18x85||551|OUT268|5976|235.5|18x85|
|492|OUT209|7038|125.5|18x85||552|OUT269|5958|125.5|18x85|
|493|OUT210|7020|235.5|18x85||553|OUT270|5940|235.5|18x85|
|494|OUT211|7002|125.5|18x85||554|OUT271|5922|125.5|18x85|
|495|OUT212|6984|235.5|18x85||555|OUT272|5904|235.5|18x85|
|496|OUT213|6966|125.5|18x85||556|OUT273|5886|125.5|18x85|
|497|OUT214|6948|235.5|18x85||557|OUT274|5868|235.5|18x85|
|498|OUT215|6930|125.5|18x85||558|OUT275|5850|125.5|18x85|
|499|OUT216|6912|235.5|18x85||559|OUT276|5832|235.5|18x85|
|500|OUT217|6894|125.5|18x85||560|OUT277|5814|125.5|18x85|
|501|OUT218|6876|235.5|18x85||561|OUT278|5796|235.5|18x85|
|502|OUT219|6858|125.5|18x85||562|OUT279|5778|125.5|18x85|
|503|OUT220|6840|235.5|18x85||563|OUT280|5760|235.5|18x85|
|504|OUT221|6822|125.5|18x85||564|OUT281|5742|125.5|18x85|
|505|OUT222|6804|235.5|18x85||565|OUT282|5724|235.5|18x85|
|506|OUT223|6786|125.5|18x85||566|OUT283|5706|125.5|18x85|
|507|OUT224|6768|235.5|18x85||567|OUT284|5688|235.5|18x85|
|508|OUT225|6750|125.5|18x85||568|OUT285|5670|125.5|18x85|
|509|OUT226|6732|235.5|18x85||569|OUT286|5652|235.5|18x85|
|510|OUT227|6714|125.5|18x85||570|OUT287|5634|125.5|18x85|
|511|OUT228|6696|235.5|18x85||571|OUT288|5616|235.5|18x85|
|512|OUT229|6678|125.5|18x85||572|OUT289|5598|125.5|18x85|
|513|OUT230|6660|235.5|18x85||573|OUT290|5580|235.5|18x85|
|514|OUT231|6642|125.5|18x85||574|OUT291|5562|125.5|18x85|
|515|OUT232|6624|235.5|18x85||575|OUT292|5544|235.5|18x85|
|516|OUT233|6606|125.5|18x85||576|OUT293|5526|125.5|18x85|
|517|OUT234|6588|235.5|18x85||577|OUT294|5508|235.5|18x85|
|518|OUT235|6570|125.5|18x85||578|OUT295|5490|125.5|18x85|
|519|OUT236|6552|235.5|18x85||579|OUT296|5472|235.5|18x85|
|520|OUT237|6534|125.5|18x85||580|OUT297|5454|125.5|18x85|
|521|OUT238|6516|235.5|18x85||581|OUT298|5436|235.5|18x85|
|522|OUT239|6498|125.5|18x85||582|OUT299|5418|125.5|18x85|
|523|OUT240|6480|235.5|18x85||583|OUT300|5400|235.5|18x85|
|524|OUT241|6462|125.5|18x85||584|OUT301|5382|125.5|18x85|
|525|OUT242|6444|235.5|18x85||585|OUT302|5364|235.5|18x85|
|526|OUT243|6426|125.5|18x85||586|OUT303|5346|125.5|18x85|
|527|OUT244|6408|235.5|18x85||587|OUT304|5328|235.5|18x85|
|528|OUT245|6390|125.5|18x85||588|OUT305|5310|125.5|18x85|
|529|OUT246|6372|235.5|18x85||589|OUT306|5292|235.5|18x85|
|530|OUT247|6354|125.5|18x85||590|OUT307|5274|125.5|18x85|
|531|OUT248|6336|235.5|18x85||591|OUT308|5256|235.5|18x85|
|532|OUT249|6318|125.5|18x85||592|OUT309|5238|125.5|18x85|
|533|OUT250|6300|235.5|18x85||593|OUT310|5220|235.5|18x85|
|534|OUT251|6282|125.5|18x85||594|OUT311|5202|125.5|18x85|
|535|OUT252|6264|235.5|18x85||595|OUT312|5184|235.5|18x85|
|536|OUT253|6246|125.5|18x85||596|OUT313|5166|125.5|18x85|
|537|OUT254|6228|235.5|18x85||597|OUT314|5148|235.5|18x85|
|538|OUT255|6210|125.5|18x85||598|OUT315|5130|125.5|18x85|
|539|OUT256|6192|235.5|18x85||599|OUT316|5112|235.5|18x85|
|540|OUT257|6174|125.5|18x85||600|OUT317|5094|125.5|18x85|



Himax Confidential

_-P.21-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A 1200CH TFT LCD Gate Driver

## _DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|601|OUT318|5076|235.5|18x85||661|OUT378|3996|235.5|18x85|
|602|OUT319|5058|125.5|18x85||662|OUT379|3978|125.5|18x85|
|603|OUT320|5040|235.5|18x85||663|OUT380|3960|235.5|18x85|
|604|OUT321|5022|125.5|18x85||664|OUT381|3942|125.5|18x85|
|605|OUT322|5004|235.5|18x85||665|OUT382|3924|235.5|18x85|
|606|OUT323|4986|125.5|18x85||666|OUT383|3906|125.5|18x85|
|607|OUT324|4968|235.5|18x85||667|OUT384|3888|235.5|18x85|
|608|OUT325|4950|125.5|18x85||668|OUT385|3870|125.5|18x85|
|609|OUT326|4932|235.5|18x85||669|OUT386|3852|235.5|18x85|
|610|OUT327|4914|125.5|18x85||670|OUT387|3834|125.5|18x85|
|611|OUT328|4896|235.5|18x85||671|OUT388|3816|235.5|18x85|
|612|OUT329|4878|125.5|18x85||672|OUT389|3798|125.5|18x85|
|613|OUT330|4860|235.5|18x85||673|OUT390|3780|235.5|18x85|
|614|OUT331|4842|125.5|18x85||674|OUT391|3762|125.5|18x85|
|615|OUT332|4824|235.5|18x85||675|OUT392|3744|235.5|18x85|
|616|OUT333|4806|125.5|18x85||676|OUT393|3726|125.5|18x85|
|617|OUT334|4788|235.5|18x85||677|OUT394|3708|235.5|18x85|
|618|OUT335|4770|125.5|18x85||678|OUT395|3690|125.5|18x85|
|619|OUT336|4752|235.5|18x85||679|OUT396|3672|235.5|18x85|
|620|OUT337|4734|125.5|18x85||680|OUT397|3654|125.5|18x85|
|621|OUT338|4716|235.5|18x85||681|OUT398|3636|235.5|18x85|
|622|OUT339|4698|125.5|18x85||682|OUT399|3618|125.5|18x85|
|623|OUT340|4680|235.5|18x85||683|OUT400|3600|235.5|18x85|
|624|OUT341|4662|125.5|18x85||684|OUT401|3582|125.5|18x85|
|625|OUT342|4644|235.5|18x85||685|OUT402|3564|235.5|18x85|
|626|OUT343|4626|125.5|18x85||686|OUT403|3546|125.5|18x85|
|627|OUT344|4608|235.5|18x85||687|OUT404|3528|235.5|18x85|
|628|OUT345|4590|125.5|18x85||688|OUT405|3510|125.5|18x85|
|629|OUT346|4572|235.5|18x85||689|OUT406|3492|235.5|18x85|
|630|OUT347|4554|125.5|18x85||690|OUT407|3474|125.5|18x85|
|631|OUT348|4536|235.5|18x85||691|OUT408|3456|235.5|18x85|
|632|OUT349|4518|125.5|18x85||692|OUT409|3438|125.5|18x85|
|633|OUT350|4500|235.5|18x85||693|OUT410|3420|235.5|18x85|
|634|OUT351|4482|125.5|18x85||694|OUT411|3402|125.5|18x85|
|635|OUT352|4464|235.5|18x85||695|OUT412|3384|235.5|18x85|
|636|OUT353|4446|125.5|18x85||696|OUT413|3366|125.5|18x85|
|637|OUT354|4428|235.5|18x85||697|OUT414|3348|235.5|18x85|
|638|OUT355|4410|125.5|18x85||698|OUT415|3330|125.5|18x85|
|639|OUT356|4392|235.5|18x85||699|OUT416|3312|235.5|18x85|
|640|OUT357|4374|125.5|18x85||700|OUT417|3294|125.5|18x85|
|641|OUT358|4356|235.5|18x85||701|OUT418|3276|235.5|18x85|
|642|OUT359|4338|125.5|18x85||702|OUT419|3258|125.5|18x85|
|643|OUT360|4320|235.5|18x85||703|OUT420|3240|235.5|18x85|
|644|OUT361|4302|125.5|18x85||704|OUT421|3222|125.5|18x85|
|645|OUT362|4284|235.5|18x85||705|OUT422|3204|235.5|18x85|
|646|OUT363|4266|125.5|18x85||706|OUT423|3186|125.5|18x85|
|647|OUT364|4248|235.5|18x85||707|OUT424|3168|235.5|18x85|
|648|OUT365|4230|125.5|18x85||708|OUT425|3150|125.5|18x85|
|649|OUT366|4212|235.5|18x85||709|OUT426|3132|235.5|18x85|
|650|OUT367|4194|125.5|18x85||710|OUT427|3114|125.5|18x85|
|651|OUT368|4176|235.5|18x85||711|OUT428|3096|235.5|18x85|
|652|OUT369|4158|125.5|18x85||712|OUT429|3078|125.5|18x85|
|653|OUT370|4140|235.5|18x85||713|OUT430|3060|235.5|18x85|
|654|OUT371|4122|125.5|18x85||714|OUT431|3042|125.5|18x85|
|655|OUT372|4104|235.5|18x85||715|OUT432|3024|235.5|18x85|
|656|OUT373|4086|125.5|18x85||716|OUT433|3006|125.5|18x85|
|657|OUT374|4068|235.5|18x85||717|OUT434|2988|235.5|18x85|
|658|OUT375|4050|125.5|18x85||718|OUT435|2970|125.5|18x85|
|659|OUT376|4032|235.5|18x85||719|OUT436|2952|235.5|18x85|
|660|OUT377|4014|125.5|18x85||720|OUT437|2934|125.5|18x85|



Himax Confidential

_-P.22-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A 1200CH TFT LCD Gate Driver

## _DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|721|OUT438|2916|235.5|18x85||781|OUT498|1836|235.5|18x85|
|722|OUT439|2898|125.5|18x85||782|OUT499|1818|125.5|18x85|
|723|OUT440|2880|235.5|18x85||783|OUT500|1800|235.5|18x85|
|724|OUT441|2862|125.5|18x85||784|OUT501|1782|125.5|18x85|
|725|OUT442|2844|235.5|18x85||785|OUT502|1764|235.5|18x85|
|726|OUT443|2826|125.5|18x85||786|OUT503|1746|125.5|18x85|
|727|OUT444|2808|235.5|18x85||787|OUT504|1728|235.5|18x85|
|728|OUT445|2790|125.5|18x85||788|OUT505|1710|125.5|18x85|
|729|OUT446|2772|235.5|18x85||789|OUT506|1692|235.5|18x85|
|730|OUT447|2754|125.5|18x85||790|OUT507|1674|125.5|18x85|
|731|OUT448|2736|235.5|18x85||791|OUT508|1656|235.5|18x85|
|732|OUT449|2718|125.5|18x85||792|OUT509|1638|125.5|18x85|
|733|OUT450|2700|235.5|18x85||793|OUT510|1620|235.5|18x85|
|734|OUT451|2682|125.5|18x85||794|OUT511|1602|125.5|18x85|
|735|OUT452|2664|235.5|18x85||795|OUT512|1584|235.5|18x85|
|736|OUT453|2646|125.5|18x85||796|OUT513|1566|125.5|18x85|
|737|OUT454|2628|235.5|18x85||797|OUT514|1548|235.5|18x85|
|738|OUT455|2610|125.5|18x85||798|OUT515|1530|125.5|18x85|
|739|OUT456|2592|235.5|18x85||799|OUT516|1512|235.5|18x85|
|740|OUT457|2574|125.5|18x85||800|OUT517|1494|125.5|18x85|
|741|OUT458|2556|235.5|18x85||801|OUT518|1476|235.5|18x85|
|742|OUT459|2538|125.5|18x85||802|OUT519|1458|125.5|18x85|
|743|OUT460|2520|235.5|18x85||803|OUT520|1440|235.5|18x85|
|744|OUT461|2502|125.5|18x85||804|OUT521|1422|125.5|18x85|
|745|OUT462|2484|235.5|18x85||805|OUT522|1404|235.5|18x85|
|746|OUT463|2466|125.5|18x85||806|OUT523|1386|125.5|18x85|
|747|OUT464|2448|235.5|18x85||807|OUT524|1368|235.5|18x85|
|748|OUT465|2430|125.5|18x85||808|OUT525|1350|125.5|18x85|
|749|OUT466|2412|235.5|18x85||809|OUT526|1332|235.5|18x85|
|750|OUT467|2394|125.5|18x85||810|OUT527|1314|125.5|18x85|
|751|OUT468|2376|235.5|18x85||811|OUT528|1296|235.5|18x85|
|752|OUT469|2358|125.5|18x85||812|OUT529|1278|125.5|18x85|
|753|OUT470|2340|235.5|18x85||813|OUT530|1260|235.5|18x85|
|754|OUT471|2322|125.5|18x85||814|OUT531|1242|125.5|18x85|
|755|OUT472|2304|235.5|18x85||815|OUT532|1224|235.5|18x85|
|756|OUT473|2286|125.5|18x85||816|OUT533|1206|125.5|18x85|
|757|OUT474|2268|235.5|18x85||817|OUT534|1188|235.5|18x85|
|758|OUT475|2250|125.5|18x85||818|OUT535|1170|125.5|18x85|
|759|OUT476|2232|235.5|18x85||819|OUT536|1152|235.5|18x85|
|760|OUT477|2214|125.5|18x85||820|OUT537|1134|125.5|18x85|
|761|OUT478|2196|235.5|18x85||821|OUT538|1116|235.5|18x85|
|762|OUT479|2178|125.5|18x85||822|OUT539|1098|125.5|18x85|
|763|OUT480|2160|235.5|18x85||823|OUT540|1080|235.5|18x85|
|764|OUT481|2142|125.5|18x85||824|OUT541|1062|125.5|18x85|
|765|OUT482|2124|235.5|18x85||825|OUT542|1044|235.5|18x85|
|766|OUT483|2106|125.5|18x85||826|OUT543|1026|125.5|18x85|
|767|OUT484|2088|235.5|18x85||827|OUT544|1008|235.5|18x85|
|768|OUT485|2070|125.5|18x85||828|OUT545|990|125.5|18x85|
|769|OUT486|2052|235.5|18x85||829|OUT546|972|235.5|18x85|
|770|OUT487|2034|125.5|18x85||830|OUT547|954|125.5|18x85|
|771|OUT488|2016|235.5|18x85||831|OUT548|936|235.5|18x85|
|772|OUT489|1998|125.5|18x85||832|OUT549|918|125.5|18x85|
|773|OUT490|1980|235.5|18x85||833|OUT550|900|235.5|18x85|
|774|OUT491|1962|125.5|18x85||834|OUT551|882|125.5|18x85|
|775|OUT492|1944|235.5|18x85||835|OUT552|864|235.5|18x85|
|776|OUT493|1926|125.5|18x85||836|OUT553|846|125.5|18x85|
|777|OUT494|1908|235.5|18x85||837|OUT554|828|235.5|18x85|
|778|OUT495|1890|125.5|18x85||838|OUT555|810|125.5|18x85|
|779|OUT496|1872|235.5|18x85||839|OUT556|792|235.5|18x85|
|780|OUT497|1854|125.5|18x85||840|OUT557|774|125.5|18x85|



Himax Confidential

_-P.23-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A 1200CH TFT LCD Gate Driver

## _DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|841|OUT558|756|235.5|18x85||901|OUT618|-324|235.5|18x85|
|842|OUT559|738|125.5|18x85||902|OUT619|-342|125.5|18x85|
|843|OUT560|720|235.5|18x85||903|OUT620|-360|235.5|18x85|
|844|OUT561|702|125.5|18x85||904|OUT621|-378|125.5|18x85|
|845|OUT562|684|235.5|18x85||905|OUT622|-396|235.5|18x85|
|846|OUT563|666|125.5|18x85||906|OUT623|-414|125.5|18x85|
|847|OUT564|648|235.5|18x85||907|OUT624|-432|235.5|18x85|
|848|OUT565|630|125.5|18x85||908|OUT625|-450|125.5|18x85|
|849|OUT566|612|235.5|18x85||909|OUT626|-468|235.5|18x85|
|850|OUT567|594|125.5|18x85||910|OUT627|-486|125.5|18x85|
|851|OUT568|576|235.5|18x85||911|OUT628|-504|235.5|18x85|
|852|OUT569|558|125.5|18x85||912|OUT629|-522|125.5|18x85|
|853|OUT570|540|235.5|18x85||913|OUT630|-540|235.5|18x85|
|854|OUT571|522|125.5|18x85||914|OUT631|-558|125.5|18x85|
|855|OUT572|504|235.5|18x85||915|OUT632|-576|235.5|18x85|
|856|OUT573|486|125.5|18x85||916|OUT633|-594|125.5|18x85|
|857|OUT574|468|235.5|18x85||917|OUT634|-612|235.5|18x85|
|858|OUT575|450|125.5|18x85||918|OUT635|-630|125.5|18x85|
|859|OUT576|432|235.5|18x85||919|OUT636|-648|235.5|18x85|
|860|OUT577|414|125.5|18x85||920|OUT637|-666|125.5|18x85|
|861|OUT578|396|235.5|18x85||921|OUT638|-684|235.5|18x85|
|862|OUT579|378|125.5|18x85||922|OUT639|-702|125.5|18x85|
|863|OUT580|360|235.5|18x85||923|OUT640|-720|235.5|18x85|
|864|OUT581|342|125.5|18x85||924|OUT641|-738|125.5|18x85|
|865|OUT582|324|235.5|18x85||925|OUT642|-756|235.5|18x85|
|866|OUT583|306|125.5|18x85||926|OUT643|-774|125.5|18x85|
|867|OUT584|288|235.5|18x85||927|OUT644|-792|235.5|18x85|
|868|OUT585|270|125.5|18x85||928|OUT645|-810|125.5|18x85|
|869|OUT586|252|235.5|18x85||929|OUT646|-828|235.5|18x85|
|870|OUT587|234|125.5|18x85||930|OUT647|-846|125.5|18x85|
|871|OUT588|216|235.5|18x85||931|OUT648|-864|235.5|18x85|
|872|OUT589|198|125.5|18x85||932|OUT649|-882|125.5|18x85|
|873|OUT590|180|235.5|18x85||933|OUT650|-900|235.5|18x85|
|874|OUT591|162|125.5|18x85||934|OUT651|-918|125.5|18x85|
|875|OUT592|144|235.5|18x85||935|OUT652|-936|235.5|18x85|
|876|OUT593|126|125.5|18x85||936|OUT653|-954|125.5|18x85|
|877|OUT594|108|235.5|18x85||937|OUT654|-972|235.5|18x85|
|878|OUT595|90|125.5|18x85||938|OUT655|-990|125.5|18x85|
|879|OUT596|72|235.5|18x85||939|OUT656|-1008|235.5|18x85|
|880|OUT597|54|125.5|18x85||940|OUT657|-1026|125.5|18x85|
|881|OUT598|36|235.5|18x85||941|OUT658|-1044|235.5|18x85|
|882|OUT599|18|125.5|18x85||942|OUT659|-1062|125.5|18x85|
|883|OUT600|0|235.5|18x85||943|OUT660|-1080|235.5|18x85|
|884|OUT601|-18|125.5|18x85||944|OUT661|-1098|125.5|18x85|
|885|OUT602|-36|235.5|18x85||945|OUT662|-1116|235.5|18x85|
|886|OUT603|-54|125.5|18x85||946|OUT663|-1134|125.5|18x85|
|887|OUT604|-72|235.5|18x85||947|OUT664|-1152|235.5|18x85|
|888|OUT605|-90|125.5|18x85||948|OUT665|-1170|125.5|18x85|
|889|OUT606|-108|235.5|18x85||949|OUT666|-1188|235.5|18x85|
|890|OUT607|-126|125.5|18x85||950|OUT667|-1206|125.5|18x85|
|891|OUT608|-144|235.5|18x85||951|OUT668|-1224|235.5|18x85|
|892|OUT609|-162|125.5|18x85||952|OUT669|-1242|125.5|18x85|
|893|OUT610|-180|235.5|18x85||953|OUT670|-1260|235.5|18x85|
|894|OUT611|-198|125.5|18x85||954|OUT671|-1278|125.5|18x85|
|895|OUT612|-216|235.5|18x85||955|OUT672|-1296|235.5|18x85|
|896|OUT613|-234|125.5|18x85||956|OUT673|-1314|125.5|18x85|
|897|OUT614|-252|235.5|18x85||957|OUT674|-1332|235.5|18x85|
|898|OUT615|-270|125.5|18x85||958|OUT675|-1350|125.5|18x85|
|899|OUT616|-288|235.5|18x85||959|OUT676|-1368|235.5|18x85|
|900|OUT617|-306|125.5|18x85||960|OUT677|-1386|125.5|18x85|



Himax Confidential

_-P.24-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A 1200CH TFT LCD Gate Driver

## _DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|961|OUT678|-1404|235.5|18x85||1021|OUT738|-2484|235.5|18x85|
|962|OUT679|-1422|125.5|18x85||1022|OUT739|-2502|125.5|18x85|
|963|OUT680|-1440|235.5|18x85||1023|OUT740|-2520|235.5|18x85|
|964|OUT681|-1458|125.5|18x85||1024|OUT741|-2538|125.5|18x85|
|965|OUT682|-1476|235.5|18x85||1025|OUT742|-2556|235.5|18x85|
|966|OUT683|-1494|125.5|18x85||1026|OUT743|-2574|125.5|18x85|
|967|OUT684|-1512|235.5|18x85||1027|OUT744|-2592|235.5|18x85|
|968|OUT685|-1530|125.5|18x85||1028|OUT745|-2610|125.5|18x85|
|969|OUT686|-1548|235.5|18x85||1029|OUT746|-2628|235.5|18x85|
|970|OUT687|-1566|125.5|18x85||1030|OUT747|-2646|125.5|18x85|
|971|OUT688|-1584|235.5|18x85||1031|OUT748|-2664|235.5|18x85|
|972|OUT689|-1602|125.5|18x85||1032|OUT749|-2682|125.5|18x85|
|973|OUT690|-1620|235.5|18x85||1033|OUT750|-2700|235.5|18x85|
|974|OUT691|-1638|125.5|18x85||1034|OUT751|-2718|125.5|18x85|
|975|OUT692|-1656|235.5|18x85||1035|OUT752|-2736|235.5|18x85|
|976|OUT693|-1674|125.5|18x85||1036|OUT753|-2754|125.5|18x85|
|977|OUT694|-1692|235.5|18x85||1037|OUT754|-2772|235.5|18x85|
|978|OUT695|-1710|125.5|18x85||1038|OUT755|-2790|125.5|18x85|
|979|OUT696|-1728|235.5|18x85||1039|OUT756|-2808|235.5|18x85|
|980|OUT697|-1746|125.5|18x85||1040|OUT757|-2826|125.5|18x85|
|981|OUT698|-1764|235.5|18x85||1041|OUT758|-2844|235.5|18x85|
|982|OUT699|-1782|125.5|18x85||1042|OUT759|-2862|125.5|18x85|
|983|OUT700|-1800|235.5|18x85||1043|OUT760|-2880|235.5|18x85|
|984|OUT701|-1818|125.5|18x85||1044|OUT761|-2898|125.5|18x85|
|985|OUT702|-1836|235.5|18x85||1045|OUT762|-2916|235.5|18x85|
|986|OUT703|-1854|125.5|18x85||1046|OUT763|-2934|125.5|18x85|
|987|OUT704|-1872|235.5|18x85||1047|OUT764|-2952|235.5|18x85|
|988|OUT705|-1890|125.5|18x85||1048|OUT765|-2970|125.5|18x85|
|989|OUT706|-1908|235.5|18x85||1049|OUT766|-2988|235.5|18x85|
|990|OUT707|-1926|125.5|18x85||1050|OUT767|-3006|125.5|18x85|
|991|OUT708|-1944|235.5|18x85||1051|OUT768|-3024|235.5|18x85|
|992|OUT709|-1962|125.5|18x85||1052|OUT769|-3042|125.5|18x85|
|993|OUT710|-1980|235.5|18x85||1053|OUT770|-3060|235.5|18x85|
|994|OUT711|-1998|125.5|18x85||1054|OUT771|-3078|125.5|18x85|
|995|OUT712|-2016|235.5|18x85||1055|OUT772|-3096|235.5|18x85|
|996|OUT713|-2034|125.5|18x85||1056|OUT773|-3114|125.5|18x85|
|997|OUT714|-2052|235.5|18x85||1057|OUT774|-3132|235.5|18x85|
|998|OUT715|-2070|125.5|18x85||1058|OUT775|-3150|125.5|18x85|
|999|OUT716|-2088|235.5|18x85||1059|OUT776|-3168|235.5|18x85|
|1000|OUT717|-2106|125.5|18x85||1060|OUT777|-3186|125.5|18x85|
|1001|OUT718|-2124|235.5|18x85||1061|OUT778|-3204|235.5|18x85|
|1002|OUT719|-2142|125.5|18x85||1062|OUT779|-3222|125.5|18x85|
|1003|OUT720|-2160|235.5|18x85||1063|OUT780|-3240|235.5|18x85|
|1004|OUT721|-2178|125.5|18x85||1064|OUT781|-3258|125.5|18x85|
|1005|OUT722|-2196|235.5|18x85||1065|OUT782|-3276|235.5|18x85|
|1006|OUT723|-2214|125.5|18x85||1066|OUT783|-3294|125.5|18x85|
|1007|OUT724|-2232|235.5|18x85||1067|OUT784|-3312|235.5|18x85|
|1008|OUT725|-2250|125.5|18x85||1068|OUT785|-3330|125.5|18x85|
|1009|OUT726|-2268|235.5|18x85||1069|OUT786|-3348|235.5|18x85|
|1010|OUT727|-2286|125.5|18x85||1070|OUT787|-3366|125.5|18x85|
|1011|OUT728|-2304|235.5|18x85||1071|OUT788|-3384|235.5|18x85|
|1012|OUT729|-2322|125.5|18x85||1072|OUT789|-3402|125.5|18x85|
|1013|OUT730|-2340|235.5|18x85||1073|OUT790|-3420|235.5|18x85|
|1014|OUT731|-2358|125.5|18x85||1074|OUT791|-3438|125.5|18x85|
|1015|OUT732|-2376|235.5|18x85||1075|OUT792|-3456|235.5|18x85|
|1016|OUT733|-2394|125.5|18x85||1076|OUT793|-3474|125.5|18x85|
|1017|OUT734|-2412|235.5|18x85||1077|OUT794|-3492|235.5|18x85|
|1018|OUT735|-2430|125.5|18x85||1078|OUT795|-3510|125.5|18x85|
|1019|OUT736|-2448|235.5|18x85||1079|OUT796|-3528|235.5|18x85|
|1020|OUT737|-2466|125.5|18x85||1080|OUT797|-3546|125.5|18x85|



Himax Confidential

_-P.25-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

1200CH TFT LCD Gate Driver

## HX8696-A

_DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|1081|OUT798|-3564|235.5|18x85||1141|OUT858|-4644|235.5|18x85|
|1082|OUT799|-3582|125.5|18x85||1142|OUT859|-4662|125.5|18x85|
|1083|OUT800|-3600|235.5|18x85||1143|OUT860|-4680|235.5|18x85|
|1084|OUT801|-3618|125.5|18x85||1144|OUT861|-4698|125.5|18x85|
|1085|OUT802|-3636|235.5|18x85||1145|OUT862|-4716|235.5|18x85|
|1086|OUT803|-3654|125.5|18x85||1146|OUT863|-4734|125.5|18x85|
|1087|OUT804|-3672|235.5|18x85||1147|OUT864|-4752|235.5|18x85|
|1088|OUT805|-3690|125.5|18x85||1148|OUT865|-4770|125.5|18x85|
|1089|OUT806|-3708|235.5|18x85||1149|OUT866|-4788|235.5|18x85|
|1090|OUT807|-3726|125.5|18x85||1150|OUT867|-4806|125.5|18x85|
|1091|OUT808|-3744|235.5|18x85||1151|OUT868|-4824|235.5|18x85|
|1092|OUT809|-3762|125.5|18x85||1152|OUT869|-4842|125.5|18x85|
|1093|OUT810|-3780|235.5|18x85||1153|OUT870|-4860|235.5|18x85|
|1094|OUT811|-3798|125.5|18x85||1154|OUT871|-4878|125.5|18x85|
|1095|OUT812|-3816|235.5|18x85||1155|OUT872|-4896|235.5|18x85|
|1096|OUT813|-3834|125.5|18x85||1156|OUT873|-4914|125.5|18x85|
|1097|OUT814|-3852|235.5|18x85||1157|OUT874|-4932|235.5|18x85|
|1098|OUT815|-3870|125.5|18x85||1158|OUT875|-4950|125.5|18x85|
|1099|OUT816|-3888|235.5|18x85||1159|OUT876|-4968|235.5|18x85|
|1100|OUT817|-3906|125.5|18x85||1160|OUT877|-4986|125.5|18x85|
|1101|OUT818|-3924|235.5|18x85||1161|OUT878|-5004|235.5|18x85|
|1102|OUT819|-3942|125.5|18x85||1162|OUT879|-5022|125.5|18x85|
|1103|OUT820|-3960|235.5|18x85||1163|OUT880|-5040|235.5|18x85|
|1104|OUT821|-3978|125.5|18x85||1164|OUT881|-5058|125.5|18x85|
|1105|OUT822|-3996|235.5|18x85||1165|OUT882|-5076|235.5|18x85|
|1106|OUT823|-4014|125.5|18x85||1166|OUT883|-5094|125.5|18x85|
|1107|OUT824|-4032|235.5|18x85||1167|OUT884|-5112|235.5|18x85|
|1108|OUT825|-4050|125.5|18x85||1168|OUT885|-5130|125.5|18x85|
|1109|OUT826|-4068|235.5|18x85||1169|OUT886|-5148|235.5|18x85|
|1110|OUT827|-4086|125.5|18x85||1170|OUT887|-5166|125.5|18x85|
|1111|OUT828|-4104|235.5|18x85||1171|OUT888|-5184|235.5|18x85|
|1112|OUT829|-4122|125.5|18x85||1172|OUT889|-5202|125.5|18x85|
|1113|OUT830|-4140|235.5|18x85||1173|OUT890|-5220|235.5|18x85|
|1114|OUT831|-4158|125.5|18x85||1174|OUT891|-5238|125.5|18x85|
|1115|OUT832|-4176|235.5|18x85||1175|OUT892|-5256|235.5|18x85|
|1116|OUT833|-4194|125.5|18x85||1176|OUT893|-5274|125.5|18x85|
|1117|OUT834|-4212|235.5|18x85||1177|OUT894|-5292|235.5|18x85|
|1118|OUT835|-4230|125.5|18x85||1178|OUT895|-5310|125.5|18x85|
|1119|OUT836|-4248|235.5|18x85||1179|OUT896|-5328|235.5|18x85|
|1120|OUT837|-4266|125.5|18x85||1180|OUT897|-5346|125.5|18x85|
|1121|OUT838|-4284|235.5|18x85||1181|OUT898|-5364|235.5|18x85|
|1122|OUT839|-4302|125.5|18x85||1182|OUT899|-5382|125.5|18x85|
|1123|OUT840|-4320|235.5|18x85||1183|OUT900|-5400|235.5|18x85|
|1124|OUT841|-4338|125.5|18x85||1184|OUT901|-5418|125.5|18x85|
|1125|OUT842|-4356|235.5|18x85||1185|OUT902|-5436|235.5|18x85|
|1126|OUT843|-4374|125.5|18x85||1186|OUT903|-5454|125.5|18x85|
|1127|OUT844|-4392|235.5|18x85||1187|OUT904|-5472|235.5|18x85|
|1128|OUT845|-4410|125.5|18x85||1188|OUT905|-5490|125.5|18x85|
|1129|OUT846|-4428|235.5|18x85||1189|OUT906|-5508|235.5|18x85|
|1130|OUT847|-4446|125.5|18x85||1190|OUT907|-5526|125.5|18x85|
|1131|OUT848|-4464|235.5|18x85||1191|OUT908|-5544|235.5|18x85|
|1132|OUT849|-4482|125.5|18x85||1192|OUT909|-5562|125.5|18x85|
|1133|OUT850|-4500|235.5|18x85||1193|OUT910|-5580|235.5|18x85|
|1134|OUT851|-4518|125.5|18x85||1194|OUT911|-5598|125.5|18x85|
|1135|OUT852|-4536|235.5|18x85||1195|OUT912|-5616|235.5|18x85|
|1136|OUT853|-4554|125.5|18x85||1196|OUT913|-5634|125.5|18x85|
|1137|OUT854|-4572|235.5|18x85||1197|OUT914|-5652|235.5|18x85|
|1138|OUT855|-4590|125.5|18x85||1198|OUT915|-5670|125.5|18x85|
|1139|OUT856|-4608|235.5|18x85||1199|OUT916|-5688|235.5|18x85|
|1140|OUT857|-4626|125.5|18x85||1200|OUT917|-5706|125.5|18x85|



Himax Confidential

_-P.26-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

1200CH TFT LCD Gate Driver

## HX8696-A

_DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|1201|OUT918|-5724|235.5|18x85||1261|OUT978|-6804|235.5|18x85|
|1202|OUT919|-5742|125.5|18x85||1262|OUT979|-6822|125.5|18x85|
|1203|OUT920|-5760|235.5|18x85||1263|OUT980|-6840|235.5|18x85|
|1204|OUT921|-5778|125.5|18x85||1264|OUT981|-6858|125.5|18x85|
|1205|OUT922|-5796|235.5|18x85||1265|OUT982|-6876|235.5|18x85|
|1206|OUT923|-5814|125.5|18x85||1266|OUT983|-6894|125.5|18x85|
|1207|OUT924|-5832|235.5|18x85||1267|OUT984|-6912|235.5|18x85|
|1208|OUT925|-5850|125.5|18x85||1268|OUT985|-6930|125.5|18x85|
|1209|OUT926|-5868|235.5|18x85||1269|OUT986|-6948|235.5|18x85|
|1210|OUT927|-5886|125.5|18x85||1270|OUT987|-6966|125.5|18x85|
|1211|OUT928|-5904|235.5|18x85||1271|OUT988|-6984|235.5|18x85|
|1212|OUT929|-5922|125.5|18x85||1272|OUT989|-7002|125.5|18x85|
|1213|OUT930|-5940|235.5|18x85||1273|OUT990|-7020|235.5|18x85|
|1214|OUT931|-5958|125.5|18x85||1274|OUT991|-7038|125.5|18x85|
|1215|OUT932|-5976|235.5|18x85||1275|OUT992|-7056|235.5|18x85|
|1216|OUT933|-5994|125.5|18x85||1276|OUT993|-7074|125.5|18x85|
|1217|OUT934|-6012|235.5|18x85||1277|OUT994|-7092|235.5|18x85|
|1218|OUT935|-6030|125.5|18x85||1278|OUT995|-7110|125.5|18x85|
|1219|OUT936|-6048|235.5|18x85||1279|OUT996|-7128|235.5|18x85|
|1220|OUT937|-6066|125.5|18x85||1280|OUT997|-7146|125.5|18x85|
|1221|OUT938|-6084|235.5|18x85||1281|OUT998|-7164|235.5|18x85|
|1222|OUT939|-6102|125.5|18x85||1282|OUT999|-7182|125.5|18x85|
|1223|OUT940|-6120|235.5|18x85||1283|OUT1000|-7200|235.5|18x85|
|1224|OUT941|-6138|125.5|18x85||1284|OUT1001|-7218|125.5|18x85|
|1225|OUT942|-6156|235.5|18x85||1285|OUT1002|-7236|235.5|18x85|
|1226|OUT943|-6174|125.5|18x85||1286|OUT1003|-7254|125.5|18x85|
|1227|OUT944|-6192|235.5|18x85||1287|OUT1004|-7272|235.5|18x85|
|1228|OUT945|-6210|125.5|18x85||1288|OUT1005|-7290|125.5|18x85|
|1229|OUT946|-6228|235.5|18x85||1289|OUT1006|-7308|235.5|18x85|
|1230|OUT947|-6246|125.5|18x85||1290|OUT1007|-7326|125.5|18x85|
|1231|OUT948|-6264|235.5|18x85||1291|OUT1008|-7344|235.5|18x85|
|1232|OUT949|-6282|125.5|18x85||1292|OUT1009|-7362|125.5|18x85|
|1233|OUT950|-6300|235.5|18x85||1293|OUT1010|-7380|235.5|18x85|
|1234|OUT951|-6318|125.5|18x85||1294|OUT1011|-7398|125.5|18x85|
|1235|OUT952|-6336|235.5|18x85||1295|OUT1012|-7416|235.5|18x85|
|1236|OUT953|-6354|125.5|18x85||1296|OUT1013|-7434|125.5|18x85|
|1237|OUT954|-6372|235.5|18x85||1297|OUT1014|-7452|235.5|18x85|
|1238|OUT955|-6390|125.5|18x85||1298|OUT1015|-7470|125.5|18x85|
|1239|OUT956|-6408|235.5|18x85||1299|OUT1016|-7488|235.5|18x85|
|1240|OUT957|-6426|125.5|18x85||1300|OUT1017|-7506|125.5|18x85|
|1241|OUT958|-6444|235.5|18x85||1301|OUT1018|-7524|235.5|18x85|
|1242|OUT959|-6462|125.5|18x85||1302|OUT1019|-7542|125.5|18x85|
|1243|OUT960|-6480|235.5|18x85||1303|OUT1020|-7560|235.5|18x85|
|1244|OUT961|-6498|125.5|18x85||1304|OUT1021|-7578|125.5|18x85|
|1245|OUT962|-6516|235.5|18x85||1305|OUT1022|-7596|235.5|18x85|
|1246|OUT963|-6534|125.5|18x85||1306|OUT1023|-7614|125.5|18x85|
|1247|OUT964|-6552|235.5|18x85||1307|OUT1024|-7632|235.5|18x85|
|1248|OUT965|-6570|125.5|18x85||1308|OUT1025|-7650|125.5|18x85|
|1249|OUT966|-6588|235.5|18x85||1309|OUT1026|-7668|235.5|18x85|
|1250|OUT967|-6606|125.5|18x85||1310|OUT1027|-7686|125.5|18x85|
|1251|OUT968|-6624|235.5|18x85||1311|OUT1028|-7704|235.5|18x85|
|1252|OUT969|-6642|125.5|18x85||1312|OUT1029|-7722|125.5|18x85|
|1253|OUT970|-6660|235.5|18x85||1313|OUT1030|-7740|235.5|18x85|
|1254|OUT971|-6678|125.5|18x85||1314|OUT1031|-7758|125.5|18x85|
|1255|OUT972|-6696|235.5|18x85||1315|OUT1032|-7776|235.5|18x85|
|1256|OUT973|-6714|125.5|18x85||1316|OUT1033|-7794|125.5|18x85|
|1257|OUT974|-6732|235.5|18x85||1317|OUT1034|-7812|235.5|18x85|
|1258|OUT975|-6750|125.5|18x85||1318|OUT1035|-7830|125.5|18x85|
|1259|OUT976|-6768|235.5|18x85||1319|OUT1036|-7848|235.5|18x85|
|1260|OUT977|-6786|125.5|18x85||1320|OUT1037|-7866|125.5|18x85|



Himax Confidential

_-P.27-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

## HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

|**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**|
|---|---|---|---|---|---|---|---|---|---|---|
|1321|OUT1038|-7884|235.5|18x85||1381|OUT1098|-8964|235.5|18x85|
|1322|OUT1039|-7902|125.5|18x85||1382|OUT1099|-8982|125.5|18x85|
|1323|OUT1040|-7920|235.5|18x85||1383|OUT1100|-9000|235.5|18x85|
|1324|OUT1041|-7938|125.5|18x85||1384|OUT1101|-9018|125.5|18x85|
|1325|OUT1042|-7956|235.5|18x85||1385|OUT1102|-9036|235.5|18x85|
|1326|OUT1043|-7974|125.5|18x85||1386|OUT1103|-9054|125.5|18x85|
|1327|OUT1044|-7992|235.5|18x85||1387|OUT1104|-9072|235.5|18x85|
|1328|OUT1045|-8010|125.5|18x85||1388|OUT1105|-9090|125.5|18x85|
|1329|OUT1046|-8028|235.5|18x85||1389|OUT1106|-9108|235.5|18x85|
|1330|OUT1047|-8046|125.5|18x85||1390|OUT1107|-9126|125.5|18x85|
|1331|OUT1048|-8064|235.5|18x85||1391|OUT1108|-9144|235.5|18x85|
|1332|OUT1049|-8082|125.5|18x85||1392|OUT1109|-9162|125.5|18x85|
|1333|OUT1050|-8100|235.5|18x85||1393|OUT1110|-9180|235.5|18x85|
|1334|OUT1051|-8118|125.5|18x85||1394|OUT1111|-9198|125.5|18x85|
|1335|OUT1052|-8136|235.5|18x85||1395|OUT1112|-9216|235.5|18x85|
|1336|OUT1053|-8154|125.5|18x85||1396|OUT1113|-9234|125.5|18x85|
|1337|OUT1054|-8172|235.5|18x85||1397|OUT1114|-9252|235.5|18x85|
|1338|OUT1055|-8190|125.5|18x85||1398|OUT1115|-9270|125.5|18x85|
|1339|OUT1056|-8208|235.5|18x85||1399|OUT1116|-9288|235.5|18x85|
|1340|OUT1057|-8226|125.5|18x85||1400|OUT1117|-9306|125.5|18x85|
|1341|OUT1058|-8244|235.5|18x85||1401|OUT1118|-9324|235.5|18x85|
|1342|OUT1059|-8262|125.5|18x85||1402|OUT1119|-9342|125.5|18x85|
|1343|OUT1060|-8280|235.5|18x85||1403|OUT1120|-9360|235.5|18x85|
|1344|OUT1061|-8298|125.5|18x85||1404|OUT1121|-9378|125.5|18x85|
|1345|OUT1062|-8316|235.5|18x85||1405|OUT1122|-9396|235.5|18x85|
|1346|OUT1063|-8334|125.5|18x85||1406|OUT1123|-9414|125.5|18x85|
|1347|OUT1064|-8352|235.5|18x85||1407|OUT1124|-9432|235.5|18x85|
|1348|OUT1065|-8370|125.5|18x85||1408|OUT1125|-9450|125.5|18x85|
|1349|OUT1066|-8388|235.5|18x85||1409|OUT1126|-9468|235.5|18x85|
|1350|OUT1067|-8406|125.5|18x85||1410|OUT1127|-9486|125.5|18x85|
|1351|OUT1068|-8424|235.5|18x85||1411|OUT1128|-9504|235.5|18x85|
|1352|OUT1069|-8442|125.5|18x85||1412|OUT1129|-9522|125.5|18x85|
|1353|OUT1070|-8460|235.5|18x85||1413|OUT1130|-9540|235.5|18x85|
|1354|OUT1071|-8478|125.5|18x85||1414|OUT1131|-9558|125.5|18x85|
|1355|OUT1072|-8496|235.5|18x85||1415|OUT1132|-9576|235.5|18x85|
|1356|OUT1073|-8514|125.5|18x85||1416|OUT1133|-9594|125.5|18x85|
|1357|OUT1074|-8532|235.5|18x85||1417|OUT1134|-9612|235.5|18x85|
|1358|OUT1075|-8550|125.5|18x85||1418|OUT1135|-9630|125.5|18x85|
|1359|OUT1076|-8568|235.5|18x85||1419|OUT1136|-9648|235.5|18x85|
|1360|OUT1077|-8586|125.5|18x85||1420|OUT1137|-9666|125.5|18x85|
|1361|OUT1078|-8604|235.5|18x85||1421|OUT1138|-9684|235.5|18x85|
|1362|OUT1079|-8622|125.5|18x85||1422|OUT1139|-9702|125.5|18x85|
|1363|OUT1080|-8640|235.5|18x85||1423|OUT1140|-9720|235.5|18x85|
|1364|OUT1081|-8658|125.5|18x85||1424|OUT1141|-9738|125.5|18x85|
|1365|OUT1082|-8676|235.5|18x85||1425|OUT1142|-9756|235.5|18x85|
|1366|OUT1083|-8694|125.5|18x85||1426|OUT1143|-9774|125.5|18x85|
|1367|OUT1084|-8712|235.5|18x85||1427|OUT1144|-9792|235.5|18x85|
|1368|OUT1085|-8730|125.5|18x85||1428|OUT1145|-9810|125.5|18x85|
|1369|OUT1086|-8748|235.5|18x85||1429|OUT1146|-9828|235.5|18x85|
|1370|OUT1087|-8766|125.5|18x85||1430|OUT1147|-9846|125.5|18x85|
|1371|OUT1088|-8784|235.5|18x85||1431|OUT1148|-9864|235.5|18x85|
|1372|OUT1089|-8802|125.5|18x85||1432|OUT1149|-9882|125.5|18x85|
|1373|OUT1090|-8820|235.5|18x85||1433|OUT1150|-9900|235.5|18x85|
|1374|OUT1091|-8838|125.5|18x85||1434|OUT1151|-9918|125.5|18x85|
|1375|OUT1092|-8856|235.5|18x85||1435|OUT1152|-9936|235.5|18x85|
|1376|OUT1093|-8874|125.5|18x85||1436|OUT1153|-9954|125.5|18x85|
|1377|OUT1094|-8892|235.5|18x85||1437|OUT1154|-9972|235.5|18x85|
|1378|OUT1095|-8910|125.5|18x85||1438|OUT1155|-9990|125.5|18x85|
|1379|OUT1096|-8928|235.5|18x85||1439|OUT1156|-10008|235.5|18x85|
|1380|OUT1097|-8946|125.5|18x85||1440|OUT1157|-10026|125.5|18x85|



Himax Confidential

_-P.28-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A 1200CH TFT LCD Gate Driver

## _DATA SHEET Preliminary V01_

||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||**No.**|**Name**|**X**|**Y**|**Bump**<br>**size(µm)**||
|---|---|---|---|---|---|---|---|---|---|---|---|---|
||1441|OUT1158|-10044|235.5|18x85||1464|OUT1181|-10458|125.5|18x85||
||1442|OUT1159|-10062|125.5|18x85||1465|OUT1182|-10476|235.5|18x85||
||1443|OUT1160|-10080|235.5|18x85||1466|OUT1183|-10494|125.5|18x85||
||1444|OUT1161|-10098|125.5|18x85||1467|OUT1184|-10512|235.5|18x85||
||1445|OUT1162|-10116|235.5|18x85||1468|OUT1185|-10530|125.5|18x85||
||1446|OUT1163|-10134|125.5|18x85||1469|OUT1186|-10548|235.5|18x85||
||1447|OUT1164|-10152|235.5|18x85||1470|OUT1187|-10566|125.5|18x85||
||1448|OUT1165|-10170|125.5|18x85||1471|OUT1188|-10584|235.5|18x85||
||1449|OUT1166|-10188|235.5|18x85||1472|OUT1189|-10602|125.5|18x85||
||1450|OUT1167|-10206|125.5|18x85||1473|OUT1190|-10620|235.5|18x85||
||1451|OUT1168|-10224|235.5|18x85||1474|OUT1191|-10638|125.5|18x85||
||1452|OUT1169|-10242|125.5|18x85||1475|OUT1192|-10656|235.5|18x85||
||1453|OUT1170|-10260|235.5|18x85||1476|OUT1193|-10674|125.5|18x85||
||1454|OUT1171|-10278|125.5|18x85||1477|OUT1194|-10692|235.5|18x85||
||1455|OUT1172|-10296|235.5|18x85||1478|OUT1195|-10710|125.5|18x85||
||1456|OUT1173|-10314|125.5|18x85||1479|OUT1196|-10728|235.5|18x85||
||1457|OUT1174|-10332|235.5|18x85||1480|OUT1197|-10746|125.5|18x85||
||1458|OUT1175|-10350|125.5|18x85||1481|OUT1198|-10764|235.5|18x85||
||1459|OUT1176|-10368|235.5|18x85||1482|OUT1199|-10782|125.5|18x85||
||1460|OUT1177|-10386|125.5|18x85||1483|OUT1200|-10800|235.5|18x85||
||1461|OUT1178|-10404|235.5|18x85||1484|OUT1201|-10818|125.5|18x85||
||1462|OUT1179|-10422|125.5|18x85||1485|PATH1|-10836|235.5|18x85||
||1463|OUT1180|-10440|235.5|18x85||||||||
| | | | | | | |



## **9.5 Alignment mark center coordinates**

|**Name**|**X**|**Y**|
|---|---|---|
|L_AMK|-11101.5|220.5|
|R_AMK|11101.5|220.5|



**==> picture [33 x 33] intentionally omitted <==**

**==> picture [86 x 92] intentionally omitted <==**

Himax Confidential

_-P.29-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_

HX8696-A

1200CH TFT LCD Gate Driver

_DATA SHEET Preliminary V01_

## **10. Ordering Information**

**Part No. Package** PD: mean COG HX8696-A00DPDxxx xxx: mean chip thickness (µm), (default 300µm)

## **11. Revision History**

||||
|---|---|---|
|**Version**|**Date**|**Description of Changes**|
|01|2009/12/21|New setup|
||2010/03/10|Page 12<br>Add PATH2 description in section 5.2.<br>Page 16<br>Correct the title ofsection9.1.|
||2010/08/27|Page 12<br>Modify section5.3: Poweron/offsequence.|
||||



**==> picture [65 x 80] intentionally omitted <==**

**==> picture [86 x 92] intentionally omitted <==**

**==> picture [33 x 33] intentionally omitted <==**

Himax Confidential

_-P.30-_

This information contained herein is the exclusive property of Himax and shall not be distributed, reproduced, or disclosed in whole or in part without prior written permission of Himax.

_August, 2010_
