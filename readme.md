  [usbmon]:https://www.kernel.org/doc/html/latest/usb/usbmon.html

# PeakTech 6070 benchtop power supply experiments

I just bought this device, and it has a USB interface which can be
used to control the device. I'd much rather have the protocol
documentet instead of some [bundled proprietary
software](https://www.peaktech.de/uk/PeakTech-P-6070-Digital-Laboratory-Power-Supply-with-USB/P-6070#downloads-tab-pane)
for Windows only, so I started digging. I'm on Arch Linux.

I have emailed the manufacturer regarding documentation, but I have
yet not received a reply.

The good news is that device shows up a a USB serial port with drivers
for Linux already in place on my `5.17` Arch Linux kernel. `dmesg` is
telling me this:

    [50000.679913] usb 1-1: new full-speed USB device number 5 using xhci_hcd
    [50000.820694] usb 1-1: New USB device found, idVendor=067b, idProduct=2303, bcdDevice= 3.00
    [50000.820699] usb 1-1: New USB device strings: Mfr=1, Product=2, SerialNumber=0
    [50000.820701] usb 1-1: Product: USB-Serial Controller
    [50000.820703] usb 1-1: Manufacturer: Prolific Technology Inc.
    [50000.821763] pl2303 1-1:1.0: pl2303 converter detected
    [50000.822340] usb 1-1: pl2303 converter now attached to ttyUSB0

    > lsusb -d 067b:2303
    Bus 001 Device 005: ID 067b:2303 Prolific Technology, Inc. PL2303 Serial Port / Mobile Action MA-8910P

After plugging in, `/dev/ttyUSB0` is available for use. Now we just
need to know what goes down that line.

I got the PeakTech software running under Wine, but that took a little
fiddling around at first.

- Wine uses symlinks under `$WINEPREFIX/dosdevices` to map `COM1` to
  `/dev/ttyUSB0`, but these were overridden on each launch! Edit
  `system.reg` instead:
  
```
    [Software\\Wine\\Ports] 1653148604
    "COM1"="/dev/ttyUSB1"
```

- The software has a few quirks. I think you need to set 'Steps' in
  the bottom right to a value.

- If you get an error `Error!! Serial COM1 does not exist`, it's
  possible things are still working.
  
- You need to click `Start` in order to initialize the COM port and
  send and receive data.
  
I used [usbmon] to monitor the serial data. Its output is a bit messy,
but doable. [usbmon] will grab all USB data, but we can filter out our
device with some grepping:

    sudo cat /sys/kernel/debug/usb/usbmon/1u | grep ':003:'

## COM-port initialization

When I click `Start`, [usbmon] output gives me this:

    ffff88b021bcda80 1898892017 C Ci:1:003:0 0 7 = 80250000 000008
    ffff88b021bcda80 1898892090 S Co:1:003:0 s 21 20 0000 0000 0007 7 = 80250000 000005
    ffff88b021bcda80 1898892385 C Ci:1:003:0 0 1 = 01
    ffff88b018447f00 1898893396 C Ci:1:003:0 0 7 = 80250000 000005
    ffff88b018447f00 1898893416 S Co:1:003:0 s 21 20 0000 0000 0007 7 = 80250000 000008
    ffff88b018447f00 1898893566 C Ci:1:003:0 0 1 = 01

I presume somewhere in there is the baud rate, parity and flow control
settings. I don't know where. But I think I can just get the settings
from instead. Check `stty` while running the bundled software, I get
this:

    klm@pisa ~> sudo stty -F /dev/ttyUSB0 -a
    speed 9600 baud; rows 0; columns 0; line = 0;
    intr = ^C; quit = ^\; erase = ^?; kill = ^U; eof = ^D; eol = <undef>; eol2 = <undef>; swtch = <undef>; start = ^Q;
    stop = ^S; susp = ^Z; rprnt = ^R; werase = ^W; lnext = ^V; discard = ^O; min = 1; time = 0;
    -parenb -parodd -cmspar cs8 hupcl -cstopb cread clocal -crtscts
    -ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff -iuclc -ixany -imaxbel -iutf8
    -opost -olcuc -ocrnl onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
    -isig -icanon -iexten -echo echoe echok -echonl -noflsh -xcase -tostop -echoprt echoctl echoke -flusho -extproc
    
## Sniffing the data

I was hoping the protocol would be some dialect of SCPI, but it's
not. It's some form of binary format. The data seems to be split into
what I will call `frames` here, starting with `f7` and ending with
`fd`.

### Turn output on

    f7010a 1e01 00 01 92 37 fd

### Turn output off

    f7010a 1e01 00 00 53 f7 fd

### Setting I

When I enter 3.456A in the "Current" input for `CH1` and click `ISET`,
this frame is sent:

    f7010a 0a01 0d80 5337fd   # 3456mA

To which the devices replies with an identical copy:

    f7010a 0a01 0d80 5337fd

I don't know why it would reply like that, but should present any
problems. Note that `0x0d80` is 3456 so that's pretty straight
forward. 

Setting "Current" to 0.000A and clicking `ISET`, this frame is sent
over the wire:

    f7010a 0a01 0000 5607fd

This means the three last bytes are probably checksum and EOF.

### Setting V

Entering various voltage levels and clicking `VSET`, I see this:

    f7010a 0901 006f 166ffd   #  1.11V
    f7010a 0901 006e d7affd   #  1.10V
    f7010a 0901 0070 57a7fd   #  1.12V
    f7010a 0901 0000 5643fd   #  0.00V
    f7010a 0901 0320 576bfd   #  8.00V
    f7010a 0901 0321 96abfd   #  8.01V
    f7010a 0901 032a d76cfd   #  8.10V
    f7010a 0901 0650 55dffd   # 16.16V


### Readouts

At about 1s intervals, the software sends this frame:

    f70103 04 05 e2eafd

Presumably this means "give the current amps, voltage readout and
their set limits". The device typically replies with something like
this:

    f701030405 41 00 00 00 00 00 00 f6 0d 80 a4ebfd

Here are a few voltage readouts along with my speculative headings:

                       ,- ?
              output  /     readout       limit settings    checksum+EOF
    cmd         on?  /    cV      mA        cV    mA
    f701030405  61  00   0000    0000      0000  0d80       45b3fd # 0.0V
    f701030405  61  00   000a    0000      000a  0d80       cfb1fd # 0.1V
    f701030405  61  00   0014    0000      0014  0d80       51b6fd # 0.2V 
    f701030405  61  00   001e    0000      001e  0d80       dbb4fd # 0.3V
    f701030405  61  00   0028    0000      0028  0d80       6db9fd # 0.4V
    f701030405  61  00   0032    0000      0032  0d80       f7bffd # 0.5V
    f701030405  61  00   003c    0000      003c  0d80       79bcfd # 0.6V
    f701030405  61  00   0046    0000      0046  0d80       83a2fd # 0.7V
    f701030405  61  00   0050    0000      0050  0d80       15a7fd # 0.8V
    f701030405  61  00   005a    0000      005a  0d80       9fa5fd # 0.9V
    f701030405  61  00   0064    0000      0064  0d80       21aafd # 1.0V

## Controlling the device from my command line interface

Finally, I now have something to work with. The following command sets
the voltage to 16.16V. The blue led display on my device is
immediately updated as I press enter.

    echo f7010a0901 06 50 55 df fd | xxd -p -r > /dev/ttyUSB0

> TODO: For this to work, you must configure your tty to match the
> `stty` output above.

## Conclusion

Based on these observations, I can make a few guesses. Based on the
0.00V VSET above, I conclude the two last bytes are some form of
checksum. I don't know which, but perhaps CRC16.

       ,----- frame start
      / ,---- device 1 maybe? 
     / /  ,-- set voltage cmd maybe?
    f701 0a0901 0000 5643fd
                  \   \ \ `-- frame end
                   \   `-`- 2 byte checksum
                    `-- 0.00V u16 fixed point (centi-Volts)

I know this is very rudementary. But if I ever decide I need to
control my bench power supply, this is a start.
