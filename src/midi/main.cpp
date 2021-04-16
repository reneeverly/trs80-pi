/*
 * Program: midi
 * Project: reneeverly/trs80-pi
 * License: Apache 2.0
 * Author: Renee Waverly Sonntag
 *
 * Description:
 *
 *      This is essentially a wrapper around the ALSA builtin programs
 *      arecordmidi and aplaymidi.  It functions like a tape recorder for
 *      MIDI, providing record/play/stop, track selection,
 *      and input/output selection.
 *
 *      TODO:
 *      [ ] handle midi ports
 *          [ ] get list of midi devices
 *          [ ] allow changing which port is recorded/played
 *      [ ] review previous tracks
 *          [ ] function to get list of .mid files
 *          [ ] how will the file name even be determined?
 *      [ ] implement more functionality for arecordmidi and aplaymidi
 *          [ ] arecordmidi parameters
 *              [ ] ticks
 *              [ ] fps
 *              [ ] bpm
 *              [ ] metronome/timesignature
 *          [ ] aplaymidi parameters
 *              [ ] delay
 */

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

// Terminal manipulation
#include "../../include/rterm.h"

// Keyboard
#include "../../include/rkeyboard.h"

using namespace std;

// Forward declarations
void drawFunctionLabels();

rterm rt;

#define ESCAPEKEY 27

int main(void) {
   rt.clear();

   // Function key labels
   drawFunctionLabels();

   rt.moveCursor(0, 0);

   // Character input loop
   int c;
   int childpid = 0;
   while(true) {
      c = getch();
   
      if (c && c != ESCAPEKEY) {
         // do nothing
      } else {
         // process key press
         int resultant = resolveEscapeSequence();

         // check for child process ended
         if (childpid != 0) {
            int status;
            waitpid(childpid, &status, WNOHANG);

            if (WIFEXITED(status)) {
               // child exited on its own
               childpid = 0;
            } else if (WIFSIGNALED(status)) {
               // child exited with failure?
               childpid = 0;
            } else {
               if (resultant != KEY_F5 && resultant != KEY_F8) {
                  // don't bother doing anything with the key press
                  // We're already either playing or recording
                  continue;
               }
               // else keys F5 and F8 proceed
            }
         }

         if (resultant == KEY_F1) {
            // f1
            childpid = fork();
            if (childpid == 0) {
               execlp("arecordmidi", "arecordmidi", "-p", "20:0", "filename.mid");
               cout << "Failed! (Could not exec.)" << endl;
               exit(-1);
            } else if (childpid < 0) {
               cout << "Failed! (Could not fork.)" << endl;
               childpid = 0;
            } else {
               // parent process
               cout << "Recording... ";
            }
         } else if (resultant == KEY_F2) {
            // f2
            childpid = fork();
            if (childpid == 0) {
               execlp("aplaymidi", "aplaymidi", "--port=20:0", "filename.mid");
               cout << "Failed! (Could not exec.)" << endl;
               exit(-1);
            } else if (childpid < 0) {
               cout << "Failed! (Could not fork.)" << endl;
               childpid = 0;
            } else {
               // parent process
               cout << "Playing... ";
            }
         } else if (resultant == KEY_F3) {
            // f3
            cout << "last track" << endl;
         } else if (resultant == KEY_F4) {
            // f4
            cout << "next track" << endl;
         } else if (resultant == KEY_F5) {
            // f5
            if (childpid != 0) {
               cout << "Stopped" << endl;
               // interrupt, don't kill or terminate
               // we want arecordmidi to finish saving its buffer
               kill(childpid, SIGINT);
               childpid = 0;
            } else {
               cout << "Nothing to stop." << endl;
            }
         } else if (resultant == KEY_F6) {
            // f6
         } else if (resultant == KEY_F7) {
            // f7
            cout << "Switch port." << endl;
         } else if (resultant == KEY_F8) {
            // f8
            exit(0);
         }
      }
   }
}

void drawFunctionLabels() {
   rt.saveCursor();

   // F1
   rt.moveCursor(rt.lines - 1, (rt.cols * 0) / 8);
   cout << "Rcrd";

   // F2
   rt.moveCursor(rt.lines - 1, (rt.cols * 1) / 8);
   cout << "Play";

   // F3
   rt.moveCursor(rt.lines - 1, (rt.cols * 2) / 8);
   cout << "Last";

   // F4
   rt.moveCursor(rt.lines - 1, (rt.cols * 3) / 8);
   cout << "Next";

   // F5
   rt.moveCursor(rt.lines - 1, (rt.cols * 4) / 8);
   cout << "Stop";

   // F7
   rt.moveCursor(rt.lines - 1, (rt.cols * 6) / 8);
   cout << "Port";

   // F8
   rt.moveCursor(rt.lines - 1, (rt.cols * 7) / 8);
   cout << "Menu";

   rt.restoreCursor();
}
