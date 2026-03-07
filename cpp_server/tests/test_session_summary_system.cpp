// Tests for: Session Summary System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/session_summary_system.h"

using namespace atlas;

// ==================== Session Summary System Tests ====================

static void testSessionSummaryCreate() {
    std::cout << "\n=== SessionSummary: Create ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_001"), "Init succeeds");
    assertTrue(sys.getStatCount("p1") == 0, "No stats");
    assertTrue(sys.getCategoryCount("p1") == 0, "No categories");
    assertTrue(!sys.isFinalized("p1"), "Not finalized");
    assertTrue(approxEqual(sys.getSessionDuration("p1"), 0.0f), "0 duration");
}

static void testSessionSummaryRecordStat() {
    std::cout << "\n=== SessionSummary: RecordStat ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.recordStat("p1", "isc_earned", 5000.0), "Record ISC");
    assertTrue(sys.recordStat("p1", "kills", 3.0), "Record kills");
    assertTrue(approxEqual(sys.getStat("p1", "isc_earned"), 5000.0), "5000 ISC");
    assertTrue(approxEqual(sys.getStat("p1", "kills"), 3.0), "3 kills");
    assertTrue(sys.getStatCount("p1") == 2, "2 stats");
    // Accumulation
    assertTrue(sys.recordStat("p1", "isc_earned", 2000.0), "Accumulate ISC");
    assertTrue(approxEqual(sys.getStat("p1", "isc_earned"), 7000.0), "7000 ISC total");
    assertTrue(sys.getStatCount("p1") == 2, "Still 2 stats");
    assertTrue(approxEqual(sys.getStat("p1", "nonexistent"), 0.0), "0 for missing");
}

static void testSessionSummaryStatMax() {
    std::cout << "\n=== SessionSummary: StatMax ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::SessionSummaryState>();
    state->max_stats = 2;
    sys.recordStat("p1", "a", 1.0);
    sys.recordStat("p1", "b", 2.0);
    assertTrue(!sys.recordStat("p1", "c", 3.0), "Max stats enforced");
    // But existing key can still accumulate
    assertTrue(sys.recordStat("p1", "a", 1.0), "Existing key still works");
    assertTrue(approxEqual(sys.getStat("p1", "a"), 2.0), "Accumulated");
}

static void testSessionSummaryCategoryStat() {
    std::cout << "\n=== SessionSummary: CategoryStat ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.addCategoryStat("p1", "Combat", "damage_dealt", 1500.0), "Add combat dmg");
    assertTrue(sys.addCategoryStat("p1", "Combat", "kills", 2.0), "Add combat kills");
    assertTrue(sys.addCategoryStat("p1", "Economy", "isc_earned", 5000.0), "Add econ ISC");
    assertTrue(sys.getCategoryCount("p1") == 2, "2 categories");
    assertTrue(approxEqual(sys.getCategoryStat("p1", "Combat", "damage_dealt"), 1500.0), "1500 dmg");
    assertTrue(approxEqual(sys.getCategoryStat("p1", "Combat", "kills"), 2.0), "2 kills");
    assertTrue(approxEqual(sys.getCategoryStat("p1", "Economy", "isc_earned"), 5000.0), "5000 ISC");
    // Accumulation within category
    assertTrue(sys.addCategoryStat("p1", "Combat", "damage_dealt", 500.0), "Accumulate dmg");
    assertTrue(approxEqual(sys.getCategoryStat("p1", "Combat", "damage_dealt"), 2000.0), "2000 dmg");
    assertTrue(approxEqual(sys.getCategoryStat("p1", "Missing", "x"), 0.0), "0 for missing cat");
}

static void testSessionSummaryCategoryMax() {
    std::cout << "\n=== SessionSummary: CategoryMax ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::SessionSummaryState>();
    state->max_categories = 2;
    sys.addCategoryStat("p1", "Combat", "a", 1.0);
    sys.addCategoryStat("p1", "Economy", "a", 1.0);
    assertTrue(!sys.addCategoryStat("p1", "Exploration", "a", 1.0), "Max categories enforced");
}

static void testSessionSummaryFinalize() {
    std::cout << "\n=== SessionSummary: Finalize ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.update(100.0f); // simulate 100s session
    assertTrue(!sys.isFinalized("p1"), "Not finalized");
    assertTrue(sys.finalizeReport("p1", 100.0f), "Finalize");
    assertTrue(sys.isFinalized("p1"), "Is finalized");
    assertTrue(approxEqual(sys.getSessionDuration("p1"), 100.0f), "100s duration");
    assertTrue(!sys.finalizeReport("p1", 150.0f), "Double finalize fails");
}

