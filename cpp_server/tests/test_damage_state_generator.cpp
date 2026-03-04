// Tests for: Damage State Generator Tests
#include "test_log.h"
#include "components/core_components.h"
#include "pcg/pcg_context.h"
#include "pcg/damage_state_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Damage State Generator Tests ====================

static void testDamageStateGeneration() {
    std::cout << "\n=== Damage State Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto state = pcg::DamageStateGenerator::generate(ctx, 0.6f, pcg::HullClass::Cruiser);
    assertTrue(state.valid, "Damage state is valid");
    assertTrue(state.level == pcg::DamageLevel::Heavy, "0.6 damage = Heavy");
    assertTrue(!state.decals.empty(), "Heavy damage produces decals");
    assertTrue(state.hull_breach_count > 0, "Heavy damage has hull breaches");
    assertTrue(state.structural_integrity > 0.0f, "Structural integrity positive");
    assertTrue(state.structural_integrity < 1.0f, "Structural integrity reduced");
}

static void testDamageStateDeterminism() {
    std::cout << "\n=== Damage State Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto s1 = pcg::DamageStateGenerator::generate(ctx, 0.4f, pcg::HullClass::Battleship);
    auto s2 = pcg::DamageStateGenerator::generate(ctx, 0.4f, pcg::HullClass::Battleship);
    assertTrue(s1.level == s2.level, "Same seed same damage level");
    assertTrue(s1.hull_breach_count == s2.hull_breach_count, "Same seed same breach count");
    assertTrue(s1.missing_module_count == s2.missing_module_count, "Same seed same missing modules");
    assertTrue(static_cast<int>(s1.decals.size()) == static_cast<int>(s2.decals.size()),
               "Same seed same decal count");
    assertTrue(approxEqual(s1.structural_integrity, s2.structural_integrity),
               "Same seed same structural integrity");
}

static void testDamageStatePristine() {
    std::cout << "\n=== Damage State Pristine ===" << std::endl;
    pcg::PCGContext ctx{ 100, 1 };
    auto state = pcg::DamageStateGenerator::generate(ctx, 0.0f, pcg::HullClass::Frigate);
    assertTrue(state.level == pcg::DamageLevel::Pristine, "0.0 damage = Pristine");
    assertTrue(state.decals.empty(), "Pristine has no decals");
    assertTrue(state.hull_breach_count == 0, "Pristine has no breaches");
    assertTrue(state.missing_module_count == 0, "Pristine has no missing modules");
    assertTrue(approxEqual(state.structural_integrity, 1.0f), "Pristine integrity = 1.0");
}

static void testDamageStateCritical() {
    std::cout << "\n=== Damage State Critical ===" << std::endl;
    pcg::PCGContext ctx{ 200, 1 };
    auto state = pcg::DamageStateGenerator::generate(ctx, 0.95f, pcg::HullClass::Capital);
    assertTrue(state.level == pcg::DamageLevel::Critical, "0.95 damage = Critical");
    assertTrue(static_cast<int>(state.decals.size()) > 10, "Critical capital has many decals");
    assertTrue(state.hull_breach_count >= 2, "Critical has multiple breaches");
    assertTrue(state.structural_integrity < 0.3f, "Critical integrity very low");
}

static void testDamageStateScalesWithClass() {
    std::cout << "\n=== Damage State Scales With Class ===" << std::endl;
    pcg::PCGContext ctx{ 300, 1 };
    auto frigate = pcg::DamageStateGenerator::generate(ctx, 0.7f, pcg::HullClass::Frigate);
    auto capital = pcg::DamageStateGenerator::generate(ctx, 0.7f, pcg::HullClass::Capital);
    assertTrue(static_cast<int>(capital.decals.size()) > static_cast<int>(frigate.decals.size()),
               "Capital has more decals than frigate at same damage");
}

static void testDamageStateLevelNames() {
    std::cout << "\n=== Damage State Level Names ===" << std::endl;
    assertTrue(pcg::DamageStateGenerator::damageLevelName(pcg::DamageLevel::Pristine) == "Pristine", "Pristine name");
    assertTrue(pcg::DamageStateGenerator::damageLevelName(pcg::DamageLevel::Light) == "Light", "Light name");
    assertTrue(pcg::DamageStateGenerator::damageLevelName(pcg::DamageLevel::Heavy) == "Heavy", "Heavy name");
    assertTrue(pcg::DamageStateGenerator::decalTypeName(pcg::DecalType::HullBreach) == "HullBreach", "HullBreach name");
    assertTrue(pcg::DamageStateGenerator::decalTypeName(pcg::DecalType::ScorchMark) == "ScorchMark", "ScorchMark name");
}


void run_damage_state_generator_tests() {
    testDamageStateGeneration();
    testDamageStateDeterminism();
    testDamageStatePristine();
    testDamageStateCritical();
    testDamageStateScalesWithClass();
    testDamageStateLevelNames();
}
