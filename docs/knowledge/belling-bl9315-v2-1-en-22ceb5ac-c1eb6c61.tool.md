**BL9315 5V 3.0A 1.5MHz Synchronous Buck Converter**

**==> picture [129 x 16] intentionally omitted <==**

**==> picture [129 x 17] intentionally omitted <==**

## GENERAL DESCRIPTION

## FEATURES

The BL9315 is a high-efficiency, DC-to-DC step-down switching regulators, capable of delivering up to 3.0A of output current. The BL9315 operates from an input voltage range of 2.5V to 5.5V and provides an output voltage from 0.6V to VIN-0.3V, making the device BL9315 ideal for low voltage power conversions. Running at a fixed frequency of 1.5MHz allows the use of small external components, such as ceramic input and output caps, as well as small inductors, while still providing low output ripples. This low noise output along with its excellent efficiency achieved by the internal synchronous rectifier, making BL9315 an ideal green replacement for large power consuming linear regulators.

Internal soft-start control circuitry reduces inrush current. Short-circuit and thermal-overload protection improves design reliability.

The BL9315 is available in ESOP8 Package.

- High Efficiency: Up to 95%

- Capable of Delivering 3.0A

- 1.5MHz Switching Frequency

- No External Schottky Diode Needed

- Low dropout 100% Duty operation

- Internal Compensation and Soft-Start

- Current Mode control

- 0.6V Reference for Low Output voltages

- Logic Control Shutdown (IQ<1uA)

- Over Voltage Protection

- Short Circuit Protection

- Thermal shutdown and UVLO

- Power good indicator

- Available in ESOP8 package

## APPLICATIONS

- Digital Cameras

- Set top boxes

- Wireless and DSL Modems

- USB supplied Devices in Notebooks

- Portable Devices

## TYPICAL APPLICATION CIRCUIT

**==> picture [6 x 5] intentionally omitted <==**

**----- Start of picture text -----**<br>
L<br>**----- End of picture text -----**<br>


**==> picture [211 x 88] intentionally omitted <==**

## PIN ASSIGNMENT

**==> picture [138 x 106] intentionally omitted <==**

**----- Start of picture text -----**<br>
NC 1 8 NC<br>IN 2 7 EN<br>SW 3 6 NC<br>GND 4 5 FB<br>ESOP8<br>**----- End of picture text -----**<br>


## ORDERING INFORMATION

|**PART NO**|**PACAKGE**|**TEMPERATURE**|**TAPE & REEL**|
|---|---|---|---|
|||||
|BL9315|ESOP8|-40 ~ +85℃|3000/REEL|



Page 1

www.belling.com.cn

V2.1

**BL9315 5V 3.0A 1.5MHz Synchronous Buck Converter**

**==> picture [129 x 16] intentionally omitted <==**

**==> picture [129 x 17] intentionally omitted <==**

## PIN DESCRIPTION

|**PIN No**|**SYMBOL**|**DESCRIPTTION**|
|---|---|---|
||||
|1|NC|No Connection.|
|2|IN|Power Supply Input Pin. Must be closely decoupled to GND with at least 10uF ceramic cap.|
|3|SW|Inductor connection. Connect an inductor between SW and the regulator output.|
|4|GND|Ground|
|5|FB|Feedback input. Connect an external resistor divider from the output to FB and GND to set<br>the output to a voltage between 0.6V and Vin|
|6|NC|No Connection.|
|7|EN|Enablepin for the IC. Drive thepin to high to enable thepart, and low to disable|
|8|NC|No Connection.|



## ABSOLUTE MAXIMUM RATINGS(Note 2)

|**Parameter**|**Parameter**|**Value**|
|---|---|---|
||||
|Max Input Voltage||6.0V|
|Max OperatingJunction Temperature(Tj)||150C|
|Ambient  Temperature(Ta)||-40C – 85C|
|Maximum Power Dissipation|ESOP8|1W|
|Storage  Temperature(Ts)||-40C - 150C|
|Lead Temperature & Time||260C,10S|



