#pragma once

#include <safetyhook.hpp>
#include <windows.h>
#include <psapi.h>

namespace live_driving {
    inline safetyhook::MidHook on_change_scene_hook;

    void create_hooks(const MODULEINFO& module_info);
    void on_change_scene(std::uint64_t scene_id);
}