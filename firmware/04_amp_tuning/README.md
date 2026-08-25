# 04: Live amplifier tuning

**Verifies:** the AD620 operates inside its input range and its output
responds to both injection polarities.

**How to use:** wire the symmetric bench chain
([docs/calibration.md](../../docs/calibration.md)), set the `*_CHANNEL`
constants to the mux channels your chain is wired to, and watch the two
half cycle voltages while adjusting the trimmers. Follow the calibration
order: offset first, with no current flowing, then gain, then re-check
offset.

**Healthy:** the two polarities land near 1.5 V and 3.5 V, clearly
separated, no clipping against either rail, and the current column shows
a stable, plausible value.

**Failure signature:** both halves near 3.6 V means the amplifier is
saturated. That is an offset problem, not a gain problem; the gain
trimmer does nothing while the output is pinned. A floating input
produces the same 3.62 V on every mux channel
([docs/failure-log.md](../../docs/failure-log.md)).
