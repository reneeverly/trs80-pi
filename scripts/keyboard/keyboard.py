#!/usr/bin/python3

# Script: keyboard.py
# Project: trs80-pi
# License: Apache 2.0
# Author: Renee Waverly Sonntag
#
# Special thanks to Belsamber for the starting point for this script
# https://fadsihave.wordpress.com/2021/01/02/gpio-based-keyboard-on-pine-a64/

# Uses non-standard GRPH and CODE layers, mostly due to difficulty getting
# unicode to work with tty, but also because they don't make that much sense
# especially given how the "[" key works differently due to keymap.

import RPi.GPIO as GPIO
from time import sleep
from evdev import UInput, ecodes as e

# Radio Shack has three vendor IDs for USB, let's use this one:
# "RadioShack Corp. (Tandy)" https://devicehunt.com/view/type/usb/vendor/08B9
VENDOR_TandyRadioShack = 0x08B9

# The case for the TRS-80 Model 100 lists the CatNo. as 26-3802, so I grabbed
# the last four to be the product id for this keyboard
PRODUCT_TRS80 = 0x3802

# I guess this can represent any change in the pin layout
SCRIPTVERSION = 1

# Create the user input device
ui = UInput(
   name = "TRS-80 Model 100 Keyboard",
   vendor = VENDOR_TandyRadioShack,
   product = PRODUCT_TRS80,
   version = SCRIPTVERSION
)

# The keyboard eats up a lot of GPIO pins to the point that it leaves
# some standard interfaces inaccessible.

# The following layouts prioritize leaving a specific interface open
# Pin requirements sourced from: https://pinout.xyz/pinout/spi#

# Layout 1 leaves SPI0 open
#       (device pins 19, 21, 23, 24, 26 or GPIO 10, 9, 11, 8, 7)
# Layout 2 leaves 1-Wire open (Belsamber's original layout)
#       (device pin 7, or GPIO 4)
# Layout 3 leaves PCM and 1-Wire open
#       (device pins 7, 12, 35, 38, 40 or GPIO 4, 18, 19, 20, 21)
# Layout 4 leaves JTAG Alt5 open
#       (device pins 7, 29, 31, 32, 33 or GPIO 4, 5, 6, 12, 13)

USE_PINLAYOUT = 1

if USE_PINLAYOUT == 1:
   # Prioritize SPI0

   # Pin configuration (Keyboard pin -> GPIO pin)
   # black wires     white wires
   # KB   GPIO       KB    GPIO
   #  1 -> 7         11 -> 0
   #  2    11        12    31
   #  3    12        13    32
   #  4    13        14    33
   #  5    15        15    35
   #  6    16        16    36
   #  7    18        17    37
   #  8    22        18    38
   #  9    29        19    40
   # 10 -> 0         20 -> 0

   # Leave open: 19, 21, 23, 24, 26
   cols = [ 7, 11, 12, 13, 15, 16, 18, 22, 29]
   rows = [31, 32, 33, 35, 36, 37, 38, 40]

elif USE_PINLAYOUT == 2:
   # Prioritize 1-Wire (Belsamber's original configuration)

   # Pin configuration (Keyboard pin -> GPIO pin)
   # black wires     white wires
   # KB   GPIO       KB    GPIO
   #  1 -> 11        11 -> 0
   #  2    12        12    23
   #  3    13        13    29
   #  4    15        14    31
   #  5    16        15    32
   #  6    18        16    33
   #  7    19        17    35
   #  8    21        18    36
   #  9    22        19    37
   # 10 -> 0         20 -> 0

   # Leave open: 7
   cols = [11, 12, 13, 15, 16, 18, 19, 21, 22]
   rows = [23, 29, 31, 32, 33, 35, 36, 37]

elif USE_PINLAYOUT == 3:
   # Prioritize PCM and get 1-Wire as a bonus

   # Pin configuration (Keyboard pin -> GPIO pin)
   # black wires     white wires
   # KB   GPIO       KB    GPIO
   #  1 -> 11        11 -> 0
   #  2    13        12    24
   #  3    15        13    26
   #  4    16        14    29
   #  5    18        15    31
   #  6    19        16    32
   #  7    21        17    33
   #  8    22        18    36
   #  9    23        19    37
   # 10 -> 0         20 -> 0

   # Leave open: 7
   # Leave open: 12, 35, 38, 40
   cols = [11, 13, 15, 16, 18, 19, 21, 22, 23]
   rows = [24, 26, 29, 31, 32, 33, 36, 37]

elif USE_PINLAYOUT == 4:
   # Prioritize JTAG Alt5

   # Pin configuration (Keyboard pin -> GPIO pin)
   # black wires     white wires
   # KB   GPIO       KB    GPIO
   #  1 -> 11        11 -> 0
   #  2    12        12    23
   #  3    13        13    24
   #  4    15        14    26
   #  5    16        15    35
   #  6    18        16    36
   #  7    19        17    37
   #  8    21        18    38
   #  9    22        19    40
   # 10 -> 0         20 -> 0

   # Leave open: 7, 29, 31, 32, 33
   cols = [11, 12, 13, 15, 16, 18, 19, 21, 22]
   rows = [23, 24, 26, 35, 36, 37, 38, 40]


