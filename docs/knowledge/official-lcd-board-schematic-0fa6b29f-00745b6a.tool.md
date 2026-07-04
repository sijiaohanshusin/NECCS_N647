## 5 inch LCD dau hter board g

## MB1860

## Table of contents

Sheet 1: Project overview (this page) Sheet 2: MB1860 LCD board

## Legend

General comment such as function title, configuration, ...

Text to be added to silkscreen. Warning text.

Notes to generate the board layout.

## Open Platform License Agreement

The Open Platform License Agreement (“Agreement”) is a binding legal contract between you ("You") and STMicroelectronics International N.V. (“ST”), a company incorporated under the laws of the Netherlands acting for the purpose of this Agreement through its Swiss branch 39, Chemin du Champ des Filles, 1228 Plan-les-Ouates, Geneva, Switzerland.

By using the enclosed reference designs, schematics, PC board layouts, and documentation, in hardcopy or CAD tool file format (collectively, the “Reference Material”), You are agreeing to be bound by the terms and conditions of this Agreement. Do not use the Reference Material until You have read and agreed to this Agreement terms and conditions. The use of the Reference Material automatically implies the acceptance of the Agreement terms and conditions.

The complete Open Platform License Agreement can be found on www.st.com/opla.

U_MB1860 MB1860.SchDot Title: **Project overview** Project: **5 inch LCD daughter board** Variant: RK050HR18C Revision: B -01 Reference: MB1860 Size: A4 Date: 30-AUG-23 Sheet: 1 of 2

**==> picture [42 x 23] intentionally omitted <==**

**==> picture [828 x 547] intentionally omitted <==**

**----- Start of picture text -----**<br>
L101 D101<br>L2<br>BLVDD 600R @ 100MHz VL 4.7uH BL_A BL_K 40 CN2<br>C103 STPS1L40M DN F C105 C4 BL_K BL_A 39 68714014022<br>4.7uF 100nF 4.7uF 38<br>U102 37<br>VDD<br>8 7 R0 36<br>SW OVP<br>3 BLGND C6 R1 35<br>VIN C7<br>BL_EN 5 CTRL LED 1 4.7uF R2 34<br>6 4 100nF R3 33<br>GND FB<br>9 2 R4 32<br>PGND RS<br>R5 31<br>TPS61042DRB R105 R6 30<br>6R2 R7 29<br>L3<br>G0 28<br>600R @ 100MHz G1 27<br>BLGND G2 26<br>G3 25<br>G4 24<br>G5 23<br>Backup Solution L1 D1 G6 22<br>VL DNF D N F BL_A G7 21<br>4.7uH B0 20<br>DN F C1 DN F C2 DN F C3 STPS1L40M DN F C5 BL_K B1 19<br>100nF 10nF 4.7uF 10nF B2 18<br>U2 B3 17<br>8 6 B4 16<br>SW Vo<br>1 5 BLGND B5 15<br>Vi NC<br>BL_EN 7 EN DNF B6 14<br>3 4 VDD B7 13<br>GND FB<br>9 2 R8 12<br>PGND Rset D NF<br>4K7 CLK 11<br>STLD40DPUR R4 R5 R1 DISP 10<br>DNF100K DNF6R2 4K7 HSYNC 9<br>VSYNC 8<br>DE 7<br>BLGND 6<br>5<br>4<br>3<br>2<br>CN1 VDD BLVDD 1<br>1 2 VL VL<br>GND GND<br>R0 3 R0 G0 4 G0 TP1<br>R1 5 6 G1<br>R2R3 79 R1R2/D12R3/D13 D6/G2D7/G3G1 810 G2G3 DNFR104K7 DNFR24K7 VH TP2 BL_A<br>R4 11 12 G4<br>R4/D14 D8/G4<br>R5 13 R5/D15 D9/G5 14 G5 BL_CTRL R3 BL_EN BLGND BLGND<br>R6R7 1715 R6 D10/G6 1618 G6G7 0R R9 TP3<br>19 R7 D11/G7 20 4K7 GND<br>GND GND<br>B0 21 B0 TE/DE 22 DE TP4<br>B1B2B3B4 27292325 B1B2/D0B3/D1 HSYNCVSYNCDISP 24262830 DISPHSYNCVSYNC H1 H2 VDDR6 VDD I2C_SCLI2C_SDA 645 CN368710614022<br>B5 31 B4/D2B5/D3 PCLKGND 32 CLK H3 H4 DNF220K WAKE 3<br>B6 33 34 INT 2<br>B6/D4 GND<br>B7 35 36 WAKE 1<br>B7/D5 RSTN<br>INT 373941 GNDINT SDASCL 384042 I2C_SDAI2C_SCL HW2 I2C address of Touch panel:Read 0xBB / Write 0xBA R7220 K C8<br>RS NOE<br>BL_CTRL 4345 BL_CTRL NWE 4446 LCD HW1 100nF<br>BLVDD 47 BL_5V CS 48<br>BLGND VDD STICKER BOARD<br>49 50<br>BLGND BLGND 3V3 VDD RK050HR18-CTG Sticker board Title: MB1860<br>Header 25x2 Project: 5 inch LCD daughter board<br>Variant: RK050HR18C<br>Revision: B -01 Reference: MB1860<br>Size: A4 Date: 30-AUG-23 Sheet: 2 of 2<br>2P<br>M<br>MP1<br>2P<br>M<br>MP1<br>**----- End of picture text -----**<br>


**==> picture [341 x 70] intentionally omitted <==**

**==> picture [267 x 69] intentionally omitted <==**

**==> picture [234 x 101] intentionally omitted <==**

**==> picture [900 x 416] intentionally omitted <==**

**==> picture [234 x 101] intentionally omitted <==**

**==> picture [87 x 39] intentionally omitted <==**

**==> picture [92 x 79] intentionally omitted <==**

**==> picture [106 x 57] intentionally omitted <==**

**==> picture [78 x 70] intentionally omitted <==**

**==> picture [59 x 88] intentionally omitted <==**

**==> picture [89 x 64] intentionally omitted <==**

**==> picture [71 x 60] intentionally omitted <==**

**==> picture [70 x 70] intentionally omitted <==**

**==> picture [237 x 85] intentionally omitted <==**

**==> picture [61 x 15] intentionally omitted <==**

**==> picture [639 x 197] intentionally omitted <==**

**==> picture [325 x 205] intentionally omitted <==**
