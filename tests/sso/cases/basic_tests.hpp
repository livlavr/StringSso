#include <gtest/gtest.h>

#include <compare>
#include <sstream>
#include <src/string.hpp>

#include <random>
#include <chrono>
#include <string>


using namespace std::chrono_literals;

template <typename F>
void MeasureTime(F&& func, std::chrono::milliseconds time_limit) {
  auto start = std::chrono::high_resolution_clock::now();

  std::invoke(func);

  auto end = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  EXPECT_LT(duration, time_limit);
}

TEST(Constructors, Default) {
  stdlike::strings::String s;
  ASSERT_EQ(s.size(), 0UL);
  ASSERT_TRUE(s.empty());
}

TEST(Constructors, FromNumberAndLetter) {
  const size_t size = 100;
  stdlike::strings::String s(size, 'a');
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(s.at(i), 'a');
  }
}

TEST(Constructors, FromCString) {
  const char* cstr = "aaa";
  stdlike::strings::String s(cstr);
  for (size_t i = 0; i < 3; ++i) {
    ASSERT_EQ(s.at(i), 'a');
  }
}

TEST(Constructors, CopyConstructor) {
  const size_t size = 100;
  stdlike::strings::String s(size, 'a');
  stdlike::strings::String s1(s);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(s.at(i), s1.at(i));
  }
}

TEST(Assignment, Simple) {
  const size_t size = 100;
  stdlike::strings::String s(size, 'a');
  stdlike::strings::String s1 = s;
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(s.at(i), s1.at(i));
  }
}

TEST(Assignment, Self) {
  const size_t size = 100;
  stdlike::strings::String s(size, 'a');
  stdlike::strings::String& sr = s;  // to avoid self assignment compile error
  s = sr;
  for (size_t i = 0; i < size; ++i) {
    ASSERT_EQ(s.at(i), s.at(i));
  }
}

TEST(Clear, Clear) {
  const size_t size = 100;
  stdlike::strings::String s(size, 'a');
  s.clear();
  ASSERT_EQ(s.size(), 0UL);
  ASSERT_TRUE(s.empty());
}

TEST(PushBack, ToExisting) {
  stdlike::strings::String s = "aba";
  s.push_back('c');
  ASSERT_EQ(s.size(), 4UL);
}

TEST(PushBack, ToDefault) {
  stdlike::strings::String s;
  s.push_back('c');
  ASSERT_EQ(s.size(), 1UL);
}

TEST(PopBack, FromNonEmpty) {
  stdlike::strings::String s = "ab";
  s.pop_back();
  ASSERT_EQ(s.size(), 1UL);
  s.pop_back();
  ASSERT_EQ(s.size(), 0UL);
}

TEST(Resize, ToLess) {
  stdlike::strings::String s = "abacaba";
  s.resize(3);
  ASSERT_EQ(s.size(), 3UL);
  ASSERT_EQ(s.at(2), 'a');
}

TEST(Resize, ToBigger) {
  stdlike::strings::String s = "aba";
  s.resize(10);
  ASSERT_EQ(s.size(), 10UL);
}

TEST(Resize, ToZero) {
  stdlike::strings::String s = "aba";
  s.resize(0);
  EXPECT_TRUE(s.empty());
}

TEST(Resize, ToBiggerWithVal) {
  stdlike::strings::String s = "aba";
  s.resize(10, 'a');
  ASSERT_EQ(s.size(), 10UL);
  ASSERT_EQ(s.at(4), 'a');
  ASSERT_EQ(s.at(0), 'a');
  ASSERT_EQ(s.at(9), 'a');
}

TEST(Reserve, ToLess) {
  stdlike::strings::String s = "abacabababacabaabacabaabaabacabaaba";
  s.reserve(1);
  ASSERT_EQ(s.size(), 35UL);
  s.shrink_to_fit();
  s.shrink_to_fit();
  ASSERT_LE(s.capacity(), s.size() * 2);
}

TEST(Reserve, ToBigger) {
  stdlike::strings::String s = "abacabababacabaabacabaabaabacabaaba";
  s.reserve(100);
  ASSERT_EQ(s.size(), 35UL);
  ASSERT_GE(s.capacity(), 100UL);
  s.shrink_to_fit();
  ASSERT_LE(s.capacity(), s.size() * 2);
}

TEST(ShrinkToFit, ShrinkToFit) {
  stdlike::strings::String s = "abacabababacabaabacabaabaabacabaaba";
  s.shrink_to_fit();
  ASSERT_LE(s.capacity(), 70UL);
  ASSERT_GE(s.capacity(), 35UL);
  ASSERT_EQ(s.size(), 35UL);
}

