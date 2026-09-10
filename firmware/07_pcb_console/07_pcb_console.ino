// 07: PCB console
// Firmware for the custom PCB (hardware/kicad): an Arduino Nano driving
// the four multiplexers and the L298N through three chained 74HC595
// shift registers. One sketch, driven by serial commands, covering the
// same bring-up steps as sketches 01-06 on the breadboard build. Sketch
// 00 (I2C scanner) works unchanged on the Nano.
//
// Commands, 9600 baud:
//   ?           help
//   r           raw ADC readout once (A0 amp output, A2-A3 shunt)
//   p           park the bridge (both L298N inputs low)
//   +           hold the bridge forward, DC, for multimeter work
//   -           hold the bridge reverse
//   c N         put all four muxes on channel N (0-15), for continuity checks
//   x A B C D   set mux channels individually: C1=A, C2=B, P1=C, P2=D
//   t           live amplifier readout, alternating polarity; any key stops
//   m           one stacked measurement on the current channels, prints R
//   w           Wenner survey over CH[], prints a pyGIMLi data file
//
// Shift register chain (hardware/README.md): Nano D4 -> U6 -> U7 -> U8.
//   U6  QA-QD = MUX A S0-S3,  QE-QH = MUX B S0-S3
//   U7  QA-QD = MUX C S0-S3,  QE-QH = MUX D S0-S3
//   U8  QA = L298N IN1,       QB = L298N IN2
// The 595 outputs are live from power-on (~OE is tied to ground on the
// board) and their power-up contents are undefined, so setup() latches a
// parked, known state before doing anything else.

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

const int PIN_SRCLK = 2;  // 74HC595 SRCLK, all three registers
const int PIN_RCLK  = 3;  // 74HC595 RCLK, all three registers
const int PIN_SER   = 4;  // 74HC595 SER of U6, first in the chain

const int N_STACK = 20;
const int SETTLE_MS = 100;            // half-cycle ~150 ms including the read
const float SHUNT_OHMS = 1000.0;      // R3; measure the real value
const float ELECTRODE_SPACING = 0.5;  // metres between adjacent electrodes

// AD620 gain is fixed by R6 = 1k on the board: G = 1 + 49.4k/R6 = 50.4
// nominal. Measure it (docs/calibration.md) and put the measured value
// here. There is no trimmer on the PCB; change R6 to change the gain.
const float AMP_GAIN = 50.4;

