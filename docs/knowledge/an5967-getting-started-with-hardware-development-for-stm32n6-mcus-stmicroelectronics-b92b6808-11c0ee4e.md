<!-- page: 1 -->
AN5967

Application note

Getting started with hardware development for STM32N6 MCUs

Introduction

This document demonstrates how to use the STM32N6 MCUs and details the minimum hardware resources required to develop
an application. It provides an overview of the development board hardware implementation, focusing on features such as power
supply, package selection, clock management, reset control, boot mode settings, and debug management.

This document also includes reference design schematics, describing the main peripherals, interfaces, and power modes. It
may refer to STM32N6 microcontrollers as STM32N657xx and STM32N647xx product lines.

AN5967 - Rev 1 - November 2024 www.st.com
For further information contact your local STMicroelectronics sales office.

<!-- page: 2 -->
AN5967
General information

1 General information

This document applies to the STM32N6 Arm®Cortex®‑M55‑based microcontrollers.
Note: Arm is a registered trademark of Arm Limited (or its subsidiaries) in the US and/or elsewhere.

Reference documents

The table below contains the main reference documents.

Table 1. Reference documents

Document
Document type Document title
reference

Arm® Cortex®‑M55, ST Neural-ART Accelerator, H264 encoder, Neo-Chrom 2.5D GPU, Datasheet DS14791
4.2 Mbyte-contiguous SRAM (STM32N6 datasheet)

Errata sheet ES0620 STM32N6 errata sheet

Reference
RM0486 STM32N6 advanced Arm®-based 32-bit MCUs manual

Programming
PM0273 STM32 Cortex®-M55 MCUs programming manual manual

User manual UM3234 How to proceed with boot ROM on STM32N6 MCUs

Application note AN5946 How to optimize low-power modes on STM32N6 MCUs

Application note AN2867 Guidelines for oscillator design on STM8AF/AL/S and STM32 MCUs/MPUs

Application note AN1709 EMC design guide for STM8, STM32, and legacy MCUs

Application note AN6000 How to build the discrete power supply for STM32N6 MCUs

AN5967 - Rev 1 page 2/36

<!-- page: 3 -->
AN5967
Terminology

2 Terminology

The table below defines the acronyms and other terms mentioned in this application note.

Table 2. Acronyms

Acronym Definition

ADC Analog-to-digital converter

AHB Advanced high-performance bus

AXI Advanced extensible interface bus; by extension, the interconnect matrix based on AXI

BKPSRAM Backup SRAM

BSEC Boot and security controller (OTP interface)

CM55 Cortex®-M55 processor

CSI Camera serial interface

CSS Clock security system

DCMI Digital camera interface (parallel interface)

DLYBSD Delay block for SDMMC; compensates external signal timings to reach the highest data rates

Direct memory access; bus master that autonomously transfers data between peripheral and
DMA
memory or between memories

EMC Electromagnetic compatibility

ETH Ethernet controller

ESD Electrostatic discharge

EMI Electromagnetic interference

EXTI Extended interrupt and event controller

FDCAN Controller area network with flexible data-rate; can also support time-triggered CAN

FMC Flexible memory controller

GPIO General-purpose input/output

GPU Graphic processing unit

HDMI High-definition multimedia interface

HDP Hardware debug port

HSE High-speed external crystal oscillator

HSI High-speed internal oscillator

I2C Inter-IC bus

I3C Improved I2C

I2S Inter-IC sound

IWDG Independent watchdog

JTAG Joint test action group (debug interface)

LCD Liquid crystal display

LPTIM Low-power timer

LSE Low-speed external crystal oscillator

LSI Low-speed internal oscillator

MLCC Multi-layer ceramic capacitor

MSI Multispeed internal oscillator

XSPI Hexadata serial peripheral interface

AN5967 - Rev 1 page 3/36

<!-- page: 4 -->
AN5967
Terminology

Acronym Definition

XSPIM XSPI IO manager

LTDC LDC TFT display controller

NVIC Nested vector interrupt controller

OTP One-time programmable memory

PCB Printed circuit board

PCI PCI interface

PHY Mixed-signal physical interface; adapts the internal logical level to a specific interface standard

PWR Power control

RCC Reset and clock control

RETRAM Retention SRAM

RNG Random number generator

ROM Read-only memory

RTC Real-time clock

SAI Serial audio interface

SDMMC Secure digital and multimedia card interface; supports SD, MMC, eMMC, and SDIO protocols

SMPS Switched-mode power supply

SPDIF Sony/Philips digital interface format

SPI Serial peripheral interface

SRAM Static random access memory

SW Software

SWD Serial wire debug

SWO Single wire output; trace port

SYSCFG System configuration

TAMP Tamper detection IP

TIM Timer

UART Universal asynchronous receiver transmitter

UCPD USB Type-C® power delivery controller

USART Universal synchronous/asynchronous receiver/transmitter

USB Universal serial bus

USB HS USB Hi-Speed USB 2.0 at 480 Mbit/s half-duplex

USBPHYC USB physical interface control

VREFBUF ADC voltage reference buffer

WWDG Window watchdog

AN5967 - Rev 1 page 4/36

<!-- page: 5 -->
AN5967
Power supplies

3 Power supplies

The device is powered by multiple power supplies that must be connected to external decoupling capacitors. The
system always requires a power supply on VDD and VDDA18AON.
Depending on the SMPS configuration mode, VDDA18PMU and VDDSMPS must be provided to start VDDCORE and
to allow all other independent supplies to power up (VDDA18ADC, VDDA18CSI, VDDA18PLL, VDDA18USB, VBAT,
VDD33USB, VDDIO2, VDDIO3, VDDIO4, and VDDIO5).

Figure 1. Power supply overview

VDDIO4 VDDIO5 VDDIO2 VDDIO3 VDDA18USB VDD33USB VDDCSI VDDA18CSI

PC[1] PC[0] Port 1 Port 2
USB HS UCPD CSI PC[12:6] PC[5:2] PO[5:0] PN[12:0] PHYs I/Os PHY PH[2,9] PE[4] PP[15:0]
I/Os I/Os
XSPIM I/Os VSS VSS VSS

VSS VSS VSS
Core domain (VCORE)
VDDCORE
VSS

I/O (CPU, system logic,
I/Os logic EXTI, peripherals, RAM)

VDD
VDDA18PMU
Retention domain
VDDSMPS
Step-down ITCM VLXSMPS
converter DTCM VFBSMPS

VSSSMPS
ITCM FLEX
VDD

LSI, WKUP, VRET RET I/O
IWDG, BSEC,
I/Os logic
RIFSC

VSS
VDDA18AON OTP, HSE, HSI, MSI VSS

VDDA18PLL PLLs Backup domain
Power
V08CAP switch

VDD VSW Backup VBKP
VBAT regulator

Power switch

Backup
BKUP I/O LSE, RTC, RAM
I/Os logic TAMP, backup
registers, reset

VSS
VDDA18ADC Analog domain VSS

VREFBUF ADCs
VREF+ VREF+
VREF- VREF-

VSSA

DT70448V3

3.1 Main power supplies
The main power supplies are the following:
- VDD: external power supply for I/Os (can be 1.8 V or 3.3 V typical)
- VDDA18AON: external power supply for analog blocks, such as reset, power management, and oscillators,
and OTP memory (always needed ON) – 1.8 V typical

AN5967 - Rev 1 page 5/36

<!-- page: 6 -->
AN5967
Power supplies

Table 3. Main power supplies

Symbol Parameter Operating conditions Min. Typ. Max. Unit

VDDA18AON Internal analog supply voltage 1.71 1.8 1.935 V

1.8 V range 1.62 1.8 1.98 V
VDD I/Os supply voltage
3.3 V range 3 3.3 3.6 V

The VDDCORE is the third power supply to run functional hardware on STM32N6 devices.
VDDCORE is generated by SMPS, internally or externally. The power supplies are:
- VDDSMPS: external power supply for the SMPS step-down converter.

Note: This power supply must be tied to VSS when the SMPS is not used.
- VDDA18PMU: external analog power supply for the SMPS step-down converter - 1.8 V typical. This power
supply can be connected to VDDSMPS through an inductor-based filter.

