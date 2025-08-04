# What is this
This is a simple dll hook for capturing scene changes in select BEMANI games.

# Supported games
- beatmania IIDX (including コナステ)
- SOUND VOLTEX (including コナステ)
- DanceDanceRevolution

# Usage
Inject `live_driving.dll` with your preferred toolset. Make sure `live_driving.yaml` is placed next to `live_driving.dll`.

# Configuration
```yml
obs_url: "ws://localhost:4455" # The address of your OBS websocket server
obs_password: "password" # Optional password if you wish to use authorization
scene_map: # Mapping of game scene IDs or names to a set of specific actions
  CStandardStageScene:
    - action: change_scene # Refer to "Actions" section for available actions
      param: "IIDX"
      timeout: 0 # Timeout in milliseconds before the action is executed
    - action: start_recording
      timeout: 0
  CStageResultScene:
    - action: stop_recording
      timeout: 5000
  default: # "default" scene will trigger on every unmapped scene, delete this section to disable
    - action: change_scene
      param: "IIDX no cam"
      timeout: 0
```

## Actions
### `change_scene`
Changes the OBS scene to the specified name in `param`.

### `start_recording`
Starts recording in OBS.

### `stop_recording`
Stops recording in OBS.

# Scene names
## IIDX
- `CStandardStageScene`
- `CLifeStageScene` (STEP UP mode)
- `CDanStageScene`
- `CPremiumFreeStageScene`
- `CArenaStageScene`
- `CBPLBattleStageScene`

## DDR
- `SelectMusicSequence` - On music select
- `DancePlaySequence` - On game play
- `ResultSequence` - On result screen

# Scene IDs
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