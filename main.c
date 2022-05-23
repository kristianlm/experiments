#include <atmel_start.h>
#include <util/delay.h>

static uint8_t rambuf[PROGMEM_PAGE_SIZE];

// start.atmel.com workaround, should be called busy!!
bool FLASH_0_is_busy() {
  return FLASH_0_is_eeprom_ready();
}

static volatile nvmctrl_status_t status;
static volatile uint8_t          rb; //           ,-- this L is _really_ important
static volatile uint32_t         myadr = 70 * 1024L;

#define ON() PA5_set_level(1);
#define OFF() PA5_set_level(0);

void hello() {
  ON();
  _delay_us(500);
  OFF();
}

int main(void) {

  atmel_start_init();

  PA5_set_dir(PORT_DIR_OUT);
  PA6_set_dir(PORT_DIR_OUT);

  PA5_set_pull_mode(PORT_PULL_OFF);
  PA6_set_pull_mode(PORT_PULL_OFF);
  
  while (1) {
    _delay_ms(1000);
    status = 30; // why can't I breakpoint _delay_ms?
    _delay_ms(1000);

    hello();
    status = FLASH_0_write_flash_byte(myadr + 0, rambuf, 1);
    hello();
    rb     = FLASH_0_read_flash_byte(myadr + 0);
    hello();
    
    if (rb != 1) { // show panic
      while(1) { ON(); _delay_ms(250); OFF(); _delay_ms(250); }
    }
  }
}
