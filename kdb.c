#include "SSD1306_minimal.h"
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <stdio.h>


#include <avr/sleep.h>
#include <avr/wdt.h>

ISR(WDT_vect) {
}


/***************************************************/
/* set new watchdog timeout prescaler value */
// 6 = 1 sek
// 7 = 2 sek
// 10 = 4 sek
// 11 = 8 sek
void enterSleep(byte prescaler) {
  MCUSR &= ~(1<<WDRF); // Clear the reset flag.

  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles). Page 55.
   */
  WDTCR |= (1<<WDCE) | (1<<WDE);
  WDTCR =
    ((prescaler>>3) & 1)<<WDP3 |
    ((prescaler>>2) & 1)<<WDP2 |
    ((prescaler>>1) & 1)<<WDP1 |
    ((prescaler>>0) & 1)<<WDP0; // page 55
  WDTCR |= _BV(WDIE); // Enable the WD interrupt (note no reset).

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode(); // The program will continue from here after the WDT timeout

  // magic procedure to disable the watchdog timer. see 8.4.2.
  MCUSR = 0x00; // clear all reset flags (I don't think you need this)
  WDTCR |= (1<<WDCE) | (1<<WDE); // turn on "Watchdog Change Enable"
  WDTCR = 0x00; // to allow turning the Watchdog off

  // I couldn't get this to work. My chip reports the BODSE bit is
  // zero, even right after setting it so I think it's we've met the
  // "limitations" described in the manual.
  // disable Brown-out detection during sleep (it consumes a lot of power)
  // page 37: First, both BODS and BODSE must be set to one. Second,
  // within four clock cycles, BODS must be set to one and BODSE must
  // be set to zero
  // MCUCR |= (1<<BODS) | (1<<BODSE);
  // MCUCR |= (1<<BODS);
  // MCUCR &= ~(1<<BODSE);
}

SSD1306_Mini oled = SSD1306_Mini(0x3C);

#define B "\x03"
const char hello[] PROGMEM = "Ingunn er s\xa2t";

int main() {
again:  
  sei();
  oled.init();
  oled.clear();

  oled.startScreen();
  oled.clear();
  oled.sendCommand(0xC0);
  
  oled.cursorTo(0, 0);
  oled.printString_P(hello);
  oled.cursorTo(128-(5*3), 0);
  oled.printString("v1");
  
  // oled.cursorTo(3, 10); for(short i = 0 ; i < 16 ; i++) oled.printChar('\x03');
  // oled.cursorTo(0, 20); for(short i = 0 ; i < 17 ; i++) oled.printChar('\x03');
  unsigned long long seconds = 0;
  while(1) {
    seconds++;
    oled.cursorTo(60, 20); oled.printChar('\x03'); enterSleep(5);
    oled.cursorTo(60, 20); oled.printChar(' ');    enterSleep(5);

    char out[12];
    sprintf(out, "%06ld", seconds);
    oled.cursorTo(80, 30); oled.printString(out);
  }
}
