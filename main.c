#include <atmel_start.h>
#include <util/delay.h>

// #include "oled.c"
// #include "font.c"
// 
i2c_error_t I2C_0_do_transfer(uint8_t *data, uint8_t size);

int main(void) {
  atmel_start_init();

  PA6_set_dir(PORT_DIR_OUT);

  _delay_ms(250);
  I2C_0_do_transfer("\x00" // commands
                    "\x20" /* set memory mode = */ "\x02" /*page*/
                    "\xA1\xCB" // flipping
                    "\xDB" /* set brightness = */  "\x40"
                    "\x81\x00" // contrast
                    "\x21\x00\x7F" // column range
                    "\x22\x00\x07" // page range
                    "\x8D\x14" // charge pump
                    "\xAF", // display on
                    18);

  while(1) {
    //_delay_ms(10);
    I2C_0_do_transfer("\x00" // commands
                      "\x00" // column lower nibble
                      "\x10" // column upper nibble
                      "\xB1", // page
                      4);

    I2C_0_do_transfer("\x40" // data
                      "\xFF\x00\xFF", // 3 columns of pixels
                      4);

    //_delay_ms(10);
    I2C_0_do_transfer("\x00" // commands
                      "\x00" // column lower nibble
                      "\x10" // column upper nibble
                      "\xB1", // page
                      4);

    I2C_0_do_transfer("\x40" // data
                      "\x00\xFF\x00", // 3 columns of pixels
                      4);
  }
}



/* int myfoododii() { */

/*   oled_init(); */
/*   //oled_clear(0); */

/*   PA6_set_pull_mode(PORT_PULL_OFF); */
/*   PA6_set_inverted(false); */
/*   PA6_set_isc(PORT_ISC_INTDISABLE_gc); */
/*   PA6_set_dir(PORT_DIR_OUT); */

/*   for(int i = 0 ; i < 100 ; i++) { */
/*     PA6_set_level(1); _delay_ms(1); */
/*     PA6_set_level(0); _delay_ms(1); */
/*   } */
  
/*   uint8_t counter = 0; */
/*   while(1) { */
/*     oled_goto(12, 4); */
/*     twi_start(); { */
/*       twi_addressWrite(address); */
/*       twi_write(0x40); */
      
/*       twi_write(0x00); */
/*       twi_write(0x00); */
/*       twi_write(0x00); */
/*       twi_write(0x00); */
/*       twi_write(0x00); */
/*       twi_write(0x00); */
/*       twi_write(0x00); */
/*     } twi_stop(); */
    
/*     //_delay_ms(10); */
    
/*     oled_goto(12, 4); */
/*     twi_start(); { */
/*       twi_addressWrite(address); */
/*       twi_write(0x40); */
      
/*       twi_write(0x00); */
/*       twi_write(0xFF); */
/*       twi_write(counter); */
/*       twi_write(counter); */
/*       twi_write(0xFF); */
/*       twi_write(0x00); */
/*     } twi_stop(); */
/*     counter++; */

/*     //_delay_ms(10); */
/*   } */

/*   /\* while(1) { *\/ */
/*   /\* oled_clear(0);// _delay_ms(200); *\/ */
/*   /\* oled_clear(1);// _delay_ms(200); *\/ */
/*   /\* } *\/ */
/*   /\* oled_clear(0); *\/ */
  
/*   //oled_page(1); */
/*   //printf("hello world 1234"); */
/*   //oled_fill(5, 0xAA); */

/*   ADC_0_enable(); */

/*   while(1) { */
/*     oled_goto(2, 5); */

/*     printf("%8x", RTC.CNT); */
/*     _delay_ms(100); */
/*     continue; */
    
/*     twi_start(); */
/*     twi_addressWrite(address); */
/*     twi_write(0x40); */

/*     for(int i = 0 ; i < 4 ; i++) { */
/*       twi_write(RTC.CNT & 0x80 ? 0xAA : 0); */
/*     } */
/*     twi_write(0); */
    
/*     for(int i = 0 ; i < 100 ; i++) { */
/*       // ADC_0_start_conversion(ADC_MUXPOS_AIN5_gc); */
/*       twi_write(ADC_0_get_conversion(ADC_MUXPOS_AIN5_gc) & 0xFF); */
/*       //twi_write(RTC.CNT); */
/*     } */

/*     //_delay_ms(1000); */
    
/*     twi_stop(); */
/*   } */
/* } */
