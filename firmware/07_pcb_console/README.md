# 07: PCB console

**Target:** the custom PCB in [hardware/](../../hardware/), Arduino Nano.
Sketches 00-06 target the Mega breadboard build and drive the mux select
lines and the H-bridge from 18 direct pins. On the PCB those 18 lines come
from three chained 74HC595 shift registers on 3 Nano pins, so the PCB gets
one sketch with a serial command for each bring-up step instead.

**Board selection:** Arduino Nano. Most clones need Processor set to
"ATmega328P (Old Bootloader)".

**Commands** (9600 baud, any line ending):

| Command | Does | Replaces breadboard sketch |
|---|---|---|
| `?` | print this list | |
| `r` | one raw ADC readout: A0 and A2-A3, counts and volts | 01 |
| `p` | park the bridge, both L298N inputs low | 02 |
| `+` / `-` | hold the bridge forward / reverse in DC, for multimeter work | 02 |
| `c N` | put all four muxes on channel N (0-15) | 03 |
| `x A B C D` | set channels individually: C1=A, C2=B, P1=C, P2=D | |
| `t` | live amplifier readout, alternating polarity; any key stops | 04 |
| `m` | one stacked measurement on the current channels, prints R | 05 |
| `w` | Wenner survey over `CH[]`, prints a pyGIMLi data file | 06 |

Sketch 00 (I2C scanner) runs unchanged on the Nano and is still the first
thing to run.

**Constants to check before use:** `AMP_GAIN` (the PCB fixes the AD620
gain with R6 = 1 kΩ, 50.4 nominal; measure it per
[docs/calibration.md](../../docs/calibration.md)), `SHUNT_OHMS` (measure
R3), `ELECTRODE_SPACING`, and `CH[]` if the `c` scan finds a bad channel.

**Bring-up order on the PCB:** with no ICs fitted, check the 5 V rail.
Fit the Nano and the ADS1115 breakout, run sketch 00. Load this sketch,
`r` to confirm the ADC reads. Fit the shift registers and the L298N with
the injection supply off, `+`/`-`/`p` and confirm IN1/IN2 toggle at the
L298N pins. Fit the muxes, `c N` and beep out continuity from each mux
common pin to terminal E N. Fit the AD620 and LM358, `t` with the bench
chain. Then `m` on the known resistor. Details in
[docs/software.md](../../docs/software.md).

**What the suspect flag means:** `t` and `m` mark a reading as suspect
when the two half-cycles read nearly the same voltage (amplifier
saturated, or an input floating) or when the output sits within 0.2 V of
a rail. Both were real failure modes on the breadboard build. There are no
trimmers on the PCB: if the amp saturates, lower the injection voltage or
raise R6 to reduce the gain.

**Power-on note:** the 595 outputs are enabled from power-on (their
output-enable pin is tied to ground on the board) with undefined
contents. `setup()` latches a parked bridge and known mux channels first
thing, but the bootloader runs for about a second before that. Power the
logic before connecting the injection supply.
