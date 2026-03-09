// Tests for: ModDocGeneratorSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/mod_doc_generator_system.h"

using namespace atlas;

// ==================== ModDocGeneratorSystem Tests ====================

static void testModDocCreate() {
    std::cout << "\n=== ModDocGenerator: Create ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");

    assertTrue(sys.createGenerator("doc1"), "Create generator");
    assertTrue(sys.getEntryCount("doc1") == 0, "No entries initially");
    assertTrue(!sys.isGenerated("doc1"), "Not generated initially");
    assertTrue(sys.getValidatedCount("doc1") == 0, "No validated initially");
    assertTrue(sys.getGenerationCount("doc1") == 0, "Generation count is 0");
}

static void testModDocRegisterType() {
    std::cout << "\n=== ModDocGenerator: RegisterType ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");

    assertTrue(sys.registerType("doc1", "Ship", "vehicles", "Player-controlled ships", 15),
               "Register Ship type");
    assertTrue(sys.getEntryCount("doc1") == 1, "Entry count is 1");
    assertTrue(sys.getEntriesByCategory("doc1", "vehicles") == 1, "1 vehicle entry");
}

static void testModDocRegisterMultipleTypes() {
    std::cout << "\n=== ModDocGenerator: RegisterMultipleTypes ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");

    assertTrue(sys.registerType("doc1", "Ship", "vehicles", "Ships", 15), "Register Ship");
    assertTrue(sys.registerType("doc1", "Module", "equipment", "Modules", 10), "Register Module");
    assertTrue(sys.registerType("doc1", "Mission", "content", "Missions", 20), "Register Mission");

    assertTrue(sys.getEntryCount("doc1") == 3, "Entry count is 3");
    assertTrue(sys.getEntriesByCategory("doc1", "vehicles") == 1, "1 vehicle");
    assertTrue(sys.getEntriesByCategory("doc1", "equipment") == 1, "1 equipment");
    assertTrue(sys.getEntriesByCategory("doc1", "content") == 1, "1 content");
}

static void testModDocRejectDuplicateType() {
    std::cout << "\n=== ModDocGenerator: RejectDuplicateType ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");

    assertTrue(sys.registerType("doc1", "Ship", "vehicles", "Ships", 15), "Register Ship");
    assertTrue(!sys.registerType("doc1", "Ship", "vehicles", "Ships again", 15), "Duplicate rejected");
    assertTrue(sys.getEntryCount("doc1") == 1, "Still 1 entry");
}

static void testModDocAddExample() {
    std::cout << "\n=== ModDocGenerator: AddExample ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");
    sys.registerType("doc1", "Ship", "vehicles", "Ships", 15);

    assertTrue(sys.addExample("doc1", "Ship"), "Add example for Ship");
}

static void testModDocAddExampleUnknownType() {
    std::cout << "\n=== ModDocGenerator: AddExampleUnknownType ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");

    assertTrue(!sys.addExample("doc1", "Ghost"), "Cannot add example for unknown type");
}

static void testModDocValidateEntry() {
    std::cout << "\n=== ModDocGenerator: ValidateEntry ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");
    sys.registerType("doc1", "Ship", "vehicles", "Ships", 15);
    sys.addExample("doc1", "Ship");

    assertTrue(sys.validateEntry("doc1", "Ship"), "Validate Ship entry");
    assertTrue(sys.getValidatedCount("doc1") == 1, "1 validated entry");
}

static void testModDocValidateFailsWithoutExample() {
    std::cout << "\n=== ModDocGenerator: ValidateFailsWithoutExample ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");
    sys.registerType("doc1", "Ship", "vehicles", "Ships", 15);

    assertTrue(!sys.validateEntry("doc1", "Ship"), "Validate fails without example");
}

static void testModDocValidateFailsEmptyDescription() {
    std::cout << "\n=== ModDocGenerator: ValidateFailsEmptyDescription ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");
    sys.registerType("doc1", "Ship", "vehicles", "", 15);
    sys.addExample("doc1", "Ship");

    assertTrue(!sys.validateEntry("doc1", "Ship"), "Validate fails with empty description");
}

static void testModDocValidateFailsZeroFields() {
    std::cout << "\n=== ModDocGenerator: ValidateFailsZeroFields ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");
    sys.registerType("doc1", "Ship", "vehicles", "Ships", 0);
    sys.addExample("doc1", "Ship");

    assertTrue(!sys.validateEntry("doc1", "Ship"), "Validate fails with 0 fields");
}