**Note2** : Absolute Maximum Ratings are threshold limit values that must not be exceeded even for an instant under any condition. Moreover, such values for any two items must not be reached simultaneously. Operation above these absolute maximum ratings may cause degradation or permanent damage to the device. These are stress ratings only and do not necessarily imply functional operation below these limits.

## RECOMMANDED OPERATING RANGE

|**SYMBOL**|**ITEMS**|**VALUE**|**UNIT**|
|---|---|---|---|
|VIN|VINSupplyVoltage|2.5 to 5.5|V|
|TOPT|OperatingTemperature|-40 to +85|C|



Page 2

www.belling.com.cn

V2.1

**BL9315**

**5V 3.0A 1.5MHz Synchronous Buck Converter**

**==> picture [129 x 16] intentionally omitted <==**

**==> picture [129 x 17] intentionally omitted <==**

## ELECTRICAL CHARACTERISTICS

VIN=5.0V,  L=2.2μH,  TA=25C,  unless  otherwise specified.

|**Symbol**|**Parameter**|**Conditions**|**Min**|**Typ**|**Max**|**Unit**|
|---|---|---|---|---|---|---|
| | | | | | | |
|VIN|Input Voltage Range||2.5||5.5|V|
|VFB|Feedback Voltage||0.588|0.6|0.612|V|
|IFB|Feedback Leakage current|||0.1|0.4|uA|
|IQ|Quiescent Current|Active, VFB=0.65V||40||uA|
|ISD|Shutdown Current|Shutdown|||1|uA|
|FSW|SwitchingFrequency|||1.5||MHz|
|RONP|PMOSFET RDSON|||100||mΩ|
|RONN|NMOSFET RDSON|||75||mΩ|
|VUVLO|UVLO Threshold||||2.5|V|
|V1HYS|UVLO Hysteresis|||0.15||V|
|ILIMIT|Peak Current Limit||5.0|||A|
|ISW|SW Leakage Current|VIN=5.5V, VSW=0 or<br>5.5V,VEN=0V|||1|uA|
|VOVP|Over Voltage Threshold|||6.15||V|
|V2HYS|OVP Hysteresis|||0.3||V|
|IEN|EN Leakage Current||||1|uA|
|ISOFT|Soft Start Time|||350||uS|
|VENH|EN Input High Voltage||1.5|||V|
|VENL|EN Input Low Voltage||||0.4|V|
|TSD|Thermal Shutdown|||150||C|
|THSD|Hysteresis Thermal Shutdown|||20||C|
|Rdis|Discharge resistor|||150||Ω|



Page 3

www.belling.com.cn

V2.1

**BL9315**

**5V 3.0A 1.5MHz Synchronous Buck Converter**

**==> picture [128 x 18] intentionally omitted <==**

**==> picture [128 x 16] intentionally omitted <==**

## TYPICAL OPERATING CHARACTERISTICS

CIN=22μF, COUT=22μF, L=2.2μH, Ce=22pF, Tested under TA=25C, unless otherwise specified Efficiency vs. Load Current Efficiency vs. Load Current

**==> picture [229 x 139] intentionally omitted <==**

**==> picture [229 x 138] intentionally omitted <==**

Efficiency vs. Load Current

**==> picture [230 x 143] intentionally omitted <==**

**==> picture [149 x 10] intentionally omitted <==**

**----- Start of picture text -----**<br>
Output Voltage vs. Output Current<br>**----- End of picture text -----**<br>


**==> picture [236 x 142] intentionally omitted <==**

Operation Temperature vs. Output Voltage

**==> picture [229 x 138] intentionally omitted <==**

**==> picture [233 x 151] intentionally omitted <==**

**----- Start of picture text -----**<br>
Input Voltage vs. Output Voltage<br>**----- End of picture text -----**<br>


