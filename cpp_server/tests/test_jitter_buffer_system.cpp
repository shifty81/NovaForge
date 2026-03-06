// Tests for: Jitter Buffer System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/jitter_buffer_system.h"

using namespace atlas;

// ==================== Jitter Buffer System Tests ====================

static void testJitterBufferCreate() {
    std::cout << "\n=== JitterBuffer: Create ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    assertTrue(sys.initialize("jb1"), "Init succeeds");
    assertTrue(sys.getSampleCount("jb1") == 0, "No samples initially");
    assertTrue(approxEqual(sys.getBufferSize("jb1"), 50.0f), "Default buffer 50ms");
    assertTrue(approxEqual(sys.getAverageJitter("jb1"), 0.0f), "No jitter initially");
    assertTrue(approxEqual(sys.getPeakJitter("jb1"), 0.0f), "No peak jitter");
    assertTrue(sys.getTotalPackets("jb1") == 0, "No packets");
    assertTrue(sys.getLostPackets("jb1") == 0, "No lost packets");
    assertTrue(sys.getUnderruns("jb1") == 0, "No underruns");
    assertTrue(sys.getOverruns("jb1") == 0, "No overruns");
}

static void testJitterBufferRecordPacket() {
    std::cout << "\n=== JitterBuffer: RecordPacket ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    sys.initialize("jb1");
    assertTrue(sys.recordPacket("jb1", 1, 100.0f, 100.0f), "Record perfect packet");
    assertTrue(sys.recordPacket("jb1", 2, 210.0f, 200.0f), "Record jittered packet");
    assertTrue(sys.getSampleCount("jb1") == 2, "2 samples recorded");
    assertTrue(sys.getTotalPackets("jb1") == 2, "2 total packets");
    // Jitter: packet1 = 0, packet2 = 10 → average = 5
    assertTrue(approxEqual(sys.getAverageJitter("jb1"), 5.0f), "Average jitter is 5ms");
    assertTrue(approxEqual(sys.getPeakJitter("jb1"), 10.0f), "Peak jitter is 10ms");
}

static void testJitterBufferLoss() {
    std::cout << "\n=== JitterBuffer: Loss ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    sys.initialize("jb1");
    sys.recordPacket("jb1", 1, 100.0f, 100.0f);
    sys.recordLoss("jb1");
    assertTrue(sys.getLostPackets("jb1") == 1, "1 lost packet");
    assertTrue(sys.getTotalPackets("jb1") == 2, "2 total packets (1 received + 1 lost)");
    float loss_rate = sys.getPacketLossRate("jb1");
    assertTrue(approxEqual(loss_rate, 0.5f), "50% loss rate");
}

static void testJitterBufferUnderrunOverrun() {
    std::cout << "\n=== JitterBuffer: UnderrunOverrun ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    sys.initialize("jb1");
    sys.recordUnderrun("jb1");
    sys.recordUnderrun("jb1");
    sys.recordOverrun("jb1");
    assertTrue(sys.getUnderruns("jb1") == 2, "2 underruns");
    assertTrue(sys.getOverruns("jb1") == 1, "1 overrun");
}

static void testJitterBufferAdaptiveSize() {
    std::cout << "\n=== JitterBuffer: AdaptiveSize ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    sys.initialize("jb1");

    // Record packets with high jitter
    sys.recordPacket("jb1", 1, 100.0f, 50.0f);   // jitter = 50
    sys.recordPacket("jb1", 2, 210.0f, 150.0f);   // jitter = 60
    // Average jitter = 55, target buffer = 110ms, adaptation moves toward it

    float before = sys.getBufferSize("jb1");
    sys.update(1.0f);  // trigger adaptive sizing
    float after = sys.getBufferSize("jb1");
    assertTrue(after > before, "Buffer grew due to high jitter");
}

static void testJitterBufferBounds() {
    std::cout << "\n=== JitterBuffer: Bounds ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    sys.initialize("jb1");
    assertTrue(sys.setBufferBounds("jb1", 10.0f, 100.0f), "Set bounds succeeds");
    assertTrue(!sys.setBufferBounds("jb1", 100.0f, 10.0f), "Inverted bounds rejected");
    assertTrue(!sys.setBufferBounds("jb1", -5.0f, 100.0f), "Negative min rejected");
}

static void testJitterBufferAdaptationRate() {
    std::cout << "\n=== JitterBuffer: AdaptationRate ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    sys.initialize("jb1");
    assertTrue(sys.setAdaptationRate("jb1", 0.5f), "Set rate succeeds");
    // Clamp test
    sys.setAdaptationRate("jb1", 2.0f);
    auto* entity = world.getEntity("jb1");
    auto* jb = entity->getComponent<components::JitterBuffer>();
    assertTrue(approxEqual(jb->adaptation_rate, 1.0f), "Rate clamped to 1.0");
}

