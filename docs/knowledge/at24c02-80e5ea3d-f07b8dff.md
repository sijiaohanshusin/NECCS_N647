<!-- page: 1 -->
Features
- Low-voltage and Standard-voltage Operation
– 1.8 (VCC = 1.8V to 5.5V)
- Internally Organized 256 x 8 (2K)
- Two-wire Serial Interface
- Schmitt Trigger, Filtered Inputs for Noise Suppression
- Bidirectional Data Transfer Protocol
- 1 MHz (5V), 400 kHz (1.8V, 2.5V, 2.7V) Compatibility
- Write Protect Pin for Hardware Data Protection
- 8-byte Page (2K) Write Modes Two-wire
- Partial Page Writes Allowed
- Self-timed Write Cycle (5 ms max) Serial EEPROM
- High-reliability
2K (256 x 8)
– Endurance: 1 Million Write Cycles
– Data Retention: 100 Years
- 8-lead PDIP, 8-lead JEDEC SOIC, 8-lead Ultra Thin Mini-MAP (MLP 2x3), 5-lead SOT23,
8-lead TSSOP and 8-ball dBGA2 Packages
- Lead-free/Halogen-free
- Available in Automotive AT24C02B
- Die Sales: Wafer Form and Tape and Reel

Description Not
The AT24C02B provides 2048 bits of serial electrically erasable and programmable Recommendedread-only memory (EEPROM) organized as 256 words of 8 bits each. The device is
optimized for use in many industrial and commercial applications where low-power for New Design
and low-voltage operation are essential. The AT24C02B is available in space-saving
8-lead PDIP, 8-lead JEDEC SOIC, 8-lead Ultra Thin Mini-MAP (MLP 2x3), 5-lead
SOT23, 8-lead TSSOP, and 8-ball dBGA2 packages and is accessed via a Two-wire
serial interface. In addition, the AT24C02B is available in 1.8V (1.8V to 5.5V) version.

Table 0-1. Pin Configuration 8-lead Ultra-Thin
Pin Name Function Mini-MAP (MLP 2x3) 8-ball dBGA2
VCC 8 1 A0 VCC 8 1 A0
A0 - A2 Address Inputs
WP 7 2 A1 WP 7 2 A1
SDA Serial Data SCL 6 3 A2 SCL 6 3 A2
SCL Serial Clock Input SDA 5 4 GND SDA 5 4 GND
Bottom View Bottom View
WP Write Protect
GND Ground 8-lead TSSOP 8-lead SOIC
A0 1 8 VCC A0 1 8 VCC
VCC Power Supply
A1 2 7 WP A1 2 7 WP
A2 3 6 SCL A2 3 6 SCL
GND 4 5 SDA GND 4 5 SDA
Note: For use of 5-lead SOT23, the
software A2, A1, and A0 bits in
5-lead SOT23 8-lead PDIP
the device address word
must be set to zero to prop- SCL 1 5 WP A0 1 8 VCC
A1 2 7 WP erly communicate. GND 2
A2 3 6 SCL
SDA 3 4 VCC GND 4 5 SDA 5126H–SEEPR–8/07

<!-- page: 2 -->
## Visual Summary (Page 2)

- page_class: timing_diagram
- confidence: 0.92
- reason_codes: diagram_heuristic, timing_keywords_detected, block_diagram_structure

Page 2 of AT24C02B datasheet contains Absolute Maximum Ratings table, a block diagram (Figure 0-1) showing EEPROM architecture with Serial Control Logic, Data Recovery, and Pin Descriptions for SCL/SDA interfaces.

<!-- page: 3 -->
AT24C02B

(GND). When the write protect pin is connected to VCC, the write protection feature is enabled
and operates as shown in Table 1-1.

Table 1-1. Write Protect

Part of the Array Protected
WP Pin
Status
24C02B

At VCC Full (2K) Array

At GND Normal Read/Write Operations

2. Memory Organization

