# AtlasForge Generator

A comprehensive Blender addon for procedurally generating spaceships with modular parts and interiors. Inspired by games like X4 Foundations, Elite Dangerous, and Eve Online.

Built as the asset generator for the [Nova Forge](https://github.com/shifty81/NOVAFORGE) project — a PVE space simulator powered by the custom Atlas Engine and Atlas UI.

## Nova Forge / Atlas Integration

This addon integrates directly with the Nova Forge project:

- **Import ships from Nova Forge JSON** — reads `data/ships/*.json` and generates matching geometry using each ship's faction, class, seed, and hardpoint data.
- **Export OBJ for Atlas Engine** — one-click export with correct axis orientation for the Atlas rendering pipeline.
- **Four faction styles** — Solari (elegant), Veyren (angular), Aurelian (organic), Keldari (rugged) map directly to Nova Forge's four factions.
- **All ship classes** — Frigates through Titans, plus Industrials, Mining Barges, and Exhumers.
- **Station generation** — procedural space stations matching Nova Forge station types (Industrial, Military, Commercial, Research, Mining) and Upwell structures (Astrahus, Fortizar, Keepstar).
- **Asteroid belt generation** — procedural asteroid belts with all 16 Nova Forge ore types and 4 belt layouts (Semicircle, Sphere, Cluster, Ring).

**→ [Full Nova Forge Integration Guide](NOVAFORGE_GUIDE.md)** — start-to-finish instructions for generating and exporting ships into the Atlas engine.

**→ [Engine Integration Reference](ENGINE_INTEGRATION.md)** — data structures, JSON schemas, ECS mappings, and C++ pseudocode for implementing ship systems in the Atlas engine.

**→ [Feature Specification](features.md)** — complete feature list, design rules, and implementation status.

## Features

- **Multiple Ship Classes**: Generate ships from small shuttles to massive titans
  - Shuttle
  - Fighter
  - Corvette
  - Frigate
  - Destroyer
  - Cruiser
  - Battlecruiser
  - Battleship
  - Carrier
  - Dreadnought
  - Capital Ship
  - Titan
  - Industrial
  - Mining Barge
  - Exhumer

- **Modular Ship Parts**:
  - Hull with configurable geometry and complexity
  - Cockpit/Bridge appropriate to ship size
  - Multiple engine configurations
  - Wing structures for smaller vessels
  - Weapon hardpoints
  - Progressive module system for ship expansion

- **Brick Taxonomy & LEGO Logic**:
  - 18 brick types across 5 categories (Core, Hull, Function, Utility, Detail)
  - Hardpoint declarations on every brick for snap-attach rules
  - Scale hierarchy bands (primary, structural, detail)
  - Grid snapping per ship class (0.5 m for shuttles up to 8 m for titans)
  - Ship DNA JSON export for reproducible/saveable ships

- **Spine-First Assembly Pipeline**:
  1. Core hull (spine)
  2. Cockpit / bridge
  3. Major structures (wings)
  4. Engines (archetype-varied)
  5. Weapons & turrets
  6. Detail modules
  7. Interior (optional)
  8. Hull taper / deform pass
  9. Bevel + auto-smooth cleanup pass

- **Engine Archetypes**:
  - Main Thrust — big, recessed, with nozzle flare
  - Maneuvering Thrusters — small, angled
  - Utility Exhaust — flat vents
  - Each archetype varies depth, radius, glow strength

- **Hull Shaping**:
  - Configurable hull taper for silhouette shaping
  - Automatic bevel + auto-smooth cleanup pass
  - Style-specific hull deformations (X4, Elite, EVE, NMS, factions)

- **Interior Generation**:
  - FPV-ready interiors scaled for human exploration
  - Corridor networks connecting ship areas
  - Bridge/cockpit interiors
  - Crew quarters with bunks
  - Cargo bays with containers
  - Engine rooms with reactor cores
  - Doorways and access points

- **Module System**:
  - Cargo modules
  - Weapon modules
  - Shield generators
  - Hangar bays
  - Sensor arrays
  - Power modules

- **Design Styles**:
  - X4 Foundations (Angular, geometric)
  - Elite Dangerous (Sleek, aerodynamic)
  - Eve Online (Organic, flowing)
  - Mixed (Combination of all styles)
  - **Nova Forge Factions**:
    - Solari (Golden, elegant — armor tanking)
    - Veyren (Angular, utilitarian — shield tanking)
    - Aurelian (Sleek, organic — drones)
    - Keldari (Rugged, industrial — missiles)

- **Station Generation**:
  - NPC station types: Industrial, Military, Commercial, Research, Mining
  - Upwell structures: Astrahus, Fortizar, Keepstar
  - Faction-specific architecture (spires, domes, blocks, scaffolding)
  - Docking bays and hangars

- **Asteroid Belt Generation**:
  - 16 ore types from Nova Forge (Dustite through Nexorite)
  - 4 belt layouts: Semicircle, Sphere, Cluster, Ring
  - Procedural deformation for natural rocky shapes
  - PBR materials matching ore visual data

- **Game Engine Integration**:
  - LOD generation (LOD0-LOD3) with per-class switch distances
  - Collision mesh generation (box, convex hull, multi-convex)
  - Animation system (turret rotation, bay doors, landing gear, radar spin)
  - Damage propagation and structural integrity simulation
  - Power flow and capacitor mechanics
  - Build validation (grid snapping, hardpoint compatibility, connectivity)
  - Catalog render setup (camera, three-point lighting, thumbnails)
  - NovaForge ship JSON import with race/class mapping
  - Batch generation and full pipeline export

## Installation

### Quickest Way: Download Pre-Built ZIP

Every push to the repository automatically builds ready-to-install ZIP files
via GitHub Actions.  No manual packaging required.

1. Go to the [**Actions** tab](../../actions/workflows/blender-addon-package.yml)
   of this repository on GitHub.
2. Click the latest successful **"Package Blender Addon"** run.
3. Scroll to the **Artifacts** section and download the ZIP for your Blender
   version:
   | File | Blender version |
   |------|----------------|
   | `AtlasForgeGenerator-blender42plus` → **`AtlasForgeGenerator.zip`** | Blender **4.2+** (Extensions platform) |
   | `AtlasForgeGenerator-legacy` → **`AtlasForgeGenerator_legacy.zip`** | Blender **2.80 – 4.1** (legacy Add-ons) |

### Blender 4.2+ — Install as Extension (Recommended)

Blender 4.2 introduced the **Extensions** platform. Use
**`AtlasForgeGenerator.zip`** (files at the ZIP root, `blender_manifest.toml`
included).

1. Open **Blender 4.2+**.
2. Go to **Edit → Preferences → Get Extensions** (top bar).
3. Open the drop-down arrow **▾** next to *Repositories* and choose
   **Install from Disk…**.
4. Select `AtlasForgeGenerator.zip`.
5. The addon appears under **Add-ons** and is enabled automatically.

> **Building manually**: `python tools/BlenderSpaceshipGenerator/package.py` in
> the repo root creates both ZIPs in the repo root.

### Blender 2.80 – 4.1 — Install Legacy Add-on

Use **`AtlasForgeGenerator_legacy.zip`** (files inside a
`BlenderSpaceshipGenerator/` folder at the ZIP root).

1. Open Blender (2.80 – 4.1).
2. Go to **Edit → Preferences → Add-ons**.
3. Click **Install…** (top-right) and select `AtlasForgeGenerator_legacy.zip`.
4. Search for **"AtlasForge"** in the add-ons list.
5. Tick the checkbox next to **AtlasForge Generator** to enable it.

### Method 3 — Direct Copy (any version)

Copy (or symlink) the entire `BlenderSpaceshipGenerator/` folder into
Blender's addons directory:

| Blender version | OS | Path |
|-----------------|----|------|
| 4.2+ | Linux | `~/.config/blender/<version>/extensions/user_default/` |
| 4.2+ | macOS | `~/Library/Application Support/Blender/<version>/extensions/user_default/` |
| 4.2+ | Windows | `%APPDATA%\Blender Foundation\Blender\<version>\extensions\user_default\` |
| 2.80–4.1 | Linux | `~/.config/blender/<version>/scripts/addons/` |
| 2.80–4.1 | macOS | `~/Library/Application Support/Blender/<version>/scripts/addons/` |
| 2.80–4.1 | Windows | `%APPDATA%\Blender Foundation\Blender\<version>\scripts\addons\` |

Restart Blender, then go to **Edit → Preferences → Add-ons**, search for
**"AtlasForge"**, and enable the checkbox.

### Verifying the Installation

After enabling the addon you should see:

1. **Sidebar tab** — In the 3D Viewport press **N** to open the sidebar.
   An **"AtlasForge"** tab appears on the right.
2. **Console output** — Open the system console (on Windows:
   *Window → Toggle System Console*; on Linux/macOS run Blender from a
   terminal) and look for:
   ```
   [AtlasForge] Addon registered (13 classes)
   [AtlasForge] All submodules loaded OK
   ```
   If any submodule failed to load, a warning line will be printed
   with the error message — this helps diagnose missing dependencies.

### Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Addon not visible after install (Blender 4.2+) | ZIP has wrong structure — `blender_manifest.toml` must be at the ZIP root | Use the pre-built **`AtlasForgeGenerator.zip`** from CI artifacts, or run `python tools/BlenderSpaceshipGenerator/package.py` |
| Nothing shown after install (Blender < 4.2) | ZIP structure wrong — folder `BlenderSpaceshipGenerator/` must be at ZIP root | Use the pre-built **`AtlasForgeGenerator_legacy.zip`** from CI artifacts |
| Addon checkbox appears but no **AtlasForge** sidebar tab | Submodule import error stopped `register()` partway | Open system console; look for `[AtlasForge] WARNING` lines; report them as an issue |
| `"No module named '…'"` error in console | ZIP was packed at the wrong level | Use the pre-built ZIPs or run `package.py` |
| Blender version error | Addon requires Blender 2.80+ | Upgrade Blender |

## Usage

### Quick Start

1. Open Blender and go to the 3D Viewport.
2. Press **N** to open the sidebar (if not already visible).
3. Click the **"AtlasForge"** tab on the right-hand side.
4. Configure your ship:
   - Select ship class (Shuttle to Titan)
   - Choose design style (Mixed, EVE, Elite, NMS, or a faction)
   - Set random seed for variation
   - Enable/disable interior generation
   - Adjust module slots and hull complexity
   - Toggle symmetry
5. Click **"Generate Spaceship"**.

### PCG Pipeline Panel

Below the main generator panel you will find the **PCG Pipeline** sub-panel
(click to expand). It provides batch universe generation:

- **Generate Universe** — create a full galaxy of star systems, planets,
  stations, ships and characters. Output is written as JSON metadata to the
  configured output directory.
- **Generate System** — create a single star system.
- **Materialize Ships** — read ship JSON files from the output directory and
  build the corresponding meshes in the current Blender scene.

### Other Operations

| Button | What It Does |
|--------|-------------|
| **Import from Project Data** | Load a ship from the project's `data/ships/` directory |
| **Batch Generate All Ships** | Generate meshes for every ship in the data directory |
| **Full Pipeline Export** | Generate → texture → LOD → export OBJ for all ships |
| **Setup Catalog Render** | Position camera + lights for a catalog-style render |
| **Export OBJ for Atlas** | Export selected ship(s) as OBJ for the Atlas engine |
| **Export Ship DNA** | Save the selected ship's generation parameters as JSON |
| **Generate Station** | Create a procedural station (Industrial, Military, etc.) |
| **Generate Asteroid Belt** | Create an asteroid belt with configurable ore types |

## Ship Classes

### Small Vessels
- **Shuttle**: Basic transport, 2 crew, minimal weapons
- **Fighter**: Single-seat combat, agile with wings
- **Corvette**: Small multi-crew, 4 crew, basic modules

### Medium Vessels
- **Frigate**: Multi-role ship, 10 crew, interior corridors
- **Destroyer**: Heavy combat focus, 25 crew, full interior
- **Industrial**: Cargo hauler, 5 crew, large hold
- **Mining Barge**: Mining vessel, 3 crew
- **Exhumer**: Advanced mining vessel, 4 crew

### Large Vessels
- **Cruiser**: Large multi-role, 50 crew, extensive interior
- **Battlecruiser**: Heavy attack cruiser, 75 crew
- **Battleship**: Heavy capital ship, 100 crew, many weapon hardpoints
- **Carrier**: Fleet carrier, 200 crew, hangar capacity
- **Capital**: Large capital ship, 500 crew, maximum module slots

### Supercapital Vessels
- **Dreadnought**: Siege capital, 400 crew, devastating firepower
- **Titan**: Supercapital flagship, 1000 crew, doomsday weapons

## Progressive Module System

Ships can be expanded with additional modules based on their class:
- Small ships: Weapon, Shield, Sensor modules
- Medium ships: Also Cargo and Power modules
- Large ships: All module types including Hangar modules

Each module is automatically placed and connected to the ship with appropriate details.

## Interior Standards

All ship interiors are built to human scale for FPV exploration:
- Standard door height: 2.0m
- Corridor width: 1.5m
- Room height: 3.0m
- All dimensions support first-person navigation

## Customization

The addon supports extensive customization:
- **Random Seed**: Change to generate different ship variations
- **Hull Complexity**: Adjust geometry detail (0.1 - 3.0)
- **Module Slots**: Add 0-10 additional modules
- **Symmetry**: Toggle symmetrical design
- **Style**: Choose specific design inspiration or mixed

## Requirements

- Blender 2.80 or higher
- No additional dependencies required

## License

This project is open source and available for modification and distribution.

## Contributing

Contributions are welcome! Feel free to submit pull requests with improvements, new features, or bug fixes.

## Credits

Inspired by the spaceship designs from:
- X4 Foundations by Egosoft
- Elite Dangerous by Frontier Developments
- Eve Online by CCP Games

