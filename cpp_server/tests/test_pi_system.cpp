// Tests for: PISystem
#include "test_log.h"
#include "components/game_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/pi_system.h"

using namespace atlas;

// ==================== PISystem Tests ====================

static void testPIInstallExtractor() {
    std::cout << "\n=== PI: InstallExtractor ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    addComp<components::PlanetaryColony>(e);

    assertTrue(sys.installExtractor("colony1", "base_metals", 100), "Install extractor");
    assertTrue(sys.getExtractorCount("colony1") == 1, "Extractor count is 1");
}

static void testPIInstallMultipleExtractors() {
    std::cout << "\n=== PI: InstallMultipleExtractors ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    addComp<components::PlanetaryColony>(e);

    assertTrue(sys.installExtractor("colony1", "base_metals", 100), "Install extractor 1");
    assertTrue(sys.installExtractor("colony1", "aqueous_liquids", 50), "Install extractor 2");
    assertTrue(sys.getExtractorCount("colony1") == 2, "Extractor count is 2");
}

static void testPIInstallProcessor() {
    std::cout << "\n=== PI: InstallProcessor ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    addComp<components::PlanetaryColony>(e);

    assertTrue(sys.installProcessor("colony1", "base_metals", "reactive_metals", 40, 5),
               "Install processor");
    assertTrue(sys.getProcessorCount("colony1") == 1, "Processor count is 1");
}

static void testPIExtractionCycle() {
    std::cout << "\n=== PI: ExtractionCycle ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(e);

    sys.installExtractor("colony1", "base_metals", 100);
    // Set short cycle time for testing
    colony->extractors[0].cycle_time = 1.0f;

    // Run one full cycle
    sys.update(1.1f);

    assertTrue(sys.getStoredResource("colony1", "base_metals") == 100, "Extracted 100 base_metals");
    assertTrue(sys.getTotalStored("colony1") == 100, "Total stored is 100");
}

static void testPIMultipleExtractionCycles() {
    std::cout << "\n=== PI: MultipleExtractionCycles ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(e);

    sys.installExtractor("colony1", "base_metals", 50);
    colony->extractors[0].cycle_time = 1.0f;

    // Run 3 cycles
    sys.update(3.5f);

    assertTrue(sys.getStoredResource("colony1", "base_metals") == 150, "Extracted 150 over 3 cycles");
}

static void testPIProcessingCycle() {
    std::cout << "\n=== PI: ProcessingCycle ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(e);

    // Pre-fill storage with input resource
    components::PlanetaryColony::StoredResource sr;
    sr.resource_type = "base_metals";
    sr.quantity = 200;
    colony->storage.push_back(sr);

    sys.installProcessor("colony1", "base_metals", "reactive_metals", 40, 5);
    colony->processors[0].cycle_time = 1.0f;

    // Run one cycle
    sys.update(1.1f);

    assertTrue(sys.getStoredResource("colony1", "base_metals") == 160, "Consumed 40 base_metals");
    assertTrue(sys.getStoredResource("colony1", "reactive_metals") == 5, "Produced 5 reactive_metals");
}

static void testPIProcessingInsufficientInput() {
    std::cout << "\n=== PI: ProcessingInsufficientInput ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(e);

    // Only 10 units, processor needs 40
    components::PlanetaryColony::StoredResource sr;
    sr.resource_type = "base_metals";
    sr.quantity = 10;
    colony->storage.push_back(sr);

    sys.installProcessor("colony1", "base_metals", "reactive_metals", 40, 5);
    colony->processors[0].cycle_time = 1.0f;

    sys.update(1.1f);

    assertTrue(sys.getStoredResource("colony1", "base_metals") == 10, "Input unchanged (insufficient)");
    assertTrue(sys.getStoredResource("colony1", "reactive_metals") == 0, "No output produced");
}