AT24C02B, 2K SERIAL EEPROM: Internally organized with 32 pages of 8 bytes each, the 2K
requires an 8-bit data word address for random word addressing.

3

5126H–SEEPR–8/07

<!-- page: 4 -->
## Visual Summary (Page 4)

- page_class: timing_diagram
- confidence: 0.95
- reason_codes: prose_readable, table_extractable, text_only_fallback

Table 2-1. Pin Capacitance(1) Applicable over recommended operating range from TA = 25C, f = 1.0 MHz, VCC = +1.8V Symbol Test Condition Max Units Conditions CI/O Input/Output Capacitance (SDA) 8 pF VI/O = 0V CIN Input Capacitance (A0, A1, A2, SCL) 6 pF VIN = 0V Note: 1. This parameter is characterized and is not 100% tested. Table 2-2. DC Characteristics Applicable over recommended operating range from: TAI = –40C to +85C, VCC = +1.8V to +5.5V, VCC = +1.8V to +5.5V (unless otherwise noted) Symbol Parameter Test Condition Min Typ Max Units

<!-- page: 5 -->
AT24C02B

Table 2-3. AC Characteristics
Applicable over recommended operating range from TAI = –40C to +85C, VCC = +1.8V to +5.5V, CL = 1 TTL Gate and
100 pF (unless otherwise noted)

1.8, 2.5, 2.7 5.0-volt

Symbol Parameter Min Max Min Max Units

tBUF Time the bus must be free before a new transmission can start 1.2 0.5 µs

tHD.STA Start Hold Time 0.6 0.25 µs

tSU.STA Start Setup Time 0.6 0.25 µs

tHD.DAT Data In Hold Time 0 0 µs

tSU.DAT Data In Setup Time 100 100 ns
tR Inputs Rise Time(1) 0.3 0.3 µs
tF Inputs Fall Time(1) 300 100 ns

tSU.STO Stop Setup Time 0.6 .25 µs

tDH Data Out Hold Time 50 50 ns

tWR Write Cycle Time 5 5 ms

Write Endurance(1) 5.0V, 25C, Byte Mode 1 Million
Cycles

Note: 1. This parameter is ensured by characterization only.

5

5126H–SEEPR–8/07

<!-- page: 6 -->
3. Device Operation

CLOCK and DATA TRANSITIONS: The SDA pin is normally pulled high with an external device.
Data on the SDA pin may change only during SCL low time periods (see Figure 5-2 on page 8).
Data changes during SCL high periods will indicate a start or stop condition as defined below.

START CONDITION: A high-to-low transition of SDA with SCL high is a start condition which
must precede any other command (see Figure 5-3 on page 8).

STOP CONDITION: A low-to-high transition of SDA with SCL high is a stop condition. After a
read sequence, the stop command will place the EEPROM in a standby power mode (see Fig-
ure 5-3 on page 8).

ACKNOWLEDGE: All addresses and data words are serially transmitted to and from the
EEPROM in 8-bit words. The EEPROM sends a zero to acknowledge that it has received each
word. This happens during the ninth clock cycle.

STANDBY MODE: The AT24C02B features a low-power standby mode which is enabled: (a)
upon power-up and (b) after the receipt of the STOP bit and the completion of any internal
operations.

2-Wire Software Reset: After an interruption in protocol, power loss or system reset, any 2-wire
part can be reset by following these steps: (a) Create a start bit condition, (b) clock 9 cycles, (c)
create another start bit followed by stop bit condition as shown below. The device is ready for
next communication after above steps have been completed.

Figure 3-1. Software reset
Start bit Dummy Clock Cycles Start bit Stop b

SCL 1 2 3 8 9

SDA

6 AT24C02B
5126H–SEEPR–8/07

<!-- page: 7 -->
## Visual Summary (Page 7)

- page_class: timing_diagram
- confidence: 0.95
- reason_codes: timing_diagram, diagnostic_visuals

