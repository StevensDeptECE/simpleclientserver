#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
/*
        Represent an exception class that outputs a numeric error code into
        a binary stream.

        To keep things simple for now, we are just writing to text. This is
        slower, but we are also writing filenames, and it's not clear how we
        could do that with a binary file so for now this is it.

        Be sure Errcode is defined for your application
*/
enum class Errcode;
class Ex {
 private:
  const char* filename;             // the filename in which the error occurred
  const std::string_view function;  // the function name
  const uint32_t lineNum;           // the linenumber of the file
  const std::string_view name;  // a string message, if any (nu  llptr if not)
  int param;                    // an integer parameter, if any
  Errcode e;
  static const char* errNames[];

 public:
  Ex(const char filename[], uint32_t lineNum, Errcode e,
     const std::string_view name = "")
      : filename(filename), lineNum(lineNum), e(e), name(name) {}

  explicit Ex(Errcode e, const std::string_view msg = "",
              const char filename[] = __FILE__,
              uint32_t lineNum = __LINE__)
      : Ex(filename, lineNum, e, msg) {}

  friend std::ostream& operator<<(std::ostream& s, const Ex& e) {
    s << e.filename << ": " << e.lineNum << " " << errNames[int(e.e)] << '\t';
    if (!e.name.empty()) {
      s << e.name;
    }
    return s;
  }
};

class FatalEx : public Ex {
 private:
 public:
  FatalEx(const char filename[], int lineNum, Errcode e,
          const std::string& name = "")
      : Ex(filename, lineNum, e, name) {}
};

// define macros that inject source file and line number into the exception
#define Ex1(e) Ex(__FILE__, __LINE__, e)
#define FatalEx1(e) FatalEx(__FILE__, __LINE__, e)

#define Ex2(e, stringparam) Ex(__FILE__, __LINE__, e, stringparam)
#define FatalEx2(e, stringparam) FatalEx(__FILE__, __LINE__, e, stringparam)
