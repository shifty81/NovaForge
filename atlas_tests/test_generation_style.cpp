#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include "../editor/tools/GenerationStylePanel.h"
#include "../editor/tools/AssetStylePanel.h"
#include "../editor/tools/ShipArchetypePanel.h"

// ── Helpers ─────────────────────────────────────────────────────────

static int gs_passed = 0;

static void ok(const char* name) {
    // Output handled by RUN_TEST in main.cpp
    ++gs_passed;
}

// ═══════════════════════════════════════════════════════════════════
//  Generation Style Engine tests
// ═══════════════════════════════════════════════════════════════════

void test_gs_create_default_ship_style() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::ShipLayout);

    assert(style.type == atlas::pcg::GenerationStyleType::ShipLayout);
    assert(!style.name.empty());
    assert(style.baseSeed == 42);
    assert(style.version == 1);
    assert(!style.parameters.empty());
    assert(style.placements.empty());
    ok("test_gs_create_default_ship_style");
}

void test_gs_create_default_station_style() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StationLayout, "My Station");

    assert(style.type == atlas::pcg::GenerationStyleType::StationLayout);
    assert(style.name == "My Station");
    assert(!style.parameters.empty());
    ok("test_gs_create_default_station_style");
}

void test_gs_create_all_default_styles() {
    for (int i = 0; i < atlas::pcg::GENERATION_STYLE_TYPE_COUNT; ++i) {
        auto type = static_cast<atlas::pcg::GenerationStyleType>(i);
        auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(type);
        assert(!style.name.empty());
        assert(!style.parameters.empty());
    }
    ok("test_gs_create_all_default_styles");
}

void test_gs_validate_valid_style() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::ShipLayout);

    bool valid = atlas::pcg::GenerationStyleEngine::validate(style);
    assert(valid);
    assert(style.valid);
    ok("test_gs_validate_valid_style");
}

void test_gs_validate_empty_name_fails() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::ShipLayout);
    style.name = "";

    bool valid = atlas::pcg::GenerationStyleEngine::validate(style);
    assert(!valid);
    assert(!style.valid);
    ok("test_gs_validate_empty_name_fails");
}

void test_gs_validate_out_of_range_param_fails() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::ShipLayout);

    // Force a parameter out of range.
    for (auto& p : style.parameters) {
        if (p.name == "massScale") {
            p.value = 999.0f;  // Way above maxValue
            break;
        }
    }

    bool valid = atlas::pcg::GenerationStyleEngine::validate(style);
    assert(!valid);
    ok("test_gs_validate_out_of_range_param_fails");
}

void test_gs_validate_duplicate_slots_fails() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StationLayout);

    atlas::pcg::PlacementEntry e1{};
    e1.slotIndex = 0;
    e1.label = "Module A";
    atlas::pcg::PlacementEntry e2{};
    e2.slotIndex = 0;  // Duplicate!
    e2.label = "Module B";

    style.placements.push_back(e1);
    style.placements.push_back(e2);

    bool valid = atlas::pcg::GenerationStyleEngine::validate(style);
    assert(!valid);
    ok("test_gs_validate_duplicate_slots_fails");
}

void test_gs_available_parameters() {
    auto params = atlas::pcg::GenerationStyleEngine::availableParameters(
        atlas::pcg::GenerationStyleType::ShipLayout);
    assert(!params.empty());
    // Should contain "hullClass".
    bool found = false;
    for (const auto& p : params) {
        if (p == "hullClass") found = true;
    }
    assert(found);
    ok("test_gs_available_parameters");
}

void test_gs_find_parameter() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::ShipLayout);

    auto* p = atlas::pcg::GenerationStyleEngine::findParameter(
        style, "massScale");
    assert(p != nullptr);
    assert(p->name == "massScale");

    auto* missing = atlas::pcg::GenerationStyleEngine::findParameter(
        style, "nonexistent");
    assert(missing == nullptr);
    ok("test_gs_find_parameter");
}

void test_gs_style_type_names() {
    assert(std::string(atlas::pcg::GenerationStyleEngine::styleTypeName(
        atlas::pcg::GenerationStyleType::ShipLayout)) == "ShipLayout");
    assert(std::string(atlas::pcg::GenerationStyleEngine::styleTypeName(
        atlas::pcg::GenerationStyleType::StationLayout)) == "StationLayout");
    assert(std::string(atlas::pcg::GenerationStyleEngine::styleTypeName(
        atlas::pcg::GenerationStyleType::AsteroidField)) == "AsteroidField");
    ok("test_gs_style_type_names");
}

void test_gs_generate_ship_style() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::ShipLayout);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.sourceType == atlas::pcg::GenerationStyleType::ShipLayout);
    assert(!result.shipResult.shipName.empty());
    assert(result.shipResult.mass > 0.0f);
    ok("test_gs_generate_ship_style");
}

void test_gs_generate_station_style() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StationLayout);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Station, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(!result.stationResult.modules.empty());
    ok("test_gs_generate_station_style");
}

void test_gs_generate_station_with_placements() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StationLayout, "Placed Station");

    // Override module count to 7 and place a Power module at slot 0.
    for (auto& p : style.parameters) {
        if (p.name == "moduleCount") {
            p.value = 7.0f;
            break;
        }
    }
    atlas::pcg::PlacementEntry pe{};
    pe.slotIndex   = 0;
    pe.contentType = static_cast<uint32_t>(
        atlas::pcg::StationModuleType::Power);
    pe.posX = 10.0f;
    pe.posY = 0.0f;
    pe.posZ = 0.0f;
    pe.locked = true;
    pe.label  = "Main Power";
    style.placements.push_back(pe);

    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Station, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.stationResult.modules.size() == 7);
    assert(result.placementsApplied >= 1);
    // The first module should be pinned to our position.
    assert(result.stationResult.modules[0].type ==
           atlas::pcg::StationModuleType::Power);
    assert(std::fabs(result.stationResult.modules[0].posX - 10.0f) < 0.01f);
    ok("test_gs_generate_station_with_placements");
}

void test_gs_generate_interior_style() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::InteriorLayout);

    // Set ship class to Cruiser (2).
    for (auto& p : style.parameters) {
        if (p.name == "shipClass") {
            p.value = 2.0f;
            break;
        }
    }
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(!result.interiorResult.rooms.empty());
    assert(result.parametersApplied >= 1);
    ok("test_gs_generate_interior_style");
}

void test_gs_generate_determinism() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::ShipLayout);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr1, mgr2;
    mgr1.initialize(style.baseSeed);
    mgr2.initialize(style.baseSeed);
    auto ctx1 = mgr1.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);
    auto ctx2 = mgr2.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);

    auto r1 = atlas::pcg::GenerationStyleEngine::generate(ctx1, style);
    auto r2 = atlas::pcg::GenerationStyleEngine::generate(ctx2, style);

    assert(r1.shipResult.shipName == r2.shipResult.shipName);
    assert(r1.shipResult.mass == r2.shipResult.mass);
    assert(r1.shipResult.turretSlots == r2.shipResult.turretSlots);
    ok("test_gs_generate_determinism");
}

void test_gs_serialize_roundtrip() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StationLayout, "TestRoundtrip");

    atlas::pcg::PlacementEntry pe{};
    pe.slotIndex   = 2;
    pe.posX = 5.0f; pe.posY = 3.0f; pe.posZ = 1.0f;
    pe.contentType = 3;
    pe.locked      = true;
    pe.label       = "Hangar";
    style.placements.push_back(pe);

    std::string data = atlas::pcg::GenerationStyleEngine::serialize(style);
    assert(!data.empty());

    auto loaded = atlas::pcg::GenerationStyleEngine::deserialize(data);
    assert(loaded.name == "TestRoundtrip");
    assert(loaded.type == atlas::pcg::GenerationStyleType::StationLayout);
    assert(loaded.baseSeed == style.baseSeed);
    assert(loaded.placements.size() == 1);
    assert(loaded.placements[0].slotIndex == 2);
    assert(loaded.placements[0].locked == true);
    assert(loaded.parameters.size() == style.parameters.size());
    ok("test_gs_serialize_roundtrip");
}

