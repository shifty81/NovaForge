// Tests for: Sleeper Cache System
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/sleeper_cache_system.h"

using namespace atlas;

// ==================== Sleeper Cache System Tests ====================

static void testSleeperCacheCreate() {
    std::cout << "\n=== SleeperCache: Create ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);
    world.createEntity("cache1");
    assertTrue(sys.initialize("cache1", "wh_cache_01"), "Init succeeds");
    assertTrue(sys.getRoomCount("cache1") == 0, "No rooms initially");
    assertTrue(sys.getContainersHacked("cache1") == 0, "No hacked containers");
    assertTrue(sys.getContainersFailed("cache1") == 0, "No failed containers");
    assertTrue(approxEqual(sys.getTotalLootValue("cache1"), 0.0f), "No loot value");
    assertTrue(approxEqual(sys.getTimeRemaining("cache1"), 600.0f), "600s time limit");
    assertTrue(!sys.isExpired("cache1"), "Not expired");
    assertTrue(sys.getCacheTier("cache1") ==
               components::SleeperCacheState::CacheTier::Limited, "Limited tier");
}

static void testSleeperCacheTiers() {
    std::cout << "\n=== SleeperCache: Tiers ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);

    world.createEntity("std1");
    sys.initialize("std1", "wh_std", components::SleeperCacheState::CacheTier::Standard);
    assertTrue(sys.getCacheTier("std1") ==
               components::SleeperCacheState::CacheTier::Standard, "Standard tier");
    assertTrue(approxEqual(sys.getTimeRemaining("std1"), 900.0f), "900s for Standard");

    world.createEntity("sup1");
    sys.initialize("sup1", "wh_sup", components::SleeperCacheState::CacheTier::Superior);
    assertTrue(sys.getCacheTier("sup1") ==
               components::SleeperCacheState::CacheTier::Superior, "Superior tier");
    assertTrue(approxEqual(sys.getTimeRemaining("sup1"), 1200.0f), "1200s for Superior");
}

static void testSleeperCacheAddRoom() {
    std::cout << "\n=== SleeperCache: AddRoom ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);
    world.createEntity("cache1");
    sys.initialize("cache1");

    assertTrue(sys.addRoom("cache1", "room_a"), "Add room A");
    assertTrue(sys.getRoomCount("cache1") == 1, "1 room");
    assertTrue(sys.addRoom("cache1", "room_b", 4, 200.0f), "Add room B with 4 sentries");
    assertTrue(sys.getRoomCount("cache1") == 2, "2 rooms");
    assertTrue(!sys.isRoomOpen("cache1", "room_a"), "Room A locked");
}

static void testSleeperCacheOpenRoom() {
    std::cout << "\n=== SleeperCache: OpenRoom ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);
    world.createEntity("cache1");
    sys.initialize("cache1");
    sys.addRoom("cache1", "room_a");

    assertTrue(sys.openRoom("cache1", "room_a"), "Open room A");
    assertTrue(sys.isRoomOpen("cache1", "room_a"), "Room A is open");
    assertTrue(!sys.openRoom("cache1", "room_a"), "Cannot re-open");
}

static void testSleeperCacheHacking() {
    std::cout << "\n=== SleeperCache: Hacking ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);
    world.createEntity("cache1");
    sys.initialize("cache1");
    sys.addRoom("cache1", "room_a");
    sys.addContainer("cache1", "room_a", "c1", 40.0f, 1000000.0f);
    sys.openRoom("cache1", "room_a");

    // Partial hack
    assertTrue(sys.hackContainer("cache1", "room_a", "c1", 20.0f), "Hack 20/40");
    assertTrue(!sys.isContainerHacked("cache1", "room_a", "c1"), "Not yet hacked");
    assertTrue(sys.getContainersHacked("cache1") == 0, "0 hacked");

    // Complete hack
    assertTrue(sys.hackContainer("cache1", "room_a", "c1", 25.0f), "Hack 45/40 done");
    assertTrue(sys.isContainerHacked("cache1", "room_a", "c1"), "Container hacked");
    assertTrue(sys.getContainersHacked("cache1") == 1, "1 hacked");
    assertTrue(approxEqual(sys.getTotalLootValue("cache1"), 1000000.0f), "1M loot");
}

