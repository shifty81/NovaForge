// Tests for: Legend System Tests
#include "test_log.h"
#include "components/narrative_components.h"
#include "ecs/system.h"
#include "systems/legend_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Legend System Tests ====================

static void testLegendEmpty() {
    std::cout << "\n=== Legend Empty ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("player1");
    addComp<components::PlayerLegend>(e);

    systems::LegendSystem sys(&world);
    assertTrue(sys.getLegendScore("player1") == 0, "No score initially");
    assertTrue(sys.getTitle("player1") == "Unknown", "Title is Unknown");
}

static void testLegendRecord() {
    std::cout << "\n=== Legend Record ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("player2");
    addComp<components::PlayerLegend>(e);

    systems::LegendSystem sys(&world);
    sys.recordLegend("player2", "destroyed_capital", "Destroyed the Warlord", 100.0f, "sys_1", 25);
    assertTrue(sys.getLegendScore("player2") == 25, "Score is 25 after event");
    auto entries = sys.getLegendEntries("player2");
    assertTrue(static_cast<int>(entries.size()) == 1, "One entry recorded");
}

static void testLegendTitle() {
    std::cout << "\n=== Legend Title ===" << std::endl;
    assertTrue(systems::LegendSystem::computeTitle(0) == "Unknown", "Score 0 = Unknown");
    assertTrue(systems::LegendSystem::computeTitle(10) == "Notable", "Score 10 = Notable");
    assertTrue(systems::LegendSystem::computeTitle(50) == "Famous", "Score 50 = Famous");
    assertTrue(systems::LegendSystem::computeTitle(100) == "Legendary", "Score 100 = Legendary");
    assertTrue(systems::LegendSystem::computeTitle(500) == "Mythic", "Score 500 = Mythic");
}


void run_legend_system_tests() {
    testLegendEmpty();
    testLegendRecord();
    testLegendTitle();
}
