// Tests for: ServerTickMetricsSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/server_tick_metrics_system.h"

using namespace atlas;

// ==================== ServerTickMetricsSystem Tests ====================

static void testServerTickMetricsCreate() {
    std::cout << "\n=== ServerTickMetrics: Create ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    assertTrue(sys.initialize("srv1", 20.0f), "Init at 20 Hz succeeds");
    assertTrue(approxEqual(sys.getTickBudgetMs("srv1"), 50.0f), "Budget 50ms");
    assertTrue(sys.getTotalTicks("srv1") == 0, "Zero ticks initially");
    assertTrue(sys.getOvertimeTicks("srv1") == 0, "Zero overtime initially");
    assertTrue(sys.getEntityCount("srv1") == 0, "Zero entities initially");
    assertTrue(approxEqual(sys.getLastTickMs("srv1"), 0.0f), "Zero last tick");
}

static void testServerTickMetricsInvalidInit() {
    std::cout << "\n=== ServerTickMetrics: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    assertTrue(!sys.initialize("missing", 20.0f), "Missing entity fails");
    world.createEntity("srv1");
    assertTrue(!sys.initialize("srv1", 0.0f), "Zero tick rate fails");
    assertTrue(!sys.initialize("srv1", -10.0f), "Negative tick rate fails");
}

static void testServerTickMetricsRecordTick() {
    std::cout << "\n=== ServerTickMetrics: RecordTick ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", 20.0f); // 50ms budget

    assertTrue(sys.recordTick("srv1", 30.0f), "Record 30ms tick");
    assertTrue(approxEqual(sys.getLastTickMs("srv1"), 30.0f), "Last tick 30ms");
    assertTrue(sys.getTotalTicks("srv1") == 1, "1 tick recorded");
    assertTrue(sys.getOvertimeTicks("srv1") == 0, "No overtime yet");

    assertTrue(sys.recordTick("srv1", 60.0f), "Record 60ms tick (over budget)");
    assertTrue(approxEqual(sys.getLastTickMs("srv1"), 60.0f), "Last tick 60ms");
    assertTrue(sys.getTotalTicks("srv1") == 2, "2 ticks recorded");
    assertTrue(sys.getOvertimeTicks("srv1") == 1, "1 overtime tick");
}

static void testServerTickMetricsMinMaxAvg() {
    std::cout << "\n=== ServerTickMetrics: MinMaxAvg ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", 20.0f);

    sys.recordTick("srv1", 10.0f);
    sys.recordTick("srv1", 30.0f);
    sys.recordTick("srv1", 20.0f);

    assertTrue(approxEqual(sys.getMinTickMs("srv1"), 10.0f), "Min 10ms");
    assertTrue(approxEqual(sys.getMaxTickMs("srv1"), 30.0f), "Max 30ms");
    assertTrue(approxEqual(sys.getAvgTickMs("srv1"), 20.0f), "Avg 20ms");
}

static void testServerTickMetricsOvertimeRatio() {
    std::cout << "\n=== ServerTickMetrics: OvertimeRatio ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", 20.0f); // 50ms budget

    sys.recordTick("srv1", 40.0f);  // under
    sys.recordTick("srv1", 55.0f);  // over
    sys.recordTick("srv1", 45.0f);  // under
    sys.recordTick("srv1", 70.0f);  // over

    float ratio = sys.getOvertimeRatio("srv1");
    assertTrue(ratio > 0.49f && ratio < 0.51f, "50% overtime ratio");
}

static void testServerTickMetricsEntityCount() {
    std::cout << "\n=== ServerTickMetrics: EntityCount ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", 20.0f);

    assertTrue(sys.setEntityCount("srv1", 100), "Set 100 entities");
    assertTrue(sys.getEntityCount("srv1") == 100, "100 entities");
    assertTrue(sys.getPeakEntityCount("srv1") == 100, "Peak 100");

    assertTrue(sys.setEntityCount("srv1", 200), "Set 200 entities");
    assertTrue(sys.getPeakEntityCount("srv1") == 200, "Peak 200");

    assertTrue(sys.setEntityCount("srv1", 50), "Set 50 entities");
    assertTrue(sys.getPeakEntityCount("srv1") == 200, "Peak stays 200");

    assertTrue(!sys.setEntityCount("srv1", -1), "Negative count rejected");
}