Start Up Waveform

**==> picture [229 x 145] intentionally omitted <==**

Shutdown Waveform

**==> picture [239 x 145] intentionally omitted <==**

Page 4

www.belling.com.cn

V2.1

**BL9315 5V 3.0A 1.5MHz Synchronous Buck Converter**

**==> picture [128 x 18] intentionally omitted <==**

**==> picture [128 x 16] intentionally omitted <==**

## OPERATION DESCRIPTION

The BL9315 high-efficiency switching regulator is a small, simple, DC-to-DC step-down converter capable of delivering up to 3.0A of output current. The device operates in pulse-width modulation (PWM) at 1.5MHz from a 2.5V to 5.5V input voltage and provides an output voltage from 0.6V to VIN-0.3V, making the BL9315 ideal for on-board post-regulation applications. An internal synchronous rectifier improves efficiency and eliminates the typical Schottky free-wheeling diode. Using the on resistance of the internal high-side MOSFET to sense switching currents eliminates current-sense resistors, further improving efficiency and cost.

## _**Loop Operation**_

BL9315 uses a PWM current-mode control scheme. An open-loop comparator compares the integrated voltage-feedback signal against the sum of the amplified current-sense signal and the slope compensation ramp. At each rising edge of the internal clock, the internal high-side MOSFET turns on until the PWM comparator terminates the on cycle. During this on-time, current ramps up through the inductor, sourcing current to the output and storing energy in the inductor. The current mode feedback system regulates the peak inductor current as a function of the output voltage error signal. During the off cycle, the internal high-side P-channel MOSFET turns off, and the internal low-side N-channel MOSFET turns on. The inductor releases the stored energy as its current ramps down while still providing current to the output.

## _**Current Sense**_

An internal current-sense amplifier senses the current through the high-side MOSFET during on time and produces a proportional current signal, which is used to sum with the slope compensation signal. The summed signal then is compared with the error amplifier output by the PWM comparator to terminate the on cycle.

## _**Short Protection**_

After the soft start is over, if the output voltage falls below 40% of the regulation level, the BL9315 will turn off both power switches, entering the short circuit protection. It will remain in this state until IN or EN voltage is recycled.

## _**Current Limit**_

There is a cycle-by-cycle current limit on the high-side MOSFET of 5.0A (min). When the current flowing out of SW exceeds this limit, the high-side MOSFET turns off and the synchronous rectifier turns on. BL9315 utilizes a frequency fold-back mode to prevent overheating during short-circuit output conditions. The device enters frequency fold-back mode when the FB voltage drops below 200mV, limiting the current to 5.0A and reducing power dissipation. Normal operation resumes upon removal of the short-circuit condition.

## _**Soft-start**_

BL9315 has a internal soft-start circuitry to reduce supply inrush current during startup conditions. When the device exits under-voltage lockout (UVLO), shutdown mode, or restarts following a thermaloverload event, the soft-start circuitry slowly ramps up current available at SW.

## _**UVLO and Thermal Shutdown**_

If IN drops below 2.5V, the UVLO circuit inhibits switching. Once IN rises above 2.5V, the UVLO clears, and the soft-start sequence activates. Thermaloverload protection limits total power dissipation in the device. When the junction temperature exceeds TJ= +150°C, a thermal sensor forces the device into shutdown, allowing the die to cool. The thermal sensor turns the device on again after the junction temperature cools by 15°C, resulting in a pulsed output during continuous overload conditions. Following a thermal-shutdown condition, the soft-start sequence begins.

## _**Output OVP**_

If the output voltage exceeds 120% of the regulation level for more than 20us, the BL9315 will turn off both power switches and turn on the discharge switch, entering over-voltage protection. I will remain in this state until IN and EN voltage is recycled.

## _**Power Good**_