static void testJitterBufferMaxSamples() {
    std::cout << "\n=== JitterBuffer: MaxSamples ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    sys.initialize("jb1");

    auto* entity = world.getEntity("jb1");
    auto* jb = entity->getComponent<components::JitterBuffer>();
    jb->max_samples = 3;

    sys.recordPacket("jb1", 1, 100.0f, 100.0f);
    sys.recordPacket("jb1", 2, 200.0f, 200.0f);
    sys.recordPacket("jb1", 3, 300.0f, 300.0f);
    sys.recordPacket("jb1", 4, 400.0f, 400.0f);
    assertTrue(sys.getSampleCount("jb1") == 3, "Max samples enforced (oldest evicted)");
    assertTrue(sys.getTotalPackets("jb1") == 4, "4 total packets counted");
}

static void testJitterBufferReset() {
    std::cout << "\n=== JitterBuffer: Reset ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    sys.initialize("jb1");
    sys.recordPacket("jb1", 1, 110.0f, 100.0f);
    sys.recordLoss("jb1");
    sys.recordUnderrun("jb1");
    assertTrue(sys.resetMetrics("jb1"), "Reset succeeds");
    assertTrue(sys.getSampleCount("jb1") == 0, "Samples cleared");
    assertTrue(sys.getTotalPackets("jb1") == 0, "Packets cleared");
    assertTrue(sys.getLostPackets("jb1") == 0, "Losses cleared");
    assertTrue(sys.getUnderruns("jb1") == 0, "Underruns cleared");
    assertTrue(approxEqual(sys.getAverageJitter("jb1"), 0.0f), "Jitter cleared");
}

static void testJitterBufferLossRate() {
    std::cout << "\n=== JitterBuffer: LossRate ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    world.createEntity("jb1");
    sys.initialize("jb1");
    // 0 packets → 0 loss rate
    assertTrue(approxEqual(sys.getPacketLossRate("jb1"), 0.0f), "0 loss rate with no packets");
    sys.recordPacket("jb1", 1, 100.0f, 100.0f);
    sys.recordPacket("jb1", 2, 200.0f, 200.0f);
    sys.recordPacket("jb1", 3, 300.0f, 300.0f);
    sys.recordLoss("jb1");
    // 3 received + 1 lost = 4 total, 1 lost → 25%
    assertTrue(approxEqual(sys.getPacketLossRate("jb1"), 0.25f), "25% loss rate");
}

static void testJitterBufferMissing() {
    std::cout << "\n=== JitterBuffer: Missing ===" << std::endl;
    ecs::World world;
    systems::JitterBufferSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.recordPacket("nonexistent", 1, 100.0f, 100.0f), "RecordPacket fails on missing");
    assertTrue(!sys.recordLoss("nonexistent"), "RecordLoss fails on missing");
    assertTrue(!sys.recordUnderrun("nonexistent"), "RecordUnderrun fails on missing");
    assertTrue(!sys.recordOverrun("nonexistent"), "RecordOverrun fails on missing");
    assertTrue(!sys.setBufferBounds("nonexistent", 10.0f, 100.0f), "SetBounds fails on missing");
    assertTrue(!sys.setAdaptationRate("nonexistent", 0.5f), "SetRate fails on missing");
    assertTrue(!sys.resetMetrics("nonexistent"), "Reset fails on missing");
    assertTrue(sys.getSampleCount("nonexistent") == 0, "0 samples on missing");
    assertTrue(approxEqual(sys.getBufferSize("nonexistent"), 0.0f), "0 buffer on missing");
    assertTrue(approxEqual(sys.getAverageJitter("nonexistent"), 0.0f), "0 jitter on missing");
    assertTrue(approxEqual(sys.getPeakJitter("nonexistent"), 0.0f), "0 peak on missing");
    assertTrue(sys.getTotalPackets("nonexistent") == 0, "0 packets on missing");
    assertTrue(sys.getLostPackets("nonexistent") == 0, "0 losses on missing");
    assertTrue(sys.getUnderruns("nonexistent") == 0, "0 underruns on missing");
    assertTrue(sys.getOverruns("nonexistent") == 0, "0 overruns on missing");
    assertTrue(approxEqual(sys.getPacketLossRate("nonexistent"), 0.0f), "0 loss rate on missing");
}


void run_jitter_buffer_system_tests() {
    testJitterBufferCreate();
    testJitterBufferRecordPacket();
    testJitterBufferLoss();
    testJitterBufferUnderrunOverrun();
    testJitterBufferAdaptiveSize();
    testJitterBufferBounds();
    testJitterBufferAdaptationRate();
    testJitterBufferMaxSamples();
    testJitterBufferReset();
    testJitterBufferLossRate();
    testJitterBufferMissing();
}
