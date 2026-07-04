**ST1S31**

**==> picture [137 x 37] intentionally omitted <==**

## 3 A DC step-down switching regulator

**Datasheet** - **production data**

**==> picture [221 x 106] intentionally omitted <==**

**----- Start of picture text -----**<br>
VFDFPN 3 x 3 - 8L  SO8<br>**----- End of picture text -----**<br>


## **Features**

- 3 A DC output current

- 2.8 V to 5.5 V input voltage

- Output voltage adjustable from 0.8 V

- 1.5 MHz switching frequency

- Internal soft-start and enable

- Integrated 60 m and 45 m power MOSFETs

- All ceramic capacitor

- Power Good (POR)

- Cycle-by-cycle current limiting

## **Applications**

- µP/ASIC/DSP/FPGA core and I/O supplies

- Point of load for: STB, TVs, DVDs

- Optical storage, hard disk drive, printers, audio/graphic cards

## **Description**

The ST1S31 device is an internally compensated 1.5 MHz fixed-frequency PWM synchronous stepdown regulator. The ST1S31 operates from 2.8 V to 5.5 V input, while it regulates an output voltage as low as 0.8 V and up to VIN.

The ST1S31 integrates a 60 m high-side switch and a 45 m synchronous rectifier allowing very high efficiency with very low output voltages.

The peak current mode control with internal compensation delivers a very compact solution with a minimum component count.

The ST1S31 device is available in 3 mm x 3 mm, 8 lead VFDFPN and SO8 packages.

- Current foldback short-circuit protection

- VFDFPN 3 x 3 - 8L, SO8 packages

## **Figure 1. Application circuit**

**==> picture [462 x 188] intentionally omitted <==**

**----- Start of picture text -----**<br>
�<br>��� ����<br>����� ��<br>����<br>��<br>����� �� ������<br>��� ����<br>������ ��<br>���<br>��<br>**----- End of picture text -----**<br>


August 2018

DocID022998 Rev 5

1/36

This is information on a product in full production.

_www.st.com_

**Contents**

**ST1S31**

## **Contents**

|**1**|**Pin**|**settings  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4**|
|---|---|---|
||1.1|Pin connection  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4|
||1.2|Pin description  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4|
|**2**|**Maximum ratings . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5**||
||Thermal data. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5||
|**3**|**Electrical characteristics  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6**||
|**4**|**Functional description  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8**||
||4.1|Output voltage adjustment . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9|
||4.2|Soft-start . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9|
||4.3|Error amplifier and control loop stability . . . . . . . . . . . . . . . . . . . . . . . . . . 10|
||4.4|Overcurrent protection  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14|
||4.5|Enable function  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15|
||4.6|Light load operation  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15|
||4.7|Hysteretic thermal shutdown  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15|
|**5**|**Application information  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16**||
||5.1|Input capacitor selection . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16|
||5.2|Inductor selection . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 17|
||5.3|Output capacitor selection  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 18|
||5.4|Thermal dissipation  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 19|
||5.5|Layout consideration  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 20|
|**6**|**Demonstration board  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 22**||
|**7**|**Typical characteristics  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 24**||
|**8**|**Package information . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30**||
||8.1|VFDFPN 3 x 3 - 8L package information  . . . . . . . . . . . . . . . . . . . . . . . . . 31|
||8.2|SO8 package information . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 33|



2/36

DocID022998 Rev 5

**ST1S31**

**Contents**

|**9**|**Order codes . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 34**|
|---|---|
|**10**|**Revision history  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 35**|



DocID022998 Rev 5

3/36

**ST1S31**

**Pin settings**

## **1 Pin settings**

## **1.1 Pin connection**

## **Figure 2. Pin connection (top view)**

**==> picture [405 x 176] intentionally omitted <==**

## **1.2 Pin description**

**Table 1. Pin description**

|||**Table 1. Pin description**|
|---|---|---|
|**No.**|**Type**|**Description**|
|1|VINA|Unregulated DC input voltage|
|2|EN|Enable input. With EN higher than 1.5 V the device is ON and with EN<br>lower than 0.5 V the device is OFF.|
|3|FB|Feedback input. Connecting the output voltage directly to this pin the<br>output voltage is regulated at 0.8 V. To have higher regulated voltages an<br>external resistor divider is required from VOUTto the FB pin.|
|4|AGND|Ground|
|5|PG|Open drain Power Good (POR) pin. It is released (open drain) when the<br>output voltage is higher than 0.92 * VOUTwith a delay of 170s. If the<br>output voltage is below 0.92 * VOUT, the POR pin goes to low impedance<br>immediately. If not used, it can be left floating or to GND.|
|6|VINSW|Power input voltage|
|7|SW|Regulator output switching pin|
|8|PGND|Power ground|
||ePAD|(VFDFPN package only) exposed pad connected to ground assuring<br>electrical contact and heat conduction.|



4/36

DocID022998 Rev 5

**ST1S31**

**Maximum ratings**

## **2 Maximum ratings**

**Table 2. Absolute maximum ratings**