NUMCOL = 9


# Default keymap (w/ debug A,B,C,D,E,F)
keymap_default = [
   e.KEY_Z, e.KEY_A, e.KEY_Q, e.KEY_O,          e.KEY_1, e.KEY_9,     e.KEY_SPACE,     e.KEY_F1, e.KEY_LEFTSHIFT,
   e.KEY_X, e.KEY_S, e.KEY_W, e.KEY_P,          e.KEY_2, e.KEY_0,     e.KEY_E,         e.KEY_F2, e.KEY_LEFTCTRL,
   e.KEY_C, e.KEY_D, e.KEY_E, e.KEY_LEFTBRACE,  e.KEY_3, e.KEY_MINUS, e.KEY_TAB,       e.KEY_F3, e.KEY_B,
   e.KEY_V, e.KEY_F, e.KEY_R, e.KEY_SEMICOLON,  e.KEY_4, e.KEY_EQUAL, e.KEY_ESC,       e.KEY_F4, e.KEY_C,
   e.KEY_B, e.KEY_G, e.KEY_T, e.KEY_APOSTROPHE, e.KEY_5, e.KEY_LEFT,  e.KEY_F,         e.KEY_F5, e.KEY_NUMLOCK,
   e.KEY_N, e.KEY_H, e.KEY_Y, e.KEY_COMMA,      e.KEY_6, e.KEY_RIGHT, e.KEY_D,         e.KEY_F6, e.KEY_CAPSLOCK,
   e.KEY_M, e.KEY_J, e.KEY_U, e.KEY_DOT,        e.KEY_7, e.KEY_UP,    e.KEY_A,         e.KEY_F7, e.KEY_RESERVED,
   e.KEY_L, e.KEY_K, e.KEY_I, e.KEY_SLASH,      e.KEY_8, e.KEY_DOWN,  e.KEY_ENTER,     e.KEY_F8, e.KEY_PAUSE,
]

# numlock keymap
keymap_numlock = keymap_default[:]
keymap_numlock[NUMCOL*6 + 2] = e.KEY_4 # U -> 4
keymap_numlock[NUMCOL*7 + 2] = e.KEY_5 # I -> 5
keymap_numlock[3] = e.KEY_6 # O -> 6
keymap_numlock[NUMCOL*6 + 1] = e.KEY_1 # J -> 1
keymap_numlock[NUMCOL*7 + 1] = e.KEY_2 # K -> 2
keymap_numlock[NUMCOL*7] = e.KEY_3 # L -> 3
keymap_numlock[NUMCOL*6] = e.KEY_0 # M -> 0

# graph keymap
keymap_graph = keymap_default[:]
keymap_graph[NUMCOL*2 + 3] = e.KEY_RIGHTBRACE # [ to ] and { to }
keymap_graph[NUMCOL*3 + 6] = e.KEY_GRAVE # [GRPH][ESC] to ` and [GRPH][SHIFT][ESC] to ~
keymap_graph[NUMCOL*7 + 6] = e.KEY_BACKSLASH # [GRPH][ENTER] to \ and [GRPH][SHIFT][ENTER] to |
keymap_graph[NUMCOL*6 + 5] = e.KEY_BRIGHTNESSUP # experimental brightness up
keymap_graph[NUMCOL*7 + 5] = e.KEY_BRIGHTNESSDOWN # experimental brightness down

# We'll just map [CODE] to [COMPOSE] for now so that we have a way to type diacritics.


# Set up GPIO
for row in rows:
   GPIO.setup(row, GPIO.OUT)

for col in cols:
   GPIO.setup(col, GPIO.IN, pull_up_down = GPIO.PUD_down)


# Polling loop
pressed = set()
sleep_time = 1/60
polls_since_press = 0

while True:
   sleep(sleep_time)
   syn = False
   for i in range(len(rows)):
      GPIO.output(rows[i], GPIO.HIGH)
      for j in range(len(cols)):
         keycode = i * (len(rows) + 1) + j
         newval = GPIO.input(cols[j]) == GPIO.HIGH
         if newval and not keycode in pressed:
            pressed.add(keycode)
            ui.write(e.EV_KEY, keymap_default[keycode], 1)
            syn = True
         elif not newval and keycode in pressed:
            pressed.discard(keycode)
            ui.write(e.EV_KEY, keymap_default[keycode], 0)
            syn = True
      if syn:
         ui.syn()
         polls_since_press = 0
         sleep_time = 1/60
      else:
         polls_since_press += 1

      if polls_since_press == 600:
         sleep_time = 1/10
      elif polls_since_press == 1200:
         sleep_time = 1/5
