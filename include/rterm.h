/*
 * Class: rterm
 * Project: reneeverly/trs80-pi
 * License: Apache 2.0
 * Author: Renee Waverly Sonntag
 * 
 * Description:
 *
 *      This class provides a simple interface for manipulating the terminal.
 *      It uses terminfo/termcap to fetch the required sequences on initialization,
 *      but makes no further shell invocations.
 *
 *      Ideally I would have used ncurses or a similar implementation,
 *      but I was borrowing a Raspberry Pi which did not have the development
 *      headers installed while waiting for mine to arrive.  Maybe I'll port it
 *      to ncurses after everything is settled.
 */

#ifndef RTERM_H
#define RTERM_H

#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdexcept>
#include <stack>
#include <sstream>

using namespace std;

class rterm {
   private:
      string sClear;
      string sMoveCursor;
      string sReverse;
      string sResetAttributes;
      string sSaveCursor;
      string sRestoreCursor;
      string sChangeScroll;
      
      string ToHex(const string&, const bool); /* for debugging */
      string processUnescapedSequence(const string, const int, const int);
      
   public:
      int cols;
      int lines;
      
      rterm();
      string exec(const char*);
      
      bool updateDimensions();
      void clear();
      void moveCursor(const int, const int);
      void reverse();
      void resetAttributes();
      void saveCursor();
      void restoreCursor();
      void changeScrollRegion(const int, const int);
      
      string getReverse();
      string getResetAttributes();
      string getSaveCursor();
      string getRestoreCursor();
};

/**
 * @constructs rterm
 * Gets the necessary control sequences and the dimensions of the terminal.
 */
rterm::rterm() {
   // Get the control sequence for clear
   sClear = exec("tput clear");
   
   // Get the unescaped control sequence for moving the cursor
   sMoveCursor = exec("tput cup");
   
   // Get the control sequence for reverse color
   sReverse = exec("tput rev");
   
   // Get the control sequence for reset attributes
   sResetAttributes = exec("tput sgr0");

   // Get the control sequence for save cursor
   sSaveCursor = exec("tput sc");

   // Get the control sequence for restore cursor
   sRestoreCursor = exec("tput rc");

   // Get the unescaped control sequence for changing the scroll region
   sChangeScroll = exec("tput csr");
   
   // Get the dimensions of the terminal
   updateDimensions();
}

/**
 * @method exec
 * Executes the provided command in the shell and returns the std output.
 * @param {const char*} cmd - the shell command to execute.
 * @returns {string} the std output of the shell command.
 * @throws {runtime_error} when popen() fails.
 */
string rterm::exec(const char* cmd) {
   char buffer[128];
   string result = "";
   FILE* pipe = popen(cmd, "r");
   if (!pipe) throw runtime_error("popen() failed!");
   try {
      while (fgets(buffer, sizeof buffer, pipe) != NULL) {
      result += buffer;
      }
   } catch (...) {
      pclose(pipe);
      throw;
   }
   pclose(pipe);
   return result;
}

/**
 * @method updateDimensions
 * Reaches out to terminfo to get the dimensions of the terminal.
 * @returns {bool} true if success, false if failure.
 */
bool rterm::updateDimensions() {
   try {
      string sCols = exec("tput cols");
      string sLines = exec("tput lines");
      
      cols = stoi(sCols);
      lines = stoi(sLines);
   } catch(...) {
      return false;
   }
   return true;
}

/**
 * @method moveCursor
 * Moves the terminal cursor to the provided coordinates.
 * @param {const int} line - the line to move the cursor to.
 * @param {const int} col - the column to move the cursor to.
 * @todo Check for valid coordinates given current terminal dimensions.
 */
void rterm::moveCursor(const int line, const int col) {
   cout << processUnescapedSequence(sMoveCursor, line, col);
}

/**
 * @private
 * @method processUnescapedSequence
 * Processes the sequence with the provided parameters
 * @param {const string} originalSequence - the sequence to process
 * @param {const int} param1 - the first parameter
 * @param {const int} param2 - the second parameter
 */