void test_gs_ship_parameter_overrides() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::ShipLayout);

    // Override turret slots to 6.
    for (auto& p : style.parameters) {
        if (p.name == "turretSlots") {
            p.value = 6.0f;
            break;
        }
    }
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.shipResult.turretSlots == 6);
    assert(result.parametersApplied >= 1);
    ok("test_gs_ship_parameter_overrides");
}

// ── Star System generation tests ────────────────────────────────────

void test_gs_generate_star_system() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StarSystem);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::StarSystem, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.sourceType == atlas::pcg::GenerationStyleType::StarSystem);
    assert(!result.starSystemResult.systemName.empty());
    assert(!result.starSystemResult.planets.empty());
    assert(!result.starSystemResult.gates.empty());
    assert(result.starSystemResult.valid);
    assert(result.parametersApplied >= 1);
    ok("test_gs_generate_star_system");
}

void test_gs_star_system_planet_count_override() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StarSystem);
    for (auto& p : style.parameters) {
        if (p.name == "planetCount") {
            p.value = 7.0f;
            break;
        }
    }
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::StarSystem, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.starSystemResult.planets.size() == 7);
    ok("test_gs_star_system_planet_count_override");
}

void test_gs_star_system_gate_count_override() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StarSystem);
    for (auto& p : style.parameters) {
        if (p.name == "gateCount") {
            p.value = 3.0f;
            break;
        }
    }
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::StarSystem, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.starSystemResult.gates.size() == 3);
    ok("test_gs_star_system_gate_count_override");
}

void test_gs_star_system_security_level() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StarSystem);
    for (auto& p : style.parameters) {
        if (p.name == "securityLevel") {
            p.value = 0.8f;
            break;
        }
    }
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::StarSystem, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(std::fabs(result.starSystemResult.securityLevel - 0.8f) < 0.01f);
    ok("test_gs_star_system_security_level");
}

void test_gs_star_system_with_placements() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StarSystem);
    for (auto& p : style.parameters) {
        if (p.name == "planetCount") { p.value = 5.0f; break; }
    }

    atlas::pcg::PlacementEntry pe{};
    pe.slotIndex   = 0;
    pe.contentType = 1;  // gas giant
    pe.posX = 2.0f;      // orbitRadius
    pe.posY = 1.5f;      // orbitAngle
    pe.posZ = 50000.0f;  // radius
    pe.locked = true;
    pe.label  = "Gas Giant";
    style.placements.push_back(pe);

    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::StarSystem, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.placementsApplied >= 1);
    assert(result.starSystemResult.planets[0].bodyType == 1);
    assert(std::fabs(result.starSystemResult.planets[0].orbitRadius - 2.0f) < 0.01f);
    assert(std::fabs(result.starSystemResult.planets[0].radius - 50000.0f) < 0.01f);
    ok("test_gs_star_system_with_placements");
}

void test_gs_star_system_determinism() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::StarSystem);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr1, mgr2;
    mgr1.initialize(style.baseSeed);
    mgr2.initialize(style.baseSeed);
    auto ctx1 = mgr1.makeRootContext(atlas::pcg::PCGDomain::StarSystem, 1, 1);
    auto ctx2 = mgr2.makeRootContext(atlas::pcg::PCGDomain::StarSystem, 1, 1);

    auto r1 = atlas::pcg::GenerationStyleEngine::generate(ctx1, style);
    auto r2 = atlas::pcg::GenerationStyleEngine::generate(ctx2, style);

    assert(r1.starSystemResult.systemName == r2.starSystemResult.systemName);
    assert(r1.starSystemResult.planets.size() == r2.starSystemResult.planets.size());
    assert(r1.starSystemResult.gates.size() == r2.starSystemResult.gates.size());
    ok("test_gs_star_system_determinism");
}

// ── Asteroid Field generation tests ─────────────────────────────────

void test_gs_generate_asteroid_field() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::AsteroidField);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Asteroid, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.sourceType == atlas::pcg::GenerationStyleType::AsteroidField);
    assert(!result.asteroidFieldResult.asteroids.empty());
    assert(!result.asteroidFieldResult.fieldName.empty());
    assert(result.asteroidFieldResult.valid);
    assert(result.parametersApplied >= 1);
    ok("test_gs_generate_asteroid_field");
}

void test_gs_asteroid_field_density_affects_count() {
    auto styleLow = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::AsteroidField);
    auto styleHigh = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::AsteroidField);

    for (auto& p : styleLow.parameters) {
        if (p.name == "density") { p.value = 0.05f; break; }
    }
    for (auto& p : styleHigh.parameters) {
        if (p.name == "density") { p.value = 0.4f; break; }
    }

    atlas::pcg::GenerationStyleEngine::validate(styleLow);
    atlas::pcg::GenerationStyleEngine::validate(styleHigh);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctxLow  = mgr.makeRootContext(atlas::pcg::PCGDomain::Asteroid, 1, 1);
    auto ctxHigh = mgr.makeRootContext(atlas::pcg::PCGDomain::Asteroid, 2, 1);

    auto rLow  = atlas::pcg::GenerationStyleEngine::generate(ctxLow, styleLow);
    auto rHigh = atlas::pcg::GenerationStyleEngine::generate(ctxHigh, styleHigh);

    assert(rLow.success && rHigh.success);
    assert(rHigh.asteroidFieldResult.asteroids.size() >
           rLow.asteroidFieldResult.asteroids.size());
    ok("test_gs_asteroid_field_density_affects_count");
}

void test_gs_asteroid_field_with_placements() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::AsteroidField);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PlacementEntry pe{};
    pe.slotIndex   = 0;
    pe.contentType = 7;  // arkonor
    pe.posX = 100.0f;
    pe.posY = 0.0f;
    pe.posZ = 50.0f;
    pe.locked = true;
    pe.label  = "Rare Rock";
    style.placements.push_back(pe);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Asteroid, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.placementsApplied >= 1);
    assert(result.asteroidFieldResult.asteroids[0].oreType == 7);
    assert(std::fabs(result.asteroidFieldResult.asteroids[0].posX - 100.0f) < 0.01f);
    ok("test_gs_asteroid_field_with_placements");
}

void test_gs_asteroid_field_determinism() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::AsteroidField);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr1, mgr2;
    mgr1.initialize(style.baseSeed);
    mgr2.initialize(style.baseSeed);
    auto ctx1 = mgr1.makeRootContext(atlas::pcg::PCGDomain::Asteroid, 1, 1);
    auto ctx2 = mgr2.makeRootContext(atlas::pcg::PCGDomain::Asteroid, 1, 1);

    auto r1 = atlas::pcg::GenerationStyleEngine::generate(ctx1, style);
    auto r2 = atlas::pcg::GenerationStyleEngine::generate(ctx2, style);

    assert(r1.asteroidFieldResult.asteroids.size() ==
           r2.asteroidFieldResult.asteroids.size());
    assert(r1.asteroidFieldResult.fieldName == r2.asteroidFieldResult.fieldName);
    ok("test_gs_asteroid_field_determinism");
}

// ── Fleet Composition generation tests ──────────────────────────────

