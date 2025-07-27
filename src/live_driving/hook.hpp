#pragma once

#include <safetyhook.hpp>
#include <windows.h>
#include <psapi.h>

#include "live_driving/obs_client.hpp"
#include "live_driving/config.hpp"

namespace live_driving {
    inline safetyhook::MidHook on_change_scene_hook;
    inline obs_client* client;
    inline std::unordered_map<std::string, scene_config> map;

    void create_hooks(const MODULEINFO& module_info, app_config& config);
    void on_change_scene(std::uint64_t scene_id, const std::string& scene_name = "");
}