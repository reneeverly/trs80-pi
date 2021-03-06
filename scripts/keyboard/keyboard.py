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
import logging
import copy

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

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

# Set up notation for GPIO
# Let's use board/device notation rather than GPIO notation.
GPIO.setmode(GPIO.BOARD)

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

# Depending on how the Pi GPIO port is oriented, you might end up with a
# situation where every single jumper wire crosses over another one.  This makes
# tracing contacts and keeping things in the correct order rather difficult.

# If you're in that sort of situation, inverting the pin layout might help.
# Should be True by default, but my Pi is in backwards so rip.
STANDARD_ORIENTATION = True

if USE_PINLAYOUT == 1:
   # Prioritize SPI0 
   # Leave open: 19, 21, 23, 24, 26

   if STANDARD_ORIENTATION:
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

      cols = [ 7, 11, 12, 13, 15, 16, 18, 22, 29]
      rows = [31, 32, 33, 35, 36, 37, 38, 40]

   else: # Reverse pinlayout
      # Pin configuration (Keyboard pin -> GPIO pin)
      # black wires     white wires
      # KB   GPIO       KB    GPIO
      #  1 -> 40         11 -> 0
      #  2    38        12    22
      #  3    37        13    18
      #  4    36        14    16
      #  5    35        15    15
      #  6    33        16    13
      #  7    32        17    12
      #  8    31        18    11
      #  9    29        19    7
      # 10 -> 0         20 -> 0

      cols = [40, 38, 37, 36, 35, 33, 32, 31, 29]
      rows = [22, 18, 16, 15, 13, 12, 11,  7]

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


# Non-standard keys:
#       row 2, col 8:   [GRPH]  set to KEY_RESERVED
INDEX_GRAPHKEY = NUMCOL * 2 + 8
#       row 3, col 8:   [CODE]  set to KEY_COMPOSE
#       row 4, col 6:   [PASTE] set to KEY_F9
#       row 4, col 8:   [NUM]   set to KEY_RESERVED
INDEX_NUMLOCK = NUMCOL * 4 + 8
#       row 5, col 6:   [LABEL] set to KEY_F10
#       row 5, col 8:   [CAPS]  set to KEY_CAPSLOCK
INDEX_CAPSLOCK = NUMCOL * 5 + 8
#       row 6, col 6:   [PRINT] set to KEY_SYSRQ
#       row 6, col 8:   This key does not exist. (KEY_RESERVED)

# Standard Keys: (Which we'd like to know the location of)
#       row 0, col 8:   [SHIFT] set to KEY_LEFTSHIFT
INDEX_LEFTSHIFT = NUMCOL * 0 + 8

# Default Keymap
keymaps = [[
   [e.KEY_Z],[e.KEY_A],[e.KEY_Q],[e.KEY_O],       [e.KEY_1],[e.KEY_9],   [e.KEY_SPACE],  [e.KEY_F1],[e.KEY_LEFTSHIFT],
   [e.KEY_X],[e.KEY_S],[e.KEY_W],[e.KEY_P],       [e.KEY_2],[e.KEY_0],  [e.KEY_BACKSPACE],[e.KEY_F2],[e.KEY_LEFTCTRL],
   [e.KEY_C],[e.KEY_D],[e.KEY_E],[e.KEY_LEFTBRACE],[e.KEY_3],[e.KEY_MINUS],[e.KEY_TAB],   [e.KEY_F3],[e.KEY_RESERVED],
   [e.KEY_V],[e.KEY_F],[e.KEY_R],[e.KEY_SEMICOLON],[e.KEY_4],[e.KEY_EQUAL],[e.KEY_ESC],   [e.KEY_F4],[e.KEY_COMPOSE],
   [e.KEY_B],[e.KEY_G],[e.KEY_T],[e.KEY_APOSTROPHE],[e.KEY_5],[e.KEY_LEFT],[e.KEY_F9],    [e.KEY_F5],[e.KEY_RESERVED],
   [e.KEY_N],[e.KEY_H],[e.KEY_Y],[e.KEY_COMMA],   [e.KEY_6],[e.KEY_RIGHT],[e.KEY_F10],    [e.KEY_F6],[e.KEY_CAPSLOCK],
   [e.KEY_M],[e.KEY_J],[e.KEY_U],[e.KEY_DOT],     [e.KEY_7],[e.KEY_UP],   [e.KEY_SYSRQ],  [e.KEY_F7],[e.KEY_RESERVED],
   [e.KEY_L],[e.KEY_K],[e.KEY_I],[e.KEY_SLASH],   [e.KEY_8],[e.KEY_DOWN], [e.KEY_ENTER],  [e.KEY_F8],[e.KEY_PAUSE],
]]

