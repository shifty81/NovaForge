// Tests for: Captain Memory System Tests
#include "test_log.h"
#include "ecs/system.h"
#include "systems/captain_memory_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Captain Memory System Tests ====================

static void testCaptainMemoryRecordEvent() {
    std::cout << "\n=== Captain Memory: Record Event ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "vs Pirates", 100.0f, 0.5f);
    assertTrue(sys.totalMemories("cap1") == 1, "One memory recorded");
    assertTrue(sys.countMemories("cap1", "combat_win") == 1, "One combat_win memory");
}

static void testCaptainMemoryMultipleEvents() {
    std::cout << "\n=== Captain Memory: Multiple Events ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "vs Pirates", 100.0f, 0.5f);
    sys.recordMemory("cap1", "combat_loss", "vs Boss", 200.0f, -0.8f);
    sys.recordMemory("cap1", "saved_by_player", "close call", 300.0f, 0.9f);
    assertTrue(sys.totalMemories("cap1") == 3, "Three memories recorded");
    assertTrue(sys.countMemories("cap1", "combat_win") == 1, "One combat_win");
    assertTrue(sys.countMemories("cap1", "combat_loss") == 1, "One combat_loss");
}

static void testCaptainMemoryAverageWeight() {
    std::cout << "\n=== Captain Memory: Average Weight ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "", 100.0f, 1.0f);
    sys.recordMemory("cap1", "combat_loss", "", 200.0f, -1.0f);
    float avg = sys.averageEmotionalWeight("cap1");
    assertTrue(approxEqual(avg, 0.0f), "Average weight is zero for balanced events");
}

static void testCaptainMemoryMostRecent() {
    std::cout << "\n=== Captain Memory: Most Recent ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "", 100.0f, 0.5f);
    sys.recordMemory("cap1", "warp_anomaly", "strange lights", 200.0f, 0.2f);
    assertTrue(sys.mostRecentEvent("cap1") == "warp_anomaly", "Most recent is warp_anomaly");
}

static void testCaptainMemoryCapacity() {
    std::cout << "\n=== Captain Memory: Capacity Cap ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    // Fill 55 memories — should cap at 50
    for (int i = 0; i < 55; ++i) {
        sys.recordMemory("cap1", "event" + std::to_string(i), "", static_cast<float>(i), 0.1f);
    }
    assertTrue(sys.totalMemories("cap1") == 50, "Memory capped at 50");
}

static void testCaptainMemoryNoEntity() {
    std::cout << "\n=== Captain Memory: No Entity ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);

    assertTrue(sys.totalMemories("nonexistent") == 0, "No memories for nonexistent entity");
    assertTrue(sys.mostRecentEvent("nonexistent").empty(), "No recent event for nonexistent entity");
}


void run_captain_memory_system_tests() {
    testCaptainMemoryRecordEvent();
    testCaptainMemoryMultipleEvents();
    testCaptainMemoryAverageWeight();
    testCaptainMemoryMostRecent();
    testCaptainMemoryCapacity();
    testCaptainMemoryNoEntity();
}
