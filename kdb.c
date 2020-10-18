#include "SSD1306_minimal.h"
#include <avr/pgmspace.h>
#include <util/delay.h>

#define DEG "\xa7" "C"

SSD1306_Mini oled;

const char hello[] = "Hello World!";


void splash() {
 oled.startScreen();
 oled.clear();

 oled.cursorTo(0, 0);
 oled.printString(hello);
 oled.cursorTo(0, 10);
 oled.printString("ATtiny85!");
}

int main() {
  oled.init(0x78);
  oled.clear();

  splash();
  while(1);
}
