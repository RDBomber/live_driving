#include <spdlog/spdlog.h>
#include <safetyhook.hpp>
#include <rttidata.h>

#include "live_driving/hook.hpp"
#include "live_driving/util.hpp"

std::string get_class_name(const MODULEINFO& module_info, const std::uintptr_t base) {
    const auto vft = *reinterpret_cast<std::uint8_t**>(base);
    const auto loc = reinterpret_cast<_RTTICompleteObjectLocator*>(*reinterpret_cast<std::uintptr_t*>(vft - sizeof(void*)));
    const auto desc = reinterpret_cast<TypeDescriptor*>(static_cast<std::uint8_t*>(module_info.lpBaseOfDll) + loc->pTypeDescriptor);
    return std::string(desc->name + 4, std::strlen(desc->name) - 6);
}

void live_driving::create_hooks(const MODULEINFO& module_info, obs_client* obs_client, const std::unordered_map<std::string, std::string>& scene_map, bool use_rtti) {
    client = obs_client;
    map = scene_map;
    static const auto game_module = module_info;

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

    if (use_rtti) {
        patterns.insert(patterns.begin(), {
            "89 05 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8B D7",
            [](safetyhook::Context& ctx) {
                const auto scene_id = ctx.rcx;
                const auto scene_name = get_class_name(game_module, ctx.rbx);
                on_change_scene(scene_id, scene_name);
            }
        });
    }

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

void live_driving::on_change_scene(std::uint64_t scene_id, const std::string& scene_name) {
    if (scene_name.empty()) {
        spdlog::info("Scene changed to {}", scene_id);
    } else {
        spdlog::info("Scene changed to {} (id: {})", scene_name, scene_id);
    }

    if(client == nullptr) {
        return;
    }

    if(const std::string key = std::to_string(scene_id); map.contains(key)) {
        client->switch_scene(map[key]);
    }
    else if (!scene_name.empty() && map.contains(scene_name)) {
        client->switch_scene(map[scene_name]);
    }
    else if(map.contains("default")) {
        client->switch_scene(map["default"]);
    }
}
