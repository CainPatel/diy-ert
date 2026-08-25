# Hardware

The signal chain, in the order the signal flows through it.

## Signal chain

| Stage | Part | Role |
|---|---|---|
| Injection | L298N H-bridge | alternates polarity, driven from two Arduino pins |
| Current sense | 1 kΩ shunt, read differentially on ADS1115 A2/A3 | measures the actual injected current |
| Potential sense | AD620 instrumentation amplifier | rejects large common-mode, amplifies the mV differential |
| Reference | LM358 unity-gain buffer on a 10k/10k divider | holds the AD620 REF pin at mid-rail (2.5 V) |
| Digitisation | ADS1115, 16-bit, I²C at 0x48 | reads the amp output on A0 |
| Switching | 4× CD74HC4067, 16 channels each | selects which four of 16 electrodes act as C1/C2/P1/P2 |

## Why four multiplexers

Each measurement needs four electrodes in different roles: two carrying
current (C1, C2) and two sensing potential (P1, P2). One mux per role means
any electrode can play any part, which is what an automated survey needs —
the same physical rod is a current electrode in one measurement and a
potential electrode in the next. The cost is control pins: 4 muxes × 4
address lines + 2 bridge inputs = 18 Arduino pins (the pin map is in
[firmware/README.md](../firmware/README.md), and the pin count is what
motivates the shift-register migration described in
[hardware/README.md](../hardware/README.md)).

## Critical design constraints — learned the hard way

> [!CAUTION]
> Every item below was learned by breaking something or by measuring
> nonsense. The first one cost four ADS1115 chips.
>
> 1. **ADC input limits.** The ADS1115 absolute maximum input is
>    VDD + 0.3 V ≈ 5.3 V. Resistor-chain nodes were measured at 6.3 V with
>    12 V injection. Every line crossing from the injection domain into 5 V
>    logic needs a 10 kΩ series resistor. Without it, fault current is
>    limited only by the chain and the chip's bond wires. With it, ~170 nA.
>    This lesson cost four chips.
>
> 2. **Amplifier input range.** The AD620's usable input window is
>    −VS + 1.9 V to +VS − 1.4 V. On a 5 V single supply that is roughly
>    1.9 V to 3.6 V. Sense nodes outside this range produce readings that
>    look plausible and are meaningless — the instrument keeps printing
>    stable numbers while measuring nothing.
>
> 3. **Symmetric resistor chains.** An asymmetric chain (100 kΩ one side,
>    10 kΩ the other) puts the sense nodes at wildly different potentials
>    depending on injection polarity — measured 6.67 V one way and 1.32 V
>    the other, both outside the amplifier's window in at least one
>    polarity. A symmetric chain keeps both sense nodes at mid-rail in both
>    polarities.
>
> 4. **Single-supply amplifiers cannot output negative voltages.** With
>    REF at 0 V, the negative half-cycle clamps at the output floor and
>    both halves read nearly identically — the differencing scheme then
>    cancels the signal along with the offset. REF must sit at mid-rail,
>    which is what the LM358 buffer is for.
>
> 5. **Measure, never assume.** The L298N's voltage drop is
>    current-dependent: the nominal 5 V supply delivered only 3.8 V across
>    OUT1–OUT2 at the working current. Using the nominal value produced a
>    45% error in computed resistance. Every constant in the firmware that
>    could be measured, was measured.

## The reference, in one paragraph

The AD620's REF pin sets what "zero differential" looks like at the output.
A bare 10k/10k divider is not stiff enough — the REF pin must be driven
from a low-impedance source or the gain equation degrades — so the divider
is buffered through an LM358 wired as a unity-gain follower. Output sits at
2.5 V, and the amp swings above and below it as the injection polarity
flips.

---

Next: [Calibration](calibration.md) — putting the amplifier in range and
measuring the gain, in the order that actually converges.
