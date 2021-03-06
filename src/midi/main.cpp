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
#include <string>
#include <sstream>
#include <map>

// Terminal manipulation
#include "../../include/rterm.h"

// Text User Interface
#include "../../include/rtui.h"

// Keyboard
#include "../../include/rkeyboard.h"

using namespace std;

// Forward declarations
void drawFunctionLabels();

rterm rt;
rtui ui(&rt);

#define ESCAPEKEY 27

int main(void) {
   string midiport;

   rt.clear();

   // Function key labels
   ui.drawFunctionLabels("Rcrd", "Play", "Prev", "Next",
      "Stop", "", "Port", "Menu");
   ui.scrollSpecial();

   rt.moveCursor(0, 0);

   // get the midi ports
   string rawports = rt.exec("arecordmidi -l");
   istringstream rawportlist(rawports);
   map<string, string> midiports;
   while (!rawportlist.eof()) {
      // get the actual port number
      string portnum;
      rawportlist >> portnum;
      // get the label (and extra field)
      char description[256];
      rawportlist.getline(description, 256);
      // package it up
      midiports.emplace(portnum, description);
   } 

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
               execlp("arecordmidi", "arecordmidi", ("--port=" + midiport).c_str(), "filename.mid");
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
               execlp("aplaymidi", "aplaymidi", ("--port=" + midiport).c_str(), "filename.mid");
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
            for (auto& x: midiports)
               cout << x.first << ":" << x.second << endl;
         } else if (resultant == KEY_F8) {
            // f8
            rt.resetTerminal();
            exit(0);
         }
      }
   }
}
