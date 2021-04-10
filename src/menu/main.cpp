/*
 * Program: menu
 * Project: reneeverly/trs80-pi
 * License: Apache 2.0
 * Author: Renee Waverly Sonntag
 *
 * Description:
 *
 *      This simulates the MENU application of the TRS-80 Model 100.
 *      Provides a list of applications and files with some additional
 *      information like the time and available disk space.
 *
 *      Currently implemented without ncurses for the reasons explained
 *      in rterm.h.
 *
 *      TODO:
 *      [ ] Fetch list of files in folder
 *      [ ] Display list of files as table
 *          [ ] Filler entries where blank
 *      [ ] Allow navigation of table
 *          [ ] Right/left keys (with wrap)
 *          [ ] Up/down keys (no wrap)
 *      [ ] fork/exec programs selected
 *      [ ] read keys into buffer for the "Select:" line
 *          [ ] Tab completion for existing files or programs
 *          [ ] Create new text file when path not exist
 */

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>

// Disk Usage
#include <sys/statvfs.h>

// Time
#include <chrono>

// Terminal manipulation
#include "../../include/rterm.h"

// Keyboard
#include "../../include/rkeyboard.h"

using namespace std;

// Forward declaration
void writeDate();
void drawInterface();

// some constants
#define ESCAPEKEY 27

rterm rt;

int main() {
   // Clear screen
   rt.clear();
   
   // Render the corner labels
   drawInterface();
   
   // move the cursor to the prompt line
   rt.moveCursor(rt.lines - 1, 8);
   
   // Character input loop
   int c;
   while(true) {
      c = getch();
   
      if (c && c != ESCAPEKEY) {
         // Depending upon how this works, we might need to
         // buffer character input for new file creation or
         // "searching" for files that exist.
      } else {
         int resultant = resolveEscapeSequence();
   
         if (resultant == KEY_RIGHT) {
            // right
         } else if (resultant == KEY_LEFT) {
            // left
         } else if (resultant == KEY_UP) {
            // up
         } else if (resultant == KEY_DOWN) {
            // down
         }
      }
   }
   return 0;
}

/**
 * @function drawInterface
 * Clears the screen and draws out the corner text fields that
 * are visible on the menu screen of the TRS-80 Model 100.
 * Time, copyright, prompt, and available storage space.
 */
void drawInterface() {
   // Clear screen
   rt.clear();

   // Write date (top left)
   writeDate();

   // Write copyright (top right)
   string copyright = "(C) Renee Waverly Sonntag";
   rt.moveCursor(0, rt.cols - copyright.length());
   cout << copyright;

   // Write prompt (bottom left)
   rt.moveCursor(rt.lines - 1, 0);
   cout << "Select: ";

   // Write disk usage (bottom right)
   rt.moveCursor(rt.lines - 1, rt.cols - 30);
   struct statvfs fiData;
   if (statvfs("/", &fiData) < 0) {
      cout << "Failed to get Disk Data.";
   } else {
      /*
       * Will overflow if trying to multiply too high.
       * Let's use KBytes instead of Bytes to reduce that chance.
       *
       * f_bavail is the total number of available blocks
       * f_frsize is the size in bytes of the minimum unit of allocation
       *      Note that statvfs is different from statfs.
       *      This is equivalent to statfs's f_bsize.
       *      statvfs has a differnt  member f_bsize which is the I/O request length.
       */
      unsigned long kbytesfree = fiData.f_bavail * (fiData.f_frsize / 1024);
      cout << setw(15) << kbytesfree << " Kilobytes free";
   }
}

/**
 * @function writeDate
 * Places the date string in the rop left corner of the screen.
 * Interestingly, it will always be the same length, so we
 * should be fine with just simply writing over it every second.
 */
void writeDate() {
   rt.moveCursor(0,0);
   auto now = time(nullptr);
   cout << put_time(localtime(&now), "%b %d, %Y %a %H:%M:%S");
}

