#include <avr/pgmspace.h>
#include "font.h"

#define ON()  (PA6_set_level(1))
#define OFF() (PA6_set_level(0))

// Constants
static int const address = 60;
static int const commands = 0x00;
static int const onecommand = 0x80;
static int const data = 0x40;
static int const onedata = 0xC0;

// OLED display **********************************************

uint8_t i2c_err() {
  if (TWI0.MSTATUS & TWI_RXACK_bm) return 1;
  if (TWI0.MSTATUS & TWI_ARBLOST_bm) return 2;
  if (TWI0.MSTATUS & TWI_BUSERR_bm) return 3;
  return 0;
}


#define TWI_IS_CLOCKHELD() (TWI0.MSTATUS & TWI_CLKHOLD_bm)
#define TWI_IS_BUSERR()    (TWI0.MSTATUS & TWI_BUSERR_bm)
#define TWI_IS_ARBLOST()   (TWI0.MSTATUS & TWI_ARBLOST_bm)
#define CLIENT_NACK()      (TWI0.MSTATUS & TWI_RXACK_bm)
#define CLIENT_ACK()     (!(TWI0.MSTATUS & TWI_RXACK_bm))

#define TWI_IS_BUSBUSY() ((TWI0.MSTATUS & TWI_BUSSTATE_BUSY_gc) == TWI_BUSSTATE_BUSY_gc)
//#define TWI_IS_BAD() ((TWI_IS_BUSERR()) | (TWI_IS_ARBLOST()) | (CLIENT_NACK()) | (TWI_IS_BUSBUSY()))

#define TWI_WAIT() while (!((TWI_IS_CLOCKHELD()) || (TWI_IS_BUSERR()) || (TWI_IS_ARBLOST()) || (TWI_IS_BUSBUSY())))

void twi_start() {
  TWI0.MCTRLB |= TWI_FLUSH_bm;
  TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc;
  TWI0.MSTATUS |= (TWI_RIF_bm | TWI_WIF_bm);
}

void twi_addressWrite(uint8_t address) {
  //_delay_ms(10);
  //ON();
  TWI0.MADDR = address << 1; // send adr, clears WIF
  //TWI_WAIT();
  while (!(TWI0.MSTATUS & TWI_WIF_bm)) {}
  //OFF();
  //if(TWI0.MSTATUS & TWI_RXACK_bm) ON();
  //_delay_us(1000);
}
void twi_write(uint8_t datum) {
  //_delay_ms(10);
  //ON();
  TWI0.MDATA = datum; // send data, clears WIF
  //TWI_WAIT();
  // TODO: check CLIENT_NACK()
  while (!(TWI0.MSTATUS & TWI_WIF_bm)) { }
  //OFF();
  //_delay_us(1000);
}

void twi_stop() {
  //_delay_ms(10);
  TWI0.MCTRLB |= TWI_MCMD_STOP_gc;
  // TWI0.MCTRLB = TWI_MCMD_STOP_gc;
  // _delay_ms(4);
  //while (!(TWI0.MSTATUS & TWI_WIF_bm)) {}

  
  /* for(int i = 0 ; i < 8 ; i++) { */
  /*   PA6_set_level(1); _delay_ms(1); */
  /*   PA6_set_level(0); _delay_ms(1); */
  /* } */

}

// ======================================== oled ========================================

void oled_init () {
  _delay_ms(250);
  twi_start();
  twi_addressWrite(address);
  twi_write(commands);

  twi_write(0x20); // set memory mode
  //                       ,-- not supported on my devices 😦 (ignored)
  twi_write(0x02); // 00=horizontal, 01=vertical, 02=page

  twi_write(0xA1); // horizontal flip: 0xA0=off 0xA1=on
  twi_write(0xC8); // vertical flip:   0xC0=off 0xC8=on

  twi_write(0xDB); // set vcom detect
  twi_write(0x40); // brightness

  twi_write(0x81); twi_write(0x0); // set constrast

  twi_write(0x21); twi_write(0); twi_write(127);  // column range
  twi_write(0x22); twi_write(0); twi_write(7);  // page range


  // only charge pump and display on were required for my Bangood
  // purchase (https://banggood.app.link/nO5rGtZfphb)
  twi_write(0x8D); twi_write(0x14); // charge pump
  twi_write(0xAF); // display on
  twi_stop();
}

// send filler number times. set filler to 0 to clear the current
// page, for example.
void oled_fill(uint8_t number, uint8_t filler) {
  twi_start();
  twi_addressWrite(address);
  twi_write(0x40); // data mode (sending screen content)

  for(uint8_t i = 0 ; i < number ; i++) {
    twi_write(filler);
  }
  twi_stop();
}

// change the current page
void oled_page(uint8_t page) {
  twi_start();
  twi_addressWrite(address);
  twi_write(commands);
  twi_write(0xB0 | page);
  twi_stop();
}

// change the current column and page
void oled_goto(uint8_t column, uint8_t page) {
  twi_start();
  twi_addressWrite(address);
  twi_write(commands);
  twi_write(0x00 | (0x0F & (column<<0))); // column lower nibble
  twi_write(0x10 | (0x0F & (column>>4))); // column upper nibble
  twi_write(0xB0 | page);
  twi_stop();
}

// turn all pixels of the screen on or off
void oled_clear(uint8_t white) {
  for(uint8_t page = 0 ; page < 8 ; page++) {
    oled_goto(0, page);
    oled_fill(128, white ? 0xFF : 0x00);
  }
}

// ======================================== stdio ========================================
#include <stdio.h>

// using send font57 pixels for character ch
void oled_print(const char ch) {
  twi_start();
  twi_addressWrite(address);
  twi_write(0x40); // data mode (sending screen content)

  twi_write( 0x00 );
  twi_write(pgm_read_byte(&(font57[((unsigned char)ch*5 )])));
  twi_write(pgm_read_byte(&(font57[((unsigned char)ch*5 + 1)])));
  twi_write(pgm_read_byte(&(font57[((unsigned char)ch*5 + 2)])));
  twi_write(pgm_read_byte(&(font57[((unsigned char)ch*5 + 3)])));
  twi_write(pgm_read_byte(&(font57[((unsigned char)ch*5 + 4)])));
  twi_write( 0x00 );

  twi_stop();
}

// stdout-able output
static uint8_t oled_x = 0;
static uint8_t oled_y = 0;
static int oled_putchar(char c, FILE *stream) {
  if(c == '\n') {
    oled_x = 0;
    oled_goto(0, ++oled_y);
    return 0;
  }
  oled_print(c);
  oled_x++;
  if(oled_x >= 18) {
    oled_x = 0;
    oled_goto(0, ++oled_y);
  }
  return 0;
}

// static FILE stdout_oled = FDEV_SETUP_STREAM(oled_putchar, NULL, _FDEV_SETUP_WRITE);
