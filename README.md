# DIY Electrical Resistivity Tomography

A four-electrode (Wenner array) earth-resistivity meter built from discrete
parts in about a month, for roughly $150. Commercial instruments that make
the same measurement start above $20,000.

This repository is an engineering log, not a product. Four ADS1115 ADC chips
were destroyed during development — all by the same root cause, a
voltage-domain mismatch — and the [failure log](docs/failure-log.md)
explaining that and every other failure is the most useful document here.
Read it before you power anything.

## What it demonstrably does

- **Bench validation: 98.6 Ω measured against a known 100 Ω resistor
  (1.4% error).** The instrument computed this from its own current and
  voltage readings, using a gain value measured with a multimeter rather
  than taken from the trimmer setting.
- **Field result: 71–72 Ω·m apparent resistivity at 0.5 m Wenner spacing,
  in moist Georgia clay soil, with 1.24 mA injected.**

## What it does not do

- **It has not produced a 2D image.** Current status: a validated
  single-point measurement and one successful field reading.
- 16-channel electrode multiplexing is built and partially working — 12 of
  16 channels conduct. The four dead channels are breadboard contact
  faults, not chip faults (see the failure log).
- The automated Wenner survey firmware exists and emits inversion-ready
  data over the working channels, but a 2D survey has not yet been
  achieved.

## Why four electrodes

The contact resistance where a steel rod meets soil measured 40–80 kΩ on
this build. A two-electrode measurement puts that resistance directly in
series with the ground resistance being measured — tens of kilohms swamping
tens of ohms. The four-electrode arrangement splits the roles: current is
injected through one pair (C1, C2) and potential is measured across a
separate pair (P1, P2). The potential pair feeds an instrumentation
amplifier and draws essentially no current, so no voltage drops across its
contact resistance — it cancels out of the result entirely. The current
pair's contact resistance only limits how much current flows, and the
current is measured directly across a shunt rather than assumed. This is
the central idea of the whole method.

## Signal path

```
Arduino Mega ──> L298N H-bridge ──> MUX A ──> C1 electrode
                                                   │
                                                 [earth]
                                                   │
             ADS1115 <── AD620 <── MUX C/D <── P1, P2 electrodes
                 ↑                                 │
                 └── 1 kΩ shunt <── MUX B <── C2 electrode
```

The H-bridge alternates the injection polarity; the actual injected current
is measured across a 1 kΩ shunt; the millivolt-level potential difference
between P1 and P2 is amplified by the AD620 (gain 76 — measured, not
nominal) and digitised by the 16-bit ADS1115. Four 16-channel multiplexers
let any of 16 electrodes play any of the four roles.

## Reading order

The documentation is written to be read in sequence:

1. [Theory](docs/theory.md) — Ohm's law through the ground, Wenner
   geometry, why the polarity alternates, why stacking works
2. [Hardware](docs/hardware.md) — the signal chain, and five design
   constraints learned the hard way
3. [Calibration](docs/calibration.md) — the offset/gain procedure in the
   order that works, with the actual numbers from this build
4. [Failure log](docs/failure-log.md) — symptom → root cause → fix, for
   every failure including the four dead ADCs
5. [Field procedure](docs/field-procedure.md) — how the field reading was
   taken, and what limits survey duration
6. [Firmware](firmware/README.md) — seven sketches that bring the
   instrument up one verified subsystem at a time. **Run them in order.**

## Repository layout

```
diy-ert/
├── docs/          theory, hardware, calibration, failure log, field procedure
├── firmware/      seven bring-up sketches, 00 through 06 — run in order
├── hardware/      PCB status, bill of materials, KiCad files
├── analysis/      pyGIMLi inversion script and data-format notes
└── data/          data format spec and the one validated field reading
```

## Cost

Roughly $150 in parts, including spares. The full bill of materials — with
notes on which parts to buy in DIP packages so they can be socketed and
swapped — is in [hardware/README.md](hardware/README.md).

## License

MIT — see [LICENSE](LICENSE).
