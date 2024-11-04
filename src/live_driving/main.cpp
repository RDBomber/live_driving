#include <windows.h>
#include <string>
#include <vector>
#include <psapi.h>
#include <spdlog/spdlog.h>

#include "live_driving/hook.hpp"

DWORD initialize(LPVOID param) {
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
        live_driving::create_hooks(module_info);
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