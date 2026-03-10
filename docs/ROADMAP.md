# Nova Forge — Project Roadmap

> **Consolidated roadmap** — For deep design detail, see the [Master Design Bible](design/MASTER_DESIGN_BIBLE.md).
> Archived planning docs are in [`docs/archive/`](archive/).

---

## Project Vision

Nova Forge is a **PvE-focused space simulator** for solo play or small groups (2–20 players), built on the custom **Atlas Engine** (C++/OpenGL). The game delivers EVE-like depth — ships, skills, fitting, combat, missions, exploration — powered by an AI-driven economy where every NPC is a real economic actor. The simulation is server-authoritative, tick-based (10–20 Hz), and deterministic for networking and replays. All UI is rendered through the custom Atlas UI retained-mode system with an EVE-style sci-fi dark theme. Content is fully moddable via JSON.

---

## Architecture

| Layer | Responsibility |
|-------|---------------|
| **Server (Authoritative)** | Tick-based simulation, ECS, AI, economy, combat, persistence |
| **Client (Display)** | Rendering, interpolation, UI, input, audio — no game logic authority |
| **Networking** | Server snapshots → Client cache → Interpolation → Render |

---

## Design Enforcement Rules (Non-Negotiable)

Every new system or feature **must** pass all three checks:

1. **Atlas-fit** — Does it belong naturally inside Atlas Engine?
2. **Simulation-first** — Does it work without UI, audio, or player input?
3. **Lore-consistent** — Would captains in-universe believe this exists?

If it fails any one → it doesn't ship.

---

## Systems Overview

| System | Key Subsystems | Status |
|--------|---------------|--------|
| **Atlas Engine Core** | ECS, Deterministic Scheduler, EventBus, Plugin System | ✅ Complete |
| **Networking** | TCP Client/Server, Interpolation, Delta Compression, Snapshot Replication, Jitter Buffer | ✅ Complete |
| **Rendering** | OpenGL, Deferred Pipeline, Shadow Mapping, Post-Processing | ✅ Complete |
| **Atlas UI** | Widgets, Panels, Docking (DockNode tree), Photon Dark Theme | ✅ Complete |
| **Atlas Editor** | 21 Panels, PCG Tools, Dock Layout, Debug Console | ✅ Complete |
| **Editor Tool Layer** | 32 ITool implementations, EditorCommandBus, UndoableCommandBus, DeltaEditStore | ✅ Complete |
| **Ship HUD** | Control Ring (shield/armor/hull arcs), Capacitor Bar, Module Rack, Target Brackets | ✅ Complete |
| **PCG Ship Generation** | Spine-based Hulls, Turrets, Faction Shape Language, Procedural Materials | ✅ Complete |
| **PCG Content** | Stations, Interiors, Characters, Galaxy Generation | ✅ Complete |
| **Combat** | Lock/Fire/Damage Layers, Wrecks, Salvage, Security Response | ✅ Complete |
| **Fleet System** | Wings (5×5), Doctrine, Morale, Chatter, Fracture Mechanics | ✅ Complete |
| **AI NPCs** | Miners, Haulers, Pirates, Traders, Security, Intent-driven Behavior Trees, NPC Schedules | ✅ Complete |
| **Economy** | Markets, Supply/Demand, Trade Routes, Dynamic Tax, Broker Fees | ✅ Complete |
| **Mining & Industry** | Ore, Ice, Refining, Manufacturing, Planetary Industry | ✅ Complete |
| **Missions** | Procedural Templates, Objectives, Rewards, Branching Chains, Consequences | ✅ Complete |
| **Exploration** | Anomalies, Wormholes, Scan Probes, Jump Gates | ✅ Complete |
| **Audio** | OpenAL, Positional 3D, Fleet Audio, Warp Adaptive Audio | ✅ Complete |
| **Physics** | Rigid Body Dynamics, AABB Collision | ✅ Complete |
| **Animation** | Graph Pipeline, Clip/Blend/Modifier/StateMachine Nodes | ✅ Complete |
| **Warp System** | Tunnel Shader Stack, Adaptive Audio, Mass-based Intensity, Anomalies | ✅ Complete |
| **Titan & Meta-Threat** | Assembly Pressure, Logistics Distortion, Rumor Propagation, Galactic Response | ✅ Complete |
| **FPS & Interiors** | EVA Airlock, Salvage Paths, Lavatory, Rig Locker, Clone Bay | ✅ Complete |
| **Planetary** | Space→Planet Transition, Rover Bay, Grid Construction, Hangars | ✅ Complete |
| **Community & Modding** | Content Repository, Validation, Mission Editor, Mod Manager, Doc Generator | ✅ Complete |
| **Vertical Slice Integration** | Full star system: fly, fight, mine, trade, dock — end-to-end | 🔧 In Progress |
| **Client Polish & Optimization** | Profiling, Spatial Partitioning, 500+ Entity Performance | 🔧 In Progress |
| **Steam / Distribution** | Packaging, Installer, Steam Integration | 📋 Planned |

