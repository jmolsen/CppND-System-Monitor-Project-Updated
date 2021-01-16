#include <string>
#include <iomanip>
#include <sstream>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: d days, HH:MM:SS
string Format::ElapsedTime(long elapsedSeconds) {
  string uptimeStr;
  int days = elapsedSeconds / 24 / 60 / 60;
  int hours = (elapsedSeconds / 60 / 60) % 24;
  int minutes = (elapsedSeconds / 60) % 60;
  int seconds = elapsedSeconds % 60;
  if (days > 0) {
    uptimeStr += std::to_string(days) + " days, ";
  }
  uptimeStr += ZeroPad(hours) + ":" + ZeroPad(minutes) + ":" + ZeroPad(seconds);
  return uptimeStr;
}

string Format::LeftPad(int widthWithPad, char padChar, long value) {
  std::ostringstream padded;
  padded << std::setw(widthWithPad) << std::setfill(padChar) << value;
  return padded.str();
}

string Format::ZeroPad(long value) {
  return LeftPad(2, '0', value);
}