Note: This power supply must be tied to VSS when the SMPS is not used.
- VLXSMPS: step-down converter supply output
- VFBSMPS: step-down converter sense feedback
- VSSSMPS: separate step-down converter ground
- VDDCORE: digital core domain supply - 0.8 V typical; dependent on VDD supply. VDD must be present before
VDDCORE.

Note: VDDCSI must be connected to VDDCORE as 0.8 V typical power.

3.2 Secondary power supplies
STM32N6 devices have the following secondary power supplies:
- VBAT: optional external power supply for backup domain when VDD is not present (VBAT mode). When no
battery is used, this power supply must be connected to VDD.
- VDDIO2: external power supply for 22 I/Os (PO[5:0] and PP[15:0]); independent from any other power
supply; dedicated to powering the XSPIM1 16-bit interface.
- VDDIO3: external power supply for 13 I/Os (PN[12:0]); independent from any other supply; dedicated to the
XSPIM2 8-bit interface.
- VDDIO4: external power supply for 10 I/Os (PC[1], PC[12:6], and PH[9,2]); independent from any other
supply; dedicated to the SDMMC1(eMMC) interface.
- VDDIO5: external power supply for six I/Os (PC[0], PC[5:2], and PE[4]); independent from any other supply;
dedicated to the SDMMC2 (SD-Card) interface.
- USB power supply:
– VDD33USB: external power supply for USB2 HS PHYs and USB Type-C® (CC1 and CC2 pins);
independent from any other supply - 3.3 V typical.
– VDDA18USB: external analog power supply for USB HS PHYs - 1.8 V typical.
- VDDA18CSI: external analog power supply for CSI D-PHY - 1.8 V typical.
- VDDCSI: CSI PHY digital supply input - 0.8 V typical.
- VDDA18PLL: external analog power supplies for PLLs - 1.8 V typical.
- VDDA18ADC: external analog power supply for ADCs and voltage reference buffers; independent from any
other supplies (1.8 V only).

Note: The ADC interface is 1.8 V only.
- VREF+: external reference voltage for ADCs, independent from any other supplies.
– When the voltage reference buffer is enabled, the internal voltage reference buffer delivers VREF+
and VREF-.
– When the voltage reference buffer is disabled, VREF+ is delivered by an independent external
reference supply.

AN5967 - Rev 1 page 6/36

<!-- page: 7 -->
AN5967
Power supplies

- VSS: common ground for all supplies except the step-down converter and analog peripherals.
- VSSA: separate analog and reference voltage ground; must be connected externally to the same supply
ground as VSS (1.8 V only).

3.3 Independent ADC supply and reference voltage
To improve the conversion accuracy and dynamic range, the ADC and reference have an independent power
supply that can be filtered separately, and shielded from noise on the PCB.
The analog operating voltage supply (VDDA18ADC) is 1.8 V typical.
- The ADC/VREFBUF voltage supply input is available on a separate VDDA18ADC pin.
- External VREF: the user can connect a separate external reference voltage ADC input on VREF+. The
voltage on VREF+ may range from 1.10 V to VDDA18ADC.
- Internal VREF: the user can enable an internal reference voltage on VREF+ in the VREFBUF block. The
voltage on VREF+ can be selected from either 1.21 V or 1.5 V. With internal VREF available on the VREF+
pin, it can be used externally (for example, for analog comparator references), if loading is kept within
datasheet values.

3.4 USB supplies
The USB supplies must be connected to the same source with independent decoupling whenever possible.
- VDD33USB: USB Hi-Speed PHY supply (3.3 V typical)
- VDDA18USB: external analog power supply for USB HS PHYs (1.8 V typical)

Note: All ground pins (VSS, VSSA) must be connected to power planes. The following table must be used as a
guideline only. Real count and values of capacitors can be adapted. This is dependent on various parameters,
such as capacitor size and dielectric, PCB technology, and product power integrity simulations.

Decoupling power figures

The table below does not include capacitors on supply sources (such as LDO or SMPS) or external devices (such
as DDR memory, SD-Card, eMMC, or flash memories).

Table 4. Power supply pins

Ground reference
Supply pin Decoupling and filtering Comments
pin

VDD VSS n x 100 nF (MLCC, 10V, X5R, ±10%) -

VBAT VSS 100 nF (MLCC, 10V, X5R, ±10%) No decoupling cap if VBAT connected to VDD

VDD33USB VSS 1 µF (MLCC, 6.3 V, X5R, ±20%) -

VDDA18PMU VSSAPMU 100 nF (MLCC, 10V, X5R, ±10%) VSSAPMU must be connected to VSS plane

2 x 10 µF (MLCC, 4V, X5R, ±20%)

VDDSMPS VSSSMPS + 2 x 1 µF (MLCC, 6.3V, X5R, ±20%) VSSSMPS must be connected to VSS plane

+ 2 x 100 nF (MLCC, 10V, X5R, ±10%)

1 µH (DFE201612P-1R0M)

VLXSMPS VSSSMPS + 2.2 nF (MLCC, 50V, X7R, ±10%) VSSSMPS must be connected to VSS plane

+ 4 x 22 µF (MLCC, 6V3, X5R, ±20%)

V08CAP VSS 4.7 µF (MLCC, 6V3, X5R, ±20%) Internal backup regulator output pin

VDDIO2 VSS n x 100 nF (MLCC, 10V, X5R, ±10%) -

VDDIO3 VSS n x 100 nF (MLCC, 10V, X5R, ±10%) -

VDDIO4 VSS n x 100 nF (MLCC, 10V, X5R, ±10%) -

VDDIO5 VSS n x 100 nF (MLCC, 10V, X5R, ±10%) -
VDDA18CSI VSS 100 nF (MLCC, 10V, X5R, +/‑10%) -

VDD18USB VSS 100 nF (MLCC, 10V, X5R, ±10%) -

AN5967 - Rev 1 page 7/36

<!-- page: 8 -->
AN5967
Power supplies

Ground reference
Supply pin Decoupling and filtering Comments
pin

VDDA18ADC VSSA 100 nF ( MLCC, 10V, X5R, ±10%) VSSA must be connected to VSS plane

VDDA18AON VSSAON 100 nF ( MLCC, 10V, X5R, ±10%) VSSAON must be connected to VSS plane

VDDA18PLL VSS 100 nF (MLCC, 10V, X5R, ±10%) -

1 µF (MLCC, 6.3V, X5R, ±20%)
VREFP VSSA VSSA must be connected to VSS plane
+ 100 nF (MLCC, 10V, X5R, ±10%)

VDDCORE VSS n x 1 µF (MLCC, 6.3 V, X5R, ±20%) -

VDDCSI VSS 1 µF (MLCC, 6.3V, X5R, ±20%) Usually connected to VDDCORE

For further details on package features, select Section 4: Package selection.

3.5 Specific IO constraints related to voltage settings
VDD, VDDIO2, VDDIO3, VDDIO4, and VDDIO5 must respect specific register settings and control sequences when
used at 3/3.3 V or 1.8 V typical.
For further information, refer to the PWR section in the reference manual RM0486.

3.6 System startup
The startup system is different from other STM32 MCU architectures that need only VDD/VDDA18AON to start up.
Multiple voltage supplies must be applied with a specific sequence to start the device.
Figure 2 and Figure 3 demonstrate the system startup sequence from power-on in different supply and SMPS
configurations.

AN5967 - Rev 1 page 8/36

<!-- page: 9 -->
AN5967
Power supplies

VDDCore supply from SMPS step-down converter (internal SMPS configuration)

Figure 2. Device startup with VCORE supplied directly from SMPS step-down converter

VDD
POR threshold

VDDA18AON
POR threshold

pwr_por_rstn

PWR_ON

VDDA18PMU / vdda18pmu_ok threshold
VDDSMPS

Vdda18pmu_ok
VOS low
VFBSMPS

VOS low vddcore_ok threshold
VCORE

tempo

Vcore_ok

Operating mode Power Reset Wait Hardware system init Run
down oscillator

ck_sys

Direct
Supply configuration Default configuration SD
supply

SDEN

(1) (2) (3) (4) (5) DT70450V2

AN5967 - Rev 1 page 9/36

<!-- page: 10 -->
AN5967
Power supplies

