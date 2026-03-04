// Tests for: ShieldRechargeSystem Tests, Shield Effect Generator Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "pcg/pcg_context.h"
#include "pcg/shield_effect_generator.h"
#include "systems/shield_recharge_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== ShieldRechargeSystem Tests ====================

static void testShieldRecharge() {
    std::cout << "\n=== Shield Recharge ===" << std::endl;
    
    ecs::World world;
    systems::ShieldRechargeSystem shieldSys(&world);
    
    auto* entity = world.createEntity("test_ship");
    auto* health = addComp<components::Health>(entity);
    health->shield_hp = 50.0f;
    health->shield_max = 100.0f;
    health->shield_recharge_rate = 5.0f;
    
    shieldSys.update(2.0f);
    assertTrue(approxEqual(health->shield_hp, 60.0f), "Shield recharges by rate * delta_time");
    
    shieldSys.update(10.0f);
    assertTrue(approxEqual(health->shield_hp, 100.0f), "Shield does not exceed max");
    
    shieldSys.update(1.0f);
    assertTrue(approxEqual(health->shield_hp, 100.0f), "Full shields stay at max");
}

static void testShieldPercentage() {
    std::cout << "\n=== Shield Percentage ===" << std::endl;
    
    ecs::World world;
    systems::ShieldRechargeSystem shieldSys(&world);
    
    auto* entity = world.createEntity("test_ship");
    auto* health = addComp<components::Health>(entity);
    health->shield_hp = 40.0f;
    health->shield_max = 200.0f;
    
    float pct = shieldSys.getShieldPercentage("test_ship");
    assertTrue(approxEqual(pct, 0.2f), "Shield percentage is correct (20%)");
    
    float noEntity = shieldSys.getShieldPercentage("nonexistent");
    assertTrue(noEntity < 0.0f, "Returns -1 for nonexistent entity");
}


// ==================== Shield Effect Generator Tests ====================

static void testShieldEffectGeneration() {
    std::cout << "\n=== Shield Effect Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto shield = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    assertTrue(shield.valid, "Shield effect is valid");
    assertTrue(shield.base_opacity > 0.0f, "Base opacity positive");
    assertTrue(shield.pattern_scale > 0.0f, "Pattern scale positive");
    assertTrue(shield.shield_radius >= 1.0f, "Shield radius >= 1.0");
    assertTrue(shield.fresnel_power >= 1.0f, "Fresnel power >= 1.0");
    assertTrue(static_cast<int>(shield.sample_impacts.size()) == 3, "Has 3 sample impacts");
}

static void testShieldEffectDeterminism() {
    std::cout << "\n=== Shield Effect Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto s1 = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Battleship, "Solari");
    auto s2 = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Battleship, "Solari");
    assertTrue(s1.pattern == s2.pattern, "Same seed same pattern");
    assertTrue(approxEqual(s1.base_color_r, s2.base_color_r), "Same seed same base color R");
    assertTrue(approxEqual(s1.base_opacity, s2.base_opacity), "Same seed same opacity");
    assertTrue(approxEqual(s1.shimmer_speed, s2.shimmer_speed), "Same seed same shimmer speed");
    assertTrue(static_cast<int>(s1.sample_impacts.size()) == static_cast<int>(s2.sample_impacts.size()),
               "Same seed same impact count");
}

static void testShieldPatternByFaction() {
    std::cout << "\n=== Shield Pattern By Faction ===" << std::endl;
    // Test pattern tendencies (statistical — use fixed seeds that produce expected results).
    pcg::PCGContext ctx{ 42, 1 };
    auto veyren  = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    auto solari  = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Solari");
    // Veyren should be Hexagonal or Lattice.
    assertTrue(veyren.pattern == pcg::ShieldPattern::Hexagonal
            || veyren.pattern == pcg::ShieldPattern::Lattice,
               "Veyren pattern is tech-style");
    // Solari should be Ornate or Smooth.
    assertTrue(solari.pattern == pcg::ShieldPattern::Ornate
            || solari.pattern == pcg::ShieldPattern::Smooth,
               "Solari pattern is ornate/smooth");
}

static void testShieldScalesWithClass() {
    std::cout << "\n=== Shield Scales With Class ===" << std::endl;
    pcg::PCGContext ctx{ 123, 1 };
    auto frigate = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Frigate, "Veyren");
    auto capital = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Capital, "Veyren");
    assertTrue(capital.pattern_scale > frigate.pattern_scale,
               "Capital pattern scale larger than frigate");
    assertTrue(capital.shield_radius > frigate.shield_radius,
               "Capital shield radius larger than frigate");
}

static void testShieldColorByFaction() {
    std::cout << "\n=== Shield Color By Faction ===" << std::endl;
    pcg::PCGContext ctx{ 300, 1 };
    auto veyren  = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    auto keldari = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Keldari");
    // Veyren shields are blue-dominant.
    assertTrue(veyren.base_color_b > veyren.base_color_r,
               "Veyren shield is blue (B > R)");
    // Keldari shields are warm.
    assertTrue(keldari.base_color_r > keldari.base_color_b,
               "Keldari shield is warm (R > B)");
}

static void testShieldImpactRipples() {
    std::cout << "\n=== Shield Impact Ripples ===" << std::endl;
    pcg::PCGContext ctx{ 400, 1 };
    auto shield = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Aurelian");
    for (const auto& imp : shield.sample_impacts) {
        assertTrue(imp.intensity >= 0.0f && imp.intensity <= 1.0f,
                   "Impact intensity in [0,1]");
        assertTrue(imp.radius > 0.0f, "Impact radius positive");
        assertTrue(imp.decay_rate > 0.0f, "Impact decay rate positive");
        assertTrue(imp.speed > 0.0f, "Impact speed positive");
    }
}

static void testShieldPatternNames() {
    std::cout << "\n=== Shield Pattern Names ===" << std::endl;
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Hexagonal) == "Hexagonal",
               "Hexagonal name");
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Smooth) == "Smooth",
               "Smooth name");
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Lattice) == "Lattice",
               "Lattice name");
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Ornate) == "Ornate",
               "Ornate name");
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Ripple) == "Ripple",
               "Ripple name");
}

static void testShieldAllClassesValid() {
    std::cout << "\n=== Shield All Classes Valid ===" << std::endl;
    std::vector<pcg::HullClass> classes = {
        pcg::HullClass::Frigate, pcg::HullClass::Destroyer,
        pcg::HullClass::Cruiser, pcg::HullClass::Battlecruiser,
        pcg::HullClass::Battleship, pcg::HullClass::Capital
    };
    bool allValid = true;
    for (auto hc : classes) {
        pcg::PCGContext ctx{ static_cast<uint64_t>(hc) + 2000, 1 };
        auto shield = pcg::ShieldEffectGenerator::generate(ctx, hc, "Aurelian");
        if (!shield.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All hull classes produce valid shield effects");
}


void run_shield_recharge_system_tests() {
    testShieldRecharge();
    testShieldPercentage();
    testShieldEffectGeneration();
    testShieldEffectDeterminism();
    testShieldPatternByFaction();
    testShieldScalesWithClass();
    testShieldColorByFaction();
    testShieldImpactRipples();
    testShieldPatternNames();
    testShieldAllClassesValid();
}
