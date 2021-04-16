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
 *      [x] Fetch list of files only in folder
 *      [x] Update time every second
 *      [ ] Figure out how to handle folders / navigate them
 *      [ ] Display list of files as table
 *          [ ] Filler entries where blank
 *      [ ] Allow navigation of table
 *          [ ] Right/left keys (with wrap, even to front)
 *          [ ] Up/down keys (no wrap)
 *          [ ] Scroll window to show more than visible
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
#include <vector>

// Time
#include <chrono>

// Filesystem
#include <filesystem>

// Threads
#include <pthread.h>

// Terminal manipulation
#include "../../include/rterm.h"

// Keyboard
#include "../../include/rkeyboard.h"

using namespace std;

// Forward declaration
void writeDate();
void drawInterface();
void *workerForWriteDate(void *);

// some constants
#define ESCAPEKEY 27

typedef struct _thread_data_t {
   int tid;
} thread_data_t;

rterm rt;

bool clock_loop;
pthread_mutex_t lock_x;

int main() {
   // unbuffer output
   cout << unitbuf;

   // Clear screen
   rt.clear();
   
   // Render the corner labels
   drawInterface();

   // Get list of files in directory
   vector<string> files;
   rt.moveCursor(1, 0);
   for (const auto & entry : filesystem::directory_iterator(".")) {
      // omit directories for now
      if (!filesystem::is_directory(entry)) {
         files.push_back(entry.path().filename());
      }
   }

   // Display list of files
   for (size_t i = 0; i < files.size(); i++) {
      cout << files[i] << endl;
   }
   
   // move the cursor to the prompt line
   rt.moveCursor(rt.lines - 1, 8);

   // start clock worker
   int rc;
   pthread_t thr[1];
   thread_data_t thr_data[1];
   clock_loop = true;
   pthread_mutex_init(&lock_x, NULL);
   thr_data[0].tid = 1;
   if ((rc = pthread_create(&thr[0], NULL, workerForWriteDate, &thr_data[0]))) {
      return 1;
      rt.saveCursor();
      rt.moveCursor(0, 0);
      cout << "Failure initializing clock." << endl;
      rt.restoreCursor();
   } 

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

   // Send signal to clock worker
   clock_loop = false;

   // wait for clock worker to terminate
   pthread_join(thr[0], NULL);

   // exit
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
   filesystem::space_info root = filesystem::space("/");
   cout << setw(19) << root.available << " Bytes free";
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

/**
 * @function workerForWriteDate
 */
void *workerForWriteDate(void *arg) {
   thread_data_t *data = (thread_data_t *)arg;

   cout << unitbuf;

   while (clock_loop) {
      // wait 1 second
      sleep(1);

      // Lock cout
      pthread_mutex_lock(&lock_x);

      // Save the cursor position
      rt.saveCursor();

      // Write the date
      writeDate();

      // Restore the cursor position
      rt.restoreCursor();

      // unlock cout
      pthread_mutex_unlock(&lock_x);
   }

   // clean up
   pthread_exit(NULL);
}
