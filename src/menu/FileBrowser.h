/*
 * Class: rtui
 * Program: menu
 * Project: reneeverly/trs80-pi
 * License: Apache 2.0
 * Author: Renee Waverly Sonntag
 * 
 * Description:
 *
 *      Provides a file browser which takes advantage of rterm and handles
 *      all drawing functionality related to selecting a file.
 */

#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <string>
#include <vector>

// terminal manipulation
#include "../../include/rterm.h"

// Temporary UTF8 support
#include "../../include/temporary_utf8.h"

class FileBrowser {
   private:
      rterm* rt;

      const vector<string>* items;
      size_t selectedIndex;

      size_t itemsPerLine;
      
   public:
      FileBrowser(rterm*, const vector<string>*);

      void redrawTable();

      void pressedLeft();
      void pressedRight();
      void pressedUp();
      void pressedDown();
      void setIndex(const size_t);

      size_t getIndex();
};

/**
 * @constructs FileBrowser
 * @param {rterm*} newrt - the rterm object to reference for terminal manip.
 */
FileBrowser::FileBrowser(rterm* newrt, const vector<string>* newitems) {
   rt = newrt;
   items = newitems;
   selectedIndex = 0;

   // set scroll region to just the table
   rt->changeScrollRegion(1, rt->lines - 2);

   redrawTable();
}

/**
 * @method redrawTable
 * Does all of the calculations required for laying out the files in a table
 * and then draw them to the screen.
 */
void FileBrowser::redrawTable() {
   size_t preferredNameLength;

   // Get the longest filename in the vector
   size_t longestNameLength = 0;
   for (auto iter : *items) {
      if (length_utf8(iter) > longestNameLength) {
         longestNameLength = length_utf8(iter);
      }
   }
   longestNameLength++; // allow for spacing

   if (longestNameLength > (rt->cols / 4)) {
      // cap the length if it's longer than one fourth of the screen width
      preferredNameLength = (rt->cols / 4);
   } else {
      // let's loop until we get an ideal number of columns
      for (size_t i = 5; i < 16; i++) {
         preferredNameLength = rt->cols / (i-1);
         if (longestNameLength > (rt->cols / i)) {
            break;
         }
      }
   }

   // adjust the object-wide variable because it's used by pressedDown/Up
   itemsPerLine = rt->cols / preferredNameLength;

   size_t itemsPerPage = itemsPerLine * (rt->lines - 2);

   // determine which page we need to render
   size_t pageNumber = selectedIndex / itemsPerPage;

   // Save cursor location
   rt->saveCursor();

   // render items
   rt->moveCursor(1,0);
   size_t itemsInThisLine = 0;
   for (size_t i = (pageNumber * itemsPerPage); i < ((pageNumber + 1) * itemsPerPage); i++) {
      // get item name or fill with " -.-" if out of range
      string thisFileName = ((i < items->size()) ? " " + items->at(i) : " -.-");

      // check if length is longer than available per column
      if (length_utf8(thisFileName) > preferredNameLength) {
         thisFileName = (substr_utf8(thisFileName, 0, (preferredNameLength / 2) - 1) + "…" + substr_utf8(thisFileName, length_utf8(thisFileName) - (preferredNameLength / 2)));
      }

      cout << ((i == selectedIndex) ? rt->getReverse() : "")
           << setw(preferredNameLength) << left
           << thisFileName
           << ((i == selectedIndex) ? rt->getResetAttributes() : "");

      itemsInThisLine++;

      if ((itemsInThisLine >= itemsPerLine) && ((i % itemsPerPage) != (itemsPerPage - 1))) {
         itemsInThisLine = 0;
         cout << endl;
      }
   }

   // restore cursor location
   rt->restoreCursor();
}

/**
 * @method pressedLeft
 * Decrements the selectedIndex with wrapping to the end.
 */
void FileBrowser::pressedLeft() {
   selectedIndex--;

   // Account for out of bounds (wrap to end)
   // IMPORTANT: size_t cannot be less than 0
   // so we have to use the same check as in pressedRight
   // but we'll set it to the last item rather than the first
   if (selectedIndex >= items->size()) {
      selectedIndex = items->size() - 1;
   }

   redrawTable();
}

/**
 * @method pressedRight
 * Increments the selectedIndex with wrapping to the start
 */
void FileBrowser::pressedRight() {
   selectedIndex++;
   
   // Account for out of bounds (wrap to start)
   if (selectedIndex >= items->size()) {
      selectedIndex = 0;
   }

   redrawTable();
}

/**
 * @method pressedUp
 * Decrements the selectedIndex such that it sits in the same column
 * but in the line above.
 * If that number is out of range it will simply stay in the present
 * location.
 */
void FileBrowser::pressedUp() {
   selectedIndex -= itemsPerLine;

   // Account for out of bounds (undo the subtraction)
   // same caveat as wih pressedLeft, size_t has no less than 0,
   // so we're just using the same comparison as in pressedDown
   if (selectedIndex >= items->size()) {
      selectedIndex += itemsPerLine;
   }

   redrawTable();
}

/**
 * @method pressedDown
 * Increments the selectedIndex such that it sits in the same column
 * but in the line below.
 * If that number is out of range it will simply stay in the present
 * location.
 */
void FileBrowser::pressedDown() {
   selectedIndex += itemsPerLine;

   // Account for out of bounds (undo the addition)
   if (selectedIndex >= items->size()) {
      selectedIndex -= itemsPerLine;
   }

   redrawTable();
}

// setIndex

/**
 * @method getIndex
 * Returns the selectedIndex
 * @returns {size_t} the selectedIndex
 */
size_t FileBrowser::getIndex() {
   return selectedIndex;
}

#endif