void test_gs_generate_fleet_composition() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::FleetComposition);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Fleet, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.sourceType == atlas::pcg::GenerationStyleType::FleetComposition);
    assert(!result.fleetResult.ships.empty());
    assert(!result.fleetResult.doctrineName.empty());
    assert(result.fleetResult.valid);
    assert(result.parametersApplied >= 1);
    ok("test_gs_generate_fleet_composition");
}

void test_gs_fleet_size_override() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::FleetComposition);
    for (auto& p : style.parameters) {
        if (p.name == "fleetSize") { p.value = 10.0f; break; }
    }
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Fleet, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(static_cast<int>(result.fleetResult.ships.size()) == 10);
    ok("test_gs_fleet_size_override");
}

void test_gs_fleet_capital_ratio() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::FleetComposition);
    for (auto& p : style.parameters) {
        if (p.name == "fleetSize") p.value = 20.0f;
        if (p.name == "capitalRatio") p.value = 0.25f;
    }
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Fleet, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.fleetResult.capitalCount == 5);
    assert(result.fleetResult.subcapCount == 15);
    assert(static_cast<int>(result.fleetResult.ships.size()) == 20);
    ok("test_gs_fleet_capital_ratio");
}

void test_gs_fleet_support_ratio() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::FleetComposition);
    for (auto& p : style.parameters) {
        if (p.name == "fleetSize") p.value = 10.0f;
        if (p.name == "capitalRatio") p.value = 0.0f;
        if (p.name == "supportRatio") p.value = 0.3f;
    }
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Fleet, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    // With 10 ships, 0 capital, 30% support = 3 support + 7 dps.
    int supportCount = 0;
    int dpsCount = 0;
    for (const auto& ship : result.fleetResult.ships) {
        if (ship.role == "dps") ++dpsCount;
        else ++supportCount;
    }
    assert(supportCount == 3);
    assert(dpsCount == 7);
    ok("test_gs_fleet_support_ratio");
}

void test_gs_fleet_with_placements() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::FleetComposition);
    for (auto& p : style.parameters) {
        if (p.name == "fleetSize") { p.value = 5.0f; break; }
    }

    atlas::pcg::PlacementEntry pe{};
    pe.slotIndex   = 0;
    pe.contentType = static_cast<uint32_t>(atlas::pcg::HullClass::Titan);
    pe.locked      = false;
    pe.label       = "Flagship";
    style.placements.push_back(pe);

    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(style.baseSeed);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Fleet, 1, 1);

    auto result = atlas::pcg::GenerationStyleEngine::generate(ctx, style);
    assert(result.success);
    assert(result.placementsApplied >= 1);
    assert(result.fleetResult.ships[0].hullClass == atlas::pcg::HullClass::Titan);
    assert(result.fleetResult.ships[0].shipName == "Flagship");
    ok("test_gs_fleet_with_placements");
}

void test_gs_fleet_determinism() {
    auto style = atlas::pcg::GenerationStyleEngine::createDefaultStyle(
        atlas::pcg::GenerationStyleType::FleetComposition);
    atlas::pcg::GenerationStyleEngine::validate(style);

    atlas::pcg::PCGManager mgr1, mgr2;
    mgr1.initialize(style.baseSeed);
    mgr2.initialize(style.baseSeed);
    auto ctx1 = mgr1.makeRootContext(atlas::pcg::PCGDomain::Fleet, 1, 1);
    auto ctx2 = mgr2.makeRootContext(atlas::pcg::PCGDomain::Fleet, 1, 1);

    auto r1 = atlas::pcg::GenerationStyleEngine::generate(ctx1, style);
    auto r2 = atlas::pcg::GenerationStyleEngine::generate(ctx2, style);

    assert(r1.fleetResult.doctrineName == r2.fleetResult.doctrineName);
    assert(r1.fleetResult.ships.size() == r2.fleetResult.ships.size());
    for (size_t i = 0; i < r1.fleetResult.ships.size(); ++i) {
        assert(r1.fleetResult.ships[i].shipName == r2.fleetResult.ships[i].shipName);
        assert(r1.fleetResult.ships[i].hullClass == r2.fleetResult.ships[i].hullClass);
    }
    ok("test_gs_fleet_determinism");
}

// ═══════════════════════════════════════════════════════════════════
//  Generation Style Panel tests
// ═══════════════════════════════════════════════════════════════════

void test_gsp_defaults() {
    atlas::editor::GenerationStylePanel panel;
    assert(std::string(panel.Name()) == "Generation Style");
    assert(panel.GetStyle().type ==
           atlas::pcg::GenerationStyleType::ShipLayout);
    assert(panel.PlacementCount() == 0);
    assert(!panel.HasAssetStyle());
    ok("test_gsp_defaults");
}

void test_gsp_new_style() {
    atlas::editor::GenerationStylePanel panel;
    panel.NewStyle(atlas::pcg::GenerationStyleType::StationLayout,
                   "Custom Station");
    assert(panel.GetStyle().type ==
           atlas::pcg::GenerationStyleType::StationLayout);
    assert(panel.GetStyle().name == "Custom Station");
    assert(!panel.Log().empty());
    ok("test_gsp_new_style");
}

void test_gsp_add_remove_placement() {
    atlas::editor::GenerationStylePanel panel;
    panel.NewStyle(atlas::pcg::GenerationStyleType::StationLayout);

    atlas::pcg::PlacementEntry pe{};
    pe.slotIndex   = 0;
    pe.label       = "Power Core";
    pe.contentType = 5;  // Power
    pe.locked      = true;
    panel.AddPlacement(pe);
    assert(panel.PlacementCount() == 1);

    atlas::pcg::PlacementEntry pe2{};
    pe2.slotIndex   = 1;
    pe2.label       = "Hangar Bay";
    pe2.contentType = 3;
    pe2.locked      = false;
    panel.AddPlacement(pe2);
    assert(panel.PlacementCount() == 2);

    bool removed = panel.RemovePlacement(0);
    assert(removed);
    assert(panel.PlacementCount() == 1);

    bool removeFail = panel.RemovePlacement(99);
    assert(!removeFail);
    ok("test_gsp_add_remove_placement");
}

void test_gsp_set_parameter() {
    atlas::editor::GenerationStylePanel panel;
    bool set = panel.SetParameter("massScale", 2.0f);
    assert(set);

    auto* p = atlas::pcg::GenerationStyleEngine::findParameter(
        panel.GetStyle(), "massScale");
    assert(p && std::fabs(p->value - 2.0f) < 0.01f);

    bool setFail = panel.SetParameter("nonexistent", 1.0f);
    assert(!setFail);
    ok("test_gsp_set_parameter");
}

void test_gsp_enable_disable_parameter() {
    atlas::editor::GenerationStylePanel panel;
    bool ok1 = panel.EnableParameter("massScale", false);
    assert(ok1);

    auto* p = atlas::pcg::GenerationStyleEngine::findParameter(
        panel.GetStyle(), "massScale");
    assert(p && !p->enabled);

    bool ok2 = panel.EnableParameter("massScale", true);
    assert(ok2);
    ok("test_gsp_enable_disable_parameter");
}

void test_gsp_generate() {
    atlas::editor::GenerationStylePanel panel;
    panel.Generate();
    assert(panel.GetResult().success);
    assert(!panel.GetResult().shipResult.shipName.empty());
    assert(!panel.Log().empty());
    ok("test_gsp_generate");
}

