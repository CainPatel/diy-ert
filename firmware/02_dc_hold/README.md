# 02: DC hold

**Verifies:** the L298N H-bridge switches, and gives a multimeter a
signal it can actually read.

**Why it exists:** a multimeter cannot track a 150 ms square wave. It
averages toward zero and reports "no current" while the bridge is
driving perfectly well. That false reading cost real time on this build
([docs/failure-log.md](../../docs/failure-log.md)). Park the bridge in
DC and the multimeter is trustworthy again.

**How to use:** serial commands `+` (forward), `-` (reverse), `p`
(park). Hold a polarity and trace the voltage along the injection path
(supply, bridge OUT1/OUT2, mux input, electrode) until it disappears. A
jumper wire that had failed internally with no visible damage was found
exactly this way.

**Also worth doing here:** measure the actual voltage across OUT1-OUT2
at your working current. On this build the nominal 5 V supply delivered
only 3.8 V. The L298N's drop is current-dependent, and assuming the
nominal value produced a 45% error in computed resistance
([docs/hardware.md](../../docs/hardware.md), constraint 5).
