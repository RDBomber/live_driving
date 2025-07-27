# What is this
This is a simple dll hook for capturing scene changes in select BEMANI games.

# Supported games
- beatmania IIDX (including コナステ)
- SOUND VOLTEX (including コナステ)

# Usage
Inject `live_driving.dll` with your preferred toolset. Make sure `live_driving.yaml` is placed next to `live_driving.dll`.

# Configuration
```yml
obs_url: "ws://localhost:4455" # The address of your OBS websocket server
obs_password: "password" # Optional password if you wish to use authorization
debug: true # Spawns a console (useful for コナステ games)
use_rtti: true # Uses RTTI class names for scene mapping, no longer have to guess scene IDs
scene_map: # Mapping of game scene IDs or names (when using RTTI) to OBS scenes
  CTestModeFlow:
    obs_scene: "IIDX"
    timeout: 1000
  default: # default will trigger on any scene that is not mapped, you can also omit this if you don't want that behaviour
    obs_scene: "IIDX no cam"
    timeout: 0
```

# Scene IDs (for use with `use_rtti: false`)
Here are some IDs I've found while testing

Feel free to contribute more by creating a PR
## IIDX 31 (may work for other versions)
- 66 - DAN COURSE gameplay
- 69 - STANDARD gameplay
- 70 - FREE gameplay
- 72 - HAZARD gameplay
- 74 - STEP UP gameplay
- 75 - PREMIUM FREE gameplay
- 76 - ARENA gameplay
- 77 - BPL BATTLE gameplay

## SOUND VOLTEX (コナステ)
- 16 - Music Select
- 40 - Gameplay
- 14 - Result
- 33 - SKILL ANALYZER select
- 15 - SKILL ANALYZER result

## How to find scene IDs
When you leave `obs_url` blank in the configuration, the hook will only output scene IDs to the console, making it easier to find the scene IDs for your game.
