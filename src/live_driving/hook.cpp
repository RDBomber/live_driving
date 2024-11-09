#include <spdlog/spdlog.h>
#include <safetyhook.hpp>

#include "live_driving/hook.hpp"
#include "live_driving/util.hpp"

void live_driving::create_hooks(const MODULEINFO& module_info, obs_client* obs_client, const std::unordered_map<std::string, std::string>& scene_map) {
    client = obs_client;
    map = scene_map;

    std::vector<std::tuple<std::string, safetyhook::MidHookFn>> patterns = {
        {
            "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 20 48 8B 41 48 45 0F",
            [](safetyhook::Context& ctx) {
                const auto scene_id = ctx.rdx;
                on_change_scene(scene_id);
            }
        },
        {
            "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 20 48 8B 41 48",
            [](safetyhook::Context& ctx) {
                const auto scene_id = ctx.rdx;
                on_change_scene(scene_id);
            }
        }
    };

    for(const auto& [pattern, callback] : patterns) {
        const auto hook_target = find_pattern(
            static_cast<std::uint8_t*>(module_info.lpBaseOfDll),
            module_info.SizeOfImage,
            pattern
        );

        if(hook_target == nullptr) {
            continue;
        }

        spdlog::info("scene change function found at: {}", static_cast<void*>(hook_target));
        on_change_scene_hook = create_mid(hook_target, callback);
        return;
    }

    spdlog::error("No scene change function found");
}

void live_driving::on_change_scene(std::uint64_t scene_id) {
    spdlog::info("Scene changed to {}", scene_id);

    if(client == nullptr) {
        return;
    }

    if(const std::string key = std::to_string(scene_id); map.contains(key)) {
        client->switch_scene(map[key]);
    }
    else if(map.contains("default")) {
        client->switch_scene(map["default"]);
    }
}
