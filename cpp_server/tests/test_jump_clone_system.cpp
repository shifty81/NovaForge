// Tests for: JumpCloneSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/jump_clone_system.h"

using namespace atlas;

// ==================== JumpCloneSystem Tests ====================

static void testJumpCloneInit() {
    std::cout << "\n=== JumpClone: Init ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getCloneCount("e1") == 0, "Zero clones initially");
    assertTrue(!sys.isOnCooldown("e1"), "No cooldown initially");
    assertTrue(approxEqual(sys.getCooldownRemaining("e1"), 0.0f), "0 cooldown");
    assertTrue(sys.getActiveCloneId("e1") == "", "No active clone");
    assertTrue(sys.getActiveStationId("e1") == "", "No active station");
    assertTrue(sys.getTotalJumps("e1") == 0, "Zero jumps");
    assertTrue(sys.getTotalClonesInstalled("e1") == 0, "Zero installed");
    assertTrue(sys.getTotalClonesDestroyed("e1") == 0, "Zero destroyed");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testJumpCloneInstall() {
    std::cout << "\n=== JumpClone: Install ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.installClone("e1", "clone1", "station_a", "Station Alpha"),
               "Install clone1");
    assertTrue(sys.getCloneCount("e1") == 1, "1 clone");
    assertTrue(sys.hasClone("e1", "clone1"), "Has clone1");
    assertTrue(sys.getCloneStation("e1", "clone1") == "station_a", "Station matches");
    assertTrue(sys.getCloneStationName("e1", "clone1") == "Station Alpha", "Name matches");
    assertTrue(sys.getTotalClonesInstalled("e1") == 1, "1 installed");

    assertTrue(sys.installClone("e1", "clone2", "station_b", "Station Beta"),
               "Install clone2");
    assertTrue(sys.getCloneCount("e1") == 2, "2 clones");
}

static void testJumpCloneInstallValidation() {
    std::cout << "\n=== JumpClone: InstallValidation ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.installClone("e1", "", "s1", "S1"), "Empty clone_id rejected");
    assertTrue(!sys.installClone("e1", "c1", "", "S1"), "Empty station_id rejected");
    assertTrue(!sys.installClone("e1", "c1", "s1", ""), "Empty station_name rejected");

    assertTrue(sys.installClone("e1", "c1", "s1", "S1"), "Valid install");
    assertTrue(!sys.installClone("e1", "c1", "s2", "S2"), "Duplicate clone_id rejected");
    assertTrue(!sys.installClone("missing", "c9", "s1", "S1"),
               "Missing entity rejected");
}

static void testJumpCloneCapacity() {
    std::cout << "\n=== JumpClone: Capacity ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxClones("e1", 3);

    assertTrue(sys.installClone("e1", "c1", "s1", "S1"), "Install 1");
    assertTrue(sys.installClone("e1", "c2", "s2", "S2"), "Install 2");
    assertTrue(sys.installClone("e1", "c3", "s3", "S3"), "Install 3 at cap");
    assertTrue(!sys.installClone("e1", "c4", "s4", "S4"),
               "Install 4 rejected at capacity");
    assertTrue(sys.getCloneCount("e1") == 3, "Still 3");
}

static void testJumpCloneDestroy() {
    std::cout << "\n=== JumpClone: Destroy ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.installClone("e1", "c1", "s1", "S1");
    sys.installClone("e1", "c2", "s2", "S2");

    assertTrue(sys.destroyClone("e1", "c1"), "Destroy c1");
    assertTrue(sys.getCloneCount("e1") == 1, "1 left");
    assertTrue(!sys.hasClone("e1", "c1"), "c1 gone");
    assertTrue(sys.hasClone("e1", "c2"), "c2 present");
    assertTrue(sys.getTotalClonesDestroyed("e1") == 1, "1 destroyed");
    assertTrue(!sys.destroyClone("e1", "c1"), "Destroy already destroyed fails");
    assertTrue(!sys.destroyClone("e1", "unknown"), "Destroy unknown fails");
}

static void testJumpCloneDestroyActiveBlocked() {
    std::cout << "\n=== JumpClone: DestroyActiveBlocked ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setCooldownDuration("e1", 10.0f); // short cooldown for testing

    sys.installClone("e1", "c1", "s1", "S1");
    sys.installClone("e1", "c2", "s2", "S2");
    sys.jumpToClone("e1", "c1");

    assertTrue(!sys.destroyClone("e1", "c1"), "Cannot destroy active clone");
    assertTrue(sys.destroyClone("e1", "c2"), "Can destroy inactive clone");
}