static void testPIStorageCapacityLimit() {
    std::cout << "\n=== PI: StorageCapacityLimit ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(e);
    colony->storage_capacity = 100.0f;

    sys.installExtractor("colony1", "base_metals", 60);
    colony->extractors[0].cycle_time = 1.0f;

    // First cycle: 60 units stored
    sys.update(1.1f);
    assertTrue(sys.getStoredResource("colony1", "base_metals") == 60, "First cycle: 60 stored");

    // Second cycle: would be 120 but capacity is 100, so skipped
    sys.update(1.0f);
    assertTrue(sys.getTotalStored("colony1") <= 100, "Storage capacity respected");
}

static void testPICpuLimit() {
    std::cout << "\n=== PI: CpuLimit ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(e);
    colony->cpu_max = 100.0f;

    // Default extractor CPU is 45, so 2 fit (90), 3rd doesn't (135)
    assertTrue(sys.installExtractor("colony1", "base_metals", 100), "Install ext 1 (45 CPU)");
    assertTrue(sys.installExtractor("colony1", "aqueous_liquids", 50), "Install ext 2 (90 CPU)");
    assertTrue(!sys.installExtractor("colony1", "noble_metals", 25), "Install ext 3 rejected (135 > 100 CPU)");
    assertTrue(sys.getExtractorCount("colony1") == 2, "Only 2 extractors");
}

static void testPIPowergridLimit() {
    std::cout << "\n=== PI: PowergridLimit ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(e);
    colony->powergrid_max = 1000.0f;

    // Default extractor PG is 550
    assertTrue(sys.installExtractor("colony1", "base_metals", 100), "Install ext 1 (550 PG)");
    assertTrue(!sys.installExtractor("colony1", "aqueous_liquids", 50), "Install ext 2 rejected (1100 > 1000 PG)");
    assertTrue(sys.getExtractorCount("colony1") == 1, "Only 1 extractor");
}

static void testPIInactiveExtractorSkipped() {
    std::cout << "\n=== PI: InactiveExtractorSkipped ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(e);

    sys.installExtractor("colony1", "base_metals", 100);
    colony->extractors[0].cycle_time = 1.0f;
    colony->extractors[0].active = false;

    sys.update(2.0f);

    assertTrue(sys.getStoredResource("colony1", "base_metals") == 0, "Inactive extractor produces nothing");
}

static void testPIGetStoredResourceNonexistent() {
    std::cout << "\n=== PI: GetStoredResourceNonexistent ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);
    auto* e = world.createEntity("colony1");
    addComp<components::PlanetaryColony>(e);

    assertTrue(sys.getStoredResource("colony1", "unobtanium") == 0, "Non-existent resource returns 0");
}

static void testPIMissingEntity() {
    std::cout << "\n=== PI: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::PISystem sys(&world);

    assertTrue(!sys.installExtractor("ghost", "base_metals", 100), "Install fails for missing");
    assertTrue(!sys.installProcessor("ghost", "a", "b", 10, 5), "Processor fails for missing");
    assertTrue(sys.getStoredResource("ghost", "base_metals") == 0, "Storage 0 for missing");
    assertTrue(sys.getTotalStored("ghost") == 0, "Total 0 for missing");
    assertTrue(sys.getExtractorCount("ghost") == 0, "Extractor count 0 for missing");
    assertTrue(sys.getProcessorCount("ghost") == 0, "Processor count 0 for missing");
}

void run_pi_system_tests() {
    testPIInstallExtractor();
    testPIInstallMultipleExtractors();
    testPIInstallProcessor();
    testPIExtractionCycle();
    testPIMultipleExtractionCycles();
    testPIProcessingCycle();
    testPIProcessingInsufficientInput();
    testPIStorageCapacityLimit();
    testPICpuLimit();
    testPIPowergridLimit();
    testPIInactiveExtractorSkipped();
    testPIGetStoredResourceNonexistent();
    testPIMissingEntity();
}
