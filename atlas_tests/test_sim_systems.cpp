/**
 * Tests for standalone PCG simulation systems:
 *   - GalaxyGenerator
 *   - EconomyDrivenGenerator
 *   - NPCEncounterGenerator
 *   - DamageStateGenerator
 *
 * These systems operate independently of the GenerationStyle engine and
 * are used directly by the game-server and editor tools.
 */

#include "../cpp_server/include/pcg/galaxy_generator.h"
#include "../cpp_server/include/pcg/economy_driven_generator.h"
#include "../cpp_server/include/pcg/npc_encounter_generator.h"
#include "../cpp_server/include/pcg/damage_state_generator.h"
#include "../cpp_server/include/pcg/pcg_manager.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::pcg;

// ══════════════════════════════════════════════════════════════════════
// GalaxyGenerator tests
// ══════════════════════════════════════════════════════════════════════

void test_galaxy_generate_default() {
    PCGManager mgr;
    mgr.initialize(42);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Galaxy, 1, 1);

    GeneratedGalaxy g = GalaxyGenerator::generate(ctx, 50);

    assert(g.valid);
    assert(g.total_systems == 50);
    assert(static_cast<int>(g.nodes.size()) == 50);
    assert(!g.routes.empty());
}

void test_galaxy_system_count_clamped() {
    PCGManager mgr;
    mgr.initialize(1);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Galaxy, 1, 1);

    // Below minimum → clamped to 10.
    GeneratedGalaxy g = GalaxyGenerator::generate(ctx, 3);
    assert(g.valid);
    assert(g.total_systems == 10);
    assert(static_cast<int>(g.nodes.size()) == 10);
}

void test_galaxy_security_zone_counts() {
    PCGManager mgr;
    mgr.initialize(99);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Galaxy, 1, 1);

    GeneratedGalaxy g = GalaxyGenerator::generate(ctx, 100);

    assert(g.valid);
    // All systems must be classified.
    int total = g.highsec_count + g.lowsec_count + g.nullsec_count;
    assert(total == g.total_systems);
    // A 100-system galaxy should have at least some of each zone.
    assert(g.highsec_count > 0);
    assert(g.lowsec_count  > 0);
    assert(g.nullsec_count > 0);
}

void test_galaxy_all_nodes_connected() {
    PCGManager mgr;
    mgr.initialize(7);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Galaxy, 1, 1);

    GeneratedGalaxy g = GalaxyGenerator::generate(ctx, 30);
    assert(g.valid);

    // Every node must have at least one neighbour.
    for (const auto& node : g.nodes) {
        assert(!node.neighbor_ids.empty());
    }
}

void test_galaxy_chokepoints_exist() {
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Galaxy, 1, 1);

    GeneratedGalaxy g = GalaxyGenerator::generate(ctx, 50);
    assert(g.valid);

    // A connected graph with 50 nodes should have chokepoints.
    int cps = 0;
    for (const auto& r : g.routes) {
        if (r.is_chokepoint) ++cps;
    }
    assert(cps > 0);
}

void test_galaxy_determinism() {
    PCGManager mgr1, mgr2;
    mgr1.initialize(888);
    mgr2.initialize(888);

    PCGContext ctx1 = mgr1.makeRootContext(PCGDomain::Galaxy, 1, 1);
    PCGContext ctx2 = mgr2.makeRootContext(PCGDomain::Galaxy, 1, 1);

    GeneratedGalaxy g1 = GalaxyGenerator::generate(ctx1, 40);
    GeneratedGalaxy g2 = GalaxyGenerator::generate(ctx2, 40);

    assert(g1.total_systems == g2.total_systems);
    assert(g1.nodes.size()  == g2.nodes.size());
    assert(g1.routes.size() == g2.routes.size());
    assert(g1.highsec_count == g2.highsec_count);
    assert(g1.nullsec_count == g2.nullsec_count);
}

