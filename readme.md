
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

> Note that `_delay_us` isn't aware of our new clock frequency and
> will thus not delay long enough.

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
