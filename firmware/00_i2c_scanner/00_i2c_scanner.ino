// 00 — I2C scanner
// Verifies: the ADS1115 is alive and answering at 0x48.
// Run this FIRST. It needs only the Arduino, the ADS1115 breakout, and
// two I2C wires (Mega: SDA = 20, SCL = 21).
//
// If nothing is found: check power to the breakout, SDA/SCL wiring, and
// the solder joints on the breakout headers. Press-fitted, unsoldered
// header pins pass visual inspection and fail electrically — that exact
// fault cost this project several days (see docs/failure-log.md).

#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  while (!Serial) {}
  Serial.println("I2C scanner. Expect the ADS1115 at 0x48.");
}

void loop() {
  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Device found at 0x");
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (found == 0) {
    Serial.println("No I2C devices found. Check power, SDA/SCL, solder joints.");
  }
  Serial.println();
  delay(5000);
}
