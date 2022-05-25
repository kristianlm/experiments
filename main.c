#include <atmel_start.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define ON() PA5_set_level(1);
#define OFF() PA5_set_level(0);

volatile uint8_t mark = 0;
volatile uint8_t dummy = 0;

ISR(RTC_PIT_vect) {
  mark = 1;
  RTC.PITINTFLAGS = RTC_PI_bm;
}

void RTC_0_init() {

  while (RTC.STATUS > 0) { }
  RTC.CLKSEL = RTC_CLKSEL_XOSC32K_gc;
  RTC.PER = 0x0100;
  RTC.CALIB = 0xff;

  while (RTC.PITSTATUS > 0) { }

  RTC.PITCTRLA = RTC_PERIOD_CYC64_gc
    | 1 << RTC_PITEN_bp;

  RTC.PITINTCTRL = 1 << RTC_PI_bp;
}

int main(void) {
  atmel_start_init();
  RTC_0_init();

  PA5_set_dir(PORT_DIR_OUT);
  PA5_set_pull_mode(PORT_PULL_OFF);

  SLPCTRL_set_sleep_mode(SLPCTRL_SMODE_PDOWN_gc);

  while (1) {
    if(mark == 0) {
      ON(); _delay_us(100); OFF(); _delay_us(100);
      ON(); _delay_us(100); OFF(); _delay_us(400);
    }
    else {
      ON(); _delay_us(100); OFF(); _delay_us(400);
    }
    sleep_cpu();

    dummy = 1;
  }
}
