// 05: Single measurement
// Verifies: the whole signal chain, end to end, against a known resistor.
// On the 100 ohm bench target this build reported 98.56 ohm (1.4% error).
//
// The measurement: N_STACK polarity cycles. Each cycle reads the amp
// output and the shunt current in BOTH polarities. Differencing the two
// half-cycles cancels any steady offset (amplifier offset, mid-rail
// error, electrode self-potential) exactly, while the real signal flips
// sign with the current and therefore doubles. Stacking averages random
// noise down by ~sqrt(N): at N = 20, bench readings went from ~0.2 ohm
// of wander to stable in the second decimal place.
//
// Send 'm' on the serial monitor to run one stacked measurement.

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

const int BRIDGE_IN1 = 6;
const int BRIDGE_IN2 = 7;

const int MUX_C1[4] = {22, 24, 26, 28};
const int MUX_C2[4] = {30, 32, 34, 36};
const int MUX_P1[4] = {38, 40, 42, 44};
const int MUX_P2[4] = {46, 48, 50, 52};

// Mux channels the target is wired to. Change to match your wiring;
// avoid channels your sketch-03 scan found dead.
const int C1_CHANNEL = 1;
const int C2_CHANNEL = 2;
const int P1_CHANNEL = 3;
const int P2_CHANNEL = 4;

const int N_STACK = 20;
const int SETTLE_MS = 100;         // half-cycle ~150 ms including the read
const float SHUNT_OHMS = 1000.0;

// Amplifier gain, MEASURED during calibration, not the trimmer's nominal
// setting: (3.5483 - 0.8205) / 0.0358 = 76 on this build. Re-derive yours
// per docs/calibration.md; this number is wrong for any other unit.
const float AMP_GAIN = 76.0;

Adafruit_ADS1115 ads;

void setMux(const int pins[4], int channel) {
  for (int b = 0; b < 4; b++) {
    digitalWrite(pins[b], (channel >> b) & 1);
  }
}

void bridgeForward() {
  digitalWrite(BRIDGE_IN1, HIGH);
  digitalWrite(BRIDGE_IN2, LOW);
}

void bridgeReverse() {
  digitalWrite(BRIDGE_IN1, LOW);
  digitalWrite(BRIDGE_IN2, HIGH);
}

// Both inputs LOW between measurements: no drive, which limits electrode
// polarisation (see docs/field-procedure.md).
void bridgePark() {
  digitalWrite(BRIDGE_IN1, LOW);
  digitalWrite(BRIDGE_IN2, LOW);
}

float readAmpVolts() {
  return ads.computeVolts(ads.readADC_SingleEnded(0));
}

float readShuntAmps() {
  return fabs(ads.computeVolts(ads.readADC_Differential_2_3())) / SHUNT_OHMS;
}

void measure() {
  // Accumulators MUST be initialised to zero. Uninitialised floats in C++
  // contain garbage, and garbage accumulated over 20 cycles looks like a
  // plausible measurement. This bug happened.
  float vSum = 0.0;
  float iSum = 0.0;

  for (int n = 1; n <= N_STACK; n++) {
    bridgeForward();
    delay(SETTLE_MS);
    float vPlus = readAmpVolts();
    float iPlus = readShuntAmps();

    bridgeReverse();
    delay(SETTLE_MS);
    float vMinus = readAmpVolts();
    float iMinus = readShuntAmps();

    vSum += (vPlus - vMinus);        // offset cancels; signal doubles
    iSum += (iPlus + iMinus) * 0.5;  // shunt read in both polarities, magnitudes averaged

    Serial.print("cycle ");
    Serial.print(n);
    Serial.print("/");
    Serial.print(N_STACK);
    Serial.print("  V+ ");
    Serial.print(vPlus, 4);
    Serial.print("  V- ");
    Serial.print(vMinus, 4);
    Serial.print("  I ");
    Serial.print((iPlus + iMinus) * 0.5 * 1000.0, 4);
    Serial.println(" mA");
  }

  bridgePark();

  // (vPlus - vMinus) spans twice the signal amplitude, so divide by
  // 2 * gain to recover the actual differential across P1-P2.
  float vMN  = (vSum / N_STACK) / (2.0 * AMP_GAIN);
  float iAvg = iSum / N_STACK;
  float r    = vMN / iAvg;

  Serial.println("---");
  Serial.print("V(P1-P2) = ");
  Serial.print(vMN * 1000.0, 3);
  Serial.println(" mV");
  Serial.print("I        = ");
  Serial.print(iAvg * 1000.0, 4);
  Serial.println(" mA");
  Serial.print("R        = ");
  Serial.print(r, 2);
  Serial.println(" ohm");
  Serial.println("Send 'm' for another measurement.");
}

void setup() {
  pinMode(BRIDGE_IN1, OUTPUT);
  pinMode(BRIDGE_IN2, OUTPUT);
  bridgePark();
  for (int b = 0; b < 4; b++) {
    pinMode(MUX_C1[b], OUTPUT);
    pinMode(MUX_C2[b], OUTPUT);
    pinMode(MUX_P1[b], OUTPUT);
    pinMode(MUX_P2[b], OUTPUT);
  }
  setMux(MUX_C1, C1_CHANNEL);
  setMux(MUX_C2, C2_CHANNEL);
  setMux(MUX_P1, P1_CHANNEL);
  setMux(MUX_P2, P2_CHANNEL);

  Serial.begin(9600);
  while (!Serial) {}
  if (!ads.begin(0x48)) {
    Serial.println("ADS1115 not found at 0x48. Go back to sketch 00.");
    while (true) {}
  }
  ads.setGain(GAIN_TWOTHIRDS);
  Serial.println("Single measurement. Send 'm' to measure.");
}

void loop() {
  if (Serial.available() && Serial.read() == 'm') {
    measure();
  }
}
