# trs80-pi
A collection of scripts and programs designed to simulate the user experience of a TRS-80 Model 100 via Raspberry Pi.

## Building Applications

Uses features of the C++17 standard, so GCC 7 or greater recommended.

GCC version 9.1 and lower require an additional flag `-lstdc++fs` which the makefile *should* detect and apply if required.

The following command builds all applications:
```
make
```

## Installing Keyboard

Install dependencies:
```
sudo pip3 install -r scripts/keyboard/requirements.txt
```

Edit line 61 of keyboard.py and select your preferred pin layout.
There are four layouts which each make sure certain pins are left open
depending upon what other things you'd like to use GPIO for.
```python
# Layout 1 leaves SPI0 open
#       (device pins 19, 21, 23, 24, 26 or GPIO 10, 9, 11, 8, 7)
# Layout 2 leaves 1-Wire open (Belsamber's original layout)
#       (device pin 7, or GPIO 4)
# Layout 3 leaves PCM and 1-Wire open
#       (device pins 7, 12, 35, 38, 40 or GPIO 4, 18, 19, 20, 21)
# Layout 4 leaves JTAG Alt5 open
#       (device pins 7, 29, 31, 32, 33 or GPIO 4, 5, 6, 12, 13)

USE_PINLAYOUT = 1 # default
```
Pay very close attention to `STANDARD_ORIENTATION` on line 69.  This should be set to True unless you are using an inverted pin layout.


Then edit /etc/rc.local, adding a call for the keyboard script:
```
/path/to/trs80-pi/scripts/keyboard/keyboard.py
```

## Miscellaneous Quality of Life Changes

Depending on the size of your display, it might be worth it to adjust the font size of the linux framebuffer:
```
sudo dpkg-reconfigure console-setup
```

I used an 8.8" display (1920x480) which Raspberry Pi OS had difficulty interfacing with.  To resolve that, I used the timings provided by innovodesign.
* https://gist.github.com/innovodesign/3f5775d19cb890c0aa59fbb96757bf4b
For screen rotation add `display_hdmi_rotate=1`.