void test_gsp_generate_with_asset_style() {
    atlas::editor::GenerationStylePanel panel;

    // Create an asset style with a shape that scales the ship.
    atlas::pcg::AssetStyle as{};
    as.name       = "Bulky";
    as.targetType = atlas::pcg::GenerationStyleType::ShipLayout;
    as.version    = 1;
    as.valid      = true;
    as.shape.name = "BulkyShape";
    as.shape.smoothing = 0.5f;
    atlas::pcg::ShapeControlPoint cp{};
    cp.posX = 0.0f; cp.posY = 0.0f; cp.posZ = 0.0f;
    cp.scaleX = 2.0f; cp.scaleY = 2.0f; cp.scaleZ = 2.0f;
    cp.weight = 1.0f;
    as.shape.controlPoints.push_back(cp);

    panel.AttachAssetStyle(as);
    assert(panel.HasAssetStyle());

    // Generate without asset style first for comparison.
    atlas::editor::GenerationStylePanel panel2;
    panel2.Generate();
    float baseMass = panel2.GetResult().shipResult.mass;

    panel.Generate();
    // The asset style should have scaled the mass.
    assert(panel.GetResult().success);
    assert(panel.GetResult().shipResult.mass > baseMass);
    ok("test_gsp_generate_with_asset_style");
}

void test_gsp_save_load_string() {
    atlas::editor::GenerationStylePanel panel;
    panel.NewStyle(atlas::pcg::GenerationStyleType::InteriorLayout,
                   "SavedInterior");
    panel.SetParameter("shipClass", 3.0f);

    std::string data = panel.SaveStyleToString();
    assert(!data.empty());

    atlas::editor::GenerationStylePanel panel2;
    panel2.LoadStyleFromString(data);
    assert(panel2.GetStyle().name == "SavedInterior");
    assert(panel2.GetStyle().type ==
           atlas::pcg::GenerationStyleType::InteriorLayout);
    ok("test_gsp_save_load_string");
}

void test_gsp_draw_does_not_crash() {
    atlas::editor::GenerationStylePanel panel;
    panel.Generate();
    panel.Draw();
    ok("test_gsp_draw_does_not_crash");
}

// ═══════════════════════════════════════════════════════════════════
//  Asset Style Library tests
// ═══════════════════════════════════════════════════════════════════

void test_as_library_add_find() {
    atlas::pcg::AssetStyleLibrary lib;
    assert(lib.size() == 0);

    atlas::pcg::AssetStyle style{};
    style.name       = "TestStyle";
    style.targetType = atlas::pcg::GenerationStyleType::ShipLayout;
    style.valid      = true;
    lib.addStyle(style);
    assert(lib.size() == 1);

    auto* found = lib.findStyle("TestStyle");
    assert(found != nullptr);
    assert(found->name == "TestStyle");

    assert(lib.findStyle("Missing") == nullptr);
    ok("test_as_library_add_find");
}

void test_as_library_replace() {
    atlas::pcg::AssetStyleLibrary lib;

    atlas::pcg::AssetStyle s1{};
    s1.name       = "Dup";
    s1.targetType = atlas::pcg::GenerationStyleType::ShipLayout;
    s1.version    = 1;
    lib.addStyle(s1);

    atlas::pcg::AssetStyle s2{};
    s2.name       = "Dup";
    s2.targetType = atlas::pcg::GenerationStyleType::StationLayout;
    s2.version    = 2;
    lib.addStyle(s2);

    assert(lib.size() == 1);
    auto* found = lib.findStyle("Dup");
    assert(found && found->version == 2);
    ok("test_as_library_replace");
}

void test_as_library_remove() {
    atlas::pcg::AssetStyleLibrary lib;

    atlas::pcg::AssetStyle s{};
    s.name = "Remove Me";
    lib.addStyle(s);
    assert(lib.size() == 1);

    bool removed = lib.removeStyle("Remove Me");
    assert(removed);
    assert(lib.size() == 0);

    bool removeFail = lib.removeStyle("Missing");
    assert(!removeFail);
    ok("test_as_library_remove");
}

void test_as_library_list_by_type() {
    atlas::pcg::AssetStyleLibrary lib;

    atlas::pcg::AssetStyle s1{};
    s1.name = "Ship1";
    s1.targetType = atlas::pcg::GenerationStyleType::ShipLayout;
    lib.addStyle(s1);

    atlas::pcg::AssetStyle s2{};
    s2.name = "Station1";
    s2.targetType = atlas::pcg::GenerationStyleType::StationLayout;
    lib.addStyle(s2);

    atlas::pcg::AssetStyle s3{};
    s3.name = "Ship2";
    s3.targetType = atlas::pcg::GenerationStyleType::ShipLayout;
    lib.addStyle(s3);

    auto ships = lib.listStyles(atlas::pcg::GenerationStyleType::ShipLayout);
    assert(ships.size() == 2);

    auto stations = lib.listStyles(
        atlas::pcg::GenerationStyleType::StationLayout);
    assert(stations.size() == 1);

    auto all = lib.listAll();
    assert(all.size() == 3);
    ok("test_as_library_list_by_type");
}

void test_as_library_clear() {
    atlas::pcg::AssetStyleLibrary lib;
    atlas::pcg::AssetStyle s{};
    s.name = "A";
    lib.addStyle(s);
    s.name = "B";
    lib.addStyle(s);
    assert(lib.size() == 2);
    lib.clear();
    assert(lib.size() == 0);
    ok("test_as_library_clear");
}

void test_as_shape_apply_to_ship() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);
    auto ship = atlas::pcg::ShipGenerator::generate(ctx);
    float origMass = ship.mass;
    float origSig  = ship.signatureRadius;

    atlas::pcg::ShapeProfile shape{};
    shape.name = "Scale2x";
    shape.smoothing = 0.5f;
    atlas::pcg::ShapeControlPoint cp{};
    cp.posX = 0; cp.posY = 0; cp.posZ = 0;
    cp.scaleX = 2.0f; cp.scaleY = 2.0f; cp.scaleZ = 2.0f;
    cp.weight = 1.0f;
    shape.controlPoints.push_back(cp);

    atlas::pcg::AssetStyleLibrary::applyShapeToShip(ship, shape);

    // Mass and signature should have changed.
    assert(ship.mass > origMass);
    assert(ship.signatureRadius > origSig);
    ok("test_as_shape_apply_to_ship");
}

void test_as_shape_apply_to_station() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Station, 1, 1);
    auto station = atlas::pcg::StationGenerator::generate(ctx);

    float origDimX = station.modules[0].dimX;

    atlas::pcg::ShapeProfile shape{};
    shape.name = "Stretch";
    shape.smoothing = 0.5f;
    atlas::pcg::ShapeControlPoint cp{};
    cp.posX = station.modules[0].posX;
    cp.posY = station.modules[0].posY;
    cp.posZ = station.modules[0].posZ;
    cp.scaleX = 3.0f; cp.scaleY = 1.0f; cp.scaleZ = 1.0f;
    cp.weight = 1.0f;
    shape.controlPoints.push_back(cp);

    atlas::pcg::AssetStyleLibrary::applyShapeToStation(station, shape);

    // First module's X dimension should have increased.
    assert(station.modules[0].dimX > origDimX);
    ok("test_as_shape_apply_to_station");
}

void test_as_surface_treatment_names() {
    assert(std::string(atlas::pcg::AssetStyleLibrary::surfaceTreatmentName(
        atlas::pcg::SurfaceTreatment::None)) == "None");
    assert(std::string(atlas::pcg::AssetStyleLibrary::surfaceTreatmentName(
        atlas::pcg::SurfaceTreatment::Greeble)) == "Greeble");
    assert(std::string(atlas::pcg::AssetStyleLibrary::surfaceTreatmentName(
        atlas::pcg::SurfaceTreatment::BattleScarred)) == "BattleScarred");
    ok("test_as_surface_treatment_names");
}

