# lowpoly_vhs_horror [Alpha]

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue?logo=cplusplus&logoColor=white)
![raylib](https://img.shields.io/badge/raylib-5.0-orange)
![CMake](https://img.shields.io/badge/CMake-3.25+-064F8C?logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows-0078D4?logo=windows&logoColor=white)
![Genre](https://img.shields.io/badge/genre-psychological%20horror-darkred)

A first-person psychological horror game built in **C++20** with raylib. You are stuck in a loop at a night gas station. Each time you leave, something changes. The store looks the same — but it isn't.

---

## Table of Contents

- [Gameplay](#gameplay)
- [Screenshots](#screenshots)
- [Requirements](#requirements)
- [Building](#building)
- [Controls](#controls)
- [Project Structure](#project-structure)

---

## Gameplay

The game takes place at an isolated gas station at night. On each loop, the world mutates — objects shift, lighting changes, the cashier moves, and eventually something that shouldn't be there starts appearing.

There are **5 loops** in total, each with a distinct mutation:

| Loop | What changes |
|---|---|
| 0 | Normal state. Talk to the cashier. |
| 1 | The cashier moved. |
| 2 | Silence. All ambient sound is gone. |
| 3 | Everything is upside down. |
| 4 | Everything is bathed in red. Music pitch drops. |
| 5 | The cashier is gone. Something else is there. |

The game ends when you look at the monster for too long.

---

## Screenshots

<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/33c70544-bb46-4cf6-81f4-78941ca2870d" />
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/fb88c733-8c52-4dd2-af86-638b92a6fb50" />
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/b77490ce-76c0-4179-8c1d-4846ee9ab576" />



---

## Requirements

| Tool / Library | Version |
|---|---|
| C++ compiler | GCC 13+ or MSVC with C++20 |
| CMake | 3.25+ |
| raylib | 5.0 (fetched automatically) |
| nlohmann-json | v3.11.2 (fetched automatically) |

Dependencies are fetched via CMake's `FetchContent` — no manual installation needed.

---

## Building

### 1. Clone the repository

```bash
git clone https://github.com/drxcm666/lowpoly_vhs_horror.git
cd lowpoly_vhs_horror
```

### 2. Configure and build

```bash
cmake -B build -S .
cmake --build build
```

The binary `MyGame` (or `MyGame.exe`) will appear in the `build/` directory.

### 3. Run

The executable must be run from the **project root** so that the `assets/` folder is accessible:

```bash
# From the project root:
./build/MyGame
```

> **Note:** The game launches in fullscreen mode by default.

---

## Controls

| Key | Action |
|---|---|
| `W` `A` `S` `D` | Move |
| Mouse | Look around |
| `E` | Interact / advance dialogue |
| `W` / `S` (in dialogue) | Navigate choices |
| `Esc` | Exit (system) |

---

## Project Structure

```
lowpoly_vhs_horror/
├── project/
│   ├── include/
│   │   ├── app.hpp               # Application lifecycle (window, render target, font)
│   │   ├── game.hpp              # Game state machine, update/render loop
│   │   ├── scene.hpp             # World geometry, triggers, lighting, mutations
│   │   ├── player.hpp            # First-person camera and movement
│   │   ├── audio.hpp             # AudioManager — sounds, music, positional emitters
│   │   ├── dialogue_manager.hpp  # Typewriter dialogue with choices and events
│   │   └── level_types.hpp       # Shared data types (CollisionBlock, Trigger, etc.)
│   └── src/
│       ├── main.cpp              # Entry point
│       ├── app.cpp               # Window creation, render-to-texture pipeline
│       ├── game.cpp              # Core game loop, interaction, event handling
│       ├── scene.cpp             # Collision, triggers, loop mutations, monster logic
│       ├── scene_load.cpp        # Model loading and lighting setup
│       ├── player.cpp            # Mouse look, WASD movement, collision response
│       ├── audio.cpp             # Sound/music loading, positional attenuation, timers
│       └── dialogue_manager.cpp  # JSON dialogue parsing, rendering, typewriter effect
├── assets/
│   ├── levels/                   # Collision and emitter definitions (.txt)
│   ├── dialogues/                # Dialogue trees per loop (.json)
│   ├── music/                    # Audio manifest and audio files
│   ├── shaders/                  # Lighting shader (rlights) + light source definitions
│   ├── font/                     # VCR OSD Mono font
│   ├── gas_station/              # Station, interior props (.glb)
│   ├── cashier/                  # Cashier character model (.glb)
│   ├── monster/                  # Monster models (.glb)
│   ├── car/                      # Car model (.glb)
│   └── sky/                      # Skybox model (.glb)
└── CMakeLists.txt
```

### Architecture overview

The game uses a simple layered architecture:

- **`App`** owns the window, render texture, and font. Runs the main loop.
- **`Game`** owns the scene, player, audio, and dialogue. Drives the state machine (`NORMAL → DIALOGUE → END`).
- **`Scene`** holds all geometry, triggers, interactables, and lights. Applies per-loop mutations via `applyMutations()`.
- **`AudioManager`** supports both global music streams and spatial emitters with distance-based volume attenuation.
- **`DialogueManager`** loads JSON conversation trees and renders them with a typewriter effect, choices, and trigger events.

### Dialogue format

Dialogues are stored in `assets/dialogues/case_N.json`. Each node has the following structure:

```json
{
  "node_id": {
    "speakerName": "Cashier",
    "text": "...",
    "choices": [
      { "text": "Option A", "nextNodeID": "node_b" }
    ],
    "nextLineId": "",
    "triggerEvent": "change_objective"
  }
}
```

`triggerEvent` fires a named event back to `Game::handleGameEvent()` to unlock triggers, change objectives, etc.

### Audio emitter format

Emitters are defined in `assets/levels/emitters.txt`:

```
// name  x  y  z  minDist  maxDist  timer  minDelay  maxDelay  zone
mus_radio  5.0  1.5  -20.0  1.0  8.0  0.0  0  0  INDOOR
sfx_creak  3.0  0.0  -18.0  1.5  6.0  5.0  4  12  INDOOR
```

Emitters prefixed with `mus_` stream music; `sfx_` play one-shot sounds on a random delay timer.
