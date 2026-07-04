<!-- page: 1 -->
ST1S31

3 A DC step-down switching regulator

Datasheet - production data

Applications

- µP/ASIC/DSP/FPGA core and I/O supplies
- Point of load for: STB, TVs, DVDs
- Optical storage, hard disk drive, printers,
audio/graphic cards
VFDFPN 3 x 3 - 8L SO8
Description

The ST1S31 device is an internally compensated
Features 1.5 MHz fixed-frequency PWM synchronous step-
down regulator. The ST1S31 operates from 2.8 V- 3 A DC output current
to 5.5 V input, while it regulates an output voltage
- 2.8 V to 5.5 V input voltage as low as 0.8 V and up to VIN.
- Output voltage adjustable from 0.8 V The ST1S31 integrates a 60 m high-side switch
- 1.5 MHz switching frequency and a 45 m synchronous rectifier allowing very
- Internal soft-start and enable high efficiency with very low output voltages.
- Integrated 60 m and 45 m power MOSFETs The peak current mode control with internal
compensation delivers a very compact solution- All ceramic capacitor
with a minimum component count.
- Power Good (POR)
The ST1S31 device is available in 3 mm x 3 mm,
- Cycle-by-cycle current limiting 8 lead VFDFPN and SO8 packages.
- Current foldback short-circuit protection
- VFDFPN 3 x 3 - 8L, SO8 packages

Figure 1. Application circuit

/
9,1 9287
9,16: 6:

9,1$

5
&LQBD (1 67 6
9)% &RXW
&LQBVZ 3*

*1'
5

August 2018 DocID022998 Rev 5 1/36

This is information on a product in full production. www.st.com

<!-- page: 2 -->
Contents ST1S31

Contents

1 Pin settings . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4

1.1 Pin connection . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4

1.2 Pin description . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4

2 Maximum ratings . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5

Thermal data. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5

3 Electrical characteristics . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6

4 Functional description . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8

4.1 Output voltage adjustment . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9

4.2 Soft-start . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9

4.3 Error amplifier and control loop stability . . . . . . . . . . . . . . . . . . . . . . . . . . 10

4.4 Overcurrent protection . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14

4.5 Enable function . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15

4.6 Light load operation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15

4.7 Hysteretic thermal shutdown . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15

5 Application information . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16

5.1 Input capacitor selection . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16

5.2 Inductor selection . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 17

5.3 Output capacitor selection . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 18

5.4 Thermal dissipation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 19

5.5 Layout consideration . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 20

6 Demonstration board . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 22

7 Typical characteristics . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 24

8 Package information . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30

8.1 VFDFPN 3 x 3 - 8L package information . . . . . . . . . . . . . . . . . . . . . . . . . 31

8.2 SO8 package information . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 33

2/36 DocID022998 Rev 5

<!-- page: 3 -->
ST1S31 Contents

9 Order codes . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 34

10 Revision history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 35

DocID022998 Rev 5 3/36

36

<!-- page: 4 -->
Pin settings ST1S31

1 Pin settings

1.1 Pin connection

Figure 2. Pin connection (top view)

1.2 Pin description

Table 1. Pin description

No. Type Description

1 VINA Unregulated DC input voltage

Enable input. With EN higher than 1.5 V the device is ON and with EN
2 EN
lower than 0.5 V the device is OFF.

Feedback input. Connecting the output voltage directly to this pin the
3 FB output voltage is regulated at 0.8 V. To have higher regulated voltages an
external resistor divider is required from VOUT to the FB pin.

4 AGND Ground

Open drain Power Good (POR) pin. It is released (open drain) when the
output voltage is higher than 0.92 * VOUT with a delay of 170 s. If the 5 PG
output voltage is below 0.92 * VOUT, the POR pin goes to low impedance
immediately. If not used, it can be left floating or to GND.

6 VINSW Power input voltage

7 SW Regulator output switching pin

8 PGND Power ground

(VFDFPN package only) exposed pad connected to ground assuring
ePAD
electrical contact and heat conduction.

4/36 DocID022998 Rev 5

<!-- page: 5 -->
ST1S31 Maximum ratings

2 Maximum ratings

Table 2. Absolute maximum ratings

Symbol Parameter Value Unit

VIN Input voltage -0.3 to 7
VEN Enable voltage -0.3 to VIN
VSW Output switching voltage -1 to VIN V
VPG Power-on reset voltage (Power Good) -0.3 to VIN
VFB Feedback voltage -0.3 to 1.5

1.5 (VFDFPN)
PTOT Power dissipation at TA < 60 °C W 0.9 (SO8)

TOP Operating junction temperature range -40 to 150 °C
Tstg Storage temperature range -55 to 150 °C

