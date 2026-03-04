// Tests for: Ship Generation Model Data Tests
#include "test_log.h"
#include "components/core_components.h"
#include "data/ship_database.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Ship Generation Model Data Tests ====================

static void testShipModelDataParsed() {
    std::cout << "\n=== ShipDatabase: Model Data Parsed ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    const data::ShipTemplate* fang = db.getShip("fang");
    if (fang) {
        assertTrue(fang->model_data.has_model_data, "Fang has model_data");
        assertTrue(fang->model_data.turret_hardpoints >= 2, "Fang turret hardpoints >= 2");
        assertTrue(fang->model_data.turret_hardpoints <= 3, "Fang turret hardpoints <= 3");
        assertTrue(fang->model_data.engine_count >= 2, "Fang engine count >= 2");
        assertTrue(fang->model_data.engine_count <= 3, "Fang engine count <= 3");
        assertTrue(fang->model_data.generation_seed > 0, "Fang generation seed > 0");
    } else {
        assertTrue(false, "Fang template found for model_data test");
    }
}

static void testShipModelDataCapitalShips() {
    std::cout << "\n=== ShipDatabase: Capital Ship Model Data ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    const data::ShipTemplate* empyrean = db.getShip("empyrean");
    if (empyrean) {
        assertTrue(empyrean->model_data.has_model_data, "Empyrean has model_data");
        assertTrue(empyrean->model_data.turret_hardpoints >= 6, "Titan turrets >= 6");
        assertTrue(empyrean->model_data.turret_hardpoints <= 10, "Titan turrets <= 10");
        assertTrue(empyrean->model_data.engine_count >= 8, "Titan engines >= 8");
        assertTrue(empyrean->model_data.engine_count <= 12, "Titan engines <= 12");
    } else {
        assertTrue(false, "Empyrean template found for capital model_data test");
    }

    const data::ShipTemplate* solarius = db.getShip("solarius");
    if (solarius) {
        assertTrue(solarius->model_data.has_model_data, "Solarius has model_data");
        assertTrue(solarius->model_data.drone_bays >= 5, "Carrier drone_bays >= 5");
        assertTrue(solarius->model_data.drone_bays <= 10, "Carrier drone_bays <= 10");
    } else {
        assertTrue(false, "Solarius template found for carrier model_data test");
    }
}

static void testShipModelDataAllShipsHaveModelData() {
    std::cout << "\n=== ShipDatabase: All Ships Have Model Data ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    auto ids = db.getShipIds();
    assertTrue(ids.size() >= 90, "At least 90 ships loaded");

    int withModelData = 0;
    for (const auto& id : ids) {
        const data::ShipTemplate* ship = db.getShip(id);
        if (ship && ship->model_data.has_model_data) {
            withModelData++;
        }
    }

    assertTrue(withModelData == static_cast<int>(ids.size()),
               "All ships have model_data (" + std::to_string(withModelData) + "/" + std::to_string(ids.size()) + ")");
}

static void testShipModelDataSeedUniqueness() {
    std::cout << "\n=== ShipDatabase: Model Data Seed Uniqueness ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    auto ids = db.getShipIds();
    std::map<int, std::string> seedMap;
    int uniqueSeeds = 0;
    for (const auto& id : ids) {
        const data::ShipTemplate* ship = db.getShip(id);
        if (ship && ship->model_data.has_model_data && ship->model_data.generation_seed > 0) {
            if (seedMap.find(ship->model_data.generation_seed) == seedMap.end()) {
                seedMap[ship->model_data.generation_seed] = id;
                uniqueSeeds++;
            }
        }
    }

    // Seeds should be mostly unique (allow small number of collisions due to hash)
    assertTrue(uniqueSeeds >= static_cast<int>(ids.size()) * 9 / 10,
               "Most seeds are unique (" + std::to_string(uniqueSeeds) + "/" + std::to_string(ids.size()) + ")");
}

static void testShipModelDataEngineCountPositive() {
    std::cout << "\n=== ShipDatabase: All Ships Have Engines ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    auto ids = db.getShipIds();
    int allHaveEngines = 0;
    for (const auto& id : ids) {
        const data::ShipTemplate* ship = db.getShip(id);
        if (ship && ship->model_data.has_model_data && ship->model_data.engine_count >= 2) {
            allHaveEngines++;
        }
    }

    assertTrue(allHaveEngines == static_cast<int>(ids.size()),
               "All ships have >= 2 engines (" + std::to_string(allHaveEngines) + "/" + std::to_string(ids.size()) + ")");
}

static void testShipModelDataMissingReturnsDefaults() {
    std::cout << "\n=== ShipDatabase: Missing Model Data Returns Defaults ===" << std::endl;

    // A ShipTemplate without model_data block should have defaults
    data::ShipTemplate empty;
    assertTrue(!empty.model_data.has_model_data, "Default template has no model_data");
    assertTrue(empty.model_data.turret_hardpoints == 0, "Default turret hardpoints is 0");
    assertTrue(empty.model_data.launcher_hardpoints == 0, "Default launcher hardpoints is 0");
    assertTrue(empty.model_data.drone_bays == 0, "Default drone bays is 0");
    assertTrue(empty.model_data.engine_count == 2, "Default engine count is 2");
    assertTrue(empty.model_data.generation_seed == 0, "Default generation seed is 0");
}


void run_pcg_ship_generator_tests() {
    testShipModelDataParsed();
    testShipModelDataCapitalShips();
    testShipModelDataAllShipsHaveModelData();
    testShipModelDataSeedUniqueness();
    testShipModelDataEngineCountPositive();
    testShipModelDataMissingReturnsDefaults();
}