Power good flag is pulled down when AS6360 startup and the FB pin voltage is still outside pre-set voltage window. During normal operation phase, when FB pin voltage drop under 90% or increase over 110%, power good flag is also pulled down.

Page 5

www.belling.com.cn

V2.1

**BL9315**

**5V 3.0A 1.5MHz Synchronous Buck Converter**

**==> picture [129 x 17] intentionally omitted <==**

**==> picture [129 x 17] intentionally omitted <==**

## APPLICATION INFORMATION

## _**Setting Output Voltages**_

Output voltages are set by external resistors. The FB threshold voltage (VFB) is 0.6V. R1= R2*[(VOUT / 0.6) - 1]

Set R2 to 100K, then R1 can be easily derived from the above equation.

**==> picture [106 x 101] intentionally omitted <==**

**----- Start of picture text -----**<br>
VOUT<br>R1<br>FB<br>R2<br>GND<br>**----- End of picture text -----**<br>


## _**Output Capacitor COUT:**_

The output capacitor is selected to handle the output ripple noise requirements, both steady state ripple and transient requirements must be taken into consideration when selecting this capacitor. For the best performance, it is recommended to use X5R or better grade ceramic capacitor with 6.3V rating and greater than 22uF capacitance.

## _**Input Capacitor Selection**_

This ripple current through input capacitor is calculated as:

**==> picture [106 x 19] intentionally omitted <==**

This formula has a maximum at VIN=2VOUT condition, where ICIN_RMS=IOUT/2. This simple worst-case condition is commonly used for the DC-DC design. With the maximum load current at 2.5A, A typical X5R or better grade ceramic capacitor with 6.3V rating and

more than 1pcs 22uF capacitor can handle this ripple current well. To minimize the potential noise problem, ceramics ceramic capacitor should really be placed close to IN and GND pins. Care should be taken to minimize the loop area formed by CIN and IN/GND pins.

## _**Output Capacitor COUT:**_

   - There are several considerations in choosing this induction.

- 1) Choose the inductance to provide the desired ripple current. It is suggested to choose the ripple to be about 40% of maximum output current. The inductance is calculated as:

**==> picture [123 x 32] intentionally omitted <==**

Where FSW is the switching frequency and IOUT.MAX is the maximum load current.

The BL9315 regulator IC is quite tolerant of different ripple current amplitude. Consequently, the final choice of inductance can be slightly off the calculation value without significantly impacting the performance.

- 2) The saturation current rating of the inductor must be selected to be greater than the peak inductor current under full load conditions.

**==> picture [169 x 27] intentionally omitted <==**

- 3) The DCR of inductor and the ore loss at the switching frequency must be low enough to achieve the desired efficiency requirement. It is desirable to choose an inductor with DCR<50mΩ to achieve a good overall efficiency.

## _**Layout Consideration**_

Layout is critical to achieve clean and stable operation. The switching power stage requires particular attention. Follow these guidelines for good PC board layout:

- 1) Place decoupling capacitors as close to the IC as possible

- 2) Connect input and output capacitors to the same power ground node with a star ground configuration then to IC ground.

- 3) Keep the high-current paths as short and wide as possible. Keep the path of switching current (CIN to IN and CIN to GND) short. Avoid vias in the switching paths.

- 4) If possible, connect IN, SW, and GND separately to a large copper area to help cool the IC to further improve efficiency and long-term reliability.

- 5) Ensure all feedback connections are short and direct. Place the feedback resistors as close to the IC as possible.

- 6) Route high-speed switching nodes away from sensitive analog area.

Page 6

www.belling.com.cn

V2.1

**BL9315**

**5V 3.0A 1.5MHz Synchronous Buck Converter**

**==> picture [129 x 17] intentionally omitted <==**

**==> picture [129 x 17] intentionally omitted <==**

## PACKAGE OUTLINE

## **ESOP8**

**==> picture [485 x 346] intentionally omitted <==**

Page 7

www.belling.com.cn

V2.1
