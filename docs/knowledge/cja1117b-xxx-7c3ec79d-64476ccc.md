<!-- page: 1 -->
JIANGSU CHANGJING ELECTRONICS TECHNOLOGY CO., LTD

1A LOW DROPOUT LINEAR REGULATOR

CJA1117B-XXX

SOT-89-3L
FEATURES
- Low Dropout Voltage: 1.15V at 1A Output Current 2 1. ADJ/GND
- Trimmed Current Limit
- On-Chip Thermal Shutdown 2. OUTPUT

- Three-Terminal Adjustable or Fixed 1.8V, 2.5V, 3.3V, 5V 3.IN
- Operation Junction Temperature: -40 ℃ to 125℃

GENERAL DESCRIPTION
The CJA1117B-XXX is a series of low dropout three-terminal regulators with a dropout of 1.15V at

1A output current.

The CJA1117B-XXX series provides current limiting and thermal shutdown. Its circuit includes a

trimmed bandage. reference to assure output voltage accuracy to be within 1.5%. Current limit is

trimmed to ensure specified. output current and controlled short-circuit current. On-chip thermal

shutdown provides protection against any combination of overload and ambient temperature that would

create excessive junction temperature.

The CJA1117B-XXX has an adjustable version, that can provide the output voltage from 1.25V to 5V with

only 2 external resistors.

APPLICATIONS
- PC Motherboard

- LCD Monitor

- Graphic Card

- DVD-Video Player

- NIC/Switch

- Telecom Modem

- ADSL Modem

- Printer and other peripheral Equipment

Marking:

A1117B A1117B = Device code
XXX Lot No. XXX: output voltage

www.jscj-elec.com 1 Rev. - 2.0

<!-- page: 2 -->
ORDERING INFORMATION

Package Operating Junction Temperature Range Part NO.

CJA1117B-ADJ

CJA1117B-1.8
SOT-89-3L -40 to 125℃ CJA1117B-2.5
CJA1117B-3.3

CJA1117B-5.0

ABOSLUTE MAXIMUM RATINGS (Ta=25℃ unless otherwise noted)

Parameter Symbol Value Unit

Input Voltage Vi 20 V
Thermal Resistance from Junction to Ambient RθJA 250 ℃/W
Operating Ambient Temperature TA -40~+85 ℃
Operating Junction Temperature Tj -40~+125 ℃
Storage Temperature Tstg -40~+125 ℃
Soldering Temperature & Time Tsolder 260℃，10s
ESD Voltage (Machine Model) VESD 400 V

Note: Stresses greater than those listed under ”Absolute Maximum Ratings” may cause permanent
damage to the device. These are stress ratings only, and functional of the device at these or any other
conditions beyond those indicated under “Recommended Operating Conditions” is not implied. Exposure
to “Absolute Maximum Ratings” for extended periods may affect device reliability.

RECOMMENDED OPERATING CONDITIONS

Parameter Symbol Value Unit

Input Voltage Vi 15 V
Operating Junction Temperature Tj -40~+125 ℃

www.jscj-elec.com 2 Rev. - 2.0

<!-- page: 3 -->
ELECTRICAL CHARACTERISTICS (VIN≤10V, TJ=25℃ unless otherwise specified. )
Parameter Symbol Part No. Test Conditions Min Typ Max Unit

IOUT=10mA, VIN -VOUT =3.23 1.231 1.250 1.269
Reference Voltage VIROC CJA1117B-ADJ V
10mA≤IOUT≤1A, 2.75V≤VIN -VOUT ≤13.25V 1.225 1.250 1.275

IOUT=10mA, VIN=3.8V 1.773 1.8 1.827
CJA1117B-1.8 V
10mA≤IOUT≤1A, 3.3V≤VIN ≤12V 1.764 1.8 1.836

IOUT=10mA, VIN=4.5V 2.463 2.5 2.538
CJA1117B-2.5 V
10mA≤IOUT≤1A, 4V≤VIN ≤12V 2.450 2.5 2.550
Output Voltage VO
IOUT=10mA, VIN=5.3V 3.251 3.3 3.350
CJA1117B-3.3 V
10mA≤IOUT≤1A, 4.8V≤VIN ≤12V 3.234 3.3 3.366

IOUT=10mA, VIN=7.0V 4.925 5.0 5.075
CJA1117B-5.0 V
10mA≤IOUT≤1A, 6.5V≤VIN ≤12V 4.9 5.0 5.1

CJA1117B-ADJ IOUT=10mA, 1.5V≤VIN -VOUT ≤12V 0.035 0.2 %

CJA1117B-1.8 IOUT=10mA, 1.5V≤VIN -VOUT ≤10.2V 1 7

Line Regulation LNR CJA1117B-2.5 IOUT=10mA, 1.5V≤VIN -VOUT ≤9.5V 1 7
mV
CJA1117B-3.3 IOUT=10mA, 1.5V≤VIN -VOUT ≤8.7V 1 7

CJA1117B-5.0 IOUT=10mA, 1.5V≤VIN -VOUT ≤7V 1 10

CJA1117B-ADJ 0.2 0.4 %

CJA1117B-1.8 7.2

Load Regulation LDR CJA1117B-2.5 VIN-VOUT=1.5V, 10mA≤IOUT ≤1A 10
mV
CJA1117B-3.3 13.2

CJA1117B-5.0 20

Dropout Voltage VD ΔVREF=1%, IOUT=1.0A 1.3 V

Current Limit Ilimit VIN-VOUT=2V 1 A

Adjust Pin Current 60 120 μA

Minimum
IL 1.5V≤VIN -VOUT ≤12V (ADJ only) 1.7 5 μA
Load Current

