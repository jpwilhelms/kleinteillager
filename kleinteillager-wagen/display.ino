#include <ss_oled.h> // version 4.1.3

// #define USE_BACKBUFFER

#ifdef USE_BACKBUFFER
static uint8_t ucBackBuffer[1024];
#else
static uint8_t *ucBackBuffer = NULL;
#endif

#define MY_OLED OLED_128x64
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR -1
#define RESET_PIN -1
#define SCL_PIN 22
#define INVERT 0
#define FLIP180 0
#define SDA_PIN 21
#define USE_HW_I2C 1

SSOLED ssoled;

void initDisplay() {
  if (oledInit(&ssoled, MY_OLED, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L) != OLED_NOT_FOUND) {
    Serial.println( "initialized oled display" );
    oledSetBackBuffer(&ssoled, ucBackBuffer); // @suppress("Invalid arguments")
    oledFill(&ssoled, 0, 1);
    oledWriteString(&ssoled, 0, 0, 0, "press button to start", FONT_SMALL, 0, 1);
  }
  else {
    Serial.println( "error initializing oled display" );
  }
}

void clearDisplay() {
  oledFill(&ssoled, 0, 1);
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
  oledWriteString(&ssoled, 0, x, y, message, FONT_SMALL, 0, 1);
}
