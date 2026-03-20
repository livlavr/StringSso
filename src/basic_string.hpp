#pragma once

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "char_traits.hpp"
#include "char_traits_specialization.hpp"
#include "details.hpp"

/*
SSO FBString:
---------------------------------------------------------------
short |        buffer 23B        | least_size 7bit | flag 1bit |
long  | data   8B  | capacity 8B | size       7bit | flag 1bit |
---------------------------------------------------------------
*/

namespace stdlike::strings {

template <typename CharType, typename CharTraitsType = CharTraits<CharType>>
class BasicString {
 public:
  BasicString() {
    set_stack_flag();
    set_sso_size(0);
  }

  BasicString(const CharType* c_str) : BasicString(c_str, cstrlen(c_str)) {};

  BasicString(std::size_t count, CharType chr) { init_string(count, chr); }

  BasicString(const BasicString& other)
      : BasicString(other.data(), other.size()) {}

  BasicString(const CharType* data, std::size_t size) {
    init_string(data, size);
  }

  ~BasicString() { destroy_data(); }

 public:
  BasicString& operator=(const BasicString& other) {
    if (this != &other) {
      BasicString copy(other);
      swap(copy);
    }

    return *this;
  }

  BasicString& operator=(const char* str) {
    if (data() != str) {
      BasicString copy(str);
      swap(copy);
    }

    return *this;
  }

  BasicString operator+(const BasicString& other) const {
    BasicString copy(*this);
    copy += other;
    return copy;
  }

  BasicString operator+(const CharType* c_str) const {
    BasicString copy(*this);
    copy += c_str;
    return copy;
  }

  BasicString operator+(CharType chr) const {
    BasicString copy(*this);
    copy += chr;
    return copy;
  }

  BasicString& operator+=(const BasicString& other) {
    append(other);

    return *this;
  }

  BasicString& operator+=(const CharType* c_str) {
    reserve(size() + cstrlen(c_str));
    CharTraitsType::copy(data() + size(), c_str, cstrlen(c_str));
    set_size(size() + cstrlen(c_str));

    return *this;
  }

  BasicString& operator+=(CharType chr) {
    push_back(chr);

    return *this;
  }

  bool operator==(const BasicString& other) const { return equal_to(other); }

  bool operator==(const CharType* other) const { return equal_to(other); }

  auto operator<=>(const BasicString& other) const {
    return (*this <=> other.data());
  }

  auto operator<=>(const CharType* other) const {
    using ReturnType = std::compare_three_way_result_t<CharType>;

    std::size_t other_size = CharTraitsType::length(other);
    std::size_t common_len = std::min(size(), other_size);
    int result = CharTraitsType::compare(data(), other, common_len);

    if (result != 0) {
      return static_cast<ReturnType>(result <=> 0);
    }

    return static_cast<ReturnType>(size() <=> other_size);
  }

  CharType& operator[](std::size_t index) { return at(index); }

  const CharType& operator[](std::size_t index) const { return at(index); }

 public:
  std::size_t size() const {
    if (is_sso()) {
      return kSsoSize - (get_flag_byte() & details::kSsoSizeMask);
    }
    return storage_.heap_layout.size & details::kHeapSizeMask;
  }

  std::size_t capacity() const {
    if (is_sso()) {
      return kSsoSize;
    }
    return storage_.heap_layout.capacity == 0 ?
               0 :
               storage_.heap_layout.capacity - 1;
  }

  bool equal_to(const BasicString& other) const {
    if (size() != other.size()) {
      return false;
    }
    return CharTraitsType::compare(data(), other.data(), size())
           == details::kEqual;
  }

  bool equal_to(const char* other) const {
    if (size() != CharTraitsType::length(other)) {
      return false;
    }
    return CharTraitsType::compare(data(), other, size()) == details::kEqual;
  }

  bool empty() const { return size() == 0; }

 public:
  void swap(BasicString& other) { std::swap(storage_, other.storage_); }

  void push_back(CharType chr) {
    if (is_sso()) {
      std::size_t sso_sz = kSsoSize - (get_flag_byte() & details::kSsoSizeMask);
      if (sso_sz < kSsoSize) {
        storage_.stack_layout.data[sso_sz] = chr;
        set_sso_size(sso_sz + 1);
        return;
      }
    }

    std::size_t current_sz = size();
    std::size_t cap = capacity();

    if (current_sz == cap) {
      resize_up();
    }

    CharType* ptr = data();
    ptr[current_sz] = chr;

    set_heap_size(current_sz + 1);
  }