Thermal data

Table 3. Thermal data

Symbol Parameter Value Unit

Maximum thermal resistance VFDFPN 50
RthJA °C/W junction ambient(1) SO8 100

1. Package mounted on demonstration board.

DocID022998 Rev 5 5/36

36

<!-- page: 6 -->
Electrical characteristics ST1S31

3 Electrical characteristics

TJ = 25 °C, VIN = 5 V, unless otherwise specified.

Table 4. Electrical characteristics

Values
Symbol Parameter Test condition Unit
Min. Typ. Max.

VIN Operating input voltage range (1) 2.8 5.5
VINON Turn-on VCC threshold (1) 2.4 V
VINOFF Turn-off VCC threshold (1) 2.0
RDSON-P High-side switch ON-resistance ISW = 300 mA 60 m
RDSON-N Low-side switch ON-resistance ISW = 300 mA 45 m
ILIM Maximum limiting current (2) 4.0 A

Oscillator

FSW Switching frequency 1.2 1.5 1.9 MHz
DMAX Maximum duty cycle (2) 95 100 %

Dynamic characteristics

0.792 0.8 0.808
VFB Feedback voltage V
Io = 10 mA to 4 A(1) 0.776 0.8 0.824

%VOUT/ Reference load regulation Io = 10 mA to 4 A(2) 0.2 0.6 %
IOUT
%VOUT/ Reference line regulation VIN = 2.8 V to 5.5 V(2) 0.2 0.3 %
VIN

DC characteristics

IQ Quiescent current Duty cycle = 0, no load VFB = 1.2 V 630 1200 µA
IQST-BY Total standby quiescent current OFF 1 µA

Enable

Device ON level 1.5
VEN EN threshold voltage V
Device OFF level 0.5

IEN EN current 0.1 µA

Power Good

PG threshold 92 %VFB

PG hystereris 30 50
PG mV
PG output voltage low Isink = 6 mA open drain 400

PG rise delay 170 µs

6/36 DocID022998 Rev 5

<!-- page: 7 -->
ST1S31 Electrical characteristics

Table 4. Electrical characteristics (continued)

Values
Symbol Parameter Test condition Unit
Min. Typ. Max.

Soft-start

TSS Soft-start duration 400 µs

Protection

Thermal shutdown 150
TSHDN °C
Hystereris 20

1. Specification referred to TJ from -40 to +125 °C. Specifications in the -40 to +125 °C temperature range are assured by
design, characterization and statistical correlation.

2. Guaranteed by design.

DocID022998 Rev 5 7/36

36

<!-- page: 8 -->
Functional description ST1S31

4 Functional description

The ST1S31 device is based on a “peak current mode”, constant frequency control. The
output voltage VOUT is sensed by the feedback pin (FB) compared to an internal reference
(0.8 V) providing an error signal that, compared to the output of the current sense amplifier,
controls the ON and OFF time of the power switch.

The main internal blocks are shown in the block diagram in Figure 3. They are:
- A fully integrated oscillator that provides the internal clock and the ramp for the slope
compensation avoiding sub-harmonic instability
- The soft-start circuitry to limit inrush current during the startup phase
- The transconductance error amplifier
- The pulse width modulator and the relative logic circuitry necessary to drive the internal
power switches
- The drivers for embedded P-channel and N-channel power MOSFET switches
- The high-side current sensing block
- The low-side current sense to implement diode emulation
- A voltage monitor circuitry (UVLO) that checks the input and internal voltages
- A thermal shutdown block, to prevent thermal runaway.

Figure 3. Block diagram

AM11417v1

8/36 DocID022998 Rev 5

<!-- page: 9 -->
ST1S31 Functional description

4.1 Output voltage adjustment

The error amplifier reference voltage is 0.8 V typical. The output voltage is adjusted
according to the following formula (see Figure 1 on page 1):

Equation 1
 R1 VOUT = 0.8   1 + ------  R2

The internal architecture of the device requires a minimum off time, cycle-by-cycle, for the
output voltage regulation. The minimum off time is typically equal to 94 ns.

The control loop compensates for conversion losses with duty cycle control. Since the
power losses are proportional to the delivered output power, the duty cycle increases with
the load current request.

Figure 4 shows the maximum regulated output voltage over the input voltage range at
different loading conditions.

Figure 4. Maximum output voltage over loading conditions

4.2 Soft-start

The soft-start is essential to assure the correct and safe startup of the step-down converter.
It avoids inrush current surge and makes the output voltage rise monotonically.

The soft-start is managed by ramping the reference of the error amplifier from 0 V to 0.8 V.
The internal soft-start capacitor is charged with a resistor to 0.8 V, then the FB pin follows
the reference so that the output voltage is regulated to rise to the set value monotonically.

