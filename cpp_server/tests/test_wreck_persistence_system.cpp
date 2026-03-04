// Tests for: Wreck Persistence System Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/wreck_persistence_system.h"

using namespace atlas;

// ==================== Wreck Persistence System Tests ====================

static void testWreckPersistenceDefaults() {
    std::cout << "\n=== Wreck Persistence Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("wreck1");
    auto* wreck = addComp<components::WreckPersistence>(e);
    assertTrue(approxEqual(wreck->lifetime, 7200.0f), "Default lifetime 7200s");
    assertTrue(approxEqual(wreck->elapsed, 0.0f), "Elapsed starts at 0");
    assertTrue(!wreck->salvage_npc_assigned, "No NPC assigned");
}

static void testWreckPersistenceExpires() {
    std::cout << "\n=== Wreck Persistence Expires ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("wreck2");
    auto* wreck = addComp<components::WreckPersistence>(e);
    wreck->lifetime = 10.0f;

    systems::WreckPersistenceSystem sys(&world);
    sys.update(5.0f);
    assertTrue(!sys.isExpired("wreck2"), "Not expired at 5s");
    sys.update(6.0f);
    assertTrue(sys.isExpired("wreck2"), "Expired after lifetime");
}

static void testWreckPersistenceAssignNPC() {
    std::cout << "\n=== Wreck Persistence Assign NPC ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("wreck3");
    auto* wreck = addComp<components::WreckPersistence>(e);

    systems::WreckPersistenceSystem sys(&world);
    sys.assignSalvageNPC("wreck3", "npc_salvager");
    assertTrue(wreck->salvage_npc_assigned, "NPC assigned");
    assertTrue(wreck->assigned_npc_id == "npc_salvager", "Correct NPC ID");
}


void run_wreck_persistence_system_tests() {
    testWreckPersistenceDefaults();
    testWreckPersistenceExpires();
    testWreckPersistenceAssignNPC();
}
