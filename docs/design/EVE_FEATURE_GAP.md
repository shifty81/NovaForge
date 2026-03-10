# EVE Online Feature Gap Analysis

## Overview
This document analyzes gameplay features from EVE Online that are missing or incomplete in Nova Forge, prioritized by importance for a complete PVE experience.

## Critical Missing Features (High Priority)

### 1. **Sovereignty & Security Status System**
**EVE Online Implementation:**
- Security status ranges from 1.0 (high-sec) to -1.0 (null-sec)
- AEGIS response in high-sec
- Gate guns and faction police
- Criminal flagging and suspect timers

**Current Status:** Data structures defined (`data/security/aegis_and_insurance.json`)
**Implementation Needed:**
- [x] AEGIS response time data per security level
- [x] Criminal flagging mechanics (suspect, criminal, weapons timer)
- [x] Security status tracking range (-10.0 to +10.0) with effects
- [x] Faction standing system consequences
- [x] Runtime AEGIS NPC spawning in server code
- [x] Gate gun AI behavior

---

### 2. **Advanced NPC AI & Aggression**
**EVE Online Implementation:**
- NPC rats spawn in belts and anomalies
- Aggression tables and priority targeting
- Rat types (frigates, cruisers, battleships) with behaviors
- Officer spawns and faction warfare NPCs

**Current Status:** Basic AI exists (approach, orbit, attack)
**Implementation Needed:**
- [x] Aggression switching based on threat/DPS
- [x] NPC fleet compositions
- [x] Officer/Commander spawns with better loot
- [x] Sleeper AI (wormhole space)
- [x] Drifter/Triglavian equivalents

---

### 3. **Bounty & Reward System**
**EVE Online Implementation:**
- Credits bounties on NPC kills
- Mission rewards (Credits + LP + items)
- Insurance payouts
- Salvage from wrecks

**Current Status:** Basic bounty values in NPC data, insurance data defined
**Implementation Needed:**
- [x] Automatic bounty payout on NPC destruction
- [x] Loyalty Points (LP) system
- [x] LP stores with unique items
- [x] Insurance contract data (`data/security/aegis_and_insurance.json`)
- [x] Salvaging mechanics with modules

---

### 4. **Wormhole & Null-Sec Mechanics**
**EVE Online Implementation:**
- Wormhole space with no local chat
- Sleeper NPCs and blue loot
- Null-sec with no AEGIS
- Anomalies and signatures that spawn dynamically

**Current Status:** Basic exploration signatures exist
**Implementation Needed:**
- [x] Wormhole generation and connections
- [x] Wormhole mass/time limits
- [x] Sleeper cache sites
- [x] Null-sec specific mechanics (bubbles, cynos)
- [x] Dynamic anomaly spawning

---

### 5. **Market & Economy**
**EVE Online Implementation:**
- Buy/sell orders with escrow
- Regional markets
- Market hubs (Jita, Amarr)
- Order expiration and fees
- Hauling and logistics

**Current Status:** Basic market data structure, contract/escrow data defined
**Implementation Needed:**
- [x] Order matching engine
- [x] Broker fees and sales tax data (`data/contracts/contracts.json`)
- [x] Order modification/cancellation
- [x] Regional price differences
- [x] Market API for clients (`market_api_system.h`)

---

## Medium Priority Features

### 6. **Clone & Medical System**
**EVE Online Implementation:**
- Relay clones for fast travel
- Medical clones with implants
- Skill point loss on pod death
- Clone grades

**Current Status:** Data structures defined (`data/character_creation/clones.json`, `data/character_creation/implants.json`)
**Implementation Needed:**
- [x] Clone grades and skill point retention data
- [x] Relay clone mechanics (24-hour cooldown, Infomorph Psychology skill)
- [x] Implant system (5 attribute slots, 4 grades)
- [x] Death mechanics (pod kill, skill point loss, implant loss)
- [x] Clone bay station service in server code
- [x] Relay clone installation UI (`relay_clone_install_ui_system.h`)

---

### 7. **Planetary Operations (PI)**
**EVE Online Implementation:**
- Extract resources from planets
- Build command centers
- Factory planets for production chains
- Export products

**Current Status:** Not implemented
**Implementation Needed:**
- [x] Planet scanning (`planet_scan_system.h`)
- [x] Resource extraction (`planet_scan_system.h`, `pi_system.h`)
- [x] Production chains (`pi_system.h`)
- [x] Customs offices (`pi_customs_system.h`)

---

### 8. **Incursions & Live Events**
**EVE Online Implementation:**
- Sansha's Nation incursions
- High/Mid/Low-sec sites
- Influence levels
- Community goals

