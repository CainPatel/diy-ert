// 06: Wenner survey
// Runs the full measurement sequence over the working electrode channels
// and prints an inversion-ready dataset in pyGIMLi's unified data format.
// Capture the serial output verbatim into survey.dat and feed it to
// analysis/invert_pygimli.py.
//
// The survey runs from loop() behind a done-flag, not from setup(), so
// output is not lost if the serial monitor attaches late: it waits for
// 's' before starting. The serial output IS the data file; this sketch
// prints nothing else.

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

const int BRIDGE_IN1 = 6;
const int BRIDGE_IN2 = 7;

const int MUX_C1[4] = {22, 24, 26, 28};
const int MUX_C2[4] = {30, 32, 34, 36};
const int MUX_P1[4] = {38, 40, 42, 44};
const int MUX_P2[4] = {46, 48, 50, 52};

const int N_STACK = 20;
const int SETTLE_MS = 100;
const float SHUNT_OHMS = 1000.0;
const float ELECTRODE_SPACING = 0.5;  // metres between adjacent electrodes

// Amplifier gain, MEASURED during calibration (docs/calibration.md).
const float AMP_GAIN = 76.0;

// Working channel map from the physical build.
// Dead: 0, 7, 11, 12 (breadboard contact faults; the dead numbers share
// no common address bit, which is what ruled out chip and addressing
// faults). Logical electrode position i along the line is wired to mux
// channel CH[i], so dead channels are invisible to the survey logic and
// the field geometry stays uniform.
const int CH[12] = {1, 2, 3, 4, 5, 6, 8, 9, 10, 13, 14, 15};
const int N_ELECTRODES = 12;

Adafruit_ADS1115 ads;

bool surveyDone = false;

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

// Same stacked measurement as sketch 05, silent (the serial stream is the
// data file). See 05_single_measurement for the annotated version.
float measureOhms() {
  float vSum = 0.0;  // accumulators initialised; uninitialised floats are garbage
  float iSum = 0.0;

  for (int n = 0; n < N_STACK; n++) {
    bridgeForward();
    delay(SETTLE_MS);
    float vPlus = readAmpVolts();
    float iPlus = readShuntAmps();

    bridgeReverse();
    delay(SETTLE_MS);
    float vMinus = readAmpVolts();
    float iMinus = readShuntAmps();

    vSum += (vPlus - vMinus);
    iSum += (iPlus + iMinus) * 0.5;
  }
  bridgePark();

  float vMN  = (vSum / N_STACK) / (2.0 * AMP_GAIN);
  float iAvg = iSum / N_STACK;
  return vMN / iAvg;
}

void runSurvey() {
  // pyGIMLi unified data format: electrode positions, then data rows of
  // a b m n rhoa with 1-indexed electrode numbers. a/b are the current
  // electrodes, m/n the potential pair. See analysis/README.md.
  Serial.println(N_ELECTRODES);
  Serial.println("# x z");
  for (int i = 0; i < N_ELECTRODES; i++) {
    Serial.print(i * ELECTRODE_SPACING, 2);
    Serial.println(" 0.0");
  }

  // Wenner spacings n = 1, 2, 3 over 12 electrodes: 9 + 6 + 3 = 18 rows.
  int total = 0;
  for (int n = 1; 3 * n < N_ELECTRODES; n++) {
    total += N_ELECTRODES - 3 * n;
  }
  Serial.println(total);
  Serial.println("# a b m n rhoa");

  for (int n = 1; 3 * n < N_ELECTRODES; n++) {
    for (int i = 0; i + 3 * n < N_ELECTRODES; i++) {
      // Wenner geometry along the line: C1  P1  P2  C2, equally spaced.
      int c1 = i;
      int p1 = i + n;
      int p2 = i + 2 * n;
      int c2 = i + 3 * n;

      setMux(MUX_C1, CH[c1]);
      setMux(MUX_P1, CH[p1]);
      setMux(MUX_P2, CH[p2]);
      setMux(MUX_C2, CH[c2]);
      delay(10);  // mux settling

      float r = measureOhms();
      float a = n * ELECTRODE_SPACING;
      float rhoa = 2.0 * PI * a * r;  // apparent resistivity, ohm-metres

      Serial.print(c1 + 1);
      Serial.print(" ");
      Serial.print(c2 + 1);
      Serial.print(" ");
      Serial.print(p1 + 1);
      Serial.print(" ");
      Serial.print(p2 + 1);
      Serial.print(" ");
      Serial.println(rhoa, 3);
    }
  }
  Serial.println("0");  // empty topography block terminates the file
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
  Serial.begin(9600);
  if (!ads.begin(0x48)) {
    while (true) {}  // ADC missing: go back to sketch 00
  }
  ads.setGain(GAIN_TWOTHIRDS);
}

void loop() {
  if (surveyDone) return;
  if (!(Serial.available() && Serial.read() == 's')) return;

  runSurvey();
  surveyDone = true;  // bridge is parked; reset the board to run again
}