string rterm::processUnescapedSequence(const string originalSequence, const int param1, const int param2) {
   string swap = originalSequence;
   int nParam1 = param1;
   int nParam2 = param2;
   
   // Check for flag to increment numeric parameters by 1.
   size_t i = swap.find("%i");
   if (i != string::npos) {
      swap.erase(i,2);
      nParam1 += 1;
      nParam2 += 1;
   }
   
   // Filter through parameters and printing them
   stack<int> pStack;
   size_t nextP = swap.find("%p");
   size_t nextD = swap.find("%d");
   while ((nextP != string::npos) || (nextD != string::npos)) {
      if (nextP < nextD) {
         if (swap[nextP + 2] == '1') {
            pStack.push(nParam1);
         } else {
            pStack.push(nParam2);
         }
         swap.erase(nextP, 3);
      } else {
         swap.replace(nextD, 2, to_string(pStack.top()));
         pStack.pop();
      }
      
      nextP = swap.find("%p");
      nextD = swap.find("%d");
   }
   return swap;
}

/**
 * @method clear
 * Clear the screen.
 */
void rterm::clear() {
   cout << sClear;
}

/**
 * @method reverse
 * Sets the "reverse" attribute, meaning that future text will be
 * written with inverted colors. This is not a "toggle" function.
 * (If bright text on dark background, will be dark text on bright background.)
 * @see resetAttributes for undoing this command.
 */
void rterm::reverse() {
   cout << sReverse;
}

/**
 * @method resetAttributes
 * Resets all attributes; future text will be written
 * with terminal default attributes.
 */
void rterm::resetAttributes() {
   cout << sResetAttributes;
}

/**
 * @method saveCursor
 * Saves the position of the cursor (nonstackable).
 */
void rterm::saveCursor() {
   cout << sSaveCursor;
}

/**
 * @method restoreCursor
 * Restores the position of the cursor (nonstackable).
 */
void rterm::restoreCursor() {
   cout << sRestoreCursor;
}

/**
 * @method changeScrollRegion
 * Adjusts the scroll region to be from the provided lines
 * @param {const int} firstline - the first line to be in the scroll region
 * @param {const int} lastline - the last line to be in the scroll region
 */
void rterm::changeScrollRegion(const int firstline, const int lastline) {
   cout << processUnescapedSequence(sChangeScroll, firstline, lastline);
}

/**
 * @method getReverse
 * @see reverse
 * Instead of executing the reverse control sequence, this method gives it to
 * you in string form.  Useful for cout inlining of output manipulation.
 * @returns {string} the control sequence for reversing colors.
 */
string rterm::getReverse() {
   return sReverse;
}

/**
 * @method getResetAttributes
 * @see resetAttributes
 * Instead of executing the reset attributes control sequence, this method gives
 * it to you in string form.  Useful for cout inlining of output manipulation.
 * @returns {string} the control sequence for reseting attributes.
 */
string rterm::getResetAttributes() {
   return sResetAttributes;
}

/**
 * @method getSaveCursor
 * @see saveCursor
 * Instead of executing the save cursor control sequence, this method gives it
 * to you in string form.  Useful for cout inlining of output manipulation.
 * @returns {string} the control sequence for saving the cursor.
 */
string rterm::getSaveCursor() {
   return sSaveCursor;
}

/**
 * @method getRestoreCursor
 * @see restoreCursor
 * Instead of executing the restore cursor control sequence, this method gives
 * it to you in string form.  Useful for cout inlining of output manipulation.
 * @returns {string} the control sequence for restoring the cursor.
 */
string rterm::getRestoreCursor() {
   return sRestoreCursor;
}

/**
 * @private
 * @method ToHex
 * Used for debugging; returns a string (or control sequence in our context)
 * as hex digits.
 * @returns {string} the hex representation of the string.
 */
string rterm::ToHex(const string& s, const bool upper_case = true) {
   ostringstream ret;

   for (string::size_type i = 0; i < s.length(); ++i) {
      ret << hex << setfill('0') << setw(2) << (upper_case ? uppercase : nouppercase) << (int)s[i];
   }

   return ret.str();
}

#endif