DocID022998 Rev 5 9/36

36

<!-- page: 10 -->
Functional description ST1S31

4.3 Error amplifier and control loop stability

The error amplifier provides the error signal to be compared with the high-side switch
current through the current sense circuitry. The non inverting input is connected with the
internal 0.8 V reference, while the inverting input is the FB pin. The compensation network is
internal and connected between the E/A output and GND.

The error amplifier of the ST1S31 device is a transconductance operational amplifier, with
high bandwidth and high output impedance.

Table 5. Characteristics of the uncompensated error amplifier

Description Value

DC gain 94 dB

gm 238 µA/V

Ro 96 M

The ST1S31 device embeds the compensation network that assures the stability of the loop
in the whole operating range. All the tools needed to check the loop stability are shown on
the next pages of this section.

In Figure 5 the simple small signal model for the peak current mode control loop is shown.

Figure 5. Block diagram of the loop for the small signal analysis

VIN
Slope GCO(s)
Compensation
High side
Switch
L GDIV(s)
Current sense VOUT

Logic Cout
And Low side
Driver Switch
PWM comparator

0.8V

R1

VC
VFB

Rc Error Amp
R2

Cc

GEA(s)

AM11418v1

Three main terms can be identified to obtain the loop transfer function:
1. From control (output of E/A) to output, GCO(s)
2. From output (VOUT) to the FB pin, GDIV(s)
3. From the FB pin to control (output of E/A), GEA(s).
The transfer function from control to output GCO(s) results:

10/36 DocID022998 Rev 5

<!-- page: 11 -->
ST1S31 Functional description

Equation 2
 s  1 + ------
RLOAD 1  z
GCO s = -----------------  --------------------------------------------------------------------------------------------  ---------------------  FH s Ri Rout  TSW  s 
1 + ---------------------------   mC   1 – D  – 0.5   1 + ------ L p

where RLOAD represents the load resistance, Ri the equivalent sensing resistor of the
current sense circuitry (0.369 ), p the single pole introduced by the LC filter and z the
zero given by the ESR of the output capacitor.

FH(s) accounts for the sampling effect performed by the PWM comparator on the output of
the error amplifier that introduces a double pole at one half of the switching frequency.

Equation 3
1
Z = ------------------------------- ESR  COUT

Equation 4
1 mC   1 – D  – 0.5
p = ------------------------------------- + --------------------------------------------- RLOAD  COUT L  COUT  fSW

where:

Equation 5
 Se
 mC = 1 + ------
 Sn

 Se = Vpp  fSW

 VIN – VOUT
 Sn = -----------------------------L  Ri

Sn represents the ON-time slope of the sensed inductor current, Se the slope of the external
ramp (VPP peak-to-peak amplitude - 0.535 V) that implements the slope compensation to
avoid sub-harmonic oscillations at duty cycle over 50%.

The sampling effect contribution FH(s) is:

Equation 6
1
FH s = ------------------------------------------
s s2
1 + ------------------ + ------
n  QP 2 n

where:

Equation 7
1
QP = ----------------------------------------------------------    mC   1 – D  – 0.5 

and

Equation 8

n = fSW

DocID022998 Rev 5 11/36

36

<!-- page: 12 -->
Functional description ST1S31

The resistor to adjust the output voltage that gives the term from output voltage to the FB
pin. GDIV(s) is:

R2
GDIV s = -------------------- R1 + R2

The transfer function from FB to Vc (output of E/A) introduces the singularities (poles and
zeroes) to stabilize the loop. The small signal model of the error amplifier with the internal
compensation network can be seen in Figure 6.

Figure 6. Small signal model for the error amplifier

VFB

Ro Co Rc Cp Cc Vd
Gm*Vd
Cc VREF

AM11419v1

RC and CC introduce a pole and a zero in the open loop gain. CP does not significantly affect
system stability and can be neglected.

So GEA(s) results:

Equation 9
GEA0   1 + s  Rc  Cc 
GEA s = -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
s2  R0   C0 + Cp Rc  Cc + s   R0  Cc + R0   C0 + Cp  + Rc  Cc  + 1

where GEA = Gm · Ro.
The poles of this transfer function are (if Cc >> C0+CP):

Equation 10
1
fP LF = --------------------------------- 2  R0  Cc

Equation 11
1
fP HF = ---------------------------------------------------- 2  Rc   C0 + Cp 

whereas the zero is defined as:

Equation 12
1
fZ = --------------------------------- 2  Rc  Cc

12/36 DocID022998 Rev 5

<!-- page: 13 -->
ST1S31 Functional description