void test_galaxy_different_seeds_differ() {
    PCGManager mgr1, mgr2;
    mgr1.initialize(1);
    mgr2.initialize(2);

    PCGContext ctx1 = mgr1.makeRootContext(PCGDomain::Galaxy, 1, 1);
    PCGContext ctx2 = mgr2.makeRootContext(PCGDomain::Galaxy, 1, 1);

    GeneratedGalaxy g1 = GalaxyGenerator::generate(ctx1, 50);
    GeneratedGalaxy g2 = GalaxyGenerator::generate(ctx2, 50);

    assert(g1.valid && g2.valid);
    // At least the first node's ID should differ.
    assert(g1.nodes[0].system_id != g2.nodes[0].system_id);
}

void test_galaxy_security_level_range() {
    PCGManager mgr;
    mgr.initialize(55);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Galaxy, 1, 1);

    GeneratedGalaxy g = GalaxyGenerator::generate(ctx, 80);
    assert(g.valid);

    for (const auto& node : g.nodes) {
        assert(node.security_level >= 0.0f);
        assert(node.security_level <= 1.0f);
    }
}

void test_galaxy_security_zone_name() {
    assert(GalaxyGenerator::securityZoneName(SecurityZone::HighSec) == "High-Sec");
    assert(GalaxyGenerator::securityZoneName(SecurityZone::LowSec)  == "Low-Sec");
    assert(GalaxyGenerator::securityZoneName(SecurityZone::NullSec) == "Null-Sec");
}

void test_galaxy_route_distances_positive() {
    PCGManager mgr;
    mgr.initialize(21);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Galaxy, 1, 1);

    GeneratedGalaxy g = GalaxyGenerator::generate(ctx, 30);
    assert(g.valid);

    for (const auto& r : g.routes) {
        assert(r.distance > 0.0f);
    }
}

// ══════════════════════════════════════════════════════════════════════
// EconomyDrivenGenerator tests
// ══════════════════════════════════════════════════════════════════════

void test_economy_generate_default() {
    PCGManager mgr;
    mgr.initialize(42);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::EconomyFleet, 1, 1);

    GeneratedEconomyFleet fleet = EconomyDrivenGenerator::generate(
        ctx, EconomyState::Prosperous, 5);

    assert(fleet.valid);
    assert(fleet.total_ships == 5);
    assert(static_cast<int>(fleet.ships.size()) == 5);
    assert(fleet.economy == EconomyState::Prosperous);
}

void test_economy_resource_rich_has_miners() {
    PCGManager mgr;
    mgr.initialize(11);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::EconomyFleet, 1, 1);

    // With enough ships, a ResourceRich economy should include miners.
    GeneratedEconomyFleet fleet = EconomyDrivenGenerator::generate(
        ctx, EconomyState::ResourceRich, 20);

    assert(fleet.valid);
    bool hasMiner = false;
    for (const auto& s : fleet.ships) {
        if (s.role == EconomyShipRole::Miner) { hasMiner = true; break; }
    }
    assert(hasMiner);
}

void test_economy_lawless_has_pirates() {
    PCGManager mgr;
    mgr.initialize(77);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::EconomyFleet, 1, 1);

    GeneratedEconomyFleet fleet = EconomyDrivenGenerator::generate(
        ctx, EconomyState::Lawless, 20);

    assert(fleet.valid);
    bool hasPirate = false;
    for (const auto& s : fleet.ships) {
        if (s.role == EconomyShipRole::Pirate) { hasPirate = true; break; }
    }
    assert(hasPirate);
}

void test_economy_quality_range() {
    PCGManager mgr;
    mgr.initialize(33);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::EconomyFleet, 1, 1);

    GeneratedEconomyFleet fleet = EconomyDrivenGenerator::generate(
        ctx, EconomyState::Declining, 10);

    assert(fleet.valid);
    for (const auto& s : fleet.ships) {
        assert(s.equipment_quality >= 0.0f && s.equipment_quality <= 1.0f);
        assert(s.damage_wear       >= 0.0f && s.damage_wear       <= 1.0f);
    }
}

