# Procedural Content Generation (PCG) Framework

> Design document for the deterministic PCG systems used in EVE OFFLINE.

## Core Principle

Everything procedural must be a **pure function** of `(Seed + Rules + Version)`.

* No hidden state.
* No frame-dependent randomness.
* No order-dependent iteration.
* If you regenerate the same sector years later it must be **bit-identical**.

```
World = Generate(Seed, Rules, Version)
```

---

## 1. Deterministic Random & Seed System

**File:** `cpp_server/include/pcg/deterministic_rng.h`

Custom xorshift64\* RNG.  No `std::rand`, no `<random>`, no platform entropy.

### Seed Hierarchy

```
UniverseSeed
 └── GalaxySeed(galaxy_id)
      └── SystemSeed(system_id)
           └── SectorSeed(sector_coords)
                └── ObjectSeed(object_id)
```

Every object type (ships, asteroids, NPCs, loot, weapons) gets an independently regenerable seed sub-tree.

---

## 2. Hash Utilities

**File:** `cpp_server/include/pcg/hash_utils.h`

* `hashCombine(a, b)` — splitmix-style 64-bit combine.
* `hash64(a, b, c, d)` — four-input hash for context creation.
* `deriveSeed(parent, objectId)` — create a child seed.

---

## 3. PCG Context & Domain

**File:** `cpp_server/include/pcg/pcg_context.h`

Domains isolate seed trees so that changing one generator never affects another:

| Domain     | Purpose              |
|------------|----------------------|
| Universe   | Root seed            |
| Galaxy     | Galaxy-level layout  |
| StarSystem | Star system contents |
| Sector     | Local sector grid    |
| Ship       | Ship generation      |
| Asteroid   | Asteroid fields      |
| NPC        | NPC personalities    |
| Fleet      | Fleet compositions   |
| Loot       | Loot tables          |
| Mission    | Mission generation   |
| CapitalShip| Capital ship interiors|
| Station    | Station layout       |
| Salvage    | Salvage/debris fields|
| Rover      | Planetary rovers     |
| Anomaly    | Anomaly sites        |
| Encounter  | NPC encounters       |

---

## 4. PCG Manager

**File:** `cpp_server/include/pcg/pcg_manager.h`

Central authority.  Owns the universe seed and creates scoped `PCGContext` objects for every generator.  No generator may invent its own seed.

---

## 5. Ship Generator

**File:** `cpp_server/include/pcg/ship_generator.h`

Generates a `GeneratedShip` from a `PCGContext`.  Follows a strict hierarchy:

1. **Select Hull Class** — weighted (frigates common, capitals rare).
2. **Derive Stats** — mass, powergrid, CPU, capacitor from hull templates.
3. **Attach Engines** — thrust ≥ mass × minimum-acceleration.
4. **Attach Weapons** — within powergrid budget; trim if over.
5. **Validate** — all constraints must pass.

Hull classes: Frigate, Destroyer, Cruiser, Battlecruiser, Battleship, Capital.

---

## 6. Fleet Doctrine Generator

**File:** `cpp_server/include/pcg/fleet_doctrine.h`

Generates a full fleet composition with role-appropriate ships.

### Doctrines

| Doctrine       | DPS  | Tackle | Logi | Scout |
|----------------|------|--------|------|-------|
| Brawler        | 60 % | 20 %   | 15 % |  5 % |
| Sniper         | 70 % |  5 %   | 15 % | 10 % |
| Kite           | 50 % | 25 %   | 10 % | 15 % |
| Logistics      | 30 % | 10 %   | 50 % | 10 % |
| CapitalSupport | 40 % | 15 %   | 20 % | 25 % |

Each fleet always includes exactly one **Commander**.

---

## 7. Asteroid Field Generator

**File:** `cpp_server/include/pcg/asteroid_field_generator.h`

Generates a disc-shaped asteroid belt with mineral types determined by security level.

### Ore Distribution by Security Level

| Security    | Common Ores                | Rare Ores              |
|-------------|----------------------------|------------------------|
| High (≥0.5) | Veldspar, Scordite (75 %)  | Pyroxeres, Plagioclase |
| Low (0.2-0.5)| Mixed (40 % common)       | Omber, Kernite         |
| Null (≤0.2) | Sparse common (20 %)       | Jaspet, Mercoxit       |

Asteroids are scattered in a disc (XZ plane) with slight vertical spread.  Radius and mineral yield scale with ore rarity.

---

## 8. Anomaly Generator

**File:** `cpp_server/include/pcg/anomaly_generator.h`

Generates exploration / combat anomaly sites with layout, difficulty, and reward scaling.

### Site Types

| Type              | Content              | Waves |
|-------------------|----------------------|-------|
| CombatSite        | Hostiles + containers| 1-6   |
| GasSite           | Gas clouds           | 0     |
| RelicSite         | Containers + debris  | 0     |
| DataSite          | Data containers      | 0     |
| OreSite           | Rich ore clusters    | 0     |
| WormholeSignature | Wormhole entrance    | 0     |

Difficulty: Trivial → Easy → Medium → Hard → Extreme (scales with lower security).

---

## 9. NPC Encounter Generator

**File:** `cpp_server/include/pcg/npc_encounter_generator.h`

Creates multi-wave hostile encounters with faction-appropriate ship compositions.

### Factions

| Faction   | High-Sec | Low-Sec | Null-Sec |
|-----------|----------|---------|----------|
| Pirate    | 60 %     | 35 %    | 25 %     |
| Rogue     | 25 %     | 20 %    | 15 %     |
| Mercenary | 15 %     | 25 %    | 20 %     |
| Navy      | —        | 20 %    | 10 %     |
| Sleeper   | —        | —       | 30 %     |

Later waves spawn larger hulls; lower security spawns heavier compositions.  Each wave has a trigger delay and spawn radius.

---

## 10. General Best Practices

* **Seeded generation** — players can share universe seeds.
* **Control mechanisms** — tweak parameters (`max_ships`, `min_asteroid_size`).
* **LOD & object pooling** — already supported via `lod_system` and `spatial_hash_system`.
* **Save data stores only deltas** — damage, ownership, player mods.  Everything else regenerates from seed.
* **Versioning** — `PCGContext.version` enables safe rule migration across game updates.

---

## 11. Future Extensions

* PCG debug visualiser (seed → object tree).
* Constraint solver with retries (genetic-style fitting).
* Server-authoritative PCG verification for multiplayer.
* Asteroid field noise-based mesh generation.
* Procedural texturing (albedo, normal, light maps per faction).
* Spine-based hull grammar (Phase 12 overhaul).
