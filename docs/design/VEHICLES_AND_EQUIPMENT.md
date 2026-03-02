# NovaForge — Vehicles & Equipment Reference

> Rovers, grav bikes, drones, player rigs, multi-tools, and weapons — all procedurally generated, modular, and deterministic.

---

## Rover System

### Overview
Rovers are modular ground vehicles stored in the ship's belly bay. They serve as mobile bases for planetary exploration, mining, and logistics.

### Deployment
- Access belly bay via ship elevator (usually near center of mass)
- Deploy via configurable ramp (front, side, or rear of ship)
- Rover bay acts as garage for both rover and grav bike
- Can modify vehicles through bay panel, selecting vehicle to upgrade or swap modules

### Rover Modules
| Module | Function |
|--------|----------|
| Mining Laser | Surface ore extraction |
| Cargo Bay | Resource storage and transport |
| Solar Panel | Passive energy recharge |
| Scanner Suite | Surface scanning and resource detection |
| Weapon Mount | Defense against hostile fauna/NPCs |
| Fabrication Unit | Field manufacturing capability |
| Drone Controller | Deploy and manage rover drones |

### Rover Classes
Rover class is determined by bay size and ship class. Larger ships support larger rovers with more module slots:
- **Light Rover** (S bay) — 2-3 module slots, basic exploration
- **Medium Rover** (M bay) — 4-5 module slots, mining/logistics
- **Heavy Rover** (L bay) — 6-8 module slots, mobile base operations

### Manufacturing
Rover hulls are manufactured in the belly bay fabrication console. The belly bay is also the manufacturing platform for vehicle counterparts.

---

## Grav Bike System

### Overview
Grav bikes are fast, light personal vehicles for surface traversal. Stored as a sub-room inside the rover bay.

### Deployment Sequence
1. Player enters grav bike sub-room in rover bay
2. Player mounts bike
3. Elevator acts as airlock and begins descending
4. Elevator moves down until ground is met
5. Mag-locks disengage
6. Player drives away

### Return Sequence
1. Player drives to deployed elevator
2. Mag-locks engage
3. Elevator retracts to ship
4. Player dismounts inside rover bay

### Grav Bike Modules
| Module | Function |
|--------|----------|
| Battery Pack | Extended range and power |
| Solar Attachment | Passive charging while exploring |

Grav bikes are intentionally simple — speed and mobility over capability.

### Deployed Elevator
The deployed grav bike elevator doubles as a remote interface to access:
- Ship systems
- Crafting menus
- Storage access
- UI interaction with ship AI

---

## Drone System

### Overview
Drones are on-demand, multi-purpose units manufactured and configured on the ship. They serve combat, utility, mining, and logistics roles.

### Manufacturing Pipeline
1. **Hull Assembly** — Done in manufacturing console/bay or mobile fab
2. **Configuration** — Done in drone bay, setting roles, modules, and stats
3. **Deployment** — Launch from drone bay hatches

### Drone Roles
| Role | Function |
|------|----------|
| Combat | Ship defense, target engagement |
| Mining | Automated ore extraction |
| Recon | Scouting, scanning, intelligence |
| Utility | Repair, towing, cargo transfer |
| Construction | Assist base/ship construction (reduces build time) |
| Salvage | Automated wreck processing |

### Drone Port System (Fleet Mode)
In fleet deploy mode, drones ferry resources between ships:
- Drone ports directly access cargo holds
- Resources transfer through drone port → storage → manufacturing chain
- Player sees drones moving between ships; internally it's item transfer
- Only active in fleet deploy mode

### Drone Stats
Like all modular entities, drone stats are derived from installed modules and parent ship systems.

### Rig Drones
The player rig can support up to 4 personal drones via the DroneController module.

---

## Player Rig System

### Overview
The rig is a power armor frame / backpack system that the player wears. It connects to the ship cockpit via a docking port, providing an Elite Dangerous-style HUD and AI system access.

### Rig Rack
- Configurable rack size: 2×2 to 8×2 slots
- Modules installed into rack slots
- Visual appearance changes with installed modules

### Rig Modules (13 Types)

| Module | Function |
|--------|----------|
| LifeSupport | Oxygen supply and recycling |
| PowerCore | Energy generation for rig systems |
| JetpackTank | EVA maneuvering fuel |
| Sensor | Environmental and threat detection |
| Shield | Personal energy shield |
| EnvironFilter | Toxic atmosphere / radiation protection |
| ToolMount | Multi-tool attachment point |
| WeaponMount | Weapon attachment point |
| DroneController | Personal drone bay (up to 4) |
| ScannerSuite | Advanced scanning capability |
| CargoPod | Personal cargo storage |
| BatteryPack | Extended power reserve |
| SolarPanel | Passive energy recharge |