  void pop_back() {
    if (not empty()) {
      remove_last_element();
    }
  }

  BasicString& append(const BasicString& other) {  // check
    reserve(size() + other.size());

    CharTraitsType::copy(data() + size(), other.data(), other.size());
    set_size(size() + other.size());
    add_null_terminator();

    return *this;
  }

  BasicString& append(CharType& chr) {
    push_back(chr);

    return *this;
  }

  BasicString& append(const CharType* ptr) {
    reserve(size() + cstrlen(ptr));

    CharTraitsType::copy(data() + size(), ptr, cstrlen(ptr));
    set_size(size() + cstrlen(ptr));
    add_null_terminator();

    return *this;
  }

  void clear() {
    set_size(0);
    add_null_terminator();
  }

  void shrink_to_fit() {
    if (size() == capacity()) {
      return;
    }
    set_ctype_string(data(), size());
  }

  void resize(std::size_t new_size, CharType chr = CharType()) {
    std::size_t old_size = size();

    if (new_size > capacity()) {
      set_ctype_string(data(), size(), get_buffer_size(new_size));
    }

    if (new_size > old_size) {
      CharTraitsType::fill(data() + old_size, chr, new_size - old_size);
    }

    set_size(new_size);
    add_null_terminator();
  }

  void reserve(std::size_t new_cap) {
    if (new_cap < get_buffer_size()) {
      return;
    }

    set_ctype_string(data(), size(), get_buffer_size(new_cap));
  }

 public:
  CharType* data() {
    return is_sso() ? storage_.stack_layout.data : storage_.heap_layout.data;
  }

  const CharType* data() const {
    return is_sso() ? storage_.stack_layout.data : storage_.heap_layout.data;
  }

  CharType& at(std::size_t idx) { return data()[idx]; }

  const CharType& at(std::size_t idx) const { return data()[idx]; }

  CharType& front() { return data()[0]; }

  const CharType& front() const { return data()[0]; }

  CharType& back() { return data()[size() - 1]; }

  const CharType& back() const { return data()[size() - 1]; }

 private:
  static CharType* allocate(const std::size_t kCapacity) {
    if (kCapacity == 0) {
      return nullptr;
    }

    return new CharType[kCapacity]();
  }

  static std::size_t cstrlen(const CharType* c_str) {
    return c_str != nullptr ? CharTraitsType::length(c_str) : 0;
  }

  void destroy_data() {
    if (not is_sso()) {
      delete[] data();
    }
  }

  void remove_last_element() {
    set_size(size() - 1);

    add_null_terminator();
  }

  void add_null_terminator() { data()[size()] = CharType(); }

  std::size_t calc_copy_size(const CharType* ptr,
                             std::size_t requested_size) const {
    if (ptr == nullptr) {
      return 0;
    }

    return std::min(requested_size, CharTraitsType::length(ptr));
  }

  std::size_t calc_new_capacity(std::size_t needed_size,
                                std::size_t hint_capacity) const {
    return std::max(get_buffer_size(needed_size), hint_capacity);
  }

  void reallocate_and_replace(std::size_t new_capacity, const CharType* src,
                              std::size_t k_copy_len, std::size_t final_size) {
    CharType* new_buffer = allocate(new_capacity);

    if (src && k_copy_len > 0) {
      CharTraitsType::copy(new_buffer, src, k_copy_len);
    }

    destroy_data();

    storage_.heap_layout.data = new_buffer;
    storage_.heap_layout.capacity = new_capacity;
    storage_.heap_layout.size = final_size;

    if (is_sso()) {
      set_heap_flag();
    }

    add_null_terminator();
  }

  void set_ctype_string(const CharType* input_data, std::size_t input_size,
                        std::size_t input_capacity = 0) {
    const std::size_t kCopyLen = calc_copy_size(input_data, input_size);
    const std::size_t kNewCapVal =
        calc_new_capacity(input_size, input_capacity);

    reallocate_and_replace(kNewCapVal, input_data, kCopyLen, input_size);
  }

