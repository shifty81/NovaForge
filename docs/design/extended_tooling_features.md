# NovaForge Extended ToolingLayer Feature List

> Comprehensive developer reference for all editor panels, live-edit workflows,
> hotkeys, physics/animation tools, PCG/DeltaEdits utilities, and debugging
> features available through the ToolingLayer.
>
> See also: [Editor Tool Layer — Design Document](editor_tool_layer.md) ·
> [Editor Tools Reference](../EDITOR_TOOLS.md)

## 1️⃣ Asset & Scene Management

| Feature | Description | Status | Header |
|---------|-------------|--------|--------|
| Multi-Selection & Group Editing | Move, rotate, scale multiple assets at once | ✅ | `multi_selection_manager.h` |
| Layer & Tag System | Categorize assets by type; toggle visibility | ✅ | `layer_tag_system.h` |
| Snap & Align Tools | Grid snap, surface snap, align to other assets | ✅ | `snap_align_tool.h` |
| Prefab / Asset Library | Drag-and-drop pre-made modules or props | ✅ | `prefab_library.h` |
| Scene Bookmarking / Checkpoints | Save camera view and asset state | ✅ | `scene_bookmark_manager.h` |

## 2️⃣ Physics & Animation Tools

| Feature | Description | Status | Header |
|---------|-------------|--------|--------|
| Dynamic Physics Tuning | Adjust rigidbodies, joints, colliders, cloth parameters live | ✅ | `PhysicsTunerPanel` (editor) |
| Animation Layer Editing | Edit multiple animation layers simultaneously | ✅ | `animation_editor_tool.h` |
| IK / Rigging Tools | Adjust inverse kinematics for characters, rigs, turrets | ✅ | `ik_rig_tool.h` |
| Simulation Freeze / Step Mode | Pause simulation mid-frame, step frame by frame | ✅ | `simulation_step_controller.h` |

## 3️⃣ Function & Behavior Editing

| Feature | Description | Status | Header |
|---------|-------------|--------|--------|
| Function / Event Assignment | Assign triggers to props, ships, consoles | ✅ | `function_assignment_tool.h` |
| Event Timeline Editor | Create sequences of actions | ✅ | `event_timeline_tool.h` |
| AI / PCG Asset Preview | Prompt AI to spawn new assets in scene | ✅ | `npc_spawner_tool.h` |

## 4️⃣ PCG & DeltaEdits Utilities

| Feature | Description | Status | Header |
|---------|-------------|--------|--------|
| PCG Snapshot / Rollback | Capture procedural scene before edits | ✅ | `pcg_snapshot_manager.h` |
| DeltaEdits Merge & Conflict Resolution | Combine multiple DeltaEdits sets | ✅ | `delta_edits_merge_tool.h` |
| Edit Propagation | Apply change to one asset and propagate to similar assets | ✅ | `edit_propagation_tool.h` |
| Visual Diff / Comparison | Compare current scene vs baseline PCG | ✅ | `visual_diff_tool.h` |

## 5️⃣ Utility & Debugging

| Feature | Description | Status | Header |
|---------|-------------|--------|--------|
| Camera & View Tools | Free-fly, orbit, orthographic views | ✅ | `camera_view_tool.h` |
| Asset Stats Panel | Show hierarchy, physics load, memory usage | ✅ | `asset_stats_panel.h` |
| Hotkey / Quick Actions | Assign common tools to keys | ✅ | `hotkey_action_manager.h` |
| Logging & Preview Console | Real-time physics, collisions, DeltaEdits feedback | ✅ | `script_console.h` |
| Batch Operations | Apply transformations or function assignments to multiple assets | ✅ | `batch_operations_tool.h` |

## 6️⃣ Advanced Workflow Features

| Feature | Description | Status | Header |
|---------|-------------|--------|--------|
| Edit While Playing | Tweak assets without fully pausing gameplay | ✅ | `live_edit_mode.h` |
| Undo / Redo Stack | Unlimited undo/redo for all editing actions | ✅ | `undoable_command_bus.h` |
| Environment Simulation Control | Change gravity, wind, atmosphere live | ✅ | `environment_control_tool.h` |
| Live Material / Shader Editing | Adjust material, color, emission in editor | ✅ | `material_shader_tool.h` |
| Resource Balancing | Resource distribution across entities | ✅ | `resource_balancer_tool.h` |
| Lighting Control | Light property editing with presets | ✅ | `lighting_control_tool.h` |
| Map Editing | Star system map editing | ✅ | `map_editor_tool.h` |
| Ship Module Editing | Ship module slot editing | ✅ | `ship_module_editor_tool.h` |

## 7️⃣ Workflow Integration

**Play → Edit → Save → PCG Baseline Cycle:**

1. Spawn Player & Scene (baseline PCG + DeltaEdits applied)
2. Pause Gameplay → Detach Equipment / Modules → Hover Mode
3. Open ToolingLayer → Select Asset / Category
4. Edit Transform / Placement, Physics, Functions, Animations
5. Live Preview / Simulation → Test Interactions
6. Apply / Commit DeltaEdits → Save modifications persistently
7. Close ToolingLayer → Gameplay resumes with changes
8. PCG Regeneration / Next Pull → DeltaEdits applied automatically

## Notes

- All assets are editable: Characters, Ships, Modules, Props, Hangar Interiors, Animations, Rigs
- DeltaEdits ensures changes persist across PCG generations
- ToolingLayer supports live feedback and precise control for fast iteration
- Hotkeys, batch operations, and layer/tag systems optimize workflow for large scenes