|**Symbol**|**Parameter**|**Value**|**Unit**|
|---|---|---|---|
|VIN|Input voltage|-0.3 to 7|V|
|VEN|Enable voltage|-0.3 to VIN||
|VSW|Output switching voltage|-1 to VIN||
|VPG|Power-on reset voltage (Power Good)|-0.3 to VIN||
|VFB|Feedback voltage|-0.3 to 1.5||
|PTOT|Power dissipation at TA< 60 °C|1.5 (VFDFPN)<br>0.9 (SO8)|W|
|TOP|Operating junction temperature range|-40 to 150|°C|
|Tstg|Storage temperature range|-55 to 150|°C|



## **Thermal data**

**Table 3. Thermal data**

|**Symbol**|**Parameter**||**Value**|**Unit**|
|---|---|---|---|---|
|RthJA|Maximum thermal resistance<br>junction ambient(1)|VFDFPN|50|°C/W|
|||SO8|100||



1. Package mounted on demonstration board.

DocID022998 Rev 5

5/36

**ST1S31**

**Electrical characteristics**

## **3 Electrical characteristics**

TJ = 25 °C, VIN = 5 V, unless otherwise specified.

**Table 4. Electrical characteristics**

|**Symbol**|**Parameter**|**Test condition**|**Values**|**Values**|**Values**|**Unit**|
|---|---|---|---|---|---|---|
||||**Min.**|**Typ.**|**Max.**||
|VIN|Operating input voltage range|(1)|2.8||5.5|V|
|VINON|Turn-on VCCthreshold|(1)||2.4|||
|VINOFF|Turn-off VCCthreshold|(1)||2.0|||
|RDSON-P|High-side switch ON-resistance|ISW= 300 mA||60||m|
|RDSON-N|Low-side switch ON-resistance|ISW= 300 mA||45||m|
|ILIM|Maximum limiting current|(2)|4.0|||A|
|**Oscillator**|||||||
|FSW|Switching frequency||1.2|1.5|1.9|MHz|
|DMAX|Maximum duty cycle|(2)|95||100|%|
|**Dynamic characteristics**|||||||
|VFB|Feedback voltage||0.792|0.8|0.808|V|
|||Io = 10 mA to 4 A(1)|0.776|0.8|0.824||
|%VOUT/<br>IOUT|Reference load regulation|Io = 10 mA to 4 A(2)||0.2|0.6|%|
|%VOUT/<br>VIN|Reference line regulation|VIN= 2.8 V to 5.5 V(2)||0.2|0.3|%|
|**DC characteristics**|||||||
|IQ|Quiescent current|Duty cycle = 0, no load VFB= 1.2 V||630|1200|µA|
|IQST-BY|Total standby quiescent current|OFF|||1|µA|
|**Enable**|||||||
|VEN|EN threshold voltage|Device ON level|1.5|||V|
|||Device OFF level|||0.5||
|IEN|EN current||||0.1|µA|
|**Power Good**|||||||
|PG|PG threshold|||92||%VFB|
||PG hystereris||30|50||mV|
||PG output voltage low|Isink = 6 mA open drain|||400||
||PG rise delay|||170||µs|



6/36

DocID022998 Rev 5

**ST1S31**

**Electrical characteristics**

**Table 4. Electrical characteristics (continued)**

|**Symbol**|**Parameter**|**Test condition**|**Values**|**Values**|**Values**|**Unit**|
|---|---|---|---|---|---|---|
||||**Min.**|**Typ.**|**Max.**||
|**Soft-start**|||||||
|TSS|Soft-start duration|||400||µs|
|**Protection**|||||||
|TSHDN|Thermal shutdown|||150||°C|
||Hystereris|||20|||



1. Specification referred to TJ from -40 to +125 °C. Specifications in the -40 to +125 °C temperature range are assured by design, characterization and statistical correlation.

2. Guaranteed by design.

DocID022998 Rev 5

7/36

**ST1S31**

**Functional description**

## **4 Functional description**

The ST1S31 device is based on a “peak current mode”, constant frequency control. The output voltage VOUT is sensed by the feedback pin (FB) compared to an internal reference (0.8 V) providing an error signal that, compared to the output of the current sense amplifier, controls the ON and OFF time of the power switch.

The main internal blocks are shown in the block diagram in _Figure 3_ . They are:

- A fully integrated oscillator that provides the internal clock and the ramp for the slope compensation avoiding sub-harmonic instability

- The soft-start circuitry to limit inrush current during the startup phase

- The transconductance error amplifier

- The pulse width modulator and the relative logic circuitry necessary to drive the internal power switches

- The drivers for embedded P-channel and N-channel power MOSFET switches

- The high-side current sensing block

- The low-side current sense to implement diode emulation

- A voltage monitor circuitry (UVLO) that checks the input and internal voltages

- A thermal shutdown block, to prevent thermal runaway.

**Figure 3. Block diagram**

**==> picture [405 x 331] intentionally omitted <==**

**----- Start of picture text -----**<br>
AM11417v1<br>**----- End of picture text -----**<br>


