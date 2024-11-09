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

DWORD initialize(LPVOID param) {
    std::filesystem::path config_path = "live_driving.yaml";
    if(!exists(config_path)) {
        spdlog::error("Config file not found");
        return EXIT_FAILURE;
    }

    YAML::Node config;
    try {
        config = YAML::LoadFile(config_path.string());
    }
    catch(const std::exception& e) {
        spdlog::error("Failed to load config file: {}", e.what());
        return EXIT_FAILURE;
    }

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

    return EXIT_SUCCESS;
}

BOOL DllMain(const HMODULE dll_instance, const DWORD reason, LPVOID) {
    if(reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    DisableThreadLibraryCalls(dll_instance);

    CreateThread(nullptr, 0, initialize, dll_instance, 0, nullptr);

    return TRUE;
}