**Current Status:** Static mission system only
**Implementation Needed:**
- [x] Dynamic incursion spawning
- [x] Multiple difficulty tiers
- [x] Fleet coordination rewards
- [x] Influence mechanics

---

### 9. **Advanced Fitting & Modules**
**EVE Online Implementation:**
- Tech II modules (better stats, higher requirements)
- Faction modules (rare drops)
- Deadspace modules (from exploration)
- Mutated modules (Abyssal)
- Implants and boosters

**Current Status:** Basic modules exist, implant data defined
**Implementation Needed:**
- [x] Tech II module variants (`tech2_module_system.h`)
- [x] Faction module drops (`tech2_module_system.h`)
- [x] Deadspace loot tables (`tech2_module_system.h`)
- [x] Meta level system (0-5+)
- [x] Implant data with slots and grades (`data/character_creation/implants.json`)

---

### 10. **Industry & Manufacturing**
**EVE Online Implementation:**
- Blueprint research (ME/TE)
- Invention (T2 blueprints)
- Reactions (moon goo)
- Capital ship construction
- Structure building

**Current Status:** Basic blueprint system
**Implementation Needed:**
- [x] Material Efficiency research
- [x] Time Efficiency research
- [x] Invention mechanics
- [x] Reaction formulas
- [x] Capital components (`capital_component_system.h`)

---

## Lower Priority (Polish Features)

### 11. **Character Customization**
- [x] Ship skins and paint jobs (`ship_skin_system.h`)
- [x] Character portraits (`character_portrait_system.h`)
- [x] Corporation logos (`corporation_logo_system.h`)
- [x] Structure skins (`structure_skin_system.h`)

### 12. **Social Features (Enhanced)**
- [x] Alliance system (`alliance_management_system.h`)
- Corporation wars
- Wardec mechanics
- [x] Citadel structures (`citadel_management_system.h`)

### 13. **Advanced Movement**
- [x] Microwarp drive signature bloom (`propulsion_module_system.h`)
- [x] Afterburner mechanics (`propulsion_module_system.h`)
- [x] Inertia modifiers (`inertia_modifier_system.h`)
- Warp disruption bubbles (null-sec)

### 14. **EWAR Expanded**
- [x] Remote sensor dampening (`sensor_dampening_system.h`)
- [x] ECM jamming (`ecm_jamming_system.h`)
- [x] Tracking disruption (`tracking_disruption_system.h`)
- [x] Guidance disruption (missiles) (`tracking_disruption_system.h`)

### 15. **Abyssal Deadspace**
- [x] Filament entry system (`abyssal_filament_system.h`)
- [x] Time-limited pockets (`abyssal_filament_system.h`)
- [x] Escalating difficulty (`abyssal_escalation_system.h`, `abyssal_weather_system.h`)
- [x] Mutaplasmid loot (`mutaplasmid_system.h`)

---

## Recommended Implementation Order

### Phase 1: Core Economy & Rewards
1. Bounty payout system
2. Market order execution
3. Salvaging mechanics
4. Loot system improvements

### Phase 2: Advanced Combat
1. Improved NPC AI
2. Officer spawns
3. Security status system
4. AEGIS mechanics

### Phase 3: Exploration & Space
1. Wormhole system
2. Dynamic anomaly spawning
3. Sleeper NPCs
4. Null-sec mechanics

### Phase 4: Industry & Progression
1. Blueprint research
2. Invention system
3. Clone system
4. Implants

### Phase 5: Social & Endgame
1. Incursions
2. Planetary Operations
3. Advanced corporation features
4. Alliance mechanics

---

## EVE-Specific Mechanics to Avoid

These EVE features are not suitable for a small-group PVE focus:

❌ **Player vs Player (PvP)**
- Ganking mechanics
- War declarations
- FW (Faction Warfare)
- Sov warfare

❌ **Extreme Grind**
- Months-long skill training
- Capital ship requirements
- Alliance politics

❌ **Market PvP**
- Market manipulation
- Station trading bots
- Scamming contracts

---

## Conclusion

To achieve a complete EVE-like PVE experience, focus on:
1. **Economy loop**: Kill NPCs → Get bounties → Buy better ships/modules
2. **Risk vs Reward**: High-sec (safe) → Null-sec (dangerous but profitable)
3. **Progression**: Frigates → Cruisers → Battleships → Capitals
4. **Content variety**: Missions, Exploration, Mining, Industry
5. **Social features**: Corporations, Fleet operations, Shared goals

The current implementation has ~70% of EVE's PVE features (data structures for most systems, runtime code for core gameplay). Completing the remaining runtime implementation would bring it to ~90% feature parity.
