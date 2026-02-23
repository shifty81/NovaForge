# Game State, View Modes & Ship Visual Differentiation

This document explains the game-state system, camera view modes, docking flow,
and ship rendering pipeline added to the Atlas client.  It is written for
someone who has never worked on this codebase before.

---

## Table of Contents

1. [Overview — What Changed and Why](#overview)
2. [Game States — Where Is the Player?](#game-states)
3. [Camera View Modes — What Does the Player See?](#camera-view-modes)
4. [Docking Flow — How Docking Works Step-by-Step](#docking-flow)
5. [Ship Interior & Cockpit Transitions](#ship-interior--cockpit-transitions)
6. [Ship Visual Differentiation — Why Ships Now Look Different](#ship-visual-differentiation)
7. [Key Bindings Reference](#key-bindings)
8. [How to Build, Test & Verify](#how-to-build-test--verify)
9. [Architecture Diagram](#architecture-diagram)

---

## Overview

Before these changes, the Atlas client had:
- **One camera mode** — the EVE-style orbit camera (third-person, rotating around
  the ship).
- **No concept of "game state"** — the client was always "in space" with no
  way to dock at a station or walk around inside a ship.
- **All ships looked the same** — the renderer used the legacy
  `createShipModel()` which fell back to identical placeholder meshes because
  no OBJ reference models are present in the build.

After these changes:
- **Three camera modes**: Orbit (RTS-style third-person), FPS (first-person
  walking), and Cockpit (first-person seated in the ship bridge).
- **Six game states** with validated transitions between them.
- **Ships use the modern modular generation pipeline** that creates visually
  distinct models per ship class and faction.

---

## Game States

The player is always in exactly one of these states:

| State | What It Means | Camera Mode |
|-------|---------------|-------------|
| **InSpace** | Flying your ship in a solar system | Orbit (default) or Cockpit |
| **Docking** | Playing the docking approach animation (3 seconds) | Orbit |
| **Docked** | Inside the station hangar, viewing your ship | Orbit |
| **StationInterior** | Walking around inside the station on foot | FPS |
| **ShipInterior** | Walking around inside your ship on foot | FPS |
| **Cockpit** | Seated in the ship cockpit with flight instruments | Cockpit |

### How Transitions Work

Not every state can go to every other state.  The code enforces a strict
transition graph:

```
                          ┌──────────────┐
                    ┌────►│   InSpace    │◄─────────────────────┐
                    │     └──────┬───────┘                      │
                    │            │ D+Click station               │
                    │            ▼                               │
                    │     ┌──────────────┐                      │
                    │     │   Docking    │  (3 sec timer)       │
                    │     └──────┬───────┘                      │
                    │            │ timer expires                 │
                    │            ▼                               │
                    │     ┌──────────────┐                      │
         Undock     │     │   Docked     │◄──────┐              │
         ───────────┘     └──┬───┬───┬───┘       │ Return to    │
                             │   │   │           │ hangar       │
                  Enter      │   │   │ Enter     │              │
                  station    │   │   │ ship      │              │
                  interior   │   │   │ interior  │              │
                             ▼   │   ▼           │              │
              ┌──────────────┐   │  ┌───────────────┐          │
              │StationInterior│  │  │ ShipInterior  │──────────┤
              └──────┬────────┘  │  └───────┬───────┘          │
                     │           │          │                   │
                     │ Enter ship│          │ Enter cockpit     │
                     └───────────┤          ▼                   │
                                 │  ┌──────────────┐           │
                                 └─►│   Cockpit    │───────────┘
                                    └──────────────┘  (V key or
                                                       undock)
```

If you try an invalid transition (like going straight from InSpace to
StationInterior), the code prints a log message and ignores the request.

### Where Is This Code?

- **Enum**: `cpp_client/include/core/application.h` — `Application::GameState`
- **Transition logic**: `cpp_client/src/core/application.cpp` — `requestStateTransition()`
- **State name helper**: `Application::gameStateName()` — returns a printable name

---

## Camera View Modes

The camera has three modes, defined in `cpp_client/include/rendering/camera.h`:

### 1. Orbit Mode (default) — `ViewMode::ORBIT`

This is the standard EVE Online third-person camera.  You see your ship from
the outside.

- **Right-click drag**: Rotate the camera around your ship.
- **Scroll wheel**: Zoom in/out (logarithmic, 10m to 5000m range).
- **Inertia**: After you release the mouse, the camera keeps spinning slowly
  and decelerates (like EVE).

This is what the user called "RTS-style orbit view".

### 2. FPS Mode — `ViewMode::FPS`

First-person view for walking inside stations and ship interiors.

- **Right-click drag**: Mouse-look (rotate your view direction).
- The camera is positioned at eye height (1.8m for station, 1.6m for ship).
- No zoom — you are "inside" looking through your character's eyes.

### 3. Cockpit Mode — `ViewMode::COCKPIT`

First-person view seated in the ship cockpit.  Similar to FPS mode but:

- Camera is at cockpit seat height (1.4m, slightly lower).
- Camera is positioned further forward (toward the ship nose).
- In space, this gives you a "pilot's eye view" of the battle.
- When docked, this lets you see the hangar from the cockpit window.

### How the Camera Switches Between Modes

When the game state changes, the camera mode is automatically set:

```
InSpace          → ORBIT
Docking          → ORBIT
Docked           → ORBIT (hangar view)
StationInterior  → FPS
ShipInterior     → FPS
Cockpit          → COCKPIT
```

### Where Is This Code?

- **Enum + API**: `cpp_client/include/rendering/camera.h` — `ViewMode`, `setViewMode()`, `setFPSPosition()`, `rotateFPS()`
- **Implementation**: `cpp_client/src/rendering/camera.cpp` — view matrix logic, mode switching
- **View matrix**: In ORBIT mode, `getViewMatrix()` uses `glm::lookAt(orbitPosition, target, up)`.
  In FPS/COCKPIT mode, it uses `glm::lookAt(fpsPosition, fpsPosition + fpsForward, up)`.

---

## Docking Flow

Here is exactly what happens when you dock at a station:

### Step 1: Get Near a Station

Fly your ship within **2500 meters** of a station.  (Stations are shown in the
overview panel on the right side of the screen.)

### Step 2: Press D, Then Click the Station

Press the **D** key to enter "Dock/Jump" mode.  The HUD shows
`DOCK / JUMP - click a station or gate`.  Then left-click on the station.

(If you click a stargate instead, you jump to another solar system.)

### Step 3: Docking Animation (3 seconds)

The game state changes to `Docking`.  Your ship stops moving.  A 3-second timer
counts down (the HUD shows `DOCKING`).  During this time, the camera stays in
orbit mode so you can watch your ship approach the station.

### Step 4: You Are Docked

After 3 seconds, the state changes to `Docked`.  The HUD shows `DOCKED`.  You
are now in the station hangar, looking at your ship from the outside (orbit
camera).

### Step 5: What You Can Do While Docked

From the `Docked` state, you can:

- **Undock** (return to space) — via the Station Services panel "Undock" button,
  or by calling `requestUndock()`.
- **Enter the station interior** — walk around the station on foot (FPS mode).
- **Enter your ship interior** — walk around inside your ship (FPS mode).
- **Go straight to the cockpit** — sit in the pilot's seat (Cockpit mode).

### Step 6: Undocking

When you undock, your ship is placed **500 meters** outside the station and the
game state returns to `InSpace` with the orbit camera.

### Where Is This Code?

- **`requestDock()`**: Checks if you're in range of a station, stops your ship,
  starts the docking animation timer.
- **`requestUndock()`**: Places your ship outside the station, clears the docked
  station ID, transitions to InSpace.
- **Docking timer**: Updated in `Application::update()` — counts down and
  automatically transitions to Docked when it reaches zero.

---

## Ship Interior & Cockpit Transitions

### Entering the Station Interior

From the `Docked` state, call `enterStationInterior()`.  This:
1. Validates the transition (Docked → StationInterior ✓).
2. Switches the camera to FPS mode.
3. Places the camera at position (0, 1.8, 0) — eye height, center of the
   interior zone — looking forward.
4. The HUD shows `STATION INTERIOR`.

### Entering the Ship Interior

From `Docked` or `StationInterior`, call `enterShipInterior()`.  This:
1. Validates the transition.
2. Switches the camera to FPS mode.
3. Places the camera at (0, 1.6, 2) — inside the ship bridge area.
4. The HUD shows `SHIP INTERIOR`.

### Entering the Cockpit

From `ShipInterior` or `Docked`, call `enterCockpit()`.  This:
1. Validates the transition.
2. Switches the camera to COCKPIT mode.
3. Places the camera at (0, 1.4, -1) — seated in the cockpit.
4. The HUD shows `COCKPIT`.

### Using the V Key

Press **V** to quickly toggle between views without calling individual methods:

| Current State | V Key Does |
|---------------|------------|
| InSpace (Orbit) | → Cockpit (fly from pilot seat) |
| InSpace (Cockpit) | → InSpace (back to orbit) |
| Docked | → Cockpit |
| ShipInterior | → Cockpit |
| Cockpit (docked) | → ShipInterior |
| Cockpit (in space) | → InSpace (orbit) |
| StationInterior | Prints "Board your ship first" |

### Returning to the Hangar

From any interior state (StationInterior, ShipInterior, Cockpit), call
`returnToHangar()` to go back to the `Docked` state (orbit camera, hangar view).

---

## Ship Visual Differentiation

### The Problem

Previously, the renderer called `Model::createShipModel()` to create the 3D
mesh for each ship entity.  This function has a three-tier fallback:

1. Try to load a `.obj` file (e.g., `veyren_frigate_fang.obj`).
2. Try to use the procedural seed OBJ generator.
3. Fall back to hardcoded procedural hull extrusion.

In practice, steps 1 and 2 fail (no OBJ files present in the build), so
**every ship ends up using step 3** — which produces similar-looking extruded
tubes with the same basic shape.  The only differences are size and vertex
colors, but the shapes are nearly identical.

### The Fix

The renderer now calls `Model::createShipModelWithRacialDesign()` instead.
This function has a better fallback:

1. Try to load a `.obj` file (same as before).
2. Try to use the procedural seed OBJ generator (same as before).
3. **NEW**: Use the `ShipPartLibrary` modular assembly system.
4. Fall back to the old extrusion only if modular assembly produces no parts.

The **ShipPartLibrary** (step 3) is the modern approach.  It:
- Has **50+ pre-defined ship parts** per faction (hull sections, engines,
  turrets, launchers, spires, framework).
- Assembles parts according to **class-specific rules** (frigates get 2 engines,
  battleships get 4; Solari ships get golden spires, Keldari get exposed
  framework).
- Applies **faction-specific colors** baked into the vertex data:
  - **Solari**: Gold/brass
  - **Veyren**: Steel blue/grey
  - **Aurelian**: Green/organic
  - **Keldari**: Rust brown

### What This Looks Like In Practice

| Ship Type | Faction | Visual Difference |
|-----------|---------|-------------------|
| Player's "Fang" | Keldari | Small rusty frigate with exposed framework |
| NPC "Crimson Order" | Crimson Order | Medium cruiser, red-tinted |
| NPC "Venom Syndicate Scout" | Venom Syndicate | Small frigate, different shape |
| NPC "Iron Corsairs Watchman" | Iron Corsairs | Destroyer-sized, different proportions |

### Where Is This Code?

- **The one-line fix**: `cpp_client/src/rendering/renderer.cpp` line ~312 —
  changed from `createShipModel()` to `createShipModelWithRacialDesign()`.
- **Modular assembly**: `cpp_client/src/rendering/model.cpp` lines 668-940 —
  the `createShipModelWithRacialDesign()` function.
- **Part library**: `cpp_client/include/rendering/ship_part_library.h` and
  `cpp_client/src/rendering/ship_part_library.cpp`.
- **Generation rules**: `cpp_client/include/rendering/ship_generation_rules.h`.

---

## Key Bindings

| Key | Action |
|-----|--------|
| **Right-click drag** | Orbit camera (space) / Mouse-look (FPS/cockpit) |
| **Scroll wheel** | Zoom in/out (orbit mode only) |
| **Q + click** | Approach target |
| **W + click** | Orbit target |
| **E + click** | Keep at range |
| **D + click** | Dock at station / Jump through stargate |
| **S + click** | Warp to target |
| **V** | Toggle view mode (Orbit ↔ Cockpit, or FPS ↔ Cockpit) |
| **Ctrl+Space** | Stop ship |
| **Tab** | Cycle targets |
| **F1–F8** | Activate modules (weapons, etc.) |
| **Esc** | Pause menu |
| **\`** (backtick) | Developer console |
| **Alt+O** | Toggle overview panel |

---

## How to Build, Test & Verify

### Fresh Build (Recommended After Pulling)

```bash
# From the repository root
make clean          # Remove old build artifacts
make test-engine    # Build and run all engine tests (including new ViewMode tests)
```

You should see at the end:
```
--- Game State & ViewMode ---
[PASS] test_camera_default_view_mode
[PASS] test_camera_set_view_mode_orbit
[PASS] test_camera_set_view_mode_fps
[PASS] test_camera_set_view_mode_cockpit
[PASS] test_camera_fps_position_and_forward
[PASS] test_camera_fps_rotate
[PASS] test_camera_orbit_position_unchanged_in_fps
[PASS] test_camera_view_mode_kills_inertia
[PASS] test_camera_view_matrix_differs_by_mode

=== All tests passed! ===
```

### Building the Full Client

```bash
make build          # Build client + server (Release mode)
# OR
make build-client   # Build just the C++ client
```

The executable will be at `cpp_client/build/atlas_client` (or similar,
depending on your CMake configuration).

### Do I Need to Delete and Re-pull?

**No.** Just `git pull` and `make clean && make build` (or `make test-engine`
for tests).  The `make clean` step ensures stale object files are removed and
everything is rebuilt fresh with the new code.

If you're having issues, the nuclear option is:
```bash
make clean
rm -rf build cpp_client/build
make build
```

---

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application                              │
│                                                                 │
│  GameState ──► determines ──► Camera ViewMode                   │
│  (InSpace, Docking, Docked,    (ORBIT, FPS, COCKPIT)           │
│   StationInterior, ShipInterior,                                │
│   Cockpit)                                                      │
│                                                                 │
│  Input (V key) ──► toggleViewMode() ──► requestStateTransition()│
│  Input (D+click) ──► requestDock() ──► Docking timer ──► Docked │
│  UI (Undock btn) ──► requestUndock() ──► InSpace                │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│                         Camera                                  │
│                                                                 │
│  ORBIT mode:   lookAt(orbitPosition, target)    ← right-drag   │
│  FPS mode:     lookAt(fpsEyePos, fpsEyePos+fwd) ← mouse-look  │
│  COCKPIT mode: lookAt(cockpitPos, cockpitPos+fwd) ← mouse-look │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│                        Renderer                                 │
│                                                                 │
│  createEntityVisual() ──► Model::createShipModelWithRacialDesign│
│                               │                                 │
│                    ┌──────────┼──────────┐                      │
│                    ▼          ▼          ▼                       │
│                 OBJ file   Seed OBJ   ShipPartLibrary           │
│                 (best)     (good)     (modular, always works)   │
│                                          │                      │
│                           ┌──────────────┼──────────┐           │
│                           ▼              ▼          ▼           │
│                        Hull parts    Engines    Turrets/etc.    │
│                        (per-faction) (per-class)                │
└─────────────────────────────────────────────────────────────────┘
```
