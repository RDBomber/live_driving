#include <windows.h>
#include <string>
#include <vector>
#include <psapi.h>
#include <spdlog/spdlog.h>
#include <filesystem>

#include "live_driving/obs_client.hpp"
#include "live_driving/hook.hpp"
#include "live_driving/config.hpp"
#include "live_driving/game.hpp"

struct initialize_params {
    HMODULE dll_instance = nullptr;
    live_driving::app_config config;
};

void handle_debug_mode() {
    if(AllocConsole()) {
        FILE* file;
        freopen_s(&file, "CONOUT$", "w", stdout);
        freopen_s(&file, "CONOUT$", "w", stderr);
    }

    spdlog::info("Debug mode enabled");
}

std::filesystem::path get_current_directory(const HMODULE dll_instance) {
    char path[MAX_PATH];
    if(!GetModuleFileNameA(dll_instance, path, MAX_PATH)) {
        spdlog::error("Failed to get module file name");
        exit(EXIT_FAILURE);
    }

    std::filesystem::path module_path(path);
    return module_path.parent_path();
}

live_driving::app_config get_config(const std::filesystem::path& config_path) {
    try {
        return live_driving::get_config(config_path);
    }
    catch(const std::exception& e) {
        const auto message = std::format("Failed to load config: {}", e.what());
        MessageBox(nullptr, message.c_str(), "live_driving.dll", MB_OK);
        exit(EXIT_FAILURE);
    }
}

DWORD WINAPI initialize(LPVOID param) {
    auto* params = static_cast<initialize_params*>(param);
    auto config = params->config;

    const auto current_process = GetCurrentProcess();

    for(const auto& [module, group] : live_driving::get_games()) {
        const auto module_handle = GetModuleHandle(module);
        if(module_handle == nullptr) {
            continue;
        }

        spdlog::info("Found module: {}", module);

        MODULEINFO module_info;
        GetModuleInformation(current_process, module_handle, &module_info, sizeof(module_info));

        live_driving::game_info game_info;
        game_info.module_info = module_info;
        game_info.group = group;

        create_hooks(game_info, config);
        break;
    }

    spdlog::warn("Could not find any supported game modules");
    delete params;

    return EXIT_SUCCESS;
}

BOOL DllMain(const HMODULE dll_instance, const DWORD reason, LPVOID) {
    if(reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    const auto module_directory = get_current_directory(dll_instance);
    const auto config_path = module_directory / "live_driving.yaml";
    const auto config = get_config(config_path);

    if(config.debug) {
        handle_debug_mode();
    }

    const auto params = new initialize_params;
    params->dll_instance = dll_instance;
    params->config = config;

    DisableThreadLibraryCalls(dll_instance);

    CreateThread(nullptr, 0, initialize, params, 0, nullptr);

    return TRUE;
}