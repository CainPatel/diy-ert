# Build guide

The physical side: what goes where, in what order, and every connection,
so the instrument can be rebuilt without the original in front of you.

Two versions exist. The breadboard build is the one every result in this
repo came from. The PCB in [hardware/](../hardware/) is designed and
DRC-clean but has not been built yet. The connection list below is the
same circuit either way; where the breadboard differs, the table says
so.

## Power

Two supplies, one ground.

| Rail | Source | Feeds |
|---|---|---|
| 5 V logic | the Arduino's USB (the Nano's 5V pin on the PCB, the Mega's on the breadboard) | muxes, shift registers, AD620, LM358, ADS1115, L298N logic (Vss and EnA) |
| Injection | a separate supply into VINJ1 (PCB) or the L298N Vs input (breadboard) | the L298N bridge only |
| Ground | common to both | everything; the L298N SENSE pins, mux enable pins, 595 output-enable pins and the ADS1115 ADDR pin all sit on it |

Power-up order, every time: logic first, confirm the ADC answers
(sketch 00), then the injection supply. Reverse it when shutting down.
With 12 V injection the resistor-chain nodes reached 6.3 V, above the
ADC's 5.3 V absolute maximum; the 10 kΩ series resistors exist for that
reason and must be in place before the injection supply is ever
connected ([hardware.md](hardware.md), constraint 1).

## Physical arrangement

The PCB floorplan, which is also the layout to copy on a breadboard:
logic on the left, the amplifier and ADC in the top-right, the injection
parts in the bottom-right, the muxes in the middle with the electrode
terminals directly below them.

```
 +------------------------------------------------------------------+
 | [Nano / Mega]                 C12   R6  [U2 AD620]    [U1        |
 |                                         [U5 LM358]     ADS1115]  |
 |          [U8 595]*     R1 R2                                     |
 | [U7 595]*    [U6 595]*          R7 R8 R9              [J3 I2C]   |
 |                                                       [J5 SPI]   |
 | [MUX A  (C1)]         [MUX B  (C2)]                              |
 |                                             R5  C10   [VINJ1]    |
 | [MUX C  (P1)]         [MUX D  (P2)]         R3        [U3 L298N] |
 |                                             R4                   |
 |   [J1  E0..E7]          [J2  E8..E15]                            |
 +------------------------------------------------------------------+
   * PCB only. The breadboard drives the mux select lines and the
     bridge directly from Mega pins.
```

The point of the arrangement is distance: the millivolt path (terminals,
mux C and D, R1/R2, AD620) stays on the far side of the board from the
L298N and its supply.

## Connection list

Taken from the KiCad netlist of the PCB. Pin numbers are the DIP or
module pins.

### Electrode bus

Each electrode terminal goes to the same-numbered input on all four
muxes.

| Electrode | Terminal | Mux input | Mux pin |
|---|---|---|---|
| E0 | J1-1 | I0 | 9 |
| E1 | J1-2 | I1 | 8 |
| E2 | J1-3 | I2 | 7 |
| E3 | J1-4 | I3 | 6 |
| E4 | J1-5 | I4 | 5 |
| E5 | J1-6 | I5 | 4 |
| E6 | J1-7 | I6 | 3 |
| E7 | J1-8 | I7 | 2 |
| E8 | J2-1 | I8 | 23 |
| E9 | J2-2 | I9 | 22 |
| E10 | J2-3 | I10 | 21 |
| E11 | J2-4 | I11 | 20 |
| E12 | J2-5 | I12 | 19 |
| E13 | J2-6 | I13 | 18 |
| E14 | J2-7 | I14 | 17 |
| E15 | J2-8 | I15 | 16 |

### Multiplexers, CD74HC4067 (DIP-24)

Common to all four: pin 24 VCC to 5 V, pin 12 GND, pin 15 (enable,
active low) to GND. Select inputs are S0 = pin 10, S1 = pin 11,
S2 = pin 14, S3 = pin 13. Pin 1 is the common.

| Mux | Role | Common (pin 1) goes to | S0-S3 from, PCB | S0-S3 from, breadboard (Mega) |
|---|---|---|---|---|
| A | C1 | L298N OUT1 | U6 QA, QB, QC, QD (pins 15, 1, 2, 3) | 22, 24, 26, 28 |
| B | C2 | shunt R3, and R4 to ADS1115 A3 | U6 QE, QF, QG, QH (pins 4, 5, 6, 7) | 30, 32, 34, 36 |
| C | P1 | R1 (10 kΩ) to AD620 +IN | U7 QA, QB, QC, QD | 38, 40, 42, 44 |
| D | P2 | R2 (10 kΩ) to AD620 -IN | U7 QE, QF, QG, QH | 46, 48, 50, 52 |

