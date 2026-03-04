// Tests for: Station Monument System tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/narrative_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/movement_system.h"
#include "systems/station_monument_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Station Monument System tests ====================

static void testMonumentBelowMinScore() {
    std::cout << "\n=== Monument Below Min Score ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 10; // Below kMonumentMinScore (25)

    std::string monumentId = monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    assertTrue(monumentId.empty(), "No monument for score below threshold");
    assertTrue(monumentSys.getMonumentCount("station1") == 0, "Zero monuments in station");
}

static void testMonumentCreatedForNotable() {
    std::cout << "\n=== Monument Created for Notable ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 30; // Notable, above threshold

    std::string monumentId = monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    assertTrue(!monumentId.empty(), "Monument created for notable player");
    assertTrue(monumentSys.getMonumentCount("station1") == 1, "One monument in station");
    assertTrue(monumentSys.getMonumentType("station1", "player1") == "Plaque",
               "Monument type is Plaque");
}

static void testMonumentTypeScaling() {
    std::cout << "\n=== Monument Type Scaling ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 110; // Statue tier

    std::string monumentId = monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    assertTrue(!monumentId.empty(), "Monument created for statue-tier score");
    assertTrue(monumentSys.getMonumentType("station1", "player1") == "Statue",
               "Monument type is Statue for score 110");
}

static void testMonumentUpgrade() {
    std::cout << "\n=== Monument Upgrade ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 30; // Plaque

    monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    assertTrue(monumentSys.getMonumentType("station1", "player1") == "Plaque",
               "Initially a Plaque");

    // Player becomes more famous
    legend->legend_score = 60; // Bust
    std::string upgradeId = monumentSys.checkAndCreateMonument("station1", "player1", 1.0f);
    assertTrue(!upgradeId.empty(), "Monument upgraded");
    assertTrue(monumentSys.getMonumentType("station1", "player1") == "Bust",
               "Upgraded to Bust");
    assertTrue(monumentSys.getMonumentCount("station1") == 1, "Still only one monument");
}

static void testMonumentNoUpgradeIfSameType() {
    std::cout << "\n=== Monument No Upgrade If Same Type ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 30; // Plaque

    monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    // Same score — should not change
    std::string result = monumentSys.checkAndCreateMonument("station1", "player1", 1.0f);
    assertTrue(result.empty(), "No monument id returned when no change");
    assertTrue(monumentSys.getMonumentCount("station1") == 1, "Count unchanged");
}

static void testMonumentMultiplePlayers() {
    std::cout << "\n=== Monument Multiple Players ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");

    for (int i = 1; i <= 3; ++i) {
        std::string pid = "player" + std::to_string(i);
        auto* ent = world.createEntity(pid);
        auto* legend = addComp<components::PlayerLegend>(ent);
        legend->legend_score = 50 * i;
        monumentSys.checkAndCreateMonument("station1", pid, 0.0f);
    }
    assertTrue(monumentSys.getMonumentCount("station1") == 3, "Three monuments for three players");
    assertTrue(monumentSys.getMonuments("station1").size() == 3, "getMonuments returns 3");
}


void run_station_monument_system_tests() {
    testMonumentBelowMinScore();
    testMonumentCreatedForNotable();
    testMonumentTypeScaling();
    testMonumentUpgrade();
    testMonumentNoUpgradeIfSameType();
    testMonumentMultiplePlayers();
}
