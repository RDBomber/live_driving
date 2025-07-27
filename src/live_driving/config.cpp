#include <format>

#include "live_driving/config.hpp"

live_driving::app_config live_driving::get_config(const std::filesystem::path& config_path) {
    if(!std::filesystem::exists(config_path)) {
        throw std::runtime_error(std::format("Config file '{}' not found.", config_path.string()));
    }

    return YAML::LoadFile(config_path.string()).as<app_config>();
}