void test_as_serialize_roundtrip() {
    atlas::pcg::AssetStyle style{};
    style.name       = "RoundtripStyle";
    style.targetType = atlas::pcg::GenerationStyleType::ShipLayout;
    style.version    = 2;
    style.valid      = true;

    style.shape.name      = "TestShape";
    style.shape.mirrorX   = true;
    style.shape.mirrorY   = false;
    style.shape.smoothing = 0.7f;
    atlas::pcg::ShapeControlPoint cp{};
    cp.posX = 1; cp.posY = 2; cp.posZ = 3;
    cp.scaleX = 1.5f; cp.scaleY = 1.0f; cp.scaleZ = 0.8f;
    cp.weight = 0.9f;
    style.shape.controlPoints.push_back(cp);

    style.palette.name             = "TestPalette";
    style.palette.surfaceTreatment = atlas::pcg::SurfaceTreatment::Greeble;
    style.palette.detailLevel      = 0.6f;
    atlas::pcg::StyleColor col{};
    col.r = 0.5f; col.g = 0.3f; col.b = 0.8f; col.a = 1.0f;
    col.regionName = "Hull";
    style.palette.colors.push_back(col);
    atlas::pcg::StyleMaterial mat{};
    mat.metallic  = 0.8f;
    mat.roughness = 0.2f;
    mat.emissive  = 0.1f;
    mat.name      = "Armor";
    style.palette.materials.push_back(mat);

    std::string data = atlas::pcg::AssetStyleLibrary::serialize(style);
    assert(!data.empty());

    auto loaded = atlas::pcg::AssetStyleLibrary::deserialize(data);
    assert(loaded.name == "RoundtripStyle");
    assert(loaded.version == 2);
    assert(loaded.shape.name == "TestShape");
    assert(loaded.shape.mirrorX == true);
    assert(std::fabs(loaded.shape.smoothing - 0.7f) < 0.01f);
    assert(loaded.shape.controlPoints.size() == 1);
    assert(std::fabs(loaded.shape.controlPoints[0].scaleX - 1.5f) < 0.01f);
    assert(loaded.palette.name == "TestPalette");
    assert(loaded.palette.surfaceTreatment ==
           atlas::pcg::SurfaceTreatment::Greeble);
    assert(loaded.palette.colors.size() == 1);
    assert(loaded.palette.materials.size() == 1);
    assert(loaded.valid);
    ok("test_as_serialize_roundtrip");
}

// ═══════════════════════════════════════════════════════════════════
//  Asset Style Panel tests
// ═══════════════════════════════════════════════════════════════════

void test_asp_defaults() {
    atlas::editor::AssetStylePanel panel;
    assert(std::string(panel.Name()) == "Asset Style");
    assert(panel.ControlPointCount() == 0);
    assert(panel.ColorCount() == 0);
    assert(panel.MaterialCount() == 0);
    assert(!panel.HasPreview());
    ok("test_asp_defaults");
}

void test_asp_new_style() {
    atlas::editor::AssetStylePanel panel;
    panel.NewStyle("Sleek Fighter",
                   atlas::pcg::GenerationStyleType::ShipLayout);
    assert(panel.GetCurrentStyle().name == "Sleek Fighter");
    assert(panel.GetCurrentStyle().targetType ==
           atlas::pcg::GenerationStyleType::ShipLayout);
    ok("test_asp_new_style");
}

void test_asp_shape_control_points() {
    atlas::editor::AssetStylePanel panel;
    panel.NewShapeProfile("Hull Warp");

    atlas::pcg::ShapeControlPoint cp1{};
    cp1.posX = 0; cp1.posY = 0; cp1.posZ = 0;
    cp1.scaleX = 1.5f; cp1.scaleY = 1.0f; cp1.scaleZ = 1.0f;
    cp1.weight = 1.0f;
    panel.AddControlPoint(cp1);
    assert(panel.ControlPointCount() == 1);

    atlas::pcg::ShapeControlPoint cp2{};
    cp2.posX = 5; cp2.posY = 0; cp2.posZ = 0;
    cp2.scaleX = 0.8f; cp2.scaleY = 1.2f; cp2.scaleZ = 1.0f;
    cp2.weight = 0.7f;
    panel.AddControlPoint(cp2);
    assert(panel.ControlPointCount() == 2);

    // Update cp1.
    cp1.scaleX = 2.0f;
    bool updated = panel.UpdateControlPoint(0, cp1);
    assert(updated);
    assert(std::fabs(
        panel.GetCurrentStyle().shape.controlPoints[0].scaleX - 2.0f) < 0.01f);

    // Remove cp2.
    bool removed = panel.RemoveControlPoint(1);
    assert(removed);
    assert(panel.ControlPointCount() == 1);

    // Out of bounds.
    assert(!panel.RemoveControlPoint(99));
    assert(!panel.UpdateControlPoint(99, cp1));
    ok("test_asp_shape_control_points");
}

void test_asp_mirror_and_smoothing() {
    atlas::editor::AssetStylePanel panel;
    panel.NewShapeProfile("Symmetric");
    panel.SetMirror(true, false);
    panel.SetSmoothing(0.8f);

    assert(panel.GetCurrentStyle().shape.mirrorX == true);
    assert(panel.GetCurrentStyle().shape.mirrorY == false);
    assert(std::fabs(panel.GetCurrentStyle().shape.smoothing - 0.8f) < 0.01f);
    ok("test_asp_mirror_and_smoothing");
}

void test_asp_palette_colors() {
    atlas::editor::AssetStylePanel panel;
    panel.NewPalette("Dark Theme");

    atlas::pcg::StyleColor c{};
    c.r = 0.1f; c.g = 0.1f; c.b = 0.2f; c.a = 1.0f;
    c.regionName = "Hull";
    panel.AddColor(c);
    assert(panel.ColorCount() == 1);

    bool set = panel.SetColor(0, 0.5f, 0.5f, 0.5f, 1.0f);
    assert(set);
    assert(std::fabs(
        panel.GetCurrentStyle().palette.colors[0].r - 0.5f) < 0.01f);

    bool removed = panel.RemoveColor(0);
    assert(removed);
    assert(panel.ColorCount() == 0);

    assert(!panel.SetColor(99, 0, 0, 0, 0));
    assert(!panel.RemoveColor(99));
    ok("test_asp_palette_colors");
}

void test_asp_palette_materials() {
    atlas::editor::AssetStylePanel panel;
    panel.NewPalette("Industrial");

    atlas::pcg::StyleMaterial mat{};
    mat.name      = "Steel";
    mat.metallic  = 0.9f;
    mat.roughness = 0.3f;
    mat.emissive  = 0.0f;
    panel.AddMaterial(mat);
    assert(panel.MaterialCount() == 1);

    mat.name = "Updated Steel";
    bool set = panel.SetMaterial(0, mat);
    assert(set);
    assert(panel.GetCurrentStyle().palette.materials[0].name == "Updated Steel");

    bool removed = panel.RemoveMaterial(0);
    assert(removed);
    assert(panel.MaterialCount() == 0);

    assert(!panel.SetMaterial(99, mat));
    assert(!panel.RemoveMaterial(99));
    ok("test_asp_palette_materials");
}

void test_asp_surface_treatment() {
    atlas::editor::AssetStylePanel panel;
    panel.NewPalette("Worn");
    panel.SetSurfaceTreatment(atlas::pcg::SurfaceTreatment::Weathered);
    assert(panel.GetCurrentStyle().palette.surfaceTreatment ==
           atlas::pcg::SurfaceTreatment::Weathered);
    ok("test_asp_surface_treatment");
}

void test_asp_detail_level() {
    atlas::editor::AssetStylePanel panel;
    panel.NewPalette("Detailed");
    panel.SetDetailLevel(0.9f);
    assert(std::fabs(
        panel.GetCurrentStyle().palette.detailLevel - 0.9f) < 0.01f);
    ok("test_asp_detail_level");
}