static void testServerTickMetricsPhases() {
    std::cout << "\n=== ServerTickMetrics: Phases ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", 20.0f);

    assertTrue(sys.recordPhase("srv1", "physics", 5.0f), "Record physics phase");
    assertTrue(sys.recordPhase("srv1", "ai", 8.0f), "Record ai phase");
    assertTrue(sys.recordPhase("srv1", "network", 3.0f), "Record network phase");

    assertTrue(sys.getPhaseCount("srv1") == 3, "3 phases recorded");
    assertTrue(approxEqual(sys.getPhaseTime("srv1", "physics"), 5.0f), "Physics 5ms");
    assertTrue(approxEqual(sys.getPhaseTime("srv1", "ai"), 8.0f), "AI 8ms");
    assertTrue(approxEqual(sys.getPhaseTime("srv1", "network"), 3.0f), "Network 3ms");

    // Update existing phase
    assertTrue(sys.recordPhase("srv1", "physics", 7.0f), "Update physics phase");
    assertTrue(approxEqual(sys.getPhaseTime("srv1", "physics"), 7.0f), "Physics updated to 7ms");
    assertTrue(sys.getPhaseCount("srv1") == 3, "Still 3 phases");
}

static void testServerTickMetricsPhaseInvalid() {
    std::cout << "\n=== ServerTickMetrics: PhaseInvalid ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", 20.0f);

    assertTrue(!sys.recordPhase("srv1", "", 5.0f), "Empty phase name rejected");
    assertTrue(!sys.recordPhase("srv1", "test", -1.0f), "Negative duration rejected");
    assertTrue(!sys.recordPhase("nonexistent", "test", 5.0f), "Missing entity rejected");
}

static void testServerTickMetricsReset() {
    std::cout << "\n=== ServerTickMetrics: Reset ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", 20.0f);

    sys.recordTick("srv1", 30.0f);
    sys.recordTick("srv1", 60.0f);
    sys.recordPhase("srv1", "physics", 5.0f);

    assertTrue(sys.resetStats("srv1"), "Reset succeeds");
    assertTrue(sys.getTotalTicks("srv1") == 0, "Ticks reset");
    assertTrue(sys.getOvertimeTicks("srv1") == 0, "Overtime reset");
    assertTrue(approxEqual(sys.getLastTickMs("srv1"), 0.0f), "Last tick reset");
    assertTrue(sys.getPhaseCount("srv1") == 0, "Phases reset");
}

static void testServerTickMetricsRecordTickInvalid() {
    std::cout << "\n=== ServerTickMetrics: RecordTickInvalid ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", 20.0f);

    assertTrue(!sys.recordTick("srv1", -1.0f), "Negative tick rejected");
    assertTrue(!sys.recordTick("nonexistent", 10.0f), "Missing entity rejected");
}

static void testServerTickMetricsUpdate() {
    std::cout << "\n=== ServerTickMetrics: Update ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", 20.0f);

    sys.update(1.0f);
    // Just verifies update doesn't crash
    assertTrue(true, "Update tick OK");
}

static void testServerTickMetricsMissing() {
    std::cout << "\n=== ServerTickMetrics: Missing ===" << std::endl;
    ecs::World world;
    systems::ServerTickMetricsSystem sys(&world);
    assertTrue(approxEqual(sys.getLastTickMs("x"), 0.0f), "Default last tick on missing");
    assertTrue(approxEqual(sys.getAvgTickMs("x"), 0.0f), "Default avg on missing");
    assertTrue(sys.getTotalTicks("x") == 0, "Default ticks on missing");
    assertTrue(sys.getOvertimeTicks("x") == 0, "Default overtime on missing");
    assertTrue(sys.getEntityCount("x") == 0, "Default entity count on missing");
    assertTrue(sys.getPeakEntityCount("x") == 0, "Default peak on missing");
    assertTrue(approxEqual(sys.getTickBudgetMs("x"), 0.0f), "Default budget on missing");
    assertTrue(!sys.resetStats("x"), "Reset fails on missing");
}

void run_server_tick_metrics_system_tests() {
    testServerTickMetricsCreate();
    testServerTickMetricsInvalidInit();
    testServerTickMetricsRecordTick();
    testServerTickMetricsMinMaxAvg();
    testServerTickMetricsOvertimeRatio();
    testServerTickMetricsEntityCount();
    testServerTickMetricsPhases();
    testServerTickMetricsPhaseInvalid();
    testServerTickMetricsReset();
    testServerTickMetricsRecordTickInvalid();
    testServerTickMetricsUpdate();
    testServerTickMetricsMissing();
}
