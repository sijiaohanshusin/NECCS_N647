<!-- page: 1 -->
BL9315
5V 3.0A 1.5MHz Synchronous Buck Converter

GENERAL DESCRIPTION FEATURES

The BL9315 is a high-efficiency, DC-to-DC step-down - High Efficiency: Up to 95%

switching regulators, capable of delivering up to 3.0A of - Capable of Delivering 3.0A

output current. The BL9315 operates from an input - 1.5MHz Switching Frequency

voltage range of 2.5V to 5.5V and provides an output - No External Schottky Diode Needed

voltage from 0.6V to VIN-0.3V, making the device - Low dropout 100% Duty operation

BL9315 ideal for low voltage power conversions. - Internal Compensation and Soft-Start

Running at a fixed frequency of 1.5MHz allows the use - Current Mode control

of small external components, such as ceramic input and - 0.6V Reference for Low Output voltages

output caps, as well as small inductors, while still - Logic Control Shutdown (IQ<1uA)

providing low output ripples. This low noise output along - Over Voltage Protection

with its excellent efficiency achieved by the internal - Short Circuit Protection

synchronous rectifier, making BL9315 an ideal green - Thermal shutdown and UVLO

replacement for large power consuming linear regulators. - Power good indicator

- Available in ESOP8 package
Internal soft-start control circuitry reduces inrush current.

Short-circuit and thermal-overload protection improves APPLICATIONS
design reliability.
- Digital Cameras

The BL9315 is available in ESOP8 Package. - Set top boxes

- Wireless and DSL Modems

- USB supplied Devices in Notebooks

- Portable Devices

TYPICAL APPLICATION CIRCUIT PIN ASSIGNMENT

L
NC 1 8 NC

IN 2 7 EN

SW 3 6 NC

GND 4 5 FB

ESOP8

ORDERING INFORMATION

PART NO PACAKGE TEMPERATURE TAPE & REEL

BL9315 ESOP8 -40 ~ +85℃ 3000/REEL

www.belling.com.cn Page 1 V2.1

<!-- page: 2 -->
BL9315
5V 3.0A 1.5MHz Synchronous Buck Converter

PIN DESCRIPTION

PIN No SYMBOL DESCRIPTTION

1 NC No Connection.
2 IN Power Supply Input Pin. Must be closely decoupled to GND with at least 10uF ceramic cap.

3 SW Inductor connection. Connect an inductor between SW and the regulator output.

4 GND Ground

FB Feedback input. Connect an external resistor divider from the output to FB and GND to set
5
the output to a voltage between 0.6V and Vin

6 NC No Connection.

7 EN Enable pin for the IC. Drive the pin to high to enable the part, and low to disable

8 NC No Connection.

ABSOLUTE MAXIMUM RATINGS(Note 2)

Parameter Value

Max Input Voltage 6.0V
Max Operating Junction Temperature(Tj) 150C
Ambient Temperature(Ta) -40C – 85C
Maximum Power Dissipation ESOP8 1W
Storage Temperature(Ts) -40C - 150C
Lead Temperature & Time 260C, 10S

Note2: Absolute Maximum Ratings are threshold limit values that must not be exceeded even for an instant under any
condition. Moreover, such values for any two items must not be reached simultaneously. Operation above these absolute
maximum ratings may cause degradation or permanent damage to the device. These are stress ratings only and do not
necessarily imply functional operation below these limits.

RECOMMANDED OPERATING RANGE
SYMBOL ITEMS VALUE UNIT
VIN VIN Supply Voltage 2.5 to 5.5 V
TOPT Operating Temperature -40 to +85 C

www.belling.com.cn Page 2 V2.1

<!-- page: 3 -->
BL9315
5V 3.0A 1.5MHz Synchronous Buck Converter

ELECTRICAL CHARACTERISTICS
VIN=5.0V, L=2.2μH, TA=25C, unless otherwise specified.

Symbol Parameter Conditions Min Typ Max Unit

VIN Input Voltage Range 2.5 5.5 V

