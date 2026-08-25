# 03 — Mux channel scan

**Verifies:** address decoding and channel continuity on all four
CD74HC4067 multiplexers.

**How to use:** the sketch steps all four muxes through channels 0–15
together, two seconds per channel, announcing each on serial. For each
announced channel, continuity-beep from the mux's common pin to the
corresponding electrode header. Record every channel that fails.

**Failure signature:** all channels reading identically at the ADC. Read
the mux common pin directly with the multimeter, not through the
amplifier — a saturated amp makes every channel look the same regardless
of what the mux does (see the 3.62 V entry in
[docs/failure-log.md](../../docs/failure-log.md)).

**From this build:** channels 0, 7, 11 and 12 were dead — breadboard
contact faults, not chip or addressing faults. The diagnostic that ruled
out the chips: in binary the dead channels are 0000, 0111, 1011 and 1100,
which share no common address bit, so no single stuck address line can
explain them. The survey firmware routes around dead channels with the
`CH[]` lookup table ([firmware/README.md](../README.md)).