1. When the system is powered on, the POR monitors VDD and VDDA18AON supplies. Once the supplies are
above the POR threshold level, the external voltage regulator providing VDDA18PMU and VDDSMPS supplies is
enabled via the PWR_ON signal.
2. The SMPS step-down converter is kept in reset as long as VDDA18PMU and VDDSMPS are not stable.
3. Once VDDA18PMU and VDDSMPS supplies are above the vdda18pmu_ok threshold level, the SMPS step-down
converter is taken out of reset, and the output level is set by default at 0.8 V (VOS low). The system is kept in
reset mode as long as VDDCORE is stable.
4. Once VDDCORE supply surpasses the vddcore_ok threshold level, the system is taken out of reset, and the
HSI oscillator is enabled.
5. Once the oscillator is stable, the system is initialized: option bytes are loaded, and the CPU starts in Run
mode.

VDDCore supply in bypass mode (external SMPS configuration - SMPS off)

Figure 3. Device startup with VCORE supplied from an external regulator

VDD

POR threshold

VDDA18AON

POR threshold

pwr_por_rstn

PWR_ON

Vddcore_ok threshold
VDDCORE

tempo

Vcore_ok

Reset Run Operating mode Powerdown Wait Hardware system oscillator init

ck_sys

Supply configuration Default configuration Bypass mode

SDEN

(1) (2) (3) (4) DT70451V1

The devices featuring the SMPS can also be used in bypass mode.
1. When the system is powered on, the POR monitors VDD and VDDA18AON supplies. Once the supplies are
above the POR threshold level, the external voltage regulator providing the VDDCORE supply is enabled via the
PWR_ON signal.
2. The system is kept in reset mode as long as VDDCORE is not stable.

AN5967 - Rev 1 page 10/36

<!-- page: 11 -->
AN5967
Power supplies

3. Once VDDCORE supply surpasses the vddcore_ok threshold level, the system is taken out of reset, and the
HSI oscillator is enabled.
4. Once the oscillator is stable, the system is initialized: option bytes are loaded and the CPU starts in run mode.
The software must disable SMPS bit clearing SDEN in PWR_CR1 as soon as possible.

3.7 SMPS configuration
The VDDCORE domain is supported. The configuration is controlled through the SDEN bit in the PWR_CR1
register.

Figure 4. System supply configurations

VDDA18PMU VDDA18PMU

VDD18 VDDSMPS VDDSMPS

VLXSMPS SMPS VLXSMPS SMPS
(on) (off)
VFBSMPS VFBSMPS

VSSSMPS VSSSMPS

VDDCORE
VCORE External supply VDDCORE
VCORE

VSS VSS

SMPS supply External supply (bypass) DT70449V2

AN5967 - Rev 1 page 11/36

<!-- page: 12 -->
AN5967
Package selection

4 Package selection

The package must be selected by considering the application requirements.
The list below summarizes the most frequent constraints:
- Number of interfaces required. Some interfaces are unavailable on some packages. Some interface
combinations are unavailable on some packages. Refer to the product datasheet for details.
- PCB technology constraints: small pitch and high ball density may require more PCB layers and higher
PCB classes, requiring stack-up with microvia (laser via) technology.
- Package height.
- PCB available area.
- Thermal constraints (larger packages have better thermal dissipation capabilities).

Table 5. Package summary

Size (mm)(1)
Characteristic
14x14 10x10 10x10 12x12 8x8 6x6

Pitch (mm) 0.8 0.5 0.65 0.8 0.5 0.4

Thickness N/A N/A N/A N/A N/A N/A

Number of IOs 165 144 126 106 90 75

Sales number VFBGA264 VFBGA223 VFBGA198 VFBGA178 VFBGA142 VFBGA169

STM32N657xx X0 L0 B0 I0 Z0 A0

STM32N647xx X0 L0 B0 I0 Z0 A0

1. Typical body size

Note: To obtain up-to-date reference availability, refer to the product datasheet.

AN5967 - Rev 1 page 12/36

<!-- page: 13 -->
AN5967
Alternate function mapping to pins

5 Alternate function mapping to pins

To explore peripheral alternate function mapping to pins easily when using the STM32N6 MCUs, use the
STM32CubeN6 tool, available on www.st.com.

Note: The screenshot below is an example only, not specific to STM32N6 MCUs. The look and feel may differ from
future CubeMX versions.

Figure 5. STM32CubeMX example screenshot

DT71878V2

AN5967 - Rev 1 page 13/36

<!-- page: 14 -->
AN5967
Clocks

6 Clocks

Different clock sources can be used to drive the subsystem clocks:
- Internal clock signal:
– HSI oscillator clock (high-speed internal clock signal): 64 MHz typical
– MSI oscillator clock (multispeed internal clock signal): 4 MHz typical
- External clock signal:
– HSE oscillator clock (high-speed external clock signal): 48 MHz typical
The devices have two secondary clock sources:
- LSI: 32 kHz low-speed internal RC, driving the independent watchdog and, optionally, the RTC used for
autowake-up from the Stop and Standby modes.
- LSE (LSE crystal): 32.768 kHz low-speed external crystal, optionally driving the real-time clock (RTCCLK).

The RCC authorizes the access of four PLL clocks with the same features. A typical allocation is:
- PLL1: clock to the CPU, buses, and AXI storage areas
- PLL2: clock to NPU and NPU storage areas
- PLL3: clock to CACHEAXI RAM and Ethernet
- PLL4: clock to display, camera module, FDCAN, and other peripherals
Each clock source can be switched on or off independently when it is not used, to optimize power consumption.
For the description of the clock tree and the details about the possible clock frequencies, refer to the product
reference manual and datasheet.

AN5967 - Rev 1 page 14/36

<!-- page: 15 -->
AN5967
HSE oscillator

7 HSE oscillator

The high-speed external clock signal (HSE) can be generated from two possible clock sources:
- HSE user external clock
- HSE external crystal

Figure 6. HSE source clock

Digital external clock Analog external clock Crystal/ceramic resonator configuration

OSC_IN OSC_OUT OSC_IN OSC_OUT VDD OSC_IN OSC_OUT

R1
R1

External digital clock External analog clock
CL1 CL2
source source Load
capacitors
DT70470V2

The possible configurations are:
- OSC_OUT is tied to GND (max 1 kΩ): HSE digital bypass.
- OSC_OUT is tied to VDDA18AON (max 1 kΩ): HSE analog bypass.
- OSC_OUT high-Z or connected to a crystal: HSE crystal mode.
When the bypass is used, PWR_ON can enable the external clock generator to save power (that is, disabled in
Standby). In that case, the OSC_IN clock input must be stable within 10 ms after the PWR_ON rising edge
occurs.

7.1 External source - HSE bypass mode
In this mode, an external clock source must be provided. It can have a frequency ranging from 8 to 48 MHz. For
the actual max value, refer to the product datasheet.
The external digital (VIL/VIH) or analog clock signal, with a duty cycle of about 50%, must drive the OSC_IN pin.

7.2 External crystal - HSE crystal mode
The external oscillator frequency ranges from 8 to 48 MHz. It has the advantage of producing a very accurate rate
on the main clock.
Using a 40-MHz crystal frequency is a good choice to get accurate USB and PCIe® high‑speed clocks. The
oscillator mode is enabled by clearing the HSEBYP bit and setting the HSEON bit.
The crystal and the load capacitors must be connected as close as possible to the oscillator pins to minimize
output distortion and startup stabilization time. The load capacitance values must be adjusted according to the
selected crystal.
For CL1 and CL2 it is recommended to use NP0/C0G capacitors, selected to meet the load requirements of the
crystal. CL1 and CL2 usually have the same value. The crystal manufacturer typically specifies a load
capacitance, which is the series combination of CL1 and CL2. The PCB and pin capacitances must be included
when sizing CL1 and CL2.
For further information, refer to the electrical characteristics and to the application note AN2867.

AN5967 - Rev 1 page 15/36

<!-- page: 16 -->
AN5967
LSE oscillator

8 LSE oscillator

The low-speed external clock signal (LSE) can be generated from two possible clock sources:
- The LSE user external clock:

Figure 7. LSE external clock

External clock configuration
LSE
VSW
OSC32_IN OSC32_OUT
VSS

External clock OSC32_OUT:
source - High-Z (open) for LSE (digital or analog) bypass DT75958V1

- The LSE external crystal/ceramic resonator:

Figure 8. LSE crystal resonators

Crystal configuration
LSE
OSC32_IN OSC32_OUT