VFB Feedback Voltage 0.588 0.6 0.612 V
IFB Feedback Leakage current 0.1 0.4 uA
IQ Quiescent Current Active, VFB=0.65V 40 uA
ISD Shutdown Current Shutdown 1 uA
FSW Switching Frequency 1.5 MHz
RONP PMOSFET RDSON 100 mΩ
RONN NMOSFET RDSON 75 mΩ
VUVLO UVLO Threshold 2.5 V
V1HYS UVLO Hysteresis 0.15 V
ILIMIT Peak Current Limit 5.0 A
VIN=5.5V, VSW=0 or
ISW SW Leakage Current 1 uA
5.5V, VEN=0V
VOVP Over Voltage Threshold 6.15 V

V2HYS OVP Hysteresis 0.3 V

IEN EN Leakage Current 1 uA
ISOFT Soft Start Time 350 uS
VENH EN Input High Voltage 1.5 V
VENL EN Input Low Voltage 0.4 V
TSD Thermal Shutdown 150 C
THSD Hysteresis Thermal Shutdown 20 C
Rdis Discharge resistor 150 Ω

www.belling.com.cn Page 3 V2.1

<!-- page: 4 -->
## Visual Summary (Page 4)

- page_class: chart
- confidence: 0.92
- reason_codes: heuristic_page_type_chart, heuristic_content_type_chart, visual_data_dominant, policy_visual_summary_override

Page contains typical operating characteristics charts for the BL9315 5V 3.0A synchronous buck converter, including efficiency vs load current (multiple input voltages), output voltage vs output current, operation temperature curves, and start-up/shutdown waveforms.

<!-- page: 5 -->
BL9315
5V 3.0A 1.5MHz Synchronous Buck Converter

OPERATION DESCRIPTION
The BL9315 high-efficiency switching regulator is a Short Protection
small, simple, DC-to-DC step-down converter capable After the soft start is over, if the output voltage falls
of delivering up to 3.0A of output current. The device below 40% of the regulation level, the BL9315 will
operates in pulse-width modulation (PWM) at 1.5MHz turn off both power switches, entering the short circuit
from a 2.5V to 5.5V input voltage and provides an protection. It will remain in this state until IN or EN
output voltage from 0.6V to VIN-0.3V, making the voltage is recycled.
BL9315 ideal for on-board post-regulation
applications. An internal synchronous rectifier Current Limit
improves efficiency and eliminates the typical There is a cycle-by-cycle current limit on the high-side
Schottky free-wheeling diode. Using the on resistance MOSFET of 5.0A (min). When the current flowing out
of the internal high-side MOSFET to sense switching of SW exceeds this limit, the high-side MOSFET turns
currents eliminates current-sense resistors, further off and the synchronous rectifier turns on. BL9315
improving efficiency and cost. utilizes a frequency fold-back mode to prevent
overheating during short-circuit output conditions.
Loop Operation The device enters frequency fold-back mode when the
BL9315 uses a PWM current-mode control scheme. FB voltage drops below 200mV, limiting the current to
An open-loop comparator compares the integrated 5.0A and reducing power dissipation. Normal
voltage-feedback signal against the sum of the operation resumes upon removal of the short-circuit
amplified current-sense signal and the slope condition.
compensation ramp. At each rising edge of the internal
clock, the internal high-side MOSFET turns on until Soft-start
the PWM comparator terminates the on cycle. During BL9315 has a internal soft-start circuitry to reduce
this on-time, current ramps up through the inductor, supply inrush current during startup conditions. When
sourcing current to the output and storing energy in the the device exits under-voltage lockout (UVLO),
inductor. The current mode feedback system regulates shutdown mode, or restarts following a thermal-
the peak inductor current as a function of the output overload event, the soft-start circuitry slowly ramps up
voltage error signal. During the off cycle, the internal current available at SW.
high-side P-channel MOSFET turns off, and the
internal low-side N-channel MOSFET turns on. The UVLO and Thermal Shutdown
inductor releases the stored energy as its current ramps If IN drops below 2.5V, the UVLO circuit inhibits
down while still providing current to the output. switching. Once IN rises above 2.5V, the UVLO clears,
and the soft-start sequence activates. Thermal-
Current Sense overload protection limits total power dissipation in
An internal current-sense amplifier senses the current the device. When the junction temperature exceeds
through the high-side MOSFET during on time and TJ= +150°C, a thermal sensor forces the device into
produces a proportional current signal, which is used shutdown, allowing the die to cool. The thermal sensor
to sum with the slope compensation signal. The turns the device on again after the junction
summed signal then is compared with the error temperature cools by 15°C, resulting in a pulsed output
amplifier output by the PWM comparator to terminate during continuous overload conditions. Following a
the on cycle. thermal-shutdown condition, the soft-start sequence
begins.
Output OVP
If the output voltage exceeds 120% of the regulation Power Good
level for more than 20us, the BL9315 will turn off Power good flag is pulled down when AS6360 start-
both power switches and turn on the discharge switch, up and the FB pin voltage is still outside pre-set
entering over-voltage protection. I will remain in this voltage window. During normal operation phase,
state until IN and EN voltage is recycled. when FB pin voltage drop under 90% or increase over
110%, power good flag is also pulled down.