static void testSleeperCacheFailHack() {
    std::cout << "\n=== SleeperCache: FailHack ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);
    world.createEntity("cache1");
    sys.initialize("cache1");
    sys.addRoom("cache1", "room_a");
    sys.addContainer("cache1", "room_a", "c1", 40.0f, 500000.0f);
    sys.openRoom("cache1", "room_a");

    assertTrue(sys.failContainer("cache1", "room_a", "c1"), "Fail hack → explosion");
    assertTrue(sys.getContainersFailed("cache1") == 1, "1 failed");
    assertTrue(!sys.isContainerHacked("cache1", "room_a", "c1"), "Not hacked");
    assertTrue(!sys.failContainer("cache1", "room_a", "c1"), "Cannot fail twice");
}

static void testSleeperCacheDestroySentries() {
    std::cout << "\n=== SleeperCache: DestroySentries ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);
    world.createEntity("cache1");
    sys.initialize("cache1");
    sys.addRoom("cache1", "room_a");

    assertTrue(sys.destroySentries("cache1", "room_a"), "Destroy sentries");
    assertTrue(!sys.destroySentries("cache1", "room_a"), "Already destroyed");
}

static void testSleeperCacheExpiry() {
    std::cout << "\n=== SleeperCache: Expiry ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);
    world.createEntity("cache1");
    sys.initialize("cache1");
    sys.addRoom("cache1", "room_a");
    sys.addContainer("cache1", "room_a", "c1", 40.0f, 1000000.0f);
    sys.openRoom("cache1", "room_a");

    // Tick to near expiry
    sys.update(590.0f);
    assertTrue(!sys.isExpired("cache1"), "Not expired at 590s");
    assertTrue(approxEqual(sys.getTimeRemaining("cache1"), 10.0f), "10s remaining");

    // Expire
    sys.update(15.0f);
    assertTrue(sys.isExpired("cache1"), "Expired at 605s");
    assertTrue(approxEqual(sys.getTimeRemaining("cache1"), 0.0f), "0s remaining");

    // Cannot hack expired site
    assertTrue(!sys.hackContainer("cache1", "room_a", "c1", 50.0f), "Cannot hack expired");
}

static void testSleeperCacheMaxRooms() {
    std::cout << "\n=== SleeperCache: MaxRooms ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);
    world.createEntity("cache1");
    sys.initialize("cache1");  // Limited tier → max 3 rooms

    for (int i = 0; i < 3; i++) {
        assertTrue(sys.addRoom("cache1", "room_" + std::to_string(i)),
                   "Add room " + std::to_string(i));
    }
    assertTrue(sys.getRoomCount("cache1") == 3, "3 rooms (max)");
    assertTrue(!sys.addRoom("cache1", "overflow"), "Overflow rejected");
}

static void testSleeperCacheMissing() {
    std::cout << "\n=== SleeperCache: Missing ===" << std::endl;
    ecs::World world;
    systems::SleeperCacheSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addRoom("nonexistent", "r1"), "AddRoom fails on missing");
    assertTrue(!sys.openRoom("nonexistent", "r1"), "OpenRoom fails on missing");
    assertTrue(!sys.hackContainer("nonexistent", "r1", "c1", 10.0f), "Hack fails on missing");
    assertTrue(!sys.failContainer("nonexistent", "r1", "c1"), "FailContainer fails on missing");
    assertTrue(!sys.destroySentries("nonexistent", "r1"), "DestroySentries fails on missing");
    assertTrue(sys.getRoomCount("nonexistent") == 0, "0 rooms on missing");
    assertTrue(sys.getContainersHacked("nonexistent") == 0, "0 hacked on missing");
    assertTrue(sys.getContainersFailed("nonexistent") == 0, "0 failed on missing");
    assertTrue(approxEqual(sys.getTotalLootValue("nonexistent"), 0.0f), "0 loot on missing");
    assertTrue(approxEqual(sys.getTimeRemaining("nonexistent"), 0.0f), "0 time on missing");
    assertTrue(sys.isExpired("nonexistent"), "Expired on missing");
    assertTrue(sys.getCacheTier("nonexistent") ==
               components::SleeperCacheState::CacheTier::Limited, "Limited on missing");
}

void run_sleeper_cache_system_tests() {
    testSleeperCacheCreate();
    testSleeperCacheTiers();
    testSleeperCacheAddRoom();
    testSleeperCacheOpenRoom();
    testSleeperCacheHacking();
    testSleeperCacheFailHack();
    testSleeperCacheDestroySentries();
    testSleeperCacheExpiry();
    testSleeperCacheMaxRooms();
    testSleeperCacheMissing();
}