### Shift registers, 74HC595 (DIP-16), PCB only

| Pin | U6 | U7 | U8 |
|---|---|---|---|
| 14 SER | Nano D4 | U6 pin 9 (QH') | U7 pin 9 (QH') |
| 11 SRCLK | Nano D2 | Nano D2 | Nano D2 |
| 12 RCLK | Nano D3 | Nano D3 | Nano D3 |
| 13 OE (active low) | GND | GND | GND |
| 10 SRCLR (active low) | 5 V | 5 V | 5 V |
| 16 VCC, 8 GND | 5 V, GND | 5 V, GND | 5 V, GND |
| 15, 1, 2, 3 (QA-QD) | MUX A S0-S3 | MUX C S0-S3 | L298N IN1 (QA), IN2 (QB); QC, QD unused |
| 4, 5, 6, 7 (QE-QH) | MUX B S0-S3 | MUX D S0-S3 | unused |
| 9 QH' | U7 SER | U8 SER | unused |

### Injection, L298N (Multiwatt-15)

| Pin | Name | Goes to |
|---|---|---|
| 1 | SENSE A | GND |
| 2 | OUT1 | MUX A common |
| 3 | OUT2 | shunt R3 (1 kΩ), and R5 (10 kΩ) to ADS1115 A2 |
| 4 | Vs | VINJ1 +, with C10 (100 µF) and C8 (100 nF) to GND |
| 5 | IN1 | U8 QA (PCB); Mega pin 6 (breadboard) |
| 6 | EnA | 5 V |
| 7 | IN2 | U8 QB (PCB); Mega pin 7 (breadboard) |
| 8 | GND | GND |
| 9 | Vss | 5 V (logic supply) |
| 10, 12 | IN3, IN4 | not connected |
| 11 | EnB | GND |
| 13, 14 | OUT3, OUT4 | not connected |
| 15 | SENSE B | GND |

The shunt R3 sits between OUT2 and MUX B's common. The ADS1115 reads
across it: A2 on the OUT2 side through R5, A3 on the mux side through
R4. On the breadboard the L298N is a driver module; keep its ENA jumper
fitted and leave bridge B unused.

### Potential sense, AD620 (DIP-8)

| Pin | Name | Goes to |
|---|---|---|
| 1, 8 | RG | R6, 1 kΩ (PCB, gain 50.4 nominal); 10 kΩ trimmer (breadboard, measured 76) |
| 2 | -IN | R2 (10 kΩ) from MUX D common |
| 3 | +IN | R1 (10 kΩ) from MUX C common |
| 4 | -VS | GND |
| 5 | REF | LM358 pin 1, with C11 (100 nF) to GND |
| 6 | OUT | R9 (10 kΩ) to ADS1115 A0 |
| 7 | +VS | 5 V |

### Reference, LM358 (DIP-8)

| Pin | Name | Goes to |
|---|---|---|
| 1 | OUT A | AD620 REF (the 2.5 V reference) |
| 2 | -IN A | pin 1 (unity-gain follower) |
| 3 | +IN A | junction of R7 (10 kΩ to 5 V) and R8 (10 kΩ to GND); the breadboard has the offset trimmer here |
| 4 | V- | GND |
| 5 | +IN B | GND |
| 6 | -IN B | pin 7 |
| 7 | OUT B | pin 6 (unused half, parked as a follower) |
| 8 | V+ | 5 V |

### ADC, ADS1115 breakout

| Module pin | Goes to |
|---|---|
| VDD | 5 V |
| GND | GND |
| SCL | Nano A5 (Mega 21), and J3-4 |
| SDA | Nano A4 (Mega 20), and J3-3 |
| ADDR | GND, giving address 0x48 |
| ALRT | not connected |
| A0 | R9 (10 kΩ) from AD620 OUT |
| A1 | not connected |
| A2 | R5 (10 kΩ) from L298N OUT2 |
| A3 | R4 (10 kΩ) from MUX B common |

On the PCB the breakout plugs into the 10-pin socket U1, whose pin order
follows the ADS1115 chip rather than the breakout header. Read review
note 1 in [hardware/README.md](../hardware/README.md) before fitting it.