void test_economy_determinism() {
    PCGManager mgr1, mgr2;
    mgr1.initialize(55);
    mgr2.initialize(55);

    auto ctx1 = mgr1.makeRootContext(PCGDomain::EconomyFleet, 1, 1);
    auto ctx2 = mgr2.makeRootContext(PCGDomain::EconomyFleet, 1, 1);

    auto f1 = EconomyDrivenGenerator::generate(ctx1, EconomyState::WarTorn, 8);
    auto f2 = EconomyDrivenGenerator::generate(ctx2, EconomyState::WarTorn, 8);

    assert(f1.valid && f2.valid);
    assert(f1.total_ships == f2.total_ships);
    assert(f1.ships.size() == f2.ships.size());
    assert(f1.ships[0].role == f2.ships[0].role);
}

void test_economy_state_names() {
    assert(!EconomyDrivenGenerator::economyStateName(EconomyState::Prosperous).empty());
    assert(!EconomyDrivenGenerator::economyStateName(EconomyState::ResourceRich).empty());
    assert(!EconomyDrivenGenerator::economyStateName(EconomyState::WarTorn).empty());
    assert(!EconomyDrivenGenerator::economyStateName(EconomyState::Declining).empty());
    assert(!EconomyDrivenGenerator::economyStateName(EconomyState::Lawless).empty());
}

void test_economy_role_names() {
    assert(!EconomyDrivenGenerator::shipRoleName(EconomyShipRole::Miner).empty());
    assert(!EconomyDrivenGenerator::shipRoleName(EconomyShipRole::Hauler).empty());
    assert(!EconomyDrivenGenerator::shipRoleName(EconomyShipRole::Patrol).empty());
    assert(!EconomyDrivenGenerator::shipRoleName(EconomyShipRole::Pirate).empty());
    assert(!EconomyDrivenGenerator::shipRoleName(EconomyShipRole::Scavenger).empty());
    assert(!EconomyDrivenGenerator::shipRoleName(EconomyShipRole::Trader).empty());
    assert(!EconomyDrivenGenerator::shipRoleName(EconomyShipRole::MilitaryEscort).empty());
}

void test_economy_war_torn_has_armed_ships() {
    PCGManager mgr;
    mgr.initialize(22);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::EconomyFleet, 1, 1);

    GeneratedEconomyFleet fleet = EconomyDrivenGenerator::generate(
        ctx, EconomyState::WarTorn, 15);

    assert(fleet.valid);
    bool hasArmed = false;
    for (const auto& s : fleet.ships) {
        if (s.is_armed) { hasArmed = true; break; }
    }
    assert(hasArmed);
}

// ══════════════════════════════════════════════════════════════════════
// NPCEncounterGenerator tests
// ══════════════════════════════════════════════════════════════════════

void test_encounter_generate_default() {
    PCGManager mgr;
    mgr.initialize(42);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Encounter, 1, 1);

    NPCEncounter enc = NPCEncounterGenerator::generate(ctx, 0.5f);

    assert(enc.valid);
    assert(!enc.waves.empty());
    assert(enc.totalShips > 0);
    assert(enc.difficultyRating >= 0.0f && enc.difficultyRating <= 1.0f);
    assert(enc.estimatedBounty > 0.0f);
}

void test_encounter_wave_count_explicit() {
    PCGManager mgr;
    mgr.initialize(13);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Encounter, 1, 1);

    NPCEncounter enc = NPCEncounterGenerator::generate(ctx, 3, 0.3f);

    assert(enc.valid);
    assert(static_cast<int>(enc.waves.size()) == 3);
}

void test_encounter_high_sec_easier() {
    PCGManager mgr1, mgr2;
    mgr1.initialize(7);
    mgr2.initialize(7);

    auto ctx1 = mgr1.makeRootContext(PCGDomain::Encounter, 1, 1);
    auto ctx2 = mgr2.makeRootContext(PCGDomain::Encounter, 1, 1);

    // Same seed, different security: null-sec should be harder.
    NPCEncounter hs = NPCEncounterGenerator::generate(ctx1, 1, 1.0f);  // high-sec
    NPCEncounter ns = NPCEncounterGenerator::generate(ctx2, 1, 0.0f);  // null-sec

    assert(hs.valid && ns.valid);
    assert(hs.difficultyRating <= ns.difficultyRating);
}