void test_asp_apply_and_preview_ship() {
    atlas::editor::AssetStylePanel panel;
    panel.NewStyle("Preview Test",
                   atlas::pcg::GenerationStyleType::ShipLayout);
    panel.NewShapeProfile("Test Shape");

    atlas::pcg::ShapeControlPoint cp{};
    cp.posX = 0; cp.posY = 0; cp.posZ = 0;
    cp.scaleX = 1.5f; cp.scaleY = 1.5f; cp.scaleZ = 1.5f;
    cp.weight = 1.0f;
    panel.AddControlPoint(cp);

    panel.ApplyAndPreview();
    assert(panel.HasPreview());
    assert(!panel.GetPreviewShip().shipName.empty());
    assert(panel.GetPreviewShip().mass > 0.0f);
    ok("test_asp_apply_and_preview_ship");
}

void test_asp_apply_and_preview_station() {
    atlas::editor::AssetStylePanel panel;
    panel.NewStyle("Station Preview",
                   atlas::pcg::GenerationStyleType::StationLayout);
    panel.ApplyAndPreview();
    assert(panel.HasPreview());
    assert(!panel.GetPreviewStation().modules.empty());
    ok("test_asp_apply_and_preview_station");
}

void test_asp_library_save_load() {
    atlas::editor::AssetStylePanel panel;
    panel.NewStyle("LibStyle",
                   atlas::pcg::GenerationStyleType::ShipLayout);
    panel.NewShapeProfile("Lib Shape");
    atlas::pcg::ShapeControlPoint cp{};
    cp.scaleX = 1.2f; cp.scaleY = 1.0f; cp.scaleZ = 1.0f;
    cp.weight = 1.0f;
    panel.AddControlPoint(cp);
    panel.SaveToLibrary();
    assert(panel.GetLibrary().size() == 1);

    // Create a different style and then load back.
    panel.NewStyle("Other", atlas::pcg::GenerationStyleType::StationLayout);
    assert(panel.GetCurrentStyle().name == "Other");

    bool loaded = panel.LoadFromLibrary("LibStyle");
    assert(loaded);
    assert(panel.GetCurrentStyle().name == "LibStyle");
    assert(panel.ControlPointCount() == 1);

    bool loadFail = panel.LoadFromLibrary("Missing");
    assert(!loadFail);
    ok("test_asp_library_save_load");
}

void test_asp_serialize_roundtrip() {
    atlas::editor::AssetStylePanel panel;
    panel.NewStyle("SerialTest",
                   atlas::pcg::GenerationStyleType::ShipLayout);
    panel.NewShapeProfile("Ser Shape");
    atlas::pcg::ShapeControlPoint cp{};
    cp.posX = 1; cp.posY = 2; cp.posZ = 3;
    cp.scaleX = 1.1f; cp.scaleY = 1.2f; cp.scaleZ = 1.3f;
    cp.weight = 0.8f;
    panel.AddControlPoint(cp);
    panel.NewPalette("Ser Palette");
    atlas::pcg::StyleColor col{};
    col.r = 0.1f; col.g = 0.2f; col.b = 0.3f; col.a = 1.0f;
    col.regionName = "Wing";
    panel.AddColor(col);

    std::string data = panel.SaveToString();
    assert(!data.empty());

    atlas::editor::AssetStylePanel panel2;
    panel2.LoadFromString(data);
    assert(panel2.GetCurrentStyle().name == "SerialTest");
    assert(panel2.ControlPointCount() == 1);
    assert(panel2.ColorCount() == 1);
    ok("test_asp_serialize_roundtrip");
}

void test_asp_draw_does_not_crash() {
    atlas::editor::AssetStylePanel panel;
    panel.ApplyAndPreview();
    panel.Draw();
    ok("test_asp_draw_does_not_crash");
}

// ═══════════════════════════════════════════════════════════════════
//  Ship Archetype Engine tests
// ═══════════════════════════════════════════════════════════════════

void test_arch_create_default_frigate() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Frigate);

    assert(!arch.name.empty());
    assert(arch.hullClass == atlas::pcg::HullClass::Frigate);
    assert(arch.version == 1);
    assert(!arch.hullShape.controlPoints.empty());
    assert(!arch.rooms.empty());
    assert(!arch.doors.empty());
    assert(!arch.hardpoints.empty());
    assert(arch.subsystems.size() == atlas::pcg::SUBSYSTEM_TYPE_COUNT);
    assert(!arch.moduleVisuals.empty());
    assert(arch.shapeVariation > 0.0f);
    ok("test_arch_create_default_frigate");
}

void test_arch_create_all_hull_classes() {
    atlas::pcg::HullClass classes[] = {
        atlas::pcg::HullClass::Frigate,
        atlas::pcg::HullClass::Destroyer,
        atlas::pcg::HullClass::Cruiser,
        atlas::pcg::HullClass::Battlecruiser,
        atlas::pcg::HullClass::Battleship,
        atlas::pcg::HullClass::Capital,
    };
    for (auto hull : classes) {
        auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(hull);
        assert(!arch.name.empty());
        assert(arch.hullClass == hull);
        assert(!arch.rooms.empty());
        assert(!arch.hardpoints.empty());
    }
    ok("test_arch_create_all_hull_classes");
}

void test_arch_capital_bigger_than_frigate() {
    auto frigate = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Frigate);
    auto capital = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Capital);

    assert(capital.rooms.size() > frigate.rooms.size());
    assert(capital.hardpoints.size() > frigate.hardpoints.size());
    assert(capital.deckCount > frigate.deckCount);
    ok("test_arch_capital_bigger_than_frigate");
}

void test_arch_validate_valid() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    bool valid = atlas::pcg::ShipArchetypeEngine::validate(arch);
    assert(valid);
    assert(arch.valid);
    ok("test_arch_validate_valid");
}

void test_arch_validate_empty_name_fails() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    arch.name = "";
    assert(!atlas::pcg::ShipArchetypeEngine::validate(arch));
    ok("test_arch_validate_empty_name_fails");
}

void test_arch_validate_no_rooms_fails() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    arch.rooms.clear();
    assert(!atlas::pcg::ShipArchetypeEngine::validate(arch));
    ok("test_arch_validate_no_rooms_fails");
}

void test_arch_validate_no_hardpoints_fails() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    arch.hardpoints.clear();
    assert(!atlas::pcg::ShipArchetypeEngine::validate(arch));
    ok("test_arch_validate_no_hardpoints_fails");
}

void test_arch_validate_duplicate_hardpoint_ids_fails() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    if (arch.hardpoints.size() >= 2) {
        arch.hardpoints[1].hardpointId = arch.hardpoints[0].hardpointId;
        assert(!atlas::pcg::ShipArchetypeEngine::validate(arch));
    }
    ok("test_arch_validate_duplicate_hardpoint_ids_fails");
}

void test_arch_validate_door_bad_room_ref_fails() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    atlas::pcg::DoorPlacement badDoor{};
    badDoor.doorId     = 999;
    badDoor.fromRoomId = 9998;
    badDoor.toRoomId   = 9999;
    arch.doors.push_back(badDoor);
    assert(!atlas::pcg::ShipArchetypeEngine::validate(arch));
    ok("test_arch_validate_door_bad_room_ref_fails");
}

void test_arch_generate_from_archetype() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    atlas::pcg::ShipArchetypeEngine::validate(arch);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);

    auto variant = atlas::pcg::ShipArchetypeEngine::generateFromArchetype(
        ctx, arch);
    assert(variant.valid);
    assert(!variant.ship.shipName.empty());
    assert(variant.ship.mass > 0.0f);
    assert(!variant.hardpoints.empty());
    assert(!variant.doors.empty());
    assert(!variant.interior.rooms.empty());
    assert(variant.archetypeName == arch.name);
    ok("test_arch_generate_from_archetype");
}