### Decoupling

100 nF between VCC and GND at every IC (C1-C9, C13-C18). C12, 10 µF, on
the 5 V rail by the Nano. C10, 100 µF, and C8, 100 nF, on the injection
supply at the L298N. C11, 100 nF, on the reference node.

### Headers, PCB only

| Ref | Pins |
|---|---|
| J3 | GND, 5 V, SDA, SCL (I2C, for a display or a second sensor) |
| J5 | 5 V, GND, MISO (D12), MOSI (D11), SCK (D13), CS (D10) (SPI, unused by the firmware; an SD card logger is the obvious use) |
| VINJ1 | injection supply +, GND |

## Assembly order, PCB

1. Bare board: with a multimeter, 5 V to GND must be open circuit, and
   VINJ1 + to GND and to 5 V both open.
2. Lowest parts first: resistors, then DIP sockets (leave the chips out),
   ceramic capacitors, the pin sockets U1, J3 and J5, the Nano's two
   header rows, the electrolytics (C12 positive to 5 V; C10 positive to
   the VINJ1 side, see review note 4), the screw terminals, and the L298N
   last, standing upright at the board edge. At a milliampere of
   injection current it needs no heatsink.
3. Power over USB with no chips fitted. Confirm 5 V at the VCC pin of
   every socket and 0 V at every GND pin. Do not skip this; a solder
   bridge found now costs nothing, found later it costs a chip.
4. Fit chips in bring-up order and test each stage before the next, as in
   [software.md](software.md): Nano and ADS1115 breakout (sketch 00);
   shift registers and L298N with the injection supply still off (sketch
   07, `+` `-` `p`, meter on the L298N inputs); muxes (`c N`, continuity
   from each mux common pin to terminal E N); AD620 and LM358 (`t` with
   the bench chain); then `m` on the 100 Ω reference. Only then connect
   the injection supply to VINJ1.

## Assembly order, breadboard

The same idea with jumper wires. What the failure log says about the
breadboard build, condensed:

- Solder the header pins on every breakout. Press-fit pins pass a visual
  check and fail electrically.
- Continuity-test jumpers before use. One failed inside its insulation
  and cost hours.
- Mux channels 0, 7, 11 and 12 on the original build were dead from
  breadboard contact faults, not chip faults. Seat the DIP-24 muxes
  firmly and scan them (sketch 03) before trusting any channel.
- Put the injection wiring on the opposite side of the board from the
  amplifier, and keep the P1/P2 wires short.
- Fit the 10 kΩ series resistors on the ADC inputs before the injection
  supply is ever connected. Four ADS1115s died without them.
- Build one subsystem at a time, in the firmware order, and verify each
  against a multimeter before adding the next.

## Electrode line

Sixteen rods (or however many channels you use), equally spaced by `a`,
numbered along the line, each on its own conductor to terminal E0-E15 in
order. Drive rods no deeper than about a/10. Run the current-pair and
potential-pair conductors as separate bundles rather than one ribbon, so
the switching edges on the current wires do not couple into the
millivolt signal. Steel rods polarise within minutes; for anything longer
than a single reading, the potential pair should be non-polarising
copper-sulphate electrodes ([field-procedure.md](field-procedure.md)).

## Parts not on the PCB

| Part | Qty | Notes |
|---|---|---|
| Stainless steel rods, 30-50 cm | 4-16 | threaded rod or tent stakes |
| Wire to the rods | 16 conductors | plus crocodile clips or ring terminals |
| 12 V supply or battery | 1 | injection; a bench supply at 5 V was used for the bench validation |
| USB cable | 1 | logic power and serial |
| Multimeter | 1 | every step of the bring-up depends on it |
| Breadboard build only: 10 kΩ multi-turn trimmers | 2 | gain and offset |
| Breadboard build only: breadboards and jumpers | 3-4 | |

## Before first power-up

- [ ] 5 V to GND open circuit
- [ ] Injection + to GND open, and not connected to 5 V anywhere
- [ ] Every chip's notch matches the socket and silkscreen
- [ ] Electrolytic polarity
- [ ] 10 kΩ resistors in place on every ADC input
- [ ] ADS1115 breakout header order matches the socket (PCB)
- [ ] Breakout headers soldered (breadboard)
- [ ] Jumpers continuity-tested (breadboard)
- [ ] Injection supply disconnected until sketch 00 passes
