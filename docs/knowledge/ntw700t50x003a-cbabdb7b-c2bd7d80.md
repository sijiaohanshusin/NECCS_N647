<!-- page: 1 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________

NTW700T50X003A Module(型号)：
Customer（客户）:

Customer P/N（客户型号）：

If there is no special request from customer, NTWwill not reserve the tooling of the
product under the
following conditions:
1.There is no response from customer in one years after N T W submit The samples;
2.There is no order in one years after the latest mass production.And correlated data
(include quality record) will be reserved one year more after tooling was discarded.

Approved by（批准）:
Qualified（合格）: Unqualified（不合格）:

PREPARED CHECKED APPROVED

<!-- page: 2 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
REVISION RECORD

REV NO REV DATE CONTENTS REMARKS

1.0 2016-08-31 First Release

Ver: A - 1 -

<!-- page: 3 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
TABLE OF CONTENT

1.0 GENERAL SPECIFICATIONS

2.0 ELECTRICAL CHARACTERISTICS

3.0 BACKLIGHT CHARACTERISTICS

4.0 DIMENSIONAL DRAWING

5.0 INTERFACE PIN CONNECTIONS

6.0 ELECTRO-OPTICAL CHARACTERISTICS

7.0 INSPECTION CRITERIA

8.0 RELIABILITY

9.0 PRECAUTIONS FOR USING LCD MODULE

10.0 USING LCD MODULES

Ver: A - 2 -

<!-- page: 4 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
1.0 General Specifications
NTW700T50X003A is a color active matrix LCD module
incorporating amorphous silicon TFT (Thin Film Transistor). It is composed of a
color TFT-LCD panel, driver IC, FPC and a back light unit. The module display
area contains 1024x 600pixels. This product accords with RoHS environmental
criterion.

I tem Contents Unit

LCD Type TFT TRANSMISSIVE /

Viewing direction ALL O' Clock

Module outline (W x HxD) 165x100x3.5 mm

Active area (WxH) 154.2144x85.92 mm

Number of Dots 1024(RGB) x600 /

Backlight Type 3*6chips white LED /

Interface Type Parallel RGB 24-bit /

Input voltage 3.3 V

Ver: A - 3 -

<!-- page: 5 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
2.0 ELECTRICAL CHARACTERISTICS

Values
Item Symbol Unit Remark
Min. Typ. Max.
DVDD 3.0 3.3 3.6 V
AVDD 9.4 9.6 9.8 V
Power Voltage
VGH 17 18 19 V
VGL -6.6 -6 -5.4 V
Input signal voltage VCOM 3.1 3.2 3.3 V
Input logic high
VIH 0.7VDD - DVDD V
voltage
Input logic low
VIL 0 - 0.3DVDD V
voltage

Note1：If users use the product out off the environmemtal operation range（temperature and
humidity）,it will have visual quality concerns.
Note2：Be sure to apply DVDD and VGL to the LCD first,and then apply VGH
Note3：DVDD setting should match the signals output voltage(refer to Note4)of customer's system
board.
Note4：DCLK,HS,VS,RESET,U/D,L/R,DE,R0-R7.G0-G7,B0-B7,MODE,DITHB.
Note5：Typical VCOM is only a reference value,it must be optimized according to each LCM. Please
ues VR and base on below application circuit.

Ver: A - 4 -

<!-- page: 6 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________

3.0 BACKLIGHT CHARACTERISTICS

Item Symbol Min Typ Max Unit Conditio
n
Forward voltage Vf - 9.0 - V If=120mA
Luminance Lv - 230 - cd/m2 If=120mA
Number of LED -- 18 Piece --
Connection mode P 3chips serial *6 -- --

Ver: A - 5 -

<!-- page: 7 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
4.0 DIMENSIONAL DRAWING

单层区域 Approved Drawn Checked Approve

易撕贴
Dwg.No

LED
3*6 CIRCUIT
DIAGRAM: 深圳南泰微科技有限公司

补强板

盖住背光焊盘 Date Version Unit ToleranceUnmarked 高温胶纸(7.5*6.5mm)

1.0 A.0mm ±0.2

Ver: A - 6 -

<!-- page: 8 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
5.0 INTERFACE PIN CONNECTIONS
Pin.No Symbol Function