// Electrode position (0-15 along the line) -> mux channel. All 16 channels
// are expected to work on the PCB; the breadboard's dead channels were
// contact faults. Edit this table if the 'c' scan finds a bad channel.
const int CH[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const int N_ELECTRODES = 16;

Adafruit_ADS1115 ads;

byte regAB = 0;      // U6: MUX A channel in bits 0-3, MUX B in bits 4-7
byte regCD = 0;      // U7: MUX C channel in bits 0-3, MUX D in bits 4-7
byte regBridge = 0;  // U8: bit 0 = IN1, bit 1 = IN2

int curC1 = 0, curC2 = 3, curP1 = 1, curP2 = 2;

float lastVMN = 0.0;
float lastI = 0.0;
bool lastSuspect = false;

// ---------------------------------------------------------------- outputs

void latch() {
  digitalWrite(PIN_RCLK, LOW);
  // The first byte shifted out travels furthest and lands in U8.
  shiftOut(PIN_SER, PIN_SRCLK, MSBFIRST, regBridge);
  shiftOut(PIN_SER, PIN_SRCLK, MSBFIRST, regCD);
  shiftOut(PIN_SER, PIN_SRCLK, MSBFIRST, regAB);
  digitalWrite(PIN_RCLK, HIGH);
}

void setMuxes(int c1, int c2, int p1, int p2) {
  curC1 = c1; curC2 = c2; curP1 = p1; curP2 = p2;
  regAB = (c1 & 0x0F) | ((c2 & 0x0F) << 4);
  regCD = (p1 & 0x0F) | ((p2 & 0x0F) << 4);
  latch();
}

void bridgeForward() { regBridge = 0b01; latch(); }  // OUT1 high: C1 positive
void bridgeReverse() { regBridge = 0b10; latch(); }
void bridgePark()    { regBridge = 0b00; latch(); }  // no drive between measurements

// ----------------------------------------------------------------- inputs

float readAmpVolts() {
  return ads.computeVolts(ads.readADC_SingleEnded(0));
}

float readShuntAmps() {
  return fabs(ads.computeVolts(ads.readADC_Differential_2_3())) / SHUNT_OHMS;
}

// Two signs of a bad reading, both seen on the breadboard build: the two
// half-cycles reading nearly the same voltage (amp saturated or an input
// floating), or the output sitting within 0.2 V of a rail.
bool suspect(float vPlus, float vMinus) {
  if (fabs(vPlus - vMinus) < 0.02) return true;
  if (vPlus < 0.2 || vPlus > 4.8 || vMinus < 0.2 || vMinus > 4.8) return true;
  return false;
}

// ------------------------------------------------------------ measurement

float measureOhms(bool verbose) {
  float vSum = 0.0;  // accumulators initialised; uninitialised floats are garbage
  float iSum = 0.0;
  lastSuspect = false;

  for (int n = 1; n <= N_STACK; n++) {
    bridgeForward();
    delay(SETTLE_MS);
    float vPlus = readAmpVolts();
    float iPlus = readShuntAmps();

    bridgeReverse();
    delay(SETTLE_MS);
    float vMinus = readAmpVolts();
    float iMinus = readShuntAmps();

    if (suspect(vPlus, vMinus)) lastSuspect = true;

    vSum += (vPlus - vMinus);        // offset cancels; signal doubles
    iSum += (iPlus + iMinus) * 0.5;  // shunt read in both polarities, magnitudes averaged

    if (verbose) {
      Serial.print(F("cycle "));
      Serial.print(n);
      Serial.print(F("/"));
      Serial.print(N_STACK);
      Serial.print(F("  V+ "));
      Serial.print(vPlus, 4);
      Serial.print(F("  V- "));
      Serial.print(vMinus, 4);
      Serial.print(F("  I "));
      Serial.print((iPlus + iMinus) * 0.5 * 1000.0, 4);
      Serial.println(F(" mA"));
    }
  }
  bridgePark();

  lastVMN = (vSum / N_STACK) / (2.0 * AMP_GAIN);
  lastI   = iSum / N_STACK;
  return lastVMN / lastI;
}

// --------------------------------------------------------------- commands

void printHelp() {
  Serial.println(F("diy-ert PCB console. Commands:"));
  Serial.println(F("  ?          help"));
  Serial.println(F("  r          raw ADC readout once"));
  Serial.println(F("  p  +  -    bridge: park / hold forward / hold reverse"));
  Serial.println(F("  c N        all muxes to channel N (0-15)"));
  Serial.println(F("  x A B C D  mux channels: C1=A C2=B P1=C P2=D"));
  Serial.println(F("  t          live amp readout, any key stops"));
  Serial.println(F("  m          stacked measurement on current channels"));
  Serial.println(F("  w          Wenner survey, pyGIMLi output"));
}

void cmdRaw() {
  int16_t a0  = ads.readADC_SingleEnded(0);
  int16_t d23 = ads.readADC_Differential_2_3();
  Serial.print(F("A0 "));
  Serial.print(a0);
  Serial.print(F(" ("));
  Serial.print(ads.computeVolts(a0), 4);
  Serial.print(F(" V)   A2-A3 "));
  Serial.print(d23);
  Serial.print(F(" ("));
  Serial.print(ads.computeVolts(d23), 4);
  Serial.println(F(" V)"));
}

void cmdLive() {
  Serial.println(F("Live readout. Any key stops."));
  while (!Serial.available()) {
    bridgeForward();
    delay(SETTLE_MS);
    float vFwd = readAmpVolts();
    float iFwd = readShuntAmps();
    bridgeReverse();
    delay(SETTLE_MS);
    float vRev = readAmpVolts();
    float iRev = readShuntAmps();

    Serial.print(F("FWD "));
    Serial.print(vFwd, 4);
    Serial.print(F(" V   REV "));
    Serial.print(vRev, 4);
    Serial.print(F(" V   diff "));
    Serial.print(vFwd - vRev, 4);
    Serial.print(F(" V   I "));
    Serial.print((iFwd + iRev) * 0.5 * 1000.0, 3);
    Serial.print(F(" mA"));
    if (suspect(vFwd, vRev)) Serial.print(F("   <-- suspect: saturated, floating, or at a rail"));
    Serial.println();
  }
  while (Serial.available()) Serial.read();
  bridgePark();
  Serial.println(F("Stopped, bridge parked."));
}

void cmdMeasure() {
  Serial.print(F("Measuring on C1="));
  Serial.print(curC1);
  Serial.print(F(" C2="));
  Serial.print(curC2);
  Serial.print(F(" P1="));
  Serial.print(curP1);
  Serial.print(F(" P2="));
  Serial.println(curP2);

  float r = measureOhms(true);

  Serial.println(F("---"));
  Serial.print(F("V(P1-P2) = "));
  Serial.print(lastVMN * 1000.0, 3);
  Serial.println(F(" mV"));
  Serial.print(F("I        = "));
  Serial.print(lastI * 1000.0, 4);
  Serial.println(F(" mA"));
  Serial.print(F("R        = "));
  Serial.print(r, 2);
  Serial.println(F(" ohm"));
  if (lastSuspect) {
    Serial.println(F("WARNING: at least one half-cycle looked saturated or at a rail. Do not trust R."));
  }
}

void cmdSurvey() {
  // pyGIMLi unified data format; the serial stream is the data file.
  Serial.println(N_ELECTRODES);
  Serial.println(F("# x z"));
  for (int i = 0; i < N_ELECTRODES; i++) {
    Serial.print(i * ELECTRODE_SPACING, 2);
    Serial.println(F(" 0.0"));
  }

  int total = 0;
  for (int n = 1; 3 * n < N_ELECTRODES; n++) total += N_ELECTRODES - 3 * n;
  Serial.println(total);
  Serial.println(F("# a b m n rhoa"));

  for (int n = 1; 3 * n < N_ELECTRODES; n++) {
    for (int i = 0; i + 3 * n < N_ELECTRODES; i++) {
      int c1 = i, p1 = i + n, p2 = i + 2 * n, c2 = i + 3 * n;
      setMuxes(CH[c1], CH[c2], CH[p1], CH[p2]);
      delay(10);
      float r = measureOhms(false);
      float rhoa = 2.0 * PI * (n * ELECTRODE_SPACING) * r;
      Serial.print(c1 + 1);
      Serial.print(F(" "));
      Serial.print(c2 + 1);
      Serial.print(F(" "));
      Serial.print(p1 + 1);
      Serial.print(F(" "));
      Serial.print(p2 + 1);
      Serial.print(F(" "));
      Serial.println(rhoa, 3);
    }
  }
  Serial.println(F("0"));
}

// ------------------------------------------------------------------- main

void setup() {
  pinMode(PIN_SRCLK, OUTPUT);
  pinMode(PIN_RCLK, OUTPUT);
  pinMode(PIN_SER, OUTPUT);
  digitalWrite(PIN_SRCLK, LOW);
  digitalWrite(PIN_RCLK, LOW);
  bridgePark();                 // latch a known, parked state immediately
  setMuxes(curC1, curC2, curP1, curP2);

  Serial.begin(9600);
  Serial.setTimeout(2000);
  if (!ads.begin(0x48)) {
    Serial.println(F("ADS1115 not found at 0x48. Run sketch 00, check the socket."));
    while (true) {}
  }
  ads.setGain(GAIN_TWOTHIRDS);  // 0-5 V amp output needs the +/-6.144 V range
  printHelp();
}

void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case '?': printHelp(); break;
    case 'r': cmdRaw(); break;
    case 'p': bridgePark();    Serial.println(F("PARKED.")); break;
    case '+': bridgeForward(); Serial.println(F("FORWARD, holding DC.")); break;
    case '-': bridgeReverse(); Serial.println(F("REVERSE, holding DC.")); break;
    case 'c': {
      int n = Serial.parseInt();
      setMuxes(n, n, n, n);
      Serial.print(F("All muxes on channel "));
      Serial.println(n);
      break;
    }
    case 'x': {
      int a = Serial.parseInt();
      int b = Serial.parseInt();
      int cc = Serial.parseInt();
      int d = Serial.parseInt();
      setMuxes(a, b, cc, d);
      Serial.print(F("C1="));  Serial.print(a);
      Serial.print(F(" C2=")); Serial.print(b);
      Serial.print(F(" P1=")); Serial.print(cc);
      Serial.print(F(" P2=")); Serial.println(d);
      break;
    }
    case 't': cmdLive(); break;
    case 'm': cmdMeasure(); break;
    case 'w': cmdSurvey(); break;
    default: break;  // ignore line endings and unknown characters
  }
}
