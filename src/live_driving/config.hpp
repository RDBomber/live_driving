#pragma once

#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <filesystem>

namespace live_driving {
    struct scene_config {
        std::string obs_scene;
        std::uint64_t timeout = 0;
    };

    struct scene_action {
        std::string action;
        std::string param;
        std::uint64_t timeout = 0;
    };

    struct app_config {
        std::string obs_url;
        std::string obs_password;
        bool debug = false;
        std::unordered_map<std::string, std::vector<scene_action>> scene_map;
    };

    app_config get_config(const std::filesystem::path& config_path);
}

template<>
struct YAML::convert<live_driving::app_config> {
    static bool decode(const Node& node, live_driving::app_config& rhs) {
        static auto constexpr required_fields = std::array{
            "obs_url",
            "obs_password",
            "debug",
            "scene_map",
        };

        if (!node.IsMap()) {
            return false;
        }

        for(const auto& field : required_fields) {
            if (!node[field]) {
                throw std::runtime_error("Missing required field: " + std::string(field));
            }
        }

        rhs.obs_url = node["obs_url"].as<std::string>();
        rhs.obs_password = node["obs_password"].as<std::string>();
        rhs.debug = node["debug"].as<bool>();
        rhs.scene_map = node["scene_map"].as<std::unordered_map<std::string, std::vector<live_driving::scene_action>>>();

        return true;
    }
};

template<>
struct YAML::convert<live_driving::scene_config> {
    static bool decode(const Node& node, live_driving::scene_config& rhs) {
        static auto constexpr required_fields = std::array{
            "obs_scene",
            "timeout",
        };

        if (!node.IsMap()) {
            return false;
        }

        for(const auto& field : required_fields) {
            if (!node[field]) {
                throw std::runtime_error("Missing required field in scene map: " + std::string(field));
            }
        }

        rhs.obs_scene = node["obs_scene"].as<std::string>();
        rhs.timeout = node["timeout"].as<std::uint64_t>();

        return true;
    }
};

template<>
struct YAML::convert<live_driving::scene_action> {
    static bool decode(const Node& node, live_driving::scene_action& rhs) {
        static auto constexpr required_fields = std::array{
            "action",
            "timeout",
        };

        if (!node.IsMap()) {
            return false;
        }

        for(const auto& field : required_fields) {
            if (!node[field]) {
                throw std::runtime_error("Missing required field in scene map: " + std::string(field));
            }
        }

        rhs.action = node["action"].as<std::string>();
        rhs.param = node["param"].IsDefined() ? node["param"].as<std::string>() : "";
        rhs.timeout = node["timeout"].as<std::uint64_t>();

        return true;
    }
};