# Failure log

The most useful document in this repository. Every entry cost real time or
real parts; each is recorded as symptom → root cause → fix so nobody has to
pay for the same lesson twice.

| Failure | Symptom | Root cause | Fix |
|---|---|---|---|
| Four dead ADS1115s | I²C device not found; pins hot to touch | Chain nodes at 6.3 V exceeding the 5.3 V absolute maximum | 10 kΩ series resistors on every line entering the ADC |
| Unsoldered breakout headers | Intermittent I²C over several days | Header pins press-fitted, not soldered | Bought a soldering station; solder every breakout |
| Amplifier always reads 3.62 V | Identical value on every mux channel | Single-supply amp with a floating input saturates regardless of gain | Ensure both amp inputs sit on real nodes; adjust offset before gain |
| Resistance reads 144 Ω instead of 100 Ω | Stable but wrong | Asymmetric chain put one sense node above the amp's input ceiling; one half-cycle measured out of range | Symmetric chain |
| Current reads zero while bridge is driving | Shunt shows no voltage | Multimeter cannot track a 150 ms square wave; it averages toward zero | Park the bridge in DC for multimeter work (firmware sketch 02) |
| Negative resistance in survey data | Physically impossible values | Amplifier railed in one polarity, producing a huge fake differential | Re-centre offset, reduce gain |
| Hours lost to a broken wire | Bridge outputs stuck near 0 V | Jumper failed internally, no visible damage | Continuity-test wires before suspecting components |
| 4 of 16 mux channels dead | Some electrodes never conduct | Breadboard contact faults, not chip or addressing faults — the dead channel numbers (0, 7, 11, 12) share no common address bit | Lookup table maps electrode position to working channel |

Notes on cost, where known: the ADC failure destroyed four chips before the
root cause was found; the unsoldered headers produced intermittent faults
for several days because the pins passed visual inspection; the broken
jumper ate several hours because every component upstream of it was
suspected first.

## What the failures have in common

Nearly all of them were **mechanical** (unsoldered headers, breadboard
contacts, a jumper broken inside its insulation) or a **voltage-domain
mismatch** (chain nodes above the ADC's absolute maximum, sense nodes
outside the amplifier's input window) — not logic errors. The code was
almost never the problem.

Two patterns are worth internalising:

1. **The dangerous failures look fine.** An unsoldered header looks
   soldered. A 6.3 V node doesn't look different from a 5 V node. An
   amplifier measuring out of range still prints stable, plausible
   numbers. The instrument lies confidently unless every stage has been
   verified against an independent reference.

2. **The debugging method that consistently worked** was isolating one
   subsystem at a time and verifying it against an independent instrument
   — a multimeter, a known resistor — before adding the next subsystem on
   top. That method is frozen into the [firmware directory](../firmware/):
   seven sketches, run strictly in order, each proving a single layer.

---

Next: [Field procedure](field-procedure.md) — taking the instrument
outside.