static void testModDocGenerate() {
    std::cout << "\n=== ModDocGenerator: Generate ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");
    sys.registerType("doc1", "Ship", "vehicles", "Ships", 15);
    sys.addExample("doc1", "Ship");
    sys.validateEntry("doc1", "Ship");

    assertTrue(sys.generate("doc1"), "Generate documentation");
    assertTrue(sys.isGenerated("doc1"), "Documentation is generated");
    assertTrue(sys.getGenerationCount("doc1") == 1, "Generation count is 1");
}

static void testModDocGenerateFailsWithUnvalidated() {
    std::cout << "\n=== ModDocGenerator: GenerateFailsWithUnvalidated ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");
    sys.registerType("doc1", "Ship", "vehicles", "Ships", 15);

    assertTrue(!sys.generate("doc1"), "Generate fails with unvalidated entries");
}

static void testModDocGenerateFailsEmpty() {
    std::cout << "\n=== ModDocGenerator: GenerateFailsEmpty ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");

    assertTrue(!sys.generate("doc1"), "Generate fails with no entries");
}

static void testModDocRegenerateIncrementsCount() {
    std::cout << "\n=== ModDocGenerator: RegenerateIncrementsCount ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");
    sys.registerType("doc1", "Ship", "vehicles", "Ships", 15);
    sys.addExample("doc1", "Ship");
    sys.validateEntry("doc1", "Ship");

    sys.generate("doc1");
    sys.generate("doc1");
    assertTrue(sys.getGenerationCount("doc1") == 2, "Generation count is 2");
}

static void testModDocSetTitleAndVersion() {
    std::cout << "\n=== ModDocGenerator: SetTitleAndVersion ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");

    assertTrue(sys.setTitle("doc1", "NovaForge Modding API"), "Set title");
    assertTrue(sys.setVersion("doc1", "2.0"), "Set version");
}

static void testModDocCategoryFilter() {
    std::cout << "\n=== ModDocGenerator: CategoryFilter ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("doc1");
    sys.createGenerator("doc1");

    sys.registerType("doc1", "Ship", "vehicles", "Ships", 15);
    sys.registerType("doc1", "Rover", "vehicles", "Rovers", 8);
    sys.registerType("doc1", "Laser", "weapons", "Lasers", 12);

    assertTrue(sys.getEntriesByCategory("doc1", "vehicles") == 2, "2 vehicles");
    assertTrue(sys.getEntriesByCategory("doc1", "weapons") == 1, "1 weapon");
    assertTrue(sys.getEntriesByCategory("doc1", "armor") == 0, "0 armor");
}

static void testModDocMissingEntity() {
    std::cout << "\n=== ModDocGenerator: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);

    assertTrue(!sys.createGenerator("ghost"), "Create fails for missing");
    assertTrue(!sys.registerType("ghost", "Ship", "v", "d", 1), "Register fails for missing");
    assertTrue(!sys.addExample("ghost", "Ship"), "AddExample fails for missing");
    assertTrue(!sys.validateEntry("ghost", "Ship"), "Validate fails for missing");
    assertTrue(!sys.generate("ghost"), "Generate fails for missing");
    assertTrue(sys.getEntryCount("ghost") == 0, "Entry count 0 for missing");
    assertTrue(sys.getEntriesByCategory("ghost", "v") == 0, "Category 0 for missing");
    assertTrue(!sys.isGenerated("ghost"), "Not generated for missing");
    assertTrue(sys.getValidatedCount("ghost") == 0, "Validated 0 for missing");
    assertTrue(sys.getGenerationCount("ghost") == 0, "Gen count 0 for missing");
    assertTrue(!sys.setTitle("ghost", "t"), "setTitle fails for missing");
    assertTrue(!sys.setVersion("ghost", "v"), "setVersion fails for missing");
}

void run_mod_doc_generator_system_tests() {
    testModDocCreate();
    testModDocRegisterType();
    testModDocRegisterMultipleTypes();
    testModDocRejectDuplicateType();
    testModDocAddExample();
    testModDocAddExampleUnknownType();
    testModDocValidateEntry();
    testModDocValidateFailsWithoutExample();
    testModDocValidateFailsEmptyDescription();
    testModDocValidateFailsZeroFields();
    testModDocGenerate();
    testModDocGenerateFailsWithUnvalidated();
    testModDocGenerateFailsEmpty();
    testModDocRegenerateIncrementsCount();
    testModDocSetTitleAndVersion();
    testModDocCategoryFilter();
    testModDocMissingEntity();
}