# Shift key behavior for specific keys
SHIFT_DEFAULT = 0 # When shift key is pressed, key will be shifted
SHIFT_ALWAYS = 1 # Regardless of shift key, key will always be shifted
SHIFT_NEVER = -1 # Regardless of shift key, key will never be shifted

# Set default keymap shift behavior to default
for key in keymaps[0]:
   key.extend([SHIFT_DEFAULT])

# --
# Keymap generators
# --

# Numlock Keymap
def addNumpad(keymap):
   keymap[NUMCOL * 6 + 2] = [e.KEY_4, SHIFT_NEVER] # U -> 4
   keymap[NUMCOL * 7 + 2] = [e.KEY_5, SHIFT_NEVER] # I -> 5
   keymap[3] = [e.KEY_6, SHIFT_NEVER] # O -> 6
   keymap[NUMCOL * 6 + 1] = [e.KEY_1, SHIFT_NEVER] # J -> 1
   keymap[NUMCOL * 7 + 1] = [e.KEY_2, SHIFT_NEVER] # K -> 2
   keymap[NUMCOL * 7] = [e.KEY_3, SHIFT_NEVER] # L -> 3
   keymap[NUMCOL * 6] = [e.KEY_0, SHIFT_NEVER] # M -> 0

# shiftfix keymap
def addShift(keymap):
   keymap[NUMCOL * 2 + 3] = [e.KEY_RIGHTBRACE, SHIFT_NEVER] # shift of "[" is "]"

# graph keymap
def addGraph(keymap):
   keymap[NUMCOL * 2 + 3] = [e.KEY_LEFTBRACE, SHIFT_ALWAYS] # [ to {
   keymap[NUMCOL * 3 + 6][0] = e.KEY_GRAVE # [GRPH][ESC] to ` and [GRPH][SHIFT][ESC] to ~
   keymap[NUMCOL * 7 + 6][0] = e.KEY_BACKSLASH # [GRPH][ENTER] to \ and [GRPH][SHIFT][ENTER] to |
   keymap[NUMCOL * 6 + 5][0] = e.KEY_BRIGHTNESSUP # experimental brightness up
   keymap[NUMCOL * 7 + 5][0] = e.KEY_BRIGHTNESSDOWN # experimental brightness down
   keymap[NUMCOL * 4 + 5][0] = e.KEY_HOME
   keymap[NUMCOL * 5 + 5][0] = e.KEY_END
   # the "." key on my TRS is broken, so here's a map of [GRPH][,] to .
   keymap[NUMCOL * 5 + 3][0] = e.KEY_DOT

# shiftfix at the same time as graph
def addGraphshift(keymap):
   keymap[NUMCOL * 2 + 3] = [e.KEY_RIGHTBRACE, SHIFT_ALWAYS] # ] to }

# --
# Generate keymaps
# --

# Keymap resolver
def resolveKeymap():
   return keymaps[
      (0, 1) [INDEX_NUMLOCK in pressed]
      + (0, 2) [INDEX_LEFTSHIFT in pressed]
      + (0, 4) [INDEX_GRAPHKEY in pressed]
   ]

keymaps.extend([None] * 7)

keymaps[1] = copy.deepcopy(keymaps[0])
addNumpad(keymaps[1])

keymaps[2] = copy.deepcopy(keymaps[0])
addShift(keymaps[2])

keymaps[3] = copy.deepcopy(keymaps[0])
addNumpad(keymaps[3])
addShift(keymaps[3])

keymaps[4] = copy.deepcopy(keymaps[0])
addGraph(keymaps[4])

keymaps[5] = copy.deepcopy(keymaps[0])
addGraph(keymaps[5])
addNumpad(keymaps[5])

