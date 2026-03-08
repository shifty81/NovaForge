// Tests for: InterestManagementPrioritySystem Tests
#include "test_log.h"
#include "components/game_components.h"
#include "systems/interest_management_priority_system.h"

using namespace atlas;

// ==================== InterestManagementPrioritySystem Tests ====================

static void testIMPCreateAndDefaults() {
    std::cout << "\n=== IMP Create And Defaults ===" << std::endl;

    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);

    bool created = sys.createPriority("player1");
    assertTrue(created, "createPriority returns true");

    bool duplicate = sys.createPriority("player1");
    assertTrue(!duplicate, "createPriority returns false for duplicate");

    assertTrue(sys.getPriorityTier("player1") == 2, "Default priority tier is 2");
    assertTrue(sys.needsUpdate("player1"), "Default needs_update is true");
    assertTrue(approxEqual(sys.getBandwidthWeight("player1"), 1.0f), "Default bandwidth_weight is 1.0");
    assertTrue(sys.getTotalUpdates("player1") == 0, "Default total_updates is 0");
    assertTrue(approxEqual(sys.getEstimatedBandwidth("player1"), 100.0f), "Default estimated bandwidth is 100.0");
}

static void testIMPTierSettings() {
    std::cout << "\n=== IMP Tier Settings ===" << std::endl;

    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    sys.createPriority("player1");

    // Tier 0: interval=0.05, weight=1.0
    sys.setPriorityTier("player1", 0);
    assertTrue(sys.getPriorityTier("player1") == 0, "Tier set to 0");
    assertTrue(approxEqual(sys.getBandwidthWeight("player1"), 1.0f), "Tier 0 weight is 1.0");
    assertTrue(approxEqual(sys.getEstimatedBandwidth("player1"), 100.0f), "Tier 0 bandwidth is 100.0");

    // Tier 1: interval=0.1, weight=0.75
    sys.setPriorityTier("player1", 1);
    assertTrue(sys.getPriorityTier("player1") == 1, "Tier set to 1");
    assertTrue(approxEqual(sys.getBandwidthWeight("player1"), 0.75f), "Tier 1 weight is 0.75");
    assertTrue(approxEqual(sys.getEstimatedBandwidth("player1"), 75.0f), "Tier 1 bandwidth is 75.0");

    // Tier 2: interval=0.25, weight=0.5
    sys.setPriorityTier("player1", 2);
    assertTrue(sys.getPriorityTier("player1") == 2, "Tier set to 2");
    assertTrue(approxEqual(sys.getBandwidthWeight("player1"), 0.5f), "Tier 2 weight is 0.5");
    assertTrue(approxEqual(sys.getEstimatedBandwidth("player1"), 50.0f), "Tier 2 bandwidth is 50.0");

    // Tier 3: interval=0.5, weight=0.25
    sys.setPriorityTier("player1", 3);
    assertTrue(sys.getPriorityTier("player1") == 3, "Tier set to 3");
    assertTrue(approxEqual(sys.getBandwidthWeight("player1"), 0.25f), "Tier 3 weight is 0.25");
    assertTrue(approxEqual(sys.getEstimatedBandwidth("player1"), 25.0f), "Tier 3 bandwidth is 25.0");

    // Clamp out-of-range tier to [0,3]
    sys.setPriorityTier("player1", -1);
    assertTrue(sys.getPriorityTier("player1") == 0, "Negative tier clamped to 0");

    sys.setPriorityTier("player1", 5);
    assertTrue(sys.getPriorityTier("player1") == 3, "Tier > 3 clamped to 3");
}

static void testIMPDistanceAutoTier() {
    std::cout << "\n=== IMP Distance Auto Tier ===" << std::endl;

    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    sys.createPriority("player1");

    // < 1000 → tier 0
    sys.setDistance("player1", 500.0f);
    assertTrue(sys.getPriorityTier("player1") == 0, "Distance 500 maps to tier 0");

    // < 5000 → tier 1
    sys.setDistance("player1", 3000.0f);
    assertTrue(sys.getPriorityTier("player1") == 1, "Distance 3000 maps to tier 1");

    // < 20000 → tier 2
    sys.setDistance("player1", 10000.0f);
    assertTrue(sys.getPriorityTier("player1") == 2, "Distance 10000 maps to tier 2");

    // >= 20000 → tier 3
    sys.setDistance("player1", 25000.0f);
    assertTrue(sys.getPriorityTier("player1") == 3, "Distance 25000 maps to tier 3");

    // Boundary: exactly 1000 → tier 1
    sys.setDistance("player1", 1000.0f);
    assertTrue(sys.getPriorityTier("player1") == 1, "Distance exactly 1000 maps to tier 1");

    // Boundary: exactly 5000 → tier 2
    sys.setDistance("player1", 5000.0f);
    assertTrue(sys.getPriorityTier("player1") == 2, "Distance exactly 5000 maps to tier 2");

    // Boundary: exactly 20000 → tier 3
    sys.setDistance("player1", 20000.0f);
    assertTrue(sys.getPriorityTier("player1") == 3, "Distance exactly 20000 maps to tier 3");

    // Zero distance → tier 0
    sys.setDistance("player1", 0.0f);
    assertTrue(sys.getPriorityTier("player1") == 0, "Distance 0 maps to tier 0");
}