void test_arch_generate_determinism() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Battleship);
    atlas::pcg::ShipArchetypeEngine::validate(arch);

    atlas::pcg::PCGManager mgr1, mgr2;
    mgr1.initialize(42);
    mgr2.initialize(42);
    auto ctx1 = mgr1.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);
    auto ctx2 = mgr2.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);

    auto v1 = atlas::pcg::ShipArchetypeEngine::generateFromArchetype(ctx1, arch);
    auto v2 = atlas::pcg::ShipArchetypeEngine::generateFromArchetype(ctx2, arch);

    assert(v1.ship.shipName == v2.ship.shipName);
    assert(v1.ship.mass == v2.ship.mass);
    assert(v1.hardpoints.size() == v2.hardpoints.size());
    ok("test_arch_generate_determinism");
}

void test_arch_apply_subsystems() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    atlas::pcg::ShipArchetypeEngine::validate(arch);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);

    auto variant = atlas::pcg::ShipArchetypeEngine::generateFromArchetype(
        ctx, arch);
    float preShield = variant.ship.shieldHP;
    float preArmor  = variant.ship.armorHP;

    // Activate "Shield Amplifier" (variant 0 in Defensive slot).
    std::vector<int> active(arch.subsystems.size(), -1);
    for (size_t i = 0; i < arch.subsystems.size(); ++i) {
        if (arch.subsystems[i].type == atlas::pcg::SubsystemType::Defensive) {
            active[i] = 0;  // Shield Amplifier
            break;
        }
    }

    atlas::pcg::ShipArchetypeEngine::applySubsystems(
        variant, arch, active);

    // Shield should have increased.
    assert(variant.ship.shieldHP > preShield);
    ok("test_arch_apply_subsystems");
}

void test_arch_apply_module_visuals() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    atlas::pcg::ShipArchetypeEngine::validate(arch);

    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);

    auto variant = atlas::pcg::ShipArchetypeEngine::generateFromArchetype(
        ctx, arch);
    float preMass = variant.ship.mass;

    std::vector<std::string> modules = {"shield", "armor", "weapon"};
    atlas::pcg::ShipArchetypeEngine::applyModuleVisuals(
        variant, arch, modules);

    // Mass should have changed (module visuals add geometry).
    assert(variant.ship.mass != preMass);
    ok("test_arch_apply_module_visuals");
}

void test_arch_subsystem_type_names() {
    assert(std::string(atlas::pcg::ShipArchetypeEngine::subsystemTypeName(
        atlas::pcg::SubsystemType::Offensive)) == "Offensive");
    assert(std::string(atlas::pcg::ShipArchetypeEngine::subsystemTypeName(
        atlas::pcg::SubsystemType::Propulsion)) == "Propulsion");
    assert(std::string(atlas::pcg::ShipArchetypeEngine::subsystemTypeName(
        atlas::pcg::SubsystemType::Core)) == "Core");
    ok("test_arch_subsystem_type_names");
}

void test_arch_serialize_roundtrip() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Destroyer);

    std::string data = atlas::pcg::ShipArchetypeEngine::serialize(arch);
    assert(!data.empty());

    auto loaded = atlas::pcg::ShipArchetypeEngine::deserialize(data);
    assert(loaded.name == arch.name);
    assert(loaded.hullClass == atlas::pcg::HullClass::Destroyer);
    assert(loaded.rooms.size() == arch.rooms.size());
    assert(loaded.doors.size() == arch.doors.size());
    assert(loaded.hardpoints.size() == arch.hardpoints.size());
    assert(loaded.subsystems.size() == arch.subsystems.size());
    assert(loaded.valid);
    ok("test_arch_serialize_roundtrip");
}

void test_arch_different_seeds_differ() {
    auto arch = atlas::pcg::ShipArchetypeEngine::createDefault(
        atlas::pcg::HullClass::Cruiser);
    atlas::pcg::ShipArchetypeEngine::validate(arch);

    atlas::pcg::PCGManager mgr1, mgr2;
    mgr1.initialize(100);
    mgr2.initialize(200);
    auto ctx1 = mgr1.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);
    auto ctx2 = mgr2.makeRootContext(atlas::pcg::PCGDomain::Ship, 1, 1);

    auto v1 = atlas::pcg::ShipArchetypeEngine::generateFromArchetype(ctx1, arch);
    auto v2 = atlas::pcg::ShipArchetypeEngine::generateFromArchetype(ctx2, arch);

    // Different seeds should produce different ships.
    assert(v1.ship.shipName != v2.ship.shipName ||
           v1.ship.mass != v2.ship.mass);
    ok("test_arch_different_seeds_differ");
}

// ═══════════════════════════════════════════════════════════════════
//  Ship Archetype Panel tests
// ═══════════════════════════════════════════════════════════════════

void test_archp_defaults() {
    atlas::editor::ShipArchetypePanel panel;
    assert(std::string(panel.Name()) == "Ship Archetype");
    assert(panel.GetHullClass() == atlas::pcg::HullClass::Frigate);
    assert(panel.RoomCount() > 0);
    assert(panel.DoorCount() > 0);
    assert(panel.HardpointCount() > 0);
    assert(panel.SubsystemSlotCount() == atlas::pcg::SUBSYSTEM_TYPE_COUNT);
    assert(!panel.HasPreview());
    ok("test_archp_defaults");
}

void test_archp_select_hull_class() {
    atlas::editor::ShipArchetypePanel panel;
    panel.SelectHullClass(atlas::pcg::HullClass::Battleship);
    assert(panel.GetHullClass() == atlas::pcg::HullClass::Battleship);
    assert(panel.RoomCount() > 5);
    assert(panel.HardpointCount() > 5);
    assert(!panel.Log().empty());
    ok("test_archp_select_hull_class");
}

void test_archp_add_remove_hull_control_point() {
    atlas::editor::ShipArchetypePanel panel;
    size_t initial = panel.HullControlPointCount();

    atlas::pcg::ShapeControlPoint cp{};
    cp.posX = 10; cp.posY = 0; cp.posZ = 5;
    cp.scaleX = 1.2f; cp.scaleY = 1.0f; cp.scaleZ = 1.0f;
    cp.weight = 0.8f;
    panel.AddHullControlPoint(cp);
    assert(panel.HullControlPointCount() == initial + 1);

    bool removed = panel.RemoveHullControlPoint(initial);
    assert(removed);
    assert(panel.HullControlPointCount() == initial);
    assert(!panel.RemoveHullControlPoint(999));
    ok("test_archp_add_remove_hull_control_point");
}

void test_archp_add_remove_room() {
    atlas::editor::ShipArchetypePanel panel;
    size_t initial = panel.RoomCount();

    atlas::pcg::InteriorRoom room{};
    room.roomId = 100;
    room.type   = atlas::pcg::InteriorRoomType::Hangar;
    room.posX   = 20.0f;
    room.dimX   = 10.0f;
    room.dimY   = 8.0f;
    room.dimZ   = 5.0f;
    panel.AddRoom(room);
    assert(panel.RoomCount() == initial + 1);

    bool removed = panel.RemoveRoom(100);
    assert(removed);
    assert(panel.RoomCount() == initial);
    assert(!panel.RemoveRoom(9999));
    ok("test_archp_add_remove_room");
}

void test_archp_add_remove_door() {
    atlas::editor::ShipArchetypePanel panel;
    size_t initial = panel.DoorCount();

    atlas::pcg::DoorPlacement door{};
    door.doorId     = 100;
    door.fromRoomId = 0;
    door.toRoomId   = 1;
    door.posX = 5.0f;
    door.width  = 1.5f;
    door.height = 2.5f;
    door.isAirlock = true;
    door.locked    = true;
    panel.AddDoor(door);
    assert(panel.DoorCount() == initial + 1);

    bool removed = panel.RemoveDoor(100);
    assert(removed);
    assert(panel.DoorCount() == initial);
    assert(!panel.RemoveDoor(9999));
    ok("test_archp_add_remove_door");
}

