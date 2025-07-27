#pragma once

#include <safetyhook.hpp>
#include <windows.h>
#include <psapi.h>

#include "live_driving/obs_client.hpp"

namespace live_driving {
    inline safetyhook::MidHook on_change_scene_hook;
    inline obs_client* client;
    inline std::unordered_map<std::string, std::string> map;

    void create_hooks(const MODULEINFO& module_info, obs_client* obs_client, const std::unordered_map<std::string, std::string>& scene_map, bool use_rtti);
    void on_change_scene(std::uint64_t scene_id, const std::string& scene_name = "");
}