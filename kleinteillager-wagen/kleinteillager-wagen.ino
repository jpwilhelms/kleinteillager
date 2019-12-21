#include <ss_oled.h>

// #define USE_BACKBUFFER

#ifdef USE_BACKBUFFER
static uint8_t ucBackBuffer[1024];
#else
static uint8_t *ucBackBuffer = NULL;
#endif

int gpioLimitSwitch = 2;
int gpioObjectDetector = 3;
int gpioPwmForward = 10;
int gpioPwmBackward = 11;
int gpioStartButton = 4;

int currentSpeed = 0;

#define DIRECTION_FORWARD 1
#define DIRECTION_BACKWARD 2
int currentDirection = DIRECTION_FORWARD;

int maxSlot = 6;
int speedForAdjust = 50;
int speedForInit = 70;
int maxSpeed = 255;

int timeForDirectionChangeMs = 500;

volatile int targetSpeed = 0;
volatile int currentSlot = 0;
int targetSlot = 0;

#define STATE_INITIAL 0
#define STATE_DRIVE_TO_OBJECT 1
#define STATE_INTERMEDIATE_OBJECT_DETECTED 2
#define STATE_ADJUSTING_TARGET_POSITION_BACKWARD 3
#define STATE_ADJUSTING_TARGET_POSITION_FORWARD 4
#define STATE_TARGET_POSITION_REACHED 5

volatile int state = STATE_INITIAL;

void log( String text ) {
  Serial.println( text );
}

void toDisplay() {
  int line = 0;
  writeToDisplay(0, line++, "       state: ", state);
  writeToDisplay(0, line++, "       speed: ", currentSpeed);
  writeToDisplay(0, line++, "   direction: ", currentDirection);
  writeToDisplay(0, line++, "current slot: ", currentSlot);
  writeToDisplay(0, line++, " target slot: ", targetSlot);
}

void writeToDisplay( int x, int y, char* text, int value ) {
  char message[80];
  sprintf( message, "%s: %d   ", text, value );
  oledWriteString(0, x, y, message, FONT_SMALL, 0, 1);
}

void changeState( int newState ) {
  log( "state: " + String(newState) );
  state = newState;
}

void changeTargetSpeed( int newSpeed ) {
  if ( newSpeed != targetSpeed ) {
    log( "speed: " + String(newSpeed) );
    targetSpeed = newSpeed;
  }
}

void receiveInterruptLimitReached() {
  log( F("limit reached") );
  changeTargetSpeed( 0 );
  changeState( STATE_TARGET_POSITION_REACHED );
  currentSlot = 0;
}

void receiveInterruptObjectDetection() {
  if ( digitalRead( gpioObjectDetector ) == LOW ) {
    receiveInterruptObjectDetected();
  }
  else {
    receiveInterruptNoObjectDetected();
  }
}
void receiveInterruptObjectDetected() {
  if ( state == STATE_DRIVE_TO_OBJECT ) {
    log( F("object reached") );
    if ( currentDirection == DIRECTION_FORWARD ) {
      currentSlot++;
    }
    else {
      currentSlot--;
    }

    log( "current slot: " + String( currentSlot ) );

    if ( currentSlot == targetSlot ) {
      if ( currentDirection == DIRECTION_FORWARD ) {
        changeTargetSpeed( 50 );
        changeState( STATE_ADJUSTING_TARGET_POSITION_FORWARD );
      }
      else {
        changeTargetSpeed( 0 );
        changeState( STATE_ADJUSTING_TARGET_POSITION_BACKWARD );
      }
    }
    else {
      changeState( STATE_INTERMEDIATE_OBJECT_DETECTED );
    }
  }
}

void receiveInterruptNoObjectDetected() {
  if ( state == STATE_ADJUSTING_TARGET_POSITION_FORWARD ) {
    changeTargetSpeed( 0 );
  }
  else if ( state == STATE_INTERMEDIATE_OBJECT_DETECTED ) {
    log( F("object was left") );
    changeState( STATE_DRIVE_TO_OBJECT );
  }
}

void setup() {
  Serial.begin( 9600 );

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  pinMode( gpioStartButton, INPUT_PULLUP );
  pinMode( gpioLimitSwitch, INPUT_PULLUP );

  pinMode( gpioObjectDetector, INPUT );
  pinMode( gpioPwmForward, OUTPUT );
  pinMode( gpioPwmBackward, OUTPUT );

  if (oledInit(OLED_128x64, 0, 0, -1, -1, 400000L) != OLED_NOT_FOUND) {
    oledSetBackBuffer(ucBackBuffer);
    oledFill(0, 1);
    oledWriteString(0, 0, 0, "press button to start", FONT_SMALL, 0, 1);
  }

  while ( digitalRead( gpioStartButton ) == HIGH ) {
    delay( 10 );
  }

  oledFill(0, 1);
  log( "starting" );

  attachInterrupt( digitalPinToInterrupt( gpioObjectDetector ), receiveInterruptObjectDetection, CHANGE );
  attachInterrupt( digitalPinToInterrupt( gpioLimitSwitch ), receiveInterruptLimitReached, FALLING );
}

void adjustSpeed() {
  currentSpeed = targetSpeed;
  
  if ( currentDirection == DIRECTION_FORWARD ) {
    analogWrite( gpioPwmBackward, 0 );
    analogWrite( gpioPwmForward, currentSpeed );
  }
  else {
    analogWrite( gpioPwmForward, 0 );
    analogWrite( gpioPwmBackward, currentSpeed );
  }
}

void loop() {
  toDisplay();

  if ( currentSpeed != targetSpeed ) {
    adjustSpeed();
  }

  if ( currentSpeed == 0 && targetSpeed == 0 ) {
    if ( state == STATE_ADJUSTING_TARGET_POSITION_FORWARD ) {
      changeState( STATE_TARGET_POSITION_REACHED );
    }
    else if ( state == STATE_ADJUSTING_TARGET_POSITION_BACKWARD ) {
      delay( timeForDirectionChangeMs );
      currentDirection = DIRECTION_FORWARD;
      changeTargetSpeed( speedForAdjust );
      changeState( STATE_ADJUSTING_TARGET_POSITION_FORWARD );
    }
    else if ( state == STATE_INITIAL ) {
      currentDirection = DIRECTION_BACKWARD;
      changeTargetSpeed( speedForInit );
    }
    else if ( state == STATE_TARGET_POSITION_REACHED ) {
      while ( targetSlot == currentSlot ) {
        targetSlot = random( 1, maxSlot + 1 );
      }

      log( "new target: " + String( targetSlot ) );
      delay( 3000 );
      changeState( STATE_DRIVE_TO_OBJECT );
      changeTargetSpeed( maxSpeed );

      if ( currentSlot < targetSlot ) {
        currentDirection = DIRECTION_FORWARD;
      }
      else {
        currentSlot++; // we are just after the current slot, moving backwards will immediately decrease it.
        currentDirection = DIRECTION_BACKWARD;
      }
    }
  }
}
