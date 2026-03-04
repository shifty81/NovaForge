// Tests for: Refining System Tests
#include "test_log.h"
#include "components/economy_components.h"
#include "components/fps_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/movement_system.h"
#include "systems/refining_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Refining System Tests ====================

static void testRefineOreBasic() {
    std::cout << "\n=== Refining: Basic Ore Refining ===" << std::endl;

    ecs::World world;
    systems::RefiningSystem refiningSys(&world);

    // Create a station with refining facility
    auto* station = world.createEntity("station1");
    auto* facility = addComp<components::RefiningFacility>(station);
    facility->efficiency = 1.0f;  // 100% efficiency
    facility->tax_rate = 0.0f;    // no tax

    // Add a recipe: 100 Ferrite → 415 Stellium
    components::RefiningFacility::RefineRecipe recipe;
    recipe.ore_type = "Ferrite";
    recipe.ore_units_required = 100;
    recipe.outputs.push_back({"Stellium", 415});
    facility->recipes.push_back(recipe);

    // Create a player with ore
    auto* player = world.createEntity("player1");
    auto* inv = addComp<components::Inventory>(player);
    inv->max_capacity = 10000.0f;
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 200;
    ore.volume = 0.1f;
    inv->items.push_back(ore);

    int refined = refiningSys.refineOre("player1", "station1", "Ferrite", 2);

    assertTrue(refined == 2, "Refined 2 batches");

    // Check ore consumed (200 - 200 = 0, should be removed)
    bool ore_found = false;
    int mineral_qty = 0;
    for (const auto& item : inv->items) {
        if (item.item_id == "Ferrite") ore_found = true;
        if (item.item_id == "Stellium") mineral_qty = item.quantity;
    }
    assertTrue(!ore_found, "Ore consumed completely");
    assertTrue(mineral_qty == 830, "Produced 830 Stellium (415 * 2)");
}

static void testRefineOreWithEfficiency() {
    std::cout << "\n=== Refining: Efficiency Affects Yield ===" << std::endl;

    ecs::World world;
    systems::RefiningSystem refiningSys(&world);

    auto* station = world.createEntity("station2");
    auto* facility = addComp<components::RefiningFacility>(station);
    facility->efficiency = 0.5f;
    facility->tax_rate = 0.0f;

    components::RefiningFacility::RefineRecipe recipe;
    recipe.ore_type = "Ferrite";
    recipe.ore_units_required = 100;
    recipe.outputs.push_back({"Stellium", 400});
    facility->recipes.push_back(recipe);

    auto* player = world.createEntity("player2");
    auto* inv = addComp<components::Inventory>(player);
    inv->max_capacity = 10000.0f;
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 100;
    ore.volume = 0.1f;
    inv->items.push_back(ore);

    int refined = refiningSys.refineOre("player2", "station2", "Ferrite", 1);
    assertTrue(refined == 1, "Refined 1 batch at 50% efficiency");

    int mineral_qty = 0;
    for (const auto& item : inv->items) {
        if (item.item_id == "Stellium") mineral_qty = item.quantity;
    }
    // 400 * 0.5 efficiency * (1 - 0.0 tax) = 200
    assertTrue(mineral_qty == 200, "50% efficiency yields 200 Stellium");
}

static void testRefineOreWithTax() {
    std::cout << "\n=== Refining: Tax Reduces Yield ===" << std::endl;

    ecs::World world;
    systems::RefiningSystem refiningSys(&world);

    auto* station = world.createEntity("station3");
    auto* facility = addComp<components::RefiningFacility>(station);
    facility->efficiency = 1.0f;
    facility->tax_rate = 0.1f;   // 10% tax

    components::RefiningFacility::RefineRecipe recipe;
    recipe.ore_type = "Ferrite";
    recipe.ore_units_required = 100;
    recipe.outputs.push_back({"Stellium", 400});
    facility->recipes.push_back(recipe);

    auto* player = world.createEntity("player3");
    auto* inv = addComp<components::Inventory>(player);
    inv->max_capacity = 10000.0f;
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 100;
    ore.volume = 0.1f;
    inv->items.push_back(ore);

    refiningSys.refineOre("player3", "station3", "Ferrite", 1);

    int mineral_qty = 0;
    for (const auto& item : inv->items) {
        if (item.item_id == "Stellium") mineral_qty = item.quantity;
    }
    // 400 * 1.0 * (1 - 0.1) = 360
    assertTrue(mineral_qty == 360, "10% tax yields 360 Stellium");
}

