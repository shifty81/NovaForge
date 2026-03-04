// Tests for: FoodProcessorSystem Tests
#include "test_log.h"
#include "components/fps_components.h"
#include "components/ship_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/food_processor_system.h"

using namespace atlas;

// ==================== FoodProcessorSystem Tests ====================

static void testFoodProcessorAddRecipe() {
    std::cout << "\n=== Food Processor: Add Recipe ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}, {"water", 1}};
    assertTrue(sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f), "Recipe added");
    assertTrue(sys.getRecipeCount("processor1") == 1, "Recipe count is 1");
}

static void testFoodProcessorDuplicateRecipe() {
    std::cout << "\n=== Food Processor: Duplicate Recipe ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    assertTrue(!sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f), "Duplicate recipe rejected");
    assertTrue(sys.getRecipeCount("processor1") == 1, "Still 1 recipe");
}

static void testFoodProcessorStartCrafting() {
    std::cout << "\n=== Food Processor: Start Crafting ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    assertTrue(sys.startCrafting("processor1", "bread", "player1"), "Crafting started");
    assertTrue(sys.getActiveJobCount("processor1") == 1, "1 active job");
    assertTrue(!sys.startCrafting("processor1", "unknown_recipe", "player1"), "Unknown recipe fails");
}

static void testFoodProcessorMaxJobs() {
    std::cout << "\n=== Food Processor: Max Jobs ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    auto* fp = addComp<components::FoodProcessor>(e);
    fp->max_concurrent_jobs = 1;

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    sys.startCrafting("processor1", "bread", "player1");
    assertTrue(!sys.startCrafting("processor1", "bread", "player2"), "Max jobs reached");
    assertTrue(sys.getActiveJobCount("processor1") == 1, "Still 1 active job");
}

static void testFoodProcessorCraftCompletion() {
    std::cout << "\n=== Food Processor: Craft Completion ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    sys.startCrafting("processor1", "bread", "player1");
    assertTrue(!sys.isJobComplete("processor1", "player1"), "Job not complete yet");
    sys.update(11.0f); // past craft_time
    assertTrue(sys.isJobComplete("processor1", "player1"), "Job complete after update");
    assertTrue(sys.getActiveJobCount("processor1") == 0, "No active jobs after completion");
}

static void testFoodProcessorCancel() {
    std::cout << "\n=== Food Processor: Cancel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    sys.startCrafting("processor1", "bread", "player1");
    assertTrue(sys.cancelCrafting("processor1", "player1"), "Crafting cancelled");
    assertTrue(sys.getActiveJobCount("processor1") == 0, "No active jobs after cancel");
    assertTrue(!sys.cancelCrafting("processor1", "player1"), "Cancel on empty fails");
}

static void testFoodProcessorPowered() {
    std::cout << "\n=== Food Processor: Powered ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    assertTrue(sys.isPowered("processor1"), "Initially powered");
    sys.setPowered("processor1", false);
    assertTrue(!sys.isPowered("processor1"), "Unpowered after toggle");

    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    assertTrue(!sys.startCrafting("processor1", "bread", "player1"), "Cannot craft when unpowered");
}

static void testFoodProcessorEfficiency() {
    std::cout << "\n=== Food Processor: Efficiency ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    sys.setEfficiency("processor1", 2.0f);
    assertTrue(approxEqual(sys.getEfficiency("processor1"), 2.0f), "Efficiency set to 2.0");

    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    sys.startCrafting("processor1", "bread", "player1");
    sys.update(6.0f); // 6.0 * 2.0 = 12.0 effective, past 10.0
    assertTrue(sys.isJobComplete("processor1", "player1"), "Job completes faster with efficiency 2.0");
}

static void testFoodProcessorMissingEntity() {
    std::cout << "\n=== Food Processor: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FoodProcessorSystem sys(&world);
    assertTrue(sys.getRecipeCount("nonexistent") == 0, "Default recipe count for missing");
    assertTrue(sys.getActiveJobCount("nonexistent") == 0, "Default job count for missing");
    assertTrue(!sys.isPowered("nonexistent"), "Default powered for missing");
    assertTrue(approxEqual(sys.getEfficiency("nonexistent"), 0.0f), "Default efficiency for missing");
}

static void testFoodProcessorComponentDefaults() {
    std::cout << "\n=== Food Processor: Component Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    auto* fp = addComp<components::FoodProcessor>(e);
    assertTrue(fp->max_concurrent_jobs == 1, "Default max_concurrent_jobs is 1");
    assertTrue(approxEqual(fp->efficiency, 1.0f), "Default efficiency is 1.0");
    assertTrue(fp->powered == true, "Default powered is true");
    assertTrue(fp->available_recipes.empty(), "Default recipes empty");
    assertTrue(fp->active_jobs.empty(), "Default jobs empty");
}


void run_food_processor_system_tests() {
    testFoodProcessorAddRecipe();
    testFoodProcessorDuplicateRecipe();
    testFoodProcessorStartCrafting();
    testFoodProcessorMaxJobs();
    testFoodProcessorCraftCompletion();
    testFoodProcessorCancel();
    testFoodProcessorPowered();
    testFoodProcessorEfficiency();
    testFoodProcessorMissingEntity();
    testFoodProcessorComponentDefaults();
}
