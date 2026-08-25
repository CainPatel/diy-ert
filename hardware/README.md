# Hardware

The electrical design and wiring reference are in
[docs/hardware.md](../docs/hardware.md). This directory covers the PCB
and the bill of materials.

## Where the PCB stands

**Schematic: complete and electrically checked.** 37 components, all 16
electrode nets verified, 10 kΩ protection resistors on every ADC input,
buffered mid-rail reference, separate injection supply terminal.

**Layout: not complete.** Attempted on 2 layers, then 4. An autorouter
round trip produced 686 DRC violations (201 net shorts, 199 dangling
tracks) from a layer-mapping mismatch in the export and re-import.

> [!IMPORTANT]
> Autorouters do not understand which nets are noise-sensitive. The
> millivolt-level path from the potential electrodes through the muxes
> to the AD620 cannot be routed like a data bus. Hand-route the analog
> paths and lock them before autorouting the rest, or hand-route
> everything on a board this size.

**Current direction:** moving from the Arduino Mega to a Nano plus three
74HC595 shift registers, which takes the 18 direct control pins (16 mux
address lines + 2 bridge inputs) down to 3 and shrinks the board
substantially.

The [kicad/](kicad/) directory holds the schematic and layout sources.
Once the schematic settles, an exported PDF or SVG of it belongs in this
directory too, so people can read it without installing KiCad.

## Bill of materials

Prices are approximate single-unit hobbyist pricing (2026). The build
came to roughly $150 including spares.

| Part | Qty | ~Price | Notes |
|---|---|---|---|
| Arduino Mega 2560 (clone) | 1 | $20 | needs 18 free digital pins with the current design |
| ADS1115 breakout | 1 + spares | $10 ea | four died during development; budget spares, and solder the headers |
| AD620AN (DIP-8) | 1 | $10 | instrumentation amplifier |
| LM358 (DIP-8) | 1 | $1 | mid-rail reference buffer |
| CD74HC4067**E** (DIP-24) | 4 | $2 ea | the E suffix is the DIP package; socket these |
| L298N driver module | 1 | $5 | injection H-bridge |
| 1 kΩ 1% resistor (shunt) | 1 | <$1 | measure its actual value, it sets the current reading |
| 10 kΩ resistors | ~20 | $2 | ADC input protection, dividers, bench chain |
| 10 kΩ multi-turn trimmers | 2 | $3 | gain and offset |
| DIP sockets (8 and 24 pin) | 6 | $3 | see note below |
| Breadboards | 3-4 | $15 | source of the four dead mux channels; a PCB is the fix |
| Stainless steel rods (electrodes) | 4-16 | $20 | threaded rod or tent stakes work |
| Hookup wire, jumpers, clips | | $10 | continuity-test before use (see the failure log) |
| 12 V supply or battery | 1 | $10 | injection supply, on its own terminal, separate from logic 5 V |

Buy DIP through-hole packages where they exist (CD74HC4067E, not the SMD
variant) and socket them. Every chip in this project was at some point
suspected of being dead, and a socketed chip is a ten second swap. Four
ADS1115s died before the root cause (chain nodes at 6.3 V against a
5.3 V absolute maximum) was found, and the swaps would have gone much
faster with everything socketed.
