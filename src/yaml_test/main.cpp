#include <iostream>
#include <string>
#include <yaml-cpp/yaml.h>

struct scene_config {
    std::string obs_scene;
    std::uint64_t timeout = 0;
};

struct app_config {
    std::string obs_url;
    std::string obs_password;
    bool debug = false;
    bool use_rtti = false;
    std::unordered_map<std::string, scene_config> scene_map;
};

template<>
struct YAML::convert<app_config> {
    static bool decode(const Node& node, app_config& rhs) {
        static auto constexpr required_fields = std::array{
            "obs_url",
            "obs_password",
            "debug",
            "use_rtti",
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
        rhs.use_rtti = node["use_rtti"].as<bool>();
        rhs.scene_map = node["scene_map"].as<std::unordered_map<std::string, scene_config>>();

        return true;
    }
};

template<>
struct YAML::convert<scene_config> {
    static bool decode(const Node& node, scene_config& rhs) {
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

void main() {
    auto node = YAML::LoadFile("live_driving.yaml");
    try {
        auto config = node.as<app_config>();

        std::cout << "OBS URL: " << config.obs_url << std::endl;
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}