Page 7 of the AT24C02B datasheet contains two timing diagrams: Figure 4-1 (Bus Timing) and Figure 5-1 (Write Cycle Timing). Both diagrams illustrate I2C protocol signal waveforms for SCL (Serial Clock) and SDA (Serial Data), including start/stop conditions, data transitions, and write cycle parameters.

<!-- page: 8 -->
Figure 5-2. Data Validity

SDA

SCL

DATA STABLE DATA STABLE

DATA
CHANGE

Figure 5-3. Start and Stop Definition

SDA

SCL

START STOP

Figure 5-4. Output Acknowledge

SCL 1 8 9

DATA IN

DATA OUT

START ACKNOWLEDGE

8 AT24C02B
5126H–SEEPR–8/07

<!-- page: 9 -->
AT24C02B

6. Device Addressing

The 2K EEPROM device requires an 8-bit device address word following a start condition to
enable the chip for a read or write operation (refer to Figure 8-1).

The device address word consists of a mandatory one, zero sequence for the first four most sig-
nificant bits as shown. This is common to all the EEPROM devices.

The next 3 bits are the A2, A1 and A0 device address bits for the 2K EEPROM. These 3 bits
must compare to their corresponding hard-wired input pins.

The eighth bit of the device address is the read/write operation select bit. A read operation is ini-
tiated if this bit is high and a write operation is initiated if this bit is low.

Upon a compare of the device address, the EEPROM will output a zero. If a compare is not
made, the chip will return to a standby state.

7. Write Operations

BYTE WRITE: A write operation requires an 8-bit data word address following the device
address word and acknowledgment. Upon receipt of this address, the EEPROM will again
respond with a zero and then clock in the first 8-bit data word. Following receipt of the 8-bit data
word, the EEPROM will output a zero and the addressing device, such as a microcontroller,
must terminate the write sequence with a stop condition. At this time the EEPROM enters an
internally timed write cycle, tWR, to the nonvolatile memory. All inputs are disabled during this
write cycle and the EEPROM will not respond until the write is complete (see Figure 8-2 on page
11).

PAGE WRITE: The 2K EEPROM is capable of an 8-byte page write.

A page write is initiated the same as a byte write, but the microcontroller does not send a stop
condition after the first data word is clocked in. Instead, after the EEPROM acknowledges
receipt of the first data word, the microcontroller can transmit up to seven data words. The
EEPROM will respond with a zero after each data word received. The microcontroller must ter-
minate the page write sequence with a stop condition (see Figure 8-3 on page 11).

The data word address lower three bits are internally incremented following the receipt of each
data word. The higher data word address bits are not incremented, retaining the memory page
row location. When the word address, internally generated, reaches the page boundary, the fol-
lowing byte is placed at the beginning of the same page. If more than eight data words are
transmitted to the EEPROM, the data word address will “roll over” and previous data will be
overwritten.

ACKNOWLEDGE POLLING: Once the internally timed write cycle has started and the
EEPROM inputs are disabled, acknowledge polling can be initiated. This involves sending a
start condition followed by the device address word. The read/write bit is representative of the
operation desired. Only if the internal write cycle has completed will the EEPROM respond with
a zero allowing the read or write sequence to continue.

9

5126H–SEEPR–8/07

<!-- page: 10 -->
8. Read Operations

Read operations are initiated the same way as write operations with the exception that the
read/write select bit in the device address word is set to one. There are three read operations:
current address read, random address read and sequential read.

CURRENT ADDRESS READ: The internal data word address counter maintains the last
address accessed during the last read or write operation, incremented by one. This address
stays valid between operations as long as the chip power is maintained. The address “roll over”
during read is from the last byte of the last memory page to the first byte of the first page. The
address “roll over” during write is from the last byte of the current page to the first byte of the
same page.

Once the device address with the read/write select bit set to one is clocked in and acknowledged
by the EEPROM, the current address data word is serially clocked out. The microcontroller does
not respond with an input zero but does generate a following stop condition (see Figure 8-4 on
page 11).