static void testRefineOreInsufficientOre() {
    std::cout << "\n=== Refining: Insufficient Ore ===" << std::endl;

    ecs::World world;
    systems::RefiningSystem refiningSys(&world);

    auto* station = world.createEntity("station4");
    auto* facility = addComp<components::RefiningFacility>(station);
    facility->efficiency = 1.0f;
    facility->tax_rate = 0.0f;

    components::RefiningFacility::RefineRecipe recipe;
    recipe.ore_type = "Ferrite";
    recipe.ore_units_required = 100;
    recipe.outputs.push_back({"Stellium", 415});
    facility->recipes.push_back(recipe);

    auto* player = world.createEntity("player4");
    auto* inv = addComp<components::Inventory>(player);
    inv->max_capacity = 10000.0f;
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 50;  // not enough for 1 batch
    ore.volume = 0.1f;
    inv->items.push_back(ore);

    int refined = refiningSys.refineOre("player4", "station4", "Ferrite", 1);
    assertTrue(refined == 0, "Cannot refine with insufficient ore");
    assertTrue(inv->items[0].quantity == 50, "Ore not consumed when refining fails");
}

static void testRefineOreNoRecipe() {
    std::cout << "\n=== Refining: No Recipe For Ore Type ===" << std::endl;

    ecs::World world;
    systems::RefiningSystem refiningSys(&world);

    auto* station = world.createEntity("station5");
    auto* facility = addComp<components::RefiningFacility>(station);
    // No recipes installed

    auto* player = world.createEntity("player5");
    auto* inv = addComp<components::Inventory>(player);
    inv->max_capacity = 10000.0f;
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 200;
    ore.volume = 0.1f;
    inv->items.push_back(ore);

    int refined = refiningSys.refineOre("player5", "station5", "Ferrite", 1);
    assertTrue(refined == 0, "Cannot refine without recipe");
}

static void testRefineOreMultipleOutputs() {
    std::cout << "\n=== Refining: Multiple Mineral Outputs ===" << std::endl;

    ecs::World world;
    systems::RefiningSystem refiningSys(&world);

    auto* station = world.createEntity("station6");
    auto* facility = addComp<components::RefiningFacility>(station);
    facility->efficiency = 1.0f;
    facility->tax_rate = 0.0f;

    // Galvite → Stellium + Vanthium
    components::RefiningFacility::RefineRecipe recipe;
    recipe.ore_type = "Galvite";
    recipe.ore_units_required = 100;
    recipe.outputs.push_back({"Stellium", 346});
    recipe.outputs.push_back({"Vanthium", 173});
    facility->recipes.push_back(recipe);

    auto* player = world.createEntity("player6");
    auto* inv = addComp<components::Inventory>(player);
    inv->max_capacity = 10000.0f;
    components::Inventory::Item ore;
    ore.item_id = "Galvite";
    ore.name = "Galvite";
    ore.type = "ore";
    ore.quantity = 100;
    ore.volume = 0.15f;
    inv->items.push_back(ore);

    int refined = refiningSys.refineOre("player6", "station6", "Galvite", 1);
    assertTrue(refined == 1, "Refined 1 batch of Galvite");

    int stellium = 0, vanthium = 0;
    for (const auto& item : inv->items) {
        if (item.item_id == "Stellium") stellium = item.quantity;
        if (item.item_id == "Vanthium") vanthium = item.quantity;
    }
    assertTrue(stellium == 346, "Produced 346 Stellium from Galvite");
    assertTrue(vanthium == 173, "Produced 173 Vanthium from Galvite");
}

static void testRefineDefaultRecipes() {
    std::cout << "\n=== Refining: Install Default Recipes ===" << std::endl;

    ecs::World world;
    systems::RefiningSystem refiningSys(&world);

    auto* station = world.createEntity("station7");
    addComp<components::RefiningFacility>(station);

    bool installed = refiningSys.installDefaultRecipes("station7");
    assertTrue(installed, "Default recipes installed");

    auto* facility = station->getComponent<components::RefiningFacility>();
    assertTrue(facility->recipes.size() == 4, "4 default recipes installed");
}


void run_refining_system_tests() {
    testRefineOreBasic();
    testRefineOreWithEfficiency();
    testRefineOreWithTax();
    testRefineOreInsufficientOre();
    testRefineOreNoRecipe();
    testRefineOreMultipleOutputs();
    testRefineDefaultRecipes();
}
