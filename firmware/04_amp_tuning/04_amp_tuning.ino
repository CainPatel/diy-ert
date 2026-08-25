// 04 — Live amplifier tuning
// Verifies: the AD620 is operating inside its input range and its output
// responds to both injection polarities.
//
// Wire the symmetric bench chain (docs/hardware.md, constraint 3) and run
// this while adjusting the trimmers. Follow the order in
// docs/calibration.md: offset FIRST, with no current flowing; then gain.
//
// Failure signature: both half-cycles reading ~3.6 V means the amp is
// saturated. That is an offset problem, not a gain problem — the gain
// trimmer changes nothing while the output is pinned.
//
// Healthy output after calibration: the two polarities land around 1.5 V
// and 3.5 V — clearly separated, not clipping either rail.

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

const int BRIDGE_IN1 = 6;
const int BRIDGE_IN2 = 7;

const int MUX_C1[4] = {22, 24, 26, 28};
const int MUX_C2[4] = {30, 32, 34, 36};
const int MUX_P1[4] = {38, 40, 42, 44};
const int MUX_P2[4] = {46, 48, 50, 52};

// Mux channels the bench chain is wired to. Change to match your wiring;
// avoid channels your sketch-03 scan found dead.
const int C1_CHANNEL = 1;
const int C2_CHANNEL = 2;
const int P1_CHANNEL = 3;
const int P2_CHANNEL = 4;

const int SETTLE_MS = 100;         // half-cycle ~150 ms including the read
const float SHUNT_OHMS = 1000.0;

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

void setup() {
  pinMode(BRIDGE_IN1, OUTPUT);
  pinMode(BRIDGE_IN2, OUTPUT);
  digitalWrite(BRIDGE_IN1, LOW);
  digitalWrite(BRIDGE_IN2, LOW);
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
    Serial.println("ADS1115 not found at 0x48 — go back to sketch 00.");
    while (true) {}
  }
  ads.setGain(GAIN_TWOTHIRDS);  // 0-5 V amp output needs the +/-6.144 V range
  Serial.println("Live tuning. Adjust offset first, then gain.");
}

void loop() {
  bridgeForward();
  delay(SETTLE_MS);
  float vFwd = ads.computeVolts(ads.readADC_SingleEnded(0));
  float iFwd = fabs(ads.computeVolts(ads.readADC_Differential_2_3())) / SHUNT_OHMS;

  bridgeReverse();
  delay(SETTLE_MS);
  float vRev = ads.computeVolts(ads.readADC_SingleEnded(0));
  float iRev = fabs(ads.computeVolts(ads.readADC_Differential_2_3())) / SHUNT_OHMS;

  Serial.print("FWD ");
  Serial.print(vFwd, 4);
  Serial.print(" V   REV ");
  Serial.print(vRev, 4);
  Serial.print(" V   diff ");
  Serial.print(vFwd - vRev, 4);
  Serial.print(" V   I ");
  Serial.print((iFwd + iRev) * 0.5 * 1000.0, 3);
  Serial.println(" mA");
}
