<!-- page: 1 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

SPECIFICATION

Revision: 0.0

WKS70WSV081-WCT

This module uses ROHS material

This specification may change without prior notice in order to improve performance or quality. Please
contact WKS R&D department for updated specification and product status before design for this product
or release of this order.

WRITTEN BY CHECKED BY APPROVED BY

Jason Eric Stone

WKS Technology Co., LTD WKS CONFIDENTIAL P.1

<!-- page: 2 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023
REVISION RECORD

REV NO. REV DATE CONTENTS REMARKS

0.0 2023-5-11 First release Preliminary

WKS Technology Co., LTD WKS CONFIDENTIAL P.2

<!-- page: 3 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023
CONTENTS

1、GENERAL INFORMATION

2、EXTERNAL DIMENSIONS

3、ABSOLUTE MAXIMUM RATINGS

4、ELECTRICAL CHARACTERISTICS

5、TOUCH CHARACTERISTICS

6、ELECTRO-OPTICAL CHARACTERISTICS

7、INTERFACE DESCRIPTION

8、RELIABILITY TEST CONDITIONS

9、INSPECTION CRITERION

WKS Technology Co., LTD WKS CONFIDENTIAL P.3

<!-- page: 4 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

1、GENERAL INFORMATION

Item of general information Contents Unit

LCD Display Size (Diagonal) 7.0” inch

Module Structure LCD + C-TOUCH + PCB -

LCD Display Type TFT/TRANSMISSIVE -

LCD Display Mode Normally Black -

Recommended Viewing Direction ALL VIEW o’clock

Module size (W×H×T) 164.90×100.00×7.98 mm

Active area (W×H) 154.21×85.92 mm

Number of pixels (Resolution) 1024(RGB)×600 pixel

Pixel pitch (W×H) 0.1506×0.1432 mm

LCD Driver IC - -

Module Interface Type 4 wire SPI interface -

Module Input voltage 5.0V V

Module Power consumption - mW

Color Numbers 262K -

Backlight Type White LED -

WKS Technology Co., LTD WKS CONFIDENTIAL P.4

<!-- page: 5 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

2、EXTERNAL DIMENSIONS

3、ABSOLUTE MAXIMUM RATINGS

Parameter of absolute
Symbol Min Max Unit
maximum ratings

Operating temperature Top -20 70 ℃

Storage temperature Tst -30 80 ℃

Humidity RH - 90%(Max 60℃) RH

Note: Absolute maximum ratings means the product can withstand short-term, not more than

120 hours. If the product is a long time to withstand these conditions, the life time would be

shorter.

WKS Technology Co., LTD WKS CONFIDENTIAL P.5

<!-- page: 6 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

4、ELECTRICAL CHARACTERISTICS(DC CHARACTERISTICS)

Parameter of DC
Symbol Min. Typ. Max. Unit
characteristics

PCB operating voltage VCC5V - 5.0 - V

LCD I/O operating voltage VDD 3.0 3.3 3.6 V

Input voltage ‘H’ level VIH 2 - 3.6 V

Input voltage ‘L’ level VIL -0.3 - 0.8 V

Output voltage ‘H’ level VOH 2.4 - - V

Output voltage ‘L’ level VOL - - 0.4 V

5、TOUCH CHARACTERISTICS

Item of CTP
Specification Unit Remark
characteristics

Panel Type Glass Cover + Glass Sensor - -

Resolution 1024 × 600 pixel -

Surface Hardness ≥6H - -

Transparency ＞82% - -

Driver IC - - -

Interface Type I2C - -

Support Points 5 - -

Sampling Rate 20~100 Hz -

Supply voltage 3.3 V -

WKS Technology Co., LTD WKS CONFIDENTIAL P.6

<!-- page: 7 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

6、ELECTRO-OPTICAL CHARACTERISTICS

Item of

electro-optical Symbol Condition Min. Typ. Max. Unit Remark Note

characteristics

Response time Tr+Tf - 30 40 ms FIG 1. 4
θ=0
Contrast Ratio CR - 500 - - FIG 2. 1
=0
Luminance uniformity WHITE - 80 - % FIG 2. 3 Ta=25℃
Surface Luminance Lv - 350 - cd/m2 FIG 2. 2
White x - 0.308 -
White
White y - 0.336 -
Red x - 0.599 -
Red θ=0
CIE (x, y) Red y - 0.338 -
=0 - FIG 2. 5
chromaticity Green x - 0.299 -
Green Ta=25℃
Green y - 0.550 -
Blue x - 0.139 -
Blue
Blue y - 0.131 -
=90(12 o’clock) - 85 - deg
Viewing =270(6 o’clock) - 85 - deg
CR 10 FIG 3. 6
angle range =0(3 o’clock) - 85 - deg
=180(9 o’clock) - 85 - deg
NTSC ratio - - - 50 - % - -