  std::size_t get_buffer_size() const {
    if (is_sso()) {
      return kSsoSize;
    }
    return storage_.heap_layout.capacity;
  }

  static std::size_t get_buffer_size(std::size_t size) { return size + 1; }

  void resize_up() { set_ctype_string(data(), size(), get_resize_capacity()); }

  std::size_t get_resize_capacity() {
    return std::max(details::kEmptyPushSize,
                    get_buffer_size() * details::kResizeUpCoef);
  }

 private:
  bool is_sso(std::size_t size) { return size <= kSsoSize; }

  bool is_sso() const { return (get_flag_byte() & details::kFlagMask) == 0; }

  void init_string(const CharType* src_data, std::size_t size) {
    if (is_sso(size)) {
      init_stack_string(src_data, size);
    } else {
      init_heap_string(src_data, size);
    }
  }

  void init_string(std::size_t count, CharType chr) {
    if (is_sso(count)) {
      init_stack_string(count, chr);
    } else {
      init_heap_string(count, chr);
    }
  }

  void init_stack_string(std::size_t count, CharType chr) {
    set_stack_flag();

    set_size(count);
    CharTraitsType::fill(data(), chr, size());

    add_null_terminator();
  }

  void init_stack_string(const CharType* src_data, std::size_t size) {
    set_stack_flag();

    if (size > 0 && src_data) {
      CharTraitsType::copy(data(), src_data, size);
    }
    set_size(size);
    add_null_terminator();
  }

  void init_heap_string(std::size_t count, CharType chr) {
    set_heap_flag();

    set_size(count);
    storage_.heap_layout.capacity = get_buffer_size(size());
    storage_.heap_layout.data = allocate(get_buffer_size());

    CharTraitsType::fill(data(), chr, size());

    add_null_terminator();
  }

  void init_heap_string(const CharType* src_data, std::size_t size) {
    set_heap_flag();

    std::size_t required_capacity = get_buffer_size(size);

    set_heap_data(allocate(required_capacity));

    if (size > 0 && src_data) {
      CharTraitsType::copy(data(), src_data, size);
    }

    set_size(size);
    set_capacity(required_capacity);

    add_null_terminator();
  }

  void set_heap_flag() { get_flag_byte() |= details::kFlagMask; }

  void set_stack_flag() { get_flag_byte() &= ~details::kFlagMask; }

  unsigned char& get_flag_byte() {
    return reinterpret_cast<unsigned char*>(this)[kSsoSize];
  }

  const unsigned char& get_flag_byte() const {
    return reinterpret_cast<const unsigned char*>(this)[kSsoSize];
  }

  void increase_size() { set_size(size() + 1); }

  void decrease_size() { set_size(size() - 1); }

  void set_size(std::size_t size) {
    if (is_sso()) {
      set_sso_size(size);
    } else {
      set_heap_size(size);
    }
  }

  void set_capacity(std::size_t capacity) {
    if (not is_sso()) {
      storage_.heap_layout.capacity = capacity;
    }
  }

  void set_heap_data(CharType* data) { storage_.heap_layout.data = data; }

  void set_sso_size(std::size_t size) {
    get_flag_byte() =
        static_cast<unsigned char>((kSsoSize - size) & details::kSsoSizeMask);
  }

  void set_heap_size(std::size_t size) {
    storage_.heap_layout.size = size;
    get_flag_byte() |= details::kFlagMask;
  }

 private:
  union Storage {
    struct {
      CharType data[kSsoSize];
      /* Padding is the flag bit */
    } stack_layout;

    struct {
      CharType* data;
      std::size_t capacity;
      std::size_t size;
    } heap_layout;
  };

  Storage storage_ = {};
};

template <typename CharType>
inline BasicString<CharType> operator+(const CharType* lhs,
                                       const BasicString<CharType>& rhs) {
  BasicString result(lhs);
  result.append(rhs);
  return result;
}

template <typename CharType>
inline BasicString<CharType> operator+(CharType lhs,
                                       const BasicString<CharType>& rhs) {
  BasicString<CharType> result;
  result.push_back(lhs);
  result.append(rhs);
  return result;
}

using String = BasicString<char>;

}  // namespace stdlike::strings
