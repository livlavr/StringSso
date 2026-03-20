#pragma once

#include <iostream>

#include "basic_string.hpp"

namespace stdlike::strings {
inline std::ostream& operator<<(std::ostream& stream, const String& str) {
  stream << str.data();
  return stream;
}

inline std::istream& operator>>(std::istream& stream, String& str) {
  if (not str.empty()) {
    str.clear();
  }

  int chr = 0;

  do {
    chr = stream.get();
  } while (chr != EOF && std::isspace(chr) != 0);

  if (chr == EOF) {
    stream.setstate(std::ios::failbit);
    return stream;
  }

  do {
    str.push_back(chr);
    chr = stream.get();
  } while (chr != EOF && std::isspace(chr) == 0);

  return stream;
}

namespace literals {
inline String operator""_s(const char* c_str, std::size_t size) {
  return String(c_str, size);
}
}  // namespace literals

}  // namespace stdlike::strings
