# NovaForge — Procedural Generation Pipeline

> Interior → Hull → Vehicles → Drones → Rigs → Tools — all recursive, deterministic, seed-locked.

## Overview

NovaForge uses a unified recursive procedural generation system where interior modules drive exterior hull shape, and parent modules constrain child modules. The same pipeline handles ships, stations, planetary bases, rovers, and player rigs.

## Core Principles

1. **Interior-Driven Exterior** — Hull shapes derived entirely from interior modules and connections
2. **Recursive Expansion** — Every module can spawn child modules (sub-rooms, hallways, bays)
3. **Modular Propagation** — Ships → Belly Bay → Vehicles → Mobile Fab → Drones → Player Rig → Tools
4. **Hull Wrapping** — Hull mesh interpolates around interior volumes, fills gaps
5. **Deterministic / Seed-Locked** — Same seed = identical layouts, modules, hull, stats
6. **Low-Poly Aesthetic** — All geometry triangulated, angular, visually readable

---

## Interior Node System

### Module Types

| Type | Function |
|------|----------|
| Deck | Main volume — vertical, side, or rear offset |
| Belly Bay | Rover, grav bike, fab bays, ramps/elevators |
| Rover / Grav Bike | Vehicles with modular stats |
| Mobile Fab | Produces drones, vehicles; inherits module rules |
| Drone Bay | Configure drone hulls, modules, and stats |
| Player Rig Bay | Power frame / armor, rig drone slots |
| Filler Room | Aesthetic or gameplay function (cargo, storage) |
| Hallway | Connects interior nodes; generates corridor mesh |
| Cockpit / Bridge | Command module, flight deck, AI interface |
| Airlock | Transition point for EVA, docking, atmosphere seal |

### Node Tree Structure

```
[Ship Node]
  ├─ Multi-Deck Hull Nodes
  │    ├─ Vertical stack
  │    ├─ Side wing (lateral expansion)
  │    └─ Rear offset
  ├─ Belly Bay Node
  │    ├─ Ramp Modules (front / side / rear)
  │    ├─ Elevator Modules (vertical / sub-room)
  │    ├─ Rover Node
  │    │    └─ Mobile Fab Node
  │    │         ├─ Assemble Vehicle Hulls
  │    │         └─ Assemble Drone Hulls
  │    └─ Grav Bike Node (sub-room + deployable elevator)
  ├─ Drone Bay Node
  │    ├─ Configure Drones (roles, modules, stats)
  │    └─ Link to Mobile Fab output
  ├─ Player Rig Node
  │    ├─ Armor / Power Frame Modules
  │    └─ Rig Drone Bay (up to 4)
  └─ Recursive Expansion Logic
       ├─ Child nodes auto-generate based on parent
       ├─ Hull snapping ensures low-poly readability
       └─ Build / Upgrade / Deployment Queue propagates automatically
```

---

## Recursive Generation Algorithm

### Module Data Structure

```cpp
struct Module {
    string name;
    ModuleType type;
    Vector3 size;
    Vector3 position;
    vector<Module*> children;
    vector<Module> installedModules;
    VehicleClass allowedClass;  // for bays

    Stats CalculateStats() {
        Stats s = baseStats[type];
        for (auto& m : installedModules) s += m.bonuses;
        return s;
    }
};
```

### Recursive Node Expansion

```cpp
void ExpandNode(Module* node) {
    for (auto& child : node->children) {
        SnapModuleToGrid(*child, *node);
        ExpandNode(child);
    }
}
```

### Hallway & Filler Generation Rules

- Auto-generated between aligned modules or multi-deck offsets
- Filler rooms provide cargo/storage, minor gameplay utility, visual continuity
- Hallways and filler rooms are convex hull contributors for exterior mesh
- Prefab filler rooms used for aesthetic variety
- Hull stretches and interpolates to wrap all modules

---

## Hull Mesh Generation

### Algorithm

```cpp
HullMesh GenerateHull(vector<Module*>& modules)
{
    HullMesh hull;

    // 1. Generate hull contribution from each module
    for (auto& m : modules) {
        HullMesh moduleHull = GenerateModuleHull(m);
        MergeHull(hull, moduleHull);
    }

    // 2. Fill gaps between modules
    for (auto& gap : FindGaps(modules)) {
        HullMesh filler = GenerateFillerGeometry(gap);
        MergeHull(hull, filler);
    }

    // 3. Generate hallway connections
    for (auto& pair : FindModulePairsForHallways(modules)) {
        HullMesh hallway = GenerateHallwayMesh(pair.first, pair.second);
        MergeHull(hull, hallway);
    }

    // 4. Stretch hull to fully enclose modules
    hull = StretchHullAroundModules(hull, modules);

    // 5. Apply low-poly styling
    hull = ApplyLowPolyStyling(hull);

    return hull;
}
```

### Hull Generation Rules

1. Convex hull computed from interior module bounding volumes
2. Hull "stretch & fill" closes gaps between modules
3. Procedural hull panels, vents, struts, and slopes added
4. Ramp / elevator slope geometry integrated into hull
5. Deck offsets produce bulges and extensions
6. Low-poly aesthetic enforced on all generated geometry

---

## Modular Propagation Chain

Parent module constraints (bay size, ship class) determine allowed child module size and class. Stats propagate deterministically.

