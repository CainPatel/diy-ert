# 05 — Single measurement

**Verifies:** the entire signal chain end to end, against a known
resistor. This is the sketch that validates the instrument.

**How to use:** wire a known resistor (100 Ω) as the target inside the
symmetric bench chain, set `AMP_GAIN` to your measured gain from
[docs/calibration.md](../../docs/calibration.md) and the `*_CHANNEL`
constants to your wiring, open the serial monitor, and send `m`.

**Expected:** ~100 Ω on a 100 Ω target. This build reported **98.56 Ω**
(1.4% error). With stacking at N = 20 the result should be stable in the
second decimal place between runs; ~0.2 Ω of wander means something
upstream is marginal — re-check the amplifier tuning (sketch 04) before
blaming noise.

**Failure signatures:**

- **Negative resistance** is physically impossible: the amplifier is
  railed in one polarity and producing a huge fake differential.
  Re-centre the offset, reduce the gain
  ([docs/failure-log.md](../../docs/failure-log.md)).
- **Plausible but wrong values that change between runs:** if the code
  was modified, check that the stacking accumulators are still
  initialised to zero — uninitialised floats contain garbage that
  averages into convincing-looking nonsense.
