// 02 — DC hold
// Verifies: the L298N H-bridge switches, using a multimeter as the
// reference instrument.
//
// Why this sketch exists: a multimeter cannot track a 150 ms square wave.
// It averages toward zero and reports "no current" while the bridge is
// driving perfectly well — a false reading that burned real debugging
// time on this build (see docs/failure-log.md). Park the bridge in DC
// and the multimeter becomes trustworthy again.
//
// Serial commands: '+' forward, '-' reverse, 'p' park (both inputs LOW).
// Hold a polarity and trace the voltage along the injection path —
// supply, bridge outputs, mux input, electrode — until it disappears.

const int BRIDGE_IN1 = 6;  // L298N IN1
const int BRIDGE_IN2 = 7;  // L298N IN2

void setup() {
  pinMode(BRIDGE_IN1, OUTPUT);
  pinMode(BRIDGE_IN2, OUTPUT);
  // Park on boot: both inputs LOW, no drive.
  digitalWrite(BRIDGE_IN1, LOW);
  digitalWrite(BRIDGE_IN2, LOW);

  Serial.begin(9600);
  while (!Serial) {}
  Serial.println("DC hold. Commands: '+' forward, '-' reverse, 'p' park.");
  Serial.println("Bridge is PARKED.");
}

void loop() {
  if (!Serial.available()) return;

  switch (Serial.read()) {
    case '+':
      digitalWrite(BRIDGE_IN1, HIGH);
      digitalWrite(BRIDGE_IN2, LOW);
      Serial.println("FORWARD. Holding DC — safe to measure.");
      break;
    case '-':
      digitalWrite(BRIDGE_IN1, LOW);
      digitalWrite(BRIDGE_IN2, HIGH);
      Serial.println("REVERSE. Holding DC — safe to measure.");
      break;
    case 'p':
      digitalWrite(BRIDGE_IN1, LOW);
      digitalWrite(BRIDGE_IN2, LOW);
      Serial.println("PARKED.");
      break;
    default:
      break;
  }
}
