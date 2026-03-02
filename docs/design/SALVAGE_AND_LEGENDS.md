# NovaForge — Salvage & Legend Systems

> Ship lifecycle: Construction → Combat → Damage → Wreck → Salvage → Legend.
> The game remembers patterns, not incidents.

---

## Ship Lifecycle

Every ship in NovaForge follows a complete lifecycle:

```
Construction → Combat → Damage → Wreck → Salvage → Legend
```

- **Construction** — Modular assembly from interior modules
- **Combat** — Module-level damage, cascading failures
- **Damage** — Scars, breaches, system loss (persistent)
- **Wreck** — Persistent world object after destruction
- **Salvage** — Player gameplay loop (EVA or ship-based)
- **Legend** — World remembers, myths form, future generation affected

Ships can also be **retired** to hangar display, preserving their history.

---

## EVA Salvage System

### Overview
Inspired by Shipbreaker gameplay — players can EVA to derelict wrecks and salvage them by hand using multi-tools, or use ship-based salvage systems for automated recovery.

### EVA Movement
- Inertia-based: players drift in zero-G
- Tether to home ship routes power and oxygen
- Auto-retract on return to airlock
- Tool recoil affects player momentum

### Salvage Operations
| Operation | Description |
|-----------|-------------|
| Module Processing | Disassemble modules for materials |
| Repair Intact Modules | Recover working components |
| Blueprint Reconstruction | Reverse-engineer technology |
| Wreck Stabilization | Secure structure before salvage |
| EVA Resupply | Restock oxygen, power, tools |

