#include <ss_oled.h>

// #define USE_BACKBUFFER

#ifdef USE_BACKBUFFER
static uint8_t ucBackBuffer[1024];
#else
static uint8_t *ucBackBuffer = NULL;
#endif

void initDisplay() {
  if (oledInit(OLED_128x64, 0, 0, -1, -1, 400000L) != OLED_NOT_FOUND) {
    oledSetBackBuffer(ucBackBuffer);
    oledFill(0, 1);
    oledWriteString(0, 0, 0, "press button to start", FONT_SMALL, 0, 1);
  }
}

void clearDisplay() {
  oledFill(0, 1);
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