RANDOM READ: A random read requires a “dummy” byte write sequence to load in the data
word address. Once the device address word and data word address are clocked in and
acknowledged by the EEPROM, the microcontroller must generate another start condition. The
microcontroller now initiates a current address read by sending a device address with the
read/write select bit high. The EEPROM acknowledges the device address and serially clocks
out the data word. The microcontroller does not respond with a zero but does generate a follow-
ing stop condition (see Figure 8-5 on page 12).

SEQUENTIAL READ: Sequential reads are initiated by either a current address read or a ran-
dom address read. After the microcontroller receives a data word, it responds with an
acknowledge. As long as the EEPROM receives an acknowledge, it will continue to increment
the data word address and serially clock out sequential data words. When the memory address
limit is reached, the data word address will “roll over” and the sequential read will continue. The
sequential read operation is terminated when the microcontroller does not respond with a zero
but does generate a following stop condition (see Figure 8-6 on page 12).

Figure 8-1. Device Address

MSB LSB

10 AT24C02B
5126H–SEEPR–8/07

<!-- page: 11 -->
AT24C02B

Figure 8-2. Byte Write

Figure 8-3. Page Write

Figure 8-4. Current Address Read

5126H-SEEPR-8/07

AIMEL®

11

<!-- page: 12 -->
AMEL®

Figure 8-5. Random Read

Figure 8-6. Sequential Read

12

AT24C02B

5126H-SEEPR-8/07

<!-- page: 13 -->
AT24C02B

AT24C02B Ordering Information

Ordering Code Voltage Package Operation Range

AT24C02B-PU (Bulk form only) 1.8 8P3
AT24C02BN-SH-B(1) (NiPdAu Lead Finish) 1.8 8S1
AT24C02BN-SH-T(2) (NiPdAu Lead Finish) 1.8 8S1
AT24C02B-TH-B(1) (NiPdAu Lead Finish) 1.8 8A2 Lead-free/Halogen-free/
Industrial Temperature
AT24C02B-TH-T(2) (NiPdAu Lead Finish) 1.8 8A2 (–40C to 85C)
AT24C02BY6-YH-T(2) (NiPdAu Lead Finish) 1.8 8Y6
AT24C02B-TSU-T(2) 1.8 5TS1
AT24C02BU3-UU-T(2) 1.8 8U3-1

Industrial Temperature
AT24C02B-W-11(3) 1.8 Die Sale
(–40C to 85C)

Notes: 1. “-B” denotes bulk.

2. “-T” denotes tape and reel. SOIC = 4K per reel. TSSOP, Ultra Thin Mini-MAP, SOT23, and dBGA2 = 5K per reel.

3. Available in tape and reel and wafer form; order as SL788 for inkless wafer form. Please contact Serial Interface Marketing.

Package Type

8P3 8-lead, 0.300" Wide, Plastic Dual Inline Package (PDIP)

8S1 8-lead, 0.150" Wide, Plastic Gull Wing Small Outline (JEDEC SOIC)

8A2 8-lead, 4.4 mm Body, Plastic Thin Shrink Small Outline Package (TSSOP)

8Y6 8-lead, 2.00 mm x 3.00 mm Body, 0.50 mm Pitch, Ultra Thin Mini-MAP, Dual No Lead Package (DFN), (MLP 2x3 mm)

5TS1 5-lead, 2.90 mm x 1.60 mm Body, Plastic Thin Shrink Small Outline Package (SOT23)

8U3-1 8-ball, die Ball Grid Array Package (dBGA2)

Options

–1.8 Low-voltage (1.8V to 5.5V)

13

5126H–SEEPR–8/07

<!-- page: 14 -->
9. Part Marking Scheme

8-PDIP

Seal Year

TOP MARK | Seal Week
| | |
|---|---|---|---|---|---|---|---|
A T M L U Y W W
|---|---|---|---|---|---|---|---|
0 2 B 1
|---|---|---|---|---|---|---|---|
* Lot Number
|---|---|---|---|---|---|---|---|
|
Pin 1 Indicator (Dot)
U = Material Set

