# 05: Single measurement

**Verifies:** the entire signal chain end to end, against a known
resistor. This is the sketch that validates the instrument.

**How to use:** wire a known resistor (100 Ω) as the target inside the
symmetric bench chain, set `AMP_GAIN` to your measured gain from
[docs/calibration.md](../../docs/calibration.md) and the `*_CHANNEL`
constants to your wiring, open the serial monitor, and send `m`.

**Expected:** about 100 Ω on a 100 Ω target. This build reported
**98.56 Ω** (1.4% error). With stacking at N = 20 the result should hold
steady in the second decimal place between runs. If you see ~0.2 Ω of
wander, something upstream is marginal; re-check the amplifier tuning
(sketch 04) before blaming noise.

**Failure signatures:**

- Negative resistance is physically impossible. The amplifier is railed
  in one polarity and producing a large fake differential. Re-centre the
  offset and reduce the gain
  ([docs/failure-log.md](../../docs/failure-log.md)).
- Plausible but wrong values that change between runs: if the code was
  modified, check that the stacking accumulators are still initialised
  to zero. Uninitialised floats contain garbage that averages into
  convincing nonsense.
