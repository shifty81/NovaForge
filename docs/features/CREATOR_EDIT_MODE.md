# Creator Edit Mode (F12) — Spec & Roadmap

> **Status**: Spec locked — implementation pending.  
> This document captures the agreed design for the in-game **Creator Edit Mode**
> toggled by `F12` in `atlas_client`.  Use it as the source of truth when
> implementing or reviewing each milestone below.

---

## Table of Contents

1. [Objective](#1-objective)
2. [Control Scheme](#2-control-scheme)
3. [Mode Behaviour](#3-mode-behaviour)
4. [Default Creator Layout](#4-default-creator-layout)
5. [Persistence & File Paths](#5-persistence--file-paths)
6. [Hot-Apply on Exit](#6-hot-apply-on-exit)
7. [Implementation Roadmap](#7-implementation-roadmap)
8. [Reference Files](#8-reference-files)

---

## 1. Objective

Creator Edit Mode turns the running game view into a **live content-authoring
workbench** without leaving the game binary.  The simulation is frozen while
you are in edit mode, so entity positions and state stay predictable; when you
leave, all changes are hot-applied and the sim resumes instantly.

Key goals:

- Zero round-trips to an external editor for layout/placement work.
- Every edit produces a persistent override so PCG honours it on the next
  generation.
- The centre of the screen is always the live viewport — panels never occlude
  the game world.

---

## 2. Control Scheme

### 2.1 Global

| Key | Context | Action |
|-----|---------|--------|
| `F12` | Anywhere | Toggle Creator Edit Mode on/off |
| `Tab` | **Play mode** (F12 OFF) | Open Character Screen (inventory, skills, bio, map, journal) |
| `Tab` | **Edit mode** (F12 ON) | Toggle Editor UI Interaction Mode (see §3.2) |

### 2.2 Edit Mode — Freecam (UI Mode OFF, default on entry)

| Input | Action |
|-------|--------|
| Mouse move | FPS mouse-look (ghost freecam) |
| `W A S D` | Freecam translation |
| Left click | Select entity under cursor |
| Left click on gizmo handle | Begin gizmo drag immediately |
| `W` / `E` / `R` | Gizmo mode — Translate / Rotate / Scale |
| `F12` | Exit edit mode |

### 2.3 Edit Mode — UI Interaction Mode (UI Mode ON, after pressing Tab)

| Input | Action |
|-------|--------|
| Mouse move | Normal cursor (no camera look) |
| Left click | Click panels, sliders, buttons |
| Left click + drag gizmo | Drag active gizmo handle |
| `Tab` | Return to Freecam mode |
| `F12` | Exit edit mode |

> **Design note — Tab dual purpose**: `Tab` serves two completely different
> roles depending on the current mode.  In play mode it opens the Character
> Screen; in edit mode it is a toggle between FPS freecam and free-cursor UI
> interaction.  These two behaviours are mutually exclusive — `Tab` is only
> forwarded to the Character Screen when `m_editMode == false`.

### 2.4 Entity Selection Rules

- Selection via left click is **only active in F12 edit mode**.
- When UI Interaction Mode is ON (cursor free), left click is consumed by
  panels; entity picking is suppressed.
- When a gizmo is already active and the click lands on a gizmo handle, the
  drag begins immediately (no deselect/reselect step).

---

## 3. Mode Behaviour

### 3.1 Entering Edit Mode (F12 ON)

1. Set `m_editMode = true`, `m_editorPaused = true`.
2. Capture camera state as `m_prevViewMode` (to restore on exit).
3. Switch camera to `EDITOR_FREECAM` — detached from the player entity, so
   your character/ship remains visible in 3rd person.
4. Default to **UI Mode OFF** (cursor captured, FPS look active).
5. Load `data/editor_layout_editmode.json`; fall back to the built-in Creator
   default if the file is absent, then save it.
6. Show the `EditorToolLayer` overlay with the Creator panel layout.

### 3.2 UI Interaction Toggle (Tab in Edit Mode)

```
Tab press → m_editUiMode = !m_editUiMode

m_editUiMode == true  → release cursor capture; camera look frozen
m_editUiMode == false → recapture cursor; camera look resumes
```

Per-frame camera look code must skip applying mouse deltas when
`m_editMode && m_editUiMode`.

### 3.3 Simulation Pause

While `m_editorPaused == true` the main update loop **skips**:

- World simulation tick (physics, AI, projectile, economy, …)
- Network game-state interpolation

These **still run** regardless:

- Input sampling
- Camera update (freecam)
- Atlas UI update
- Rendering (full scene + overlay)

This gives a frozen, deterministic world that you can fly around and inspect.

### 3.4 Exiting Edit Mode (F12 OFF)

1. Auto-save (see §5).
2. Apply hot-reload (see §6).
3. Restore `m_prevViewMode` camera.
4. Set `m_editMode = false`, `m_editorPaused = false`.
5. Hide the `EditorToolLayer` overlay.
6. Sim resumes on the next frame.

---

## 4. Default Creator Layout

When `data/editor_layout_editmode.json` is absent (or `Layout → Reset Edit
Layout` is chosen), the following dock tree is generated and saved.

### 4.1 Dock Ratios

| Dock | Fraction of viewport width / height |
|------|--------------------------------------|
| Left dock | **22 %** of viewport width |
| Right dock | **26 %** of viewport width |
| Bottom dock | **22 %** of viewport height |
| Centre | remaining area — **live game view** |

### 4.2 Panel Assignment

| Dock | Tabs (first tab is active on load) |
|------|-------------------------------------|
| **Left** | `SceneGraphPanel` *(active)*, `DataBrowserPanel` |
| **Right** | `AssetPalettePanel` *(active)*, `LiveSceneManager`, *(future)* `SelectionInspector` |
| **Bottom** | `PCGPreviewPanel` *(active)*, `GenerationStylePanel`, `AssetStylePanel` |
| **Centre** | *(empty — live game view)* |

> If the `EditorLayout` dock-node implementation requires at least one panel in
> every node, place a `SpacerPanel` (zero-height, transparent, non-interactive)
> in the centre node as a fallback.

### 4.3 Recovery UX

A menu item is reserved for layout recovery:

```
MenuBar → Layout → Reset Edit Layout
```

Invoking it overwrites `data/editor_layout_editmode.json` with the built-in
Creator default and reloads it immediately.

### 4.4 Layout JSON (default)

The canonical default is committed at `data/editor_layout_editmode.json`.  The
JSON schema matches `data/editor_layout.json` (same `EditorLayout::SaveToFile`
/ `LoadFromFile` format).  Key fields:

```jsonc
{
  "version": 1,
  "docks": {
    "left":   { "ratio": 0.22, "tabs": ["SceneGraphPanel", "DataBrowserPanel"],   "active": 0 },
    "right":  { "ratio": 0.26, "tabs": ["AssetPalettePanel", "LiveSceneManager"], "active": 0 },
    "bottom": { "ratio": 0.22, "tabs": ["PCGPreviewPanel", "GenerationStylePanel", "AssetStylePanel"], "active": 0 },
    "center": { "tabs": [], "active": -1, "_note": "Live game view. SpacerPanel fallback if required." }
  }
}
```

See `data/editor_layout_editmode.json` for the full committed file.

---

## 5. Persistence & File Paths

Auto-save is triggered every time edit mode is **exited** (F12 OFF).

| File | Purpose |
|------|---------|
| `data/pcg_overrides.json` | Per-entity property overrides that PCG must honour on re-generation |
| `data/delta_edits.json` | Fine-grained property delta edits (recorded by `DeltaEditStore`) |
| `data/editor_layout_editmode.json` | Current panel layout for edit mode (docks, ratios, active tabs) |

Edits saved to `pcg_overrides.json` become **PCG anchors**: the next PCG
generation pass uses them as hard constraints, so manually placed or adjusted
entities remain stable across sessions.

---

## 6. Hot-Apply on Exit

After auto-save and before sim resumes, a hot-reload pass applies the recorded
changes to the live scene without restarting:

1. Re-read `data/pcg_overrides.json` and reapply entity transforms / property
   values to all live entities (same path used by `LiveSceneManager::Populate`).
2. Rebuild any preview meshes / procedural geometry that depend on overrides
   (e.g., hangar ship placement, station prop transforms).
3. *(Phase 2+)* Re-index `data/generated/**/meta.json` via `AssetRegistry` and
   hot-swap any visual assets whose `asset_id` changed during the edit session.

The result: toggling F12 OFF immediately shows the authored state in the
running game.

---

## 7. Implementation Roadmap

Use this checklist to track progress.  Each item can become a GitHub issue.

### Milestone 1 — Edit Mode Layout (prerequisite for all testing)

- [ ] Add `data/editor_layout_editmode.json` (default Creator dock tree)
- [ ] In `EditorToolLayer::enterEditMode()`, load `editor_layout_editmode.json`
      instead of `editor_layout.json`; generate + save default if absent
- [ ] Add `Layout → Reset Edit Layout` menu item that regenerates the default
      and reloads it
- [ ] Verify centre dock stays clear (live viewport not occluded)

### Milestone 2 — Ghost Freecam + Sim Pause

- [ ] Add `m_editMode`, `m_editorPaused`, `m_editUiMode`, `m_prevViewMode`
      members to `Application`
- [ ] On F12 ON: set flags, save camera state, switch to `EDITOR_FREECAM`
- [ ] Gate simulation tick in the update loop on `!m_editorPaused`
- [ ] Implement `EDITOR_FREECAM` camera state (detached FPS freecam, ghost view)
- [ ] On F12 OFF: restore camera state, clear flags

### Milestone 3 — Tab Dual-Purpose Routing

- [ ] In key callback: if `m_editMode` → route `Tab` to toggle `m_editUiMode`
      and cursor capture; else → route `Tab` to Character Screen stub
- [ ] Implement cursor capture / release logic for the UI mode toggle
- [ ] Skip mouse-look deltas per frame when `m_editMode && m_editUiMode`

### Milestone 4 — In-World Selection + Gizmo Drag

- [ ] Ray-cast pick from camera through cursor (CPU AABB or bounding sphere)
- [ ] Left-click select: active only when `m_editMode && !m_editUiMode`
- [ ] Store `selectedEntityId`; notify `EditorToolLayer::SetSelectedEntity(id)`
- [ ] If click lands on gizmo handle → start drag immediately (no deselect step)
- [ ] Show context-sensitive inspector panels for the selected entity's
      component set (Transform always; NPC / Ship / Station sections
      conditionally)

### Milestone 5 — Auto-Save + Hot-Apply on Exit

- [ ] Call save pipeline automatically on F12 OFF
      (`pcg_overrides.json`, `delta_edits.json`, `editor_layout_editmode.json`)
- [ ] Implement hot-apply step: reapply overrides to live scene entities
- [ ] Validate that toggling F12 OFF → ON keeps authored changes

### Milestone 6 — Generated Asset Integration (Phase 2)

- [ ] Extend `AssetRegistry` to index `data/generated/**/meta.json`
- [ ] Add "Visual" section to Inspector: `visual_type`, `asset_id` drop-down,
      "Apply" button
- [ ] Hot-swap visual asset on Apply (GLB first, OBJ fallback)
- [ ] Smoke-test with Blender kitbash catalog

### Milestone 7 — Character Screen (Play Mode Tab)

- [ ] Stub `CharacterScreen` UI (opens on `Tab` in play mode)
- [ ] Implement inventory tab
- [ ] Implement skills tab
- [ ] Implement bio / character info tab
- [ ] Implement map tab
- [ ] Implement journal / quest log tab

---

## 8. Reference Files

The following existing files are the primary touchpoints for implementation.
No changes to these files are required by this documentation PR.

| File | Relevance |
|------|-----------|
| `cpp_client/src/core/application.cpp` | F12 key handler; location for `m_editMode` flags and Tab routing |
| `cpp_client/src/core/application_rendering.cpp` | Overlay render pass; must remain active when `m_editorPaused` |
| `cpp_client/src/editor/editor_tool_layer.cpp` | `init()` loads layout; `saveOverrides()` / `saveDeltaEdits()` pipeline; target for `enterEditMode()` / `exitEditMode()` |
| `editor/ui/EditorLayout.h` / `.cpp` | `LoadFromFile`, `SaveToFile`, dock-node tree, `DrawNode()` |
| `data/editor_layout_editmode.json` | Default Creator layout committed in this PR |
| `data/pcg_overrides.json` | PCG anchor overrides (written on edit mode exit) |
| `data/delta_edits.json` | Fine-grained property deltas (written on edit mode exit) |