Y = Seal Year

WW = Seal Week

02B = Device

1 = Voltage Indicator

*Lot Number to Use ALL Characters in Marking

BOTTOM MARK
No Bottom Mark

14 AT24C02B
5126H–SEEPR–8/07

<!-- page: 15 -->
AT24C02B

8-SOIC
Seal Year

TOP MARK | Seal Week
| | |
|---|---|---|---|---|---|---|---|
A T M L H Y W W
|---|---|---|---|---|---|---|---|
0 2 B 1
|---|---|---|---|---|---|---|---|
* Lot Number
|---|---|---|---|---|---|---|---|
|
Pin 1 Indicator (Dot)
H = Material Set

Y = Seal Year

WW = Seal Week

02B = Device

1 = Voltage Indicator

*Lot Number to Use ALL Characters in Marking

BOTTOM MARK
No Bottom Mark

15

5126H–SEEPR–8/07

<!-- page: 16 -->
8-TSSOP

TOP MARK

Pin 1 Indicator (Dot)
|
|---|---|---|---|
* H Y W W
|---|---|---|---|---|
0 2 B 1
|---|---|---|---|---|

H = Material Set

Y = Seal Year

WW = Seal Week

02B = Device

V = Voltage Indicator

BOTTOM MARK
|---|---|---|---|---|---|---|
X X
|---|---|---|---|---|---|---|
A A A A A A A
|---|---|---|---|---|---|---|
<- Pin 1 Indicator
Lot Number

XX = Country of Origin

AAAAAA = Lot Number

16 AT24C02B
5126H–SEEPR–8/07

<!-- page: 17 -->
AT24C02B

SOT23
TOP MARK
|---|---|---|---|---|
Line 1 -----------> 2 B 1 W U
|---|---|---|---|---|
*
|

2B = Device

1 = Voltage Indicator

W = Write Protect Feature

U = Material Set

Pin 1 Indicator (Dot)

BOTTOM MARK

|---|---|---|---|
Y M T C
|---|---|---|---|

Y = One Digit Year Code

M = Seal Month

TC = Trace Code

ULTRA THIN MINI MAP
TOP MARK
|---|---|---|
0 2 B
|---|---|---|
H 1
|---|---|---|
Y T C
|---|---|---|
*
|
02B = Device

H = Material Set

1 = Voltage Indicator

Y = Year of Assembly

TC = Trace Code

Pin 1 Indicator (Dot)

17

5126H–SEEPR–8/07

<!-- page: 18 -->
dBGA2
TOP MARK

LINE 1-------> 02BU
LINE 2-------> YMTC
|<-- Pin 1 This Corner
02B = Device

U = Material Set

Y = One Digit Year Code

M = Seal Month

TC = Trace Code

18 AT24C02B
5126H–SEEPR–8/07

<!-- page: 19 -->
AT24C02B

10. Packaging Information

8P3 – PDIP

E 1

E1

N

Top View c
eA

End View

COMMON DIMENSIONS
D (Unit of Measure = inches)
e
D1 A2 A SYMBOL MIN NOM MAX NOTE
A – – 0.210 2

A2 0.115 0.130 0.195

b 0.014 0.018 0.022 5

b2 0.045 0.060 0.070 6

b3 0.030 0.039 0.045 6

c 0.008 0.010 0.014

D 0.355 0.365 0.400 3
b2 L D1 0.005 – – 3
b3 E 0.300 0.310 0.325 4
4 PLCS b
E1 0.240 0.250 0.280 3
Side View e 0.100 BSC
eA 0.300 BSC 4

L 0.115 0.130 0.150 2

