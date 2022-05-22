#include <atmel_start.h>
#include <util/delay.h>

int main(void) {

  atmel_start_init();

  PA5_set_dir(PORT_DIR_OUT);
  PA6_set_dir(PORT_DIR_OUT);

  PA5_set_pull_mode(PORT_PULL_OFF);
  PA6_set_pull_mode(PORT_PULL_OFF);

  while (1) {
    _delay_ms(1000);

    PA5_set_level(1);    PA5_set_level(0);
    
    ccp_write_io((void*)&(CLKCTRL.OSCHFCTRLA),CLKCTRL_FREQSEL_24M_gc);

    PA5_set_level(1);    PA5_set_level(0);

    _delay_us(100);
    
    PA5_set_level(1);    PA5_set_level(0);
    ccp_write_io((void*)&(CLKCTRL.OSCHFCTRLA),CLKCTRL_FREQSEL_4M_gc);
  }
}