1,2 VLED+ Power for LED backlight (Anode)
3,4 VLED- Power for LED backlight (Cathode)
5 GND Power ground
6 VCOM Common Voltage
7 DVDD Digital Power
8 MODE DE/SYNC mode select. Normally pull high
H: DE mode. L: HSD/VSD mode
9: DE Data Enable signal.
10 VSD Vertical sync input. Negative polarity
11 HSD Horizontal sync input. Negative polarity
12-19 B7-R0 Blue Data
20-27 G7-G0 Green Data
28-35 R7-R0 Red Data
36 GND Ground
37 DCLK Colock signal
38 GND Display on/off
39 SHLR Left or Right Display Control
40 UPDN Up / Down Display Control
41 VDDG Positive Power for TFT
42 VEEG Negative Power for TFT
43 AVDD Analog Power
44 RSTB Global reset pin. Active low to enter reset state.
Suggest to connecting with an RC reset circuit for stability.
Normally pull high. (R=10KΩ，C=1μF)
45 NC Not connect
46 VCOM Common Voltage
47 DITH Dithering setting
DITH=”H” 6bit resolution(last 2 bit of input data truncated)
DITH=”L” 8bit resolution(default setting)
48 GND Power ground
49 NC Not connect
50 NC Not connect

Ver: A - 7 -

<!-- page: 9 -->
## Visual Summary (Page 9)

- page_class: timing_diagram
- confidence: 0.95
- reason_codes: timing_diagram_page, dense_table_present, diagram_complexity, policy_visual_summary_override

Page contains AC Electrical Characteristics table listing timing parameters (setup/hold times) with values in ns and ms. Below is a two-part Input Clock and Data Timing Diagram showing signal waveforms for CLKIN, Data, DEN, VSD, and HSD with annotated timing intervals.

<!-- page: 10 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
5.1.3 Timing

Values
Item Symbol Unit Remark
Min. Typ. Max.
Horizontal Display Area thd - 1024 - DCLK
DCLK Frequency fclk 40.8 51.2 67.2 MHz
One Horizontal Line th 1114 1344 1400 DCLK
HS pulse width thpw 1 - 40 DCLK
HS Blanking thb 160 160 160 DCLK
HS Front Porch thfp 16 160 216 DCLK
Vertical Display Area tvd - 600 - TH
VS period time tv 624 635 750 TH
VS pulse width tvpw 1 - 20 TH
VS Blanking tvb 23 23 23 TH
VS Front Porch tvfp 1 12 127 TH

5.1.4 Data Input Format

Ver: A - 9 -

<!-- page: 11 -->
## Visual Summary (Page 11)

- page_class: timing_diagram
- confidence: 0.95
- reason_codes: timing_diagram_content, visual_sequence_required, policy_visual_summary_override

Page displays two timing diagrams illustrating the Power On/Off sequence for Shenzhen Nantai Micro Technology Co., Ltd. Diagram 'a' shows the power-on ramp-up order (DV_DD -> VGL -> AVDD -> VGH -> Data -> B/L) with specific delay constraints (>20ms, >10ms). Diagram 'b' illustrates the corresponding power-off sequence (B/L -> Data -> VGH -> AVDD -> VGL -> DV_DD) and includes a note specifying supported data signals.

<!-- page: 12 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
6. ELECTRO-OPTICAL CHARACTERISTICS

Item Symbol Condition Temp Min Typ Max Units Note
Transmittance Tr -- 4.8 5.0 ---- % --
Rise Time (Tr) ---- ---- ----
-10℃
Decay Time (Td) ---- ---- ----
Response Time (Tr+Td) 25℃ ---- 30 40 θ=ψ= 0 msec 6.3
Rise Time (Tr) ---- ---- ----
60℃
Decay Time (Td) ---- ---- ----
ψ=0° ---- 85 ----
Viewing Angle ψ=90° ---- 85 ----
θ 25℃ Deg 6.4 Range ψ=180° ---- 85 ----
ψ=270° ---- 85 ----
Contrast Ratio Cr θ=ψ= 0 25℃ 600 800 ---- --- 6.5

6.1 Electro-Optical Characteristics Test Method

Response Time

Photo-detector
Light Source for

Reflective Type

θo θ

Liquid Crystal
Polarizer
Panel

Light Source for

Transmission Type

Ver: A - 11 -

<!-- page: 13 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
6.2 Definition of Operating Voltage, Vop

Vop = (V10,ON +V90,OFF)/2

Brightness Curve for Selected Segment
(%)

