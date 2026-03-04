// Tests for: Farming Deck System Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "pcg/deck_graph.h"
#include "systems/farming_deck_system.h"

using namespace atlas;

// ==================== Farming Deck System Tests ====================

static void testFarmingDeckInit() {
    std::cout << "\n=== Farming Deck: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    assertTrue(sys.initializeDeck("ship1", "owner_001", 6), "Deck initialized");
    assertTrue(sys.getPlotCount("ship1") == 0, "No plots initially");
    assertTrue(!sys.initializeDeck("ship1", "owner_002", 4), "Duplicate init rejected");
}

static void testFarmingDeckPlant() {
    std::cout << "\n=== Farming Deck: Plant ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);

    assertTrue(sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain), "Crop planted");
    assertTrue(sys.getPlotCount("ship1") == 1, "1 plot");
    assertTrue(sys.getGrowthStage("ship1", "plot_a") == "planted", "Stage is planted");
    assertTrue(approxEqual(sys.getGrowthProgress("ship1", "plot_a"), 0.0f), "Progress at 0");
}

static void testFarmingDeckGrowth() {
    std::cout << "\n=== Farming Deck: Growth ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    // Update enough to advance growth
    for (int i = 0; i < 50; i++) {
        sys.waterPlot("ship1", "plot_a", 0.5f);
        sys.fertilizePlot("ship1", "plot_a", 0.5f);
        sys.update(0.5f);
    }
    assertTrue(sys.getGrowthProgress("ship1", "plot_a") > 0.0f, "Growth progressed");
    std::string stage = sys.getGrowthStage("ship1", "plot_a");
    assertTrue(stage != "planted", "Stage advanced beyond planted");
}

static void testFarmingDeckHarvest() {
    std::cout << "\n=== Farming Deck: Harvest ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    // Grow to harvestable
    for (int i = 0; i < 200; i++) {
        sys.waterPlot("ship1", "plot_a", 1.0f);
        sys.fertilizePlot("ship1", "plot_a", 1.0f);
        sys.update(0.1f);
    }
    assertTrue(sys.getGrowthStage("ship1", "plot_a") == "harvestable", "Crop is harvestable");

    float yield = sys.harvestCrop("ship1", "plot_a");
    assertTrue(yield > 0.0f, "Harvest yields food");
    assertTrue(sys.getGrowthStage("ship1", "plot_a") == "empty", "Plot reset to empty");
    assertTrue(sys.getTotalFoodProduced("ship1") > 0.0f, "Total food increased");
}

static void testFarmingDeckHarvestNotReady() {
    std::cout << "\n=== Farming Deck: Harvest Not Ready ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    sys.update(0.1f);  // barely any growth
    float yield = sys.harvestCrop("ship1", "plot_a");
    assertTrue(approxEqual(yield, 0.0f), "Cannot harvest unready crop");
}

static void testFarmingDeckWater() {
    std::cout << "\n=== Farming Deck: Water ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    // Water should consume first, then add
    sys.update(1.0f);  // consume some water
    assertTrue(sys.waterPlot("ship1", "plot_a", 0.5f), "Watered plot");
}

static void testFarmingDeckFertilize() {
    std::cout << "\n=== Farming Deck: Fertilize ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    sys.update(1.0f);  // consume some nutrients
    assertTrue(sys.fertilizePlot("ship1", "plot_a", 0.5f), "Fertilized plot");
}

static void testFarmingDeckWithering() {
    std::cout << "\n=== Farming Deck: Withering ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    // Drain water low so it withers on next update cycle
    // Water starts at 1.0, consumption is 0.02/s. Use many small updates without rewatering.
    // Set growth rate slow by using low light
    sys.setLightLevel("ship1", 0.01f);  // minimal light = very slow growth
    for (int i = 0; i < 200; i++) {
        sys.update(0.5f);
    }
    assertTrue(sys.getGrowthStage("ship1", "plot_a") == "withered", "Crop withered without water");
}

static void testFarmingDeckPower() {
    std::cout << "\n=== Farming Deck: Power ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    assertTrue(sys.setPowerEnabled("ship1", false), "Power disabled");
    float progress_before = sys.getGrowthProgress("ship1", "plot_a");
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getGrowthProgress("ship1", "plot_a"), progress_before), "No growth without power");
}

static void testFarmingDeckMissing() {
    std::cout << "\n=== Farming Deck: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FarmingDeckSystem sys(&world);
    assertTrue(!sys.initializeDeck("nonexistent", "owner", 6), "Init fails on missing");
    assertTrue(!sys.plantCrop("nonexistent", "plot", components::FarmingDeck::CropType::Grain), "Plant fails on missing");
    assertTrue(sys.getPlotCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(approxEqual(sys.getTotalFoodProduced("nonexistent"), 0.0f), "Food 0 on missing");
}


void run_farming_deck_system_tests() {
    testFarmingDeckInit();
    testFarmingDeckPlant();
    testFarmingDeckGrowth();
    testFarmingDeckHarvest();
    testFarmingDeckHarvestNotReady();
    testFarmingDeckWater();
    testFarmingDeckFertilize();
    testFarmingDeckWithering();
    testFarmingDeckPower();
    testFarmingDeckMissing();
}
