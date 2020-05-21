#include <IRremote.h> // version 2.2.3

IRrecv irrecv( gpioIrReceive );

decode_results results;

void setupIrReceiver() {
  irrecv.enableIRIn(); // Start the receiver
  Serial.println( "Enabled IRin" );
}

void irAdjustTargetSlot() {
  if( irrecv.decode( &results ) ) {
    if( results.value == 0xA55A03FC || results.value == 0xA55A3AC5 ) {
      // key "BD"
      changeTargetSlot( 1 );
    }
    else if( results.value == 0xA55AA15E ) {
      // key "DVD"
      changeTargetSlot( 2 );
    }
    else if( results.value == 0xA55A32CD ) {
      // key "CD"
      changeTargetSlot( 3 );
    }
    else if( results.value == 0xA55AE21D ) {
      // key "TUNER"
      changeTargetSlot( 4 );
    }
    else if( results.value == 0xA55AD32C || results.value == 0xA55A7986 ) {
      // key "iPod"
      changeTargetSlot( 5 );
    }
    else if( results.value == 0xA55A08F7 ) {
      // key "SAT/CBL"
      changeTargetSlot( 6 );
    }
    else {
      Serial.println( results.value, HEX );
    }

    irrecv.resume(); // Receive the next value
  }
}
