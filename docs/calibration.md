# Calibration

The gain and offset trimmers interact, so the order below matters. Done out
of order, the procedure converges slowly or not at all.

Run firmware sketch [04_amp_tuning](../firmware/04_amp_tuning/) while doing
this — it prints both half-cycle voltages live.

## Procedure

1. **Offset, with no current flowing.** Tie both amplifier inputs to the
   same mid-rail node (a 10k/10k divider from 5 V). The differential input
   is then exactly zero, so whatever appears at the output is pure offset.
   Trim to 2.5 V.

   > [!NOTE]
   > Tying the inputs to ground instead does not work — 0 V common-mode is
   > outside the AD620's input range (see [hardware.md](hardware.md),
   > constraint 2), and the output tells you nothing about the offset.

2. **Gain, with injection running.** Return the inputs to the sense nodes.
   Trim until the two polarities land roughly at 1.5 V and 3.5 V — clear
   separation, no clipping against either rail.

3. **Re-check offset.** The trimmers interact; expect the offset to have
   drifted. Iterate steps 1–2 until both hold.

4. **Compute the gain from measurement, not from theory.**

   ```
   GAIN = (V_plus − V_minus) / (differential measured directly with a multimeter)
   ```

   The trimmer's nominal setting and the AD620 gain-resistor equation are
   starting points, not answers.

## Worked example — the actual build

| Quantity | Value | How obtained |
|---|---|---|
| Chain resistance | 20.9 kΩ | measured with multimeter, out of circuit |
| Injection voltage | 3.8 V | measured across OUT1–OUT2, not the 5 V nominal |
| V(M−N) | ±17.9 mV | measured directly, symmetric in both polarities |
| Gain | 76 | (3.5483 − 0.8205) / 0.0358 |
| **Result** | **98.56 Ω** on a 100 Ω resistor | instrument's own computation |

The symmetry of the ±17.9 mV reading was itself the diagnostic that
mattered: identical magnitude in both polarities is what confirmed both
half-cycles were finally being sensed inside the amplifier's input range.
The earlier asymmetric chain produced numbers that were just as stable —
and a 144 Ω answer for a 100 Ω resistor, because one half-cycle was being
measured out of range (see the [failure log](failure-log.md)).

The measured gain (76 for this build) goes into the `AMP_GAIN` constant in
firmware sketches 05 and 06. It is a property of this particular trimmer
setting, not of the design — re-derive it after any change to the analog
front end.

---

Next: [Failure log](failure-log.md) — every way this instrument broke, and
what each failure actually was.