90% Brightness Curve for
Non-selected Segment Brightness
100%

10%

V10,ON V90,OFF Driving Voltage

6.3 Definition of Optical Response Time

Non-selecte
d Non-selected
Waveform Waveform
Selected Waveform

(%)
Brightness 100
90%
10%

Time (ms)
Rise Time, Tr Decay Time, Td

Ver: A - 12 -

<!-- page: 14 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
6.4 Definition of Viewing Angle θ and

12 O’clock Direction
Normal :

θ=0°
θ φ=180°

9 O’clock Direction

3 O’clock Direction

φ=270° φ=90°

φ

φ=0°

Viewing Direction

6 O’clock Direction

6.5 Definition of Contrast ratio, CR

CR = Brightness of Non-selected Segment (B2)
Brightness of Selected Segment (B1)

Brightness Curve for

Selected Segment

Brightness Curve for
(%) Contrast
Non-selected Segment
100% B2

CR,max Brightness

0% B1

V,max Driving Voltage

Ver: A - 13 -

<!-- page: 15 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
7.INSPECTION CRITERIA

7.1 Inspection Conditions

7.11 Environmental conditions

The environmental conditions for inspection shall be as follows
Room temperature: 20±3°C
Humidity: 65±20%RH

7.12 The external visual inspection

With a single 20-watt fluorescent lamp as the light source, the inspection was in the distance of 30cm
or more from the LCD to the inspector's eyes .

7.2 Light Method

Fluorescent lamp perpendicular to the display surface.

Ver: A - 14 -

<!-- page: 16 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________

Inspection distance and angle

Inspection should be performed within angleφ(φis usually 30°) from Z axis to each X and Y.
Inspection distance in any direction within φmust be kept 30±5cm from the display surface.

.
7.3 Classification of defects
7.3.1Major defect
A major defect refers to a defect that may substantially degrade usability for product
applications.

7.3.2 Minor defect
A minor defect refers to a defect which is not considered to be able substantially degrade the
product application or a defect that deviates from existing standards almost unrelated to the effective
use of the product or its operation.

8.RELIABILITY

8.1MTBF
The LCD module shall be designed to meet a minimum MTBF value of 50000 hours with normal.
(25°C in the room without sunlight)
8.2 Tests

Ver: A - 15 -

<!-- page: 17 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
NO. Test Item Test condition Criterion
High Temperature Storage 80℃±2℃ 96H 1
Restore 2H at 25℃
Power off
Low Temperature Storage -30℃±2℃ 96H 2
Restore 2H at 25℃
Power off
High Temperature Operation 70℃±2℃ 96H 3
Restore 2H at 25℃
Power on
Low Temperature Operation -20℃±2℃ 96H 4
Restore 2H at 25℃
Power on
High Temperature & 60℃±2℃ 90%RH 96H 5
Humidity Operation Power on
Temperature Cycle --30℃←→25℃←→80℃ Aftertesting,cosmeti 6
30min 5min 30min c
and electrical defects after 10 cycle, Restore 2H at 25℃
should not happen.
Power off

Vibration Test 10Hz~150Hz, 100m/s2, 120min 7
Shock Test Half-sinewave,300m/s2,11ms 8

9 PRECAUTIONS FOR USING LCD MODULE

9.1 HANDING PRECAUTIONS

(1) The display panel is made of glass. Do not subject it to a mechanical shock or impact by dropping
it.
(2) If the display panel is damaged and the liquid crystal substance leaks out, be sure not to get any in
your mouth. If the substance contacts your skin or clothes, wash it off using soap and water.
(3) Do not apply excessive force to the display surface or the adjoining areas since this may cause the
color tone to vary.
(4) The polarizer covering the display surface of the LCD module is soft and easily scratched. Handle
this polarizer carefully.
(5) If the display surface becomes contaminated,breathe on the surface and gently wipe it with a soft
dry cloth. If it is heavily contaminated, moisten a cloth with one of the following solvents:
- Isopropyl alcohol
- Ethyl alcohol

Ver: A - 16 -

<!-- page: 18 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.