www.belling.com.cn Page 5 V2.1

<!-- page: 6 -->
BL9315
5V 3.0A 1.5MHz Synchronous Buck Converter

APPLICATION INFORMATION
Setting Output Voltages more than 1pcs 22uF capacitor can handle this ripple
Output voltages are set by external resistors. The FB current well. To minimize the potential noise problem,
threshold voltage (VFB) is 0.6V. ceramics ceramic capacitor should really be placed
R1= R2*[(VOUT / 0.6) - 1] close to IN and GND pins. Care should be taken to
Set R2 to 100K, then R1 can be easily derived from minimize the loop area formed by CIN and IN/GND
the above equation. pins.

VOUT
Output Capacitor COUT:
There are several considerations in choosing this
R1 induction.
1) Choose the inductance to provide the desired ripple
FB
current. It is suggested to choose the ripple to be
R2 about 40% of maximum output current. The
inductance is calculated as:
GND

Output Capacitor COUT:
Where FSW is the switching frequency and IOUT.MAX
The output capacitor is selected to handle the output
is the maximum load current.
ripple noise requirements, both steady state ripple and
The BL9315 regulator IC is quite tolerant of
transient requirements must be taken into consideration different ripple current amplitude. Consequently,
when selecting this capacitor. For the best performance, the final choice of inductance can be slightly off the
it is recommended to use X5R or better grade ceramic calculation value without significantly impacting
capacitor with 6.3V rating and greater than 22uF
the performance.
capacitance.
2) The saturation current rating of the inductor must be
selected to be greater than the peak inductor current
Input Capacitor Selection under full load conditions.
This ripple current through input capacitor is calculated
as:

3) The DCR of inductor and the ore loss at the
This formula has a maximum at VIN=2VOUT condition, switching frequency must be low enough to achieve
where ICIN_RMS=IOUT/2. This simple worst-case condition the desired efficiency requirement. It is desirable to
is commonly used for the DC-DC design. choose an inductor with DCR<50mΩ to achieve a
With the maximum load current at 2.5A, A typical X5R good overall efficiency.
or better grade ceramic capacitor with 6.3V rating and

Layout Consideration
Layout is critical to achieve clean and stable operation. The switching power stage requires particular attention. Follow

these guidelines for good PC board layout:
1) Place decoupling capacitors as close to the IC as 4) If possible, connect IN, SW, and GND separately to
possible a large copper area to help cool the IC to further
2) Connect input and output capacitors to the same improve efficiency and long-term reliability.
power ground node with a star ground configuration 5) Ensure all feedback connections are short and direct.
then to IC ground. Place the feedback resistors as close to the IC as
3) Keep the high-current paths as short and wide as possible.
possible. Keep the path of switching current (CIN 6) Route high-speed switching nodes away from
to IN and CIN to GND) short. Avoid vias in the sensitive analog area.
switching paths.

www.belling.com.cn Page 6 V2.1

<!-- page: 7 -->
## Visual Summary (Page 7)

- page_class: block_diagram
- confidence: 0.85
- reason_codes: visual_content_dominant, diagram_structure_required, policy_visual_summary_override

ESOP8 package outline diagram showing top and bottom views with pin locations labeled.