TEST(Swap, Two) {
  stdlike::strings::String s = "aboba";
  stdlike::strings::String t = "abiba";
  s.swap(t);
  ASSERT_EQ(s.size(), t.size());
  ASSERT_EQ(s.at(2), 'i');
  ASSERT_EQ(t.at(2), 'o');
}

TEST(Swap, One) {
  stdlike::strings::String s = "aboba";
  s.swap(s);
  ASSERT_EQ(s.size(), s.size());
  ASSERT_EQ(s.at(2), 'o');
}

TEST(At, NonConst) {
  stdlike::strings::String s = "aboba";
  const bool is_ref = std::is_reference_v<decltype(s.at(0))>;
  EXPECT_TRUE(is_ref);
  s.at(0) = 'c';
  const bool is_char = std::is_same_v<std::remove_reference_t<decltype(s.at(0))>, char>;
  EXPECT_TRUE(is_char);
}

TEST(At, Const) {
  const stdlike::strings::String s = "aboba";
  const bool is_const_ref =
      std::is_reference_v<decltype(s.at(0))> && std::is_const_v<std::remove_reference_t<decltype(s.at(0))>>;
  const bool is_not_ref = !std::is_reference_v<decltype(s.at(0))>;
  EXPECT_TRUE(is_const_ref || is_not_ref);
}

TEST(IndexAccessOperator, NonConst) {
  stdlike::strings::String s = "aboba";
  const bool is_ref = std::is_reference_v<decltype(s[0])>;
  EXPECT_TRUE(is_ref);
  s[0] = 'c';
  const bool is_char = std::is_same_v<std::remove_reference_t<decltype(s[0])>, char>;
  EXPECT_TRUE(is_char);
}

TEST(IndexAccessOperator, Const) {
  const stdlike::strings::String s = "aboba";
  const bool is_const_ref =
      std::is_reference_v<decltype(s[0])> && std::is_const_v<std::remove_reference_t<decltype(s[0])>>;
  const bool is_not_ref = !std::is_reference_v<decltype(s[0])>;
  EXPECT_TRUE(is_const_ref || is_not_ref);
}

TEST(FrontBack, Const) {
  const stdlike::strings::String s = "abob";
  bool is_const_ref =
      std::is_reference_v<decltype(s.front())> && std::is_const_v<std::remove_reference_t<decltype(s.front())>>;
  bool is_not_ref = !std::is_reference_v<decltype(s.front())>;
  EXPECT_TRUE(is_const_ref || is_not_ref);
  EXPECT_EQ(s.front(), 'a');
  is_const_ref =
      std::is_reference_v<decltype(s.back())> && std::is_const_v<std::remove_reference_t<decltype(s.back())>>;
  is_not_ref = !std::is_reference_v<decltype(s.back())>;
  EXPECT_TRUE(is_const_ref || is_not_ref);
  EXPECT_EQ(s.back(), 'b');
}

TEST(FrontBack, NonConst) {
  stdlike::strings::String s = "abob";
  bool is_ref = std::is_reference_v<decltype(s.front())>;
  bool is_char = std::is_same_v<std::remove_reference_t<decltype(s.front())>, char>;
  EXPECT_TRUE(is_ref && is_char);
  s.front() = 'c';
  EXPECT_EQ(s.at(0), 'c');

  is_ref = std::is_reference_v<decltype(s.back())>;
  is_char = std::is_same_v<std::remove_reference_t<decltype(s.back())>, char>;
  EXPECT_TRUE(is_ref && is_char);
  s.back() = 'c';
  EXPECT_EQ(s.at(s.size() - 1), 'c');
}

TEST(Data, NonConst) {
  stdlike::strings::String s = "abob";
  bool are_same = std::is_same_v<decltype(s.data()), char*>;
  EXPECT_TRUE(are_same);
}

TEST(Data, Const) {
  const stdlike::strings::String s = "abob";
  bool are_same = std::is_same_v<decltype(s.data()), const char*>;
  EXPECT_TRUE(are_same);
}

TEST(SizeCapacity, IsUnsigned) {
  stdlike::strings::String s;
  EXPECT_TRUE(std::is_unsigned_v<decltype(s.size())>);
  EXPECT_TRUE(std::is_unsigned_v<decltype(s.capacity())>);
}

TEST(Compare, Equal) {
  stdlike::strings::String s1 = "aboba";
  stdlike::strings::String s2 = "aboba";
  ASSERT_TRUE(s1 == s2);
  ASSERT_FALSE(s1 != s2);
  ASSERT_FALSE(s1 < s2);
  ASSERT_TRUE(s1 <= s2);
  ASSERT_FALSE(s1 > s2);
  ASSERT_TRUE(s1 >= s2);
  ASSERT_TRUE((s1 <=> s2) == 0);
  ASSERT_TRUE((s1 <=> s2) == std::strong_ordering::equal);
}