Notes: 1. This drawing is for general information only; refer to JEDEC Drawing MS-001, Variation BA, for additional information.
2. Dimensions A and L are measured with the package seated in JEDEC seating plane Gauge GS-3.
3. D, D1 and E1 dimensions do not include mold Flash or protrusions. Mold Flash or protrusions shall not exceed 0.010 inch.
4. E and eA measured with the leads constrained to be perpendicular to datum.
5. Pointed or rounded lead tips are preferred to ease insertion.
6. b2 and b3 maximum dimensions do not include Dambar protrusions. Dambar protrusions shall not exceed 0.010 (0.25 mm).
01/09/02

TITLE DRAWING NO. REV.
2325 Orchard Parkway 8P3, 8-lead, 0.300" Wide Body, Plastic Dual
8P3 B
R San Jose, CA 95131 In-line Package (PDIP)

19

5126H–SEEPR–8/07

<!-- page: 20 -->
8S1 – JEDEC SOIC

C

1

E E1

L N

∅
Top View
End View

e B
A COMMON DIMENSIONS
(Unit of Measure = mm)

SYMBOL MIN NOM MAX NOTE
A1 A 1.35 – 1.75

A1 0.10 – 0.25

b 0.31 – 0.51

C 0.17 – 0.25

D 4.80 – 5.00
D
E1 3.81 – 3.99

E 5.79 – 6.20
Side View e 1.27 BSC

L 0.40 – 1.27

∅ 0˚ – 8˚

Note: These drawings are for general information only. Refer to JEDEC Drawing MS-012, Variation AA for proper dimensions, tolerances, datums, etc.

10/7/03

