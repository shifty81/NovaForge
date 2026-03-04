// Tests for: Phase 5 Continued: Compressed Persistence Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "data/world_persistence.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Phase 5 Continued: Compressed Persistence Tests ====================

static void testPersistenceCompressedSaveLoad() {
    std::cout << "\n=== Persistence: Compressed Save/Load ===" << std::endl;
    ecs::World world;

    // Create entities with various components
    for (int i = 0; i < 50; ++i) {
        std::string id = "compress_ship_" + std::to_string(i);
        auto* e = world.createEntity(id);

        auto* pos = addComp<components::Position>(e);
        pos->x = static_cast<float>(i * 500);
        pos->y = static_cast<float>(i * 100);
        pos->z = 0.0f;

        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 300.0f + static_cast<float>(i);
        hp->shield_max = 500.0f;
        hp->armor_hp = 200.0f;
        hp->armor_max = 300.0f;
        hp->hull_hp = 100.0f;
        hp->hull_max = 200.0f;

        auto* ship = addComp<components::Ship>(e);
        ship->ship_type = "Frigate";
    }

    assertTrue(world.getEntityCount() == 50, "Created 50 entities for compressed test");

    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_compressed_test.json.gz";

    bool saved = persistence.saveWorldCompressed(&world, filepath);
    assertTrue(saved, "Compressed save succeeded");

    // Load into fresh world
    ecs::World world2;
    bool loaded = persistence.loadWorldCompressed(&world2, filepath);
    assertTrue(loaded, "Compressed load succeeded");
    assertTrue(world2.getEntityCount() == 50, "Loaded world has 50 entities");

    // Verify a sample
    auto* e10 = world2.getEntity("compress_ship_10");
    assertTrue(e10 != nullptr, "Ship 10 exists after compressed load");
    auto* pos10 = e10->getComponent<components::Position>();
    assertTrue(pos10 != nullptr, "Position component preserved");
    assertTrue(approxEqual(pos10->x, 5000.0f), "Position x preserved through compression");

    auto* hp10 = e10->getComponent<components::Health>();
    assertTrue(hp10 != nullptr, "Health component preserved");
    assertTrue(approxEqual(hp10->shield_hp, 310.0f), "Health preserved through compression");

    // Clean up
    std::remove(filepath.c_str());
}

static void testPersistenceCompressedSmaller() {
    std::cout << "\n=== Persistence: Compressed File Is Smaller ===" << std::endl;
    ecs::World world;

    for (int i = 0; i < 100; ++i) {
        std::string id = "size_ship_" + std::to_string(i);
        auto* e = world.createEntity(id);
        auto* pos = addComp<components::Position>(e);
        pos->x = static_cast<float>(i * 1000);
        pos->y = static_cast<float>(i * 200);
        pos->z = static_cast<float>(i * 50);
        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 500.0f;
        hp->shield_max = 500.0f;
        auto* ship = addComp<components::Ship>(e);
        ship->ship_type = "Cruiser";
    }

    data::WorldPersistence persistence;
    std::string jsonPath = "/tmp/eve_size_test.json";
    std::string gzPath   = "/tmp/eve_size_test.json.gz";

    persistence.saveWorld(&world, jsonPath);
    persistence.saveWorldCompressed(&world, gzPath);

    // Compare file sizes
    struct stat jsonStat, gzStat;
    stat(jsonPath.c_str(), &jsonStat);
    stat(gzPath.c_str(), &gzStat);

    assertTrue(gzStat.st_size < jsonStat.st_size,
               "Compressed file is smaller than JSON");
    assertTrue(gzStat.st_size > 0, "Compressed file is not empty");

    std::remove(jsonPath.c_str());
    std::remove(gzPath.c_str());
}


void run_compressed_persistence_tests() {
    testPersistenceCompressedSaveLoad();
    testPersistenceCompressedSmaller();
}
