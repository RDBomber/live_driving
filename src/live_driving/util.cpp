#include <vector>

#include "live_driving/util.hpp"

std::uint8_t* live_driving::find_pattern(std::uint8_t* memory, const std::uint64_t size, const std::string& pattern) {
    static auto compare_memory = [](const uint8_t* data, const std::vector<int>& pattern_bytes) {
        const auto pattern_size = pattern_bytes.size();
        for(auto i = 0; i < pattern_size; i++) {
            if(pattern_bytes[i] != -1 && data[i] != static_cast<uint8_t>(pattern_bytes[i])) {
                return false;
            }
        }

        return true;
    };

    std::vector<int> pattern_bytes;
    const auto pattern_size = pattern.size();
    size_t i = 0;
    while(i < pattern_size) {
        if(pattern[i] == '?') {
            pattern_bytes.push_back(-1);
        }
        else {
            pattern_bytes.push_back(std::stoi(pattern.substr(i, 2), nullptr, 16));
        }

        i += 3;
    }

    const auto pattern_bytes_size = pattern_bytes.size();
    for(i = 0; i < size - pattern_bytes_size; i++) {
        if(compare_memory(memory + i, pattern_bytes)) {
            return memory + i;
        }
    }

    return nullptr;
}