static void testJumpCloneJump() {
    std::cout << "\n=== JumpClone: Jump ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setCooldownDuration("e1", 100.0f);

    sys.installClone("e1", "c1", "s1", "S1");
    sys.installClone("e1", "c2", "s2", "S2");

    assertTrue(sys.jumpToClone("e1", "c1"), "Jump to c1");
    assertTrue(sys.getActiveCloneId("e1") == "c1", "Active is c1");
    assertTrue(sys.getActiveStationId("e1") == "s1", "Station is s1");
    assertTrue(sys.isOnCooldown("e1"), "On cooldown after jump");
    assertTrue(approxEqual(sys.getCooldownRemaining("e1"), 100.0f), "100s cooldown");
    assertTrue(sys.getTotalJumps("e1") == 1, "1 jump");

    // Cannot jump while on cooldown
    assertTrue(!sys.jumpToClone("e1", "c2"), "Cannot jump on cooldown");
}

static void testJumpCloneCooldownDecay() {
    std::cout << "\n=== JumpClone: CooldownDecay ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setCooldownDuration("e1", 50.0f);

    sys.installClone("e1", "c1", "s1", "S1");
    sys.installClone("e1", "c2", "s2", "S2");

    sys.jumpToClone("e1", "c1");
    assertTrue(sys.isOnCooldown("e1"), "On cooldown");

    sys.update(25.0f);
    assertTrue(approxEqual(sys.getCooldownRemaining("e1"), 25.0f), "25s remaining");
    assertTrue(sys.isOnCooldown("e1"), "Still on cooldown");

    sys.update(30.0f);
    assertTrue(!sys.isOnCooldown("e1"), "Cooldown expired");
    assertTrue(approxEqual(sys.getCooldownRemaining("e1"), 0.0f), "0 remaining");

    // Can jump again
    assertTrue(sys.jumpToClone("e1", "c2"), "Jump to c2 after cooldown");
    assertTrue(sys.getActiveCloneId("e1") == "c2", "Active is c2");
    assertTrue(sys.getTotalJumps("e1") == 2, "2 jumps total");
}

static void testJumpCloneJumpValidation() {
    std::cout << "\n=== JumpClone: JumpValidation ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setCooldownDuration("e1", 10.0f);

    sys.installClone("e1", "c1", "s1", "S1");

    assertTrue(!sys.jumpToClone("e1", ""), "Empty clone_id rejected");
    assertTrue(!sys.jumpToClone("e1", "nonexistent"), "Nonexistent clone rejected");

    sys.jumpToClone("e1", "c1");
    sys.update(20.0f); // clear cooldown
    assertTrue(!sys.jumpToClone("e1", "c1"), "Cannot jump to same active clone");
}

static void testJumpCloneImplants() {
    std::cout << "\n=== JumpClone: Implants ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.installClone("e1", "c1", "s1", "S1");

    assertTrue(sys.addImplant("e1", "c1", "imp1", "Memory Augmentation", 1),
               "Add implant slot 1");
    assertTrue(sys.getImplantCount("e1", "c1") == 1, "1 implant");
    assertTrue(sys.hasImplant("e1", "c1", "imp1"), "Has imp1");

    assertTrue(sys.addImplant("e1", "c1", "imp2", "Neural Boost", 3),
               "Add implant slot 3");
    assertTrue(sys.getImplantCount("e1", "c1") == 2, "2 implants");

    // Duplicate implant ID
    assertTrue(!sys.addImplant("e1", "c1", "imp1", "Other", 5),
               "Duplicate implant_id rejected");

    // Slot already occupied
    assertTrue(!sys.addImplant("e1", "c1", "imp3", "Other", 1),
               "Slot 1 occupied rejected");

    // Invalid slots
    assertTrue(!sys.addImplant("e1", "c1", "imp4", "Other", 0),
               "Slot 0 rejected");
    assertTrue(!sys.addImplant("e1", "c1", "imp4", "Other", 11),
               "Slot 11 rejected");

    // Validation
    assertTrue(!sys.addImplant("e1", "c1", "", "Name", 5),
               "Empty implant_id rejected");
    assertTrue(!sys.addImplant("e1", "c1", "imp5", "", 5),
               "Empty implant_name rejected");
    assertTrue(!sys.addImplant("e1", "nonexistent", "imp5", "Name", 5),
               "Nonexistent clone rejected");
}

