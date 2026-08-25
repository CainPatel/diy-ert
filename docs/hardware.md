# Hardware

The signal chain, in the order the signal flows through it, plus enough
wiring detail to reproduce the breadboard build. The KiCad schematic
belongs in [hardware/kicad/](../hardware/kicad/).

## Signal chain

| Stage | Part | Role |
|---|---|---|
| Injection | L298N H-bridge | alternates polarity, driven from two Arduino pins |
| Current sense | 1 kΩ shunt, read differentially on ADS1115 A2/A3 | measures the actual injected current |
| Potential sense | AD620 instrumentation amplifier | rejects large common-mode, amplifies the mV differential |
| Reference | LM358 unity-gain buffer on a 10k/10k divider | holds the AD620 REF pin at mid-rail (2.5 V) |
| Digitisation | ADS1115, 16-bit, I2C at 0x48 | reads the amp output on A0 |
| Switching | 4x CD74HC4067, 16 channels each | selects which four of 16 electrodes act as C1/C2/P1/P2 |

## Why four multiplexers

Each measurement needs four electrodes in different roles: two carrying
current, two sensing potential. One mux per role means any electrode can
play any part, which an automated survey needs, since the same rod is a
current electrode in one measurement and a potential electrode in the
next. The cost is control pins: 4 muxes x 4 address lines + 2 bridge
inputs = 18 Arduino pins. The pin map is in
[firmware/README.md](../firmware/README.md).

## Design constraints, learned the hard way

> [!CAUTION]
> Everything in this list was learned by breaking something or by
> measuring nonsense. Item 1 cost four chips.

1. **ADC input limits.** The ADS1115 absolute maximum input is
   VDD + 0.3 V, about 5.3 V. Chain nodes were measured at 6.3 V with 12 V
   injection. Every line crossing from the injection domain into 5 V
   logic gets a 10 kΩ series resistor. Without one, fault current is
   limited only by the chain and the chip's bond wires; with one, about
   170 nA.

2. **Amplifier input range.** The AD620's usable input window is
   -VS + 1.9 V to +VS - 1.4 V. On a 5 V single supply that is roughly
   1.9 V to 3.6 V. Sense nodes outside this window produce readings that
   look plausible and mean nothing.

3. **Symmetric resistor chains.** An asymmetric chain (100 kΩ one side,
   10 kΩ the other) puts the sense nodes at wildly different potentials
   depending on injection polarity: 6.67 V one way, 1.32 V the other, out
   of the amp's window in at least one polarity. A symmetric chain keeps
   both nodes near mid-rail in both polarities.

4. **A single supply amplifier cannot output negative voltages.** With
   REF at 0 V the negative half cycle clamps at the output floor and both
   halves read nearly the same, so the differencing cancels the signal
   along with the offset. REF has to sit at mid-rail. That is what the
   LM358 buffer is for.

5. **Measure, never assume.** The L298N's voltage drop depends on
   current. The nominal 5 V supply delivered 3.8 V across OUT1-OUT2 at
   the working current, and using the nominal value gave a 45% error in
   computed resistance.

## Wiring reference

### Injection path

```
 12 V ----+
          |
      +--------+  OUT1            +-------+
      | L298N  |----------------->| MUX A |-----> C1 rod
 IN1->|        |                  +-------+          |
 IN2->|        |                                  [ earth ]
      |        |  OUT2   1 kΩ     +-------+          |
      |        |<-------/\/\/-----| MUX B |<----- C2 rod
      +--------+        shunt     +-------+
                          |
                 ADS1115 A2 / A3 (differential, via 10 kΩ each)
```

Current leaves OUT1, goes through mux A to the C1 rod, through the
ground, back in at C2, through mux B and the shunt, and returns to OUT2.
Driving IN1/IN2 opposite ways sets the polarity; both low parks the
bridge with no drive.

### Sense path

```
 P1 rod --> MUX C --[10 kΩ]--> IN+ (3) __
                                         \
                                    AD620 >-- OUT (6) --[10 kΩ]--> ADS1115 A0
                                       __/
 P2 rod --> MUX D --[10 kΩ]--> IN- (2)
                               REF (5) <-- LM358 buffer <-- 2.5 V divider
```

### Mid-rail reference

```
 5 V --[10 kΩ]--+--[10 kΩ]-- GND
                |
                +--> LM358 IN+, unity gain --> AD620 REF
```

The AD620 REF pin needs a low impedance drive; a bare divider is not
stiff enough, hence the buffer. The offset trim adjusts this node and the
gain trim is the RG resistance between AD620 pins 1 and 8. The two
interact, which is why the order in [calibration.md](calibration.md)
matters.

### AD620 (DIP-8)

| Pin | Name | Goes to |
|---|---|---|
| 1, 8 | RG | gain trimmer |
| 2 | IN- | MUX D common, via 10 kΩ |
| 3 | IN+ | MUX C common, via 10 kΩ |
| 4 | -VS | GND |
| 5 | REF | LM358 buffer output, 2.5 V |
| 6 | OUT | ADS1115 A0, via 10 kΩ |
| 7 | +VS | 5 V |

### ADS1115 breakout

| Pin | Goes to |
|---|---|
| VDD, GND | 5 V, GND |
| SDA, SCL | Mega 20, 21 |
| ADDR | GND (address 0x48) |
| A0 | AD620 OUT, via 10 kΩ |
| A1 | unused |
| A2, A3 | across the 1 kΩ shunt, via 10 kΩ each |

Solder the breakout headers. Press-fit pins were good for several days of
intermittent I2C on this build.

### CD74HC4067 (x4)

All four muxes are wired the same on the electrode side: the 16 channel
pins go to the 16 electrode headers, EN to GND, S0-S3 to the Arduino (pin
map in [firmware/README.md](../firmware/README.md)). Only the common pin
differs per mux: A to bridge OUT1, B to the shunt, C to AD620 IN+, D to
AD620 IN-.

---

Next: [Calibration](calibration.md), putting the amplifier in range and
measuring the gain.
