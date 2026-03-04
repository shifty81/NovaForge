// Tests for: Difficulty Scaling System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/difficulty_scaling_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Difficulty Scaling System Tests ====================

static void testDifficultyInitializeZone() {
    ecs::World world;
    systems::DifficultyScalingSystem sys(&world);

    auto* sys_entity = world.createEntity("sys1");
    auto dz = std::make_unique<components::DifficultyZone>();
    sys_entity->addComponent(std::move(dz));

    bool ok = sys.initializeZone("sys1", 0.5f);
    assertTrue(ok, "initializeZone returns true");

    auto* zone = sys_entity->getComponent<components::DifficultyZone>();
    assertTrue(approxEqual(zone->security_status, 0.5f, 0.01f),
               "Security status set correctly");
    assertTrue(zone->npc_hp_multiplier > 1.0f,
               "HP multiplier > 1 for midsec");
}

static void testDifficultyHighsecLowMultipliers() {
    float hp = systems::DifficultyScalingSystem::hpMultiplierFromSecurity(1.0f);
    float dmg = systems::DifficultyScalingSystem::damageMultiplierFromSecurity(1.0f);
    assertTrue(approxEqual(hp, 1.0f, 0.01f), "Highsec HP multiplier = 1.0");
    assertTrue(approxEqual(dmg, 1.0f, 0.01f), "Highsec damage multiplier = 1.0");
}

static void testDifficultyNullsecHighMultipliers() {
    float hp = systems::DifficultyScalingSystem::hpMultiplierFromSecurity(0.0f);
    float dmg = systems::DifficultyScalingSystem::damageMultiplierFromSecurity(0.0f);
    assertTrue(hp > 2.0f, "Nullsec HP multiplier > 2.0");
    assertTrue(dmg > 1.5f, "Nullsec damage multiplier > 1.5");
}

static void testDifficultyLootScaling() {
    float highsec = systems::DifficultyScalingSystem::lootMultiplierFromSecurity(1.0f);
    float nullsec = systems::DifficultyScalingSystem::lootMultiplierFromSecurity(0.0f);
    assertTrue(nullsec > highsec, "Nullsec has better loot than highsec");
}

static void testDifficultyOreScaling() {
    float highsec = systems::DifficultyScalingSystem::oreMultiplierFromSecurity(1.0f);
    float nullsec = systems::DifficultyScalingSystem::oreMultiplierFromSecurity(0.0f);
    assertTrue(nullsec > highsec, "Nullsec has richer ore than highsec");
}

static void testDifficultyMaxTierFromSecurity() {
    int highsec = systems::DifficultyScalingSystem::maxTierFromSecurity(1.0f);
    int nullsec = systems::DifficultyScalingSystem::maxTierFromSecurity(0.0f);
    assertTrue(highsec == 1, "Highsec max tier = 1");
    assertTrue(nullsec == 5, "Nullsec max tier = 5");
}

static void testDifficultyApplyToNPC() {
    ecs::World world;
    systems::DifficultyScalingSystem sys(&world);

    auto* sys_entity = world.createEntity("sys1");
    auto dz = std::make_unique<components::DifficultyZone>();
    sys_entity->addComponent(std::move(dz));
    sys.initializeZone("sys1", 0.0f);  // nullsec

    auto* npc = world.createEntity("npc1");
    auto hp = std::make_unique<components::Health>();
    hp->hull_hp = 100.0f;
    hp->hull_max = 100.0f;
    hp->armor_hp = 100.0f;
    hp->armor_max = 100.0f;
    hp->shield_hp = 100.0f;
    hp->shield_max = 100.0f;
    npc->addComponent(std::move(hp));

    auto wpn = std::make_unique<components::Weapon>();
    wpn->damage = 10.0f;
    npc->addComponent(std::move(wpn));

    bool applied = sys.applyToNPC("npc1", "sys1");
    assertTrue(applied, "applyToNPC returns true");

    auto* health = npc->getComponent<components::Health>();
    assertTrue(health->hull_hp > 100.0f, "NPC hull HP scaled up in nullsec");
    assertTrue(health->shield_hp > 100.0f, "NPC shield HP scaled up in nullsec");

    auto* weapon = npc->getComponent<components::Weapon>();
    assertTrue(weapon->damage > 10.0f, "NPC damage scaled up in nullsec");
}

static void testDifficultySpawnRateScaling() {
    float highsec = systems::DifficultyScalingSystem::spawnRateFromSecurity(1.0f);
    float nullsec = systems::DifficultyScalingSystem::spawnRateFromSecurity(0.0f);
    assertTrue(approxEqual(highsec, 1.0f, 0.01f), "Highsec spawn rate = 1.0");
    assertTrue(nullsec > highsec, "Nullsec has higher spawn rate");
}


void run_difficulty_scaling_system_tests() {
    testDifficultyInitializeZone();
    testDifficultyHighsecLowMultipliers();
    testDifficultyNullsecHighMultipliers();
    testDifficultyLootScaling();
    testDifficultyOreScaling();
    testDifficultyMaxTierFromSecurity();
    testDifficultyApplyToNPC();
    testDifficultySpawnRateScaling();
}