```
[Ship Belly Bay] → Rover Bay (size/class)
  ├─ Rover A (modules: mining, cargo, solar) → stats calculated
  ├─ Grav Bike (modules: battery, solar) → stats calculated
  └─ Mobile Fab → Drones inherit constraints

[Player Rig] → Equipped Multi-Tool / Weapon
  ├─ Modules: mining / combat / hacking / scanning
  └─ Stats derived from modules & rig
```

### Bay Fitting Logic

```cpp
struct Bay {
    Vector3 size;
    VehicleClass allowedClass;
    bool CanFit(Vehicle& v) {
        return v.size <= size && v.cls <= allowedClass;
    }
};
```

### Module Size Scaling

| Equivalence | Description |
|-------------|-------------|
| 2 XS = 1 S | Two extra-small modules fill one small slot |
| 1 M = 3 S | One medium module replaces three small |
| 1 L = 5 S | One large module replaces five small |
| 1 XL = 2 L | One extra-large module replaces two large |

When stepping down module sizes, rooms modularly adapt to fill remaining space.

---

## Build / Upgrade / Deployment Queue

### Build Queue

```
[Ship Build Start]
  ├─ Hull + Multi-Deck Expansion (t0)
  ├─ Belly Bay Setup (t1)
  │    ├─ Rover / Grav Bike Build (t2)
  │    └─ Mobile Fab → Produce Drones (t3)
  ├─ Drone Bay Configuration (t4)
  └─ Player Rig + Tools/Weapons (t5)
```

- Parallel timers allowed
- Deterministic order ensures reproducible results
- Deployment queue automatically propagates upgrades

### Construction Factors

- Player resources (materials, credits)
- Player skills (affects build time and success)
- Supporting units (drones/rovers/ships reduce time)
- Time-based progression (build queue system)

```cpp
void UpgradeModule(Module* mod, Player* player, vector<Drone*> assistants) {
    float baseTime = 60.0f;
    float skillModifier = 1.0f - player->skills.engineering * 0.05f;
    float droneSpeed = 1.0f + assistants.size() * 0.1f;
    float buildTime = baseTime * skillModifier / droneSpeed;

    BuildTask task;
    task.targetModule = mod;
    task.timeRemaining = buildTime;
    task.onComplete = [mod]() { mod->tier = std::min(5, mod->tier + 1); };
    globalBuildQueue.AddTask(task);
}
```

---

## Module Tier System

Each module has upgrade tiers (1–5) with procedural visual cues:

| Tier | Visual Character |
|------|-----------------|
| 1 | Basic geometry, minimal detail |
| 2 | Slight extensions, vents, small lights |
| 3 | Specialized sub-modules, visible machinery |
| 4 | Conveyor integration, external walkways |
| 5 | Full industrial complexity, multi-level |

Higher-tier modules have additional structural complexity, accent panels, vents, lights, and hull extensions while maintaining the low-poly aesthetic.

---

## Ship Class Interior Templates

### Interior System Weights

Ship specialization is determined by the weight of installed interior systems:

| System Weight | Ship Profile |
|--------------|-------------|
| Heavy mining modules | Industrial hull, bulky, ore bays visible |
| Heavy combat modules | Angular hull, weapon hardpoints prominent |
| Heavy exploration | Sensor arrays, sleek profile, scanner domes |
| Heavy logistics | Wide cargo hull, docking ports, conveyor lines |
| Balanced | Versatile profile, jack-of-all-trades |

### Room Types

Bridge, Engineering, CargoHold, CrewQuarters, MedicalBay, Armory, Corridor, Airlock, HangarBay, ScienceLab, FabricationBay, RoverBay, DroneControlRoom

### Corridor Generation

- Hub-and-spoke corridors from central elevator
- Non-linear connections when ≥ 4 rooms per deck
- Central elevator aligned with bridge accesses all floors
- Mesh stretches to form around interior modules and fill between them

---

## Planetary Base Generation

Uses the same recursive pipeline as ships:

```
Planetary Base
  └─ Core Hub Module
      ├─ Docking / Hangars → Ship & Rover Bays → Multi-Tool & Rig Stations
      ├─ Living Quarters / Labs → Player & NPC interaction
      ├─ Power & Resource Modules → Energy / Fabrication / Refinery
      ├─ Defensive Modules → Turrets, Shield Nodes
      └─ Corridor / Filler Modules → Connect everything
```

### Flat-Site Integration

Landing site is procedurally flattened or detected. Generation resumes around the landing site, providing a flat area for rover deployment and base construction.

---

## Full Pipeline Summary

```
[Interior Modules]
   ↓ Recursive Node Expansion
[Hallways & Filler Rooms]
   ↓ Hull Mesh Generation
[Exterior Hull + Panels/Vents/Accents]
   ↓ Vehicle / Drone / Player Rig Module Assignment
   ↓ Build / Upgrade / Deployment Queue
   ↓ Deterministic, Seed-Locked Modular Game Objects
```

- Any ship size, class, or configuration supported
- Interior layout drives exterior, gameplay, and stats
- All procedural, modular, and deterministic

---

## Related Documents

- [Master Design Bible](MASTER_DESIGN_BIBLE.md) — complete system overview
- [Vehicles & Equipment](VEHICLES_AND_EQUIPMENT.md) — vehicle and tool specifications
- [Salvage & Legends](SALVAGE_AND_LEGENDS.md) — EVA salvage and legend system
- [Ship Modeling](../SHIP_MODELING.md) — current procedural ship model generation

---

*Extracted from iterative design sessions. This document defines the generation rules — implementation status tracked in [ROADMAP](../ROADMAP.md).*
