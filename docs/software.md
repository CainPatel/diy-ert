# Software guide

How to install the toolchain, put firmware on the board, take a
measurement, capture a survey to a file, and invert it. The per-sketch
READMEs in [firmware/](../firmware/) say what each sketch verifies and
what its failure looks like; this page is the end-to-end walkthrough.

## What runs where

| Build | Board | Firmware |
|---|---|---|
| Breadboard (the validated build) | Arduino Mega 2560 | sketches 00 through 06, in order |
| Custom PCB | Arduino Nano | sketch 00, then 07 (one sketch, serial commands) |

The analysis script runs on a laptop in Python, after a survey.

## Toolchain

### Arduino IDE

1. Install Arduino IDE 2.x from arduino.cc. The AVR board package (Mega,
   Nano) comes with it.
2. Library Manager (books icon in the left sidebar): search
   "Adafruit ADS1X15", install, and accept the prompt to also install
   Adafruit BusIO. Sketches 00, 02 and 03 need no libraries at all.
3. Tools > Board: "Arduino Mega or Mega 2560" for the breadboard,
   "Arduino Nano" for the PCB. For a Nano clone also set
   Tools > Processor to "ATmega328P (Old Bootloader)". If an upload fails
   with "programmer is not responding", try the other bootloader option.
4. Tools > Port: the board's serial port. macOS: `/dev/cu.usbserial-*` or
   `/dev/cu.usbmodem*`. Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`. Windows:
   `COMn`. Clones with a CH340 USB chip may need the CH340 driver on
   Windows and older macOS.
5. File > Open the sketch's `.ino`, then the upload arrow.

If the IDE refuses a sketch folder whose name starts with a digit, rename
the folder and the `.ino` together (for example `s05_single_measurement`)
and keep the number.

### arduino-cli

The same thing from a terminal. These are the commands the sketches in
this repo were compile-tested with; substitute your port.

```
arduino-cli core install arduino:avr
arduino-cli lib install "Adafruit ADS1X15"

# breadboard build, Mega
arduino-cli compile --fqbn arduino:avr:mega firmware/05_single_measurement
arduino-cli upload  --fqbn arduino:avr:mega -p /dev/cu.usbserial-1420 firmware/05_single_measurement

# PCB, Nano clone with the old bootloader
arduino-cli compile --fqbn arduino:avr:nano:cpu=atmega328old firmware/07_pcb_console
arduino-cli upload  --fqbn arduino:avr:nano:cpu=atmega328old -p /dev/cu.usbserial-1420 firmware/07_pcb_console

