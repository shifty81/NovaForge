# NovaForge Editor Tools — Developer Reference

> Consolidated reference for the EditorToolLayer infrastructure, tool inventory,
> editor panels, build configuration, and test coverage.

## 1. Overview

The **EditorToolLayer** is an optional overlay embedded in the game client that
provides developer tooling for content iteration without leaving the running
game. Toggle it with **F12**. For release builds, set `NOVAFORGE_EDITOR_TOOLS=OFF`
to compile out all editor headers, sources, and runtime objects — zero overhead
in the shipping binary.

## 2. Architecture

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
│  │  └──────────────────────────────────────────┘    │   │
│  │                                                  │   │
│  │  ┌──────────────────────────────────────────┐    │   │
│  │  │ KeybindManager + UndoStack + AIAggregator│    │   │
│  │  └──────────────────────────────────────────┘    │   │
│  └──────────────────────────────────────────────────┘   │
│  #endif                                                 │
└─────────────────────────────────────────────────────────┘
```

### Source layout

| Path | Contents |
|------|----------|
| `cpp_client/include/editor/` | Tool headers (32 header-only tools + core interfaces) |
| `cpp_client/src/editor/` | Core infrastructure implementations: `editor_command_bus.cpp`, `editor_tool_layer.cpp`, `undoable_command_bus.cpp`, `editor_event_bus.cpp`, `scene_bookmark_manager.cpp` |
| `editor/` | Standalone editor panels (`panels/`), tools (`tools/`), AI and UI subsystems |

### Lifecycle

1. **Construction** — `Application` creates `EditorToolLayer` (lightweight).
2. **init()** — Creates all panels, wires keybinds, loads saved layout.
3. **Per frame** — `draw(ctx)` renders the editor overlay when active.
4. **handleKeyPress()** — Forwards keys from the client input system.
5. **shutdown()** — Tears down panels before UI context destruction.

## 3. Core Infrastructure Classes

| Class | Header | Purpose |
|-------|--------|---------|
| `ITool` | `editor/itool.h` | Base interface — `Name()`, `Activate()`, `Deactivate()`, `Update()`, `IsActive()` |
| `ICommand` | `editor/editor_command_bus.h` | Fire-and-forget command interface |
| `IUndoableCommand` | `editor/undoable_command_bus.h` | Undo/redo command interface |
| `EditorCommandBus` | `editor/editor_command_bus.h` | FIFO command queue with `post()` / `process()` |
| `UndoableCommandBus` | `editor/undoable_command_bus.h` | Command bus with undo/redo stack |
| `EditorEventBus` | `editor/editor_event_bus.h` | Publish/subscribe event system |
| `EditorToolLayer` | `editor/editor_tool_layer.h` | Main overlay — owns panels, layout, tools |
| `DeltaEditStore` | `engine/ecs/DeltaEditStore.h` | Records and persists property edits |
| `SceneBookmarkManager` | `editor/scene_bookmark_manager.h` | Save/restore camera + scene checkpoints |
| `LayerTagSystem` | `editor/layer_tag_system.h` | Entity categorization and visibility control |

All header paths are relative to `cpp_client/include/` unless otherwise noted.

## 4. Implemented Tools

The following 27 tool headers live in `cpp_client/include/editor/`:

| Header | Description |
|--------|-------------|
| `animation_editor_tool.h` | Skeletal animation editing (keyframes, blend weights) |
| `asset_stats_panel.h` | Asset hierarchy, physics load, memory stats |
| `batch_operations_tool.h` | Mass transformations across multiple assets |
| `camera_view_tool.h` | Free-fly, orbit, orthographic camera control |
| `delta_edits_merge_tool.h` | Merge multiple DeltaEdit sets |
| `edit_propagation_tool.h` | Propagate edits to similar PCG assets |
| `environment_control_tool.h` | Gravity, wind, atmosphere live control |
| `event_timeline_tool.h` | Sequenced action timelines |
| `function_assignment_tool.h` | Behavioral trigger assignment |
| `hotkey_action_manager.h` | Configurable hotkey bindings |
| `ik_rig_tool.h` | Inverse kinematics chain management |
| `layer_tag_system.h` | Entity layer/tag categorization |
| `lighting_control_tool.h` | Light property editing with presets |
| `live_edit_mode.h` | Edit while simulation runs (pause / slow-mo / full-speed) |
| `map_editor_tool.h` | Star system map editing |
| `material_shader_tool.h` | Live material/shader property editing |
| `multi_selection_manager.h` | Multi-entity selection and group operations |
| `npc_spawner_tool.h` | NPC placement and behavior configuration |
| `pcg_snapshot_manager.h` | PCG state capture and rollback |
| `prefab_library.h` | Reusable asset prefab management |
| `resource_balancer_tool.h` | Resource distribution balancing |
| `scene_bookmark_manager.h` | Camera/scene state bookmarks |
| `script_console.h` | Runtime command console with logging |
| `ship_module_editor_tool.h` | Ship module slot editing |
| `simulation_step_controller.h` | Frame-step and simulation pause |
| `snap_align_tool.h` | Grid and surface snapping |
| `visual_diff_tool.h` | Visual comparison of scene vs PCG baseline |

## 5. Editor Panels

Standalone panels in `editor/panels/`:

| Panel | Description |
|-------|-------------|
| `ViewportPanel` | 3D scene viewport |
| `PCGPreviewPanel` | Procedural generation preview |
| `GenerationStylePanel` | PCG generation style controls |
| `AssetStylePanel` | Asset visual style settings |
| `ShipArchetypePanel` | Ship archetype configuration |
| `GamePackagerPanel` | Build packaging interface |
| `CharacterSelectPanel` | Character creation / selection |
| `MissionEditorPanel` | Mission graph editor |
| `SceneGraphPanel` | Scene hierarchy browser |
| `DataBrowserPanel` | Data asset browser |
| `ModuleEditorPanel` | Ship module editor |
| `NPCEditorPanel` | NPC authoring |
| `GalaxyMapPanel` | Galaxy/universe map |
| `FleetFormationPanel` | Fleet formation layout |
| `LiveSceneManager` | Live scene session management |
| `AssetPalettePanel` | Asset palette / drag-and-drop |
| `PhysicsTunerPanel` | Physics parameter tuning |

## 6. Build & Usage

### CMake flags

| Flag | Default | Effect |
|------|---------|--------|
| `NOVAFORGE_EDITOR_TOOLS` | `OFF` | Compile editor tool sources into the client |
| `BUILD_ATLAS_TESTS` | `OFF` | Build the Atlas test suite |

### Build commands

```bash
# Client with editor tools (development)
make build-client-editor