The embedded compensation network is RC = 80 k, CC = 55 pF while CP and CO can be
considered as negligible. The error amplifier output resistance is 96 Mso the relevant
singularities are:

Equation 13

fZ = 36  2 kHz fP LF = 30 Hz

So closing the loop, the loop gain GLOOP(s) is:

Equation 14

GLOOP s = GCO GDIVs  GEAs  s

Example 1:
VIN = 5 V, VOUT = 1.2 V, Iomax = 3 A, L = 1.0 H, Cout = 47 µF (MLCC), R1 = 10 k,
R2 = 20 k(see Section 5.2 and Section 5.3 for inductor and output capacitor selection
guidelines).

The module and phase bode plot are reported in Figure 7 and Figure 8.

The bandwidth is 117 kHz and the phase margin is 63 degrees.

Figure 7. Module bode plot

DocID022998 Rev 5 13/36

36

<!-- page: 14 -->
Functional description ST1S31

Figure 8. Phase bode plot

4.4 Overcurrent protection

The ST1S31 device implements overcurrent protection sensing the current flowing through
the high-side current switch.

If the current exceeds the overcurrent threshold the high-side is turned off, implementing
a cycle-by-cycle current limitation. Since the regulation loop is no longer fixing the duty
cycle, the output voltage is unregulated and the FB pin falls accordingly to the new duty
cycle.

If the FB falls below 0.2 V, the peak current limit is reduced to around 2.3 A and the
switching frequency is reduced to assure that the inductor current is properly limited below
the above mentioned value and above 1.2 A. This strategy is called “current foldback”.

The mechanism to adjust the switching frequency during the current foldback condition
exploits the low-side current sense circuitry. If FB is lower than 0.2 V, the high-side power
MOSFET is turned off when the current reaches the current foldback threshold (2.3 A), then,
after a proper deadtime that avoids the cross conduction, the low-side is turned on until the
low-side current is lower than a valley threshold (1.2 A). Once the low-side is turned off, the
high-side is immediately turned on. In this way the frequency is adjusted to keep the
inductor current ripple between the current foldback value (2.3 A) and valley threshold
(1.2 A), so properly limiting the output current in case of overcurrent or short-circuit.

It should be noted that in some cases, mainly with very low output voltages, the hard
overcurrent can cause the FB to find the new equilibrium just over the current foldback
threshold (0.2 V). In this case no frequency reduction is enabled, then the inductor current
may diverge. This means that the ripple current during the minimum ON-time is higher than
the ripple current during the OFF-time (the switching period minus the minimum ON-time),
so pulse-by-pulse, the average current is rising, exceeding the current limit.

In order to avoid too high current, a further protection is activated when the high-side current
exceeds a further current threshold (OCP2) slightly over the current limit (OCP1). If the
current triggers the second threshold, the converter stops switching, the reference of the
error amplifier is pulled down and then it restarts with a soft-start procedure. If the
overcurrent condition is still active, the current foldback with frequency reduction properly
limits the output current to 2.3 A.

14/36 DocID022998 Rev 5

<!-- page: 15 -->
ST1S31 Functional description

4.5 Enable function

The enable feature allows the device to be put into standby mode. With the EN pin is lower
than 0.4 V, the device is disabled and the power consumption is reduced to less than 10 A.
With the EN pin higher than 1.2 V, the device is enabled. If the EN pin is left floating, an
internal pull-down ensures that the voltage at the pin reaches the inhibit threshold and the
device is disabled. The pin is also VIN compatible.

4.6 Light load operation

With peak current mode control loop the output of the error amplifier is proportional to the
load current. In the ST1S31 device, to increase light load efficiency when the output of the
error amplifier falls below a certain threshold, the high-side turn-on is prevented.

This mechanism reduces the switching frequency at light load in order to save the switching
losses.

4.7 Hysteretic thermal shutdown

The thermal shutdown block generates a signal that turns off the power stage if the junction
temperature goes above 150 °C. Once the junction temperature goes back to about 130 °C,
the device restarts in normal operation.

DocID022998 Rev 5 15/36

36

<!-- page: 16 -->
Application information ST1S31

5 Application information

5.1 Input capacitor selection

The capacitor connected to the input must be capable of supporting the maximum input
operating voltage and the maximum RMS input current required by the device. The input
capacitor is subject to a pulsed current, the RMS value of which is dissipated over its ESR,
affecting the overall system efficiency.

So the input capacitor must have an RMS current rating higher than the maximum RMS
input current and an ESR value compliant with the expected efficiency.

The maximum RMS input current flowing through the capacitor can be calculated as:

Equation 15

2  D2 D2
IRMS = IO  D – -------------- + ------  2