arduino-cli monitor -p /dev/cu.usbserial-1420 -c baudrate=9600
```

### Serial monitor

9600 baud, everywhere. Commands are single characters, so the line-ending
setting does not matter. Opening the port resets the board; wait for the
banner before typing. Any terminal works: the IDE's Serial Monitor,
`arduino-cli monitor`, or `screen /dev/cu.usbserial-1420 9600` (quit with
Ctrl-A then K).

## Bring-up walkthrough, breadboard

In order. Do not wire the next stage until the current one passes. Each
step: what must be wired, what to type, what you should see, and where to
look if you do not.

**00, I2C scanner.** Wired: Mega, ADS1115 breakout, SDA (20), SCL (21),
5 V, GND. Nothing to type. Expect `Device found at 0x48` every five
seconds. If nothing is found: power at the breakout, SDA/SCL, and the
header solder joints, in that order.

**01, raw ADC.** Wired: as above. Nothing to type; it prints A0, A1 and
A2-A3 twice a second. Put the 2.5 V mid-rail on A0 and check the printed
volts against a multimeter on the same node. Expect agreement within a
few millivolts. A2-A3 near zero with current flowing means the shunt
sense wires are off.

**02, DC hold.** Wired: add the L298N, its supply, the shunt, and the
bench chain. Type `+`, `-`, or `p`. The bridge holds that state so a
multimeter can read it (a multimeter cannot follow the 150 ms square wave
the real measurement uses). Trace the voltage along the injection path
and find where it stops. Measure the actual voltage across OUT1-OUT2 at
your working current; it was 3.8 V, not 5 V, on this build.

**03, mux scan.** Wired: add the four muxes and their 16 select lines.
Nothing to type; it steps all four muxes through channels 0-15, two
seconds each, announcing the channel. Continuity-beep from each mux's
common pin to the matching electrode header. Write down every channel
that fails; they go in `CH[]` later. If all channels look the same at the
ADC, measure at the mux common pin, not through the amplifier.

**04, amplifier tuning.** Wired: add the AD620, the LM358 reference, and
the trimmers; set the `*_CHANNEL` constants to your bench wiring. Nothing
to type; it prints the forward and reverse half-cycle voltages
continuously. Follow [calibration.md](calibration.md): offset first with
no current, then gain, then offset again. Healthy looks like

```
FWD 3.5483 V   REV 0.8205 V   diff 2.7278 V   I 0.182 mA
```

Both halves near 3.6 V means saturated; fix the offset, not the gain.

**05, single measurement.** Wired: everything, with the 100 Ω reference
in the bench chain. Set `AMP_GAIN` to your measured gain. Type `m`.
Expect twenty cycle lines and then a summary; on this build the numbers
were

```
cycle 20/20  V+ 3.548  V- 0.821  I 0.1818 mA
---
V(P1-P2) = 17.9 mV
I        = 0.1818 mA
R        = 98.56 ohm
```

A negative R means the amp is railed in one polarity. A result that
changes by tenths of an ohm between runs means something upstream is
marginal.

**06, Wenner survey.** Wired: the electrode line, via the working
channels in `CH[]`. Type `s` after the port has settled. It prints a
complete pyGIMLi data file and nothing else, then parks the bridge. See
"Capturing a survey" below before you type `s`.

## Bring-up walkthrough, PCB

Sketch 00 first, unchanged. Then sketch 07 and its commands, in this
order: `r` (ADC reads), `+` `-` `p` (bridge switches, confirm at the
L298N pins with a multimeter), `c N` (mux continuity, terminal E N), `t`
(amplifier in range on the bench chain), `m` (known resistor), `w`
(survey). The [07 README](../firmware/07_pcb_console/README.md) has the
command table and the order in which to fit the chips.

## Configuring the firmware

Everything adjustable is a `const` near the top of each sketch.

| Constant | Where | Meaning | Change it when |
|---|---|---|---|
| `AMP_GAIN` | 05, 06, 07 | measured amplifier gain | after every calibration; 76 on the breadboard build, 50.4 nominal on the PCB |
| `SHUNT_OHMS` | 04-07 | shunt resistance | measure the actual resistor and use that |
| `N_STACK` | 05-07 | polarity cycles averaged per measurement | 20 gives second-decimal stability; more is slower, not much better |
| `SETTLE_MS` | 04-07 | wait after each polarity flip before reading | leave at 100 unless the amp output has not settled (check with `t`) |
| `ELECTRODE_SPACING` | 06, 07 | metres between adjacent rods | your line |
| `CH[]` | 06, 07 | electrode position to mux channel | after the mux scan finds dead channels |
| `*_CHANNEL` | 04, 05 | which mux channel each bench node is on | your bench wiring |
| Pin constants | all | Mega or Nano pins | never, unless you rewire |

## Taking a field measurement

1. Follow [field-procedure.md](field-procedure.md) for the line, the
   contact check, and power-up order.
2. Sketch 05 (or 07 with `x` to pick channels): type `m`, wait about six
   seconds.
3. Write down I, V(P1-P2) and R from the summary, plus spacing, soil, and
   moisture, in the format in [data/README.md](../data/README.md). Record
   the raw values, not just the final resistivity.
4. Apparent resistivity is 2πa·R. For the validated reading:
   2π x 0.5 m x 22.98 Ω = 72.2 Ω·m.
5. Take the number promptly. Readings drift upward within minutes as the
   steel electrodes polarise.

## Capturing a survey to a file

The survey sketches print the data file over serial. Getting it into a
file:

- **Sketch 06** prints only the data, so any capture of the serial
  output is the file. Open the capture first, then type `s`.
- **Sketch 07** also prints its banner and command replies. Capture
  everything, then run `python analysis/clean_capture.py capture.txt
  survey.dat`, which keeps just the data block.

Ways to capture:

```
# arduino-cli: everything the board prints goes to the file, typing still works
arduino-cli monitor -p /dev/cu.usbserial-1420 -c baudrate=9600 | tee capture.txt

# screen (macOS, Linux)
screen -L -Logfile capture.txt /dev/cu.usbserial-1420 9600
```

In the Arduino IDE, select all the text in the Serial Monitor output pane
after the survey ends and paste it into a file. On Windows, PuTTY's
Session > Logging > "Printable output" does the same job.

A survey over 12 electrodes takes about 18 x 6 s, roughly two minutes;
over 16 electrodes (sketch 07 default) about 35 x 6 s.

## Running the inversion

```
conda create -n ert -c gimli -c conda-forge pygimli
conda activate ert
cd analysis
python invert_pygimli.py survey.dat        # lam = 20
python invert_pygimli.py survey.dat 50     # smoother
```

A window opens with the inverted resistivity section; close it to exit.
What to check before believing it: every `rhoa` in the file is positive
and in a sensible range (tens of Ω·m for the clay this was built on);
electrode numbers in the data rows are 1-indexed; and the picture does
not change character when you double `lam`. With 18-35 data points the
regularisation is doing most of the work. See
[analysis/README.md](../analysis/README.md).

## Troubleshooting

| Symptom | Look at |
|---|---|
| `ADS1115 not found` at boot | sketch 00; power, SDA/SCL, solder |
| Upload fails, "programmer is not responding" | wrong board or bootloader selection; wrong port; CH340 driver |
| Sketch folder rejected by the IDE | rename folder and `.ino` together, keep the number |
| Both half-cycles the same voltage | amp saturated or input floating; [failure log](failure-log.md), 3.62 V entry |
| Negative resistance | amp railed in one polarity; recentre offset (breadboard) or reduce injection voltage (PCB) |
| R stable but wrong | asymmetric chain, or nominal injection voltage assumed; [calibration.md](calibration.md) |
| Multimeter says no current while measuring | it cannot track the square wave; sketch 02 or `+`/`-` |
| Some electrodes never conduct | mux scan; edit `CH[]` |
| Survey capture has junk before the data | `analysis/clean_capture.py` |
| Inversion shows structure that vanishes at higher `lam` | it was noise; keep `lam` high |
