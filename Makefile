
LIBS=SSD1306_minimal.cpp TinyWireM.cpp USI_TWI_Master.cpp

kdb.hex: kdb.elf
	avr-objcopy -O ihex $< $@

kdb.elf: kdb.c Makefile ${LIBS}
	avr-g++ -mmcu=attiny85 -DF_CPU=8000000UL -Os -lprintf_min -I . -g kdb.c -o kdb.elf ${LIBS}

flash: kdb.hex
	avrdude -B 4 -c atmelice_isp -p attiny85 -U flash:w:kdb.hex:i

.PHONY: flash
