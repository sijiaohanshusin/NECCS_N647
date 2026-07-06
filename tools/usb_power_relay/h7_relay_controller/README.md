# H7 USB Power Relay Controller

This folder contains a small STM32H7-side relay controller for power-cycling an
ST-LINK or target board from Windows.

It is designed for the common one-channel relay board marked:

```text
Control side: DC+  DC-  IN
Contact side: NO   COM  NC
```

## Wiring

Cut only the red USB +5 V/VBUS wire in a USB extension cable.

```text
PC-side USB red wire      -> COM
device-side USB red wire  -> NC
NO                        -> not connected

USB black GND             -> direct through
USB D+ / D-               -> direct through
USB shield                -> direct through
```

Using `COM` + `NC` makes USB power on by default. The relay energizes only when
the controller intentionally cuts power.

Relay control wiring:

```text
Relay DC+ -> H7 board 5V
Relay DC- -> H7 board GND
Relay IN  -> H7 GPIO output
```

Do not drive the relay coil from a GPIO pin. The GPIO only drives `IN`; the
relay board power must come from a real 5 V rail. If a 3.3 V H7 GPIO does not
trigger the relay reliably in `H` mode, use `L` mode with
`RELAY_ACTIVE_LEVEL GPIO_PIN_RESET`, or add a transistor/driver stage.

Default firmware settings:

```c
#define RELAY_GPIO_PORT    GPIOB
#define RELAY_GPIO_PIN     GPIO_PIN_1
#define RELAY_ACTIVE_LEVEL GPIO_PIN_SET
```

Put the relay board jumper on `H` for active-high trigger. If you use `L`, set
`RELAY_ACTIVE_LEVEL` to `GPIO_PIN_RESET`.

## STM32H7 project integration

1. Copy `relay_controller.c` and `relay_controller.h` into the old H7 CubeIDE
   project.
2. Configure the relay GPIO as output push-pull. Default is `PB1`.
3. Call `UsbPowerRelay_Init()` after `MX_GPIO_Init()`.
4. If using UART commands, copy the relevant code from
   `main_cube_hal_example.c` into the H7 project's `main.c`.
5. Set `RELAY_UART_HANDLE` if your board's virtual COM port is not `huart3`.

The example command protocol is intentionally tiny:

```text
0 or n/N: USB power ON  (relay inactive, COM-NC closed)
1 or f/F: USB power OFF (relay active, COM-NC open)
r or R  : cut USB power for the default delay, then restore
?       : report current state
```

The existing project at `D:\Project\NECCS\H7_Original` has been integrated with
a standalone relay CLI. In standalone mode the H7 initializes only GPIO and
USART1, then waits for commands before starting the old display/audio app. This
keeps relay recovery available even when the full H7 app is unhealthy.

```text
status
off
on
cycle 5000
test
cfg relay status
cfg relay off
cfg relay on
cfg relay cycle 5000
```

Verified on 2026-07-06 with COM5 at 921600 baud. Leave DTR/RTS deasserted by
default; asserting them can disturb some USB-serial/reset wirings.

For N647 debug recovery, prefer a two-relay setup:

1. Relay 1 cuts ST-LINK USB VBUS.
2. Relay 2 cuts the N647 target board input power.

If only one relay is available, cutting ST-LINK VBUS still helps USB/ST-LINK
enumeration, but it may not recover an N647 AP1/core-ID failure if the target
board remains powered.

## Windows control

List possible serial ports:

```powershell
powershell -ExecutionPolicy Bypass -File .\usb_power_cycle_h7.ps1 -ListPorts
```

Power-cycle with a 5 second off window:

```powershell
powershell -ExecutionPolicy Bypass -File .\usb_power_cycle_h7.ps1 -Port COM5 -Action Cycle -OffMs 5000
```

Restore USB power:

```powershell
powershell -ExecutionPolicy Bypass -File .\usb_power_cycle_h7.ps1 -Port COM5 -Action On
```

Cut USB power:

```powershell
powershell -ExecutionPolicy Bypass -File .\usb_power_cycle_h7.ps1 -Port COM5 -Action Off
```

From the repository root, optional N647 recovery:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\usb_power_relay\h7_relay_controller\n647_recover_via_h7_relay.ps1 -Port COM5 -RunRamDebug -SkipBuild
```