CL1 Load CL2
capacitors DT75959V1

8.1 External source - LSE bypass mode
In this mode, an external clock source must be provided. It can have a frequency of up to 40 kHz.
The external digital (VIL/VIH) or analog clock signal, with a duty cycle of approximately 50%, must drive the
OSC32_IN pin, while the OSC32_OUT pin must be left high-Z.
The configuration of the bypass mode and the selection between digital and analog is done using the RCC
registers (RCC_CR register).
For further details, refer to the reference manual RM0486.

8.2 External crystal - LSE crystal mode
The LSE crystal is a 32.768 kHz low-speed external crystal. It has the advantage of providing a low-power, but
highly accurate, clock source to the real-time clock peripheral (RTC) for clock, calendar, or other timing functions.
The resonator and the load capacitors must be connected as close as possible to the oscillator pins to minimize
output distortion and startup stabilization time. The load capacitance values CL1 and CL2 must be adjusted
according to the selected oscillator.
It is recommended to use a medium-high or high drive on the LSE oscillator.
For further details, refer to the electrical characteristics in the product datasheet and to the application note
AN2867.

AN5967 - Rev 1 page 16/36

<!-- page: 17 -->
AN5967
Clock security system

9 Clock security system

The clock security system (CSS) detects failures of the LSE and HSE oscillators.
For further details, refer to the reference manual RM0486.

9.1 CSS on HSE
The clock security system can be activated by software. In this case, the clock detector is enabled after the HSE
oscillator startup delay, and disabled when this oscillator is stopped.
If the CSS detects a failure on the HSE oscillator clock, it can generate an application reset.

9.2 CSS on LSE
The clock security system can be activated by software. In this case, the clock detector is enabled after the LSE
oscillator startup delay, and disabled when this oscillator is stopped.
If the CSS detects a failure on the LSE oscillator clock, it stops the RTC/TAMP clock source and signals the
failure to the TAMP block for security protection and system wake-up.

AN5967 - Rev 1 page 17/36

<!-- page: 18 -->
AN5967
Boot configuration

10 Boot configuration

The BOOT0 and BOOT1 pins and one OTP word determine the boot mode on STM32N6 devices.
The BOOT1 is a nondedicated boot pin. If flash memory boot is selected, the flash memory device selection is
done using OTP fuses. If serial boot is selected, interfaces can be disabled by fuse.

Note: - The BOOT1 pin check has priority on the BOOT0 pin check.
- If the BOOT1 pin is not set, BOOT0 is checked.
- If the BOOT1 pin is selected but not allowed in the current life cycle, the BOOT0 pin is checked.

Table 6. Boot source description

BOOT0 pin BOOT1 pin Boot source Interface OTP11 word value

- 1 Development boot - -

XSPI serial NOR (in default, SPI, and
OTP11 [8:5] = 0x0011
single)

XSPI HyperFlash™ (8-bit) OTP11 [8:5] = 0x0101

eMMC SDMMC1 OTP11 [8:5] = 0x0010
0 0 Flash memory boot eMMC™ SDMMC2 OTP11 [8:5] = 0x1000

SD-Card SDMMC1 (up to SD
OTP11 [8:5] = 0x0001
standard v6.0)

SD-Card SDMMC2 (up to SD
OTP11 [8:5] = 0x0111
standard v6.0)

OTP11 [16:9] = 0x1111111x x= 0 USB
USB boot: USB 2.0 OTG_HS
enable, x=1 USB disable
1 0 Serial boot
OTP11 [16:9] = 0x111111x1 x= 0
UART boot (default mode)
UART enable, x=1 UART disable

Note: Default modes correspond to no fused configuration on the OTP11 word.
BOOT0 = 0: OTP11 [8:5] = 0x0000
BOOT0 = 1: OTP11 [16:9] = 0x0000

For further details, refer to the user manual UM3234.

AN5967 - Rev 1 page 18/36

<!-- page: 19 -->
AN5967
Debug management

11 Debug management

The host/target interface is the hardware equipment that connects the host to the application board. It consists of
three components:
- A hardware debug tool
- A JTAG or SWD connector
- A cable connecting the host to the debug tool

11.1 SWJ debug port (serial wire and JTAG)
The core of the STM32N6 MCUs integrates the serial wire/JTAG debug port (SWJ-DP). It is an Arm® standard
CoreSight™ debug port, combining a JTAG-DP (5-pin) interface and an SW-DP (2-pin) interface.
- The JTAG debug port (JTAG-DP) provides a 5-pin standard JTAG interface to the AHP-AP port.
- The serial wire debug port (SW-DP) provides a 2-pin (clock + data) interface to the AHB-AP port.
The two pins of the SW-DP are multiplexed with two of the five JTAG pins of the JTAG-DP.

11.2 Debug LED
The PG10 (AF11) pin has specific BOOTFAILED behavior:
- During the boot phase, in the case of boot failure, the BOOTFAILED (PG10) pin is set to low open-drain,
meaning the debug LED lights up brightly. In most cases, if secure boot is disabled, this fail is invisible as it
immediately falls back to a UART/USB boot.
- During the UART/USB boot, the PG10 pin toggles open-drain at a rate of a few hertz until a connection
starts; the debug LED blinks fast.
- In development boot mode, the PG10 pin is set to low open-drain, meaning the debug LED lights up
brightly.
- In all other cases, the PG10 pin is kept in its reset value (high-Z until further software configuration).
The recommendation is to use this debug pin. The LED connection quickly shows the system activity, making
PG10 perfect for quick low-level boot error signaling. In most cases, the LED circuitry does not conflict with usage
for other purposes.
For further details, refer to the user manual UM3234.

AN5967 - Rev 1 page 19/36

<!-- page: 20 -->
AN5967
Recommendations

12 Recommendations

12.1 PCB
For technical reasons, it is mandatory to use a multilayer PCB:
- A separate layer is dedicated to the ground (VSS).
- Another layer is dedicated to power supplies like VDD, VDDCPU, and VDDCORE.
This provides good decoupling and a good shielding effect.

12.2 Component position
A preliminary layout of the PCB must separate the different circuits according to their EMI contribution. This
reduces the cross‑coupling on the PCB (noisy, high-current circuits, low-voltage circuits, and digital components).

12.3 Ground and power supplies (VSSx, VDDx)
Due to the large power and high frequencies involved in STM32N6 devices, it is mandatory to use the PCB with at
least four layers, and with dedicated power planes for VSSx and VDDx.

12.4 I/O speed settings
It is important to set the right output drive on IOs to have sufficient rise and fall time, but also to avoid additional
ringing and noise. When there are no specific requirements for IO speed, it is mandatory to set GPIO_IOSPEEDR
to 0.
As a first approximation, the following drawings and tables can be used to select the correct settings quickly,
according to signal frequency and capacitive load. This setting might need to be tailored in the case of a signal
integrity issue. Whenever a GPIO_OSPEEDR value of 2 or 3 is used, related IO compensation must be enabled
in SYSCFG.
There are five independent IO compensations for each of the five independent IO supplies: VDD, VDDIO2, VDDIO3,
VDDIO4, and VDDIO5.
For further details, refer to the product datasheet and reference manual.
There are five independent IO voltage sections (VDD, VDDIO2, VDDIO3, DDIO4, and VDDIO5), which, in some
AFMUX setting cases, can be shared between different interfaces.
When VDD, VDDIO2, VDDIO3, VDDIO4, or VDDIO5 work at 1.8 V, settings must be applied in
PWR_SVMCR1.VDDIO4VRSEL (for VDDIO4), PWR_SVMCR2.VDDIO5VRSEL (for VDDIO5), and
PWR_SVMCR3.VDDIOxVRSEL (for VDD, VDDIO2, and VDDIO3). Without these settings, the IOs work in degraded
mode.

Note: To avoid IO damage due to inappropriate settings, in addition to PWR settings, OTP bits (HSLV_VDDIOx) must
be programmed when a specific domain (VDD, VDDIO1, VDDIO2, VDDIO3, or VDDIO3) is used below 2.5 V on a
product. For further details, refer to the related sections in the reference manual RM0486.

