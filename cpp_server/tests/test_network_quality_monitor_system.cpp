// Tests for: Network Quality Monitor System
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/network_quality_monitor_system.h"

using namespace atlas;

// ==================== Network Quality Monitor System Tests ====================

static void testNetworkQualityCreate() {
    std::cout << "\n=== NetworkQuality: Create ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    assertTrue(sys.initialize("conn1", "player_42"), "Init succeeds");
    assertTrue(sys.getConnectionId("conn1") == "player_42", "Connection ID matches");
    assertTrue(approxEqual(sys.getLatency("conn1"), 0.0f), "0 latency initially");
    assertTrue(approxEqual(sys.getJitter("conn1"), 0.0f), "0 jitter initially");
    assertTrue(approxEqual(sys.getPacketLossPct("conn1"), 0.0f), "0% loss initially");
    assertTrue(approxEqual(sys.getSnapshotRate("conn1"), 20.0f), "20Hz snapshot rate");
    assertTrue(sys.getSamplesReceived("conn1") == 0, "0 samples initially");
    assertTrue(!sys.isDegraded("conn1"), "Not degraded initially");
}

static void testNetworkQualityInitValidation() {
    std::cout << "\n=== NetworkQuality: InitValidation ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    assertTrue(!sys.initialize("conn1", ""), "Empty connection_id rejected");
    assertTrue(!sys.initialize("nonexistent", "abc"), "Missing entity rejected");
}

static void testNetworkQualityLatencySample() {
    std::cout << "\n=== NetworkQuality: LatencySample ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    sys.initialize("conn1", "player_1");

    // First sample sets latency directly
    assertTrue(sys.recordLatencySample("conn1", 50.0f), "Record 50ms");
    assertTrue(approxEqual(sys.getLatency("conn1"), 50.0f), "Latency 50ms after first sample");
    assertTrue(sys.getSamplesReceived("conn1") == 1, "1 sample");

    // Second sample uses EMA: 0.8*50 + 0.2*100 = 60
    assertTrue(sys.recordLatencySample("conn1", 100.0f), "Record 100ms");
    assertTrue(approxEqual(sys.getLatency("conn1"), 60.0f), "EMA latency ~60ms");
    assertTrue(sys.getSamplesReceived("conn1") == 2, "2 samples");
}

static void testNetworkQualityLatencyValidation() {
    std::cout << "\n=== NetworkQuality: LatencyValidation ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    sys.initialize("conn1", "player_1");
    assertTrue(!sys.recordLatencySample("conn1", -5.0f), "Negative latency rejected");
    assertTrue(!sys.recordLatencySample("nonexistent", 50.0f), "Missing entity rejected");
}

static void testNetworkQualityMinMaxLatency() {
    std::cout << "\n=== NetworkQuality: MinMaxLatency ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    sys.initialize("conn1", "player_1");
    sys.recordLatencySample("conn1", 50.0f);
    sys.recordLatencySample("conn1", 20.0f);
    sys.recordLatencySample("conn1", 150.0f);
    assertTrue(approxEqual(sys.getMinLatency("conn1"), 20.0f), "Min latency 20ms");
    assertTrue(approxEqual(sys.getMaxLatency("conn1"), 150.0f), "Max latency 150ms");
}

static void testNetworkQualityPacketLoss() {
    std::cout << "\n=== NetworkQuality: PacketLoss ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    sys.initialize("conn1", "player_1");
    assertTrue(sys.recordPacketLoss("conn1", 5, 100), "Record 5/100 lost");
    assertTrue(approxEqual(sys.getPacketLossPct("conn1"), 5.0f), "5% loss");
    assertTrue(sys.recordPacketLoss("conn1", 10, 100), "Record 10/100 more");
    // Cumulative: 15 lost / 200 total = 7.5%
    assertTrue(approxEqual(sys.getPacketLossPct("conn1"), 7.5f), "7.5% cumulative");
}

static void testNetworkQualityPacketLossValidation() {
    std::cout << "\n=== NetworkQuality: PacketLossValidation ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    sys.initialize("conn1", "player_1");
    assertTrue(!sys.recordPacketLoss("conn1", -1, 100), "Negative lost rejected");
    assertTrue(!sys.recordPacketLoss("conn1", 5, 0), "Zero total rejected");
    assertTrue(!sys.recordPacketLoss("nonexistent", 5, 100), "Missing entity rejected");
}

static void testNetworkQualityDegradation() {
    std::cout << "\n=== NetworkQuality: Degradation ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    sys.initialize("conn1", "player_1");

    // Record high latency to trigger degradation
    for (int i = 0; i < 10; i++) {
        sys.recordLatencySample("conn1", 250.0f);
    }
    sys.update(0.05f); // trigger evaluation
    assertTrue(sys.isDegraded("conn1"), "Degraded with high latency");
    assertTrue(sys.getSnapshotRate("conn1") < 20.0f, "Snapshot rate reduced");
}

static void testNetworkQualityResetStats() {
    std::cout << "\n=== NetworkQuality: ResetStats ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    sys.initialize("conn1", "player_1");
    sys.recordLatencySample("conn1", 100.0f);
    sys.recordPacketLoss("conn1", 10, 100);
    assertTrue(sys.resetStats("conn1"), "Reset succeeds");
    assertTrue(approxEqual(sys.getLatency("conn1"), 0.0f), "Latency reset to 0");
    assertTrue(approxEqual(sys.getPacketLossPct("conn1"), 0.0f), "Loss reset to 0");
    assertTrue(sys.getSamplesReceived("conn1") == 0, "Samples reset to 0");
    assertTrue(!sys.isDegraded("conn1"), "Not degraded after reset");
    assertTrue(!sys.resetStats("nonexistent"), "Reset fails on missing");
}

static void testNetworkQualityCustomThresholds() {
    std::cout << "\n=== NetworkQuality: CustomThresholds ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    world.createEntity("conn1");
    sys.initialize("conn1", "player_1");
    sys.setLatencyThreshold(50.0f);  // very strict

    // 80ms > 50ms threshold should degrade
    for (int i = 0; i < 10; i++) {
        sys.recordLatencySample("conn1", 80.0f);
    }
    sys.update(0.05f);
    assertTrue(sys.isDegraded("conn1"), "Degraded with strict threshold");
}

static void testNetworkQualityMissing() {
    std::cout << "\n=== NetworkQuality: Missing ===" << std::endl;
    ecs::World world;
    systems::NetworkQualityMonitorSystem sys(&world);
    assertTrue(approxEqual(sys.getLatency("x"), 0.0f), "0 latency on missing");
    assertTrue(approxEqual(sys.getJitter("x"), 0.0f), "0 jitter on missing");
    assertTrue(approxEqual(sys.getPacketLossPct("x"), 0.0f), "0 loss on missing");
    assertTrue(approxEqual(sys.getSnapshotRate("x"), 20.0f), "20Hz on missing");
    assertTrue(sys.getSamplesReceived("x") == 0, "0 samples on missing");
    assertTrue(!sys.isDegraded("x"), "Not degraded on missing");
    assertTrue(sys.getConnectionId("x").empty(), "Empty conn id on missing");
}

void run_network_quality_monitor_system_tests() {
    testNetworkQualityCreate();
    testNetworkQualityInitValidation();
    testNetworkQualityLatencySample();
    testNetworkQualityLatencyValidation();
    testNetworkQualityMinMaxLatency();
    testNetworkQualityPacketLoss();
    testNetworkQualityPacketLossValidation();
    testNetworkQualityDegradation();
    testNetworkQualityResetStats();
    testNetworkQualityCustomThresholds();
    testNetworkQualityMissing();
}
