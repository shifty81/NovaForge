# Master Test Solar System — Design Document

> A developer-only sandbox solar system where every PCG asset type is
> represented, editable, and persistent. Changes made here propagate to all
> procedural generation, shaping future worlds for normal players.
>
> See also: [Editor Tool Layer](editor_tool_layer.md) ·
> [Extended Tooling Features](extended_tooling_features.md)

## Concept Overview

### Purpose

A hidden test solar system where all PCG assets (planets, stations, ships,
rigs, props, terrain) exist in full detail. Developers use it to tweak asset
placement, shapes, and interactions. Changes propagate to PCG algorithms,
shaping future procedural generations for the live game.

### Access Control

- Restricted to developers / in-editor mode.
- Normal players never see this solar system.

### Persistence

- Every change is stored in DeltaEdits.
- Updates influence all future procedural generation, serving as a "guideline
  template" for the engine.

## Scene & Asset Hierarchy

```
TestSolarSystem
 ├─ Planets
 │   ├─ Planet_01 (spawnable, editable terrain)
 │   ├─ Planet_02
 │   └─ Planet_X
 ├─ Stations
 │   ├─ Station_01
 │   └─ Station_02
 ├─ Ships
 │   ├─ PlayerShips
 │   └─ NPCShips
 ├─ Rigs / Modules
 │   └─ All types for ships, stations, planets
 ├─ Props
 │   ├─ Environmental (asteroids, debris)
 │   └─ Interactive (crates, barrels, machinery)
 └─ TestAssets
     ├─ AI spawnable prefabs
     └─ Procedural template pieces
```

### Requirements

- Every PCG asset type must exist at least once in scene.
- All assets fixed in scene, but fully editable via ToolingLayer.
- Real-time mesh updates for terrain, stations, ships, rigs, and props.

## Tooling Integration

### Editing Features

- **Move / Rotate / Scale** — Full 3D transform on all assets.
- **Mesh Editing** — Live mesh regeneration updates the scene.
- **Physics & Procedural Parameters** — Modify stiffness, drag, environmental
  influence.
- **Function Assignment** — Assign test behaviors for ships, stations, or
  props.
- **Category Filtering** — Easily isolate asset types (planets, stations,
  ships, props).

### Live Preview

- Physics simulation, collisions, and animations active in real-time.
- Can pause, step frame, or scrub simulation to test interactions.

### DeltaEdits Storage

- Every modification saved and linked to PCG baseline.
- Changes applied automatically when procedural generation builds "normal"
  player solar systems.

## Developer Workflow

| Step | Action | Notes |
|------|--------|-------|
| 1 | Load Test Solar System | Normal players cannot access this scene |
| 2 | Open ToolingLayer | Full UI available |
| 3 | Select Asset / Category | Planets, Stations, Ships, Modules, Props |
| 4 | Edit Transform / Mesh / Parameters | Adjust positions, shapes, physics, functions |
| 5 | Live Preview Simulation | Test environment reactions, AI spawns, physics |
| 6 | Commit DeltaEdits | Persist modifications to PCG baseline |
| 7 | Apply to PCG Engine | Future procedural worlds for normal players inherit these modifications |
| 8 | Iterative Testing | Update meshes, physics, and placement until desired PCG "guidelines" are reached |

## Implementation Considerations

### Hidden Scene Loading

- Test system can be loaded only in editor mode or via a developer flag.
- Avoid memory load in production builds for normal players.

### PCG Integration

- PCG algorithms reference asset positions, scales, and parameters from Test
  Solar System.
- For example: planet spacing, station placement rules, asteroid density, ship
  designs, and rig configurations.

### Real-Time Mesh Updates

- Changes in editor immediately update visual meshes, collision meshes, and
  procedural placement logic.

### DeltaEdits Propagation

- Modifications to any asset (planet, station, ship, module) automatically
  update PCG templates.
- DeltaEdits versioned so you can rollback or branch for experiments.

### Full Asset Coverage

Must include:

- All ship types & rigs
- Planet types & terrain variants
- Station modules
- Props (interactive & environmental)
- Characters, NPCs, AI spawn templates

Ensures PCG engine can "learn" the full asset palette.

## JSON Blueprint

The following JSON blueprint defines the initial structure of the Master Test
Solar System scene, including all asset categories, physics parameters, and
DeltaEdits scaffolding.