8/36

DocID022998 Rev 5

**ST1S31**

**Functional description**

## **4.1 Output voltage adjustment**

The error amplifier reference voltage is 0.8 V typical. The output voltage is adjusted according to the following formula (see _Figure 1 on page 1_ ):

## **Equation 1**

**==> picture [92 x 23] intentionally omitted <==**

The internal architecture of the device requires a minimum off time, cycle-by-cycle, for the output voltage regulation. The minimum off time is typically equal to 94 ns.

The control loop compensates for conversion losses with duty cycle control. Since the power losses are proportional to the delivered output power, the duty cycle increases with the load current request.

_Figure 4_ shows the maximum regulated output voltage over the input voltage range at different loading conditions.

## **Figure 4. Maximum output voltage over loading conditions**

**==> picture [381 x 231] intentionally omitted <==**

## **4.2 Soft-start**

The soft-start is essential to assure the correct and safe startup of the step-down converter. It avoids inrush current surge and makes the output voltage rise monotonically.

The soft-start is managed by ramping the reference of the error amplifier from 0 V to 0.8 V. The internal soft-start capacitor is charged with a resistor to 0.8 V, then the FB pin follows the reference so that the output voltage is regulated to rise to the set value monotonically.

DocID022998 Rev 5

9/36

**ST1S31**

**Functional description**

## **4.3 Error amplifier and control loop stability**

The error amplifier provides the error signal to be compared with the high-side switch current through the current sense circuitry. The non inverting input is connected with the internal 0.8 V reference, while the inverting input is the FB pin. The compensation network is internal and connected between the E/A output and GND.

The error amplifier of the ST1S31 device is a transconductance operational amplifier, with high bandwidth and high output impedance.

**Table 5. Characteristics of the uncompensated error amplifier**

|**Description**|**Value**|
|---|---|
|DC gain|94 dB|
|gm|238 µA/V|
|Ro|96 M|



The ST1S31 device embeds the compensation network that assures the stability of the loop in the whole operating range. All the tools needed to check the loop stability are shown on the next pages of this section.

In _Figure 5_ the simple small signal model for the peak current mode control loop is shown.

**Figure 5. Block diagram of the loop for the small signal analysis**

**==> picture [405 x 259] intentionally omitted <==**

**----- Start of picture text -----**<br>
VIN<br>Slope GCO(s)<br>Compensation<br>High side<br>Switch<br>L GDIV(s)<br>Current sense VOUT<br>Logic Cout<br>And  Low side<br>Driver Switch<br>PWM comparator<br>0.8V<br>R1<br>VC<br>VFB<br>Rc<br>Error Amp<br>R2<br>Cc<br>GEA(s)<br>AM11418v1<br>**----- End of picture text -----**<br>


Three main terms can be identified to obtain the loop transfer function:

1. From control (output of E/A) to output, GCO(s)

2. From output (VOUT) to the FB pin, GDIV(s)

3. From the FB pin to control (output of E/A), GEA(s).

The transfer function from control to output GCO(s) results:

10/36

DocID022998 Rev 5

**ST1S31**

**Functional description**

## **Equation 2**

GCOs = ---------------RLOADRi **-**  1 -------------------------------------------------------------------------------------------+ R -------------------------out L TSW **-**  m1 C  1 – D – 0.5 **-**   -------------------11 ++  ----- -----sspz **-**  FHs

where _RLOAD_ represents the load resistance, _Ri_ the equivalent sensing resistor of the current sense circuitry (0.369 ),  _p_ the single pole introduced by the LC filter and  _z_ the zero given by the ESR of the output capacitor.

FH(s) accounts for the sampling effect performed by the PWM comparator on the output of the error amplifier that introduces a double pole at one half of the switching frequency.

## **Equation 3**

1 Z = ------------------------------ESR  COUT

## **Equation 4**

**==> picture [161 x 24] intentionally omitted <==**

where:

## **Equation 5**

**==> picture [90 x 67] intentionally omitted <==**

_Sn_ represents the ON-time slope of the sensed inductor current, _Se_ the slope of the external ramp (VPP peak-to-peak amplitude - 0.535 V) that implements the slope compensation to avoid sub-harmonic oscillations at duty cycle over 50%.

The sampling effect contribution FH(s) is:

## **Equation 6**

where: **Equation 7** and

## **Equation 8**

**==> picture [115 x 175] intentionally omitted <==**

DocID022998 Rev 5

11/36

**ST1S31**

**Functional description**

The resistor to adjust the output voltage that gives the term from output voltage to the FB pin. GDIV(s) is:

**==> picture [79 x 24] intentionally omitted <==**

The transfer function from FB to Vc (output of E/A) introduces the singularities (poles and zeroes) to stabilize the loop. The small signal model of the error amplifier with the internal compensation network can be seen in _Figure 6_ .

**Figure 6. Small signal model for the error amplifier**

**==> picture [405 x 172] intentionally omitted <==**

