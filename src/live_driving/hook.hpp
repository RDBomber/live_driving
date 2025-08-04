#pragma once

#include <safetyhook.hpp>
#include <windows.h>
#include <variant>

#include "live_driving/obs_client.hpp"
#include "live_driving/config.hpp"
#include "live_driving/game.hpp"

namespace live_driving {
    using callback_fn = std::variant<
        void (*)(safetyhook::Context32&),
        void (*)(safetyhook::Context64&)
    >;

    struct hook {
        const char* pattern;
        const char* description;
        game_group group;
        callback_fn callback;
    };

    inline safetyhook::MidHook on_change_scene_hook;
    inline obs_client* client;
    inline app_config config;
    inline game_info game;

    void create_hooks(const game_info& game_data, app_config& cfg);
    void on_change_scene(const std::string& scene_name);
    std::string get_class_name(std::uintptr_t address);
    auto get_hooks();
    void handle_scene_actions(const std::vector<scene_action>& actions);
    std::string process_scene_name(const std::string& scene_name);
}