### Derived Stats
Oxygen, power, cargo capacity, shield strength, and jetpack fuel are all computed from installed modules:
```cpp
void RigSystem::CalculateStats(RigLoadout& rig) {
    for (auto& mod : rig.installed_modules) {
        switch (mod.type) {
            case LifeSupport: rig.oxygen += mod.tier * BASE_O2; break;
            case PowerCore:   rig.power += mod.tier * BASE_POWER; break;
            case CargoPod:    rig.cargo += mod.tier * BASE_CARGO; break;
            case Shield:      rig.shield += mod.tier * BASE_SHIELD; break;
            case JetpackTank: rig.jetfuel += mod.tier * BASE_FUEL; break;
        }
    }
}
```

### Movement Modes
- **On Foot** — Standard first-person movement
- **In Rig** — Enhanced movement, different feel from on-foot
- **Full Rig** — Full power armor, different capabilities based on setup
- Movement while in armor is distinctly different from when on foot

### Ship Connection
The rig plugs into a port on the cockpit seat:
- Connects player to ship AI systems
- Provides HUD overlay (Elite Dangerous-style)
- Ship power supplements rig power while docked

---

## Multi-Tool System

### Overview
The multi-tool is the player's primary utility instrument. It is procedurally generated with swappable modules that change its function, stats, and visual appearance.

### Functions
| Function | Description |
|----------|-------------|
| Mining | Ore extraction, surface drilling |
| Salvaging | Component recovery, wreck cutting |
| Hacking | Lock bypass, system override |
| Scanning | Material analysis, hazard detection |
| Cutting | Hull penetration, material processing |
| Repair | Component restoration, seal repair |

### Module System
Multi-tool modules are interchangeable and follow the same modular system as all equipment:
- Each module changes function and stats
- Modules have tiers (1–5) affecting efficiency
- Visual appearance reflects installed modules

### Tool Upgrade Matrices

#### Salvage Cutter Upgrades
| Axis | Effect |
|------|--------|
| Precision | Narrower cut cone |
| Thermal Control | Less fire spread |
| Feedback | Joint stress warnings |
| Energy Efficiency | Longer EVA use |
| Safety Lockouts | Prevent catastrophic cuts |

#### Grapple / Tether Tool Upgrades
| Axis | Effect |
|------|--------|
| Dynamic Tension | Prevents snap |
| Mass Estimation | Predict momentum |
| Auto-Brake | Prevent spin deaths |
| Multi-anchor | Team pulls |

#### Scanner Tool Upgrades
| Axis | Effect |
|------|--------|
| Hazard Overlay | Fire/radiation visualization |
| Ghost Signal | AI echo detection |
| Structural Graph | Collapse risk assessment |
| Blueprint Trace | Rare tech identification |

### Design Rule
Tools do not become "stronger" — they become **safer, cleaner, smarter**. Precision and safety over raw power.

---

## Weapon System

### Overview
Weapons use the same modular system as multi-tools. From modular handguns to long guns, all use interchangeable modules.

### Weapon Classes
| Class | Description |
|-------|-------------|
| Sidearm | Compact, quick-draw, limited modules |
| Main Weapon | Full-size, multiple module slots |
| Multi-Tool | Utility-focused, switchable functions |

### Modular Configuration
- All weapons are PCG (procedurally generated)
- Modules change function, stats, and visual appearance
- Same module system across sidearm, main weapon, and multi-tool
- Modules are interchangeable between weapon classes where size permits

### Stats
Weapon stats derived entirely from installed modules — no hardcoded weapon archetypes.

---

## Module Size Compatibility

All module size distinctions carry over to everything in the game:

| Entity | Module Sizes Used |
|--------|------------------|
| Ship | XS through TITAN |
| Rover | XS through L |
| Grav Bike | XS only |
| Drone | XS through M |
| Player Rig | XS through S |
| Multi-Tool | XS |
| Weapons | XS through S |
| Planetary Base | S through XXL |

The same modular system governs all entities. Size and class constraints cascade from parent to child.

---

## Related Documents

- [Master Design Bible](MASTER_DESIGN_BIBLE.md) — complete system overview
- [Procedural Systems](PROCEDURAL_SYSTEMS.md) — recursive generation pipeline
- [Salvage & Legends](SALVAGE_AND_LEGENDS.md) — EVA salvage and legend system

---

*Extracted from iterative design sessions. Implementation status tracked in [ROADMAP](../ROADMAP.md).*