**----- Start of picture text -----**<br>
V FB<br>Ro Co CcRc Cp<br>Vd<br>Gm*Vd<br>Cc<br>VREF<br>AM11419v1<br>**----- End of picture text -----**<br>


RC and CC introduce a pole and a zero in the open loop gain. CP does not significantly affect system stability and can be neglected.

So GEA(s) results:

## **Equation 9**

**==> picture [323 x 30] intentionally omitted <==**

where GEA = Gm · Ro.

The poles of this transfer function are (if Cc >> C0+CP):

## **Equation 10**

## **Equation 11**

**==> picture [113 x 73] intentionally omitted <==**

whereas the zero is defined as:

**Equation 12**

**==> picture [73 x 22] intentionally omitted <==**

12/36

DocID022998 Rev 5

**ST1S31**

**Functional description**

The embedded compensation network is RC = 80 k, CC = 55 pF while CP and CO can be considered as negligible. The error amplifier output resistance is 96 M so the relevant singularities are:

## **Equation 13**

fZ = 36 2 kHz fP LF = 30 Hz

So closing the loop, the loop gain GLOOP(s) is:

## **Equation 14**

**==> picture [161 x 10] intentionally omitted <==**

## **Example 1** :

VIN = 5 V, VOUT = 1.2 V, Iomax = 3 A, L = 1.0 H, Cout = 47 µF (MLCC), R1 = 10 k, R2 = 20 k(see _Section 5.2_ and _Section 5.3_ for inductor and output capacitor selection guidelines).

The module and phase bode plot are reported in _Figure 7_ and _Figure 8_ .

The bandwidth is 117 kHz and the phase margin is 63 degrees.

**Figure 7. Module bode plot**

**==> picture [405 x 199] intentionally omitted <==**

DocID022998 Rev 5

13/36

**ST1S31**

**Functional description**

## **Figure 8. Phase bode plot**

**==> picture [405 x 187] intentionally omitted <==**

## **4.4 Overcurrent protection**

The ST1S31 device implements overcurrent protection sensing the current flowing through the high-side current switch.

If the current exceeds the overcurrent threshold the high-side is turned off, implementing a cycle-by-cycle current limitation. Since the regulation loop is no longer fixing the duty cycle, the output voltage is unregulated and the FB pin falls accordingly to the new duty cycle.

If the FB falls below 0.2 V, the peak current limit is reduced to around 2.3 A and the switching frequency is reduced to assure that the inductor current is properly limited below the above mentioned value and above 1.2 A. This strategy is called “current foldback”.

The mechanism to adjust the switching frequency during the current foldback condition exploits the low-side current sense circuitry. If FB is lower than 0.2 V, the high-side power MOSFET is turned off when the current reaches the current foldback threshold (2.3 A), then, after a proper deadtime that avoids the cross conduction, the low-side is turned on until the low-side current is lower than a valley threshold (1.2 A). Once the low-side is turned off, the high-side is immediately turned on. In this way the frequency is adjusted to keep the inductor current ripple between the current foldback value (2.3 A) and valley threshold (1.2 A), so properly limiting the output current in case of overcurrent or short-circuit.

It should be noted that in some cases, mainly with very low output voltages, the hard overcurrent can cause the FB to find the new equilibrium just over the current foldback threshold (0.2 V). In this case no frequency reduction is enabled, then the inductor current may diverge. This means that the ripple current during the minimum ON-time is higher than the ripple current during the OFF-time (the switching period minus the minimum ON-time), so pulse-by-pulse, the average current is rising, exceeding the current limit.

In order to avoid too high current, a further protection is activated when the high-side current exceeds a further current threshold (OCP2) slightly over the current limit (OCP1). If the current triggers the second threshold, the converter stops switching, the reference of the error amplifier is pulled down and then it restarts with a soft-start procedure. If the overcurrent condition is still active, the current foldback with frequency reduction properly limits the output current to 2.3 A.

14/36

DocID022998 Rev 5

**ST1S31**

**Functional description**

## **4.5 Enable function**

The enable feature allows the device to be put into standby mode. With the EN pin is lower than 0.4 V, the device is disabled and the power consumption is reduced to less than 10 A. With the EN pin higher than 1.2 V, the device is enabled. If the EN pin is left floating, an internal pull-down ensures that the voltage at the pin reaches the inhibit threshold and the device is disabled. The pin is also VIN compatible.

## **4.6 Light load operation**

With peak current mode control loop the output of the error amplifier is proportional to the load current. In the ST1S31 device, to increase light load efficiency when the output of the error amplifier falls below a certain threshold, the high-side turn-on is prevented.

This mechanism reduces the switching frequency at light load in order to save the switching losses.

## **4.7 Hysteretic thermal shutdown**

The thermal shutdown block generates a signal that turns off the power stage if the junction temperature goes above 150 °C. Once the junction temperature goes back to about 130 °C, the device restarts in normal operation.

DocID022998 Rev 5

15/36

**ST1S31**

**Application information**

## **5 Application information**

## **5.1 Input capacitor selection**

