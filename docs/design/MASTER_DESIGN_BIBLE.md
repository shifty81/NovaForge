# NovaForge — Master Design Bible

> Consolidated from iterative AI–human design sessions (2025–2026). This document is the canonical reference for all game systems, procedural generation rules, and design philosophy.

---

## Design Philosophy

### Core Pillars

- **Interior-driven ship construction:** hull shapes are derived entirely from interior modules
- **AI and player parity:** if the AI can build it, the player can build it — using the same system
- **Fighter → Titan scale:** one unified system from the smallest craft to world-ships
- **Persistent universe:** damage, wrecks, scars, and history never reset
- **PVE co-op focus:** all challenge from physics, AI, and environment — no PvP
- **Data-driven everything:** JSON/data tables for items, stats, behaviors, seeds
- **Mod-first architecture:** content grows without code changes
- **Deterministic simulation:** same seed always produces identical results

### What NovaForge Is Not

- Not a scripted narrative game
- Not a power-fantasy escalation treadmill
- Not a sandbox without meaning
- Not an asset-copied EVE clone
- Not client-authoritative

### Contribution Rules

When adding features:

- ❌ Do not script outcomes
- ❌ Do not hard-code hero moments
- ❌ Do not add permanent player penalties
- ✅ Prefer data-driven bias
- ✅ Prefer probability over certainty
- ✅ Prefer systems that interact, not override

If a feature does not support legend formation, world bias, or emergent storytelling, it does not belong in NovaForge.

---

## Ship Construction System

### Ship = Module Graph, Not a Mesh

Ships are assembled from snap-together modules connected at hardpoints. The ship is a graph, not a merged mesh.

```
Ship
 ├─ Core Module
 │   ├─ Engine Block
 │   ├─ Reactor
 │   └─ Weapon Spine
 ├─ Belly Bay
 │   ├─ Rover Bay
 │   └─ Grav Bike Dock
 └─ Hangar Section
```

- Nodes = modules
- Edges = hardpoint connections
- Meshes are never merged — each module retains its own geometry

### Interior-Driven Exterior

The hull shape is derived entirely from interior modules and their connections:

| Interior Module   | Exterior Hull Cue                              |
| ----------------- | ---------------------------------------------- |
| Belly Bay         | Ramp doors, hull bulge, industrial panels      |
| Rover Dock        | Hull bulge, ramp slope, mag-lock rails         |
| Grav Bike Dock    | Sub-room airlock, hull hatch, mag-lock panel   |
| Mobile Fab        | Vent stacks, crane supports, panels            |
| Drone Bay         | Launch hatches, antennae, accent panels        |
| Player Rig Bay    | Hull indentation, docking clamps               |
| Elevator / Lift   | Vertical reinforced panels                     |
| Filler / Hallway  | Convex hull bridging, low-poly panels          |

### Module Size Classes

| Size  | Usage             | Example                    |
| ----- | ----------------- | -------------------------- |
| XS    | Fighters, shuttles| Starter craft              |
| S     | Frigates          | Light combat/scout         |
| M     | Destroyers        | Mid-range combat           |
| L     | Cruisers          | Versatile combat platform  |
| XL    | Battleships       | Heavy combat               |
| XXL   | Dreadnoughts      | Capital siege              |
| XXXL  | Supercapitals     | Fleet command              |
| TITAN | World ships       | Civilizational threshold   |

Module sizes are interchangeable within constraints: 2 XS modules = 1 S slot, 1 M = 3 S, 1 L = 5 S, etc. Rooms modularly adapt when stepping down module sizes.

### Hardpoint System

Hardpoints are authored as named objects in OBJ files:

- Format: `o hp_<type>_<id>` (e.g., `hp_engine_0`, `hp_weapon_L`, `hp_spine_0`)
- Position defines snap point
- Orientation optional (identity if omitted)
- Coordinate system: +Z forward, +Y up, +X right, units = meters, origin = module center

**Snapping Math:**

```
WorldChild = WorldParent * ParentHardpointTransform * inverse(ChildHardpointTransform)
```

