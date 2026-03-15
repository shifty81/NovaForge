# AtlasForge Generator Implementation Plan

Comprehensive implementation roadmap for evolving this addon
into the full **AtlasForge Generator** — a seed-deterministic, batch-ready
procedural content generation (PCG) pipeline that can be integrated into
any game project requiring procedural 3D assets.

> This plan represents the direction for the project as an engine-agnostic
> PCG tool that can be merged and adapted to specific projects.

---

## Table of Contents

1. [Vision](#1-vision)
2. [Current State](#2-current-state)
3. [Phase 1 — Full PCG Pipeline (Python + Blender)](#3-phase-1--full-pcg-pipeline-python--blender)
4. [Phase 2 — C++ Integration Layer](#4-phase-2--c-integration-layer)
5. [Phase 3 — Unified Material System](#5-phase-3--unified-material-system)
6. [Phase 4 — ECS Core & Simulation](#6-phase-4--ecs-core--simulation)
7. [Phase 5 — Multiplayer & Persistence](#7-phase-5--multiplayer--persistence)
8. [Phase 6 — Environmental Propagation & Debug Tools](#8-phase-6--environmental-propagation--debug-tools)
9. [Phase 7 — Addon Update System](#9-phase-7--addon-update-system)
10. [Directory Structure (Target)](#10-directory-structure-target)
11. [Integration Points with Projects](#11-integration-points-with-projects)

---

## 1. Vision

This addon already handles procedural ship, station, and
asteroid generation.  The next evolution turns it into the **AtlasForge
Generator** — a single-command pipeline that can produce an entire playable
universe of assets:

```
Galaxy → Systems → Planets → Stations → Ships → Characters
```

Every object is seed-deterministic and JSON-linked so any game engine can
place, simulate, and modify assets at runtime. The tool is designed to be
merged into specific projects and customized for their needs.

---

## 2. Current State

### ✅ Implemented (Blender Addon)

- 18 ship classes (Shuttle → Titan, Industrial, Mining, Exotic, etc.)
- 9-stage spine-first assembly pipeline
- Brick taxonomy (18 types, 5 categories) with Ship DNA export
- Engine archetypes, turret system, hull taper & cleanup passes
- Interior generation (cockpit, bridge, corridors, quarters, cargo, engine room)
- Module system (6 types, progressive availability)
- 4 faction styles (Solari, Veyren, Aurelian, Keldari) + game-inspired styles
- Procedural PBR textures with weathering
- Station generation (8 types, 4 faction architectures)
- Asteroid belt generation (16 ore types, 4 layouts)
- Project JSON import and OBJ export for game engines
- LOD generation, collision meshes, animation system
- Damage propagation, power flow simulation, build validator

### 🔲 Not Yet Implemented

- ~~Galaxy / system / planet generators (Python)~~ ✓ Implemented (pcg_pipeline)
- ~~Character mesh generation~~ ✓ Implemented (character_generator)
- C++ procedural hull meshing integration (pybind11)
- Blender ↔ C++ texture pipeline
- Unified material system (solid / liquid / gas)
- Terraforming engine framework
- ECS core implementation
- Multithreaded job scheduler
- Deterministic multiplayer sync
- Save / load state serialization
- Galaxy-wide environmental propagation
- Addon update system

## 3. Phase 1 — Full PCG Pipeline (Python + Blender)

**Goal:** Add Python generator modules so a single batch script can create an
entire galaxy of assets.

### New Modules

| Module | Purpose |
|--------|---------|
| `generators/galaxy_generator.py` | Seed-based galaxy with N systems |
| `generators/system_generator.py` | Stars, planets, station slots, ship slots per system |
| `generators/planet_generator.py` | Planet type, size, orbital params, biome |
| `generators/station_generator.py` | Station type, modules, hardpoints (wraps existing `station_generator.py`) |
| `generators/ship_generator_batch.py` | Headless Blender ship generation (wraps existing `ship_generator.py`) |
| `generators/character_generator.py` | Race, body type, cyber-limb procedural characters |

### Batch Script

`scripts/batch_generate.py` — single entry point:

```bash
python scripts/batch_generate.py --seed 987654 --systems 5
```

Produces:

```
build/
├── galaxy.json
├── systems/   (per-system JSON)
├── planets/   (per-planet JSON)
├── stations/  (OBJ + metadata JSON)
├── ships/     (OBJ + metadata JSON + LODs)
└── characters/ (metadata JSON)
```

### Data Flow

```
Universe Seed
    → Galaxy Generator (N systems)
        → System Generator (stars, planets, ships)
            → Planet Generator (type, size, biome, stations)
            → Station Generator → Blender headless → OBJ export
            → Ship Generator → Blender headless → OBJ export + LODs
```

---

## 4. Phase 2 — C++ Integration Layer

**Goal:** Bridge Blender Python and the C++ Atlas engine for runtime mesh gen.

| Component | Description |
|-----------|-------------|
| `pybind11` wrapper | Expose C++ hull meshing to Python |
| PCG style hooks | `pcg_asset_style.h` deformation functions |
| Texture pipeline | Blender material → C++ PBR params |

---

## 5. Phase 3 — Unified Material System

**Goal:** Shared material/substance definitions across Blender and C++.

---

## 6. Phase 4 — ECS Core & Simulation

**Goal:** Port brick simulation to the Atlas ECS.

---

## 7. Phase 5 — Multiplayer & Persistence

**Goal:** Deterministic sync and save/load for generated content.

---

## 8. Phase 6 — Environmental Propagation & Debug Tools

### Galaxy-Wide Propagation

Each star system stores `SystemEnvironment { heat, pollution, radiation,
stability }`.  Changes propagate between planets and systems creating
emergent feedback loops (economy, warfare, migration).

---

## 9. Phase 7 — Addon Update System ✅ COMPLETE

**Goal:** Keep the Blender addon evergreen as projects evolve.

| Feature | Description | Status |
|---------|-------------|--------|
| **Auto-import templates** | Load new JSON/YAML templates from a central directory | ✅ `template_manager.py` |
| **Versioned generators** | Each generator module carries a version | ✅ `version_registry.py` |
| **Manual override protection** | Objects with `af_manual_override = True` are skipped by regeneration | ✅ `override_manager.py` |

---

## 10. Directory Structure (Target)

When fully implemented, the repository will look like:

```
AtlasForgeGenerator/
├── __init__.py                   # Blender addon entry point
├── ship_generator.py             # (existing) ship generation
├── ship_parts.py                 # (existing) ship components
├── interior_generator.py         # (existing) interior generation
├── module_system.py              # (existing) module system
├── brick_system.py               # (existing) brick taxonomy & Ship DNA
├── atlas_exporter.py             # (existing) JSON import + OBJ export
├── station_generator.py          # (existing) station generation
├── asteroid_generator.py         # (existing) asteroid belts
├── texture_generator.py          # (existing) procedural PBR materials
├── novaforge_importer.py         # (existing) project JSON import
├── render_setup.py               # (existing) catalog render setup
├── lod_generator.py              # (existing) LOD generation
├── collision_generator.py        # (existing) collision meshes
├── animation_system.py           # (existing) ship animations
├── damage_system.py              # (existing) damage propagation
├── power_system.py               # (existing) power flow simulation
├── build_validator.py            # (existing) build validation
├── pcg_panel.py                  # (existing) PCG pipeline Blender panel
├── pcg_pipeline/                 # (existing) batch PCG pipeline modules
│   ├── __init__.py
│   ├── galaxy_generator.py
│   ├── system_generator.py
│   ├── planet_generator.py
│   ├── terrain_generator.py
│   ├── character_generator.py
│   └── batch_generate.py
├── density_field.py              # (existing) procedural geometry computation
├── slot_grid.py                  # (existing) modular slot placement
├── traversal_system.py           # (existing) interior navigation
├── fleet_logistics.py            # (existing) fleet-level logistics
├── rig_system.py                 # (existing) character rigging
├── lighting_system.py            # interior/exterior ship lighting
├── greeble_system.py             # surface detail pass (panels, vents, pipes)
├── preset_library.py             # save/load generation presets (JSON)
├── README.md
├── USAGE.md
├── EXAMPLES.md
├── TECHNICAL.md
├── ENGINE_INTEGRATION.md
├── NOVAFORGE_GUIDE.md
├── IMPLEMENTATION_SUMMARY.md
├── NOVAFORGE_PLAN.md             # This file
├── features.md
├── test_validation.py
└── test_addon.py
```

---

## 11. Integration Points with Projects

This generator is designed to be merged into any game project requiring
procedural 3D assets.  Key integration touchpoints:

| Project Path | Generator Output |
|----------------|-----------------|
| `data/ships/*.json` | Ship metadata consumed by `novaforge_importer.py` |
| `data/ships/obj_models/` | OBJ meshes exported by `atlas_exporter.py` |
| `include/pcg/hull_mesher.h` | C++ hull meshing (Phase 2 pybind11) |
| `include/pcg/pcg_asset_style.h` | Style & deformation hooks |
| `schemas/atlas.build.v1.json` | Build manifest validation |

### Workflow

1. Merge this repository as a tool into your project.
2. Define or update ship/station/planet JSON in your project's `data/` directory.
3. Run the batch generator to produce OBJ meshes, textures, LODs, and metadata.
4. Place outputs into your project's asset directories.
5. Your game engine loads assets at runtime using the JSON metadata for
   placement, simulation, and rendering.

---

*This plan is a living document.  Update it as phases are completed and new
requirements emerge.*
