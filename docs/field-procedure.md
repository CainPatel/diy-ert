# Field procedure

How the validated field reading was taken, written as a repeatable
procedure. This is a single-point Wenner measurement using firmware sketch
[05_single_measurement](../firmware/05_single_measurement/).

## Equipment

- The instrument, a 12 V injection supply or battery, and a laptop
- Four stainless steel electrode rods and a hammer
- Tape measure
- Multimeter, for the contact check
- Water, for dry ground

## Procedure

1. **Lay out the line.** Four electrodes equally spaced by `a`, in the
   order C1 — P1 — P2 — C2. The validated reading used a = 0.5 m. Drive
   each rod no deeper than about a/10 so it behaves like a point
   electrode.

2. **Check electrode contact.** Multimeter between adjacent rods. In moist
   Georgia clay this build measured 40–80 kΩ. That sounds disqualifying
   and is not — it is exactly why the instrument uses four electrodes (see
   the [README](../README.md)): at the current pair, contact resistance
   only limits how much current flows, and the current is measured across
   the shunt rather than assumed; at the potential pair, no current flows,
   so its contact resistance drops no voltage and cancels out of the
   result. If contact resistance is far higher than this, wet the soil
   around the rods.

3. **Wire and power up.** C1/C2 to the outer rods, P1/P2 to the inner
   pair. Power the logic first and confirm the ADC answers (sketch 00
   takes seconds), then enable the injection supply.

4. **Measure promptly, and record everything raw.** Run sketch 05. Log the
   injected current, the measured potential, and the computed R — not just
   the final ρ, so the result can be recomputed later. The validated
   reading, in full:

   ```
   # Wenner array, a = 0.5 m, Georgia clay soil, moist
   # I = 1.24 mA, V(P1-P2) = 28.5 mV
   # R = 22.98 Ω, ρ = 2πaR = 72.20 Ω·m
   ```

5. **Between readings, the firmware parks the bridge** (both H-bridge
   inputs low). Do not leave DC flowing while setting up the next reading
   — see the drift note below.

## The known limitation: polarisation drift

Readings drifted from 71 to 90 Ω·m over several minutes as the steel
potential electrodes polarised. This bounds how long a survey can run:
past that window, later measurements read systematically higher for
electrochemical rather than geological reasons, and the drift becomes a
false gradient in the data.

The standard fix is non-polarising electrodes for the potential pair — a
copper rod in saturated copper-sulphate solution inside a porous pot. They
are cheap to build and are the hardware change this project needs before
any multi-minute survey is credible.

---

Next: [Firmware](../firmware/README.md) — bring-up in seven sketches, run
in order.