12.5 ESD/EMI protections
Electrostatic discharge (ESD) and electromagnetic interference (EMI) protections must be considered from the
start of product development, as it can be very complex and expensive to add them later.
ESD and EMI are driven by global standards (such as IEC 61000 and JESD 22), which require certification in
most countries, to allow mandatory marking to be applied on a product (such as CE and FCC). They are also
driven by standardized interface certification or requirements (for example, USB).
Although the STM32N6 MCUs embed device-level ESD protection, external components must manage the final
product protection, especially on interfaces with external user access in the final product (such as Ethernet, USB,
and SD-Card). Some components provide ESD protection, as well as EMI common-mode filtering. For example,
ECMF02-2AMX6 used on USB.
Section 13: Reference design examples provides examples of ESD/EMI protections. For further details, refer to
the application note AN1709.

AN5967 - Rev 1 page 20/36

<!-- page: 21 -->
AN5967
Recommendations

12.6 Sensitive signals
When designing an application, the EMC (electromagnetic compatibility) performance can be improved by closely
observing the following:
- Signals for which a temporary disturbance affects the running process permanently (such as interrupts and
handshaking strobe signals, not the case for LED commands). For these signals, a surrounding ground
trace, shorter lengths, and the absence of noisy or sensitive traces nearby (the crosstalk effect) improve
the EMC performance. For digital signals, the best possible electrical margin must be reached for the two
logical states and slow Schmitt triggers are recommended to eliminate parasitic states.
- Noisy signals (such as clocks).
- Sensitive signals (such as high-Z ones).
For more information, refer to the application note AN1709.

12.7 Unused I/Os and features
The STM32N6 MCUs are designed for a wide range of applications. Often a particular application does not use
100% of the resources.
To increase the EMC performance, unused clocks, counters, or I/Os must not be left free. For example, I/Os must
be set to "0" or "1" (external or internal pull-up or pull-down to the unused I/O pins) and unused features must be
"frozen" or disabled.

AN5967 - Rev 1 page 21/36

<!-- page: 22 -->
AN5967
Reference design examples

13 Reference design examples

This section provides examples to help the user connect major and critical interfaces to the STM32N6 MCUs.

13.1 Clock
STM32N6 MCUs use two clock sources:
- LSE: 32.768 kHz crystal for the embedded RTC
- HSE: 8 to 48 MHz (crystal or external oscillator) as the main clock
Refer to Section 6: Clocks for more details.

Table 7. HSE BOM for oscillator or crystal example for NX2016SA - 40 MHz

Oscillator Crystal
-
(OSC_OUT = logic 0) (OSC_OUT = crystal pin)

X1 NZ2016SH 40 MHz NX2016SA 40 MHz

R1 10 Ω - (open)

R2 10 kΩ/30 KΩ (1) - (open)

R3 - (open)/33 kΩ(1) 0 Ω

R4 1 kΩ - (open)

C1 - (open) 6.8 pF

C2 - (open) 6.8 pF

C3 10 nF - (open)

1. For respectively VDD = 3.3 V and VDD = 1.8 V. In case of VDD = 3.3 V, a resistor divider formed by R2/(R3+R4) is required
as the oscillator pin 1 (Enable) must be limited to a VDDA18AON (1.8 V) voltage, which supplies the external oscillator.

13.2 SD card

Note: As boot is always done in "Standard" mode (3-V IOs), if the card is used by the application in UHS-I, a power
cycle on the card supply is required after a Reset or Standby mode. NRSTC1MS can be used for this.
Good signal integrity is dependent on the board, GPIO strength settings (GPIO_OSPEEDR registers), and VDD
voltage.

When using VDDIO1 = 1.8 V, a setting of VDDIOxVRSEL can be required to ensure the adequate speed on pins
used on SDMMC1 outputs. If needed, the impedance matching resistor must be placed as close as possible to
the output driver pin. Values in the example below work in most cases, but can be tailored to IO drive strengths
and PCB impedance.
Before the VCC_SDCARD shutdown (for example, before entering Standby mode), the SDMMC1 driver must set all
signals going to the card to 0 or high-Z. The example is independent of MPU I/O voltage VDD and relies on
variable VDDIO1 that can be set either to 3.0/3.3 V, or 1.8 V typical using one of the following:
- SDVSEL1 (0 or high-Z = 3/3.3 V (default), 1 = 1.8 V) connected to an external regulator or other
component managing the VDDIO1 voltage
- A regular GPIO output connected to an external regulator or other component managing the VDDIO1
voltage
- An I²C bus when used with PMIC
If no programmable VDDIO_SDCARD is available on the platform, VDDIO1 can be connected to VCC_SDCARD. In this
case, UHS-I is not supported.

AN5967 - Rev 1 page 22/36

<!-- page: 23 -->
AN5967
Reference design examples

Figure 9. SD-Card with embedded level shifter connection

VDDIO_SDCARD VDDIO_SDCARD VCC_SDCARD
STM32N6x VDDIO3 3V/3.3V typ. during initialization 3V/3.3V typ. µSD Card
VDDIO3 1.8V typ. for UHS-I mode VDD Socket
RPU RPU 10k
PC12 SDMMC1_CK 22 CLK
PH2 SDMMC1_CMD
CMD
PC8 SDMMC1_D0 MSD02N16EMIF06- DAT0
SDMMC1 PC9 SDMMC1_D1 (CLK <52MHz)or
2 x USBLC6- DAT1
PC10 SDMMC1_D2 4SC6
DAT2
PC10 SDMMC1_D3
DAT3

SDVSEL1
GPIOx / SYSCFG optional
Push-pull Bold and plain lines: default pins and minimum set of signals required by low level Boot ROM during SD Card Boot. DT71888V1

Note: When switching to UHS-I mode (VDDIO1 = 1.8 V), VDDIOxVRSEL must be set only when VDDIO1 is within the
1.8 V allowed range. In case of a reset of the SD-Card to the legacy 3/3.3V range and to avoid damage to the
I/Os, VDDIOxVRSEL must be cleared before the voltage is outside the 1.8 V allowed range.

13.3 USB
Multiple USB options are possible. Examples are listed below:
- 1 × Hi-Speed USB device (Figure 1 or Figure 2)
- 1 × Hi-Speed USB device (Figure 1 or Figure 2) + 1 × USB Hi-Speed host (Figure 10. USB hi-speed host
example)
- 1 × SuperSpeed USB host (refer to the note below).
- 1 × SuperSpeed USB dual-role
- 1 × SuperSpeed USB dual-role + 1 × USB Hi-Speed host (Figure 10)
The case of multiple Hi-Speed USB hosts using an external USB hub component is not described here.

Note: In the case of on-board flash memory programming using the STM32CubeProgrammer, at least one USB with
device capabilities is required. This is achieved with Figure 1, Figure 2, or Figure 1 with constraints.

Table 8. USB high-speed PCB routing recommendations

Recommendation Min Typ Max Unit

Differential impedance 76.5 90 103.5 Ω

Single-ended impedance 38.25 45 51.75 Ω

-50 - +50 mils
Length matching within a pair (including package)
-1.27 - +1.27 mm

- - 8 inches
Max trace length (up to connector or first active component)
- - 203 mm

Max number of vias (recommended value) - - 2 -

Distance between any differential trace and other signals S-2S S-3S or more (1)

Do not route over a power plane split. No stubs (point to point only). No right angles

1. Definition can be found in the DDR memory routing guidelines.

13.3.1 USB hi-speed host with Type-A connector (USBH)
A 200 Ω 1% resistor should be connected between USBH_HS_TXRTUNE and VSS.

AN5967 - Rev 1 page 23/36

<!-- page: 24 -->
## Visual Summary (Page 24)

- page_class: timing_diagram
- confidence: 0.92
- reason_codes: timing_diagram_detected, schematic_content, mixed_layout

Page contains two primary block diagrams: Figure 10 illustrates a USB hi-speed host example with STM32N6x and STMPS2151 components, while Figure 11 shows a 10/100M Ethernet PHY connection using LAN742A. Both figures include signal traces, component pinouts, and impedance specifications.

<!-- page: 25 -->
AN5967
Reference design examples

Note: 1. ETH1 is either ETH1 direct or ETHSW port2 (ETHSW is not available on some part numbers).
2. ETH2 is not available on some part numbers.
3. ETH3 is ETHSW port1. ETHSW is not available on some part numbers.
4. Decoupling capacitors are not shown.
As RCC cannot provide the 25 MHz reference clock to the PHY in low-power modes, the dedicated 25 MHz
crystal is required on the PHY in case a "wake-up on LAN" (WOL) is needed for the platform.
Setting RCC PLLs to get 25 MHz output for PHY clocking can constrain other RCC frequencies. In this case, it is
more flexible to put a dedicated 25 MHz crystal on the PHY.