void test_encounter_null_sec_higher_bounty() {
    PCGManager mgr1, mgr2;
    mgr1.initialize(9);
    mgr2.initialize(9);

    auto ctx1 = mgr1.makeRootContext(PCGDomain::Encounter, 1, 1);
    auto ctx2 = mgr2.makeRootContext(PCGDomain::Encounter, 1, 1);

    NPCEncounter hs = NPCEncounterGenerator::generate(ctx1, 2, 1.0f);
    NPCEncounter ns = NPCEncounterGenerator::generate(ctx2, 2, 0.0f);

    assert(hs.valid && ns.valid);
    // Null-sec encounters carry more value per ship.
    assert(ns.estimatedBounty >= hs.estimatedBounty);
}

void test_encounter_determinism() {
    PCGManager mgr1, mgr2;
    mgr1.initialize(66);
    mgr2.initialize(66);

    auto ctx1 = mgr1.makeRootContext(PCGDomain::Encounter, 1, 1);
    auto ctx2 = mgr2.makeRootContext(PCGDomain::Encounter, 1, 1);

    auto e1 = NPCEncounterGenerator::generate(ctx1, 2, 0.5f);
    auto e2 = NPCEncounterGenerator::generate(ctx2, 2, 0.5f);

    assert(e1.valid && e2.valid);
    assert(e1.totalShips == e2.totalShips);
    assert(e1.faction    == e2.faction);
    assert(e1.waves.size() == e2.waves.size());
}

void test_encounter_calculate_bounty() {
    PCGManager mgr;
    mgr.initialize(100);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Encounter, 1, 1);

    NPCEncounter enc = NPCEncounterGenerator::generate(ctx, 2, 0.4f);
    assert(enc.valid);

    float bounty = NPCEncounterGenerator::calculateBounty(enc);
    assert(bounty > 0.0f);
    // Calculated bounty should match the stored estimate.
    assert(std::fabs(bounty - enc.estimatedBounty) < 0.01f);
}

void test_encounter_wave_trigger_delays() {
    PCGManager mgr;
    mgr.initialize(44);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Encounter, 1, 1);

    NPCEncounter enc = NPCEncounterGenerator::generate(ctx, 3, 0.5f);
    assert(enc.valid);
    assert(static_cast<int>(enc.waves.size()) == 3);

    // First wave starts immediately.
    assert(enc.waves[0].triggerDelay == 0.0f);
    // Subsequent waves have positive delays.
    for (size_t i = 1; i < enc.waves.size(); ++i) {
        assert(enc.waves[i].triggerDelay > 0.0f);
    }
}

// ══════════════════════════════════════════════════════════════════════
// DamageStateGenerator tests
// ══════════════════════════════════════════════════════════════════════

void test_damage_generate_pristine() {
    PCGManager mgr;
    mgr.initialize(42);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::DamageState, 1, 1);

    GeneratedDamageState ds = DamageStateGenerator::generate(ctx, 0.0f, HullClass::Frigate);

    assert(ds.valid);
    assert(ds.level == DamageLevel::Pristine);
    assert(ds.hull_breach_count  == 0);
    assert(ds.missing_module_count == 0);
    assert(std::fabs(ds.structural_integrity - 1.0f) < 0.01f);
}

void test_damage_generate_critical() {
    PCGManager mgr;
    mgr.initialize(5);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::DamageState, 1, 1);

    GeneratedDamageState ds = DamageStateGenerator::generate(ctx, 1.0f, HullClass::Battleship);

    assert(ds.valid);
    assert(ds.level == DamageLevel::Critical);
    assert(ds.hull_breach_count > 0);
    assert(ds.structural_integrity < 0.5f);
}

