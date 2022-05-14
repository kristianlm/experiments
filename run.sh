#/bin/sh

pymcuprog -d avr128db28 erase &&
    make -C gcc &&
    pymcuprog -d avr128db28 write -f gcc/AtmelStart.hex --verify
