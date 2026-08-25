// 03: Mux channel scan
// Verifies: address decoding and channel continuity on all four
// CD74HC4067 multiplexers.
//
// The sketch steps every mux through channels 0-15 together, holding each
// channel for two seconds. Continuity-beep from each mux's common pin to
// the selected electrode header with a multimeter.
//
// Failure signature: if every channel reads identically at the ADC,
// measure at the mux common pin directly, NOT through the amplifier; a
// saturated amp makes all channels look the same regardless of what the
// mux is doing.
//
// On this build, channels 0, 7, 11 and 12 turned out dead. They were
// breadboard contact faults, not chip or addressing faults: the dead
// channel numbers share no common address bit, which rules out a stuck
// address line. The survey firmware (sketch 06) routes around them with
// a lookup table.

const int MUX_C1[4] = {22, 24, 26, 28};  // address lines S0..S3
const int MUX_C2[4] = {30, 32, 34, 36};
const int MUX_P1[4] = {38, 40, 42, 44};
const int MUX_P2[4] = {46, 48, 50, 52};

void setMux(const int pins[4], int channel) {
  for (int b = 0; b < 4; b++) {
    digitalWrite(pins[b], (channel >> b) & 1);
  }
}

void setup() {
  for (int b = 0; b < 4; b++) {
    pinMode(MUX_C1[b], OUTPUT);
    pinMode(MUX_C2[b], OUTPUT);
    pinMode(MUX_P1[b], OUTPUT);
    pinMode(MUX_P2[b], OUTPUT);
  }
  Serial.begin(9600);
  while (!Serial) {}
  Serial.println("Mux scan: all four muxes step through channels 0-15.");
  Serial.println("Two seconds per channel. Probe continuity now.");
}

void loop() {
  for (int ch = 0; ch < 16; ch++) {
    setMux(MUX_C1, ch);
    setMux(MUX_C2, ch);
    setMux(MUX_P1, ch);
    setMux(MUX_P2, ch);
    Serial.print("Channel ");
    Serial.print(ch);
    Serial.println(" selected on all muxes (C1/C2/P1/P2).");
    delay(2000);
  }
  Serial.println("--- scan complete, restarting ---");
}
