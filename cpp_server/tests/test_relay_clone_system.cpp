// Tests for: Relay Clone System
#include "test_log.h"
#include "components/game_components.h"
#include "systems/relay_clone_system.h"

using namespace atlas;

// ==================== Relay Clone System Tests ====================

static void testRelayCloneCreate() {
    std::cout << "\n=== RelayClone: Create ===" << std::endl;
    ecs::World world;
    systems::RelayCloneSystem sys(&world);
    world.createEntity("char1");
    assertTrue(sys.initialize("char1", "pilot_alpha"), "Init succeeds");
    assertTrue(sys.getCloneCount("char1") == 0, "No clones initially");
    assertTrue(sys.getTotalJumps("char1") == 0, "0 jumps");
    assertTrue(!sys.isOnCooldown("char1"), "Not on cooldown");
    assertTrue(sys.getMaxClones("char1") == 1, "Default max = 1");
    assertTrue(approxEqual(sys.getCooldownRemaining("char1"), 0.0f), "0 cooldown remaining");
}

static void testRelayCloneInstall() {
    std::cout << "\n=== RelayClone: Install ===" << std::endl;
    ecs::World world;
    systems::RelayCloneSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    assertTrue(sys.installClone("char1", "clone1", "station_jita", "Jita 4-4"), "Install succeeds");
    assertTrue(sys.getCloneCount("char1") == 1, "1 clone");
    // Default max = 1, so second should fail
    assertTrue(!sys.installClone("char1", "clone2", "station_amarr", "Amarr"), "2nd install rejected (max 1)");
    assertTrue(!sys.installClone("char1", "clone1", "station_dodixie"), "Duplicate ID rejected");
}

static void testRelayCloneDestroy() {
    std::cout << "\n=== RelayClone: Destroy ===" << std::endl;
    ecs::World world;
    systems::RelayCloneSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.installClone("char1", "clone1", "station_jita");
    assertTrue(sys.destroyClone("char1", "clone1"), "Destroy succeeds");
    assertTrue(sys.getCloneCount("char1") == 0, "0 clones after destroy");
    assertTrue(!sys.destroyClone("char1", "clone1"), "Double destroy fails");
}

static void testRelayCloneJump() {
    std::cout << "\n=== RelayClone: Jump ===" << std::endl;
    ecs::World world;
    systems::RelayCloneSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.installClone("char1", "clone1", "station_jita");
    assertTrue(sys.jumpToClone("char1", "clone1"), "Jump succeeds");
    assertTrue(sys.getTotalJumps("char1") == 1, "1 jump");
    assertTrue(sys.isOnCooldown("char1"), "On cooldown after jump");
    assertTrue(sys.getCooldownRemaining("char1") > 0.0f, "Cooldown > 0");
    assertTrue(!sys.jumpToClone("char1", "clone1"), "Can't jump while on cooldown");
}

static void testRelayCloneCooldownTick() {
    std::cout << "\n=== RelayClone: CooldownTick ===" << std::endl;
    ecs::World world;
    systems::RelayCloneSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.installClone("char1", "clone1", "station_jita");
    sys.jumpToClone("char1", "clone1");

    // Default cooldown = 86400s (24h). Tick 86400s to clear it.
    sys.update(86400.0f);
    assertTrue(!sys.isOnCooldown("char1"), "Cooldown expired after 24h");
    assertTrue(approxEqual(sys.getCooldownRemaining("char1"), 0.0f), "0 cooldown remaining");
    assertTrue(sys.jumpToClone("char1", "clone1"), "Can jump again after cooldown");
    assertTrue(sys.getTotalJumps("char1") == 2, "2 total jumps");
}

static void testRelayCloneImplants() {
    std::cout << "\n=== RelayClone: Implants ===" << std::endl;
    ecs::World world;
    systems::RelayCloneSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.installClone("char1", "clone1", "station_jita");
    assertTrue(sys.addImplant("char1", "clone1", "imp_perception_3"), "Add implant succeeds");
    assertTrue(!sys.addImplant("char1", "clone1", "imp_perception_3"), "Duplicate implant rejected");
    assertTrue(sys.addImplant("char1", "clone1", "imp_willpower_2"), "Add second implant");
    assertTrue(sys.removeImplant("char1", "clone1", "imp_perception_3"), "Remove implant succeeds");
    assertTrue(!sys.removeImplant("char1", "clone1", "imp_perception_3"), "Double remove fails");
    assertTrue(!sys.addImplant("char1", "nonexistent_clone", "imp_test"), "Implant to missing clone fails");
}

static void testRelayCloneMaxClones() {
    std::cout << "\n=== RelayClone: MaxClones ===" << std::endl;
    ecs::World world;
    systems::RelayCloneSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    assertTrue(sys.setMaxClones("char1", 3), "Set max clones = 3");
    assertTrue(sys.getMaxClones("char1") == 3, "Max clones is 3");
    sys.installClone("char1", "c1", "s1");
    sys.installClone("char1", "c2", "s2");
    sys.installClone("char1", "c3", "s3");
    assertTrue(sys.getCloneCount("char1") == 3, "3 clones installed");
    assertTrue(!sys.installClone("char1", "c4", "s4"), "4th clone rejected");
}

static void testRelayCloneJumpNonexistent() {
    std::cout << "\n=== RelayClone: JumpNonexistent ===" << std::endl;
    ecs::World world;
    systems::RelayCloneSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    assertTrue(!sys.jumpToClone("char1", "nonexistent"), "Jump to nonexistent clone fails");
    assertTrue(sys.getTotalJumps("char1") == 0, "0 jumps");
}

static void testRelayCloneMissing() {
    std::cout << "\n=== RelayClone: Missing ===" << std::endl;
    ecs::World world;
    systems::RelayCloneSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.installClone("nonexistent", "c1", "s1"), "Install fails on missing");
    assertTrue(!sys.destroyClone("nonexistent", "c1"), "Destroy fails on missing");
    assertTrue(!sys.jumpToClone("nonexistent", "c1"), "Jump fails on missing");
    assertTrue(!sys.addImplant("nonexistent", "c1", "imp"), "Add implant fails on missing");
    assertTrue(!sys.removeImplant("nonexistent", "c1", "imp"), "Remove implant fails on missing");
    assertTrue(sys.getCloneCount("nonexistent") == 0, "0 clones on missing");
    assertTrue(sys.getTotalJumps("nonexistent") == 0, "0 jumps on missing");
    assertTrue(approxEqual(sys.getCooldownRemaining("nonexistent"), 0.0f), "0 cooldown on missing");
    assertTrue(!sys.isOnCooldown("nonexistent"), "Not on cooldown on missing");
    assertTrue(!sys.setMaxClones("nonexistent", 5), "SetMaxClones fails on missing");
    assertTrue(sys.getMaxClones("nonexistent") == 0, "0 max clones on missing");
}

void run_relay_clone_system_tests() {
    testRelayCloneCreate();
    testRelayCloneInstall();
    testRelayCloneDestroy();
    testRelayCloneJump();
    testRelayCloneCooldownTick();
    testRelayCloneImplants();
    testRelayCloneMaxClones();
    testRelayCloneJumpNonexistent();
    testRelayCloneMissing();
}
