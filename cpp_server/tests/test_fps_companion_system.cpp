// Tests for: FPSCompanionSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/fps_companion_system.h"

using namespace atlas;

// ==================== FPSCompanionSystem Tests ====================

static void testCompanionFollow() {
    std::cout << "\n=== FPSCompanion: Follow ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    addComp<components::FPSCompanion>(e);

    assertTrue(sys.getState("vip_1") == 0, "Initial state: Waiting");
    assertTrue(sys.startFollowing("vip_1", "player_1"), "Start following");
    assertTrue(sys.getState("vip_1") == 1, "State: Following");
}

static void testCompanionStopFollow() {
    std::cout << "\n=== FPSCompanion: Stop Follow ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    addComp<components::FPSCompanion>(e);

    sys.startFollowing("vip_1", "player_1");
    assertTrue(sys.stopFollowing("vip_1"), "Stop following");
    assertTrue(sys.getState("vip_1") == 0, "Back to Waiting");
    assertTrue(!sys.stopFollowing("vip_1"), "Can't stop when not following");
}

static void testCompanionDamage() {
    std::cout << "\n=== FPSCompanion: Damage ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    addComp<components::FPSCompanion>(e);

    sys.startFollowing("vip_1", "player_1");
    assertTrue(sys.applyDamage("vip_1", 30.0f), "Damage applied");
    assertTrue(approxEqual(sys.getHealth("vip_1"), 70.0f), "Health is 70");
    assertTrue(!sys.applyDamage("vip_1", -10.0f), "Negative rejected");
}

static void testCompanionDeath() {
    std::cout << "\n=== FPSCompanion: Death ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    addComp<components::FPSCompanion>(e);

    sys.applyDamage("vip_1", 150.0f);
    assertTrue(sys.isDead("vip_1"), "VIP is dead");
    assertTrue(approxEqual(sys.getHealth("vip_1"), 0.0f), "Health clamped to 0");
    assertTrue(!sys.startFollowing("vip_1", "player_1"), "Can't follow when dead");
    assertTrue(!sys.heal("vip_1", 50.0f), "Can't heal when dead");
    assertTrue(!sys.rescue("vip_1"), "Can't rescue when dead");
}

static void testCompanionHeal() {
    std::cout << "\n=== FPSCompanion: Heal ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    auto* comp = addComp<components::FPSCompanion>(e);
    comp->panic_threshold = 30.0f;

    sys.startFollowing("vip_1", "player_1");
    sys.applyDamage("vip_1", 80.0f);  // Health → 20 (below 30% panic)
    assertTrue(sys.getState("vip_1") == 3, "State: Injured");

    assertTrue(sys.heal("vip_1", 50.0f), "Heal applied");
    assertTrue(approxEqual(sys.getHealth("vip_1"), 70.0f), "Health is 70");
    assertTrue(sys.getState("vip_1") == 1, "Back to Following after heal");
}

static void testCompanionHealCap() {
    std::cout << "\n=== FPSCompanion: Heal Cap ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    addComp<components::FPSCompanion>(e);

    sys.applyDamage("vip_1", 10.0f);
    sys.heal("vip_1", 500.0f);  // Overheal
    assertTrue(approxEqual(sys.getHealth("vip_1"), 100.0f), "Health capped at max");
}

static void testCompanionRescue() {
    std::cout << "\n=== FPSCompanion: Rescue ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    addComp<components::FPSCompanion>(e);

    sys.startFollowing("vip_1", "player_1");
    assertTrue(sys.rescue("vip_1"), "Rescue succeeds");
    assertTrue(sys.isRescued("vip_1"), "VIP is rescued");
    assertTrue(!sys.startFollowing("vip_1", "player_1"), "Can't follow after rescued");
}

static void testCompanionMorale() {
    std::cout << "\n=== FPSCompanion: Morale ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    auto* comp = addComp<components::FPSCompanion>(e);
    comp->morale_decay_rate = 10.0f;
    comp->morale_recovery_rate = 5.0f;

    sys.startFollowing("vip_1", "player_1");
    assertTrue(approxEqual(sys.getMorale("vip_1"), 100.0f), "Full morale");

    sys.applyDamage("vip_1", 5.0f);  // Small damage, morale drops
    assertTrue(sys.getMorale("vip_1") < 100.0f, "Morale dropped after damage");

    // Morale recovers over time while following
    sys.update(5.0f);
    assertTrue(sys.getMorale("vip_1") > 90.0f, "Morale recovering");
}

static void testCompanionHide() {
    std::cout << "\n=== FPSCompanion: Command Hide ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    addComp<components::FPSCompanion>(e);

    sys.startFollowing("vip_1", "player_1");
    assertTrue(sys.commandHide("vip_1"), "Command hide succeeds");
    assertTrue(sys.getState("vip_1") == 2, "State: Hiding");
}

static void testCompanionAutoHide() {
    std::cout << "\n=== FPSCompanion: Auto-Hide on Low Health ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    auto* e = world.createEntity("vip_1");
    auto* comp = addComp<components::FPSCompanion>(e);
    comp->panic_threshold = 50.0f;  // 50% = hide at 50hp

    sys.startFollowing("vip_1", "player_1");
    comp->health = 40.0f;  // Below threshold
    sys.update(0.1f);
    assertTrue(sys.getState("vip_1") == 2, "Auto-hide when health low");
}

static void testCompanionMissingEntity() {
    std::cout << "\n=== FPSCompanion: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FPSCompanionSystem sys(&world);

    assertTrue(!sys.startFollowing("nope", "player_1"), "Follow fails");
    assertTrue(!sys.applyDamage("nope", 10.0f), "Damage fails");
    assertTrue(!sys.heal("nope", 10.0f), "Heal fails");
    assertTrue(!sys.rescue("nope"), "Rescue fails");
    assertTrue(sys.getState("nope") == 0, "State is 0");
    assertTrue(approxEqual(sys.getHealth("nope"), 0.0f), "Health is 0");
    assertTrue(approxEqual(sys.getMorale("nope"), 0.0f), "Morale is 0");
}

void run_fps_companion_system_tests() {
    testCompanionFollow();
    testCompanionStopFollow();
    testCompanionDamage();
    testCompanionDeath();
    testCompanionHeal();
    testCompanionHealCap();
    testCompanionRescue();
    testCompanionMorale();
    testCompanionHide();
    testCompanionAutoHide();
    testCompanionMissingEntity();
}