```json
{
  "testSolarSystem": {
    "id": "master_template_system",
    "access": "developer_only",
    "environment": {
      "gravity": 1.0,
      "atmosphereType": "earthLike",
      "windVector": [0.0, 0.0, 0.0]
    },
    "planets": [
      {
        "id": "planet_01",
        "type": "rocky",
        "position": [0, 0, 0],
        "rotation": [0, 0, 0],
        "scale": [1, 1, 1],
        "meshEditable": true,
        "physicsParameters": { "gravity": 1.0, "drag": 0.1 },
        "props": [],
        "stations": []
      },
      {
        "id": "planet_02",
        "type": "gas_giant",
        "position": [10000, 0, 0],
        "rotation": [0, 0, 0],
        "scale": [5, 5, 5],
        "meshEditable": true,
        "physicsParameters": { "gravity": 3.0, "drag": 0.05 },
        "props": [],
        "stations": []
      }
    ],
    "stations": [
      {
        "id": "station_01",
        "type": "trading",
        "position": [500, 0, 0],
        "rotation": [0, 45, 0],
        "modules": [
          { "id": "docking_bay_01", "editable": true, "functionAssignable": true },
          { "id": "power_core_01", "editable": true, "functionAssignable": true }
        ],
        "props": [
          { "id": "container_01", "editable": true, "functionAssignable": true }
        ]
      }
    ],
    "ships": [
      {
        "id": "player_freighter_01",
        "position": [0, 10, 0],
        "rotation": [0, 0, 0],
        "modules": [
          { "id": "engine_01", "editable": true, "functionAssignable": true },
          { "id": "turret_01", "editable": true, "functionAssignable": true },
          { "id": "cargo_bay_01", "editable": true, "functionAssignable": true }
        ],
        "rigs": [
          {
            "id": "main_rig",
            "bones": ["spine", "turret_mount", "engine_mount"],
            "editable": true
          }
        ],
        "props": [
          { "id": "external_antenna_01", "editable": true, "functionAssignable": true }
        ]
      }
    ],
    "props": [
      { "id": "asteroid_small_01", "type": "environment", "editable": true, "functionAssignable": false },
      { "id": "fuel_barrel_01", "type": "prop", "editable": true, "functionAssignable": true }
    ],
    "characters": [
      {
        "id": "player_character_01",
        "position": [1, 0, 0],
        "rotation": [0, 0, 0],
        "skeleton": {
          "bones": [
            "spine_base", "spine_mid", "spine_top", "head",
            "left_arm", "right_arm", "left_leg", "right_leg"
          ],
          "animations": [
            {
              "id": "walk_forward",
              "liveEditable": true,
              "physicsInfluenced": false
            },
            {
              "id": "cape_idle",
              "liveEditable": true,
              "physicsInfluenced": true,
              "physicsParameters": {
                "stiffness": 0.8,
                "damping": 0.3,
                "gravityInfluence": 1.0,
                "drag": 0.25
              },
              "environmentPresets": {
                "zeroG": { "stiffness": 0.5, "drag": 0.05 },
                "lowGPlanet": { "stiffness": 0.7, "drag": 0.15 },
                "earthLike": { "stiffness": 0.8, "drag": 0.25 },
                "windy": { "stiffness": 0.9, "drag": 0.35 }
              }
            }
          ]
        }
      }
    ],
    "toolingLayer": {
      "enabled": true,
      "uiPanels": {
        "selectionPanel": [
          "Characters", "Ships", "Modules", "Props",
          "Hangar Interiors", "Planets", "Stations"
        ],
        "physicsPanel": [
          "Stiffness", "Damping", "Gravity Influence",
          "Drag", "Vertex/Bone Weighting"
        ],
        "functionPanel": [
          "Assign Function / Trigger",
          "Preview Function Execution"
        ],
        "environmentPresetPanel": [
          "Zero-G", "Low-G", "Earth-Like", "Windy/Turbulent"
        ],
        "livePreviewPanel": [
          "Play/Pause Simulation", "Scrub Timeline",
          "Move Character/Assets"
        ],
        "deltaEditsPanel": [
          "Save Modifications", "Assign to PCG Baseline",
          "Versioning per Preset"
        ]
      }
    },
    "deltaEdits": {
      "enabled": true,
      "persistent": true,
      "records": []
    }
  }
}
```

## Bonus Features

- **AI-Assisted Placement** — AI can suggest optimal positions for assets or
  generate variations on the fly.
- **Environment Simulation Controls** — Adjust gravity, atmosphere, and wind
  to test how assets respond.
- **Multi-Developer Merge** — Multiple devs can edit Test Solar System; merge
  DeltaEdits sets to consolidate guidelines.
- **Undo / Redo Stack** — Critical for experimentation without breaking
  baseline templates.
