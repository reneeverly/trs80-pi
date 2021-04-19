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
 *      [ ] Figure out how to handle folders / navigate them
 *      [ ] fork/exec programs selected
 *      [ ] config file? for default programs given extension/format
 *      [ ] read keys into buffer for the "Select:" line
 *          [x] Read utf8 characters into buffer
 *          [x] Pop utf8 characters for backspace/delete
 *          [ ] Tab completion for existing files or programs
 *          [ ] Create new text file when path not exist
 *          [ ] Key combo to clear buffer
 */

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

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

// FileBrowser
# include "FileBrowser.h"

using namespace std;

// some constants
#define ESCAPEKEY 27

typedef struct _thread_data_t {
   int tid;
} thread_data_t;

rterm rt;
FileBrowser* fb;

bool clock_loop;
pthread_mutex_t lock_x;

// Forward declaration
void writeDate();
void drawInterface();
void *workerForWriteDate(void *);
bool sortAlphabetic(string one, string two);
bool sortReverseAlphabetic(string one, string two);
void sigintHandler(int signum);
void pop_back_utf8(string& utf8);
size_t length_utf8(const string& str);

int main() {
   // unbuffer output
   cout << unitbuf;

   // Clear screen
   rt.clear();

   // register SIGING handler
   signal(SIGINT, sigintHandler);
   
   // Render the corner labels
   drawInterface();

   // Get list of files in directory
   vector<string> files;
   vector<string> apps;
   rt.moveCursor(1, 0);
   for (const auto & entry : filesystem::directory_iterator(".")) {
      // omit directories and whatnot
      if (filesystem::is_regular_file(entry)) {
         // get permissions using bitwise and (not logical and)
         if ((filesystem::status(entry).permissions() & filesystem::perms::others_exec) != filesystem::perms::none) {
            // executable
            apps.push_back(entry.path().filename());
         } else {
            // not executable
            files.push_back(entry.path().filename());
         }
      }
   }

   // sort alphabetically
   stable_sort(apps.begin(), apps.end(), sortReverseAlphabetic);
   stable_sort(files.begin(), files.end(), sortAlphabetic);
   
   // prepend files with apps
   for (auto appname : apps) {
      files.emplace(files.begin(), appname);
   }
   

   // Display list of files
   fb = new FileBrowser(&rt, &files);
   

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

   // move the cursor to the prompt line
   rt.moveCursor(rt.lines - 1, 8);

   // Character input loop
   int c;
   string searchKey = "";
   while(true) {
      c = getch();
      
      // lock cout mutex
      pthread_mutex_lock(&lock_x);
   
      if (c && c != ESCAPEKEY) {

         if ((c == '\n') && (searchKey.length() == 0)) {
            // get index
            size_t i = fb->getIndex();
            // fork exec
            //int pid = fork();
            // exec(files[i]);
         } else if ((c == '\n') && (searchKey.length() > 0)) {
            // validate filename,
               // forkexec
         } else if ((c == '\t') && (searchKey.length() > 0)) {
            // scan for partial matches
            // if only one, set it
            // else do nothing
         } else if ((c == 0x08) || (c == 0x7f)) {
            // backspace!
            pop_back_utf8(searchKey);
         } else if (c >= 0x20 ) {
            // a regular old letter
            searchKey.push_back(c);
         }
         
         // move cursor to prompt line
         rt.moveCursor(rt.lines - 1, 8);
         // print length + 1 blanks
         cout << setw(length_utf8(searchKey) + 1) << "";
            
         // move cursor to prompt line
         rt.moveCursor(rt.lines - 1, 8);
         // print searchKey in full
         cout << searchKey;
      } else {
         int resultant = resolveEscapeSequence();
   
         if (resultant == KEY_RIGHT) {
            fb->pressedRight();
         } else if (resultant == KEY_LEFT) {
            fb->pressedLeft();
         } else if (resultant == KEY_UP) {
            fb->pressedUp();
         } else if (resultant == KEY_DOWN) {
            fb->pressedDown();
         }
      }

      // unlock cout mutex
      pthread_mutex_unlock(&lock_x);
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

/**
 * @function sortAlphabetic
 * Via stable_sort sorts alphabetically
 */
bool sortAlphabetic(string one, string two) {
   return one < two;
}

/**
 * @function sortReverseAlphabetic
 * Via stable_sort sorts reverse alphabetically
 */
bool sortReverseAlphabetic(string one, string two) {
   return one > two;
}

void sigintHandler(int signum) {
   // deconstruct FileBrowser (restore default terminal scroll)
   delete fb;

   exit(signum);
}

/**
 * @function pop_back_utf8
 * https://stackoverflow.com/questions/37623359/how-to-remove-the-last-character-of-a-utf-8-string-in-c
 */
void pop_back_utf8(string& utf8) {
    if(utf8.empty())
        return;

    auto cp = utf8.data() + utf8.size();
    while(--cp >= utf8.data() && ((*cp & 0b10000000) && !(*cp & 0b01000000))) {}
    if(cp >= utf8.data())
        utf8.resize(cp - utf8.data());
}

/**
 * @function length_utf8
 * https://stackoverflow.com/questions/4063146/getting-the-actual-length-of-a-utf-8-encoded-stdstring
 */
size_t length_utf8(const string& str) {
    size_t c,i,ix,q;
    for (q=0, i=0, ix=str.length(); i < ix; i++, q++)
    {
        c = (unsigned char) str[i];
        if      (c>=0   && c<=127) i+=0;
        else if ((c & 0xE0) == 0xC0) i+=1;
        else if ((c & 0xF0) == 0xE0) i+=2;
        else if ((c & 0xF8) == 0xF0) i+=3;
        //else if (($c & 0xFC) == 0xF8) i+=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
        //else if (($c & 0xFE) == 0xFC) i+=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
        else return 0;//invalid utf8
    }
    return q;
}
