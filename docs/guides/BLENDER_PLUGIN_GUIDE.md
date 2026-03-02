# Blender Spaceship Generator — Usage Guide

**Addon location:** `tools/BlenderSpaceshipGenerator/`
**Blender version:** 3.0+ recommended (minimum 2.80)
**No external dependencies required**

---

## Table of Contents

1. [Installation](#1-installation)
2. [Quick Start — Generate Your First Ship](#2-quick-start--generate-your-first-ship)
3. [Panel Reference](#3-panel-reference)
4. [Ship Classes](#4-ship-classes)
5. [Design Styles & Factions](#5-design-styles--factions)
6. [Generating Stations](#6-generating-stations)
7. [Generating Asteroid Belts](#7-generating-asteroid-belts)
8. [Importing Ships from Nova Forge JSON](#8-importing-ships-from-nova-forge-json)
9. [Exporting OBJ Models for the Atlas Engine](#9-exporting-obj-models-for-the-atlas-engine)
10. [Ship DNA Export & Reproducibility](#10-ship-dna-export--reproducibility)
11. [Batch Generation (Headless)](#11-batch-generation-headless)
12. [PCG Pipeline — Universe Generation](#12-pcg-pipeline--universe-generation)
13. [Interior Exploration (Walk Mode)](#13-interior-exploration-walk-mode)
14. [Scripting API](#14-scripting-api)
15. [Troubleshooting](#15-troubleshooting)

---

## 1. Installation

### Step 1 — Locate the addon

The addon lives inside the Nova Forge repository:

```
NovaForge/
└── tools/
    └── BlenderSpaceshipGenerator/
        ├── __init__.py          ← main addon entry point
        ├── ship_generator.py
        ├── ship_parts.py
        ├── interior_generator.py
        ├── module_system.py
        ├── brick_system.py
        ├── atlas_exporter.py
        ├── station_generator.py
        ├── asteroid_generator.py
        ├── texture_generator.py
        └── pcg_pipeline/        ← standalone universe generator
```

### Step 2 — Install in Blender

1. Open **Blender** (3.0+ recommended).
2. Go to **Edit → Preferences → Add-ons**.
3. Click **Install…** (top-right).
4. Navigate to `tools/BlenderSpaceshipGenerator/` and select the folder (or a ZIP of it).
5. Click **Install Add-on**.

### Step 3 — Enable

1. In the Add-ons list search for **"Spaceship Generator"**.
2. Check the checkbox to enable it.

### Step 4 — Verify

1. In the 3D Viewport press **N** to open the sidebar.
2. Click the **Spaceship** tab.
3. You should see the full panel with Ship Configuration, Generation Options,
   Nova Forge / Atlas Integration, Station Generation, Asteroid Belt
   Generation, and Ship DNA Export sections.

---

## 2. Quick Start — Generate Your First Ship

1. Open the **Spaceship** sidebar panel (press **N** in the 3D Viewport).
2. Set **Ship Class** to **Fighter**.
3. Set **Style** to **Mixed** (or pick a Nova Forge faction).
4. Leave **Random Seed** at `1`.
5. Click **Generate Spaceship**.
6. The ship appears at the 3D cursor location.

**Navigate around the ship:**

| Action | Input |
|--------|-------|
| Orbit | Middle-mouse drag |
| Zoom | Scroll wheel |
| Pan | Shift + Middle-mouse drag |

To generate a **different** version of the same class, change the **Random Seed** and click **Generate Spaceship** again.

---

## 3. Panel Reference

The sidebar panel (3D Viewport → **N** → **Spaceship** tab) exposes these controls:

### Ship Configuration

| Setting | Description | Default |
|---------|-------------|---------|
| **Ship Class** | Size/type of vessel (Shuttle → Titan, plus Industrial, Mining Barge, Exhumer, Explorer, Hauler, Exotic) | Fighter |
| **Style** | Visual design language (see [§5](#5-design-styles--factions)) | Mixed |
| **Random Seed** | Integer seed for procedural variation — same seed = same ship | 1 |

### Generation Options

| Setting | Description | Default |
|---------|-------------|---------|
| **Generate Interior** | Create FPV-ready rooms and corridors inside the ship | Enabled |
| **Module Slots** | Number of additional modules (0–10) — more slots = larger ship | 2 |
| **Hull Complexity** | Geometry detail multiplier (0.1–3.0) | 1.0 |
| **Symmetry** | Mirror the design left/right | Enabled |

### Naming & Hardpoints

| Setting | Description | Default |
|---------|-------------|---------|
| **Naming Prefix** | String prepended to all generated Blender objects (e.g. `NOVAFORGE`) | (empty) |
| **Turret Hardpoints** | Number of visual turrets with base, ring, and barrel (0–10) | 0 |

### Hull Shaping

| Setting | Description | Default |
|---------|-------------|---------|
| **Hull Taper** | Silhouette taper factor (0.5 = aggressive taper, 1.0 = no taper) | 0.85 |

### Texture Options

| Setting | Description | Default |
|---------|-------------|---------|
| **Generate Textures** | Apply procedural PBR materials (noise panels, voronoi plating) | Enabled |
| **Weathering** | Dirt and wear intensity (0.0–1.0) — only visible when textures are enabled | 0.0 |

### Nova Forge / Atlas Integration

| Control | Purpose |
|---------|---------|
| **Ship JSON** | Path to a Nova Forge `data/ships/*.json` file |
| **Import from Nova Forge JSON** | Parse the JSON and generate matching ships |
| **Export Path** | Directory for OBJ output |
| **Export OBJ for Atlas** | Export the selected ship as `.obj` + `.mtl` with correct Atlas axis orientation |

### Station Generation

| Control | Options |
|---------|---------|
| **Station Type** | Industrial, Military, Commercial, Research, Mining, Astrahus, Fortizar, Keepstar |
| **Station Faction** | Solari, Veyren, Aurelian, Keldari |
| **Generate Station** | Create the station at the 3D cursor |

### Asteroid Belt Generation

| Control | Options |
|---------|---------|
| **Belt Layout** | Semicircle, Sphere, Cluster, Ring |
| **Primary Ore** | 16 ore types (Dustite → Nexorite) |
| **Asteroid Count** | 5–200 |
| **Generate Asteroid Belt** | Create the belt at the 3D cursor |

### Ship DNA Export

| Control | Purpose |
|---------|---------|
| **Ship DNA Path** | File path for `.json` DNA export |
| **Export Ship DNA** | Save the selected ship's DNA to a JSON file |

---

## 4. Ship Classes

| Class | Scale | Engines | Weapons | Turrets | Wings | Crew | Interior |
|-------|-------|---------|---------|---------|-------|------|----------|
| Shuttle | 1.0 | 2 | 0 | 0 | No | 2 | Cockpit only |
| Fighter | 1.5 | 2 | 2 | 1 | Yes | 1 | Cockpit only |
| Corvette | 3.0 | 3 | 4 | 2 | Yes | 4 | Cockpit + corridor + quarters |
| Frigate | 5.0 | 4 | 6 | 3 | No | 10 | Cockpit + corridor + quarters |
| Destroyer | 8.0 | 4 | 8 | 4 | No | 25 | Bridge + corridors + cargo + engine room |
| Cruiser | 12.0 | 6 | 12 | 6 | No | 50 | Full multi-deck interior |
| Battlecruiser | 15.0 | 6 | 14 | 7 | No | 75 | Full multi-deck interior |
| Battleship | 18.0 | 8 | 16 | 8 | No | 100 | Full multi-deck interior |
| Carrier | 25.0 | 10 | 10 | 6 | No | 200 | Full interior + hangars |
| Dreadnought | 30.0 | 5 | 18 | 10 | No | 400 | Full interior |
| Capital | 35.0 | 12 | 20 | 10 | No | 500 | Full interior |
| Titan | 50.0 | 10 | 24 | 10 | No | 1000 | Full interior |
| Industrial | 6.0 | 3 | 1 | 0 | No | 5 | Cockpit + corridor + quarters |
| Mining Barge | 4.0 | 2 | 0 | 0 | No | 3 | Cockpit + corridor + quarters |
| Exhumer | 5.0 | 3 | 0 | 0 | No | 4 | Cockpit + corridor + quarters |
| Explorer | 2.0 | 2 | 1 | 1 | Yes | 1 | Cockpit only |
| Hauler | 5.5 | 4 | 0 | 0 | No | 2 | Cockpit + corridor + quarters |
| Exotic | 2.5 | 2 | 2 | 1 | Yes | 1 | Cockpit only |

---

## 5. Design Styles & Factions

### Game-Inspired Styles

| Style | Character | Hull Treatment |
|-------|-----------|---------------|
| **Mixed** | Balanced blend of all styles | Light bevel |
| **X4** | Angular, geometric, industrial | Sharp bevels, hard edges |
| **Elite** | Sleek, aerodynamic | Tapered shapes |
| **Eve** | Organic, flowing curves | Spherical cast modifier |
| **NMS** | Colorful, varied | Rounded + bevel; per-seed random colors |

### Nova Forge Factions

| Faction | Visual Style | Tank Focus | Hull Modifier |
|---------|-------------|------------|---------------|
| **Solari** | Golden, elegant | Armor | Smooth bevel + light cast |
| **Veyren** | Angular, utilitarian | Shield | Sharp bevel |
| **Aurelian** | Sleek, organic | Drones | Subdivision + spherical cast |
| **Keldari** | Rugged, industrial | Missiles | Heavy bevel |

When importing from Nova Forge JSON, the ship's `race` field is automatically mapped to the matching faction style.

---

## 6. Generating Stations

1. Scroll to the **Station Generation** section in the Spaceship panel.
2. Select a **Station Type** (Industrial, Military, Commercial, Research, Mining, Astrahus, Fortizar, Keepstar).
3. Select a **Station Faction** (Solari, Veyren, Aurelian, Keldari).
4. Set the **Random Seed**.
5. Click **Generate Station**.

### Station Types

| Type | Scale | Sections | Docking Bays | Hangars |
|------|-------|----------|-------------|---------|
| Industrial | 80 | 4 | 2 | Yes |
| Military | 100 | 5 | 3 | Yes |
| Commercial | 90 | 4 | 4 | No |
| Research | 60 | 3 | 1 | No |
| Mining | 70 | 3 | 2 | Yes |
| Astrahus | 50 | 3 | 2 | No |
| Fortizar | 100 | 5 | 4 | Yes |
| Keepstar | 200 | 8 | 8 | Yes |

### Faction Architecture

| Faction | Architecture | Features |
|---------|-------------|----------|
| Solari | Golden cathedral | Spires, domes, radial symmetry |
| Veyren | Industrial blocks | Angular, cubic symmetry |
| Aurelian | Organic domes | Spherical, organic curves |
| Keldari | Rusted patchwork | Asymmetric, scaffolding |

---

## 7. Generating Asteroid Belts

1. Scroll to the **Asteroid Belt Generation** section.
2. Select a **Belt Layout** (Semicircle, Sphere, Cluster, Ring).
3. Select a **Primary Ore** type (16 types from Dustite to Nexorite).
4. Set the **Asteroid Count** (5–200).
5. Set the **Random Seed**.
6. Click **Generate Asteroid Belt**.

Each asteroid receives:
- Procedural deformation (icosphere + displacement) for natural rocky shapes
- Random rotation and size variation
- PBR material matching the ore's color, roughness, and metallic values

### Ore Types

| Ore | Color | Security Level |
|-----|-------|---------------|
| Dustite | Brown-orange | Highsec |
| Ferrite | Gray metallic | Highsec |
| Ignaite | Red-brown | Highsec |
| Crystite | Green crystalline | Highsec |
| Shadite | Golden-brown | Highsec/Lowsec |
| Corite | Blue-cyan | Highsec/Lowsec |
| Lumine | Dark red | Lowsec |
| Sangite | Bright red | Lowsec |
| Glacite | Golden | Lowsec |
| Densite | Light gray | Lowsec/Nullsec |
| Voidite | Dark brown | Nullsec |
| Spodumain | Silvery | Nullsec |
| Pyranite | Purple | Nullsec |
| Stellite | Green luminescent | Nullsec |
| Cosmite | Orange-gold | Nullsec |
| Nexorite | Cyan radioactive | Nullsec |

---

## 8. Importing Ships from Nova Forge JSON

Instead of configuring each ship manually, you can import ships directly from
Nova Forge's game data files.

### Step 1 — Locate the JSON

Nova Forge ship definitions live in:

```
NovaForge/data/ships/
├── frigates.json
├── destroyers.json
├── cruisers.json
├── battlecruisers.json
├── battleships.json
├── capitals.json
├── industrials.json
├── mining_barges.json
├── exhumers.json
├── tech2_frigates.json
├── tech2_destroyers.json
├── tech2_cruisers.json
├── tech2_battlecruisers.json
└── tech2_battleships.json
```

### Step 2 — Set the JSON path

In the **Nova Forge / Atlas Integration** section, click the folder icon next to **Ship JSON** and browse to one of the files above.

### Step 3 — Import

Click **Import from Nova Forge JSON**.

The addon will:
- Parse every ship definition in the file.
- Map the `race` field → faction style (Solari, Veyren, Aurelian, Keldari).
- Use the `generation_seed` from `model_data` for consistent generation.
- Map the ship `class` → generator ship class.
- Calculate module slots from `high_slots + mid_slots + low_slots`.

### Field Mapping

| Nova Forge JSON field | Generator parameter |
|-----------------------|---------------------|
| `class` (Frigate, Titan, etc.) | `ship_class` |
| `race` (Solari, Veyren, etc.) | `style` |
| `model_data.generation_seed` | `seed` |
| `model_data.turret_hardpoints` | turret count |
| `model_data.engine_count` | engine count |
| `high_slots + mid_slots + low_slots` | `module_slots` (÷3, max 10) |

---

## 9. Exporting OBJ Models for the Atlas Engine

### Step 1 — Select the ship

Click the **Hull** object of the ship you want to export (the parent object in the collection).

### Step 2 — Set export path

In the **Nova Forge / Atlas Integration** section, set **Export Path** to:

```
NovaForge/data/ships/obj_models/
```

### Step 3 — Export

Click **Export OBJ for Atlas**.

This will:
- Select the hull and all child objects (cockpit, engines, weapons, modules, interior).
- Apply all modifiers.
- Export as `.obj` with matching `.mtl` file.
- Use the correct axis orientation for Atlas (forward: −Z, up: Y).

### Step 4 — Rename for Atlas

For the Atlas engine to find the model, the file name must match the ship `id` from the JSON:

```bash
mv Hull.obj fang.obj
mv Hull.mtl fang.mtl
```

Or rename the hull object in Blender before exporting (double-click the object name in the Outliner).

### Verify

```bash
ls NovaForge/data/ships/obj_models/
# Should see:  fang.obj  fang.mtl
```

---

## 10. Ship DNA Export & Reproducibility

Every generated ship stores its **Ship DNA** as a JSON custom property on the hull object. Ship DNA records the seed, class, style, grid size, and every placed brick — enabling exact reproduction.

### Export Ship DNA

1. Select the hull object.
2. In the **Ship DNA Export** section, set a `.json` file path.
3. Click **Export Ship DNA**.

### Ship DNA Format

```json
{
  "seed": 917221,
  "class": "CRUISER",
  "style": "SOLARI",
  "naming_prefix": "NOVAFORGE",
  "grid_size": 2.0,
  "bricks": [
    { "type": "STRUCTURAL_SPINE", "pos": [0, 0, 0] },
    { "type": "ENGINE_BLOCK", "pos": [0, -4, 0], "archetype": "MAIN_THRUST" }
  ]
}
```

### What Ship DNA enables

- **Reproducibility** — same DNA → identical ship.
- **Save/Load** — store designs as files.
- **NPC generation** — NPCs use the same brick system as players.
- **Damage simulation** — destroying bricks removes entries from the DNA.
- **Salvage** — dropped bricks reference real brick types.
- **Sharing** — exchange ship designs between players.

---

## 11. Batch Generation (Headless)

To generate and export every ship in Nova Forge data at once without opening the Blender GUI:

### From the command line

Save this script as `batch_generate.py` (or use the one already in `tools/BlenderSpaceshipGenerator/pcg_pipeline/batch_generate.py`):

```python
import bpy
import os
from BlenderSpaceshipGenerator import ship_generator, atlas_exporter

NOVAFORGE_DIR = '/path/to/NovaForge'
SHIPS_DIR = os.path.join(NOVAFORGE_DIR, 'data', 'ships')
EXPORT_DIR = os.path.join(NOVAFORGE_DIR, 'data', 'ships', 'obj_models')
os.makedirs(EXPORT_DIR, exist_ok=True)

all_ships = atlas_exporter.load_all_ships(SHIPS_DIR)
print(f"Found {len(all_ships)} ship definitions")

for ship_id, ship_data in all_ships.items():
    print(f"Generating {ship_id}...")
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()

    config = atlas_exporter.parse_ship_config(ship_data)
    hull = ship_generator.generate_spaceship(
        ship_class=config['ship_class'],
        seed=config['seed'],
        generate_interior=False,
        module_slots=config['module_slots'],
        hull_complexity=1.0,
        symmetry=config['symmetry'],
        style=config['style'],
    )
    hull.name = ship_id

    bpy.ops.object.select_all(action='DESELECT')
    hull.select_set(True)
    for child in hull.children_recursive:
        child.select_set(True)

    filepath = os.path.join(EXPORT_DIR, f"{ship_id}.obj")
    atlas_exporter.export_obj(filepath)
    print(f"  Exported → {filepath}")

print("Done!")
```

Run it headless:

```bash
blender --background --python batch_generate.py
```

---

## 12. PCG Pipeline — Universe Generation

The `pcg_pipeline/` subdirectory provides a **standalone** seed-based universe generator that works without Blender for metadata, or with Blender in headless mode for full mesh export.

### Standalone metadata generation (no Blender)

```bash
cd tools/BlenderSpaceshipGenerator
python -m pcg_pipeline.batch_generate --seed 123456 --systems 10 --output-dir ../../build
```

This creates JSON metadata for galaxies, star systems, planets, stations, ships, and characters — all deterministic from the seed.

### With Blender mesh export

```bash
python -m pcg_pipeline.batch_generate --seed 123456 --systems 5 --export-meshes
```

### Pipeline Modules

| Module | Description |
|--------|-------------|
| `galaxy_generator.py` | Top-level galaxy with N star systems |
| `system_generator.py` | Star system with stars, planets, stations, ships |
| `planet_generator.py` | Planet type, biome, atmosphere, foliage, liquids |
| `station_generator.py` | Station type, modules, faction |
| `ship_generator.py` | Ship class, faction, modules, hardpoints |
| `character_generator.py` | Race, body type, cybernetic limbs |
| `batch_generate.py` | Single-command batch orchestrator |

### Validate the pipeline (no Blender required)

```bash
python tools/BlenderSpaceshipGenerator/pcg_pipeline/test_pcg_pipeline.py
```

---

## 13. Interior Exploration (Walk Mode)

All ship interiors are built to human scale (1.8 m tall) for first-person exploration.

### How to explore

1. Generate a ship with **Generate Interior** enabled.
2. Select any interior object (e.g. `Bridge_Floor`).
3. Press `/` to enter local view (hides exterior, shows only interior).
4. Press **Shift + F** to enter walk mode.
5. **WASD** to move, **mouse** to look.
6. Press **Esc** to exit walk mode.

### Interior dimensions

| Measurement | Value |
|-------------|-------|
| Human height | 1.8 m |
| Door height | 2.0 m |
| Door width | 1.0 m |
| Corridor width | 1.5 m |
| Corridor height | 2.5 m |
| Room height | 3.0 m |

### Room types by ship size

| Ship Size | Rooms |
|-----------|-------|
| Shuttle / Fighter | Cockpit only |
| Corvette / Frigate | Cockpit + corridor + crew quarters |
| Destroyer+ | Bridge + corridor network + quarters + cargo bay + engine room |

When modules are fitted, additional module-specific rooms are generated (Armory, Shield Control, Sensor Ops, Power Core Room, Hangar Bay, Cargo Hold).

---

## 14. Scripting API

You can generate ships programmatically from Blender's Python console or a script:

```python
import bpy
from BlenderSpaceshipGenerator import ship_generator, texture_generator

# Generate a ship
hull = ship_generator.generate_spaceship(
    ship_class='CRUISER',
    seed=42,
    generate_interior=True,
    module_slots=5,
    hull_complexity=1.5,
    symmetry=True,
    style='SOLARI',
    naming_prefix='NOVAFORGE',
    turret_hardpoints=6,
    hull_taper=0.85,
)

# Apply textures
texture_generator.apply_textures_to_ship(
    hull,
    style='SOLARI',
    seed=42,
    weathering=0.3,
)

# Access generated objects
collection = bpy.data.collections['Spaceship_CRUISER_42']
for obj in collection.objects:
    print(f"Generated: {obj.name}")
```

### Key functions

| Function | Module | Purpose |
|----------|--------|---------|
| `generate_spaceship(...)` | `ship_generator` | Generate a complete ship |
| `apply_textures_to_ship(hull, ...)` | `texture_generator` | Apply PBR materials |
| `generate_station(station_type, faction, seed)` | `station_generator` | Generate a station |
| `generate_asteroid_belt(layout, ore_types, count, seed)` | `asteroid_generator` | Generate an asteroid belt |
| `load_ship_data(json_path)` | `atlas_exporter` | Load Nova Forge ship JSON |
| `parse_ship_config(ship_data)` | `atlas_exporter` | Convert JSON → generator params |
| `load_all_ships(data_dir)` | `atlas_exporter` | Load all ships from a directory |
| `export_obj(filepath)` | `atlas_exporter` | Export selected objects as OBJ |

---

## 15. Troubleshooting

### Addon doesn't appear after installation

- Verify you enabled it in Edit → Preferences → Add-ons (search "Spaceship Generator").
- Check the Blender System Console for Python errors (Window → Toggle System Console on Windows, or launch Blender from a terminal on Linux/macOS).
- Verify Blender version is 2.80+.
- Verify all Python files are in the same directory (`__init__.py`, `ship_generator.py`, `ship_parts.py`, `interior_generator.py`, `module_system.py`, `brick_system.py`, `atlas_exporter.py`, `station_generator.py`, `asteroid_generator.py`, `texture_generator.py`).

### Generation is slow

- Disable **Generate Interior** for exterior-only work.
- Lower **Hull Complexity** (0.5–1.0).
- Reduce **Module Slots**.
- Use smaller ship classes.

### Ship appears at wrong location

- The ship generates at the 3D cursor position.
- Press **Shift + S → Cursor to World Origin** to reset the cursor.

### Parts seem disconnected

- All parts are parented to the hull object.
- Select the hull and press **Alt + G** to reset position.

### Interior too small / large

- Interior uses standard human scale (1.8 m).
- Check your Blender unit settings: **Scene → Unit Scale** should be `1.0`.

### "Select a valid Nova Forge ship JSON file"

- The Ship JSON path must point to a `.json` file.
- Use the folder icon to browse to the file.
- Make sure the path ends with `.json`.

### OBJ export fails

- Make sure you have an **active object** selected (click a ship hull).
- Make sure the **Export Path** is set and the directory exists.
- Use Blender 3.x+ for best OBJ exporter compatibility.

### Models don't load in the Atlas engine

- Verify the `.obj` file name matches the ship `id` from the JSON exactly.
- Verify the file is in `data/ships/obj_models/`.
- Check the Atlas engine console for model loading errors.
- Make sure the `.mtl` file is in the same directory as the `.obj`.

### Validate without Blender

Run the structure validation test (no Blender required):

```bash
python tools/BlenderSpaceshipGenerator/test_validation.py
```

---

## Further Reading

- [tools/BlenderSpaceshipGenerator/README.md](../../tools/BlenderSpaceshipGenerator/README.md) — Feature overview
- [tools/BlenderSpaceshipGenerator/NOVAFORGE_GUIDE.md](../../tools/BlenderSpaceshipGenerator/NOVAFORGE_GUIDE.md) — Full Nova Forge integration walkthrough
- [tools/BlenderSpaceshipGenerator/ENGINE_INTEGRATION.md](../../tools/BlenderSpaceshipGenerator/ENGINE_INTEGRATION.md) — Ship DNA schemas, brick taxonomy, ECS mappings, and C++ pseudocode for Atlas engine developers
- [tools/BlenderSpaceshipGenerator/TECHNICAL.md](../../tools/BlenderSpaceshipGenerator/TECHNICAL.md) — Architecture, design patterns, and extension points
- [tools/BlenderSpaceshipGenerator/features.md](../../tools/BlenderSpaceshipGenerator/features.md) — Complete feature specification and design rules
- [tools/BlenderSpaceshipGenerator/EXAMPLES.md](../../tools/BlenderSpaceshipGenerator/EXAMPLES.md) — Example configurations and fleet building
- [tools/README.md](../../tools/README.md) — Overview of all Nova Forge modding tools
