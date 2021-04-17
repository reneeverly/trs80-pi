/*
 * Class: rtui
 * Project: reneeverly/trs80-pi
 * License: Apache 2.0
 * Author: Renee Waverly Sonntag
 * 
 * Description:
 *
 *      Wraps around an rterm and provides more advanced graphics functions.
 */

#ifndef RTUI_H
#define RTUI_H

#include <string>

#include "rterm.h"

using namespace std;

class rtui {
   private:
      rterm* rt;
   public:
      rtui(rterm* newrt);

      void moveCursorToTop();
      void moveCursorToBottom();
      void moveCursorToVeryBottom();

      void scrollSpecial();
      void scrollDefault();

      void scrollUp();
      void scrollDown();

      void drawFunctionLabels(const string labels[8]);
      void drawFunctionLabels(const string, const string, const string,
         const string, const string, const string, const string, const string);
};

/**
 * @constructs rtui
 */
rtui::rtui(rterm* newrt) {
   rt = newrt;
}

/**
 * @method moveCursorToTop
 * Moves the cursor to the top of the scrolling region
 */
void rtui::moveCursorToTop() {
   rt->moveCursor(0, 0);
}

/**
 * @method moveCursorToButton
 * Moves the cursor to the bottom of the scrolling region
 */
void rtui::moveCursorToBottom() {
   rt->moveCursor(rt->lines - 2, 0);
}

/**
 * @method moveCursorToVeryBottom
 * Moves the cursor to the very bottom of the terminal
 */
void rtui::moveCursorToVeryBottom() {
   rt->moveCursor(rt->lines - 1, 0);
}

/**
 * @method scrollSpecial
 * Sets up the terminal to scroll in the preferred behavior.
 * Omits the very last line of the terminal so that the labels aren't
 * overwritten.
 */
void rtui::scrollSpecial() {
   rt->changeScrollRegion(0, rt->lines - 2);
}

/**
 * @method scrollDefault
 * Sets up scrolling to the standard behavior of the terminal.
 * The very last line of the terminal will be overwritten when scrolling.
 */
void rtui::scrollDefault() {
   rt->changeScrollRegion(0, rt->lines - 1);
}

/**
 * @TODO
 */
void rtui::scrollUp() {
}

/**
 * @TODO
 */
void rtui::scrollDown() {
}

/**
 * @method drawFunctionLabels
 * Draws function labels in the last line of the screen
 * @param {const string [8]} labels - the labels to use
 */
void rtui::drawFunctionLabels(const string labels[8]) {
   rt->saveCursor();

   for (size_t i = 0; i < 8; i++) {
      rt->moveCursor(rt->lines - 1, (rt->cols * i) / 8);
      cout << labels[i];
   }

   rt->restoreCursor();
}

/**
 * @method drawFunctionLabels_alternate
 * @see drawFunctionLabels
 * Provides a way to draw function labels without making your own array.
 */
void rtui::drawFunctionLabels(const string f1, const string f2, const string f3,
      const string f4, const string f5, const string f6, const string f7,
      const string f8) {
   string composed[8] = {f1, f2, f3, f4, f5, f6, f7, f8};
   drawFunctionLabels(composed);
}

#endif
