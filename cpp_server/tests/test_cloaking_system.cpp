// Tests for: CloakingSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/cloaking_system.h"

using namespace atlas;

// ==================== CloakingSystem Tests ====================

static void testCloakingActivate() {
    std::cout << "\n=== Cloaking: Activate ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    addComp<components::CloakingState>(e);

    systems::CloakingSystem sys(&world);
    assertTrue(sys.getPhase("ship1") == "Inactive", "Initial phase is Inactive");
    assertTrue(sys.activateCloak("ship1"), "Cloak activated");
    assertTrue(sys.getPhase("ship1") == "Activating", "Phase is Activating");
    assertTrue(!sys.activateCloak("ship1"), "Cannot activate while activating");
}

static void testCloakingFullCycle() {
    std::cout << "\n=== Cloaking: Full Cycle ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 2.0f;
    cloak->deactivation_time = 1.0f;
    auto* cap = addComp<components::Capacitor>(e);
    cap->capacitor = 100.0f;
    cap->capacitor_max = 100.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(2.0f);  // activation_time reached
    assertTrue(sys.isCloaked("ship1"), "Ship is cloaked after activation_time");
    assertTrue(sys.deactivateCloak("ship1"), "Decloak initiated");
    assertTrue(sys.getPhase("ship1") == "Deactivating", "Phase is Deactivating");
    sys.update(1.0f);  // deactivation_time reached
    assertTrue(sys.getPhase("ship1") == "Inactive", "Phase returns to Inactive");
}

static void testCloakingCapacitorDrain() {
    std::cout << "\n=== Cloaking: Capacitor Drain ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 1.0f;
    cloak->fuel_per_second = 10.0f;
    auto* cap = addComp<components::Capacitor>(e);
    cap->capacitor = 25.0f;
    cap->capacitor_max = 100.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(1.0f);  // become cloaked
    assertTrue(sys.isCloaked("ship1"), "Ship cloaked");
    sys.update(2.0f);  // drain 20 cap (25 - 20 = 5)
    assertTrue(sys.isCloaked("ship1"), "Still cloaked with remaining cap");
    assertTrue(cap->capacitor < 10.0f, "Capacitor drained");
}

static void testCloakingCapacitorEmpty() {
    std::cout << "\n=== Cloaking: Capacitor Empty Force Decloak ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 1.0f;
    cloak->fuel_per_second = 50.0f;
    auto* cap = addComp<components::Capacitor>(e);
    cap->capacitor = 10.0f;
    cap->capacitor_max = 100.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(1.0f);  // become cloaked
    assertTrue(sys.isCloaked("ship1"), "Ship cloaked");
    sys.update(1.0f);  // drain 50 cap but only 10 available → force decloak
    assertTrue(!sys.isCloaked("ship1"), "Force decloaked by cap empty");
    assertTrue(sys.getPhase("ship1") == "Deactivating", "Phase is Deactivating");
    assertTrue(approxEqual(cap->capacitor, 0.0f), "Capacitor at zero");
}

static void testCloakingProximityDecloak() {
    std::cout << "\n=== Cloaking: Proximity Decloak ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 1.0f;
    cloak->proximity_decloak_range = 2000.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(1.0f);
    assertTrue(sys.isCloaked("ship1"), "Ship cloaked");
    assertTrue(!sys.proximityDecloak("ship1", 3000.0f), "No decloak at 3000m");
    assertTrue(sys.proximityDecloak("ship1", 1500.0f), "Decloak at 1500m");
    assertTrue(sys.getPhase("ship1") == "Deactivating", "Phase is Deactivating");
}

static void testCloakingTargetingLockout() {
    std::cout << "\n=== Cloaking: Targeting Lockout ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 1.0f;
    cloak->targeting_delay = 5.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(1.0f);  // cloaked
    sys.deactivateCloak("ship1");
    assertTrue(sys.isTargetingLocked("ship1"), "Targeting locked after decloak");
    assertTrue(approxEqual(sys.getTargetingLockoutRemaining("ship1"), 5.0f), "Lockout is 5s");
    sys.update(3.0f);  // 3s pass
    assertTrue(sys.isTargetingLocked("ship1"), "Still locked after 3s");
    sys.update(3.0f);  // 3 more seconds
    assertTrue(!sys.isTargetingLocked("ship1"), "Lockout expired");
}

static void testCloakingCovertOps() {
    std::cout << "\n=== Cloaking: Covert Ops ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->cloak_type = components::CloakingState::CloakType::CovertOps;
    cloak->can_warp_while_cloaked = true;

    systems::CloakingSystem sys(&world);
    assertTrue(sys.canWarpCloaked("ship1"), "CovertOps can warp cloaked");
}

static void testCloakingDecloakCount() {
    std::cout << "\n=== Cloaking: Decloak Count ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 0.1f;
    cloak->deactivation_time = 0.1f;

    systems::CloakingSystem sys(&world);
    assertTrue(sys.getDecloakCount("ship1") == 0, "Initial decloak count 0");
    sys.activateCloak("ship1");
    sys.update(0.1f);  // cloaked
    sys.deactivateCloak("ship1");
    assertTrue(sys.getDecloakCount("ship1") == 1, "Decloak count 1");
    sys.update(0.1f);  // inactive
    sys.activateCloak("ship1");
    sys.update(0.1f);  // cloaked
    sys.deactivateCloak("ship1");
    assertTrue(sys.getDecloakCount("ship1") == 2, "Decloak count 2");
}

static void testCloakingSetProximityRange() {
    std::cout << "\n=== Cloaking: Set Proximity Range ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    addComp<components::CloakingState>(e);

    systems::CloakingSystem sys(&world);
    assertTrue(sys.setProximityRange("ship1", 5000.0f), "Set range success");
    assertTrue(!sys.setProximityRange("nonexistent", 5000.0f), "Set range on missing fails");
}

static void testCloakingMissing() {
    std::cout << "\n=== Cloaking: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CloakingSystem sys(&world);
    assertTrue(sys.getPhase("nonexistent") == "unknown", "Default phase for missing");
    assertTrue(!sys.isCloaked("nonexistent"), "Default not cloaked for missing");
    assertTrue(!sys.isTargetingLocked("nonexistent"), "Default no lockout for missing");
    assertTrue(approxEqual(sys.getTargetingLockoutRemaining("nonexistent"), 0.0f), "Default lockout 0");
    assertTrue(approxEqual(sys.getFuelRate("nonexistent"), 0.0f), "Default fuel rate 0");
    assertTrue(!sys.canWarpCloaked("nonexistent"), "Default no warp for missing");
    assertTrue(sys.getDecloakCount("nonexistent") == 0, "Default decloak count 0");
}


void run_cloaking_system_tests() {
    testCloakingActivate();
    testCloakingFullCycle();
    testCloakingCapacitorDrain();
    testCloakingCapacitorEmpty();
    testCloakingProximityDecloak();
    testCloakingTargetingLockout();
    testCloakingCovertOps();
    testCloakingDecloakCount();
    testCloakingSetProximityRange();
    testCloakingMissing();
}
