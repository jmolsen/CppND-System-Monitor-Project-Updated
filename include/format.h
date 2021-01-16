#ifndef FORMAT_H
#define FORMAT_H

#include <string>

namespace Format {
std::string ElapsedTime(long elapsedSeconds);
std::string LeftPad(int widthWithPad, char padChar, long value);
std::string ZeroPad(long value);
};  // namespace Format

#endif