void test_damage_level_progression() {
    PCGManager mgr;
    mgr.initialize(8);

    struct TestCase { float norm; DamageLevel expected; };
    TestCase cases[] = {
        { 0.00f, DamageLevel::Pristine  },
        { 0.10f, DamageLevel::Light     },
        { 0.30f, DamageLevel::Moderate  },
        { 0.65f, DamageLevel::Heavy     },
        { 0.90f, DamageLevel::Critical  },
    };

    for (const auto& tc : cases) {
        PCGContext ctx = mgr.makeRootContext(PCGDomain::DamageState, 1, 1);
        GeneratedDamageState ds = DamageStateGenerator::generate(ctx, tc.norm, HullClass::Cruiser);
        assert(ds.valid);
        assert(ds.level == tc.expected);
    }
}

void test_damage_decals_increase_with_severity() {
    PCGManager mgr;
    mgr.initialize(3);

    auto ctxLight    = mgr.makeRootContext(PCGDomain::DamageState, 1, 1);
    auto ctxCritical = mgr.makeRootContext(PCGDomain::DamageState, 1, 2);

    GeneratedDamageState light    = DamageStateGenerator::generate(ctxLight,    0.1f, HullClass::Cruiser);
    GeneratedDamageState critical = DamageStateGenerator::generate(ctxCritical, 1.0f, HullClass::Cruiser);

    assert(light.valid && critical.valid);
    // Critical damage should produce more decals than light damage.
    assert(critical.decals.size() > light.decals.size());
}

void test_damage_determinism() {
    PCGManager mgr1, mgr2;
    mgr1.initialize(77);
    mgr2.initialize(77);

    auto ctx1 = mgr1.makeRootContext(PCGDomain::DamageState, 1, 1);
    auto ctx2 = mgr2.makeRootContext(PCGDomain::DamageState, 1, 1);

    auto d1 = DamageStateGenerator::generate(ctx1, 0.6f, HullClass::Destroyer);
    auto d2 = DamageStateGenerator::generate(ctx2, 0.6f, HullClass::Destroyer);

    assert(d1.valid && d2.valid);
    assert(d1.level  == d2.level);
    assert(d1.decals.size() == d2.decals.size());
    assert(d1.hull_breach_count == d2.hull_breach_count);
}

void test_damage_structural_integrity_range() {
    PCGManager mgr;
    mgr.initialize(200);

    float levels[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
    for (float lvl : levels) {
        PCGContext ctx = mgr.makeRootContext(PCGDomain::DamageState, 1, 1);
        GeneratedDamageState ds = DamageStateGenerator::generate(ctx, lvl, HullClass::Cruiser);
        assert(ds.valid);
        assert(ds.structural_integrity >= 0.0f);
        assert(ds.structural_integrity <= 1.0f);
    }
}

void test_damage_level_names() {
    assert(!DamageStateGenerator::damageLevelName(DamageLevel::Pristine).empty());
    assert(!DamageStateGenerator::damageLevelName(DamageLevel::Light).empty());
    assert(!DamageStateGenerator::damageLevelName(DamageLevel::Moderate).empty());
    assert(!DamageStateGenerator::damageLevelName(DamageLevel::Heavy).empty());
    assert(!DamageStateGenerator::damageLevelName(DamageLevel::Critical).empty());
}

void test_damage_decal_type_names() {
    assert(!DamageStateGenerator::decalTypeName(DecalType::ScorchMark).empty());
    assert(!DamageStateGenerator::decalTypeName(DecalType::ArmorCrack).empty());
    assert(!DamageStateGenerator::decalTypeName(DecalType::HullBreach).empty());
    assert(!DamageStateGenerator::decalTypeName(DecalType::MissingPlate).empty());
    assert(!DamageStateGenerator::decalTypeName(DecalType::StructuralBend).empty());
    assert(!DamageStateGenerator::decalTypeName(DecalType::ElectricalScar).empty());
}