TITLE DRAWING NO. REV.
1150 E. Cheyenne Mtn. Blvd. 8S1, 8-lead (0.150" Wide Body), Plastic Gull Wing 8S1 B
R Colorado Springs, CO 80906 Small Outline (JEDEC SOIC)

20 AT24C02B
5126H–SEEPR–8/07

<!-- page: 21 -->
AT24C02B

8A2 – TSSOP

3 2 1

Pin 1 indicator
this corner

E1 E

L1

N
L
Top View End View

COMMON DIMENSIONS
(Unit of Measure = mm)

SYMBOL MIN NOM MAX NOTE

D 2.90 3.00 3.10 2, 5 A
b E 6.40 BSC

E1 4.30 4.40 4.50 3, 5

A – – 1.20

e A2 A2 0.80 1.00 1.05

b 0.19 – 0.30 4
D
e 0.65 BSC

Side View L 0.45 0.60 0.75

L1 1.00 REF

Notes: 1. This drawing is for general information only. Refer to JEDEC Drawing MO-153, Variation AA, for proper dimensions, tolerances,
datums, etc.
2. Dimension D does not include mold Flash, protrusions or gate burrs. Mold Flash, protrusions and gate burrs shall not exceed
0.15 mm (0.006 in) per side.
3. Dimension E1 does not include inter-lead Flash or protrusions. Inter-lead Flash and protrusions shall not exceed 0.25 mm
(0.010 in) per side.
4. Dimension b does not include Dambar protrusion. Allowable Dambar protrusion shall be 0.08 mm total in excess of the
b dimension at maximum material condition. Dambar cannot be located on the lower radius of the foot. Minimum space between
protrusion and ad acent lead is 0.07 mm.
5. Dimension D and E1 to be determined at Datum Plane H. 5/30/02

TITLE DRAWING NO. REV.
2325 Orchard Parkway 8A2, 8-lead, 4.4 mm Body, Plastic
8A2 B
R San Jose, CA 95131 Thin Shrink Small Outline Package (TSSOP)

21

5126H–SEEPR–8/07

<!-- page: 22 -->
8Y6 - Mini Map

A D2D2 b
(8X)(8X)

PinPin 11
IndexIndex
AreaArea

E E2E2

PinPin 11 IDID

LL (8X)(8X)

D
ee (6X)(6X)
A2A2 A1A1

1.501.50 REF.REF.

A3A3 COMMON DIMENSIONS
(Unit of Measure = mm)

SYMBOL MIN NOM MAX NOTE

D 2.00 BSC

E 3.00 BSC

D2 1.40 1.50 1.60

E2 - - 1.40

A - - 0.60

A1 0.0 0.02 0.05

A2 - - 0.55

A3 0.20 REF

L 0.20 0.30 0.40

e 0.50 BSC

b 0.20 0.25 0.30 2

Notes: 1. This drawing is for general information only. Refer to JEDEC Drawing MO-229, for proper dimensions,
tolerances, datums, etc.
2. Dimension b applies to metallized terminal and is measured between 0.15 mm and 0.30 mm from the terminal tip. If the
terminal has the optional radius on the other end of the terminal, the dimension should not be measured in that radius area.
3. Soldering the large thermal pad is optional, but not recommended. No electrical connection is accomplished to the
device through this pad, so if soldered it should be tied to ground
10/16/07

TITLE DRAWING NO. REV.
2325 Orchard Parkway
8Y6, 8-lead 2.0 x 3.0 mm Body, 0.50 mm Pitch, Utlra Thin Mini-Map,
R San Jose, CA 95131 8Y6 D Dual No Lead Package (DFN) ,(MLP 2x3)

22 AT24C02B
5126H–SEEPR–8/07

<!-- page: 23 -->
AT24C02B

5TS1 – SOT23

e1

5 4 C

E1 E C L

L1

1 2 3
Top View End View

b

A2 A

Seating
Plane
e A1

D

Side View COMMON DIMENSIONS
(Unit of Measure = mm)

NOTES: 1. This drawing is for general information only. Refer to JEDEC Drawing SYMBOL MIN NOM MAX NOTE
MO-193, Variation AB, for additional information. A – – 1.10
2. Dimension D does not include mold flash, protrusions, or gate burrs.
Mold flash, protrusions, or gate burrs shall not exceed 0.15 mm per end. A1 0.00 – 0.10
Dimension E1 does not include interlead flash or protrusion. Interlead A2 0.70 0.90 1.00
flash or protrusion shall not exceed 0.15 mm per side.
3. The package top may be smaller than the package bottom. Dimensions c 0.08 – 0.20 4
D and E1 are determined at the outermost extremes of the plastic body D 2.90 BSC 2, 3
exclusive of mold flash, tie bar burrs, gate burrs, and interlead flash, but
including any mismatch between the top and bottom of the plastic body. E 2.80 BSC 2, 3
4. These dimensions apply to the flat section of the lead between 0.08 mm E1 1.60 BSC 2, 3
and 0.15 mm from the lead tip.
5. Dimension "b" does not include Dambar protrusion. Allowable Dambar L1 0.60 REF
protrusion shall be 0.08 mm total in excess of the "b" dimension at e 0.95 BSC
maximum material condition. The Dambar cannot be located on the lower
radius of the foot. Minimum space between protrusion and an adjacent lead e1 1.90 BSC
shall not be less than 0.07 mm. b 0.30 – 0.50 4, 5

6/25/03
TITLE DRAWING NO. REV.
1150 E. Cheyenne Mtn. Blvd. 5TS1, 5-lead, 1.60 mm Body, Plastic Thin Shrink
R Colorado Springs, CO 80906 Small Outline Package (SHRINK SOT) PO5TS1 A

23

5126H–SEEPR–8/07

<!-- page: 24 -->
8U3-1 – dBGA2

E

D
1. b

A1
PIN 1 BALL PAD CORNER
A2
Top View
A

PIN 1 BALL PAD CORNER Side View

1 2 3 4
(d1)

d

8 7 6 5

e

COMMON DIMENSIONS
(Unit of Measure = mm)
(e1)
SYMBOL MIN NOM MAX NOTE
Bottom View A 0.71 0.81 0.91
8 SOLDER BALLS
A1 0.10 0.15 0.20

A2 0.40 0.45 0.50

b 0.20 0.25 0.30

D 1.50 BSC
1. Dimension “b” is measured at the maximum solder ball diameter.
E 2.00 BSC

This drawing is for general information only. e 0.50 BSC

e1 0.25 REF

d 1.00 BSC

d1 0.25 REF

6/24/03
TITLE DRAWING NO. REV.
1150 E. Cheyenne Mtn. Blvd. 8U3-1, 8-ball, 1.50 x 2.00 mm Body, 0.50 mm pitch,
R Colorado Springs, CO 80906 Small Die Ball Grid Array Package (dBGA2) PO8U3-1 A

24 AT24C02B
5126H–SEEPR–8/07

<!-- page: 25 -->
AT24C02B

11. Revision History

Doc. Rev. Date Comments

Updated to new template
5126H 8/2007 Updated common graphics
Added Part Makring Scheme

Removed reference to Waffle Pack on page 1 and Page 13
Added lines to Ordering Code table
Removed NC row in the table
Added note on Pg 1
5126G 4/2007
Corrected format on table 5
Removed Memory Reset section
Added 2-Wire software reset section and figure
Corrected Figures 7-11

Corrected dBGA2 package code on Pg 13
5126F 2/2007
Removed ‘Preliminary’

Added Ultra-Thin on Pg 1
5126E 2/2007
Modified Ordering Information table

Implemented Revision History
5126D 7/2006 Added Preliminary status; Added ‘Available in Automotive’ to
Features

25

5126H–SEEPR–8/07

<!-- page: 26 -->
Headquarters International

Atmel Corporation Atmel Asia Atmel Europe Atmel Japan
2325 Orchard Parkway Room 1219 Le Krebs 9F, Tonetsu Shinkawa Bldg.
San Jose, CA 95131 Chinachem Golden Plaza 8, Rue Jean-Pierre Timbaud 1-24-8 Shinkawa
USA 77 Mody Road Tsimshatsui BP 309 Chuo-ku, Tokyo 104-0033
Tel: 1(408) 441-0311 East Kowloon 78054 Saint-Quentin-en- Japan
Fax: 1(408) 487-2600 Hong Kong Yvelines Cedex Tel: (81) 3-3523-3551
Tel: (852) 2721-9778 France Fax: (81) 3-3523-7581
Fax: (852) 2722-1369 Tel: (33) 1-30-60-70-00
Fax: (33) 1-30-60-71-11

Product Contact

Web Site Technical Support Sales Contact
www.atmel.com s_eeprom@atmel.com www.atmel.com/contacts

Literature Requests
www.atmel.com/literature

Disclaimer: The information in this document is provided in connection with Atmel products. No license, express or implied, by estoppel or otherwise, to any
intellectual property right is granted by this document or in connection with the sale of Atmel products. EXCEPT AS SET FORTH IN ATMEL’S TERMS AND CONDI-
TIONS OF SALE LOCATED ON ATMEL’S WEB SITE, ATMEL ASSUMES NO LIABILITY WHATSOEVER AND DISCLAIMS ANY EXPRESS, IMPLIED OR STATUTORY
WARRANTY RELATING TO ITS PRODUCTS INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE, OR NON-INFRINGEMENT. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT, CONSEQUENTIAL, PUNITIVE, SPECIAL OR INCIDEN-
TAL DAMAGES (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS, BUSINESS INTERRUPTION, OR LOSS OF INFORMATION) ARISING OUT OF
THE USE OR INABILITY TO USE THIS DOCUMENT, EVEN IF ATMEL HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. Atmel makes no
representations or warranties with respect to the accuracy or completeness of the contents of this document and reserves the right to make changes to specifications
and product descriptions at any time without notice. Atmel does not make any commitment to update the information contained herein. Unless specifically provided
otherwise, Atmel products are not suitable for, and shall not be used in, automotive applications. Atmel’s products are not intended, authorized, or warranted for use
as components in applications intended to support or sustain life.

© 2007 Atmel Corporation. All rights reserved. Atmel®, logo and combinations thereof, and others, are registered trademarks or trademarks of
Atmel Corporation or its subsidiaries. Other terms and product names may be trademarks of others.

5126H–SEEPR–8/07