The capacitor connected to the input must be capable of supporting the maximum input operating voltage and the maximum RMS input current required by the device. The input capacitor is subject to a pulsed current, the RMS value of which is dissipated over its ESR, affecting the overall system efficiency.

So the input capacitor must have an RMS current rating higher than the maximum RMS input current and an ESR value compliant with the expected efficiency.

The maximum RMS input current flowing through the capacitor can be calculated as:

## **Equation 15**

**==> picture [115 x 28] intentionally omitted <==**

where _Io_ is the maximum DC output current, _D_ is the duty cycle, and is the efficiency. Considering = 1, this function has a maximum at D = 0.5 and is equal to Io/2.

The peak-to-peak voltage across the input capacitor can be calculated as:

## **Equation 16**

**==> picture [222 x 24] intentionally omitted <==**

where _ESR_ is the equivalent series resistance of the capacitor.

Given the physical dimension, ceramic capacitors can well meet the requirements of the input filter sustaining a higher input RMS current than electrolytic / tantalum types. In this case the equation of CIN as a function of the target peak-to-peak voltage ripple (VPP) can be written as follows:

## **Equation 17**

**==> picture [177 x 24] intentionally omitted <==**

neglecting the small ESR of ceramic capacitors.

Considering = 1, this function has its maximum in D = 0.5, therefore, given the maximum peak-to-peak input voltage (VPP_MAX), the minimum input capacitor (CIN_MIN) value is:

## **Equation 18**

**==> picture [118 x 25] intentionally omitted <==**

Typically, CIN is dimensioned to keep the maximum peak-to-peak voltage ripple in the order of 1% of VINMAX.

16/36

DocID022998 Rev 5

**ST1S31**

**Application information**

The placement of the input capacitor is very important to avoid noise injection and voltage spikes on the input voltage pin. So the CIN must be placed as close as possible to the VIN_SW pin. In _Table 6_ some multilayer ceramic capacitors suitable for this device are given.

**Table 6. Input MLCC capacitors**

|**Manufacturer**|**Series**|**Cap value (**µ**F)**|**Rated voltage (V)**|
|---|---|---|---|
|Murata|GRM21|10|10|
|TDK|C3225|10|25|
||C3216|10|16|
|TAIYO YUDEN|LMK212|22|10|



A ceramic bypass capacitor, as close as possible to the VINA pin so that additional parasitic ESR and ESL are minimized, is suggested in order to prevent instability on the output voltage due to noise. The value of the bypass capacitor can go from 330 nF to 1 µF.

## **5.2 Inductor selection**

The inductance value fixes the current ripple flowing through the output capacitor. So the minimum inductance value to have the expected current ripple must be selected. The rule to fix the current ripple value is to have a ripple at 20% - 40% of the output current.

In continuous current mode (CCM), the inductance value can be calculated by _Equation 19_ :

## **Equation 19**

**==> picture [161 x 21] intentionally omitted <==**

where _TON_ is the conduction time of the high-side switch and _TOFF_ is the conduction time of the low-side switch (in CCM, FSW = 1/(TON + TOFF)). The maximum current ripple, given the VOUT, is obtained at maximum TOFF, that is, at minimum duty cycle (see previous section to calculate minimum duty). So by fixing IL = 20% to 30% of the maximum output current, the minimum inductance value can be calculated:

## **Equation 20**

**==> picture [101 x 25] intentionally omitted <==**

where _FSWMIN_ is the minimum switching frequency, according to _Table 4_ . The slope compensation, to prevent the sub-harmonic instability in the peak current control loop, is internally managed and so fixed. This implies a further lower limit for the inductor value. To assure sub-harmonic stability:

## **Equation 21**

**==> picture [91 x 12] intentionally omitted <==**

where _VPP_ is the peak-to-peak value of the slope compensation ramp. The inductor value selected based on _Equation 20_ must satisfy _Equation 21_ . The peak current through the inductor is given by _Equation 22_ :

DocID022998 Rev 5

17/36

**ST1S31**

**Application information**

## **Equation 22**

**==> picture [65 x 21] intentionally omitted <==**

So if the inductor value decreases, the peak current (which must be lower than the current limit of the device) increases. The higher the inductor value, the higher the average output current that can be delivered, without reaching the current limit.

In _Table 7_ some inductor part numbers are listed.

**Table 7. Inductors**

|**Manufacturer**|**Series**|**Inductor value (**µ**H)**|**Saturation current (A)**|
|---|---|---|---|
|Coilcraft|XAL50xx|1.2 to 3.3|6.3 to 9|
||XAL60xx|2.2 to 5.6|7.4 to 11|
||MSS1048|1.0 to 3.8|6.5 to 11|
|Würth|WE-HCI 7030|1.5 to 4.7|7 to 14|
||WE-PD type L|1.5 to 3.5|6.4 to 10|
|Coiltronics|DR73|1.0 to 2.2|5.5 to7.9|
||DR74|1.5 to 3.3|5.4 to 8.35|



## **5.3 Output capacitor selection**