**Mirroring (L→R):**

```
MirrorMatrix = scale(-1, 1, 1)
// Apply before inverse child hardpoint
```

---

## Multi-Deck Ship Layout

### Deck Expansion Rules

- Multi-deck expansions can be vertical, side-offset, or rear-offset
- Not all decks must be vertical — side and rear snaps create different ship profiles
- No two ships are the same unless intentionally built that way
- Once happy with design and layout, player submits build and waits for construction timer

### Interior Generation Pipeline

1. Place interior modules on grid
2. Recursive expansion generates child modules (hallways, filler, bays)
3. Gaps between modules are filled with hallways or filler rooms
4. Hull mesh stretches and interpolates to wrap all modules
5. Industrial accents (panels, vents, struts, slopes) added automatically
6. Color/accent rules applied per module type

### Traversal System

- **Central elevator** aligned with bridge accesses all floors down to belly hangar
- **Ramps** — configurable placement (side, front, or rear)
- **Ladders** — vertical movement between adjacent decks
- **Doors** — standard, airlock, security types with state machines
- **Elevators** — vertical travel for personnel and vehicles
- The cockpit/flight deck/control room is its own module

---

## Belly Bay & Vehicle Systems

### Belly Bay

Every frigate-class and above has a belly rover bay accessed via elevator near center of mass:

- Contains rover parking, grav bike dock, equipment locker
- Equipment locker is a changing room for armor, rig modules, and weapon loadouts
- Ramp configurable: side, front, or rear placement
- Can manufacture vehicles and drones via fabrication console

### Rover System

- Modular vehicle with configurable modules: mining, cargo, solar, scanner, weapons
- Enter via ramp with configurable placement
- Size/class determined by bay size
- Has its own interior: rig locker, equipment mount, scannable rooms
- Open hangar in unsafe environment damages unsuited players

### Grav Bike System

- Self-contained mount elevator for surface deployment
- Sub-room inside rover bay — get on bike, deploy via elevator airlock
- Elevator descends until ground is met, mag-locks disengage
- Modules: battery upgrade and solar attachment for charging while exploring
- Deployed elevator serves as interface to access ship systems, crafting, storage

### Vehicle Stats

All vehicle stats derived from installed modules:

```cpp
struct Vehicle {
    VehicleClass cls;
    Vector3 size;
    vector<Module> installedModules;
    Stats CalculateStats() {
        Stats s = baseStats[cls];
        for (auto& m : installedModules) s += m.bonuses;
        return s;
    }
};
```

---

## Drone System

### Manufacturing

- Drone hulls assembled in manufacturing console/bay
- Drone bay is where you configure the assembled drone
- Drones are on-demand, multi-purpose for ship needs
- Mobile Fab inside belly bay can also assemble drone hulls

### Drone Roles & Configuration

- Defense, attack, recon, utility roles
- Modular features based on ship's installed systems and modules
- Ship systems and modules determine what drone capabilities are supported

### Drone Port System (Fleet Mode)

- Drones ferry resources between ships through drone ports that directly access cargo
- Drone ports connect to storage and manufacturing chains
- Only active in fleet deploy mode
- Player sees drones passing items between ships; internally it's item transfer until manufacturing chain completes

---

## Player Rig System

### The Rig

The player rig is essentially a power armor frame/backpack:

- Plugs into cockpit seat port for ship AI system connection
- Provides Elite Dangerous-style HUD when plugged in
- Movement while in armor is different from on-foot or full rig
- 2×2 to 8×2 rack sizes with module slots

### Rig Modules (13 types)

LifeSupport, PowerCore, JetpackTank, Sensor, Shield, EnvironFilter, ToolMount, WeaponMount, DroneController, ScannerSuite, CargoPod, BatteryPack, SolarPanel

### Multi-Tool System

The multi-tool is PCG (procedurally generated) with swappable modules:

- Functions: mining, salvaging, hacking, scanning, cutting
- Same modular system as weapons
- Sidearm, main weapon, and multi-tool are all PCG with interchangeable modules
- From modular handgun to long guns, all use same module system
- Modules change function, stats, and visual appearance

### Weapon Attachment

An animated attachment point on the rig for the player's main weapon/multi-tool.

---

## Star Rating System

### Multi-System Ratings

Every ship has per-system ratings (1–5 stars), not just a single score:

| System                  | What Affects It                                    |
| ----------------------- | -------------------------------------------------- |
| Combat / Weapons        | Turret upgrades, hull reinforcement, targeting     |
| Mining / Salvage        | Excavators, refinery modules, multi-tool synergy   |
| Fabrication / Crafting  | Mobile fabs, drone-assisted production             |
| Exploration / Sensors   | Sensor modules, scout drones, rig enhancements     |
| Cargo / Logistics       | Bay size, rover/grav bike integration, storage     |
| Defense / Survival      | Shields, armor, hull integrity, support drones     |

### Rating Factors

- Installed module tiers (higher tier = more contribution)
- Player skills (Engineering, Fabrication, Piloting)
- Module synergy (drones, rigs increase effectiveness)
- Module placement optimization (centralized core = better)
- Economic factors (planetary base resources, station proximity)

### AI Integration

AI ships make decisions based on system-specific ratings:

- Mining fleet → select ships with high Mining ratings
- Combat fleet → prioritize high Combat/Defense ratings
- Exploration → choose ships with high Sensors ratings
- Multi-system ships are versatile but may be "jack-of-all-trades"

---

## Planet Landing & Surface Systems

### Flat-Site Detection

- Ship can land anywhere without risk of clipping or uneven terrain
- Procedural flat-site detection or creation guarantees a safe landing spot
- Generation normally resumes around landing site, giving a nice flat area to build on

### Planetary Bases

Uses same PCG engine logic as ships — minimal new code:

- Modular decks/rooms: command hub, living quarters, labs, fabrication bays
- Recursive expansion: modules snap to side, rear, or above other modules
- Hull wrapping / mesh generation with low-poly aesthetic
- Build/upgrade queue with timed construction

### Base Module Types

CoreHub, LivingQuarters, Lab, Workshop, FabricationBay, ShipHangar, VehicleBay, DroneFabBay, Corridor, FillerRoom, DefenseModule

### Construction Support

- Ships and rovers as build platforms with drone assistance
- Drones help construction, reducing build time
- Fleet drones can be assigned to construction tasks

---

## FPS & Interior Combat

### Mode System

The game supports seamless transitions between:

- **FPS** — First-person on foot / in rig
- **FPS Flight** — First-person ship piloting
- **RTS** — Fleet command overlay from fleet terminal

### FPS Combat State Machine

States: Idle → Engaged → Breached → Recovery

- Breach events from hull damage, decompression, fire
- Recovery involves repair, seal, stabilize sequences
- Interior actions directly affect space combat in real time

### Interior-Exterior Coupling

- Exterior modules map to interior zones (Engine → engine room, Hangar → flight deck)
- Shared health and power state between interior and exterior
- Hull deformation visible from both perspectives

---

## Damage & Cascading Failure

### Per-Module Damage

Each module tracks: HP, power state, connectivity

- Destroying a module removes its function
- May isolate other modules (graph disconnection)

### Cascading Failure Examples

- Reactor destroyed → weapons offline
- Engine cluster lost → mobility collapse
- Spine destroyed → ship splits
- No scripted events — pure system behavior

### Capital Weapon Targeting

Weapons target roles, not random HP:

| Intent  | Target  |
| ------- | ------- |
| Cripple | Engines |
| Disarm  | Weapons |
| Blind   | Sensors |
| Break   | Spines  |

Titans lose sections, not health bars.

---

## Performance Budgets

