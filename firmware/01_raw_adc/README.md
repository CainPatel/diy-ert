# 01: Raw ADC readout

**Verifies:** the ADS1115 converts a known voltage correctly, on the
inputs this instrument actually uses: A0 single-ended (amplifier output)
and A2/A3 differential (current shunt).

**How to use:** feed A0 a known voltage (the 2.5 V mid-rail node is
convenient) and confirm the printed value against a multimeter on the
same node. Then put a small DC current through the shunt (sketch 02 can
hold the bridge in DC) and check the A2-A3 differential the same way.
Requires the Adafruit ADS1X15 library.

**Failure signature:** raw counts near 0 on A2/A3 while current is
flowing means the shunt sense wires are disconnected.

Do not move on to the amplifier sketches until the ADC agrees with the
multimeter here. Every later sketch reads the world through this chip.