The current in the output capacitor has a triangular waveform which generates a voltage ripple across it. This ripple is due to the capacitive component (charge or discharge of the output capacitor) and the resistive component (due to the voltage drop across its ESR). So the output capacitor must be selected in order to have a voltage ripple compliant with the application requirements.

The amount of the voltage ripple can be calculated starting from the current ripple obtained by the inductor selection.

## **Equation 23**

**==> picture [160 x 24] intentionally omitted <==**

For a ceramic (MLCC) capacitor, the capacitive component of the ripple dominates the resistive one. While for an electrolytic capacitor the opposite is true.

As the compensation network is internal, the output capacitor should be selected in order to have a proper phase margin and then a stable control loop.

The equations of _Section 5.2_ help to check loop stability given the application conditions, the value of the inductor and of the output capacitor.

18/36

DocID022998 Rev 5

**ST1S31**

**Application information**

In _Table 8_ some capacitor series are listed.

**Table 8. Output capacitors**

|**Manufacturer**|**Series**|**Cap value (**µ**F)**|**Rated voltage (V)**|**ESR (m****)**|
|---|---|---|---|---|
|Murata|GRM32|22 to 100|6.3 to 25|< 5|
||GRM31|10 to 47|6.3 to 25|< 5|
|Panasonic|ECJ|10 to 22|6.3|< 5|
||EEFCD|10 to 68|6.3|15 to 55|
|Sanyo|TPA/B/C|100 to 470|4 to 16|40 to 80|
|TDK|C3225|22 to 100|6.3|< 5|



## **5.4 Thermal dissipation**

The thermal design is important to prevent the thermal shutdown of the device if junction temperature goes above 150 °C. The three different sources of losses within the device are:

- a) conduction losses due to the on-resistance of high-side switch (RHS) and low-side switch (RLS); these are equal to:

## **Equation 24**

**==> picture [193 x 14] intentionally omitted <==**

where _D_ is the duty cycle of the application. Note that the duty cycle is theoretically given by the ratio between VOUT and VIN, but it is actually slightly higher to compensate the losses of the regulator.

- b) switching losses due to high-side power MOSFET turn-on and turn-off; these can be calculated as:

## **Equation 25**

**==> picture [270 x 21] intentionally omitted <==**

where _TRISE_ and _TFALL_ are the overlap times of the voltage across the high-side power switch (VDS) and the current flowing into it during the turn-on and turn-off phases, as shown in _Figure 9_ . _TSW_ is the equivalent switching time. For this device the typical value for the equivalent switching time is 20 ns.

- c) Quiescent current losses, calculated as:

## **Equation 26**

**==> picture [56 x 10] intentionally omitted <==**

where _IQ_ is the quiescent current (IQ = 1.2 mA maximum).

DocID022998 Rev 5

19/36

**ST1S31**

**Application information**

The junction temperature TJ can be calculated as:

## **Equation 27**

TJ = TA + RthJA  PTOT

where _TA_ is the ambient temperature and _PTOT_ is the sum of the power losses just seen. _RthJA_ is the equivalent thermal resistance junction to ambient of the device; it can be calculated as the parallel of many paths of heat conduction from the junction to the ambient. For this device the path through the exposed pad is the one conducting the largest amount of heat. The RthJA measured on the demonstration board described in _Section 5.5_ is about 50 °C/W for the VFDFPN and 100 °C/W for the SO8 package.

## **Figure 9. Switching losses**

**==> picture [405 x 228] intentionally omitted <==**

**----- Start of picture text -----**<br>
VIN<br>VSW<br>I<br>SW,HS<br>V<br>DS,HS<br>PSW<br>PCOND,HS PCOND,LS<br>TFALL TRISE<br>AM11422v1<br>**----- End of picture text -----**<br>


## **5.5 Layout consideration**

The PC board layout of the switching DC-DC regulator is very important to minimize the noise injected in high impedance nodes, to reduce interference generated by the high switching current loops and to optimize the reliability of the device.

In order to avoid EMC problems, the high switching current loops must be as short as possible. In the buck converter there are two high switching current loops: during the on-time, the pulsed current flows through the input capacitor, the high-side power switch, the inductor and the output capacitor; during the off-time, through the low-side power switch, the inductor and the output capacitor.

The input capacitor connected to VINSW must be placed as close as possible to the device, to avoid spikes on VINSW due to the stray inductance and the pulsed input current.

In order to prevent dynamic unbalance between VINSW and VINA, the trace connecting the VINA pin to the input must be derived from VINSW.

20/36

DocID022998 Rev 5

**ST1S31**

**Application information**

The feedback pin (FB) connection to the external resistor divider is a high impedance node, so the interference can be minimized by routing the feedback node with a very short trace and as far as possible from the high current paths.

A single point connection from signal ground to power ground is suggested.

Thanks to the exposed pad of the device, the ground plane helps to reduce the thermal resistance junction to ambient; so a large ground plane, soldered to the exposed pad, enhances the thermal performance of the converter allowing high power conversion.

## **Figure 10. PCB layout example**

**==> picture [405 x 301] intentionally omitted <==**