static void testSessionSummaryIscPerHour() {
    std::cout << "\n=== SessionSummary: IscPerHour ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.recordStat("p1", "isc_earned", 10000.0);
    sys.update(1800.0f); // 30 min
    sys.finalizeReport("p1", 1800.0f);
    double isc_ph = sys.getIscPerHour("p1");
    assertTrue(approxEqual(isc_ph, 20000.0, 1.0), "20000 ISC/h");
}

static void testSessionSummaryGrade() {
    std::cout << "\n=== SessionSummary: Grade ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    // Grade F (no stats)
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.getPerformanceGrade("p1") == "F", "F grade with no stats");
    // Grade D (kills=2 → 20 points)
    sys.recordStat("p1", "kills", 2.0);
    assertTrue(sys.getPerformanceGrade("p1") == "D", "D grade");
    // Grade C (objectives=1 → +20 = 40)
    sys.recordStat("p1", "objectives_completed", 1.0);
    assertTrue(sys.getPerformanceGrade("p1") == "C", "C grade");
    // Grade B (objectives=2 → +20 = 60)
    sys.recordStat("p1", "objectives_completed", 1.0);
    assertTrue(sys.getPerformanceGrade("p1") == "B", "B grade");
    // Grade S (isc_earned=1000000 → +100 = 200)
    sys.recordStat("p1", "isc_earned", 1000000.0);
    assertTrue(sys.getPerformanceGrade("p1") == "S", "S grade");
}

static void testSessionSummaryDerivedMetrics() {
    std::cout << "\n=== SessionSummary: DerivedMetrics ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.recordStat("p1", "damage_dealt", 25000.0);
    sys.recordStat("p1", "damage_received", 8000.0);
    sys.recordStat("p1", "objectives_completed", 5.0);
    assertTrue(approxEqual(sys.getTotalDamageDealt("p1"), 25000.0), "25000 damage dealt");
    assertTrue(approxEqual(sys.getTotalDamageReceived("p1"), 8000.0), "8000 damage received");
    assertTrue(sys.getObjectivesCompleted("p1") == 5, "5 objectives completed");
}

static void testSessionSummaryUpdate() {
    std::cout << "\n=== SessionSummary: Update ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::SessionSummaryState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testSessionSummaryMissing() {
    std::cout << "\n=== SessionSummary: Missing ===" << std::endl;
    ecs::World world;
    systems::SessionSummarySystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails");
    assertTrue(!sys.recordStat("nonexistent", "a", 1.0), "recordStat fails");
    assertTrue(approxEqual(sys.getStat("nonexistent", "a"), 0.0), "0 stat");
    assertTrue(sys.getStatCount("nonexistent") == 0, "0 stats");
    assertTrue(!sys.addCategoryStat("nonexistent", "C", "a", 1.0), "addCatStat fails");
    assertTrue(approxEqual(sys.getCategoryStat("nonexistent", "C", "a"), 0.0), "0 catStat");
    assertTrue(sys.getCategoryCount("nonexistent") == 0, "0 categories");
    assertTrue(!sys.finalizeReport("nonexistent", 0), "finalize fails");
    assertTrue(!sys.isFinalized("nonexistent"), "not finalized");
    assertTrue(approxEqual(sys.getSessionDuration("nonexistent"), 0.0f), "0 duration");
    assertTrue(approxEqual(sys.getIscPerHour("nonexistent"), 0.0), "0 isc/h");
    assertTrue(sys.getPerformanceGrade("nonexistent") == "F", "F grade");
    assertTrue(approxEqual(sys.getTotalDamageDealt("nonexistent"), 0.0), "0 dmg dealt");
    assertTrue(approxEqual(sys.getTotalDamageReceived("nonexistent"), 0.0), "0 dmg received");
    assertTrue(sys.getObjectivesCompleted("nonexistent") == 0, "0 objectives");
}

void run_session_summary_system_tests() {
    testSessionSummaryCreate();
    testSessionSummaryRecordStat();
    testSessionSummaryStatMax();
    testSessionSummaryCategoryStat();
    testSessionSummaryCategoryMax();
    testSessionSummaryFinalize();
    testSessionSummaryIscPerHour();
    testSessionSummaryGrade();
    testSessionSummaryDerivedMetrics();
    testSessionSummaryUpdate();
    testSessionSummaryMissing();
}
