# Data

## Format

Two kinds of records belong in this directory:

**Survey files (`*.dat`):** the verbatim serial capture from firmware
sketch 06, in pyGIMLi's unified data format. The layout is documented in
[analysis/README.md](../analysis/README.md).

**Single manual readings:** commented blocks, one per measurement,
carrying every raw value needed to recompute the result:

```
# Wenner array, a = 0.5 m, Georgia clay soil, moist
# I = 1.24 mA, V(P1-P2) = 28.5 mV
# R = 22.98 Ω, ρ = 2πaR = 72.20 Ω·m
```

Each number states how it was obtained. `I` is measured across the 1 kΩ
shunt. `V(P1-P2)` is the stacked, offset-cancelled amplifier reading
divided by the measured gain. `R = V/I`. `ρ = 2πaR` is computed from
those, not measured.

## The one validated field reading

The block above is it, the single successful field measurement this
instrument has produced: Wenner array at a = 0.5 m in moist Georgia
clay, 1.24 mA injected, 28.5 mV across the potential pair, giving
R = 22.98 Ω and an apparent resistivity of 72.20 Ω·m.

## Known caveat: polarisation drift

Readings drifted from 71 to 90 Ω·m over several minutes as the steel
potential electrodes polarised. The 71-72 Ω·m value is the prompt
reading, taken before drift set in. Non-polarising copper sulphate
electrodes are the standard fix and are cheap to build. Until then, the
drift bounds how long a survey can run before it turns into a false
gradient in the data. Details in
[docs/field-procedure.md](../docs/field-procedure.md).
