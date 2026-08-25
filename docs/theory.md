# Theory

The instrument measures the electrical resistivity of the ground. All of
it reduces to Ohm's law plus one geometric conversion.

## Ohm's law, applied through the ground

Inject a known current `I` into the earth through two outer electrodes.
That current creates a potential field in the soil. Measure the potential
difference `V` it produces across two inner electrodes, and

```
R = V / I
```

is a resistance in ohms, measured the same way a multimeter measures a
resistor, except the "resistor" is the ground between the electrodes. In
the validated field reading, `I` was measured across a 1 kΩ shunt
(1.24 mA) and `V` came from the instrumentation amplifier (28.5 mV),
giving R = 22.98 Ω.

## The Wenner array and the geometric factor

Four electrodes in a line, equally spaced by `a`:

```
C1 ---- P1 ---- P2 ---- C2
    a       a       a
```

Current flows between the outer pair (C1, C2). Potential is measured
across the inner pair (P1, P2). The measured R depends on the electrode
geometry as well as on the ground, so it gets converted to an apparent
resistivity:

```
rho_a = 2 * pi * a * R
```

The geometric factor 2πa converts a circuit measurement (ohms) into a
material property (ohm-metres). For the field reading:
2π x 0.5 m x 22.98 Ω = 72.2 Ω·m.

## Why the polarity alternates

Steady DC polarises the electrodes: ions pile up at the metal-soil
interface and build an opposing voltage. The earth also carries its own
self-potential from groundwater flow and electrochemistry. Both drift
more than the signal is worth. The fix is to alternate the injection
polarity and difference the two half cycles:

```
V(+) =  V_signal + V_offset
V(-) = -V_signal + V_offset

V(+) - V(-) = 2 * V_signal
```

Any steady offset (amplifier offset, mid-rail error, self-potential)
cancels exactly, while the real signal flips sign with the current, so it
survives and doubles. An L298N H-bridge does the flipping here, at about
150 ms per half cycle.

## Why stacking works

Averaging N repeated cycles knocks random noise down by roughly sqrt(N).
Measured effect on this build: bench readings went from about 0.2 Ω of
wander to stable in the second decimal place at N = 20. At ~300 ms per
full cycle a stacked measurement takes about six seconds.

## Depth of investigation

A Wenner array senses to a depth of roughly a/2, so wider spacing images
deeper.

- One spacing gives one number: the apparent resistivity down to about
  a/2. This is what the instrument has demonstrably produced.
- Multiple spacings about the same centre (a sounding) give a 1D depth
  profile.
- A 2D section needs many electrodes and automated switching, which is
  what the 16 channel multiplexing is for. That part is only partially
  done: 12 of 16 channels work and no 2D survey has been run yet.

---

Next: [Hardware](hardware.md), the circuit that implements this and the
five design constraints that were learned the hard way.
