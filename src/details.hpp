#pragma once

#include <cstddef>
#include <limits>

namespace stdlike::strings {

static constexpr std::size_t kSsoSize = 23;

namespace details {
static constexpr std::size_t kInitialSize = 0;
static constexpr std::size_t kResizeUpCoef = 3;
static constexpr std::size_t kEmptyPushSize = 2;
static constexpr int kEqual = 0;
static constexpr int kGreater = 1;
static constexpr int kLess = -1;
static constexpr unsigned char kFlagMask = 0x80;     // 1000 0000
static constexpr unsigned char kSsoSizeMask = 0x7F;  // 0111 1111
static constexpr std::size_t kHeapSizeMask =
    std::numeric_limits<std::size_t>::max() >> 1;
}  // namespace details
}  // namespace stdlike::strings