Note 1. Contrast Ratio (CR) is defined mathematically by the following formula. For more

information see FIG 2.:

Note 2. Surface luminance is the LCD surface from the surface with all pixels displaying

white. For more information see FIG 2.

Lv=Average Surface Luminance with all white pixels (P1,P2,P3,P4,P5,P6,P7,P8,P9)

Note 3. The uniformity in surface luminance（WHITE）is determined by measuring

luminance at each test position 1 through 9, and then dividing the maximum luminance of

WKS Technology Co., LTD WKS CONFIDENTIAL P.7

<!-- page: 8 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

9points luminance by minimum luminance of 9 points luminance. For more information see

FIG 2.

Note 4. Response time is the time required for the display to transition from White to

black(Rise Time, Tr) and from black to white(Decay Time, Tf). For additional information

see FIG 1.

Note 5. CIE (x, y) chromaticity ,The x,y value is determined by screen active area position 5.

For more information see FIG 2.

Note 6. Viewing angle is the angle at which the contrast ratio is greater than a specific value.

For TFT module, the specific value of contrast ratio is 10.The angles are determined for the

horizontal or x axis and the vertical or y axis with respect to the z axis which is normal to the

LCD surface. For more information see FIG 3.

Note 7. For Viewing angle and response time testing, the testing data is base on

Autronic-Melchers’s ConoScope. Series Instruments. For contrast ratio, Surface Luminance,

Luminance uniformity and CIE，the testing data is base on BM-7 photo detector.

Note 8. For TN type TFT transmissive module, Gray scale reverse occurs in the direction of

panel viewing angle.

FIG.1. The definition of Response Time

FIG.2. Measuring method for Contrast ratio, surface luminance, Luminance

WKS Technology Co., LTD WKS CONFIDENTIAL P.8

<!-- page: 9 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

uniformity, CIE (x , y) chromaticity

A : H/6 ;
B : V/6 ;
H,V : Active Area(AA) size
Measurement instrument: BM-7; Light spot size=5mm, 350mm distance from the LCD
surface to detector lens.

FIG.3. The definition of viewing angle

WKS Technology Co., LTD WKS CONFIDENTIAL P.9

<!-- page: 10 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

7、INTERFACE DESCRIPTION

7.1、J1&J2 Interface Description

NO. Symbol I/O DESCRIPTION

1~2 VCC5V Power supply Module Power supply (5V Typ.)

3~4 GND Power supply Power ground

5 LCD_CS I Chip Select pin for 3-wire or 4-wire serial I/F.

6 LCD_MOSI I Data input pin of 4-wire SPI I/F.

7 LCD_SCLK I Clock of 3-wire or 4-wire serial I/F.

8 LCD_INT O The interrupt output for host to indicate the status.

9 LCD_RST I This is an active low Reset pin for LCD.

Data output pin of 4-wire SPI I/F.
10 LCD_MISO O
Bi-direction data pin of 3-wire SPI I/F.

11 CTP_INT O CTP External interrupt to the host

12 CTP_SCL I CTP I2C clock input

13 CTP_SDA I/O CTP I2C data input and output

14 CTP_RST I CTP external reset signal, Low is active

15 NC - No connection

16 GND Power supply Power ground

17 NC - No connection

18 NC - No connection

19 NC - No connection

20 GND Power supply Power ground

WKS Technology Co., LTD WKS CONFIDENTIAL P.10

<!-- page: 11 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

7.2、J3 Interface Description (SPI Flash burning interface)

NO. Symbol I/O DESCRIPTION

1 SF_CLK I Serial Clock Input

2 SF_DI I Data Input

3 SF_DO O Data output

4 SF_CS I Chip Select Input

LCD RESET signal. This pin must be pull low when burning SPI
5 LCD_RST I
FLASH

6 GND Power supply Power ground

7 NC - No connection

8 NC - No connection

9~10 3.3V Power supply Power supply for the SPI Flash (3.3V Typ.)

WKS Technology Co., LTD WKS CONFIDENTIAL P.11

