int gpioLimitSwitch = 2;
int gpioObjectDetector = 3;
int gpioPwmForward = 10;
int gpioPwmBackward = 5; // pin 11 is used for ir receive!
int gpioStartButton = 4;

int gpioIrReceive = 8;

#define DIRECTION_FORWARD 1
#define DIRECTION_BACKWARD 2

int currentDirection = 0;
int targetDirection = 0;

int maxSlot = 6;
int speedForAdjust = 100;
int speedForInit = 100;
int speedForBreak = 200;
int durationForBreakMs = 5;
int maxSpeed = 140;

int timeForDirectionChangeMs = 500;

volatile int currentSpeed = 0;
volatile int targetSpeed = 0;

volatile int currentSlot = 0;
volatile int targetSlot = 0;

#define STATE_INITIAL 0
#define STATE_DRIVE_TO_SLOT 2
#define STATE_OBJECT_DETECTED 3
#define STATE_TARGET_POSITION_REACHED 5

volatile int state = STATE_INITIAL;

void log(String text) {
  Serial.println(text);
}

void changeState(int newState) {
  if (state != newState) {
    log("state: " + String(newState));
    state = newState;
  }
}

void changeTargetSpeed(int newSpeed) {
  if (newSpeed != targetSpeed) {
    log("speed: " + String(newSpeed));
    targetSpeed = newSpeed;
  }
}

void setup() {
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  pinMode(gpioStartButton, INPUT_PULLUP);
  pinMode(gpioLimitSwitch, INPUT_PULLUP);

  pinMode(gpioObjectDetector, INPUT);
  pinMode(gpioPwmForward, OUTPUT);
  pinMode(gpioPwmBackward, OUTPUT);

  initDisplay();
  setupIrReceiver();

  while (digitalRead(gpioStartButton) == HIGH) {
    delay(10);
  }

  state = STATE_INITIAL;
  clearDisplay();
  log("starting");
}

void setMotorDirectionAndSpeed() {
  if ((currentDirection != targetDirection)
      || ((currentSpeed != 0) && (targetSpeed == 0))) {
    if (currentDirection == DIRECTION_FORWARD) {
      analogWrite(gpioPwmForward, 0);
      analogWrite(gpioPwmBackward, speedForBreak);
    }
    else {
      analogWrite(gpioPwmBackward, 0);
      analogWrite(gpioPwmForward, speedForBreak);
    }

    delay(durationForBreakMs);
    analogWrite(gpioPwmForward, 0);
    analogWrite(gpioPwmBackward, 0);
    currentSpeed = 0;
    currentDirection = targetDirection;
  }

  currentSpeed = targetSpeed;

  if (currentDirection == DIRECTION_FORWARD) {
    analogWrite(gpioPwmBackward, 0);
    analogWrite(gpioPwmForward, currentSpeed);
  }
  else if (currentDirection == DIRECTION_BACKWARD) {
    analogWrite(gpioPwmForward, 0);
    analogWrite(gpioPwmBackward, currentSpeed);
  }
}

void changeTargetDirection(int newDirection) {
  if (newDirection != targetDirection) {
    log("new direction: " + String(newDirection));
    targetDirection = newDirection;
  }
}

void setDirectionToTargetSlot() {
  if (currentSlot < targetSlot) {
    changeTargetDirection(DIRECTION_FORWARD);
  }
  else if (currentSlot > targetSlot) {
    changeTargetDirection(DIRECTION_BACKWARD);
  }
}

void changeCurrentSlot(int newCurrentSlot) {
  if (newCurrentSlot != currentSlot) {
    log("current slot: " + String(newCurrentSlot));
    currentSlot = newCurrentSlot;
  }
}

void changeTargetSlot(int newTargetSlot) {
  if (newTargetSlot != targetSlot) {
    log("target slot: " + String(newTargetSlot));
    targetSlot = newTargetSlot;
  }
}

bool objectDetected() {
  return (digitalRead(gpioObjectDetector) == LOW);
}

bool noObjectDetected() {
  return (digitalRead(gpioObjectDetector) == HIGH);
}

void loop() {
  if (digitalRead(gpioLimitSwitch) == LOW) {
    changeCurrentSlot(0);
    changeTargetSlot(1);
    changeState( STATE_DRIVE_TO_SLOT);
    changeTargetSpeed(maxSpeed);
    changeTargetDirection(DIRECTION_FORWARD);
    log("position reset");
  }
  else {
    if (state == STATE_INITIAL) {
      changeTargetSpeed(speedForInit);
      changeTargetDirection(DIRECTION_BACKWARD);
      log("driving to first slot");
    }
    else if ((state == STATE_DRIVE_TO_SLOT) && objectDetected()
        && (currentDirection == DIRECTION_FORWARD)) {
      log("object detected forward");
      changeState(STATE_OBJECT_DETECTED);

      if (currentSlot == targetSlot - 1) {
        changeTargetSpeed(speedForAdjust);
      }
    }
    else if ((state == STATE_OBJECT_DETECTED) && noObjectDetected()
        && (currentDirection == DIRECTION_FORWARD)) {
      log("object left forward");
      changeCurrentSlot(currentSlot + 1);

      if (currentSlot != targetSlot) {
        changeState( STATE_DRIVE_TO_SLOT);
      }
      else {
        changeState( STATE_TARGET_POSITION_REACHED);
        changeTargetSpeed(0);
      }
    }
    else if ((state == STATE_DRIVE_TO_SLOT) && objectDetected()
        && (currentDirection == DIRECTION_BACKWARD)) {
      log("object detected backward");
      changeState(STATE_OBJECT_DETECTED);
      changeCurrentSlot(currentSlot - 1);

      if (currentSlot + 1 == targetSlot) {
        changeTargetSpeed(speedForAdjust);
        changeTargetDirection(DIRECTION_FORWARD);
      }
    }
    else if ((state == STATE_OBJECT_DETECTED) && noObjectDetected()
        && (currentDirection == DIRECTION_BACKWARD)) {
      log("object left backward");
      changeState( STATE_DRIVE_TO_SLOT);
    }
    else if (state == STATE_DRIVE_TO_SLOT) {
      changeTargetSpeed(maxSpeed);
    }
    else if (state == STATE_TARGET_POSITION_REACHED) {
      if (currentSlot == targetSlot) {
        changeTargetSpeed(0);

        // if breaking pushed the wagon back to the object, try again to reach a good position
        if (objectDetected()) {
          changeCurrentSlot(currentSlot - 1);
          changeTargetSpeed(speedForAdjust);
          changeState(STATE_DRIVE_TO_SLOT);
        }
      }
      else {
        changeState(STATE_DRIVE_TO_SLOT);
        changeTargetSpeed(maxSpeed);
      }
    }

    if (state == STATE_DRIVE_TO_SLOT || state == STATE_TARGET_POSITION_REACHED) {
      irAdjustTargetSlot();
      setDirectionToTargetSlot();
    }

  }

  setMotorDirectionAndSpeed();
  toDisplay();
}