# Client without editor tools (release)
make build-client

# Standalone editor
make build-editor
```

### Direct CMake

```bash
cmake .. -DNOVAFORGE_EDITOR_TOOLS=ON   # embed tools
cmake .. -DNOVAFORGE_EDITOR_TOOLS=OFF  # strip tools
```

### Runtime toggle

Press **F12** to show/hide the editor overlay at runtime (development builds only).

## 7. Default Keybindings

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

## 8. Test Coverage

All tools have corresponding test files in `atlas_tests/`. Run the suite with:

```bash
make test-engine
```

Or build with `-DBUILD_ATLAS_TESTS=ON` and run the `AtlasTests` target directly.

Editor-specific test files include:

| Test file | Covers |
|-----------|--------|
| `test_editor_tool_layer.cpp` | EditorToolLayer lifecycle and panel wiring |
| `test_editor_event_bus.cpp` | Publish/subscribe event system |
| `test_undoable_command_bus.cpp` | Undo/redo command stack |
| `test_animation_editor_tool.cpp` | Animation editor tool |
| `test_ship_module_editor_tool.cpp` | Ship module editor |
| `test_map_editor_tool.cpp` | Map editor tool |
| `test_editor_panels.cpp` | Panel registration and rendering |
| `test_editor_menubar.cpp` | Menu bar integration |
| `test_camera_view_tool.cpp` | Camera view modes |
| `test_snap_align_tool.cpp` | Snap/align operations |
| `test_multi_selection_manager.cpp` | Multi-selection logic |
| `test_scene_bookmark_manager.cpp` | Scene bookmarks |
| `test_pcg_snapshot_manager.cpp` | PCG snapshot/rollback |
| `test_delta_edits_merge_tool.cpp` | DeltaEdits merging |
| `test_edit_propagation_tool.cpp` | Edit propagation |
| `test_visual_diff_tool.cpp` | Visual diff comparisons |
| `test_event_timeline_tool.cpp` | Event timelines |
| `test_function_assignment_tool.cpp` | Function assignment |
| `test_npc_spawner_tool.cpp` | NPC spawner tool |
| `test_prefab_library.cpp` | Prefab management |
| `test_script_console.cpp` | Script console |
| `test_hotkey_action_manager.cpp` | Hotkey configuration |
| `test_simulation_step_controller.cpp` | Simulation stepping |
| `test_resource_balancer_tool.cpp` | Resource balancer |
| `test_ik_rig_tool.cpp` | IK rig tool |
| `test_environment_control_tool.cpp` | Environment controls |
| `test_lighting_control_tool.cpp` | Lighting tool |
| `test_layer_tag_system.cpp` | Layer/tag system |
| `test_batch_operations_tool.cpp` | Batch operations |
| `test_live_edit_mode.cpp` | Live edit mode |
| `test_material_shader_tool.cpp` | Material/shader editing |
| `test_asset_stats_panel.cpp` | Asset stats panel |
| `test_physics_tuner_panel.cpp` | Physics tuner panel |
| `test_asset_palette_panel.cpp` | Asset palette panel |
| `test_mission_editor.cpp` | Mission editor |
| `test_data_editors.cpp` | Data browser/editors |

## 9. Related Documentation

- [Editor Tool Layer — Design](design/editor_tool_layer.md) — Architecture, lifecycle, and build flag details.
- [Extended Tooling Features](design/extended_tooling_features.md) — Feature categories and workflow integration.
- [PCG Framework](design/pcg_framework.md) — Procedural content generation pipeline.
- [Atlas Integration](ATLAS_INTEGRATION.md) — Atlas engine integration reference.