where Io is the maximum DC output current, D is the duty cycle, and is the efficiency.
Considering = 1, this function has a maximum at D = 0.5 and is equal to Io/2.

The peak-to-peak voltage across the input capacitor can be calculated as:

Equation 16

IO  D D  VPP = -------------------------  1 – ---   D + ---   1 – D  + ESR  IO CIN  FSW

where ESR is the equivalent series resistance of the capacitor.

Given the physical dimension, ceramic capacitors can well meet the requirements of the
input filter sustaining a higher input RMS current than electrolytic / tantalum types. In this
case the equation of CIN as a function of the target peak-to-peak voltage ripple (VPP) can be
written as follows:

Equation 17
IO  D D --------------------------  CIN =  1 – ---   D + ---   1 – D  VPP  FSW

neglecting the small ESR of ceramic capacitors.
Considering = 1, this function has its maximum in D = 0.5, therefore, given the maximum
peak-to-peak input voltage (VPP_MAX), the minimum input capacitor (CIN_MIN) value is:

Equation 18
IO
CIN_MIN = ----------------------------------------------- 2  VPP_MAX  FSW

Typically, CIN is dimensioned to keep the maximum peak-to-peak voltage ripple in the order
of 1% of VINMAX.

16/36 DocID022998 Rev 5

<!-- page: 17 -->
ST1S31 Application information

The placement of the input capacitor is very important to avoid noise injection and voltage
spikes on the input voltage pin. So the CIN must be placed as close as possible to the
VIN_SW pin. In Table 6 some multilayer ceramic capacitors suitable for this device are
given.

Table 6. Input MLCC capacitors

Manufacturer Series Cap value (µF) Rated voltage (V)

Murata GRM21 10 10

C3225 10 25
TDK
C3216 10 16

TAIYO YUDEN LMK212 22 10

A ceramic bypass capacitor, as close as possible to the VINA pin so that additional parasitic
ESR and ESL are minimized, is suggested in order to prevent instability on the output
voltage due to noise. The value of the bypass capacitor can go from 330 nF to 1 µF.

5.2 Inductor selection

The inductance value fixes the current ripple flowing through the output capacitor. So the
minimum inductance value to have the expected current ripple must be selected. The rule to
fix the current ripple value is to have a ripple at 20% - 40% of the output current.

In continuous current mode (CCM), the inductance value can be calculated by Equation 19:

Equation 19
VIN – VOUT VOUT
IL = -----------------------------  TON = --------------  TOFF L L

where TON is the conduction time of the high-side switch and TOFF is the conduction time of
the low-side switch (in CCM, FSW = 1/(TON + TOFF)). The maximum current ripple, given the
VOUT, is obtained at maximum TOFF, that is, at minimum duty cycle (see previous section to
calculate minimum duty). So by fixing IL = 20% to 30% of the maximum output current, the
minimum inductance value can be calculated:

Equation 20
VOUT 1 – DMIN
LMIN = ----------------  ---------------------- IMAX FSWMIN

where FSWMIN is the minimum switching frequency, according to Table 4. The slope
compensation, to prevent the sub-harmonic instability in the peak current control loop, is
internally managed and so fixed. This implies a further lower limit for the inductor value. To
assure sub-harmonic stability:

Equation 21
L  Vout   2  Vpp  fsw 

where VPP is the peak-to-peak value of the slope compensation ramp. The inductor value
selected based on Equation 20 must satisfy Equation 21. The peak current through the
inductor is given by Equation 22:

DocID022998 Rev 5 17/36

36

<!-- page: 18 -->
Application information ST1S31

Equation 22
IL
IL  PK = IO + ------- 2

So if the inductor value decreases, the peak current (which must be lower than the current
limit of the device) increases. The higher the inductor value, the higher the average output
current that can be delivered, without reaching the current limit.

In Table 7 some inductor part numbers are listed.

Table 7. Inductors

Manufacturer Series Inductor value (µH) Saturation current (A)

XAL50xx 1.2 to 3.3 6.3 to 9

Coilcraft XAL60xx 2.2 to 5.6 7.4 to 11

MSS1048 1.0 to 3.8 6.5 to 11

WE-HCI 7030 1.5 to 4.7 7 to 14
Würth
WE-PD type L 1.5 to 3.5 6.4 to 10

DR73 1.0 to 2.2 5.5 to7.9
Coiltronics
DR74 1.5 to 3.3 5.4 to 8.35

5.3 Output capacitor selection

The current in the output capacitor has a triangular waveform which generates a voltage
ripple across it. This ripple is due to the capacitive component (charge or discharge of the
output capacitor) and the resistive component (due to the voltage drop across its ESR). So
the output capacitor must be selected in order to have a voltage ripple compliant with the
application requirements.

