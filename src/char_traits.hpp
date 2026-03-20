#pragma once

#include "details.hpp"

namespace stdlike::strings {

template <typename CharType>
class CharTraits {
 public:
  static bool eq(const CharType& k_lhs, const CharType& k_rhs) {
    return k_lhs == k_rhs;
  }

  static bool lt(const CharType& k_lhs, const CharType& k_rhs) {
    return k_lhs < k_rhs;
  };

  static int compare(const CharType* lhs, const CharType* rhs,
                     std::size_t count) {
    for (std::size_t idx = 0; idx < count; ++idx) {
      if (not eq(lhs[idx], rhs[idx])) {
        if (lt(lhs[idx], rhs[idx])) {
          return details::kLess;
        }
        return details::kGreater;
      }
    }

    return details::kEqual;
  }

  static void assign(CharType& dst, const CharType& src) { dst = src; }

  static void copy(CharType* dst, const CharType* src, std::size_t count) {
    for (std::size_t idx = 0; idx < count; ++idx) {
      assign(dst[idx], src[idx]);
    }
  }

  static std::size_t length(const CharType* str) {
    std::size_t current_length = 0;
    CharType default_value = CharType();
    while (str[current_length] != default_value) {
      ++current_length;
    }

    return current_length;
  }

  static void fill(CharType* str, const CharType& k_chr, std::size_t length) {
    for (std::size_t idx = 0; idx < length; ++idx) {
      assign(str[idx], k_chr);
    }
  }
};

}  // namespace stdlike::strings
