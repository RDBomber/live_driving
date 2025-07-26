#include <windows.h>
#include <string>
#include <vector>
#include <psapi.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <thread>

#include "live_driving/obs_client.hpp"
#include "live_driving/hook.hpp"

struct initialize_params {
    HMODULE dll_instance;
    YAML::Node config;
};

void handle_debug_mode() {
    AllocConsole();
    FILE* file;
    freopen_s(&file, "CONOUT$", "w", stdout);
    freopen_s(&file, "CONOUT$", "w", stderr);

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

YAML::Node get_config(const std::filesystem::path& config_path) {
    if(!exists(config_path)) {
        spdlog::error("Config file not found");
        exit(EXIT_FAILURE);
    }

    return YAML::LoadFile(config_path.string());
}

DWORD initialize(LPVOID param) {
    initialize_params* params = static_cast<initialize_params*>(param);
    auto config = params->config;

    std::optional<live_driving::obs_client> obs_client;
    std::unordered_map<std::string, std::string> map;

    if(config["obs_url"]) {
        auto url = config["obs_url"].as<std::string>();
        std::string password;
        if(config["obs_password"]) {
            password = config["obs_password"].as<std::string>();
        }

        if(config["scene_map"]) {
            map = config["scene_map"].as<std::unordered_map<std::string, std::string>>();
        }

        live_driving::obs_client client(url, password);
        obs_client = client;
    }

    const std::vector<std::string> game_modules = {
        "bm2dx.dll",
        "soundvoltex.dll",
        "sv6c.exe",
        "bm2dx.exe",
    };

    const auto current_process = GetCurrentProcess();

    for(const auto& module : game_modules) {
        const auto module_handle = GetModuleHandle(module.c_str());
        if(module_handle == nullptr) {
            continue;
        }

        spdlog::info("Found module: {}", module);

        MODULEINFO module_info;
        GetModuleInformation(current_process, module_handle, &module_info, sizeof(module_info));
        create_hooks(module_info, obs_client.has_value() ? &obs_client.value() : nullptr, map);

        if(obs_client.has_value()) {
            std::thread([&obs_client = obs_client] {
                obs_client->listen();
            }).join();
        }

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
    auto config = get_config(config_path);

    if(config["debug"]) {
        if(config["debug"].as<bool>()) {
            handle_debug_mode();
        }
    }

    const auto params = new initialize_params;
    params->dll_instance = dll_instance;
    params->config = config;

    DisableThreadLibraryCalls(dll_instance);

    CreateThread(nullptr, 0, initialize, params, 0, nullptr);

    return TRUE;
}