**----- Start of picture text -----**<br>
Via to connect the thermal pad<br>To bottom or inner ground plane Star center for common ground<br>Short high<br>switching<br>current loop<br>Input cap as<br>close as possible<br>to VINSW pin<br>Short FB trace VINA derived from Cin<br>To avoid dynamic voltage drop<br>Between VINA and VINSW<br>AM11423v1<br>**----- End of picture text -----**<br>


DocID022998 Rev 5

21/36

**ST1S31**

**Demonstration board**

## **6 Demonstration board**

## **Figure 11. Demonstration board schematic**

**==> picture [405 x 194] intentionally omitted <==**

**----- Start of picture text -----**<br>
5V<br>VIN<br>U1 ST1S31<br>1 VIN_A PGND 8 L1 3.3V<br>C3 2 EN SW 7 Vout<br>1u 3 6 2.2uH<br>FB VIN_SW<br>4 5 C1 C2<br>AGND ePAD PGOOD 10u 22u<br>R3<br>R1 10k<br>R2 62.5k C4<br>20k<br>NC<br>0 0<br>AM11424v1<br>**----- End of picture text -----**<br>


**Table 9. Component list**

|**Reference**|**Part number**|**Description**|**Manufacturer**|
|---|---|---|---|
|U1|ST1S31||ST|
|L1|DR73 2R2|2.2 µH, Isat = 5.5 A|Coiltronics|
|C1|C3225X7RE106K|10 µF 25 V X7R|TDK|
|C2|C3225X7R1C226M|22 µF 16 V X7R|TDK|
|C3||1 µF 25 V X7R||
|C4||NC||
|R1||62.5 k||
|R2||20 k||
|R3||10 k||



22/36

DocID022998 Rev 5

**ST1S31**

**Demonstration board**

## **Figure 12. Demonstration board PCB top and bottom, DFN package**

**==> picture [462 x 215] intentionally omitted <==**

## **Figure 13. Demonstration board PCB top and bottom, SO8 package**

**==> picture [462 x 214] intentionally omitted <==**

DocID022998 Rev 5

23/36

**ST1S31**

**Typical characteristics**

## **7 Typical characteristics**

## **Figure 14. Efficiency curves: VIN = 3.3 V**

**==> picture [452 x 235] intentionally omitted <==**

## **Figure 15. Efficiency curves: VIN = 3.3 V (log scale)**

**==> picture [450 x 231] intentionally omitted <==**

24/36

DocID022998 Rev 5

**ST1S31**

**Typical characteristics**

## **Figure 16. Load regulation (VIN = 3.3 V)**

**==> picture [448 x 230] intentionally omitted <==**

## **Figure 17. Efficiency curves: VIN = 4.0 V**

**==> picture [440 x 231] intentionally omitted <==**

DocID022998 Rev 5

25/36

**ST1S31**

**Typical characteristics**

## **Figure 18. Efficiency curves: VIN = 4.0 V (log scale)**

**==> picture [442 x 232] intentionally omitted <==**

## **Figure 19. Load regulation (VIN = 4.0 V)**

**==> picture [444 x 264] intentionally omitted <==**

26/36

DocID022998 Rev 5

**ST1S31**

**Typical characteristics**

## **Figure 20. Efficiency curves: VIN = 5.0 V**

**==> picture [450 x 260] intentionally omitted <==**

## **Figure 21. Efficiency curves: VIN = 5.0 V (log scale)**

**==> picture [446 x 259] intentionally omitted <==**

DocID022998 Rev 5

27/36

**ST1S31**

**Typical characteristics**

## **Figure 22. Load regulation (VIN = 5.0 V)**

**==> picture [444 x 258] intentionally omitted <==**

28/36

DocID022998 Rev 5

**ST1S31**

**Typical characteristics**

**==> picture [460 x 189] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 23. Zero load operation Figure 24. Overcurrent protection<br>V IN =5V, V OUT [=1.2V, I] O [=0A] V IN =5.5V , V OUT =1.2V, I LOAD [=0.5A ] -> 4.8A<br>Green: IL (100mA/div) Timescale 2us/div Green: IL (1A/div) Timescale 100us/div<br>Yellow: SW (1V/div) Yellow: SW (1V/div)<br>Red: VOUT  (20mV/div) Red: VOUT (200mV/div)<br>AM11429v1 AM11430v1<br>**----- End of picture text -----**<br>


**==> picture [462 x 199] intentionally omitted <==**

**----- Start of picture text -----**<br>
Figure 25. 100 mA operation Figure 26. Short-circuit protection<br>VIN =5V, VOUT=1.2V, IO=100mA<br>VIN=5.5V, VOUT=1.2V<br>Green: IL (1A/div) Timescale 50us/div<br>Green: IL (100mA/div) Timescale 2us/div Yellow: SW (1V/div)<br>Yellow: SW (1V/div) Red: VOUT (200mV/div)<br>Red: VOUT  (20mV/div) AM11430v1<br>AM11431v1<br>**----- End of picture text -----**<br>


DocID022998 Rev 5