---

## Development Phases

| Phase | Name | Version | Status |
|-------|------|---------|--------|
| **0** | Foundation — Engine, rendering, ECS, basic flight, HUD, audio | v0.1 | ✅ Complete |
| **1** | Core Fleet — Player + 4 captains, fitting, mining, warp, stations | v0.1–v0.2 | ✅ Complete |
| **2** | Living Universe — Background sim, NPC life, economy engine, combat aftermath | v0.2 | ✅ Complete |
| **3** | Wing System & Midgame — 3×5 wings, doctrine, tactical overlay, imperfect info | v0.3 | ✅ Complete |
| **4** | Full Fleet Doctrine & Endgame — 5×5 fleet, ideology, fracture, persistence | v0.4+ | ✅ Complete |
| **5** | Titan Systems & Meta-Threat — Galactic emergent threats, warp anomalies | v0.5+ | ✅ Complete |
| — | Vertical Slice — One full star system, end-to-end playable | — | 🔧 In Progress |
| — | Client Integration & Polish — Smooth networking, full HUD, performance | — | 🔧 In Progress |

---

## Current Priorities

1. **Vertical Slice** — One full star system: fly, dock, mine, trade, fight, warp — end-to-end playable
2. **Client–Server Integration** — Wire up all server systems to C++ client rendering and UI
3. **Performance Profiling** — Maintain 20 Hz server tick with 500+ entities; 60 FPS client with 200 visible ships
4. **Atlas UI Polish** — All game panels using Atlas UI docking system with EVE-style theme
5. **Network Smoothness** — Interpolation, jitter buffer, and lag compensation feel solid at 100ms latency
6. **AI Economic Actors Live** — Miners, haulers, pirates creating visible supply/demand cycles
7. **Content Balance Pass** — Missions, rewards, difficulty scaling tuned for solo and co-op play

---

## Explicitly Out of Scope

- ❌ PvP combat
- ❌ Player empires / sovereignty
- ❌ Full MMO economy simulation
- ❌ Scripted storylines / quest chains
- ❌ Twitch shooter mechanics
- ❌ Clickable tactical overlays
- ❌ Fake distances or rubber-band difficulty
- ❌ Asset-copied EVE clone
- ❌ Client-authoritative logic
- ❌ Theme-park content

---

## Development Milestones

| Milestone | Target | Criteria |
|-----------|--------|----------|
| **Engine Baseline** | ✅ Done | ECS, rendering, audio, physics, networking, UI framework |
| **All Server Systems** | ✅ Done | 17378+ test assertions passing across 365 ECS systems |
| **Atlas Engine Modules** | ✅ Done | 915 engine test assertions (input, camera, audio, animation, plugin) |
| **Editor Tool Layer** | ✅ Done | 32 tools, 17 panels, EditorCommandBus, UndoableCommandBus, DeltaEditStore, 374+ test assertions |
| **Vertical Slice** | In Progress | One star system fully playable end-to-end |
| **Alpha** | TBD | 2–4 players, smooth networking, core gameplay loop |
| **Beta** | TBD | 8+ players, full UI, content variety, performance targets met |
| **Release** | TBD | Packaged builds, mod support, documentation |

---

## Naming Conventions

| Scope | Prefix | Example |
|-------|--------|---------|
| Engine-wide | `Atlas` | `AtlasDeterministicScheduler` |
| Simulation | `Sim` | `SimFleetMoraleComponent` |
| Rendering | `AtlasRender` | `AtlasRenderTacticalOverlay` |
| UI | `UI_` | `UI_TacticalOverlayRing` |
| Game-specific | `EVO_` | `EVO_PirateCoalitionDoctrine` |

**Forbidden:** Generic names (`Manager`, `Controller`, `Handler`), engine-agnostic names (`InventorySystem`).
Components = pure data, no logic, serializable. Systems = stateless logic, deterministic, scheduled by Atlas ECS.

---

*For detailed system designs, see the [Master Design Bible](design/MASTER_DESIGN_BIBLE.md).*
*Archived planning documents: [`docs/archive/`](archive/)*
