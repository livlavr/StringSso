#include <gtest/gtest.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O0")
#endif

#include "cases/basic_tests.hpp"
#include "cases/sso.hpp"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}