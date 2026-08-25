# 06 — Wenner survey

**Runs:** the full automated measurement sequence over the 12 working
electrode channels — 18 measurements across three Wenner spacings (9 at
a = 0.5 m, 6 at 1.0 m, 3 at 1.5 m) — and prints an inversion-ready
dataset in pyGIMLi's unified data format.

**How to use:** open the serial monitor, start logging to a file, then
send `s`. The survey runs from `loop()` behind a done-flag and waits for
the start command, so nothing is lost if the monitor attaches late. The
serial output **is** the data file — the sketch prints nothing else.
Capture it verbatim as `survey.dat` and feed it to
[analysis/invert_pygimli.py](../../analysis/invert_pygimli.py).

**The `CH[]` table** maps logical electrode position (0–11, twelve
equally-spaced rods along the line) to a working mux channel, so the dead
channels (0, 7, 11, 12 — breadboard contact faults) are invisible to the
survey logic and the field geometry stays uniform. If your build has
different dead channels, run sketch 03, then edit the table.

**Failure signature:** negative apparent-resistivity values are
physically impossible — the amplifier railed in one polarity and produced
a fake differential. Go back to sketch 04, re-centre the offset, reduce
the gain ([docs/failure-log.md](../../docs/failure-log.md)).

**Status note:** the sequence runs and emits well-formed data over the
working channels, but a full 2D field survey has not yet been achieved —
see the [repository README](../../README.md) for honest status, and
[docs/field-procedure.md](../../docs/field-procedure.md) for the
electrode-polarisation drift that currently bounds survey duration.
