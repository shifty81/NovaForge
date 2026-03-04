// Tests for: ImperfectInformationSystem Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/imperfect_information_system.h"

using namespace atlas;

// ==================== ImperfectInformationSystem Tests ====================

static void testIntelRecord() {
    std::cout << "\n=== Imperfect Information: Record Intel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    assertTrue(sys.recordIntel("observer1", "target1", 0.8f, 100.0f), "Intel recorded");
    assertTrue(sys.hasIntel("observer1", "target1"), "Has intel on target1");
    assertTrue(sys.getConfidence("observer1", "target1") > 0.0f, "Confidence > 0");
    assertTrue(sys.getIntelCount("observer1") == 1, "One intel entry");
    assertTrue(sys.getTotalScans("observer1") == 1, "One scan recorded");
}

static void testIntelConfidenceDistance() {
    std::cout << "\n=== Imperfect Information: Confidence by Distance ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    sys.recordIntel("observer1", "close_target", 0.8f, 10.0f);
    sys.recordIntel("observer1", "far_target", 0.8f, 5000.0f);
    float close_conf = sys.getConfidence("observer1", "close_target");
    float far_conf = sys.getConfidence("observer1", "far_target");
    assertTrue(close_conf > far_conf, "Close target has higher confidence");
}

static void testIntelDecay() {
    std::cout << "\n=== Imperfect Information: Intel Decay ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    auto* intel = addComp<components::EntityIntel>(e);
    intel->entries.push_back({"target1", 0.5f, 0.8f, 0.0f, 0.1f, 100.0f, false});

    systems::ImperfectInformationSystem sys(&world);
    float initial = sys.getConfidence("observer1", "target1");
    sys.update(2.0f);  // 2 seconds of decay at 0.1/s = 0.2 lost
    float after = sys.getConfidence("observer1", "target1");
    assertTrue(after < initial, "Confidence decayed over time");
    assertTrue(sys.getIntelAge("observer1", "target1") > 0.0f, "Intel age increased");
}

static void testIntelGhost() {
    std::cout << "\n=== Imperfect Information: Sensor Ghost ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    auto* intel = addComp<components::EntityIntel>(e);
    intel->ghost_threshold = 0.1f;
    intel->entries.push_back({"target1", 0.15f, 0.8f, 0.0f, 0.1f, 100.0f, false});

    systems::ImperfectInformationSystem sys(&world);
    assertTrue(!sys.isGhost("observer1", "target1"), "Not ghost initially");
    sys.update(1.0f);  // 0.15 - 0.1 = 0.05, below 0.1 threshold
    assertTrue(sys.isGhost("observer1", "target1"), "Became ghost after decay");
}

static void testIntelRefresh() {
    std::cout << "\n=== Imperfect Information: Refresh Intel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    auto* intel = addComp<components::EntityIntel>(e);
    intel->entries.push_back({"target1", 0.3f, 0.5f, 10.0f, 0.01f, 200.0f, true});

    systems::ImperfectInformationSystem sys(&world);
    assertTrue(sys.isGhost("observer1", "target1"), "Was ghost");
    sys.recordIntel("observer1", "target1", 0.9f, 50.0f);
    assertTrue(!sys.isGhost("observer1", "target1"), "No longer ghost after refresh");
    assertTrue(sys.getConfidence("observer1", "target1") > 0.3f, "Confidence improved");
}

static void testIntelClear() {
    std::cout << "\n=== Imperfect Information: Clear Intel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    sys.recordIntel("observer1", "target1", 0.8f, 100.0f);
    sys.recordIntel("observer1", "target2", 0.6f, 200.0f);
    assertTrue(sys.getIntelCount("observer1") == 2, "Two entries");
    assertTrue(sys.clearIntel("observer1", "target1"), "Cleared target1");
    assertTrue(!sys.hasIntel("observer1", "target1"), "target1 removed");
    assertTrue(sys.hasIntel("observer1", "target2"), "target2 still present");
}

static void testIntelClearAll() {
    std::cout << "\n=== Imperfect Information: Clear All ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    sys.recordIntel("observer1", "target1", 0.8f, 100.0f);
    sys.recordIntel("observer1", "target2", 0.6f, 200.0f);
    assertTrue(sys.clearAllIntel("observer1"), "Cleared all");
    assertTrue(sys.getIntelCount("observer1") == 0, "No entries after clear all");
}

static void testIntelSensorStrength() {
    std::cout << "\n=== Imperfect Information: Sensor Strength ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    assertTrue(approxEqual(sys.getSensorStrength("observer1"), 1.0f), "Default sensor strength 1.0");
    sys.setSensorStrength("observer1", 2.0f);
    assertTrue(approxEqual(sys.getSensorStrength("observer1"), 2.0f), "Sensor strength set to 2.0");

    sys.recordIntel("observer1", "target1", 0.5f, 10.0f);
    float strong_conf = sys.getConfidence("observer1", "target1");
    assertTrue(strong_conf > 0.5f, "Higher sensor strength gives better confidence");
}

static void testIntelZeroDecayRemoval() {
    std::cout << "\n=== Imperfect Information: Zero Decay Removal ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    auto* intel = addComp<components::EntityIntel>(e);
    intel->entries.push_back({"target1", 0.05f, 0.5f, 0.0f, 0.1f, 100.0f, false});

    systems::ImperfectInformationSystem sys(&world);
    sys.update(1.0f);  // 0.05 - 0.1 = 0, removed
    assertTrue(!sys.hasIntel("observer1", "target1"), "Removed entry at zero confidence");
}

static void testIntelMissing() {
    std::cout << "\n=== Imperfect Information: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::ImperfectInformationSystem sys(&world);
    assertTrue(approxEqual(sys.getConfidence("nonexistent", "t"), 0.0f), "Default confidence 0");
    assertTrue(!sys.hasIntel("nonexistent", "t"), "No intel on missing");
    assertTrue(approxEqual(sys.getIntelAge("nonexistent", "t"), 0.0f), "Default age 0");
    assertTrue(!sys.isGhost("nonexistent", "t"), "Not ghost for missing");
    assertTrue(sys.getIntelCount("nonexistent") == 0, "Default count 0");
    assertTrue(sys.getTotalScans("nonexistent") == 0, "Default scans 0");
    assertTrue(approxEqual(sys.getSensorStrength("nonexistent"), 0.0f), "Default sensor 0");
}


void run_imperfect_information_system_tests() {
    testIntelRecord();
    testIntelConfidenceDistance();
    testIntelDecay();
    testIntelGhost();
    testIntelRefresh();
    testIntelClear();
    testIntelClearAll();
    testIntelSensorStrength();
    testIntelZeroDecayRemoval();
    testIntelMissing();
}