TEST(Compare, EqualCString) {
  stdlike::strings::String s1 = "aboba";
  const char* s2 = "aboba";
  ASSERT_TRUE(s1 == "aboba");
  ASSERT_FALSE(s1 != s2);
  ASSERT_FALSE(s1 < "aboba");
  ASSERT_TRUE(s1 <= s2);
  ASSERT_FALSE(s1 > s2);
  ASSERT_TRUE(s1 >= s2);
  ASSERT_TRUE((s1 <=> s2) == 0);
  ASSERT_TRUE((s1 <=> s2) == std::strong_ordering::equal);
  ASSERT_TRUE((s1 <=> "aboba") == std::strong_ordering::equal);
}

TEST(Compare, NotEqual) {
  stdlike::strings::String s1 = "aboba";
  stdlike::strings::String s2 = "abiba";
  ASSERT_FALSE(s1 == s2);
  ASSERT_TRUE(s1 != s2);
  ASSERT_FALSE(s1 < s2);
  ASSERT_FALSE(s1 <= s2);
  ASSERT_TRUE(s1 > s2);
  ASSERT_TRUE(s1 >= s2);
  ASSERT_TRUE((s1 <=> s2) > 0);
  ASSERT_TRUE((s1 <=> s2) == std::strong_ordering::greater);
}

TEST(Compare, NotEqualCString) {
  stdlike::strings::String s1 = "aboba";
  const char* s2 = "abiba";
  ASSERT_FALSE(s1 == s2);
  ASSERT_TRUE(s1 != s2);
  ASSERT_FALSE(s1 < s2);
  ASSERT_FALSE(s1 <= s2);
  ASSERT_TRUE(s1 > s2);
  ASSERT_TRUE(s1 >= s2);
  ASSERT_TRUE((s1 <=> s2) > 0);
  ASSERT_TRUE((s1 <=> s2) == std::strong_ordering::greater);
}

TEST(Compare, NotEqualDiffLen) {
  stdlike::strings::String s1 = "aboba";
  const char* s2 = "abobaba";
  const char* s3 = "abob";
  ASSERT_TRUE(s1 != s2);
  ASSERT_TRUE(s1 != s3);
  ASSERT_TRUE(s1 < s2);
  ASSERT_TRUE(s1 > s3);
  ASSERT_TRUE((s1 <=> s2) == std::strong_ordering::less);
  ASSERT_TRUE((s1 <=> s3) == std::strong_ordering::greater);
}

TEST(Compare, SecondArgument) {
  const char* s1 = "aboba";
  stdlike::strings::String s2 = "aboba";
  ASSERT_TRUE(s1 == s2);
  ASSERT_FALSE(s1 != s2);
  ASSERT_FALSE(s1 < s2);
  ASSERT_TRUE(s1 <= s2);
  ASSERT_FALSE(s1 > s2);
  ASSERT_TRUE(s1 >= s2);
  ASSERT_TRUE((s1 <=> s2) == 0);
  ASSERT_TRUE((s1 <=> s2) == std::strong_ordering::equal);
}

TEST(Append, Easy) {
  stdlike::strings::String s = "aboba";
  stdlike::strings::String t = "biba";
  s.append(t);
  EXPECT_TRUE(s.equal_to("abobabiba"));
  ASSERT_EQ(s.size(), 9UL);
}

TEST(Append, OperatorPlusAssign_Simple) {
  stdlike::strings::String s = "aboba";
  stdlike::strings::String t = "biba";
  s += t;
  EXPECT_TRUE(s == "abobabiba");
  EXPECT_EQ(s.size(), 9UL);
}

TEST(Append, OperatorPlusAssign_CString) {
  stdlike::strings::String s = "aboba";
  s += "biba";
  EXPECT_TRUE(s == "abobabiba");
  EXPECT_EQ(s.size(), 9UL);
}


TEST(Append, OperatorPlusAssign_Char) {
  stdlike::strings::String s = "abob";
  s += 'a';
  EXPECT_TRUE(s == "aboba");
}

TEST(Append, OperatorPlusAssign_Self) {
  stdlike::strings::String s = "aboba";
  s += s;
  EXPECT_TRUE(s == "abobaaboba");
  EXPECT_EQ(s.size(), 10UL);
}

TEST(Append, OperatorPlusAssign_Chain) {
  stdlike::strings::String s = "ab";
  stdlike::strings::String t = "ob";
  stdlike::strings::String r = "a";
  s += t += r;
  EXPECT_TRUE(s == "aboba");
  EXPECT_TRUE(t == "oba");
  EXPECT_TRUE(r == "a");
}

TEST(Append, OperatorPlus_Simple) {
  stdlike::strings::String s = "aboba";
  stdlike::strings::String t = "biba";
  EXPECT_TRUE(s + t == "abobabiba");
  EXPECT_TRUE(s == "aboba");
  EXPECT_TRUE(t == "biba");
}

