# Firmware

Seven sketches that bring the breadboard instrument up one verified
subsystem at a time, plus one sketch for the custom PCB. The end-to-end
walkthrough (toolchain, uploading, what to type, capturing output) is in
[docs/software.md](../docs/software.md).

> [!WARNING]
> **Run the sketches in order.** Every hardware failure on this project,
> including four destroyed ADS1115s, came from powering up an unverified
> circuit. Each sketch verifies one subsystem against an independent
> reference (a multimeter, a known resistor) before the next subsystem
> goes on top.

| # | Sketch | Board | Verifies | Failure signature |
|---|---|---|---|---|
| 00 | [I2C scanner](00_i2c_scanner/) | Mega or Nano | ADC is alive at 0x48 | nothing found: check power, SDA/SCL, solder |
| 01 | [Raw ADC readout](01_raw_adc/) | Mega or Nano | ADC reads a known voltage correctly | raw counts near 0 on A2/A3: shunt sense wires disconnected |
| 02 | [DC hold](02_dc_hold/) | Mega | H-bridge switches; lets a multimeter settle | trace voltage along the path, find where it disappears |
| 03 | [Mux channel scan](03_mux_scan/) | Mega | Address decoding and channel continuity | all channels identical: read the mux output directly, not through the amp |
| 04 | [Live amp tuning](04_amp_tuning/) | Mega | Amp operating in range | both halves near 3.6 V: saturated, adjust offset first, not gain |
| 05 | [Single measurement](05_single_measurement/) | Mega | Whole chain, against a known resistor | should report ~100 Ω on a 100 Ω target |
| 06 | [Wenner survey](06_wenner_survey/) | Mega | Full sequence over working channels | outputs pyGIMLi format rows |
| 07 | [PCB console](07_pcb_console/) | Nano (PCB) | steps 01-06 as serial commands, through the shift registers | see its README |

Sketches 00-06 are the breadboard build (Arduino Mega, 18 direct control
pins). Sketch 07 is the PCB (Arduino Nano, 3 pins into three 74HC595
shift registers), where one sketch with a command per step replaces
02-06; 00 runs unchanged on the Nano first.

All eight compile cleanly with arduino-cli 1.5 and AVR core 1.8.8
(00-06 for `arduino:avr:mega`, 00, 01 and 07 for `arduino:avr:nano`).

## Requirements

- Arduino IDE or arduino-cli, with the **Adafruit ADS1X15** library
  installed (Library Manager, search "Adafruit ADS1X15"; accept the
  Adafruit BusIO dependency). Sketches 00, 02 and 03 need only the core
  libraries.
- Serial monitor at 9600 baud.
- Some Arduino IDE versions refuse sketch folders that start with a
  digit. If yours does, rename the folder and the `.ino` together (for
  example `s00_i2c_scanner/s00_i2c_scanner.ino`) and keep the ordering
  prefix.

## Pin map, breadboard (Arduino Mega)

| Function | Pins |
|---|---|
| L298N IN1 / IN2 | 6, 7 |
| Mux C1, address S0-S3 | 22, 24, 26, 28 |
| Mux C2, address S0-S3 | 30, 32, 34, 36 |
| Mux P1, address S0-S3 | 38, 40, 42, 44 |
| Mux P2, address S0-S3 | 46, 48, 50, 52 |
| ADS1115 | SDA = 20, SCL = 21, address 0x48; A0 = amp output, A2/A3 = shunt |

18 control pins total (16 mux address lines + 2 bridge inputs).

## Pin map, PCB (Arduino Nano)

| Function | Pins |
|---|---|
| 74HC595 SRCLK, RCLK, SER | D2, D3, D4 |
| ADS1115 | SDA = A4, SCL = A5, address 0x48 |
| Mux select lines and L298N IN1/IN2 | from the shift registers; chain order and bit layout in [hardware/README.md](../hardware/README.md) |

## Implementation details that carry the lessons

These are deliberate. Keep them if you modify the code:

- The stacking accumulators are explicitly initialised to zero.
  Uninitialised floats in C++ contain garbage, and garbage accumulated
  over 20 cycles looks like a plausible measurement. This bug happened.
- The shunt is read in both polarities and the magnitudes averaged, so
  bridge asymmetry and differential offset average out of the current
  value.
- The bridge is parked with both inputs LOW between measurements, to
  limit electrode polarisation
  ([docs/field-procedure.md](../docs/field-procedure.md)).
- The survey runs from `loop()` behind a done-flag, not from `setup()`,
  and waits for a start command, so output is not lost if the serial
  monitor opens late.
- A `CH[]` lookup table maps physical electrode position to a working mux
  channel, so dead channels are invisible to the survey logic:

```cpp
// Working channel map from the physical build
// Dead: 0, 7, 11, 12 (breadboard contact faults)
const int CH[12] = {1,2,3,4,5,6,8,9,10,13,14,15};
```

- On the PCB, `setup()` latches a parked bridge into the shift registers
  before anything else, because their outputs are live from power-on
  with undefined contents.

## Calibration constants

`AMP_GAIN` is a measured value, not a design constant: 76 on the
breadboard build, (3.5483 - 0.8205) / 0.0358; 50.4 nominal on the PCB
from R6 = 1 kΩ, to be measured. Run the procedure in
[docs/calibration.md](../docs/calibration.md) and substitute your own
value. Same for the shunt: measure its actual resistance rather than
trusting the band code.