The amount of the voltage ripple can be calculated starting from the current ripple obtained
by the inductor selection.

Equation 23
IMAX
VOUT = ESR  IMAX + ------------------------------------ 8  COUT  fSW

For a ceramic (MLCC) capacitor, the capacitive component of the ripple dominates the
resistive one. While for an electrolytic capacitor the opposite is true.

As the compensation network is internal, the output capacitor should be selected in order to
have a proper phase margin and then a stable control loop.

The equations of Section 5.2 help to check loop stability given the application conditions,
the value of the inductor and of the output capacitor.

18/36 DocID022998 Rev 5

<!-- page: 19 -->
ST1S31 Application information

In Table 8 some capacitor series are listed.

Table 8. Output capacitors

Manufacturer Series Cap value (µF) Rated voltage (V) ESR (m)

GRM32 22 to 100 6.3 to 25 < 5
Murata
GRM31 10 to 47 6.3 to 25 < 5

ECJ 10 to 22 6.3 < 5
Panasonic
EEFCD 10 to 68 6.3 15 to 55

Sanyo TPA/B/C 100 to 470 4 to 16 40 to 80

TDK C3225 22 to 100 6.3 < 5

5.4 Thermal dissipation

The thermal design is important to prevent the thermal shutdown of the device if junction
temperature goes above 150 °C. The three different sources of losses within the device are:
a) conduction losses due to the on-resistance of high-side switch (RHS) and low-side
switch (RLS); these are equal to:

Equation 24

2 2
PCOND = RHS  IOUT  D + RLS  IOUT   1 – D 

where D is the duty cycle of the application. Note that the duty cycle is theoretically given by
the ratio between VOUT and VIN, but it is actually slightly higher to compensate the losses of
the regulator.
b) switching losses due to high-side power MOSFET turn-on and turn-off; these can
be calculated as:

Equation 25
 TRISE + TFALL 
PSW = VIN  IOUT  ------------------------------------------  Fsw = VIN  IOUT  TSW  FSW 2

where TRISE and TFALL are the overlap times of the voltage across the high-side power
switch (VDS) and the current flowing into it during the turn-on and turn-off phases, as shown
in Figure 9. TSW is the equivalent switching time. For this device the typical value for the
equivalent switching time is 20 ns.
c) Quiescent current losses, calculated as:

Equation 26

PQ = VIN  IQ

where IQ is the quiescent current (IQ = 1.2 mA maximum).

DocID022998 Rev 5 19/36

36

<!-- page: 20 -->
Application information ST1S31

The junction temperature TJ can be calculated as:

Equation 27

TJ = TA + RthJA  PTOT

where TA is the ambient temperature and PTOT is the sum of the power losses just seen.
RthJA is the equivalent thermal resistance junction to ambient of the device; it can be
calculated as the parallel of many paths of heat conduction from the junction to the ambient.
For this device the path through the exposed pad is the one conducting the largest amount
of heat. The RthJA measured on the demonstration board described in Section 5.5 is about
50 °C/W for the VFDFPN and 100 °C/W for the SO8 package.

Figure 9. Switching losses

VIN
VSW

ISW,HS

VDS,HS

PSW

PCOND,HS PCOND,LS

TFALL TRISE

AM11422v1

5.5 Layout consideration

The PC board layout of the switching DC-DC regulator is very important to minimize the
noise injected in high impedance nodes, to reduce interference generated by the high
switching current loops and to optimize the reliability of the device.

In order to avoid EMC problems, the high switching current loops must be as short as
possible. In the buck converter there are two high switching current loops: during the
on-time, the pulsed current flows through the input capacitor, the high-side power switch,
the inductor and the output capacitor; during the off-time, through the low-side power switch,
the inductor and the output capacitor.

The input capacitor connected to VINSW must be placed as close as possible to the device,
to avoid spikes on VINSW due to the stray inductance and the pulsed input current.

In order to prevent dynamic unbalance between VINSW and VINA, the trace connecting the
VINA pin to the input must be derived from VINSW.

20/36 DocID022998 Rev 5

<!-- page: 21 -->
ST1S31 Application information

The feedback pin (FB) connection to the external resistor divider is a high impedance node,
so the interference can be minimized by routing the feedback node with a very short trace
and as far as possible from the high current paths.

A single point connection from signal ground to power ground is suggested.

Thanks to the exposed pad of the device, the ground plane helps to reduce the thermal
resistance junction to ambient; so a large ground plane, soldered to the exposed pad,
enhances the thermal performance of the converter allowing high power conversion.

Figure 10. PCB layout example

Via to connect the thermal pad
To bottom or inner ground plane Star center for common ground

