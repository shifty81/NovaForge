# NovaForge Extended ToolingLayer Feature List

> Comprehensive developer reference for all editor panels, live-edit workflows,
> hotkeys, physics/animation tools, PCG/DeltaEdits utilities, and debugging
> features available through the ToolingLayer.
>
> See also: [Editor Tool Layer — Design Document](editor_tool_layer.md)

## 1️⃣ Asset & Scene Management

| Feature | Description | Example / Notes |
|---------|-------------|-----------------|
| Multi-Selection & Group Editing | Move, rotate, scale multiple assets at once | Ship fleets, multiple props in hangar |
| Layer & Tag System | Categorize assets (Characters, Ships, Props, Hangar, PCG type) | Toggle visibility to reduce clutter |
| Snap & Align Tools | Grid snap, surface snap, align to other assets | Precision placement for modules, props |
| Prefab / Asset Library | Drag-and-drop pre-made modules or props | Store edited objects as reusable prefabs |
| Scene Bookmarking / Checkpoints | Save camera view and asset state | Quick return during complex edits |

## 2️⃣ Physics & Animation Tools

| Feature | Description | Example / Notes |
|---------|-------------|-----------------|
| Dynamic Physics Tuning | Adjust rigidbodies, joints, colliders, cloth parameters live | Gravity, drag, wind influence per asset |
| Animation Layer Editing | Edit multiple animation layers simultaneously | Blend procedural cape physics with keyframe animations |
| IK / Rigging Tools | Adjust inverse kinematics for characters, rigs, turrets | Test procedural animation responses in real-time |
| Simulation Freeze / Step Mode | Pause simulation mid-frame, step frame by frame | Fine-tune physics & animation interaction |

## 3️⃣ Function & Behavior Editing

| Feature | Description | Example / Notes |
|---------|-------------|-----------------|
| Function / Event Assignment | Assign triggers to props, ships, consoles | Door opens, turret fires, lights flicker |
| Event Timeline Editor | Create sequences of actions | Spawn → Animation → Physics → Trigger |
| AI / PCG Asset Preview | Prompt AI to spawn new assets in scene | Categorized by style: planets, stations, ships, props |

## 4️⃣ PCG & DeltaEdits Utilities

| Feature | Description | Example / Notes |
|---------|-------------|-----------------|
| PCG Snapshot / Rollback | Capture procedural scene before edits | Revert if edits break layout or gameplay |
| DeltaEdits Merge & Conflict Resolution | Combine multiple DeltaEdits sets | Collaborative editing workflow |
| Edit Propagation | Apply change to one asset and propagate to similar assets | Move a ship engine module → all ships update |
| Visual Diff / Comparison | Compare current scene vs baseline PCG | See all modifications visually before committing |

## 5️⃣ Utility & Debugging

| Feature | Description | Example / Notes |
|---------|-------------|-----------------|
| Camera & View Tools | Free-fly, orbit, orthographic views | Precise asset placement |
| Asset Stats Panel | Show hierarchy, physics load, memory usage | Useful for optimization |
| Hotkey / Quick Actions | Assign common tools to keys | Move, rotate, snap, freeze physics, open panels |
| Logging & Preview Console | Real-time physics, collisions, DeltaEdits feedback | Debug live asset behavior |
| Batch Operations | Apply transformations or function assignments to multiple assets | Useful for mass edits |

## 6️⃣ Bonus Developer-Friendly Features

| Feature | Description | Notes |
|---------|-------------|-------|
| Edit While Playing | Tweak assets without fully pausing gameplay | Useful for gameplay testing |
| Undo / Redo Stack | Unlimited undo/redo for all editing actions | Critical for large scenes |
| Environment Simulation Control | Change gravity, wind, atmosphere live | Test ships, rigs, physics interactions |
| Live Material / Shader Editing | Adjust material, color, emission in editor | Instant visual feedback |
| Modular UI Panels | Dock, collapse, or hide panels | Customize workspace for workflow |

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
