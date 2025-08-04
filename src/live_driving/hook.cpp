#include <array>
#include <spdlog/spdlog.h>
#include <safetyhook.hpp>
#include <rttidata.h>

#include "live_driving/hook.hpp"
#include "live_driving/util.hpp"

auto live_driving::get_hooks() {
    return std::array {
        hook {
            "89 05 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8B D7",
            "CBaseScene Arcade IIDX RTTI hook",
            game_group::CBaseScene,
            [](safetyhook::Context64& ctx) {
                const auto scene_name = get_class_name(ctx.rbx);
                on_change_scene(scene_name);
            }
        },
        hook {
            "48 8D 0D ?? ?? ?? ?? 33 F6 C7 44",
            "CBaseScene Arcade SDVX RTTI hook",
            game_group::CBaseScene,
            [](safetyhook::Context64& ctx) {
                const auto scene_name = get_class_name(ctx.rsi);
                on_change_scene(scene_name);
            }
        },
        hook {
            "48 8D 0D ?? ?? ?? ?? 33 ED C7 44",
            "CBaseScene Konasute SDVX RTTI hook",
            game_group::CBaseScene,
            [](safetyhook::Context64& ctx) {
                const auto scene_name = get_class_name(ctx.rsi);
                on_change_scene(scene_name);
            }
        },
        hook {
            "89 05 ?? ?? ?? ?? 48 8B CB 48 8B 03 FF 50",
            "CBaseScene INFINITAS RTTI hook",
            game_group::CBaseScene,
            [](safetyhook::Context64& ctx) {
                const auto scene_name = get_class_name(ctx.rbx);
                on_change_scene(scene_name);
            }
        },
        hook {
            "FF D0 81 4E",
            "sequence RTTI hook",
            game_group::sequence,
            [](safetyhook::Context32& ctx) {
                const auto scene_name = get_class_name(ctx.ecx);
                on_change_scene(scene_name);
            }
        },
        hook {
            "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 20 48 8B 41 48 45 0F",
            "CBaseScene Arcade ID hook",
            game_group::CBaseScene,
            [](safetyhook::Context64& ctx) {
                const auto scene_id = ctx.rdx;
                on_change_scene(std::to_string(scene_id));
            }
        },
        hook {
            "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 20 48 8B 41 48",
            "CBaseScene INFINITAS ID hook",
            game_group::CBaseScene,
            [](safetyhook::Context64& ctx) {
                const auto scene_id = ctx.rdx;
                on_change_scene(std::to_string(scene_id));
            }
        }
    };
}

std::string live_driving::get_class_name(const std::uintptr_t base) {
    const auto vft = *reinterpret_cast<std::uint8_t**>(base);
    const auto loc = reinterpret_cast<_RTTICompleteObjectLocator*>(*reinterpret_cast<std::uintptr_t*>(vft - sizeof(void*)));
#if defined(_WIN64)
    const auto desc = reinterpret_cast<TypeDescriptor*>(static_cast<std::uint8_t*>(game.module_info.lpBaseOfDll) + loc->pTypeDescriptor);
#else
    const auto desc = reinterpret_cast<TypeDescriptor*>(loc->pTypeDescriptor);
#endif
    return std::string(desc->name + 4, std::strlen(desc->name) - 6);
}

void live_driving::create_hooks(const game_info& game_data, app_config& cfg) {
    game = game_data;
    config = cfg;

    for(const auto& hook : get_hooks()) {
        if(hook.group != game.group) {
            continue;
        }

        const auto hook_target = find_pattern(
            static_cast<std::uint8_t*>(game.module_info.lpBaseOfDll),
            game.module_info.SizeOfImage,
            hook.pattern
        );

        if(hook_target == nullptr) {
            continue;
        }

        spdlog::info("Pattern matched for hook: {} at {}", hook.description, static_cast<void*>(hook_target));

        on_change_scene_hook = safetyhook::create_mid(hook_target, std::get<safetyhook::MidHookFn>(hook.callback));

        if(!config.obs_url.empty() && !config.obs_password.empty()) {
            client = new obs_client(config.obs_url, config.obs_password);

            std::thread([] {
                client->listen();
            }).join();
        }

        return;
    }

    spdlog::error("No scene change function found");
}

void live_driving::on_change_scene(const std::string& scene_name) {
    if (scene_name.find("Actor") != std::string::npos) {
        return;
    }

    auto processed_name = process_scene_name(scene_name);

    spdlog::info("Scene changed to {}", processed_name);

    if(client == nullptr) {
        return;
    }

    if (!scene_name.empty() && config.scene_map.contains(processed_name)) {
        handle_scene_actions(config.scene_map[processed_name]);
    }
    else if(config.scene_map.contains("default")) {
        handle_scene_actions(config.scene_map["default"]);
    }
}

void live_driving::handle_scene_actions(const std::vector<scene_action>& actions) {
    for(const auto& action : actions) {
        if(action.action == "change_scene") {
            client->switch_scene(action.param, action.timeout);
        }
        else if(action.action == "start_recording") {
            client->begin_recording(action.timeout);
        }
        else if(action.action == "stop_recording") {
            client->end_recording(action.timeout);
        }
        else {
            spdlog::warn("Unknown action: {}", action.action);
        }
    }
}

std::string live_driving::process_scene_name(const std::string& scene_name) {
    if(const auto at_pos = scene_name.find('@'); at_pos != std::string::npos) {
        auto processed_name = scene_name.substr(0, at_pos);
        return processed_name;
    }

    return scene_name;
}

