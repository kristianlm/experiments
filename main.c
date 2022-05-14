#include <atmel_start.h>
#include <util/delay.h>

#include "oled.c"
#include "font.c"

/* i2c_error_t I2C_0_do_transfer(uint8_t *data, uint8_t size); */

/* int main2(void) { */
/*   atmel_start_init(); */

/*   PA6_set_dir(PORT_DIR_OUT); */

/*   _delay_ms(250); */
/*   I2C_0_do_transfer("\x00" // commands */
/*                     "\x20" /\* set memory mode = *\/ "\x02" /\*page*\/ */
/*                     "\xA1\xCB" // flipping */
/*                     "\xDB" /\* set brightness = *\/  "\x40" */
/*                     "\x81\x00" // contrast */
/*                     "\x21\x00\x7F" // column range */
/*                     "\x22\x00\x07" // page range */
/*                     "\x8D\x14" // charge pump */
/*                     "\xAF", // display on */
/*                     18); */

/*   while(1) { */
/*     //_delay_ms(10); */
/*     I2C_0_do_transfer("\x00" // commands */
/*                       "\x00" // column lower nibble */
/*                       "\x10" // column upper nibble */
/*                       "\xB1", // page */
/*                       4); */

/*     I2C_0_do_transfer("\x40" // data */
/*                       "\xFF\x00\xFF", // 3 columns of pixels */
/*                       4); */

/*     //_delay_ms(10); */
/*     I2C_0_do_transfer("\x00" // commands */
/*                       "\x00" // column lower nibble */
/*                       "\x10" // column upper nibble */
/*                       "\xB1", // page */
/*                       4); */

/*     I2C_0_do_transfer("\x40" // data */
/*                       "\x00\xFF\x00", // 3 columns of pixels */
/*                       4); */
/*   } */
/* } */

volatile static uint32_t seconds = 0;

ISR(RTC_CNT_vect) {
  RTC.INTFLAGS = RTC_CMP_bm;
  cli(); seconds++; sei();
}

int main() {
  atmel_start_init();
  oled_init();

  PA6_set_dir(PORT_DIR_OUT);
  
  oled_clear(0);
  
  ADC_0_enable();

  const char alph[] = "0123456789ABCDEF";
  int16_t res = 0;
  while(1) {
    
    oled_goto(40, 1);
    oled_print(alph[(seconds>>12) & 0x0F]);
    oled_print(alph[(seconds>> 8) & 0x0F]);
    oled_print(alph[(seconds>> 4) & 0x0F]);
    oled_print(alph[(seconds>> 0) & 0x0F]);
    oled_print(':');
    oled_print(alph[(RTC.CNT>>12) & 0x0F]);
    oled_print(alph[(RTC.CNT>> 8) & 0x0F]);
    oled_print(alph[(RTC.CNT>> 4) & 0x0F]);
    oled_print(alph[(RTC.CNT>> 0) & 0x0F]);

    oled_goto(0, 5);

    twi_start(); {
      twi_addressWrite(address);
      twi_write(0x40);

      twi_write(0x00);
      twi_write(0xFF);
      twi_write(0x00);
    
      for(int i = 0 ; i < 128-6 ; i++) {
        res = (ADC_0_get_conversion(ADC_MUXPOS_AIN5_gc) >> 2) / 4/*SAMPNUM*/; // uint8_t
        res -= 0x80;
        res /= 7;
        res += 4;
        twi_write(1<<res);
      }
    
      twi_write(0x00);
      twi_write(0xFF);
      twi_write(0x00);
    
    } twi_stop();

    if(0) { // print last result for debugging
      oled_goto(54, 7);
      oled_print(alph[(res>>12) & 0x0F]);
      oled_print(alph[(res>> 8) & 0x0F]);
      oled_print(alph[(res>> 4) & 0x0F]);
      oled_print(alph[(res>> 0) & 0x0F]);
    }

  }
}
