# Editor Tool Layer — Design Document

> Refactored editor embedded as an optional tooling overlay inside the game client.
> Toggle with **F12**; compile-out entirely for release builds.

## Overview

The **EditorToolLayer** makes all editor panels (viewport, PCG preview,
ship archetype, etc.) available as an overlay on top of the running game.
This lets developers iterate on content without leaving the game client.

When ready for release, set `NOVAFORGE_EDITOR_TOOLS=OFF` and every
editor header, source file, and runtime object is compiled out — zero
overhead in the shipping build.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│  Game Client (Application)                              │
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────┐ │
│  │ Renderer     │  │ AtlasHUD     │  │ Title Screen  │ │
│  └──────────────┘  └──────────────┘  └───────────────┘ │
│                                                         │
│  #ifdef NOVAFORGE_EDITOR_TOOLS                          │
│  ┌──────────────────────────────────────────────────┐   │
│  │ EditorToolLayer                                  │   │
│  │                                                  │   │
│  │  ┌──────────────┐  ┌──────────────────────────┐  │   │
│  │  │ ITool[]      │  │ EditorCommandBus         │  │   │
│  │  └──────────────┘  └──────────────────────────┘  │   │
│  │                                                  │   │
│  │  ┌──────────────────────────────────────────┐    │   │
│  │  │ EditorLayout (dock tree + menu bar)      │    │   │
│  │  │  ┌────────────┐ ┌──────────┐ ┌────────┐ │    │   │
│  │  │  │ Viewport   │ │ PCG Prev │ │ Ship A.│ │    │   │
│  │  │  │ SceneGraph │ │ GenStyle │ │ Assets │ │    │   │
│  │  │  │ GalaxyMap  │ │ Mission  │ │ Fleet  │ │    │   │
│  │  │  │ DataBrowse │ │ CharSel  │ │ Module │ │    │   │
│  │  │  │ NPC Editor │ │ Packager │ │        │ │    │   │
│  │  │  └────────────┘ └──────────┘ └────────┘ │    │   │
│  │  └──────────────────────────────────────────┘    │   │
│  │                                                  │   │
│  │  ┌──────────────────────────────────────────┐    │   │
│  │  │ KeybindManager + UndoStack + AIAggregator│    │   │
│  │  └──────────────────────────────────────────┘    │   │
│  └──────────────────────────────────────────────────┘   │
│  #endif                                                 │
└─────────────────────────────────────────────────────────┘
```

## Key Classes

| Class | Location | Purpose |
|-------|----------|---------|
| `ITool` | `cpp_client/include/editor/itool.h` | Base interface for editor tools |
| `EditorCommandBus` | `cpp_client/include/editor/editor_command_bus.h` | Decoupled FIFO command queue |
| `EditorToolLayer` | `cpp_client/include/editor/editor_tool_layer.h` | Main overlay — owns panels, layout, keybinds |

## Build Flags

| Flag | Default | Effect |
|------|---------|--------|
| `NOVAFORGE_EDITOR_TOOLS` | OFF | Compile editor tool sources into the client |

### Build commands

```bash
# Client with editor tools (development)
make build-client-editor

# Client without editor tools (release)
make build-client

# Standalone editor (unchanged)
make build-editor
```

### CMake usage

```bash
cmake .. -DNOVAFORGE_EDITOR_TOOLS=ON   # embed tools
cmake .. -DNOVAFORGE_EDITOR_TOOLS=OFF  # strip tools
```

## Lifecycle

1. **Construction** — `Application` creates `EditorToolLayer` (lightweight).
2. **init()** — Creates all panels, wires keybinds, loads saved layout.
3. **Per frame** — `draw(ctx)` renders the editor overlay when active.
4. **handleKeyPress()** — Forwards keys from the client input system.
5. **shutdown()** — Tears down panels before UI context destruction.

## Key Bindings (default)

| Key | Action |
|-----|--------|
| F12 | Toggle editor overlay |
| Ctrl+Z | Undo |
| Ctrl+Y | Redo |
| W | Translate gizmo |
| E | Rotate gizmo |
| R | Scale gizmo |
| G | Toggle grid |
| Ctrl+S | Save layout + overrides |

## Character Select → Hangar Spawn

After completing the title screen's character creation flow, the player now
spawns **docked at the nearest station** instead of floating in open space.
The game state transitions directly to `GameState::Docked` with the camera
in orbit mode showing the hangar environment.

### Flow

```
Title Screen (6 pages) → "Enter Hangar" clicked
  → m_playCb fires
  → Find nearest station in solar system
  → Teleport player to station position
  → Set GameState::Docked + orbit camera
  → Show "DOCKED" mode indicator
```

## Future Work (from editornewfeat.md)

### Phase 1 — Tool Extensions
- Multi-selection & group editing
- Snap/align tools (grid, surface, asset alignment)
- Prefab / asset library (drag-and-drop reusable modules)
- Scene bookmarking / camera checkpoints

### Phase 2 — Physics & Animation Tools
- Dynamic physics tuning (rigidbody, joints, cloth)
- Animation layer editing
- IK / rigging tools for characters and turrets
- Simulation freeze / step-through mode

### Phase 3 — Function & Behavior Editing
- Function / event assignment to props and entities
- Event timeline editor for scripted sequences
- AI / PCG asset preview with prompt-based generation

### Phase 4 — PCG & DeltaEdits Utilities
- PCG snapshot / rollback
- DeltaEdits merge & conflict resolution
- Edit propagation across similar assets
- Visual diff / comparison (current vs baseline)

### Phase 5 — Advanced Workflow
- "Edit While Playing" mode (tweak during gameplay)
- Environment simulation control (gravity, wind, atmosphere)
- Live material / shader editing
- Batch operations for mass transformations
