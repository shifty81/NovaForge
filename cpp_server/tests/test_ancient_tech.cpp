// Tests for: Ancient Tech Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/ancient_tech_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Ancient Tech Tests ====================

static void testAncientTechDefaults() {
    std::cout << "\n=== Ancient Tech Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);
    assertTrue(tech->state == components::AncientTechModule::TechState::Broken, "Starts Broken");
    assertTrue(!tech->isUsable(), "Not usable when broken");
}

static void testAncientTechRepair() {
    std::cout << "\n=== Ancient Tech Repair ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("tech2");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->repair_cost = 10.0f;

    systems::AncientTechSystem sys(&world);
    assertTrue(sys.startRepair("tech2"), "Start repair succeeds");
    assertTrue(sys.getState("tech2") == components::AncientTechModule::TechState::Repairing, "State is Repairing");

    // Simulate enough time to complete (repair_cost * 0.5 = 5.0 seconds needed)
    sys.update(6.0f);
    assertTrue(sys.getState("tech2") == components::AncientTechModule::TechState::Repaired, "State is Repaired");
    assertTrue(sys.isUsable("tech2"), "Usable after repair");
}

static void testAncientTechReverseEngineer() {
    std::cout << "\n=== Ancient Tech Reverse Engineer ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("tech3");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;
    tech->blueprint_id = "bp_ancient_weapon";

    systems::AncientTechSystem sys(&world);
    std::string bp = sys.reverseEngineer("tech3");
    assertTrue(bp == "bp_ancient_weapon", "Returns correct blueprint");
    assertTrue(tech->reverse_engineered, "Marked as reverse engineered");
}


void run_ancient_tech_tests() {
    testAncientTechDefaults();
    testAncientTechRepair();
    testAncientTechReverseEngineer();
}
