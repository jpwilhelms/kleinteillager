#include <IRremote.h>

IRrecv irrecv(gpioIrReceive);

decode_results results;

void setupIrReceiver()
{
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
}

void irAdjustTargetSlot() {
  if (irrecv.decode(&results)) {
    if( results.value == 0xA55A3AC5 ) {
      // key "BD"
      targetSlot = 1;
    }
    else if( results.value == 0xA55AA15E ) {
      // key "DVD"
      targetSlot = 2;
    }
    else if( results.value == 0xA55A32CD ) {
      // key "CD"
      targetSlot = 3;
    }
    else if( results.value == 0xA55AE21D ) {
      // key "TUNER"
      targetSlot = 4;
    }
    else if( results.value == 0xA55A7986 ) {
      // key "iPod"
      targetSlot = 5;
    }
    else if( results.value == 0xA55A08F7 ) {
      // key "SAT/CBL"
      targetSlot = 6;
    }

    irrecv.resume(); // Receive the next value
  }
}