Quiescent Current Iq VIN= VOUT+1.25V(ADJ except) 5 10 mA

f=120Hz,COUT=22μFTantalum,
Ripple Rejection RR 60 75 dB
VIN-VOUT=3V, IOUT=1A

Temperature Stability 0.5 %

Long-Term Stability TA=125℃ , 1000hrs 0.3 %

RMS Output
TA=25 ℃ , 10Hz≤f ≤10kHz 0.003 %
Noise (% of VOUT)
Thermal
25 ℃
Shutdown Hysteresis

* With package soldering to copper area over backside ground plane or internal power plane RθJA can vary from 46 ℃/W to

>90℃/W depending on mounting technique and the size of the copper area

www.jscj-elec.com 3 Rev. - 2.0

<!-- page: 4 -->
## Visual Summary (Page 4)

- page_class: block_diagram
- confidence: 0.95
- reason_codes: visual_content_dominant, diagram_structure_required, policy_visual_summary_override

Page contains two circuit diagrams: a Functional Block Diagram showing internal architecture with thermal protection and input/output stages, and a Typical Application Circuit illustrating voltage regulation configurations for 12V and 7V inputs using CJA1117B-ADJ and CJA1117B-5.0 regulators.

<!-- page: 5 -->
## Visual Summary (Page 5)

- page_class: chart
- confidence: 0.95
- reason_codes: heuristic_page_type_chart, heuristic_content_type_chart, visual_data_dominance, policy_visual_summary_override

Page contains six electrical characteristic graphs for a device (likely a voltage regulator). Top-left shows Dropout Voltage vs Output Current at three temperatures. Top-right shows Dropout Voltage vs Junction Temperature with multiple current conditions. Middle-left displays Load Regulation deviation over temperature range -50°C to 125°C. Middle-right plots Reference Voltage stability against junction temperature. Bottom-left illustrates Output Voltage drift across the full temperature range. Bottom-right indicates Minimum Load Current requirements versus temperature.

<!-- page: 6 -->
## Visual Summary (Page 6)

- page_class: chart
- confidence: 0.95
- reason_codes: heuristic_chart_detected, visual_data_priority, policy_visual_summary_override

Page contains three performance graphs: Adjust Pin Current vs Junction Temperature (34-48 µA range), Short-circuit Current vs Junction Temperature (~1.4-1.6 A range), and PSRR vs Frequency (70 dB down to ~55 dB at 1 MHz). All charts plot against junction temperature or frequency axes.

<!-- page: 7 -->
SOT-89-3L Package Outline Dimensions

Dimensions In Millimeters Dimensions In Inches
Symbol
Min Max Min Max
A 1.400 1.600 0.055 0.063
b 0.320 0.520 0.013 0.020
b1 0.400 0.580 0.016 0.023
c 0.350 0.440 0.014 0.017
D 4.400 4.600 0.173 0.181
D1 1.550 REF. 0.061 REF.
E 2.300 2.600 0.091 0.102
E1 3.940 4.250 0.155 0.167
e 1.500 TYP. 0.060 TYP.
e1 3.000 TYP. 0.118 TYP.
L 0.900 1.200 0.035 0.047

SOT-89-3L Suggested Pad Layout

NOTICE
JSCJ reserves the right to make modifications,enhancements,improvements,corrections or other
changes without further notice to any product herein. JSCJ does not assume any liability arising
out of the application or use of any product described herein.

www.jscj-elec.com 7 Rev. - 2.0

<!-- page: 8 -->
SOT-89-3L Tape and Reel

SOT-89-3L Embossed Carrier Tape

Packaging Description:

SOT-89-3L parts are shipped in tape. The carrier tape is made from a dissipative (carbon filled) polycarbonate resin. The cover tape is a multilayer film (Heat Activated Adhesive in nature) primarily composed of polyester film, adhesive layer, sealant, and anti-static sprayed agent. These reeled parts in standard option are shipped with 1,000 units per 7" or \(18.0\mathrm{cm}\) diameter reel. The reels are clear in color and is made of polystyrene plastic (anti-static coated).

<table><tr><td colspan="11">Dimensions are in millimeter</td></tr><tr><td>Pkg type</td><td>A</td><td>B</td><td>C</td><td>d</td><td>E</td><td>F</td><td>P0</td><td>P</td><td>P1</td><td>W</td></tr><tr><td>SOT-89-3L</td><td>4.85</td><td>4.45</td><td>1.85</td><td>∅1.50</td><td>1.75</td><td>5.50</td><td>4.00</td><td>8.00</td><td>2.00</td><td>12.00</td></tr></table>

SOT-89-3L Tape Leader and Trailer

SOT-89-3L Reel

<table><tr><td colspan="9">Dimensions are in millimeter</td></tr><tr><td>Reel Option</td><td>D</td><td>D1</td><td>D2</td><td>G</td><td>H</td><td>I</td><td>W1</td><td>W2</td></tr><tr><td>7&quot;Dia</td><td>Ø180.00</td><td>60.00</td><td>R32.00</td><td>R86.50</td><td>R30.00</td><td>Ø13.00</td><td>13.20</td><td>16.50</td></tr></table>

<table><tr><td>REEL</td><td>Reel Size</td><td>Box</td><td>Box Size(mm)</td><td>Carton</td><td>Carton Size(mm)</td><td>G.W.(kg)</td></tr><tr><td>1000 pcs</td><td>7 inch</td><td>10,000 pcs</td><td>203×203×195</td><td>40,000 pcs</td><td>438×438×220</td><td></td></tr></table>

www.jscj-elec.com

8

Rev.-2.0
