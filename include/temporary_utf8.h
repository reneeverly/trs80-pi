#ifndef TEMPORARY_UTF8_H
#define TEMPORARY_UTF8_H

#include <string>

using namespace std;

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
   for (q=0, i=0, ix=str.length(); i < ix; i++, q++) {
      c = (unsigned char) str[i];
      if      (c>=0   && c<=127) i+=0;
      else if ((c & 0xE0) == 0xC0) i+=1;
      else if ((c & 0xF0) == 0xE0) i+=2;
      else if ((c & 0xF8) == 0xF0) i+=3;
      //else if (($c & 0xFC) == 0xF8) i+=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
      //else if (($c & 0xFE) == 0xFC) i+=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
      else return -1;//invalid utf8
   }
   return q;
}

/**
 * @function substr_utf8
 * https://stackoverflow.com/questions/30995246/substring-of-a-stdstring-in-utf-8-c11?noredirect=1&lq=1
 */
string substr_utf8(const string& str, size_t start, size_t leng)
{
    if (leng==0) { return ""; }
    size_t c, i, ix, q, min=string::npos, max=string::npos;
    for (q=0, i=0, ix=str.length(); i < ix; i++, q++)
    {
        if (q==start){ min=i; }
        if (q<=start+leng || leng==string::npos){ max=i; }

        c = (unsigned char) str[i];
        if      (
                 //c>=0   &&
                 c<=127) i+=0;
        else if ((c & 0xE0) == 0xC0) i+=1;
        else if ((c & 0xF0) == 0xE0) i+=2;
        else if ((c & 0xF8) == 0xF0) i+=3;
        //else if (($c & 0xFC) == 0xF8) i+=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
        //else if (($c & 0xFE) == 0xFC) i+=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
        else return "";//invalid utf8
    }
    if (q<=start+leng || leng==string::npos){ max=i; }
    if (min==string::npos || max==string::npos) { return ""; }
    return str.substr(min, max - min);
}

/**
 * @function substr_utf8
 * For when only a start index is provided
 */
string substr_utf8(const string& str, size_t start) {
   return substr_utf8(str, start, length_utf8(str) - start);
}

#endif
