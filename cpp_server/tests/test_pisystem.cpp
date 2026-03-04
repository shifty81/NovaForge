// Tests for: PISystem Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/pi_system.h"

using namespace atlas;

// ==================== PISystem Tests ====================

static void testPIInstallExtractor() {
    std::cout << "\n=== PI Install Extractor ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_1";
    colony->owner_id = "player1";
    colony->planet_type = "barren";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;

    bool ok = piSys.installExtractor("colony1", "base_metals", 100);
    assertTrue(ok, "Extractor installed successfully");
    assertTrue(piSys.getExtractorCount("colony1") == 1, "1 extractor present");
    assertTrue(colony->extractors[0].resource_type == "base_metals", "Extractor resource type correct");
    assertTrue(colony->extractors[0].quantity_per_cycle == 100, "Extractor quantity correct");
}

static void testPIInstallProcessor() {
    std::cout << "\n=== PI Install Processor ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony2");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_2";
    colony->owner_id = "player1";
    colony->planet_type = "temperate";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;

    bool ok = piSys.installProcessor("colony2", "base_metals", "refined_metals", 40, 5);
    assertTrue(ok, "Processor installed successfully");
    assertTrue(piSys.getProcessorCount("colony2") == 1, "1 processor present");
    assertTrue(colony->processors[0].input_type == "base_metals", "Processor input type correct");
    assertTrue(colony->processors[0].output_type == "refined_metals", "Processor output type correct");
}

static void testPIExtractionCycle() {
    std::cout << "\n=== PI Extraction Cycle ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony3");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_3";
    colony->owner_id = "player1";
    colony->planet_type = "lava";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;
    colony->storage_capacity = 10000.0f;

    piSys.installExtractor("colony3", "heavy_metals", 50);
    // Set short cycle time for testing
    colony->extractors[0].cycle_time = 10.0f;

    assertTrue(piSys.getTotalStored("colony3") == 0, "Storage starts empty");

    // Tick for one full cycle
    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony3", "heavy_metals") == 50,
               "50 heavy_metals extracted after 1 cycle");

    // Tick for another cycle
    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony3", "heavy_metals") == 100,
               "100 heavy_metals after 2 cycles");
}

static void testPIProcessingCycle() {
    std::cout << "\n=== PI Processing Cycle ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony4");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_4";
    colony->owner_id = "player1";
    colony->planet_type = "oceanic";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;
    colony->storage_capacity = 10000.0f;

    // Pre-load raw materials
    components::PlanetaryColony::StoredResource sr;
    sr.resource_type = "aqueous_liquids";
    sr.quantity = 200;
    colony->storage.push_back(sr);

    piSys.installProcessor("colony4", "aqueous_liquids", "water", 40, 5);
    colony->processors[0].cycle_time = 10.0f;

    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony4", "aqueous_liquids") == 160,
               "40 aqueous_liquids consumed");
    assertTrue(piSys.getStoredResource("colony4", "water") == 5,
               "5 water produced");
}

static void testPICpuPowergridLimit() {
    std::cout << "\n=== PI CPU/PG Limit ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony5");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_5";
    colony->owner_id = "player1";
    colony->planet_type = "gas";
    colony->cpu_max = 100.0f;    // Very limited
    colony->powergrid_max = 600.0f;

    bool ok1 = piSys.installExtractor("colony5", "noble_gas", 50);
    assertTrue(ok1, "First extractor fits");

    // Second extractor should fail (cpu 45+45=90 fits, but pg 550+550=1100 > 600)
    bool ok2 = piSys.installExtractor("colony5", "reactive_gas", 30);
    assertTrue(!ok2, "Second extractor rejected (PG exceeded)");
    assertTrue(piSys.getExtractorCount("colony5") == 1, "Still only 1 extractor");
}

static void testPIStorageCapacityLimit() {
    std::cout << "\n=== PI Storage Capacity Limit ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony6");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_6";
    colony->owner_id = "player1";
    colony->planet_type = "barren";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;
    colony->storage_capacity = 100.0f;

    piSys.installExtractor("colony6", "base_metals", 60);
    colony->extractors[0].cycle_time = 10.0f;

    // First cycle: 60 extracted (< 100 capacity)
    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony6", "base_metals") == 60,
               "60 extracted (under capacity)");

    // Second cycle: 60 + 60 = 120 > 100 capacity, should not extract
    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony6", "base_metals") == 60,
               "Still 60 (storage full, extraction skipped)");
}


void run_pisystem_tests() {
    testPIInstallExtractor();
    testPIInstallProcessor();
    testPIExtractionCycle();
    testPIProcessingCycle();
    testPICpuPowergridLimit();
    testPIStorageCapacityLimit();
}