<!-- page: 12 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

8、RELIABILITY TEST CONDITIONS

No. Test Item Test Condition

1 High Temperature Storage 80℃/120 hours

2 Low Temperature Storage -30℃/120 hours

3 High Temperature Operating 70℃/120 hours

4 Low Temperature Operating -20℃/120 hours

5 Temperature Cycle Storage -20℃(30min.)~25(5min.)~70℃(30min.)×10cycles

A、Inspection after test:

Inspection after 2~4 hours storage at room temperature, the sample shall be free from

defects:

Air bubble in the LCD;

Sealleak;

Non-display;

Missing segments;

Glass crack;

Current is twice higher than initial value.

B、Remark:

The test samples should be applied to only one test item.

Sample size for each test item is 5~10pcs.

Failure Judgment Criterion: Basic Specification, Electrical Characteristic, Mechanical

Characteristic, Optical Characteristic.

WKS Technology Co., LTD WKS CONFIDENTIAL P.12

<!-- page: 13 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

9、INSPECTION CRITERION

This specification is made to be used as the standard of acceptance/rejection criteria for

TFT-LCD/IPS TFT-LCD module product, and this specification is applicable only in the

case that the size of module equal to or exceed than 3.5 inch.

9.1 Sample plan

Sampling plan according to GB/T2828.1-2003/ISO 2859-1：1999 and ANSI/ASQC

Z1.4-1993,normal level 2 and based on:

Major defect: AQL 0.65

Minor defect: AQL 1.5

9.2 Inspection condition

Viewing distance for cosmetic inspection is about 30cm with bare eyes, and under an

environment of 20~40W light intensity, all directions for inspecting the sample should be

within 45°against perpendicular line. (Normal temperature 20~25℃and normal humidity 60

±15%RH )

9.3 Definition of Inspection Item.

A、Definition of inspection zone in LCD.

Zone A: character/Digit area

WKS Technology Co., LTD WKS CONFIDENTIAL P.13

<!-- page: 14 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

Zone B: viewing area except Zone A (Zone A + Zone B=minimum Viewing area)

Zone C: Outside viewing area (invisible area after assembly in customer’s product)

Fig.1 Inspection zones in an LCD

Note: As a general rule, visual defects in Zone C are permissible, when it is no trouble for

quality and assembly of customer’s product.

B、Definition of some visual defect

Because of losing all or part function, bad pixel dots appear bright and the

Bright dot size is more than 50% of one dot in which LCD panel is displaying under

black pattern.

Dots appear dark and unchanged in size in which LCD panel is displaying
Dark dot
under pure red, green, blue picture, or pure whiter picture.

9.4 Major Defect

Item Items to be Classification
Inspection standard
No. inspected of defects

1) No display
2) Display abnormally
3) Missing vertical, horizontal segment
1 Functional defects 4) Short circuit
5) Excess power consumption
major 6)Backlight no lighting, flickering and abnormal
lighting

2 Missing Missing component
Overall outline dimension beyond the drawing is not
3 Outline dimension
allowed

WKS Technology Co., LTD WKS CONFIDENTIAL P.14

<!-- page: 15 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

9.5 Minor Defect

Item Items to be Classification
Inspection standard
No. inspected of defects

Zone Acceptable Qty
A+B
3.5”～7” 7～10.1” ＞10.1” C
Bright pixel dot 1 2 3
Bright dot Dark pixel dot 4 4 4
1 /dark dot 2bright dots adjacent 0 0 0 Minor defect 2dark dots adjacent 0 0 0 Acceptable
Total bright and dark dots 5 6 7
Note: Minimum distance between defective dots is more than 5mm;
Pixel dots’ function is normal, but bright dots caused by foreign
material and other reasons are judged by the dot defect of 5.2.
Zone Acceptable Qty
A+B
Size(mm) 3.5”～7” 7～10.1” ＞10.1” C
Φ≤0.2 Acceptable Acceptable Acceptable

0.2＜Φ≤0.5 4 5 6
2 Minor Acceptable Φ＞0.5 0 0 0

Note:
1. Minimum distance between defective dots is more than 5 mm;
2. The quantity of defect is zero in operating condition.

Zone Acceptable Qty

Size (mm) A+B

Length Width 3.5”～7” 7～10.1” ＞10.1” C
Linear
3 Minor
defect Ignore W≤0.05 Acceptable Acceptable Acceptable
0.05＜
L≤5.0 4 5 6
W≤0.1 Acceptable
L＞5.0 W＞0.1 0 0 0

