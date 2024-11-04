#pragma once

#include <cstdint>
#include <string>

namespace live_driving {
    std::uint8_t* find_pattern(std::uint8_t* memory, std::uint64_t size, const std::string& pattern);
}