Short high
switching
current loop

Input cap as
close as possible
to VINSW pin

Short FB trace VINA derived from Cin
To avoid dynamic voltage drop
Between VINA and VINSW

AM11423v1

DocID022998 Rev 5 21/36

36

<!-- page: 22 -->
Demonstration board ST1S31

6 Demonstration board

Figure 11. Demonstration board schematic

5V
VIN

U1U1 ST1S31ST1S31
1 8 L1L1 3.3V VIN_A PGND

C3C3 2 7 Vout EN SW
1u1u
3 6 2.2uH2.2uH
FB VIN_SW

4 5 C1C1 C2C2
AGND ePAD PGOOD 10u10u 22u22u

R3R3

R1R1 10k10k

R2R2 62.5k62.5k C4C4
20k20k

NCNC

0 0

AM11424v1

Table 9. Component list

Reference Part number Description Manufacturer

U1 ST1S31 ST

L1 DR73 2R2 2.2 µH, Isat = 5.5 A Coiltronics

C1 C3225X7RE106K 10 µF 25 V X7R TDK

C2 C3225X7R1C226M 22 µF 16 V X7R TDK

C3 1 µF 25 V X7R

C4 NC

R1 62.5 k

R2 20 k

R3 10 k

22/36 DocID022998 Rev 5

<!-- page: 23 -->
## Visual Summary (Page 23)

- page_class: block_diagram
- confidence: 0.95
- reason_codes: visual_content_dominant, pcb_diagrams_require_visual_summary, policy_visual_summary_override

Page displays two PCB layout diagrams for ST1S31 demonstration boards: Figure 12 shows the DFN package variant and Figure 13 shows the SO8 package variant, both illustrating component placement including capacitors (C1-C4), resistors (R1-R3), inductor (L1), and input/output connectors.

<!-- page: 24 -->
## Visual Summary (Page 24)

- page_class: chart
- confidence: 0.95
- reason_codes: heuristic_page_type_chart, heuristic_content_type_chart, visual_data_dominance, policy_visual_summary_override

Page displays two efficiency characteristic graphs for ST1S31 under VIN=3.3V: Figure 14 shows linear scale output current (0-3A) with three curves for Vout 1.2V, 1.8V, and 2.5V; Figure 15 presents log-scale output current (0.001-1A) showing efficiency peaking around 90% across all outputs.

<!-- page: 25 -->
## Visual Summary (Page 25)

- page_class: chart
- confidence: 0.95
- reason_codes: heuristic_page_type_chart, heuristic_content_type_chart, visual_data_over_text, policy_visual_summary_override

Page contains two performance graphs for ST1S31: Figure 16 shows load regulation vs output current at VIN=3.3V (blue line ~-0.4% to -0.5%, black line ~-0.2% to -0.7%), and Figure 17 shows efficiency curves at VIN=4.0V for Vout levels of 1.2V, 1.8V, and 2.5V (efficiency peaks near 90-95%).

<!-- page: 26 -->
## Visual Summary (Page 26)

- page_class: chart
- confidence: 0.95
- reason_codes: heuristic_chart_detected, visual_data_priority, policy_visual_summary_override

Page contains two performance graphs for ST1S31: Figure 18 shows efficiency curves (log scale) at VIN=4.0V across three output voltages (1.2V, 1.8V, 2.5V), peaking near 90-95% efficiency; Figure 19 displays load regulation (% deviation from nominal voltage) versus output current for 1.2V and 1.8V outputs, showing stable regulation within ±0.4% across the tested range.

<!-- page: 27 -->
## Visual Summary (Page 27)

- page_class: chart
- confidence: 0.95
- reason_codes: heuristic_chart_detected, visual_data_priority, policy_visual_summary_override

Page contains two efficiency characteristic graphs for the ST1S31 device at VIN=5.0V: Figure 20 shows linear scale output current (0-3A) and Figure 21 shows log scale output current (0.001-1A). Both charts plot Efficiency [%] vs Output Current [A] with four curves representing Vout levels of 1.2V, 1.8V, 2.5V, and 3.3V.

<!-- page: 28 -->
## Visual Summary (Page 28)

- page_class: chart
- confidence: 0.95
- reason_codes: heuristic_chart_detected, visual_data_primary, minimal_native_text, policy_visual_summary_override

Figure 22 displays the load regulation characteristics of the ST1S31 device at VIN=5.0V, plotting Load Regulation (%) against Output Current (A). Two curves are shown: a blue line for Vout=1.2V and a red line for Vout=1.8V. Both exhibit high initial ripple that stabilizes near -0.4% to -0.6% as current increases from 0A to 3A.

