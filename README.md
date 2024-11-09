# What is this
This is a simple dll hook for capturing scene changes in select BEMANI games.

# Supported games
- beatmania IIDX
- SOUND VOLTEX

# Usage
Inject `live_driving.dll` with your preferred toolset. Make sure `live_driving.yml` is placed next to `live_driving.dll`.

# Configuration
```yml
obs_url: "ws://localhost:4455" # The address of your OBS websocket server
obs_password: "password" # Optional password if you wish to use authorization
scene_map: # Mapping of game scene IDs to OBS scenes
  1: "IIDX"
  default: "IIDX no cam" # `default` will trigger on any scene that is not mapped, you can also omit this if you don't want that behaviour
```

# Scene IDs
Here are some IDs I've found while testing
## IIDX 31 (may work for other versions)
