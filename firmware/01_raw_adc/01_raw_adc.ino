// 01: Raw ADC readout
// Verifies: the ADS1115 converts a known voltage correctly on the inputs
// this instrument actually uses: A0 single-ended (amplifier output) and
// A2/A3 differential (current shunt).
//
// Feed A0 a known voltage (the 2.5 V mid-rail is convenient) and confirm
// the printed value against a multimeter on the same node before trusting
// anything downstream.
//
// Failure signature: raw counts near 0 on A2-A3 while current flows means
// the shunt sense wires are disconnected.

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  if (!ads.begin(0x48)) {
    Serial.println("ADS1115 not found at 0x48. Go back to sketch 00.");
    while (true) {}
  }
  // GAIN_TWOTHIRDS: +/-6.144 V full scale, 0.1875 mV/bit. The amp output
  // spans 0-5 V, so this is the only range that covers it without
  // clipping the reading. (The pin itself must still never exceed
  // VDD + 0.3 V; the gain setting changes the readable range, not the
  // absolute maximum. See docs/hardware.md, constraint 1.)
  ads.setGain(GAIN_TWOTHIRDS);
  Serial.println("A0_raw  A0_V     A1_raw  A1_V     A2-A3_raw  A2-A3_V");
}

void loop() {
  int16_t a0  = ads.readADC_SingleEnded(0);
  int16_t a1  = ads.readADC_SingleEnded(1);
  int16_t d23 = ads.readADC_Differential_2_3();

  Serial.print(a0);
  Serial.print("  ");
  Serial.print(ads.computeVolts(a0), 4);
  Serial.print(" V   ");
  Serial.print(a1);
  Serial.print("  ");
  Serial.print(ads.computeVolts(a1), 4);
  Serial.print(" V   ");
  Serial.print(d23);
  Serial.print("  ");
  Serial.print(ads.computeVolts(d23), 4);
  Serial.println(" V");

  delay(500);
}
