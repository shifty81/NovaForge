// Tests for: AbyssalEscalationSystem
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/abyssal_escalation_system.h"

using namespace atlas;
using Phase = components::AbyssalEscalationState::EscalationPhase;

// ==================== AbyssalEscalationSystem Tests ====================

static void testAbyssalEscalationCreate() {
    std::cout << "\n=== AbyssalEscalation: Create ===" << std::endl;
    ecs::World world;
    systems::AbyssalEscalationSystem sys(&world);
    world.createEntity("pocket1");
    assertTrue(sys.initialize("pocket1", "pocket_01", 1), "Init T1 succeeds");
    assertTrue(sys.getCurrentPhase("pocket1") == Phase::Wave1, "Start at Wave1");
    assertTrue(!sys.isBossSpawned("pocket1"), "No boss spawned");
    assertTrue(!sys.isBossKilled("pocket1"), "Boss not killed");
    assertTrue(!sys.isRunCompleted("pocket1"), "Not complete");
    assertTrue(sys.getEnemiesKilled("pocket1") == 0, "0 enemies killed");
    assertTrue(approxEqual(sys.getDpsReceived("pocket1"), 0.0f), "0 DPS received");
    assertTrue(sys.getTotalLootValue("pocket1") == 0, "0 loot");
}

static void testAbyssalEscalationWaveProgression() {
    std::cout << "\n=== AbyssalEscalation: WaveProgression ===" << std::endl;
    ecs::World world;
    systems::AbyssalEscalationSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1", "p1", 1);

    assertTrue(sys.completeWave("pocket1"), "Complete Wave1");
    assertTrue(sys.getCurrentPhase("pocket1") == Phase::Wave2, "Now at Wave2");

    assertTrue(sys.completeWave("pocket1"), "Complete Wave2");
    assertTrue(sys.getCurrentPhase("pocket1") == Phase::Boss, "Now at Boss");
}

static void testAbyssalEscalationBossFlow() {
    std::cout << "\n=== AbyssalEscalation: BossFlow ===" << std::endl;
    ecs::World world;
    systems::AbyssalEscalationSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1", "p1", 1);

    sys.completeWave("pocket1");
    sys.completeWave("pocket1");

    // Can't spawn boss before reaching Boss phase -- already there
    assertTrue(sys.spawnBoss("pocket1"), "Spawn boss at Boss phase");
    assertTrue(sys.isBossSpawned("pocket1"), "Boss is spawned");

    assertTrue(sys.killBoss("pocket1"), "Kill boss");
    assertTrue(sys.isBossKilled("pocket1"), "Boss killed");
    assertTrue(sys.isRunCompleted("pocket1"), "Run completed");
    assertTrue(!sys.isBossSpawned("pocket1") || sys.isBossKilled("pocket1"),
               "Boss state consistent");
}

static void testAbyssalEscalationLootScaling() {
    std::cout << "\n=== AbyssalEscalation: LootScaling ===" << std::endl;
    ecs::World world;
    systems::AbyssalEscalationSystem sys(&world);

    for (int tier = 1; tier <= 5; tier++) {
        std::string eid = "pocket_t" + std::to_string(tier);
        world.createEntity(eid);
        sys.initialize(eid, "p", tier);
        sys.completeWave(eid);
        sys.completeWave(eid);
        sys.spawnBoss(eid);
        sys.killBoss(eid);
        int expected = 10000000 * tier;
        assertTrue(sys.getTotalLootValue(eid) == expected,
                   "T" + std::to_string(tier) + " loot = " + std::to_string(expected));
    }
}

static void testAbyssalEscalationDamageTracking() {
    std::cout << "\n=== AbyssalEscalation: DamageTracking ===" << std::endl;
    ecs::World world;
    systems::AbyssalEscalationSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1", "p1", 2);

    sys.applyDamage("pocket1", 1000.0f);
    sys.applyDamage("pocket1", 500.0f);
    assertTrue(approxEqual(sys.getDpsReceived("pocket1"), 1500.0f),
               "1500 total damage");
}

static void testAbyssalEscalationKillTracking() {
    std::cout << "\n=== AbyssalEscalation: KillTracking ===" << std::endl;
    ecs::World world;
    systems::AbyssalEscalationSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1", "p1", 1);

    for (int i = 0; i < 12; i++) sys.recordKill("pocket1");
    assertTrue(sys.getEnemiesKilled("pocket1") == 12, "12 enemies killed");
}

static void testAbyssalEscalationDoubleBoss() {
    std::cout << "\n=== AbyssalEscalation: DoubleBoss ===" << std::endl;
    ecs::World world;
    systems::AbyssalEscalationSystem sys(&world);
    world.createEntity("pocket1");
    sys.initialize("pocket1", "p1", 1);
    sys.completeWave("pocket1");
    sys.completeWave("pocket1");
    sys.spawnBoss("pocket1");
    assertTrue(!sys.spawnBoss("pocket1"), "Can't spawn boss twice");
}

static void testAbyssalEscalationMissing() {
    std::cout << "\n=== AbyssalEscalation: Missing ===" << std::endl;
    ecs::World world;
    systems::AbyssalEscalationSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.completeWave("nonexistent"), "CompleteWave fails");
    assertTrue(!sys.spawnBoss("nonexistent"), "SpawnBoss fails");
    assertTrue(!sys.killBoss("nonexistent"), "KillBoss fails");
    assertTrue(!sys.applyDamage("nonexistent", 100.0f), "ApplyDamage fails");
    assertTrue(!sys.recordKill("nonexistent"), "RecordKill fails");
    assertTrue(sys.getCurrentPhase("nonexistent") == Phase::Wave1, "Default phase");
    assertTrue(!sys.isBossSpawned("nonexistent"), "Not spawned");
    assertTrue(!sys.isBossKilled("nonexistent"), "Not killed");
    assertTrue(!sys.isRunCompleted("nonexistent"), "Not complete");
    assertTrue(sys.getEnemiesKilled("nonexistent") == 0, "0 kills");
    assertTrue(sys.getTotalLootValue("nonexistent") == 0, "0 loot");
}

void run_abyssal_escalation_system_tests() {
    testAbyssalEscalationCreate();
    testAbyssalEscalationWaveProgression();
    testAbyssalEscalationBossFlow();
    testAbyssalEscalationLootScaling();
    testAbyssalEscalationDamageTracking();
    testAbyssalEscalationKillTracking();
    testAbyssalEscalationDoubleBoss();
    testAbyssalEscalationMissing();
}
