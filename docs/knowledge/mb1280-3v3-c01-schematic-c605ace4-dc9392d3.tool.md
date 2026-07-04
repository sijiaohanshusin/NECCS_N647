## STMod+ Fanout

## MB1280

## Table of contents

Sheet 1: Project overview (this page) Sheet 2: MB1280

## Legend

General comment such as function title, configuration, ...

Text to be added to silkscreen. Warning text.

Notes to generate the board layout.

## Open Platform License Agreement

The Open Platform License Agreement (“Agreement”) is a binding legal contract between you ("You") and STMicroelectronics International N.V. (“ST”), a company incorporated under the laws of the Netherlands acting for the purpose of this Agreement through its Swiss branch 39, Chemin du Champ des Filles, 1228 Plan-les-Ouates, Geneva, Switzerland.

By using the enclosed reference designs, schematics, PC board layouts, and documentation, in hardcopy or CAD tool file format (collectively, the “Reference Material”), You are agreeing to be bound by the terms and conditions of this Agreement. Do not use the Reference Material until You have read and agreed to this Agreement terms and conditions. The use of the Reference Material automatically implies the acceptance of the Agreement terms and conditions.

The complete Open Platform License Agreement can be found on www.st.com/opla.

U_MB1280 MB1280.SchDoc Title: **Project overview** Project: **STMod+ Fanout** Variant: 3V3 Revision: C -01 Reference: MB1280 Size: A4 Date: 18-SEP-2019 Sheet: 1 of 2

**==> picture [42 x 23] intentionally omitted <==**

## STMod+

**==> picture [814 x 489] intentionally omitted <==**

**----- Start of picture text -----**<br>
5V Grove connector<br>3V3 VCC<br>CN1 BSN20BK VCC<br>STMOD#1-NSS/CTS STMOD#1 STMOD#11 STMOD#11-INT R1 R2<br>STMOD#2-MOSIp/TX STMOD#2 12 1211 STMOD#12 STMOD#12-RST G 4K7 4K7<br>STMOD#3-MISOp/RX STMOD#3 3 13 STMOD#13 STMOD#13-ADC CN3<br>STMOD#4-SCK/RTS STMOD#4 45 1415 STMOD#14 STMOD#14-PWM STMOD#7BSN20BK 2 S D 3 I2C_SCL I2C_SCLI2C_SDA 12 SCLSDA<br>6 16 3<br>STMOD#7-SCL STMOD#7 7 17 STMOD#17 STMOD#17-DF-D3 G T2 4<br>STMOD#8-MOSIs STMOD#8 STMOD#18 STMOD#18-DF-CK3<br>8 18<br>STMOD#9-MISOs STMOD#9 9 19 STMOD#19 STMOD#19-DF-D7 STMOD#10 2 3 I2C_SDA Header 4x1<br>STMOD#10-SDA STMOD#10 STMOD#20 STMOD#20-DF-CK7 S D<br>10 20<br>Header 10x2 STMod+ STMOD#7 T1Solder Bridge ON I2C_SCL support Grove- Barometer sensor (BMP180)<br>STMOD#10 Solder Bridge ONSB2 I2C_SDA support Grove- LCD RGB Backlight<br>SB1 VCC<br>3V3<br>CN12 Bread board connectors CN9 CN5 ESP-01 connector STMOD#3 CN2 RX<br>1 1<br>STMOD#13 STMOD#14 STMOD#2 TX<br>1 1 2 2<br>STMOD#12 STMOD#11<br>2 2 3<br>STMOD#1 STMOD#3 header 2x1<br>3 3 4<br>STMOD#4 STMOD#2 CN4<br>4 4<br>STMOD#9 STMOD#7 STMOD#3 RXD Header 4x1<br>5 5 1 2<br>STMOD#8 6 STMOD#10 6 GPIO2 STMOD#14 3 4 STMOD#13 CH_PD<br>5V 7 5V 7 Only PAD for Power GPIO0 STMOD#11 5 6 STMOD#12 RST<br>TXD STMOD#2 VCC<br>8 8 7 8 3V3<br>STMOD#17 9 STMOD#19 9 5V support Grove- NFC<br>STMOD#18 STMOD#20 Socket 4x2<br>10 10<br>CN6<br>Header 10x1 Header 10x1<br>1<br>Only PAD for bread board connection 2<br>header 2x1<br>5V U1 3V3<br>1 5<br>Vin Vout<br>3 VCC<br>EN<br>Mikrobus connectors 3V3 C2 2 4 C3 5V JP1<br>CN7 1uF GND BYPASS 1uF 1<br>CN11 CN10 LDK120M33R 2<br>STMOD#13 1 AN PWM STMOD#14 1 1 C1 C5 C4 3<br>STMOD#12 2 1 RST INT STMOD#11 2 1 2 100nF 10nF 100nF 3V3<br>2 2<br>STMOD#1 3 CS RX STMOD#3 3 header 2x1 Header 3x1<br>3 3<br>STMOD#4 4 SCK TX STMOD#2 4<br>4 4<br>STMOD#9 5 5 MISO SCL STMOD#7 5 5 closed pin2 and pin3 for default<br>STMOD#8 6 MOSI SDA STMOD#10 6<br>6 6<br>3V3 7 7 +3.3V +5V 5V 7 7 Only PAD for Power<br>8 GND GND 8<br>8 8<br>5V TP1 HW Mechanical parts<br>Socket 8x1 Socket 8x1<br>CN8 TP HW1 HW2<br>Sticker board<br>1<br>PCB<br>2 BOARD REF<br>header 2x1 MBxxxxy<br>Title: MB1280<br>Project: STMod+ Fanout<br>Variant: 3V3<br>Revision: C -01 Reference: MB1280<br>Size: A4 Date: 18-SEP-2019 Sheet: 2 of 2<br>1<br>1<br>**----- End of picture text -----**<br>


**==> picture [510 x 447] intentionally omitted <==**

**==> picture [506 x 407] intentionally omitted <==**

**==> picture [510 x 449] intentionally omitted <==**

**==> picture [21 x 8] intentionally omitted <==**

**==> picture [506 x 407] intentionally omitted <==**