| Ship Size      | Max Modules | Sim Rate |
| -------------- | ----------- | -------- |
| Fighter        | ≤ 5         | 60 Hz    |
| Frigate        | ≤ 15        | 30 Hz    |
| Cruiser        | ≤ 40        | 20 Hz    |
| Battleship     | ≤ 80        | 10 Hz    |
| Dreadnought    | ≤ 150       | 5 Hz     |
| Titan Section  | ≤ 300       | 1–2 Hz   |

Budgets are enforced, not suggestions.

---

## Low-Poly Visual System

### Surrounded Aesthetic

The game targets a "Surrounded" art style — low-poly with intention:

- Vertex-color driven shading
- Hard edge normals (no auto-smoothing)
- Limited light count per object
- Flat / controlled lighting model
- Small fixed material set (10–20 max)
- Solid colors + gradients, palette-driven materials

### Aesthetic Enforcement Rules

- Hull silhouette always communicates interior function
- Panels, vents, slopes, struts, airlocks added procedurally per module type
- Color/accent rules per module type:
  - Belly Bay → metallic gray + yellow hazard
  - Fab → industrial orange / dark gray
  - Drone Bay → lighter panels, small accents
  - Rig Dock → neutral steel / tech blue

### Visual Star Rating Cues

| Stars | Hull Appearance                                                       |
| ----- | --------------------------------------------------------------------- |
| 1     | Small, sparse, flat surfaces, minimal detail                          |
| 2     | Slight hull extensions, vents, small lights                           |
| 3     | Modular protrusions, visible machinery, more accent colors            |
| 4     | Multi-level hull, ramps, visible conduits, bright lights              |
| 5     | Full industrial detailing, external machinery, decorative antennae    |

---

## Titan & World Ships

### Titan = Composite World Object

```
Titan
 ├─ City Section
 ├─ Engine Wall
 ├─ Weapon Spine
```

Each section has: independent module graph, independent save file, independent simulation. Titans never fully load at once.

### Chunk Streaming

| Distance | Detail Level        |
| -------- | ------------------- |
| Far      | Proxy / billboard   |
| Mid      | Low-fidelity sim    |
| Near     | Full modules        |
| Interior | FPS detail          |

---

## Build / Upgrade / Deployment Queue

### Workflow

1. Player places module → snaps to grid
2. Recursive expansion generates child modules
3. Hull mesh generated dynamically
4. Build/upgrade timers assigned per module
5. Deployment propagation (rover→ramp, bike→elevator, drones→fab)

### Queue Structure

```
[Ship Build Start]
  ├─ Hull + Deck Expansion (t0)
  ├─ Belly Bay Setup (t1)
  │    ├─ Rover / Grav Bike (t2)
  │    └─ Mobile Fab → Drone Hulls (t3)
  ├─ Drone Bay Configuration (t4)
  └─ Player Rig + Tools (t5)
```

Parallel timers allowed; deterministic order ensures reproducible builds.

### Drag-and-Drop Grid System

All build systems use a drag-and-drop snap-to-grid system:

- Ship builder, rover builder, grav bike builder, rig builder (suit systems view)
- Live hull mesh preview during placement
- Ghost snapping for alignment
- Undo/redo command stack

---

## Persistence & Saves

### Ship Save (Binary)

Saved: module hashes, connections, HP & power, DNA seed

Never saved: mesh data, geometry, baked transforms

### World Save

- Sector-based files
- Titan sections saved independently
- Wreck fields persist
- Offline simulation advances state between sessions

---

## Related Documents

- [Procedural Systems Pipeline](PROCEDURAL_SYSTEMS.md) — recursive generation details
- [Vehicles & Equipment](VEHICLES_AND_EQUIPMENT.md) — rover, grav bike, drone, rig, multi-tool specs
- [Salvage & Legends](SALVAGE_AND_LEGENDS.md) — EVA salvage, ghost system, legendary wrecks, progression
- [ROADMAP](../ROADMAP.md) — implementation status and phase tracking
- [Ship Modeling](../SHIP_MODELING.md) — current procedural ship model generation
- [Design Document](DESIGN.md) — EVE-aligned game systems reference

---

*This document is a living reference. Updated as design decisions evolve through continued AI–human collaboration.*