static void testJumpCloneRemoveImplant() {
    std::cout << "\n=== JumpClone: RemoveImplant ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.installClone("e1", "c1", "s1", "S1");
    sys.addImplant("e1", "c1", "imp1", "Memory Aug", 1);
    sys.addImplant("e1", "c1", "imp2", "Neural Boost", 3);

    assertTrue(sys.removeImplant("e1", "c1", "imp1"), "Remove imp1");
    assertTrue(sys.getImplantCount("e1", "c1") == 1, "1 implant left");
    assertTrue(!sys.hasImplant("e1", "c1", "imp1"), "imp1 gone");
    assertTrue(sys.hasImplant("e1", "c1", "imp2"), "imp2 present");
    assertTrue(!sys.removeImplant("e1", "c1", "imp1"), "Remove already removed");
    assertTrue(!sys.removeImplant("e1", "c1", "unknown"), "Remove unknown");
    assertTrue(!sys.removeImplant("e1", "nonexistent", "imp2"), "Remove from unknown clone");
}

static void testJumpCloneClearClones() {
    std::cout << "\n=== JumpClone: ClearClones ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.installClone("e1", "c1", "s1", "S1");
    sys.installClone("e1", "c2", "s2", "S2");
    sys.jumpToClone("e1", "c1");

    assertTrue(sys.clearClones("e1"), "ClearClones succeeds");
    assertTrue(sys.getCloneCount("e1") == 0, "0 clones after clear");
    assertTrue(sys.getActiveCloneId("e1") == "", "Active clone cleared");
    assertTrue(sys.getTotalClonesInstalled("e1") == 2, "Total installed preserved");
}

static void testJumpCloneConfiguration() {
    std::cout << "\n=== JumpClone: Configuration ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setMaxClones("e1", 20), "Set max clones");
    assertTrue(!sys.setMaxClones("e1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxClones("e1", -1), "Negative max rejected");

    assertTrue(sys.setCooldownDuration("e1", 3600.0f), "Set cooldown");
    assertTrue(!sys.setCooldownDuration("e1", -1.0f), "Negative cooldown rejected");
    assertTrue(sys.setCooldownDuration("e1", 0.0f), "Zero cooldown allowed");

    assertTrue(sys.setActiveStation("e1", "s99"), "Set active station");
    assertTrue(sys.getActiveStationId("e1") == "s99", "Active station matches");
}

static void testJumpCloneMissing() {
    std::cout << "\n=== JumpClone: Missing ===" << std::endl;
    ecs::World world;
    systems::JumpCloneSystem sys(&world);

    assertTrue(!sys.installClone("none", "c1", "s1", "S1"),
               "Install fails on missing");
    assertTrue(!sys.destroyClone("none", "c1"), "Destroy fails on missing");
    assertTrue(!sys.clearClones("none"), "Clear fails on missing");
    assertTrue(!sys.addImplant("none", "c1", "i1", "N", 1),
               "AddImplant fails on missing");
    assertTrue(!sys.removeImplant("none", "c1", "i1"),
               "RemoveImplant fails on missing");
    assertTrue(!sys.jumpToClone("none", "c1"), "Jump fails on missing");
    assertTrue(!sys.setMaxClones("none", 5), "SetMax fails on missing");
    assertTrue(!sys.setCooldownDuration("none", 100), "SetCooldown fails on missing");
    assertTrue(!sys.setActiveStation("none", "s1"), "SetStation fails on missing");
    assertTrue(sys.getCloneCount("none") == 0, "0 count on missing");
    assertTrue(!sys.hasClone("none", "c1"), "No clone on missing");
    assertTrue(sys.getCloneStation("none", "c1") == "", "Empty station on missing");
    assertTrue(sys.getCloneStationName("none", "c1") == "", "Empty name on missing");
    assertTrue(sys.getImplantCount("none", "c1") == 0, "0 implants on missing");
    assertTrue(!sys.hasImplant("none", "c1", "i1"), "No implant on missing");
    assertTrue(approxEqual(sys.getCooldownRemaining("none"), 0.0f), "0 cooldown on missing");
    assertTrue(!sys.isOnCooldown("none"), "Not on cooldown on missing");
    assertTrue(sys.getActiveCloneId("none") == "", "Empty active on missing");
    assertTrue(sys.getActiveStationId("none") == "", "Empty station on missing");
    assertTrue(sys.getTotalJumps("none") == 0, "0 jumps on missing");
    assertTrue(sys.getTotalClonesInstalled("none") == 0, "0 installed on missing");
    assertTrue(sys.getTotalClonesDestroyed("none") == 0, "0 destroyed on missing");
}

void run_jump_clone_system_tests() {
    testJumpCloneInit();
    testJumpCloneInstall();
    testJumpCloneInstallValidation();
    testJumpCloneCapacity();
    testJumpCloneDestroy();
    testJumpCloneDestroyActiveBlocked();
    testJumpCloneJump();
    testJumpCloneCooldownDecay();
    testJumpCloneJumpValidation();
    testJumpCloneImplants();
    testJumpCloneRemoveImplant();
    testJumpCloneClearClones();
    testJumpCloneConfiguration();
    testJumpCloneMissing();
}
