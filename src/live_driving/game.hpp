#pragma once

#include <Windows.h>
#include <psapi.h>

namespace live_driving {
    enum class game_group {
        CBaseScene,
        sequence,
    };

    struct game_info {
        MODULEINFO module_info;
        game_group group;
    };

    auto get_games();
}

inline auto live_driving::get_games() {
    return std::array {
        std::tuple {
            "bm2dx.dll",
            game_group::CBaseScene,
        },
        std::tuple {
            "bm2dx.exe",
            game_group::CBaseScene,
        },
        std::tuple {
            "soundvoltex.dll",
            game_group::CBaseScene,
        },
        std::tuple {
            "sv6c.exe",
            game_group::CBaseScene,
        },
        std::tuple {
            "gamemdx.dll",
            game_group::sequence,
        },
    };
}
