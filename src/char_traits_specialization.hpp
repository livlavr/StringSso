#pragma once

#include <cstring>

#include "char_traits.hpp"

namespace stdlike::strings {

template <>
inline int CharTraits<char>::compare(const char* lhs, const char* rhs,
                                     std::size_t count) {
  int res = std::memcmp(lhs, rhs, count);

  if (res < 0) {
    return details::kLess;
  }
  if (res > 0) {
    return details::kGreater;
  }

  return details::kEqual;
}

template <>
inline void CharTraits<char>::copy(char* dst, const char* src,
                                   std::size_t count) {
  std::memcpy(dst, src, count);
}

template <>
inline std::size_t CharTraits<char>::length(const char* str) {
  if (str != nullptr) {
    return std::strlen(str);
  }

  return 0;
}

template <>
inline void CharTraits<char>::fill(char* str, const char& chr,
                                   std::size_t length) {
  std::memset(str, chr, length);
}

}  // namespace stdlike::strings