29/36

**ST1S31**

**Package information**

## **8 Package information**

In order to meet environmental requirements, ST offers these devices in different grades of ECOPACK[®] packages, depending on their level of environmental compliance. ECOPACK specifications, grade definitions and product status are available at: _www.st.com_ . ECOPACK is an ST trademark.

30/36

DocID022998 Rev 5

**ST1S31**

**Package information**

## **8.1 VFDFPN 3 x 3 - 8L package information**

**Figure 27. VFDFPN 3 x 3 - 8L package outline**

**==> picture [462 x 370] intentionally omitted <==**

**----- Start of picture text -----**<br>
8057023<br>**----- End of picture text -----**<br>


**Table 10. VFDFPN 3 x 3 - 8L package mechanical data**

|**Symbol**|**Dimensions (mm)**<br>**Min.**<br>**Typ.**<br>**Max.**|**Dimensions (mm)**<br>**Min.**<br>**Typ.**<br>**Max.**|**Dimensions (mm)**<br>**Min.**<br>**Typ.**<br>**Max.**|
|---|---|---|---|
|||**Typ.**|**Max.**|
|A|0.80|0.90|1.00|
|A1|0.0||0.05|
|b|0.25|0.30|0.35|
|D||3.00||
|D2|2.234|2.384|2.484|
|E||3.00||
|E2|1.496|1.646|1.746|
|e||0.65||
|L|0.30|0.40|0.50|



DocID022998 Rev 5

31/36

**ST1S31**

**Package information**

## **Figure 28. VFDFPN 3 x 3 - 8L recommended footprint[(1)]**

**==> picture [405 x 299] intentionally omitted <==**

**----- Start of picture text -----**<br>
�����������<br>**----- End of picture text -----**<br>


**==> picture [97 x 7] intentionally omitted <==**

**----- Start of picture text -----**<br>
1. Dimensions are in mm.<br>**----- End of picture text -----**<br>


32/36

DocID022998 Rev 5

**ST1S31**

**Package information**

## **8.2 SO8 package information**

## **Figure 29. SO8 package outline**

0016023_Rev_E

**Table 11. SO8 package mechanical data**

||**Table 11. SO8package mechanical data**|**Table 11. SO8package mechanical data**|**Table 11. SO8package mechanical data**|
|---|---|---|---|
|**Symbol**|**Dimensions (mm)**<br>**Min.**<br>**Typ.**<br>**Max.**|||
|||**Typ.**|**Max.**|
|A|||1.75|
|A1|0.10||0.25|
|A2|1.25|||
|b|0.28||0.48|
|c|0.17||0.23|
|D|4.80|4.90|5.00|
|E|5.80|6.00|6.20|
|E1|3.80|3.90|4.00|
|e||1.27||
|h|0.25||0.50|
|L|0.40||1.27|
|L1||1.04||
|k|0°||8°|
|ccc|||0.10|



DocID022998 Rev 5

33/36

**Order codes**

**ST1S31**

## **9 Order codes**

**Table 12. Ordering information**

|**Order codes**|**Package**|
|---|---|
|ST1S31PUR|VFDFPN 3 x 3 - 8 L|
|ST1S31D-R|SO8|



34/36

DocID022998 Rev 5

**ST1S31**

**Revision history**

## **10 Revision history**

**Table 13. Document revision history**

|**Date**|**Revision**|**Changes**|
|---|---|---|
|12-Nov-2014|3|Updated Figure 2: Pin connection (top view) on page 3 (replaced<br>by new figure).<br>Minor modifications throughout document.|
|03-Mar-2016|4|Updated value in Table 3 on page 5 and Section 5.4 on page 19<br>(replaced 40 °C/W by 50 °C/W).<br>Added Section 4.1 on page 9.<br>Updated Section 7 on page 24 [added Figure 14 on page 24 to<br>Figure 22 on page 28 (replaced figures 13 and 14)].<br>Minor modifications throughout document.|
|03-Aug-2018|5|Updated_Figure 1: Application circuit_on the cover page.|



DocID022998 Rev 5

35/36

**ST1S31**

## **IMPORTANT NOTICE – PLEASE READ CAREFULLY**

STMicroelectronics NV and its subsidiaries (“ST”) reserve the right to make changes, corrections, enhancements, modifications, and improvements to ST products and/or to this document at any time without notice. Purchasers should obtain the latest relevant information on ST products before placing orders. ST products are sold pursuant to ST’s terms and conditions of sale in place at the time of order acknowledgement.

Purchasers are solely responsible for the choice, selection, and use of ST products and ST assumes no liability for application assistance or the design of Purchasers’ products.

No license, express or implied, to any intellectual property right is granted by ST herein.

Resale of ST products with provisions different from the information set forth herein shall void any warranty granted by ST for such product.

ST and the ST logo are trademarks of ST. All other product or service names are the property of their respective owners.

Information in this document supersedes and replaces information previously supplied in any prior versions of this document.

© 2018 STMicroelectronics – All rights reserved

36/36

DocID022998 Rev 5