void test_archp_add_remove_hardpoint() {
    atlas::editor::ShipArchetypePanel panel;
    size_t initial = panel.HardpointCount();

    atlas::pcg::HardpointDefinition hp{};
    hp.hardpointId = 100;
    hp.posX = 15.0f; hp.posY = 3.0f; hp.posZ = 2.0f;
    hp.facingDeg = 0.0f;
    hp.arcDeg    = 120.0f;
    hp.size      = atlas::pcg::WeaponSize::Large;
    hp.isDorsal  = true;
    hp.groupTag  = "broadside";
    panel.AddHardpoint(hp);
    assert(panel.HardpointCount() == initial + 1);

    // Update.
    hp.arcDeg = 180.0f;
    bool updated = panel.UpdateHardpoint(100, hp);
    assert(updated);
    assert(!panel.UpdateHardpoint(9999, hp));

    bool removed = panel.RemoveHardpoint(100);
    assert(removed);
    assert(panel.HardpointCount() == initial);
    assert(!panel.RemoveHardpoint(9999));
    ok("test_archp_add_remove_hardpoint");
}

void test_archp_subsystem_editing() {
    atlas::editor::ShipArchetypePanel panel;

    // Add a custom variant to the first subsystem slot.
    atlas::pcg::SubsystemVariant var{};
    var.name       = "Custom Variant";
    var.type       = atlas::pcg::SubsystemType::Offensive;
    var.dpsBonus   = 0.25f;
    var.shapeModifier.name = "Custom Shape";
    var.shapeModifier.smoothing = 0.5f;

    bool added = panel.AddSubsystemVariant(0, var);
    assert(added);
    assert(!panel.AddSubsystemVariant(999, var));

    // Set active variant.
    bool set = panel.SetActiveVariant(0, 0);
    assert(set);
    assert(!panel.SetActiveVariant(999, 0));
    ok("test_archp_subsystem_editing");
}

void test_archp_module_visual_rules() {
    atlas::editor::ShipArchetypePanel panel;
    size_t initial = panel.ModuleVisualRuleCount();

    atlas::pcg::ModuleVisualRule rule{};
    rule.moduleCategory = "drone";
    rule.effectType     = "bay";
    rule.scaleMin = 0.5f;
    rule.scaleMax = 1.5f;
    panel.AddModuleVisualRule(rule);
    assert(panel.ModuleVisualRuleCount() == initial + 1);

    bool removed = panel.RemoveModuleVisualRule(initial);
    assert(removed);
    assert(panel.ModuleVisualRuleCount() == initial);
    assert(!panel.RemoveModuleVisualRule(999));
    ok("test_archp_module_visual_rules");
}

void test_archp_variation_bounds() {
    atlas::editor::ShipArchetypePanel panel;
    panel.SetVariationBounds(0.3f, 0.2f, 0.4f);
    assert(std::fabs(panel.GetArchetype().shapeVariation - 0.3f) < 0.01f);
    assert(std::fabs(panel.GetArchetype().sizeVariation - 0.2f) < 0.01f);
    assert(std::fabs(panel.GetArchetype().detailVariation - 0.4f) < 0.01f);
    ok("test_archp_variation_bounds");
}

void test_archp_generate_preview() {
    atlas::editor::ShipArchetypePanel panel;
    panel.SelectHullClass(atlas::pcg::HullClass::Cruiser);
    panel.GeneratePreview();
    assert(panel.HasPreview());
    assert(panel.GetPreview().valid);
    assert(!panel.GetPreview().ship.shipName.empty());
    assert(!panel.GetPreview().hardpoints.empty());
    assert(!panel.Log().empty());
    ok("test_archp_generate_preview");
}

void test_archp_generate_preview_with_subsystems() {
    atlas::editor::ShipArchetypePanel panel;
    panel.SelectHullClass(atlas::pcg::HullClass::Cruiser);

    // Activate variant 0 for all subsystem slots.
    std::vector<int> active(atlas::pcg::SUBSYSTEM_TYPE_COUNT, 0);
    panel.GeneratePreviewWithSubsystems(active);
    assert(panel.HasPreview());
    assert(panel.GetPreview().valid);
    ok("test_archp_generate_preview_with_subsystems");
}

void test_archp_generate_preview_with_modules() {
    atlas::editor::ShipArchetypePanel panel;
    panel.SelectHullClass(atlas::pcg::HullClass::Battleship);

    std::vector<std::string> modules = {"shield", "weapon", "engine"};
    panel.GeneratePreviewWithModules(modules);
    assert(panel.HasPreview());
    assert(!panel.Log().empty());
    ok("test_archp_generate_preview_with_modules");
}

void test_archp_save_load_string() {
    atlas::editor::ShipArchetypePanel panel;
    panel.SelectHullClass(atlas::pcg::HullClass::Destroyer);

    std::string data = panel.SaveToString();
    assert(!data.empty());

    atlas::editor::ShipArchetypePanel panel2;
    panel2.LoadFromString(data);
    assert(panel2.GetArchetype().hullClass ==
           atlas::pcg::HullClass::Destroyer);
    assert(panel2.RoomCount() == panel.RoomCount());
    assert(panel2.HardpointCount() == panel.HardpointCount());
    ok("test_archp_save_load_string");
}

void test_archp_draw_does_not_crash() {
    atlas::editor::ShipArchetypePanel panel;
    panel.GeneratePreview();
    panel.Draw();
    ok("test_archp_draw_does_not_crash");
}

void test_archp_save_load_file() {
    atlas::editor::ShipArchetypePanel panel;
    panel.SelectHullClass(atlas::pcg::HullClass::Cruiser);

    std::string path = "/tmp/test_archp_save_load.json";
    bool saved = panel.SaveToFile(path);
    assert(saved);

    atlas::editor::ShipArchetypePanel panel2;
    bool loaded = panel2.LoadFromFile(path);
    assert(loaded);
    assert(panel2.GetArchetype().hullClass ==
           atlas::pcg::HullClass::Cruiser);
    assert(panel2.RoomCount() == panel.RoomCount());
    assert(panel2.HardpointCount() == panel.HardpointCount());
    ok("test_archp_save_load_file");
}

void test_archp_load_nonexistent_file() {
    atlas::editor::ShipArchetypePanel panel;
    bool loaded = panel.LoadFromFile("/tmp/nonexistent_archetype_test.json");
    assert(!loaded);
    ok("test_archp_load_nonexistent_file");
}

void test_gsp_save_load_file() {
    atlas::editor::GenerationStylePanel panel;
    panel.NewStyle(atlas::pcg::GenerationStyleType::StationLayout,
                   "FileTestStation");

    std::string path = "/tmp/test_gsp_save_load.json";
    bool saved = panel.SaveStyleToFile(path);
    assert(saved);

    atlas::editor::GenerationStylePanel panel2;
    bool loaded = panel2.LoadStyleFromFile(path);
    assert(loaded);
    assert(panel2.GetStyle().name == "FileTestStation");
    assert(panel2.GetStyle().type ==
           atlas::pcg::GenerationStyleType::StationLayout);
    ok("test_gsp_save_load_file");
}

void test_gsp_load_nonexistent_file() {
    atlas::editor::GenerationStylePanel panel;
    bool loaded = panel.LoadStyleFromFile("/tmp/nonexistent_style_test.json");
    assert(!loaded);
    ok("test_gsp_load_nonexistent_file");
}