<!-- page: 29 -->
## Visual Summary (Page 29)

- page_class: chart
- confidence: 0.95
- reason_codes: chart_page, visual_data_dominant, text_supplementary_only, policy_visual_summary_override

Page contains four oscilloscope waveform charts illustrating ST1S31 typical characteristics: zero load operation (Fig.23), overcurrent protection triggering at 4.8A (Fig.24), 100mA continuous operation stability (Fig.25), and short-circuit protection response (Fig.26). Each chart displays IL, SW, and VOUT signals with specified voltage/division scales.

<!-- page: 30 -->
Package information ST1S31

8 Package information

In order to meet environmental requirements, ST offers these devices in different grades of
ECOPACK® packages, depending on their level of environmental compliance. ECOPACK
specifications, grade definitions and product status are available at: www.st.com.
ECOPACK is an ST trademark.

30/36 DocID022998 Rev 5

<!-- page: 31 -->
ST1S31 Package information

8.1 VFDFPN 3 x 3 - 8L package information

Figure 27. VFDFPN 3 x 3 - 8L package outline

8057023

Table 10. VFDFPN 3 x 3 - 8L package mechanical data

Dimensions (mm)
Symbol
Min. Typ. Max.

A 0.80 0.90 1.00

A1 0.0 0.05

b 0.25 0.30 0.35

D 3.00

D2 2.234 2.384 2.484

E 3.00

E2 1.496 1.646 1.746

e 0.65

L 0.30 0.40 0.50

DocID022998 Rev 5 31/36

36

<!-- page: 32 -->
## Visual Summary (Page 32)

- page_class: block_diagram
- confidence: 0.95
- reason_codes: visual_dominant, diagram_content, policy_visual_summary_override

Technical block diagram illustrating the recommended footprint dimensions for the ST1S31 VFDLPN package (3x3-8L), showing a central body with specified height of 3.50mm and width of 2.55mm, flanked by eight pins arranged in two rows with precise spacing measurements including 0.65mm, 0.725mm, 0.35mm, and 0.325mm.

<!-- page: 33 -->
ST1S31 Package information

8.2 SO8 package information

Figure 29. SO8 package outline

0016023_Rev_E

Table 11. SO8 package mechanical data

Dimensions (mm)
Symbol
Min. Typ. Max.

A 1.75

A1 0.10 0.25

A2 1.25

b 0.28 0.48

c 0.17 0.23

D 4.80 4.90 5.00

E 5.80 6.00 6.20

E1 3.80 3.90 4.00

e 1.27

h 0.25 0.50

L 0.40 1.27

L1 1.04

k 0° 8°

ccc 0.10

DocID022998 Rev 5 33/36

36

<!-- page: 34 -->
Order codes ST1S31

9 Order codes

Table 12. Ordering information

Order codes Package

ST1S31PUR VFDFPN 3 x 3 - 8 L

ST1S31D-R SO8

34/36 DocID022998 Rev 5

<!-- page: 35 -->
ST1S31 Revision history

10 Revision history

Table 13. Document revision history

Date Revision Changes

Updated Figure 2: Pin connection (top view) on page 3 (replaced
12-Nov-2014 3 by new figure).
Minor modifications throughout document.

Updated value in Table 3 on page 5 and Section 5.4 on page 19
(replaced 40 °C/W by 50 °C/W).
Added Section 4.1 on page 9.
03-Mar-2016 4
Updated Section 7 on page 24 [added Figure 14 on page 24 to
Figure 22 on page 28 (replaced figures 13 and 14)].
Minor modifications throughout document.

03-Aug-2018 5 Updated Figure 1: Application circuit on the cover page.

DocID022998 Rev 5 35/36

36

<!-- page: 36 -->
ST1S31

IMPORTANT NOTICE – PLEASE READ CAREFULLY

STMicroelectronics NV and its subsidiaries (“ST”) reserve the right to make changes, corrections, enhancements, modifications, and
improvements to ST products and/or to this document at any time without notice. Purchasers should obtain the latest relevant information on
ST products before placing orders. ST products are sold pursuant to ST’s terms and conditions of sale in place at the time of order
acknowledgement.

Purchasers are solely responsible for the choice, selection, and use of ST products and ST assumes no liability for application assistance or
the design of Purchasers’ products.

No license, express or implied, to any intellectual property right is granted by ST herein.

Resale of ST products with provisions different from the information set forth herein shall void any warranty granted by ST for such product.

ST and the ST logo are trademarks of ST. All other product or service names are the property of their respective owners.

Information in this document supersedes and replaces information previously supplied in any prior versions of this document.

© 2018 STMicroelectronics – All rights reserved

36/36 DocID022998 Rev 5