keymaps[6] = copy.deepcopy(keymaps[0])
addGraph(keymaps[6])
addGraphshift(keymaps[6])

keymaps[7] = copy.deepcopy(keymaps[0])
addGraph(keymaps[7])
addGraphshift(keymaps[7])
addNumpad(keymaps[7])

# We'll just map [CODE] to [COMPOSE] for now so that we have a way to type diacritics.
# Also, note that the compose layer is editable, and supports UTF-8 sequences with recent kernels.

# Set up GPIO
for row in rows:
   logging.info(f"Setting pin {row} as an output")
   GPIO.setup(row, GPIO.OUT)

for col in cols:
   logging.info(f"Setting pin {col} as an input")
   GPIO.setup(col, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)


# Polling loop

pressed = set()
repeated = set()
sleep_time = 1/60
polls_since_press = 0

while True:

   try:
      sleep(sleep_time)
      syn = False
      for i in range(len(rows)):
         GPIO.output(rows[i], GPIO.HIGH)
         for j in range(len(cols)):
            keycode = i * (len(rows) + 1) + j
            newval = GPIO.input(cols[j]) == GPIO.HIGH
            if newval and not keycode in pressed:
               # The key is being pressed for the first time
               pressed.add(keycode)

               # check for shift behavior
               if INDEX_LEFTSHIFT in pressed and resolveKeymap()[keycode][1] == SHIFT_NEVER:
                  ui.write(e.EV_KEY, e.KEY_LEFTSHIFT, 0)
               elif not INDEX_LEFTSHIFT in pressed and resolveKeymap()[keycode][1] == SHIFT_ALWAYS:
                  ui.write(e.EV_KEY, e.KEY_LEFTSHIFT, 1)

               # Turn on the key
               ui.write(e.EV_KEY, resolveKeymap()[keycode][0], 1)

               # If capslock, immediately turn it back off
               if keycode == INDEX_CAPSLOCK:
                  ui.write(e.EV_KEY, resolveKeymap()[keycode][0], 0)

               syn = True
            elif newval and keycode in pressed and not keycode in repeated and polls_since_press == 30:
               # the key is still pressed and it's been a half of a second since

               # unless capslock or shift, trigger typematic
               if keycode != INDEX_CAPSLOCK and keycode != INDEX_LEFTSHIFT \
                     and keycode != INDEX_NUMLOCK and keycode != INDEX_GRAPHKEY:
                  repeated.add(keycode)
                  ui.write(e.EV_KEY, resolveKeymap()[keycode][0], 2)
               
                  syn = True
            elif newval and keycode in pressed and keycode in repeated and polls_since_press == 5:
               # typematic is enabled for this key on 5/60 second delay
               ui.write(e.EV_KEY, resolveKeymap()[keycode][0], 2)

               syn = True
            elif not newval and keycode in pressed:
               # the key is being released
               pressed.discard(keycode)
               repeated.discard(keycode)

               # check for shift behavior
               if INDEX_LEFTSHIFT in pressed and resolveKeymap()[keycode][1] == SHIFT_NEVER:
                  ui.write(e.EV_KEY, e.KEY_LEFTSHIFT, 1)
               elif not INDEX_LEFTSHIFT in pressed and resolveKeymap()[keycode][1] == SHIFT_ALWAYS:
                  ui.write(e.EV_KEY, e.KEY_LEFTSHIFT, 0)

               # If capslock, turn it back on temporarily
               if keycode == INDEX_CAPSLOCK:
                  ui.write(e.EV_KEY, resolveKeymap()[keycode][0], 1)

               # Turn off the key
               ui.write(e.EV_KEY, resolveKeymap()[keycode][0], 0)

               syn = True
         GPIO.output(rows[i], GPIO.LOW)
      if syn:
         ui.syn()
         polls_since_press = 0
         sleep_time = 1/60
      else:
         polls_since_press += 1
   
      if polls_since_press == 600: # 10 seconds
         sleep_time = 1/10
      elif polls_since_press == 1200: # 120 seconds after 10 seconds
         sleep_time = 1/5

   except KeyboardInterrupt:
      continue

GPIO.cleanup()