Alternatively, if PHY allows it and if the RCC can provide a precise 50 MHz clock (to be checked with respect to
HSE quartz frequency and RCC, other peripheral, or core clock frequency settings), the STM32N6 devices can
provide a 50 MHz ETH_CLK to the PHY, and REF_CLK is left unconnected on both sides. This saves BOM and
area, as well as power on some PHYs.

Figure 12. 10/100M Ethernet PHY connection (with REFCLK from RCC)

VDD
9 VDDIO VDD1A 193.3V
LAN8742A VDD2A 1 10/100M
Ethernet Transceiver VDDCR 6

STM32N6x
VDD VDD

ETHx_CLK (50 MHz Reference)
RCC 22 5 XTAL1/CLKIN VDD
Pull-up should
be set by clock path enabled in SYSCFG SW open 4 XTAL2

ETHx_RMII_CRS_DV
11 CRS_DV/MODE2
ETHx_RMII_RXD0
8 RXD0/MODE0
ETHx_RMII_RXD1
7 RXD1/MODE1 LED1/nINT/nPME/REGOFF 3 10k Green
open 10 RXER/PHYAD0 3.3V 270

Yellow
LED2/nINT/nPME 2 optional 270
ETH1 ETHx_RMII_TX_EN /nINTSEL
22 16 TXEN
ETHx_RMII_TXD0
22 17 TXD0
ETHx_RMII_TXD1
22 18 TXD1 3.3V
49.9 49.9 49.9 49.9 3.3V
RXN
22 VDD ETHx_MDC
1k5
22 MDC RXP
23 USBLC6- 10/100 VDD PHYAD[2:0] = 0b000 13
ETHx_MDIO 1k5 4SC6 RJ45 connector with MDIO TXN
Magnetics 12 20
ETHx_PHY_INTN optional
14 nINT/REFCLKO TXP 21 and LEDs
Pxx GPIOxx
GPIO
1%
Pxx 100Ω differential traces WKUPx 10k 15 nRST exposed thermal pad RBIAS 24 12k1
PWR DT71891V1

Note: 1. ETH1 is either ETH1 direct or ETHSW port2 (ETHSW is not available on some part numbers).
2. ETH2 is not available on some part numbers.
3. ETH3 is ETHSW port1. ETHSW is not available on some part numbers.
4. Decoupling capacitors are not shown.
As the RCC cannot provide the 50 MHz reference clock to the PHY in low-power modes, this option is not
possible in case a a "wake-up on LAN" (WOL) is needed for the platform.
Setting RCC PLLs to get 50 MHz output for PHY clocking can constrain other RCC frequencies. In that case,
this option is not possible.

AN5967 - Rev 1 page 25/36

<!-- page: 26 -->
AN5967
Reference design examples

Table 9. ETH RMII pins

Pin name Signal direction(1) ETH1(2) ETH2(3) ETH3(3)(4) Comments

Optional 25 MHz or 50 MHz
ETHx_CLK → PF3, PF5, PF8 PF4, PG3 -(5)
reference(6)

Optional if 50 MHz provided by
ETHx_RMII_REF_CLK ← PA14 PC0, PF6 PA5
ETHx_CLK

ETHx_RMII_CRS_DV ← PA11 PC3, PF8 PA2 -

ETHx_RMII_RXD0 ← PF1 PG0 PA9 -

ETHx_RMII_RXD1 ← PC2 PC12 PA10 -

ETHx_RMII_TX_EN → PA13 PC4 PA3 -

ETHx_RMII_TXD0 → PA15 PC7 PA6 -

ETHx_RMII_TXD1 → PC1 PC8 PA7 -

ETHx_MDC → PA9, PF0, PF4 PC6, PG4, PH10 -(7) -

→
ETHx_MDIO PA10, PF2, PF5 PC5, PF9, PH11 - -
←

ETHx_PHY_INTN ← PA12, PC6, PF5 PF5, PG3 PA1 Optional

1. → MPU to PHY, ← PHY to MPU.
2. Can also be used as ETHSW port2. ETHSW is not available on some part numbers.
3. Not available on some part numbers.
4. Equivalent to ETHSW port1.
5. If needed, ETH1_CLK must be used.
6. As RCC cannot provide the reference clock to the PHY in low-power modes, a dedicated 25 MHz crystal is required on the
PHY if "wake-up on LAN" (WOL) is needed for the platform.
7. ETH3 PHY share THE same MDC/MDIO pins as ETH1 PHY (need to use different address for the PHY).

13.4.2 Gigabit Ethernet

Note: Good signal integrity is dependent on the board, GPIO strength settings (GPIO_OSPEEDR registers), and VDD
voltage.

When using VDD = 1.8 V, setting VDDIOxRSEL can be required to ensure the adequate speed on pins used on
ETHx outputs. If needed, the impedance matching resistors must be placed as close as possible to the output
driver pin. The values in the example below work in most cases, but can be tailored to the I/O drive strengths and
PCB impedance of each side.

AN5967 - Rev 1 page 26/36

<!-- page: 27 -->
AN5967
Reference design examples

Figure 13. Gigabit Ethernet PHY connection with VDD = 3.3 V (RTL8211F)

3.3V 3.3V
28 DVDD_RG AVDD3V3 11, 40
3.3V
VDD = 3.3V ± 5% DVDD3V3 29 (PHY 3.3V can be shut down during STANDBY) RTL8211F(I)-CG
optional if 10/100/1000M
RCC
STM32N6x VDD provides a Ethernet Transceiver REG_OUT 30 AVDD10 (1.0V) 25 MHz VDD
clock AVDD10 3
ETHx_CLK (25 MHz Reference)
RCC or VDD 22 37 XTAL_OUT (EXT_CLK) AVDD10 8, 38 25MHz should 125 MHz Pull-up
be set by SW clock path enabled in SYSCFG 36 XTAL_IN DVDD10 21
or ETHx_RGMII_CLK125 optional if RCC 125 MHz is available 22 35 CLKOUT
ETHx_RGMII_RX_CL
22 (PHY_AD1) RXC PHY_AD1=0 K 27 Green 4k7 CFG_LDO1 = 0 4k7 ETHx_RGMII_RX_CT
22 510 (PHY_AD2) (CFG_LDO1) RXCTL PHY_AD2=0 RXDLY=1 L 26 LED2 34 VDD 4k7 or ETHx_RGMII_RXD0 4k7
22 (RXDLY) RXD0 RXDLY=0 25 TXDLY=1 VDD 3.3V 4k7 4k7 or ETHx_RGMII_RXD1
22 (TXDLY) RXD1 (CFG_EXT) LED0 TXDLY=0 VDD 24 32 PLLOFF=1 Green 4k7 CFG_EXT = 1 4k7 or ETHx_RGMII_RXD2 4k7
22 510 (PLLOFF) RXD2 PLLOFF=0 23 VDD PHY_AD0=1
4k7 ETHx_RGMII_RXD3 4k7
22 (PHY_AD0) (CFG_LDO0) RXD3 LED1 22 33 Yellow ETHx_RGMII_GTX_CLK 4k7CFG_LDO0=0
22 20 TXC 510 ETH1 ETHx_RGMII_TX_CTL
22 19 TXCTL MDIN3 10 ETHx_RGMII_TXD0
22 18 TXD0 MDIP3 9 ETHx_RGMII_TXD1
22 17 TXD1 MDIN2 7 ETHx_RGMII_TXD2
22
2 x 16 TXD2 MDIP2 6 ETHx_RGMII_TXD3 HSP051-
22 TXD3 MDIN1 4M10 10/100/1000 15 5 DVDD ETHx_MDC
1k5 RJ45 connector 22 MDC MDIP1 13 4 DVDD with Magnetics ETHx_MDIO 1k5 PHY_AD[2:0] = 0b001
14 MDIO MDIN0 2 and LEDs
ETHx_PHY_INTN
INTB / PMEB MDIP0 1 Pxx WKUPx optional DVDD 31 PWR 10k 12 PHYRSTB RSET 39 2k49 1% Pxx GPIOxx
GPIO exposed thermal pad 100Ω differential traces
41 50Ω traces unless otherwise noted. Traces should be short with balanced length. DT71892V1