WKS Technology Co., LTD WKS CONFIDENTIAL P.15

<!-- page: 16 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023
5.4.1 Polarizer Position
( i) Shifting in position should not exceed the glass outline
dimension.
( ii ) Incomplete covering of the viewing area due to shifting is not
allowed.
5.4.2 Dirt on polarizer
Dirt which can be wiped easily should be acceptable.
5.4.3 Polarizer Dent & Air bubble
Zone Acceptable Qty
A+B
Size(mm) 3.5”～7” 7～10.1” ＞10.1” C
Φ≤0.2 Acceptable Acceptable Acceptable

0.2＜Φ≤0.5 4 5 6 Acceptable Polarizer Φ＞0.5 0 0 0
4 Minor
defect
5.4.4 Polarizer scratch
(i) If the polarizer scratch can be seen after cover assembling
or in the operating condition, judge by the linear defect of 5.3.
( ii )If the polarizer scratch can be seen only in non-operating
condition or some special angle, judge by the following:
Zone Acceptable Qty

Size (mm) A+B

Length Width 3.5”～7” 7～10.1” ＞10.1” C

Ignore W≤0.05 Acceptable Acceptable Acceptable

1.0＜L 0.05＜
4 5 6
≤5.0 W≤0.20 Acceptable
L＞5.0 W＞0.2 0 0 0

MURA Using 3% ND filter, it’s NG if it can be seen in R,G,B picture.

5
Minor

White/Black Visible under：ND3%；D≦0.15mm, Acceptable；
dot（MURA） 0.15mm<D≦0.5mm, N≦4；D>0.5mm, Not allowable.

WKS Technology Co., LTD WKS CONFIDENTIAL P.16

<!-- page: 17 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023
(i) Crack
Cracks are not allowed.

Minor

(ii) TFT chips on corner

Minor

Glass
6 X Y Z Acceptable
defect
Not more than the
≤3.0 ≤3.0 N≤3
thickness of glass
Chips on the corner of terminal shall not be allowed to extend
into the ITO pad or expose perimeter seal.
(iii) Usual surface crack

Minor

X Y Z Acceptable
Not more than the
≤1.5 ≤1.5 N≤4
thickness of glass
It is only applicable to the upper glass of LCD.

WKS Technology Co., LTD WKS CONFIDENTIAL P.17

<!-- page: 18 -->
TOUCH MODULE WKS70WSV081-WCT Version: 0.0 May 11, 2023

9.6 Module Cosmetic Criteria

Item Items to be Classification
Inspection Standard
No. inspected of defects
1 Difference in Spec. Not allowable Major
2 Pattern peeling No substrate pattern peeling and floating Major
No soldering missing Major
3 Soldering defects No soldering bridge Major
No cold soldering Minor
Visible copper foil (Φ0.5 mm or more) on substrate
4 Resist flaw on PCB Minor
pattern is not allowed
5 FPC gold finger No dirt, breaking, oxidation lead to black Major
Backlight plastic No deformation, crack, breaking, backlight positioning column
6 Minor
frame breaking, obvious nick.
Marking printing No dark marking, incomplete, deformation lead to
7 Minor
effect unable to judge
Accretion of
8 metallic No accretion of metallic foreign matter (Not exceed Φ0.2mm) Minor
Foreign matter
9 Stain No stain to spoil cosmetic badly Minor
10 Plate discoloring No plate fading, rusting and discoloring Minor

a. Soldering side of PCB Solder to form a ‘Filet’ all around the Minor
lead. Solder should not hide the lead form perfectly.
1. Lead parts
b. Components side(In case of ‘Through Hole PCB') Minor
Solder to reach the Components side of PCB.

Either ‘Toe’(A) or ‘Seal’(B)of the lead to be covered by “Filet”.
Lead form to be assume over Solder.
2. Flat packages Minor

11
(3/2) H ≥h ≥(1/2) H
Minor
3. Chips

a. The spacing between solder ball and the conductor or solder Minor
pad h ≥0.13 mm. The diameter of solder ball d≤0.15 mm.
4. Solder ball/Solder b. The quantity of solder balls or solder splashes isn’t beyond 5 in Minor
splash 600 mm2.

c. Solder balls/Solder splashes do not violate minimum electrical
Major
clearance.

WKS Technology Co., LTD WKS CONFIDENTIAL P.18