TEST(Append, OperatorPlus_CString) {
  stdlike::strings::String s = "aboba";
  EXPECT_TRUE(s + "biba" == "abobabiba");
  EXPECT_TRUE(s == "aboba");
}

TEST(Append, OperatorPlus_Char) {
  stdlike::strings::String s = "abob";
  EXPECT_TRUE(s + 'a' == "aboba");
  EXPECT_TRUE(s == "abob");
}

TEST(Append, OperatorPlus_Self) {
  stdlike::strings::String s = "aboba";
  EXPECT_TRUE(s + s == "abobaaboba");
  EXPECT_TRUE(s == "aboba");
}

TEST(Append, OperatorPlus_Chain) {
  stdlike::strings::String s = "ab";
  stdlike::strings::String t = "ob";
  stdlike::strings::String r = "a";
  EXPECT_TRUE(s + t + r == "aboba");
}

TEST(Append, OperatorPlus_ChainWithChar) {
  stdlike::strings::String s = "ab";
  stdlike::strings::String r = "ba";
  EXPECT_TRUE(s + 'o' + r == "aboba");
}

TEST(Append, OperatorPlus_CStringFirst) {
  stdlike::strings::String s = "biba";
  EXPECT_TRUE("aboba" + s == "abobabiba");
}

TEST(Append, OperatorPlus_CharFirst) {
  stdlike::strings::String s = "boba";
  EXPECT_TRUE('a' + s == "aboba");
}

TEST(Suffix, Simple) {
  using namespace stdlike::strings::literals;
  EXPECT_TRUE((std::is_same_v<decltype("aboba"_s), stdlike::strings::String>));
  stdlike::strings::String s("aboba");
  EXPECT_TRUE("aboba"_s == s);
}

TEST(Stream, Input) {
  std::stringstream stream("Hello World 10");
  stdlike::strings::String s;
  stream >> s;
  EXPECT_EQ(s, "Hello");
  int x = 0;
  stream >> s >> x;
  EXPECT_EQ(s, "World");
  EXPECT_EQ(x, 10);
}

TEST(Stream, Output) {
  std::stringstream stream;
  stdlike::strings::String s("Hello World");

  stream << s;
  std::string result;
  stream >> result;
  EXPECT_EQ(result, "Hello");

  stream >> s;
  EXPECT_EQ(s, "World");
}

#ifndef DEBUG

TEST(Stress, Concat_PlusAssignmentFibonacciStyle) {
  const auto kTimeLimit = 600ms;
  auto test = [](){
    stdlike::strings::String s = "(";
    stdlike::strings::String t = ")";
    std::string s_s = "(";
    std::string t_s = ")";
    const size_t num_iterations = 18;
    for (size_t i = 0; i < num_iterations; ++i) {
      s += t;
      t += s;
      s_s += t_s;
      t_s += s_s;
    }
    EXPECT_TRUE(t == t_s.data());
    EXPECT_TRUE(s == s_s.data());
  };

  MeasureTime(test, kTimeLimit);
}

TEST(Stress, Append_Random) {
  const auto kTimeLimit = 10ms;

  auto test = [](){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 25);
    std::string expected;
    stdlike::strings::String s;
    const size_t kNumIterations = 1 << 10;
    for (size_t i = 0; i < kNumIterations; ++i) {
      stdlike::strings::String to_add = "a";
      to_add.at(0) += distrib(gen);
      expected.append(to_add.data());
      s.append(to_add);
    }
    EXPECT_TRUE(s.equal_to(expected.data()));
  };

  MeasureTime(test, kTimeLimit);
}

TEST(Stress, Append_Fibonacci) {
  const auto kTimeLimit = 600ms;

  auto test = [](){
    stdlike::strings::String s = "(";
    stdlike::strings::String t = ")";
    std::string s_s = "(";
    std::string t_s = ")";
    const size_t kNumIterations = 18;
    for (size_t i = 0; i < kNumIterations; ++i) {
      s.append(t);
      t.append(s);
      s_s.append(t_s);
      t_s.append(s_s);
    }
    EXPECT_TRUE(t.equal_to(t_s.data()));
    EXPECT_TRUE(s.equal_to(s_s.data()));
  };

  MeasureTime(test, kTimeLimit);
}

TEST(Stress, PushBack_Random) {
  const auto kTimeLimit = 100ms;
  auto test = []() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 25);
    std::string expected;
    stdlike::strings::String s;
    const size_t kNumIterations = 1 << 20;
    for (size_t i = 0; i < kNumIterations; ++i) {
      char to_push = 'a';
      to_push += distrib(gen);
      expected.push_back(to_push);
      s.push_back(to_push);
    }
    EXPECT_TRUE(s.equal_to(expected.data()));
  };
  MeasureTime(test, kTimeLimit);
}

#endif