### Co-op Salvage Rules
- Shared physics consequences (one player's cut affects all)
- Environmental failure, never griefing
- Tool recoil, mass, decompression affect all nearby players
- Coordinated pulls with multi-anchor tethers

---

## Ghost Data System

Ghost behavior emerges from failed AI state persistence in wrecks — no magic, pure systems.

### Mathematical Model

```
GhostIntensity = (AI_Trauma × SystemIntegrity × TimeDecay) / SalvageInterference
```

| Variable | Range | Description |
|----------|-------|-------------|
| AI_Trauma | 0–1 | How violently the ship's AI was disrupted |
| SystemIntegrity | 0–1 | How intact the wreck's systems remain |
| TimeDecay | e^(-t/k) | Natural decay over time |
| SalvageInterference | 1–∞ | Player salvage activity reduces ghosts |

### Ghost Intensity Effects

| Intensity | Effect |
|-----------|--------|
| < 0.2 | Passive echoes (audio, flickering) |
| 0.2–0.5 | System glitches (doors, lights, comms) |
| 0.5–0.8 | Active hazards (systems activate, gravity shifts) |
| > 0.8 | Hostile automation (defense systems engage) |

### Player Interaction
- Cutting power reduces SystemIntegrity → reduces ghost capability
- Careful stabilization reduces AI_Trauma → safer salvage
- Rushing salvage increases ghost risk
- Reckless play is objectively worse than methodical approach

---

## Legendary Wrecks

Legendary wrecks are hand-authored templates with procedural variance — designed, not random.

### Structure
```cpp
struct LegendaryWreck {
    ShipClass originClass;
    Faction originFaction;
    UniqueFailureMode failure;
    GhostProfile ghost;
    SalvageCeiling maxRecovery;
};
```

### Templates

#### ☢️ "The Reactor Saint"
- Titan-class, reactor exploded mid-jump
- Massive radiation hazard
- Ghost loops emergency hymns
- Requires stabilization before salvage

#### 🛡️ "The Last Wall"
- Juggernaut, armor intact but interior collapsed
- Defensive AI still active
- Salvage = siege-style gameplay

#### 🕳️ "The Vanishing Carrier"
- Supercarrier, hangars torn open
- Fighters embedded inside hull
- Gravity anomalies throughout

### End States (Player-Driven)

| Outcome | Result |
|---------|--------|
| Full Salvage | Legendary modules and materials |
| Stabilize | Permanent explorable dungeon |
| Convert | Operational salvage station |
| Memorialize | World landmark / monument |

---

## Legend Taxonomy

### The Golden Rule
> The game remembers **patterns**, not **incidents**.

If you remember everything → noise. If you remember nothing → no myth.

### Memory Tiers

#### 🥇 Tier 1 — Persistent Legends (Never Forgotten)
Rare, powerful, identity-defining. Change future generation rules.

**Examples:**
- Signature playstyle dominance (stealth-heavy, explosive-heavy)
- Boss archetypes defeated
- Campaign-ending choices
- World-scale failures or victories

```cpp
struct PersistentLegend {
    LegendType type;
    float intensity;  // grows slowly
};
```

**Effects:** Alters probabilities, enemy doctrine, boss construction

#### 🥈 Tier 2 — Campaign Memory (Fades Slowly)
Matters within and between campaigns, but not forever.

**Examples:**
- How a faction was treated
- Which mission types succeeded
- Which biomes were most lethal

```cpp
struct CampaignMemory {
    MemoryType type;
    float weight;  // slowly decays
};
```

**Effects:** Biases mission generation, narrative beats

#### 🥉 Tier 3 — Run Memory (Ephemeral)
Used for moment-to-moment AI adaptation only.

**Examples:**
- Player camping patterns
- Weapon overuse
- Repeated retreat patterns

```cpp
struct RunMemory {
    TacticalPattern pattern;
};
```

**Effects:** AI adapts in real-time
**Not saved, never becomes legend**

#### ❌ Explicitly Forgotten (By Design)
These must NEVER persist:
- Individual deaths
- Exact room layouts
- Specific weapon loadouts
- RNG outcomes
- Momentary mistakes

Forgetting is what keeps legends mythic.

### Promotion Rules
A memory only becomes a Legend if:
1. It appears across **multiple runs**
2. It survives **different biomes**
3. It meaningfully **affects difficulty**

```cpp
if (pattern.repeatsAcrossRuns() && pattern.isDominant())
    PromoteToLegend();
```

---

## Boss Mutation System

Bosses are procedural organisms, not characters. They mutate based on legend pressure, not player stats.

### Boss Anatomy
```cpp
struct Boss {
    BossCore core;                    // Never changes
    vector<BossModule> modules;       // Mutate
    BossBehaviorProfile behavior;     // Adapts
};
```

**Core (Immutable):** Identity silhouette, arena scale, movement class
**Modules (Mutable):** Shields, weakpoints, minion spawners, environmental hazards

### Mutation Inputs

| Legend Trait | Boss Response |
|-------------|--------------|
| High explosives | Shield phasing |
| High precision | Moving weakpoints |
| High stealth | Area scans |
| High aggression | Counter-rush modules |

### Mutation Rules
- **Max 2 mutations per boss**
- **Never remove all weaknesses**
- Bosses telegraph changes visually
- Bosses never become bullet sponges
- Bosses adapt to **habits**, not skill

> "It's the same boss… but it learned something."

---

## Campaign Structure

### Campaign = 3–5 Chapters
Each chapter has a **theme**, not a plot. Infinite variation, finite meaning.

| Chapter | Theme | Purpose |
|---------|-------|---------|
| 1. Incursion | Entry into unknown space | Orientation |
| 2. Escalation | Enemy coordination increases | Tension |
| 3. Fracture | Breaches, zero-G, ambiguity | Instability |
| 4. Confrontation | Boss encounter, high legend expression | Resolution |
| 5. Aftermath (optional) | Environmental storytelling | Memory formation |

### Campaign Rules
- Campaigns never guarantee success
- Failure still produces legend
- Campaigns end, legends persist

---

## Player Progression Trees

Progression is **horizontal + mastery-based**, not power creep. No classes, no respec punishment. Skills unlock capabilities, not raw damage.

### 🌲 EVA Operations Tree
*"Stay alive, move better, cut smarter"*

| Tier | Unlocks |
|------|---------|
| I | Reduced inertia drift |
| II | Oxygen efficiency |
| III | Zero-G rotation dampening |
| IV | Emergency thruster burst |
| V | EVA near-reactor tolerance |

Affects: EVA resupply efficiency, survival during wreck stabilization

### 🌲 Salvage Engineering Tree
*"Extract intact value"*

| Tier | Unlocks |
|------|---------|
| I | Basic module detachment |
| II | Joint stress visualization |
| III | Intact module recovery chance |
| IV | Live system disconnection |
| V | Reactor-safe extraction |

Feeds: module processing, repair intact modules

### 🌲 Analysis & Data Tree
*"Turn wrecks into knowledge"*

| Tier | Unlocks |
|------|---------|
| I | Wreck value scan |
| II | Hazard prediction |
| III | Ghost data analysis |
| IV | Blueprint reconstruction |
| V | Legendary wreck identification |

Feeds: blueprint reconstruction, ghost AI understanding

### 🌲 Infrastructure & Logistics Tree
*"Make salvage scalable"*

| Tier | Unlocks |
|------|---------|
| I | Portable EVA resupply |
| II | Wreck stabilization fields |
| III | Station module efficiency |
| IV | Multi-wreck processing |
| V | Legendary wreck anchoring |

Feeds: wreck stabilization, EVA resupply, station expansion

### Station Integration

| Station Function | Feeds From |
|-----------------|------------|
| Module Processing | Salvage Engineering |
| Repair Intact Modules | Tool precision + skill |
| Blueprint Reconstruction | Analysis tree |
| Wreck Stabilization | Infrastructure tree |
| EVA Resupply | Logistics upgrades |

Stations are the spine of progression, not an afterthought.

---

## Player Progression Path

```
Pilot → Captain → Admiral → Architect
```

| Stage | Capabilities |
|-------|-------------|
| Pilot | Fly ships, basic salvage |
| Captain | Command fleets, design ships |
| Admiral | Fleet doctrine, multi-system operations |
| Architect | Build Titans, shape the universe |

---

## Related Documents

- [Master Design Bible](MASTER_DESIGN_BIBLE.md) — complete system overview
- [Procedural Systems](PROCEDURAL_SYSTEMS.md) — recursive generation pipeline
- [Vehicles & Equipment](VEHICLES_AND_EQUIPMENT.md) — rover, grav bike, drone, rig, multi-tool specs

---

*Extracted from iterative design sessions. Implementation status tracked in [ROADMAP](../ROADMAP.md).*
