  [bloom]:https://bloom.oscillate.io/

# Current measurements

I wanted to learn how to measure current consumption. I got it working
in the end (provided my ammeter is correct).

I used a 100Ω resistor as a "shunt resistor". It gives me good enough
accuracy readings I think: My scope likes ranges in 10s of
millivolts. That yields 100µA which I think makes sense. A larger
resistor would allow higher accuracy, but would also influence the
circuitry under test more.

I set the probe scale on my scope to 0.01X and the units to
`[A]`. This means I can read off the current directly in the display -
very nice feature! I don't know how/if the scope division scale affect
the readings or if it's just for display. My actual scope is set to
1X.

## Changing the core clock frequency

This little snippet lets me see:

- current consumption at 4Mhz
- current consumption at 24Mhz
- how long it takes the device to change the core clock frequency.

```C
  while (1) {
    _delay_ms(1000);

    PA5_set_level(1);    PA5_set_level(0);
    
    ccp_write_io((void*)&(CLKCTRL.OSCHFCTRLA),CLKCTRL_FREQSEL_24M_gc);

    PA5_set_level(1);    PA5_set_level(0);

    _delay_us(100); // OBS! much shorter than 100µs!!
    
    PA5_set_level(1);    PA5_set_level(0);
    ccp_write_io((void*)&(CLKCTRL.OSCHFCTRLA),CLKCTRL_FREQSEL_4M_gc);
  }
```

`ACK` below is `PA5`. Here, we're looking at 3 "`ACKS`":

1. 24Mhz mode initializing
2. 24Mhz mode initialized
3. back to 4Mhz mode 

Note that `_delay_us` isn't aware of our new clock frequency and will
thus not delay long enough.

Also, note that my programmer (Atmel-ICE) produces a lot of noise and
distorts the current measurements. I keep it disconnected during
testing.

VCC is 3V.

![ss.png](ss.png)

These measurements tell us we're consuming `1.1mA` at 4Mhz and `4.2mA`
in 24Mhz. That is nice because they seem to agree with the datasheets
of `1.0mA` in 4Mhz and `4.1mA` in 24Mhz power consumtion under 3V.

They also claim it takes the AVR128DB28 about `23µs` to go from 4Mhz
at 24Mhz.

> The datasheets also recommend connecting a 1µF capacitor across all
> power sources. These were not connected here as they, naturally,
> disrupted the measurements.

## Power consumtion during flash write

This was much harder than I thought. start.atmel.com's
`nvmctrl_basic_example.c` takes you quite far but not quite
there. This code snippet does not take into the flash regions into
account, and you have to read the datasheet thoroughly. Make sure you
set the BOOTSIZE FUSE and put all the `FLASH_0_..` functions in this
section. The default setup will just give you NVM_ERROR as you're
trying to write to the same flash section as the one that's executing.
The datasheets makes this quite clear:

> For security reasons, it is not possible to write to the section of
> Flash the code is currently executing from.

I made my BOOTSIZE half of the flash like this:

    pymcuprog -d avr128db28 write -m fuses -o 0x08 -l 128
    
That sets the BOOTSIZE FUSE to flash page 128, which is at flash byte
address 512*128 (64KiB).

Now, with that in order I managed to write a single byte to flash and
read it back:

```C
// ...
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
  
  while (1) {
    _delay_ms(1000);
    status = 30; // why can't I breakpoint _delay_ms?
    _delay_ms(1000);

    hello(); // pulse (1)
    status = FLASH_0_write_flash_byte(myadr + 0, rambuf, 1);
    hello(); // pulse (2)
    rb     = FLASH_0_read_flash_byte(myadr + 0);
    hello(); // pulse (3)
    
    if (rb != 1) { // show panic
      while(1) { ON(); _delay_ms(250); OFF(); _delay_ms(250); }
    }
  }
}
```

We're sending 3 pulses onto our oscilloscope:

1. start of test (oscilloscope trigger on falling edge)
2. done writing flash
3. done reading flash

These are shown shown in purple (`PA5`) on my oscilloscope:

![duration and peak](total-duration-and-peak_mA.png)

Last 2 pulses are right next to each other: reading the flash is
substantially faster than writing, less than 15µs. Perhaps
unsurprising since it can even be memory-mapped to RAM.

Writing the flash takes about `19ms`. There is a peak power
consumption at `5.3mA` for that lasts `10ms`. It consumes slightly
above-average for the remainder of the time if my ammeter setup is
correct.

Note that this particular snippet pretends to write a single byte to
flash, but it doesn't. It reads the entire flash page into `rambuf`,
changes a single byte, and writes the page back. Pages are 512. So,
for a datalogging application, you probably want to buffer up 512
bytes in RAM if you can afford the risk.

Note that I'm running on the default `4Mhz`, at `3V`.

### Debugging with [bloom] and avr-gdb

I'd never really used gdb before, and I've never really used the
debugging facilities of my Atmel-ICE. I was happily surprised to find
[bloom], a bridge between avr-gdb and the Atmel-ICE
programmer/debugger hardware.

Using [bloom] and `avr-gdb` I was quickly able to find that reading
the flash worked, but writing failed. `gdb` is something I've always
wanted to learn more about.

> I also tried [pyAVRdbg](https://github.com/stemnic/pyAVRdbg) but
> couldn't get that to work.