_______________________________________________________________________________________________
(6) Solvents other than those above mentioned may damage the polarizer.
Especially, do not use the following:
- Water
- Ketone
- Aromatic solvents
(7) Extra care to minimize corrosion of the electrode. Water droplets, moisture condensation or a
current flow in a high-humidity environment accelerates corrosion of the electrode.
(8) Install the LCD Module by using the mounting holes. When mounting the LCD Module, make
sure it is free of twisting, warping and distortion. In particular, do not forcibly pull or bend the
I/O cable or the backlight cable.
(9) Do not attempt to disassemble or process the LCD Module.
(10) NC terminal should be open. Do not connect anything.
(11) If the logic circuit power is off, do not apply the input signals.
(12) To prevent destruction of the elements by static electricity, be careful to maintain an optimum
work environment.

- Be sure to ground the body when handling he LCD Module.
- Tools required for assembling, such as soldering irons, must be properly grounded.
-To reduce the amount of static electricity generated, do not conduct assembling and other work
under dry conditions.
-The LCD Module is coated with a film to protect the display surface. Exercise care when peeling
off this protective film since static electricity may be generated.
9.2 STORAGE PRECAUTIONS
When storing The LCD Module, avoid exposure to direct sunlight of fluorescent lamps. Keep the
modules in bags (avoid high temperature/ high humidity and low temperatures below 0℃).
Whenever possible, the LCD Module should be stored in the same conditions in which they were
shipped from our company.

9.3 OTHERS
Liquid crystals solidify under low temperature (below the storage temperature range) leading to
defective orientation or the generation of air bubbles (black or white). Air bubbles may also be
generated if the module is subject to a low temperature.
If the LCD Module have been operating for a long time showing the same display
patterns the display patterns may remain on the screen as ghost images and a slight contrast
irregularity may also appear. A normal operating status can be recovered by suspending use for some
time. It should be noted that this phenomenon does not adversely affect performance reliability.
To minimize the performance degradation of the LCD Module resulting from destruction

Ver: A - 17 -

<!-- page: 19 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.
_______________________________________________________________________________________________
caused by static electricity etc. exercise care to avoid holding the following sections when handling
the modules.
- Exposed area of the printed circuit board.
- Terminal electrode sections.

10. USING LCD MODULES

10.1 LIQUID CRYSTAL DISPLAY MODULES

LCD is composed of glass and polarizer. Pay attention to the following items when
handling.
(1) Please keep the temperature within specified range for use and storage. Polarization degradation,
bubble generation or polarizer peel-off may occur with high temperature and high humidity.
(2) Do not touch, push or rub the exposed polarizers with anything harder than a HB pencil lead
(glass, tweezers, etc).
(3) N-hexane is recommended for cleaning the adhesives used to attach front/rear polarizers and
reflectors made of organic substances, which will be damaged by chemicals such as acetone,
toluene, toluene, ethanol and isopropyl alcohol.
(4) When the display surface becomes dusty, wipe gently with absorbent cotton or other soft material
like chamois soaked in petroleum ether. Do not scrub hard to avoid damaging the display surface.
(5) Wipe off saliva or water drops immediately, contact with water over a long period of time may
cause deformation or color fading.
(6) Avoid contacting oil and fats.
(7) Condensation on the surface and contact with terminals due to cold will damage, stain or
polarizers. After products are tested at low temperature they must be warmed up in a container
before coming is contacting with room temperature air.
(8) Do not put or attach anything on the display area to avoid leaving marks on.
(9) Do not touch the display with bare hands. This will stain the display area and degrade insulation
between terminals (some cosmetics are determinate to the polarizers).
(10)As glass is fragile, it tends to become or chipped during handling especially on the edges. Please
avoid dropping or jarring.

10.2 INSTALLING LCD MODULE
Attend to the following items when installing the LCM.
(1) Cover the surface with a transparent protective plate to protect the polarizer and LC cell.
(2) When assembling the LCM into other equipment, the spacer to the bit between the LCM and the

Ver: A - 18 -

<!-- page: 20 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.
_______________________________________________________________________________________________
fitting plate should have enough height to avoid causing stress to the module surface, refer to the
individual specifications for measurements. The measurement tolerance should be ±0.1mm.

10.3 ELECTRO-STATIC DISCHARGE CONTROL

