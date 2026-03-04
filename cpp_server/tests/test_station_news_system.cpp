// Tests for: Station News System Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/station_news_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Station News System Tests ====================

static void testStationNewsEmpty() {
    std::cout << "\n=== Station News Empty ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("station1");
    addComp<components::StationNewsFeed>(e);

    systems::StationNewsSystem sys(&world);
    assertTrue(sys.getNewsCount("station1") == 0, "No news initially");
    auto news = sys.getNews("station1");
    assertTrue(news.empty(), "Empty news list");
}

static void testStationNewsAddCombat() {
    std::cout << "\n=== Station News Add Combat ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("station2");
    addComp<components::StationNewsFeed>(e);

    systems::StationNewsSystem sys(&world);
    sys.reportCombatEvent("station2", "Pirates defeated", 100.0f);
    assertTrue(sys.getNewsCount("station2") == 1, "One news entry");
    auto news = sys.getNews("station2");
    assertTrue(news[0].category == "combat", "Category is combat");
}

static void testStationNewsMaxEntries() {
    std::cout << "\n=== Station News Max Entries ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("station3");
    auto* feed = addComp<components::StationNewsFeed>(e);
    feed->max_entries = 5;

    systems::StationNewsSystem sys(&world);
    for (int i = 0; i < 10; i++) {
        sys.reportCombatEvent("station3", "Event " + std::to_string(i), static_cast<float>(i));
    }
    assertTrue(sys.getNewsCount("station3") == 5, "Entries trimmed to max");
}


void run_station_news_system_tests() {
    testStationNewsEmpty();
    testStationNewsAddCombat();
    testStationNewsMaxEntries();
}
