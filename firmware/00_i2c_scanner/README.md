# 00: I2C scanner

**Verifies:** the ADS1115 is alive and answering at address 0x48. Run
this before anything else. It needs only the Arduino, the ADS1115
breakout, and two I2C wires.

**Expected output:** `Device found at 0x48`, repeated every five seconds.

**Failure signature:** nothing found. Check, in this order:

1. Power to the breakout (VDD and GND with a multimeter, at the chip
   side of the header, not the wire side)
2. SDA/SCL wiring. Mega: SDA = pin 20, SCL = pin 21
3. The solder joints on the breakout headers

Press-fitted, unsoldered header pins pass visual inspection and fail
electrically. That exact fault produced several days of intermittent I2C
on this build ([docs/failure-log.md](../../docs/failure-log.md)).