static void testIMPUpdateCycle() {
    std::cout << "\n=== IMP Update Cycle ===" << std::endl;

    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    sys.createPriority("player1");

    // Set tier 0: update_interval = 0.05
    sys.setPriorityTier("player1", 0);

    // After creation needs_update is true; a small update that does not exceed interval
    // First clear needs_update by acknowledging it
    assertTrue(sys.needsUpdate("player1"), "Initially needs_update is true");

    // Run update with small delta — accumulate time but don't exceed interval yet
    sys.update(0.02f);
    // time_since_update accumulated 0.02, interval is 0.05 — not triggered yet
    // (needs_update was true initially; after update it should have been reset and re-triggered)
    // The updateComponent sets needs_update=true when time_since_update >= update_interval
    // With initial needs_update=true + 0.02 dt, the system resets timer and increments total_updates
    int updates_after_first = sys.getTotalUpdates("player1");
    assertTrue(updates_after_first >= 1, "At least one update triggered from initial needs_update");

    // Now run enough time to trigger another update
    sys.update(0.06f);
    assertTrue(sys.needsUpdate("player1"), "needs_update true after sufficient time");
    assertTrue(sys.getTotalUpdates("player1") > updates_after_first, "Total updates incremented");
}

static void testIMPBandwidthEstimation() {
    std::cout << "\n=== IMP Bandwidth Estimation ===" << std::endl;

    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    sys.createPriority("player1");

    sys.setPriorityTier("player1", 0);
    assertTrue(approxEqual(sys.getEstimatedBandwidth("player1"), 1.0f * 100.0f),
               "Tier 0 estimated bandwidth = 100.0");

    sys.setPriorityTier("player1", 1);
    assertTrue(approxEqual(sys.getEstimatedBandwidth("player1"), 0.75f * 100.0f),
               "Tier 1 estimated bandwidth = 75.0");

    sys.setPriorityTier("player1", 2);
    assertTrue(approxEqual(sys.getEstimatedBandwidth("player1"), 0.5f * 100.0f),
               "Tier 2 estimated bandwidth = 50.0");

    sys.setPriorityTier("player1", 3);
    assertTrue(approxEqual(sys.getEstimatedBandwidth("player1"), 0.25f * 100.0f),
               "Tier 3 estimated bandwidth = 25.0");
}

static void testIMPMissingEntity() {
    std::cout << "\n=== IMP Missing Entity ===" << std::endl;

    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);

    assertTrue(!sys.setClientId("ghost", 42), "setClientId fails for nonexistent entity");
    assertTrue(!sys.setPriorityTier("ghost", 1), "setPriorityTier fails for nonexistent entity");
    assertTrue(!sys.setDistance("ghost", 100.0f), "setDistance fails for nonexistent entity");
    assertTrue(!sys.needsUpdate("ghost"), "needsUpdate returns false for nonexistent entity");
    assertTrue(sys.getPriorityTier("ghost") == -1, "getPriorityTier returns -1 for nonexistent entity");
    assertTrue(approxEqual(sys.getBandwidthWeight("ghost"), 0.0f), "getBandwidthWeight returns 0 for nonexistent entity");
    assertTrue(sys.getTotalUpdates("ghost") == 0, "getTotalUpdates returns 0 for nonexistent entity");
    assertTrue(approxEqual(sys.getEstimatedBandwidth("ghost"), 0.0f), "getEstimatedBandwidth returns 0 for nonexistent entity");
}

void run_interest_management_priority_system_tests() {
    testIMPCreateAndDefaults();
    testIMPTierSettings();
    testIMPDistanceAutoTier();
    testIMPUpdateCycle();
    testIMPBandwidthEstimation();
    testIMPMissingEntity();
}
