<!-- page: 1 -->
SD NAND

FLSD016GFTLT

1

<!-- page: 2 -->
SD NAND

Contents
1. Overview .... 3
1.1 Product Description .... 3
1.2 Features Summary .... 3
2. Pin Assignment .... 4
3. Product List .... 5
4. Current Consumption .... 6
5. Operational Environment .... 6
6. Physical Dimension .... 7
7. Recommended Schematic .... 8

2

<!-- page: 3 -->
SD NAND
1. Overview

1.1 Product Description
SD NAND are highly integrated flash memories. Can be use in the device which can support
SD2.0 standard. It is accessible via a dedicated serial interface optimized for fast and reliable
data transmission. It has been developed to provide an inexpensive, mechanically robust
storage medium in card form for multimedia consumer applications. SD NAND allows the
design of inexpensive players and drivers without moving parts. A low power consumption and a
wide supply voltage range favors mobile, battery-powered application such as audio players,
organizers, palmtops, electronic books, encyclopedia and dictionaries. Using very effective data
compression schemes such as MPEG, the SD card will deliver enough capacity for all kinds of
multimedia data.

1.2 Features Summary

-Capacity: 2GB
-Complies to SD Specification V2.0
-Voltage range for communication: 2.7~3.6V
-Variable clock rate 0-25 MHz (standard), 0-50 MHz (high performance)
-Up to 25 MB/sec data transfer rate (using four parallel data lines)
-password protection (CMD42-LOCK_UNLOCK)
-Sophisticated system for error recovery including a powerful ECC
-Global Wear Leveling
-Power management for low power operation

3

<!-- page: 4 -->
## Visual Summary (Page 4)

- page_class: timing_diagram
- confidence: 0.92
- reason_codes: diagram_present, table_dominant, mixed_layout

Page 4 contains a top-view pin assignment diagram for an SD NAND flash chip and Table 1 detailing pin functions for both SD Mode and SPI Mode interfaces, including signal names, types (I/O/PP, I, S), and descriptions.

<!-- page: 5 -->
SD NAND

3.Product List

<table><tr><td>Part Number</td><td>Capacity</td><td>Actual Size</td><td>Sequential R/W</td><td>Package</td></tr><tr><td>FLSD016GFTLT</td><td>2GB</td><td>1.87GB</td><td>23/8 MB/s</td><td>6.60*8.00 (mm)</td></tr></table>

Table 2: Product List

5

<!-- page: 6 -->
SD NAND

4. Current Consumption
Standby current: 200uA（Maximum value）
Standby current: 150uA（average value）
Operating current: 36mA（Maximum value）
Operating current: 30mA（average value）

5. Operational Environment

Parameter Range
-0℃~70℃ Operating
Temperature
-25℃~85℃ Non-Operating

Operating 25% to 85%, non-condensing
Humidity
Non-Operating 25% to 85%, non-condensing

Table 3: Operational Environment

6

<!-- page: 7 -->
SD NAND

6. Physical Dimension

Figure 2:Top View and Side View

DIMENSION (mm)
Item Symbol
MIN. NOM. MAX
Total height A 0.70 0.75 0.80
Mold thickness A2 0.47 0.50 0.53
SBT thickness A3 0.23 0.25 0.27
Lead width b 0.80 0.85 0.90
Lead width b1 0.35 0.40 0.45
X D 7.90 8.00 8.10
Package size
Y E 6.50 6.60 6.70
Lead pitch e 1.27BSC
Package profile of a surface aaa 0.25
Lead position bbb 0.10
Paralleliam ccc 0.10
Package profile of a surface eee 0.08

7

<!-- page: 8 -->
SD NAND

7. Recommended Schematic

Figure 4: Recommended Schematic
Notice:
1. SCLK should be reserved a position for a 0 ohm resistor.
2. Capacitor C1 should be connected with VCC as closely as possible.
3. We recommend that SDD0, SDD1, SDD2, SDD3, SCLK, CMD should be surrounded by GND.
If not, please make sure the distance between lines is 2 times wider than the line width.
4. The pads in the middle are fixed, please connect GND.

8
