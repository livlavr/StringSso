#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>
#include <new>
#include <random>
#include <stdexcept>

#include <src/string.hpp>

namespace details {

std::size_t alloc_count = 0;
std::size_t dealloc_count = 0;

}  // namespace details

inline void ResetAllcoations() {
  details::alloc_count = 0;
  details::dealloc_count = 0;
}

inline void CheckNoAllocations() {
  EXPECT_EQ(details::alloc_count, 0);
}

inline void CheckNoDeallocations() {
  EXPECT_EQ(details::dealloc_count, 0);
}

inline void CheckSingleAllocation() {
  EXPECT_TRUE(details::alloc_count == 1);
}

inline void CheckSingleDeallocation() {
  EXPECT_TRUE(details::dealloc_count == 1);
}

inline stdlike::strings::String GenerateRandomString(std::size_t length) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, 25);

  stdlike::strings::String result;
  for (std::size_t i = 0; i < length; ++i) {
    char symbol = 'a' + distrib(gen);
    result.push_back(symbol);
  }

  return result;
}

void* operator new(std::size_t size) {
  ++details::alloc_count;
  return malloc(size);
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
  ++details::alloc_count;
  return malloc(size);
}

void* operator new[](std::size_t size) {
  ++details::alloc_count;
  return malloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
  ++details::alloc_count;
  return malloc(size);
}

void operator delete(void* data) noexcept {
  ++details::dealloc_count;
  free(data);
}

void operator delete[](void* data) noexcept {
  ++details::dealloc_count;
  free(data);
}

void operator delete(void* data, [[maybe_unused]] std::size_t size) noexcept {
  ++details::dealloc_count;
  free(data);
}

void operator delete[](void* data, [[maybe_unused]] std::size_t size) noexcept {
  ++details::dealloc_count;
  free(data);
}

struct SsoFixture : public testing::Test {
 protected:
  void SetUp() override {
    ResetAllcoations();
  }

  static inline const char* const kShortString = "Short String";
  static inline const std::size_t kShortStringSize = 12;

  static inline const char* const kShortStringPrefix = "Short";
  static inline const std::size_t kShortStringPrefixSize = 5;

  static inline const char* const kOtherShortString = "String";
  static inline const std::size_t kOtherShortStringSize = 6;

  static inline const char* const kLongString = "Very very very very very long String";
  static inline const std::size_t kLongStringSize = 36;
};

static_assert(stdlike::strings::kSsoSize < 32, "Small strings should be small!");
static_assert(stdlike::strings::kSsoSize >= 14, "Small strings should optimeze bigger strings!");
static_assert(sizeof(stdlike::strings::String) - stdlike::strings::kSsoSize < sizeof(void*) * 3, "Small strings should take place of some fields!");

TEST_F(SsoFixture, DefaultConstruction) {
  {
    stdlike::strings::String str;
    CheckNoAllocations();

    EXPECT_EQ(str.size(), 0);
    EXPECT_TRUE(str.empty());
    EXPECT_EQ(str, "");
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, ConstructionCString) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    EXPECT_EQ(str.size(), kShortStringSize);
    EXPECT_EQ(str, kShortString);
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, ConstructionCopy) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    stdlike::strings::String str_copy = str;
    CheckNoAllocations();

    EXPECT_EQ(str_copy, str);
    EXPECT_EQ(str_copy, kShortString);
    EXPECT_EQ(str_copy.size(), kShortStringSize);
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, AssignmentCopy) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    stdlike::strings::String str_copy;
    CheckNoAllocations();

    str_copy = str;
    CheckNoAllocations();

    EXPECT_EQ(str_copy, str);
    EXPECT_EQ(str_copy, kShortString);
    EXPECT_EQ(str_copy.size(), kShortStringSize);
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, AssignmentCString) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    str = kOtherShortString;
    CheckNoAllocations();

    EXPECT_EQ(str, kOtherShortString);
    EXPECT_EQ(str.size(), kOtherShortStringSize);
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, AssignmentCStringBig) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    str = kLongString;
    CheckSingleAllocation();

    EXPECT_EQ(str, kLongString);
    EXPECT_EQ(str.size(), kLongStringSize);
  }
  CheckSingleDeallocation();
}

TEST_F(SsoFixture, AssignmentSelf) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    str = str;
    CheckNoAllocations();

    EXPECT_EQ(str, kShortString);
    EXPECT_EQ(str.size(), kShortStringSize);
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, ResizeToSmaller) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    str.resize(kShortStringPrefixSize);
    CheckNoAllocations();

    EXPECT_EQ(str, kShortStringPrefix);
    EXPECT_EQ(str.size(), kShortStringPrefixSize);
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, ResizeToBigger) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    str.resize(kLongStringSize);
    CheckSingleAllocation();

    EXPECT_EQ(str.size(), kLongStringSize);

    str.resize(kShortStringSize);
    EXPECT_EQ(str, kShortString);
  }
  CheckSingleDeallocation();
}

TEST_F(SsoFixture, ReserveToSmaller) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    str.reserve(kShortStringPrefixSize);
    CheckNoAllocations();

    EXPECT_EQ(str, kShortString);
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, ReserveToBigger) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    str.reserve(kLongStringSize);
    CheckSingleAllocation();

    EXPECT_EQ(str, kShortString);
  }
  CheckSingleDeallocation();
}

TEST_F(SsoFixture, IndexAccess) {
  {
    stdlike::strings::String str(kShortString);
    CheckNoAllocations();

    EXPECT_EQ(str.size(), kShortStringSize);
    EXPECT_EQ(str, kShortString);
    for (std::size_t i = 0; i < kShortStringSize; ++i) {
      EXPECT_EQ(str[i], kShortString[i]);
      EXPECT_EQ(str.at(i), kShortString[i]);
    }
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, PushBack) {
  {
    stdlike::strings::String str(kShortStringPrefix);
    CheckNoAllocations();

    str.push_back(' ');
    str.push_back('S');
    str.push_back('t');
    str.push_back('r');
    str.push_back('i');
    str.push_back('n');
    str.push_back('g');
    CheckNoAllocations();

    EXPECT_EQ(str, kShortString);
  }
  CheckNoDeallocations();
}

TEST_F(SsoFixture, SsoLargestString) {
  {
    stdlike::strings::String str = GenerateRandomString(stdlike::strings::kSsoSize);
    CheckNoAllocations();

    str.push_back('a');
    CheckSingleAllocation();
  }
  CheckSingleDeallocation();
}