Since this module uses a CMOS LSI, the same careful attention should be paid for
electrostatic discharge as for an ordinary CMOS IC.
(1) Make certain that you are grounded when handing LCM.
(2) Before removing LCM from its packing case or incorporating it into a set, be sure the module and
your body have the same electric potential.
(3) When soldering the terminal of LCM, make certain the AC power source for the soldering iron
does not leak.
(4) When using an electric screwdriver to attach LCM, the screwdriver should be of ground
potentiality to minimize as much as possible any transmission of electromagnetic waves produced
sparks coming from the commutator of the motor.
(5) As far as possible, make the electric potential of your work clothes and that of the workbenches to
the ground potential.
(6) To reduce the generation of static electricity , be careful that the air in the work is not too dried. A relative humidity

of 50%-60% is recommended.
10.4 PRECAUTION FOR SOLDERING TO THE LCM
(1) Observe the following when soldering lead wire, connector cable and etc. to the LCM.
-Soldering iron temperature: 280 ±10°C.
-Soldering time: 3-4 sec.
-Solder: eutectic solder.
If soldering flux is used, be sure to remove any remaining flux after finishing to soldering
operation. (This does not apply in the case of a non-halogen type of flux.) It is recommended that you
protect the LCD surface with a cover during soldering the prevent any damage due to flux spatters.
(2) When soldering the electroluminescent panel and PC board, the panel and board should not be
detached more than three times. This maximum number is determined by the temperature and
time conditions mentioned above, though there may be some variance depending on the
temperature of the soldering iron.
(3) When removing the electroluminescent panel from the PC board, be sure the solder has
completely melted, otherwise the soldered pad on the PC board could be damaged.

10.5 PRECAUTIONS FOR OPERATION

Ver: A - 19 -

<!-- page: 21 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.
_______________________________________________________________________________________________
(1) Viewing angle varies with the change of liquid crystal driving voltage (Vo). Adjust Vo to show the
best contrast.
(2) Driving the LCD in the voltage above the limit will shorten its lifetime.
(3) Response time is greatly delayed at temperature below the operating temperature range. However,
this does not mean the LCD will be out of the order. It will recover when it returns to the
specified temperature range.
(4) If the display area is pushed hard during operation, the display will become abnormal. However,
it will return to normal if it is turned off and then on.
(5) Condensation on terminals can cause an electrochemical reaction disrupting the terminal circuit.
Therefore, it must be used under the relative condition of 40°C, 50% RH.
(6) When turning the power on, input each signal after the positive/negative voltage becomes stable.

10.6 STORAGE
When storing LCDS as spares for some years, the following precaution are necessary.
(1) Store them in a sealed polyethylene bag. If properly scaled, there is no need for desiccant.
(2) Store them in a dark place. Do not expose to sunlight or fluorescent light, keep the temperature
between 0°C and 35°C.
(3) The polarizer surface should not come in contact with any other objects. (We advise you to store
them in the container in which they were shipped.)
(4) Environmental conditions:
-Do not leave them for more than 168hrs. at 60 °C.
-Should not be left for more than 48hrs. at –20 °C.

10.7 SAFETY

(1) It is recommended to crush damaged or unnecessary LCDs into pieces and wash them off with
solvents such as acetone and ethanol, which should later be burned.
(2) If any liquid leaks out of a damaged glass cell and comes in contact with the hands, wash off
thoroughly with soap and water.

10.8 LIMITED WARRANTY

Unless agreed between JST and customer, JST will replace or repair any of its LCD and
modules which are found to be functionally defective when inspected in accordance with JST LCD
acceptance standards (copies available upon request) for a period of one year from date of shipments.
Cosmetic/visual defects must be returned to JST within 90 days of shipment. Confirmation of such
date shall be based on freight documents. The warranty liability of JST is limited to repair and/or

Ver: A - 20 -

<!-- page: 22 -->
深圳南泰微科技有限公司

Shenzhen Nantai Micro Technology Co. , Ltd.
_______________________________________________________________________________________________
replacement on the terms set forth above. JST will not be responsible for any subsequent or
consequential events.

10.9. RETURN LCM UNDER WARRANTY
No warranty can be granted if the precautions stated above have been disregarded. The
typical examples of violations are:
-Broken LCD glass.
-PCB eyelet’s damaged or modified.
-PCB conductors damaged.
-Circuit modified in any way, including addition of components.
-PCB tampered with by grinding, engraving or painting varnish.
-Soldering to or modifying the bezel in any manner.
Module repairs will be invoiced to the customer upon mutual agreement. Modules must
be returned with sufficient description of the failures or defects. Any connectors or cable installed by
the customer must be removed completely without damaging the PCB eyelet’s conductors and
terminals.

Ver: A - 21 -