Figure 14. Gigabit Ethernet PHY connection (ADIN1300xCPZ)

VDD
25, 31, 40 VDDIO
3.3V
VDD = 1.8V ± 5% or 3.3V ± 5% AVDD_3P3 11, 20
(PHY 3.3V and 0.9V can be shutdown during STANDBY) ADIN1300xCPZ
10/100/1000M
Ethernet Transceiver STM32N6x 0.9V VDD
optional if RCC
VDD provides a DVDD_0P9 4, 28, 36
25 MHz clock
ETHx_CLK (25 MHz Reference)
Note: max input 2.5V
22 68 (CLK_IN) RCC 9 XTAL_I or VDD Only for 240 25MHz Pull-up should VDD = 3.3V 125 MHz XTAL_O open be set by SW 8 clock path enabled in SYSCFG
or ETHx_RGMII_CLK125 optional if RCC 125 MHz is available 22
VDD MACIF_SEL0=0 27 GP_CLK (MDIX_MODE1)
ETHx_RGMII_RX_CLK 4k7
22
VDD MACIF_SEL0=0 34 RXC (MACIF_SEL0) CLK25_REF 6 open
ETHx_RGMII_RX_CTL 4k7
22 35 RX_CTL (MACIF_SEL1) VDD PHYAD_0=1
ETHx_RGMII_RXD0 4k7
22 (PHYAD_0) RXD0
VDD 33 Active High output 3.3V 4k7PHYAD_1=0 ETHx_RGMII_RXD1
22 RXD1 Green 32 (PHYAD_1) (PHY_CFG1) LINK_ST 26 VDD PHY_CFG1=0 PHYAD_2=0
ETHx_RGMII_RXD2 4k7 10K
22 510 (PHYAD_2) RXD2 30 VDD PHYAD_3=0 3.3V 3.3V ETHx_RGMII_RXD3 4k7
22 (PHYAD_3) RXD3 Yellow 29 10k PHY_CFG0=1 ETHx_RGMII_GTX_CLK Note: IO on AVDD_3P3
ETH1(1) 22
38 TXC (PHY_CFG0) LED_0 21 Active Low output 510 ETH2(2) ETHx_RGMII_TX_CTL
22 TX_CTL MDI_3_N ETH3(3) 37 19 ETHx_RGMII_TXD0
22 TXD0 MDI_3_P
ETHx_RGMII_TXD1 39 18
22 1 TXD1 MDI_2_N 17 ETHx_RGMII_TXD2
22
2 x
ETHx_RGMII_TXD3 2 TXD2 MDI_2_P 16 HSP051-
22 4M10 MDI_1_N 10/100/1000 3 TXD3 VDD 15 ETHx_MDC
1k5 RJ45 connector 22 MDC MDI_1_P
VDD 14 with Magnetics ETHx_MDIO 1k5 PHY_AD[2:0] = 0b001 23
MDIO MDI_0_N
24 13 and LEDs
ETHx_PHY_INTN optional
INT_N MDI_0_P
Pxx WKUPx 3V3 22 Note: IO on AVDD_3P3 Active Low reset input PWR 12
10k 7 RESET_N REXT 10 3k01 1% Pxx GPIOxx Active High reset output
GPIO 10 nF GND exposed thermal pad 100Ω differential traces
100k 5 41 50Ω traces unless otherwise noted. Traces should be short with balanced length DT73900V1

AN5967 - Rev 1 page 27/36

<!-- page: 28 -->
AN5967
Reference design examples

Note: 1. ETH1 is either ETH1 direct or ETHSW port2 (ETHSW is not available on some part numbers).
2. ETH2 is not available on some part numbers.
3. ETH3 is ETHSW port1. ETHSW is not available on some part numbers.
4. Decoupling capacitors are not shown.
As RCC cannot provide the 25 MHz reference clock to the PHY in low-power modes, the dedicated 25 MHz
crystal is required on the PHY in case "wake-up on LAN" (WOL) is needed for the platform.
Setting RCC PLLs to get 25 MHz output for PHY can constrain other RCC frequencies. In that case, it is more
flexible to put a dedicated 25 MHz crystal on the PHY.

Figure 15. Gigabit Ethernet PHY connection with VDD = 1.8 V (RTL8211F)

3.3V 3.3V
28 DVDD_RG AVDD3V3 11, 40
3.3V
VDD = 3.3V ± 5% DVDD3V3 29 (PHY 3.3V can be shut down during STANDBY) RTL8211F(I)-CG
optional if 10/100/1000M
RCC
STM32N6x VDD provides a Ethernet Transceiver REG_OUT 30 AVDD10 (1.0V) 25 MHz VDD
clock AVDD10 3
ETHx_CLK (25 MHz Reference)
RCC or VDD 22 37 XTAL_OUT (EXT_CLK) AVDD10 8, 38 25MHz should 125 MHz Pull-up
be set by SW clock path enabled in SYSCFG 36 XTAL_IN DVDD10 21
or ETHx_RGMII_CLK125 optional if RCC 125 MHz is available 22 35 CLKOUT
ETHx_RGMII_RX_CL
22 (PHY_AD1) RXC PHY_AD1=0 K 27 Green CFG_LDO1 = 0 4k7 4k7 ETHx_RGMII_RX_CT
22 510 (PHY_AD2) (CFG_LDO1) RXCTL PHY_AD2=0 RXDLY=1 L 26 LED2 34 VDD 4k7 or ETHx_RGMII_RXD0 4k7
22 (RXDLY) RXD0 RXDLY=0 25 TXDLY=1 VDD 3.3V 4k7 4k7 or ETHx_RGMII_RXD1
22 (TXDLY) RXD1 (CFG_EXT) LED0 TXDLY=0 VDD 24 32 PLLOFF=1 Green CFG_EXT = 1 4k7 4k7 or ETHx_RGMII_RXD2 4k7
22 510 (PLLOFF) RXD2 PLLOFF=0 23 VDD PHY_AD0=1
4k7 ETHx_RGMII_RXD3 4k7
22 (PHY_AD0) (CFG_LDO0) RXD3 LED1 22 33 Yellow CFG_LDO0=0 ETHx_RGMII_GTX_CLK 4k7
22 20 TXC 510 ETH1 ETHx_RGMII_TX_CTL
22 19 TXCTL MDIN3 10 ETHx_RGMII_TXD0
22 18 TXD0 MDIP3 9 ETHx_RGMII_TXD1
22 17 TXD1 MDIN2 7 ETHx_RGMII_TXD2
22
2 x 16 TXD2 MDIP2 6 ETHx_RGMII_TXD3 HSP051-
22 TXD3 MDIN1 4M10 10/100/1000 15 5 DVDD ETHx_MDC
1k5 RJ45 connector 22 MDC MDIP1 13 4 DVDD with Magnetics ETHx_MDIO 1k5 PHY_AD[2:0] = 0b001
14 MDIO MDIN0 2 and LEDs
ETHx_PHY_INTN
INTB / PMEB MDIP0 1 Pxx WKUPx optional DVDD 31 PWR 10k 12 PHYRSTB RSET 39 2k49 1% Pxx GPIOxx
GPIO exposed thermal pad 100Ω differential traces
41 50Ω traces unless otherwise noted. Traces should be short with balanced length. DT73901V1

Note: 1. ETH1 is either ETH1 direct or ETHSW port2 (ETHSW is not available on some part numbers).
2. ETH2 is not available on some part numbers.
3. ETH3 is ETHSW port1. ETHSW is not available on some part numbers.
4. Decoupling capacitors are not shown.
As RCC cannot provide the 25MHz reference clock to the PHY in low-power modes, the dedicated 25 MHz
crystal is required on the PHY in case "wake-up on LAN" (WOL) is needed for the platform.
Setting RCC PLLs to get 25 MHz output for PHY can constrain other RCC frequencies. In that case, it is more
flexible to put a dedicated 25 MHz crystal on the PHY.

AN5967 - Rev 1 page 28/36

<!-- page: 29 -->
AN5967
Reference design examples

Table 10. ETH RGMII pins

Pin name Signal direction(1) ETH1 (2) ETH2 (3) ETH3 (3)(4) comments

ETHx_CLK → PF3, PF5, PF8 PF4, PG3 -(5) Optional 25 MHz reference (6)

Optional if 125 MHz is fed
ETHx_RGMII_CLK125 ← PC4, PH9 PF8, PG2 -(7) internally from RCC to ETH
IP

ETHx_RGMII_RX_CLK ← PA14 PC0, PF6 PA5 -

ETHx_RGMII_RX_CTL ← PA11 PC3, PF8 PA2 -

ETHx_RGMII_RXD0 ← PF1 PG0 PA9 -

ETHx_RGMII_RXD1 ← PC2 PC12 PA10 -

ETHx_RGMII_RXD2 ← PG0, PH12 PA0, PF9 PH7 -

ETHx_RGMII_RXD3 ← PC12, PH13 PC11, PG1 PH8 -

ETHx_RGMII_GTX_CLK → PC0 PF7 PH2 -

ETHx_RGMII_TX_CTL → PA13 PC4 PA3 -

ETHx_RGMII_TXD0 → PA15 PC7 PA6 -

ETHx_RGMII_TXD1 → PC1 PC8 PA7 -

ETHx_RGMII_TXD2 → PC7, PH10 PC9, PF10 PH6 -

ETHx_RGMII_TXD3 → PC8, PH11 PC10, PF11 PH3 -

ETHx_MDC → PA9, PF0, PF4 PC6, PG4, PH10 -(8) -

→
ETHx_MDIO PA10, PF2, PF5 PC5, PF9, PH11 -(8) -
←

ETHx_PHY_INTN ← PA12, PC6, PF5 PF5, PG3 PA1 Optional

1. → MPU to PHY, ← PHY to MPU.
2. Can also be used as ETHSW port2. ETHSW is not available on some part numbers.
3. Not available on some part numbers.
4. Equivalent to ETHSW port1.
5. If needed, ETH1_CLK must be used.
6. As RCC cannot provide the reference clock to the PHY in low-power modes, a dedicated 25 MHz crystal is required on the
PHY if "wake-up on LAN" (WOL) is needed for the platform.
7. If needed, ETH1_RGMII_CLK125 must be used.
8. ETH3 PHY share the same MDC/MDIO pins than ETH1 PHY (need to use a different address for the PHY).

13.5 Camera serial interface
As pixel data received by the camera serial interface (CSI) are processed by DCMIPP, the parallel high-resolution
sensor interface is not available when using CSI. In that case, a second parallel low-performance sensor is still
possible using DCMI. Refer to the reference manual for details.
A 200 Ω 1% resistor must connect CSI_REXT and VSS.

AN5967 - Rev 1 page 29/36

<!-- page: 30 -->
## Visual Summary (Page 30)

- page_class: timing_diagram
- confidence: 0.92
- reason_codes: diagram_dominant, table_structure_required, timing_context

Figure 16 shows a block diagram of a CSI camera module interface with timing signals (PIXCLK, VSYNC, HSYNC) and control/data paths to an MCU. Table 11 provides detailed PCB routing recommendations for differential/single-ended impedance, length matching, via count, and trace spacing.

<!-- page: 31 -->
SI

AN5967

Reference design examples

Revision history

Table 12. Document revision history

<table><tr><td>Date</td><td>Version</td><td>Changes</td></tr><tr><td>28-Nov-2024</td><td>1</td><td>Initial release</td></tr></table>

AN5967 - Rev 1

page 31/36

<!-- page: 32 -->
AN5967
Contents

Contents

1 General information . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .2
2 Terminology . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .3
3 Power supplies . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .5

3.1 Main power supplies . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5

3.2 Secondary power supplies. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6

3.3 Independent ADC supply and reference voltage. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7

3.4 USB supplies . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7

3.5 Specific IO constraints related to voltage settings. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8

3.6 System startup. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8

3.7 SMPS configuration . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
4 Package selection . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .12
5 Alternate function mapping to pins . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .13
6 Clocks. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .14
7 HSE oscillator . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .15

7.1 External source - HSE bypass mode . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .15

7.2 External crystal - HSE crystal mode . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .15
8 LSE oscillator . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .16

8.1 External source - LSE bypass mode. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .16

8.2 External crystal - LSE crystal mode . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .16
9 Clock security system . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .17

9.1 CSS on HSE . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .17

9.2 CSS on LSE. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .17
10 Boot configuration. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .18
11 Debug management . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .19

11.1 SWJ debug port (serial wire and JTAG) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .19

11.2 Debug LED . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .19
12 Recommendations. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .20

12.1 PCB. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .20

12.2 Component position . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .20
12.3 Ground and power supplies (VSSx, VDDx) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .20

12.4 I/O speed settings . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .20

12.5 ESD/EMI protections . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .20

12.6 Sensitive signals . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .21

AN5967 - Rev 1 page 32/36

<!-- page: 33 -->
AN5967
Contents

12.7 Unused I/Os and features . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .21
13 Reference design examples. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .22

13.1 Clock. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .22

13.2 SD card . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .22

13.3 USB. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .23

13.3.1 USB hi-speed host with Type-A connector (USBH) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 23

13.4 Ethernet . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .24

13.4.1 10/100M Ethernet . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 24

13.4.2 Gigabit Ethernet . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 26

13.5 Camera serial interface . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .29
Revision history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .31
List of tables . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .34
List of figures. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .35

AN5967 - Rev 1 page 33/36

<!-- page: 34 -->
AN5967
List of tables

List of tables

Table 1. Reference documents . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 2
Table 2. Acronyms . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 3
Table 3. Main power supplies. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6
Table 4. Power supply pins . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7
Table 5. Package summary . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
Table 6. Boot source description . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 18
Table 7. HSE BOM for oscillator or crystal example for NX2016SA - 40 MHz . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 22
Table 8. USB high-speed PCB routing recommendations . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 23
Table 9. ETH RMII pins. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 26
Table 10. ETH RGMII pins . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 29
Table 11. CSI PCB routing recommendations . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30
Table 12. Document revision history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 31

AN5967 - Rev 1 page 34/36

<!-- page: 35 -->
AN5967
List of figures

List of figures

Figure 1. Power supply overview . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5
Figure 2. Device startup with VCORE supplied directly from SMPS step-down converter. . . . . . . . . . . . . . . . . . . . . . . . . 9
Figure 3. Device startup with VCORE supplied from an external regulator . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
Figure 4. System supply configurations . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
Figure 5. STM32CubeMX example screenshot. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 13
Figure 6. HSE source clock . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15
Figure 7. LSE external clock . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16
Figure 8. LSE crystal resonators. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16
Figure 9. SD-Card with embedded level shifter connection. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 23
Figure 10. USB hi-speed host example . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 24
Figure 11. 10/100M Ethernet PHY connection example. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 24
Figure 12. 10/100M Ethernet PHY connection (with REFCLK from RCC). . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 25
Figure 13. Gigabit Ethernet PHY connection with VDD = 3.3 V (RTL8211F) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 27
Figure 14. Gigabit Ethernet PHY connection (ADIN1300xCPZ) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 27
Figure 15. Gigabit Ethernet PHY connection with VDD = 1.8 V (RTL8211F) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 28
Figure 16. CSI example . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30

AN5967 - Rev 1 page 35/36

<!-- page: 36 -->
AN5967

IMPORTANT NOTICE – READ CAREFULLY

STMicroelectronics NV and its subsidiaries (“ST”) reserve the right to make changes, corrections, enhancements, modifications, and improvements to ST
products and/or to this document at any time without notice. Purchasers should obtain the latest relevant information on ST products before placing orders. ST
products are sold pursuant to ST’s terms and conditions of sale in place at the time of order acknowledgment.

Purchasers are solely responsible for the choice, selection, and use of ST products and ST assumes no liability for application assistance or the design of
purchasers’ products.

No license, express or implied, to any intellectual property right is granted by ST herein.

Resale of ST products with provisions different from the information set forth herein shall void any warranty granted by ST for such product.

ST and the ST logo are trademarks of ST. For additional information about ST trademarks, refer to www.st.com/trademarks. All other product or service names
are the property of their respective owners.

Information in this document supersedes and replaces information previously supplied in any prior versions of this document.

© 2024 STMicroelectronics – All rights reserved

AN5967 - Rev 1 page 36/36
