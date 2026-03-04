// Tests for: Character Mesh Cybernetics Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "components/fleet_components.h"
#include "components/fps_components.h"
#include "components/ship_components.h"
#include "pcg/pcg_context.h"
#include "systems/fleet_system.h"
#include "pcg/capital_ship_generator.h"
#include "pcg/character_mesh_system.h"
#include "pcg/collision_manager.h"
#include "pcg/deck_graph.h"
#include "pcg/elevator_system.h"
#include "pcg/fleet_doctrine.h"
#include "pcg/hull_mesher.h"
#include "pcg/room_graph.h"
#include "pcg/rover_system.h"
#include "pcg/salvage_system.h"
#include "pcg/ship_designer.h"
#include "pcg/ship_generator.h"
#include "pcg/snappable_grid.h"
#include "pcg/station_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Character Mesh Cybernetics Tests ====================

static void testCharacterBodyTypeOrganic() {
    std::cout << "\n=== PCG: Character organic body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    auto ch = cms.generate(42, Race::TerranDescendant, BodyType::Organic, sl);
    assertTrue(ch.cyber_percentage < 0.01f, "Organic has 0% cyber");
    assertTrue(ch.cyberLimbs.empty(), "Organic has no cyber limbs");
    assertTrue(ch.integrated_weapon_count == 0, "Organic has no weapon limbs");
}

static void testCharacterBodyTypeAugmented() {
    std::cout << "\n=== PCG: Character augmented body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    auto ch = cms.generate(100, Race::SynthBorn, BodyType::Augmented, sl);
    assertTrue(ch.cyber_percentage > 0.0f, "Augmented has some cyber");
    assertTrue(ch.cyber_percentage < 0.5f, "Augmented is less than 50% cyber");
    assertTrue(!ch.cyberLimbs.empty(), "Augmented has cyber limbs");
    assertTrue(static_cast<int>(ch.cyberLimbs.size()) <= 2, "Augmented has 1-2 limbs");
}

static void testCharacterBodyTypeCybernetic() {
    std::cout << "\n=== PCG: Character cybernetic body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    auto ch = cms.generate(200, Race::PureAlien, BodyType::Cybernetic, sl);
    assertTrue(ch.cyber_percentage >= 0.5f, "Cybernetic is 50%+ cyber");
    assertTrue(static_cast<int>(ch.cyberLimbs.size()) >= 3, "Cybernetic has 3+ limbs");
}

static void testCharacterBodyTypeFullSynth() {
    std::cout << "\n=== PCG: Character full synth body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    auto ch = cms.generate(300, Race::HybridEvolutionary, BodyType::FullSynth, sl);
    assertTrue(ch.cyber_percentage >= 0.99f, "FullSynth is 100% cyber");
    assertTrue(static_cast<int>(ch.cyberLimbs.size()) == 6, "FullSynth has all 6 limbs replaced");
}

static void testCharacterBodyTypeMechFrame() {
    std::cout << "\n=== PCG: Character mech frame body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    sl.height = 0.5f;
    auto organic = cms.generate(400, Race::TerranDescendant, BodyType::Organic, sl);
    auto mech    = cms.generate(400, Race::TerranDescendant, BodyType::MechFrame, sl);
    assertTrue(mech.total_height > organic.total_height, "MechFrame is taller than organic");
    assertTrue(mech.strength_multiplier > 1.5f, "MechFrame has high strength");
    assertTrue(mech.speed_multiplier < 1.0f, "MechFrame is slower");
}

static void testCharacterBodyTypeNames() {
    std::cout << "\n=== PCG: Character body type names ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::Organic) == "Organic", "Organic name");
    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::Augmented) == "Augmented", "Augmented name");
    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::Cybernetic) == "Cybernetic", "Cybernetic name");
    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::FullSynth) == "FullSynth", "FullSynth name");
    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::MechFrame) == "MechFrame", "MechFrame name");
}

static void testCharacterBackwardCompatibility() {
    std::cout << "\n=== PCG: Character backward compat ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    // Old API (no body type) should still work and produce Organic.
    auto ch = cms.generate(42, Race::TerranDescendant, sl);
    assertTrue(ch.bodyType == BodyType::Organic, "Old API defaults to Organic");
    assertTrue(ch.cyberLimbs.empty(), "Old API has no cyber limbs");
}

static void testCharacterReferenceMeshArchive() {
    std::cout << "\n=== PCG: Character reference mesh archive ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    assertTrue(cms.referenceMeshArchive().empty(), "No archive by default");

    cms.setReferenceMeshArchive("human.zip");
    assertTrue(cms.referenceMeshArchive() == "human.zip", "Archive path stored");

    CharacterSliders sl;
    auto ch = cms.generate(42, Race::TerranDescendant, BodyType::Organic, sl);
    assertTrue(ch.referenceMeshArchive == "human.zip", "Generated character carries archive path");
}

static void testCharacterUniformScale() {
    std::cout << "\n=== PCG: Character uniform scale ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;

    // Short character
    CharacterSliders shortSl;
    shortSl.height = 0.0f; // minimum
    auto shortCh = cms.generate(10, Race::TerranDescendant, BodyType::Organic, shortSl);

    // Tall character
    CharacterSliders tallSl;
    tallSl.height = 1.0f; // maximum
    auto tallCh = cms.generate(10, Race::TerranDescendant, BodyType::Organic, tallSl);

    assertTrue(shortCh.uniformScale > 0.0f, "Short character has positive scale");
    assertTrue(tallCh.uniformScale > shortCh.uniformScale, "Tall character has larger scale");
}

static void testCharacterMorphWeights() {
    std::cout << "\n=== PCG: Character morph weights ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    sl.height = 0.8f;
    sl.build = 0.3f;
    sl.limb_length = 0.6f;
    sl.torso_proportion = 0.7f;
    sl.head_shape = 0.2f;

    auto ch = cms.generate(55, Race::SynthBorn, BodyType::Organic, sl);
    assertTrue(ch.morphWeights.count("height") > 0, "height morph present");
    assertTrue(ch.morphWeights.count("build") > 0, "build morph present");
    assertTrue(ch.morphWeights.count("limb_length") > 0, "limb_length morph present");
    assertTrue(ch.morphWeights.count("torso_proportion") > 0, "torso_proportion morph present");
    assertTrue(ch.morphWeights.count("head_shape") > 0, "head_shape morph present");
    assertTrue(ch.morphWeights.at("height") >= 0.0f && ch.morphWeights.at("height") <= 1.0f,
               "height morph in [0,1]");
    assertTrue(ch.morphWeights.at("build") >= 0.0f && ch.morphWeights.at("build") <= 1.0f,
               "build morph in [0,1]");
}

static void testFleetDoctrineGeneration() {
    std::cout << "\n=== PCG: FleetDoctrine basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 999, 1 };

    auto fleet = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Brawler, 10);

    assertTrue(fleet.doctrine == atlas::pcg::FleetDoctrine::Brawler, "Doctrine stored");
    assertTrue(static_cast<int>(fleet.slots.size()) == 10, "Correct ship count");
    assertTrue(fleet.totalShips == 10, "totalShips matches");

    // At least one commander.
    bool hasCmd = false;
    for (const auto& s : fleet.slots) {
        if (s.role == atlas::pcg::FleetRole::Commander) hasCmd = true;
    }
    assertTrue(hasCmd, "Fleet has a commander");
}

static void testFleetDoctrineDeterminism() {
    std::cout << "\n=== PCG: FleetDoctrine determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 888, 1 };

    auto f1 = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Sniper, 8);
    auto f2 = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Sniper, 8);

    assertTrue(f1.slots.size() == f2.slots.size(), "Same fleet size");
    bool allMatch = true;
    for (size_t i = 0; i < f1.slots.size(); ++i) {
        if (f1.slots[i].role != f2.slots[i].role) { allMatch = false; break; }
        if (f1.slots[i].ship.mass != f2.slots[i].ship.mass) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Two fleets from same seed are identical");
}

static void testFleetDoctrineRoles() {
    std::cout << "\n=== PCG: FleetDoctrine role distribution ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 1234, 1 };

    auto fleet = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Logistics, 20);

    int logiCount = 0;
    for (const auto& s : fleet.slots) {
        if (s.role == atlas::pcg::FleetRole::Logistics) logiCount++;
    }
    // Logistics doctrine should have many logi ships (50% target).
    assertTrue(logiCount >= 5, "Logistics doctrine has ≥ 5 logi ships in 20-ship fleet");
}

static void testFleetDoctrineZeroShips() {
    std::cout << "\n=== PCG: FleetDoctrine zero ships ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 0, 1 };

    auto fleet = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Brawler, 0);
    assertTrue(fleet.slots.empty(), "Zero-ship fleet has no slots");
}

// ── Procedural Generation Systems tests ─────────────────────────────────

static void testRoomGraphGeneration() {
    std::cout << "\n=== PCG: RoomGraph generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 42, 1 };
    auto rooms = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    assertTrue(!rooms.empty(), "Rooms generated for deck");
    assertTrue(rooms.size() >= 2, "At least 2 rooms per deck");
    for (const auto& r : rooms) {
        assertTrue(r.dimX > 0 && r.dimY > 0 && r.dimZ > 0, "Room has positive dimensions");
    }
}

static void testRoomGraphDeterminism() {
    std::cout << "\n=== PCG: RoomGraph determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 42, 1 };
    auto rooms1 = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    auto rooms2 = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    assertTrue(rooms1.size() == rooms2.size(), "Same seed → same room count");
    bool allMatch = true;
    for (size_t i = 0; i < rooms1.size(); ++i) {
        if (rooms1[i].roomId != rooms2[i].roomId) { allMatch = false; break; }
        if (rooms1[i].type != rooms2[i].type) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical rooms");
}

static void testDeckGraphGeneration() {
    std::cout << "\n=== PCG: DeckGraph generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 100, 1 };
    auto decks = atlas::pcg::generateDeckStack(3, ctx);
    assertTrue(decks.size() >= 2, "At least 2 decks generated");
    for (size_t i = 0; i < decks.size(); ++i) {
        assertTrue(decks[i].index == static_cast<int>(i), "Deck index correct");
    }
}

static void testDeckGraphCorridors() {
    std::cout << "\n=== PCG: DeckGraph corridor connections ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 200, 1 };
    auto rooms = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    auto corridors = atlas::pcg::connectRooms(rooms);
    // Linear corridors: N-1.  Hub-and-spoke adds N-2 more if N >= 4.
    size_t expected = rooms.size() - 1;
    if (rooms.size() >= 4) expected += rooms.size() - 2;
    assertTrue(corridors.size() == expected, "Corridors match linear + hub-and-spoke count");
}

static void testElevatorGeneration() {
    std::cout << "\n=== PCG: ElevatorSystem generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 300, 1 };
    auto decks = atlas::pcg::generateDeckStack(3, ctx);
    assertTrue(decks.size() >= 2, "Have multiple decks for elevator test");
    auto elevator = atlas::pcg::generateElevator(decks[0], static_cast<int>(decks.size()), ctx);
    assertTrue(elevator.floorCount == static_cast<int>(decks.size()), "Elevator covers all floors");
    assertTrue(elevator.buttons.size() == decks.size(), "Button for each floor");
}

static void testHullMesherGeneration() {
    std::cout << "\n=== PCG: HullMesher generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 400, 1 };
    auto decks = atlas::pcg::generateDeckStack(2, ctx);
    for (auto& d : decks) {
        d.rooms = atlas::pcg::generateRoomsForDeck(d.index, ctx, 2);
    }
    auto hull = atlas::pcg::generateHullMesh(decks, 1.0f);
    assertTrue(!hull.vertices.empty(), "Hull has vertices");
    assertTrue(!hull.indices.empty(), "Hull has indices");
    assertTrue(hull.indices.size() % 3 == 0, "Hull indices are triangles");
}

static void testCapitalShipGeneratorDeterminism() {
    std::cout << "\n=== PCG: CapitalShipGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 500, 1 };
    auto ship1 = atlas::pcg::CapitalShipGenerator::generate(ctx);
    auto ship2 = atlas::pcg::CapitalShipGenerator::generate(ctx);
    assertTrue(ship1.shipClass == ship2.shipClass, "Same ship class");
    assertTrue(ship1.decks.size() == ship2.decks.size(), "Same deck count");
    assertTrue(ship1.valid == ship2.valid, "Same validity");
}

static void testCapitalShipGeneratorClassOverride() {
    std::cout << "\n=== PCG: CapitalShipGenerator class override ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 600, 1 };
    auto ship = atlas::pcg::CapitalShipGenerator::generate(ctx, 5);
    assertTrue(ship.shipClass == 5, "Ship class override applied");
    assertTrue(ship.decks.size() >= 2, "Capital has multiple decks");
    assertTrue(!ship.hull.vertices.empty(), "Capital has hull mesh");
}

static void testCapitalShipGeneratorValidity() {
    std::cout << "\n=== PCG: CapitalShipGenerator validity ===" << std::endl;
    bool allValid = true;
    for (uint64_t i = 1; i <= 50; ++i) {
        atlas::pcg::PCGContext ctx{ i * 71, 1 };
        auto ship = atlas::pcg::CapitalShipGenerator::generate(ctx);
        if (!ship.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All 50 capital ships are valid");
}

static void testShipDesignerOverride() {
    std::cout << "\n=== PCG: ShipDesigner room override ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 700, 1 };
    auto decks = atlas::pcg::generateDeckStack(2, ctx);
    for (auto& d : decks) {
        d.rooms = atlas::pcg::generateRoomsForDeck(d.index, ctx, 2);
    }
    int targetRoom = decks[0].rooms[0].roomId;
    atlas::pcg::ShipDesignerSave save;
    save.pcgVersion = 1;
    save.shipClass = 2;
    save.seed = 700;
    save.roomOverrides.push_back({ targetRoom, atlas::pcg::RoomType::Reactor });

    atlas::pcg::applyDesignerOverrides(decks, save);
    assertTrue(decks[0].rooms[0].type == atlas::pcg::RoomType::Reactor, "Room type overridden");
}

static void testShipDesignerSaveLoad() {
    std::cout << "\n=== PCG: ShipDesigner save/load ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 800, 1 };
    auto decks = atlas::pcg::generateDeckStack(2, ctx);
    for (auto& d : decks) {
        d.rooms = atlas::pcg::generateRoomsForDeck(d.index, ctx, 2);
    }
    auto save = atlas::pcg::saveShipLayout(decks, 2, 800);
    assertTrue(save.pcgVersion == 1, "Save version set");
    assertTrue(save.shipClass == 2, "Save class stored");
    assertTrue(save.seed == 800, "Save seed stored");
}

static void testSnappableGridCreation() {
    std::cout << "\n=== PCG: SnappableGrid creation ===" << std::endl;
    atlas::pcg::SnappableGrid grid(10, 10, 10, 5.0f);
    assertTrue(grid.width() == 10, "Grid width correct");
    assertTrue(grid.height() == 10, "Grid height correct");
    assertTrue(grid.depth() == 10, "Grid depth correct");
    assertTrue(std::abs(grid.cellSize() - 5.0f) < 0.001f, "Grid cell size correct");
}

static void testSnappableGridPlacement() {
    std::cout << "\n=== PCG: SnappableGrid placement ===" << std::endl;
    atlas::pcg::SnappableGrid grid(10, 10, 10, 5.0f);
    assertTrue(grid.placeContent(0, 0, 0, 1), "Place content at (0,0,0)");
    assertTrue(!grid.placeContent(0, 0, 0, 2), "Cannot overwrite occupied cell");
    auto* cell = grid.getCell(0, 0, 0);
    assertTrue(cell != nullptr, "Cell exists");
    assertTrue(cell->occupied, "Cell is occupied");
    assertTrue(cell->contentType == 1, "Content type correct");
    assertTrue(grid.removeContent(0, 0, 0), "Remove content");
    assertTrue(!cell->occupied, "Cell cleared after removal");
}

static void testSnappableGridSectorGeneration() {
    std::cout << "\n=== PCG: SnappableGrid sector generation ===" << std::endl;
    atlas::pcg::SnappableGrid grid(20, 20, 20, 5.0f);
    atlas::pcg::PCGContext ctx{ 1234, 1 };
    grid.generateSector(ctx);
    int occupied = 0;
    for (int x = 0; x < 20; ++x)
        for (int y = 0; y < 20; ++y)
            for (int z = 0; z < 20; ++z) {
                auto* c = grid.getCell(x, y, z);
                if (c && c->occupied) ++occupied;
            }
    assertTrue(occupied > 0, "Sector generation placed content");
    assertTrue(occupied < 20*20*20, "Not all cells occupied");
}

static void testStationGeneratorBasic() {
    std::cout << "\n=== PCG: StationGenerator basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 2000, 1 };
    auto station = atlas::pcg::StationGenerator::generate(ctx);
    assertTrue(!station.modules.empty(), "Station has modules");
    assertTrue(station.modules.size() >= 3, "Station has >= 3 modules");
    assertTrue(station.valid, "Station is valid");
}

static void testStationGeneratorDeterminism() {
    std::cout << "\n=== PCG: StationGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 2100, 1 };
    auto s1 = atlas::pcg::StationGenerator::generate(ctx);
    auto s2 = atlas::pcg::StationGenerator::generate(ctx);
    assertTrue(s1.modules.size() == s2.modules.size(), "Same module count");
    bool allMatch = true;
    for (size_t i = 0; i < s1.modules.size(); ++i) {
        if (s1.modules[i].type != s2.modules[i].type) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical stations");
}

static void testStationGeneratorPower() {
    std::cout << "\n=== PCG: StationGenerator power balance ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 2200, 1 };
    auto station = atlas::pcg::StationGenerator::generate(ctx);
    atlas::pcg::StationGenerator::recalculatePower(station);
    assertTrue(station.totalPowerProduction >= station.totalPowerConsumption,
               "Station power production >= consumption");
}

static void testSalvageFieldGeneration() {
    std::cout << "\n=== PCG: SalvageSystem field generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 3000, 1 };
    auto field = atlas::pcg::SalvageSystem::generateSalvageField(ctx, 20);
    assertTrue(field.totalNodes == 20, "Correct node count");
    assertTrue(static_cast<int>(field.nodes.size()) == 20, "Nodes vector matches");
    float totalValue = atlas::pcg::SalvageSystem::calculateTotalValue(field);
    assertTrue(totalValue > 0.0f, "Salvage has positive value");
}

static void testSalvageFieldDeterminism() {
    std::cout << "\n=== PCG: SalvageSystem determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 3100, 1 };
    auto f1 = atlas::pcg::SalvageSystem::generateSalvageField(ctx, 15);
    auto f2 = atlas::pcg::SalvageSystem::generateSalvageField(ctx, 15);
    bool allMatch = true;
    for (size_t i = 0; i < f1.nodes.size(); ++i) {
        if (f1.nodes[i].category != f2.nodes[i].category) { allMatch = false; break; }
        if (f1.nodes[i].value != f2.nodes[i].value) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical salvage field");
}

static void testSalvageFieldHiddenNodes() {
    std::cout << "\n=== PCG: SalvageSystem hidden nodes ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 3200, 1 };
    auto field = atlas::pcg::SalvageSystem::generateSalvageField(ctx, 100);
    assertTrue(field.hiddenNodes > 0, "Some nodes are hidden (require scan)");
    assertTrue(field.hiddenNodes < 100, "Not all nodes are hidden");
}

static void testRoverGeneration() {
    std::cout << "\n=== PCG: RoverSystem generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 4000, 1 };
    auto rover = atlas::pcg::RoverSystem::generate(ctx);
    assertTrue(rover.valid, "Rover is valid");
    assertTrue(!rover.modules.empty(), "Rover has modules");
    assertTrue(rover.maxSpeed > 0.0f, "Rover has positive speed");
    assertTrue(rover.mass > 0.0f, "Rover has positive mass");
}

static void testRoverDeployDock() {
    std::cout << "\n=== PCG: RoverSystem deploy/dock ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 4100, 1 };
    auto rover = atlas::pcg::RoverSystem::generate(ctx);
    assertTrue(!rover.deployed, "Rover starts docked");
    assertTrue(atlas::pcg::RoverSystem::deploy(rover, 10.0f, 0.0f, 5.0f), "Deploy succeeded");
    assertTrue(rover.deployed, "Rover is deployed");
    assertTrue(!atlas::pcg::RoverSystem::deploy(rover, 0, 0, 0), "Cannot deploy when already deployed");
    assertTrue(atlas::pcg::RoverSystem::dock(rover), "Dock succeeded");
    assertTrue(!rover.deployed, "Rover is docked");
}

static void testRoverCargoCapacity() {
    std::cout << "\n=== PCG: RoverSystem cargo capacity ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 4200, 1 };
    auto rover = atlas::pcg::RoverSystem::generate(ctx);
    int cargo = atlas::pcg::RoverSystem::totalCargoCapacity(rover);
    assertTrue(cargo > 0, "Rover has cargo capacity");
}

static void testCollisionManagerBasic() {
    std::cout << "\n=== PCG: CollisionManager basic operations ===" << std::endl;
    atlas::pcg::CollisionManager mgr;
    assertTrue(mgr.volumeCount() == 0, "Starts empty");

    atlas::pcg::AABB box = atlas::pcg::CollisionManager::computeRoomAABB(0, 0, 0, 4, 4, 4);
    atlas::pcg::CollisionVolume vol{ 1, box, true, true };
    mgr.addVolume(vol);
    assertTrue(mgr.volumeCount() == 1, "One volume after add");
    assertTrue(mgr.testPoint(0, 0, 0), "Point inside volume detected");
    assertTrue(!mgr.testPoint(10, 10, 10), "Point outside volume not detected");
}

static void testCollisionManagerAABBQuery() {
    std::cout << "\n=== PCG: CollisionManager AABB query ===" << std::endl;
    atlas::pcg::CollisionManager mgr;
    atlas::pcg::AABB box1 = atlas::pcg::CollisionManager::computeRoomAABB(0, 0, 0, 4, 4, 4);
    atlas::pcg::AABB box2 = atlas::pcg::CollisionManager::computeRoomAABB(10, 0, 0, 4, 4, 4);
    mgr.addVolume({ 1, box1, true, true });
    mgr.addVolume({ 2, box2, true, true });

    atlas::pcg::AABB query = { -1, -1, -1, 1, 1, 1 };
    auto results = mgr.queryRegion(query);
    assertTrue(results.size() == 1, "Query finds one overlapping volume");
    assertTrue(results[0] == 1, "Correct volume found");
}

static void testCollisionManagerRemove() {
    std::cout << "\n=== PCG: CollisionManager remove ===" << std::endl;
    atlas::pcg::CollisionManager mgr;
    atlas::pcg::AABB box = atlas::pcg::CollisionManager::computeRoomAABB(0, 0, 0, 4, 4, 4);
    mgr.addVolume({ 1, box, true, true });
    mgr.removeVolume(1);
    assertTrue(mgr.volumeCount() == 0, "Volume removed");
    assertTrue(!mgr.testPoint(0, 0, 0), "Point no longer detected after removal");
}


void run_character_mesh_cybernetics_tests() {
    testCharacterBodyTypeOrganic();
    testCharacterBodyTypeAugmented();
    testCharacterBodyTypeCybernetic();
    testCharacterBodyTypeFullSynth();
    testCharacterBodyTypeMechFrame();
    testCharacterBodyTypeNames();
    testCharacterBackwardCompatibility();
    testCharacterReferenceMeshArchive();
    testCharacterUniformScale();
    testCharacterMorphWeights();
    testFleetDoctrineGeneration();
    testFleetDoctrineDeterminism();
    testFleetDoctrineRoles();
    testFleetDoctrineZeroShips();
    testRoomGraphGeneration();
    testRoomGraphDeterminism();
    testDeckGraphGeneration();
    testDeckGraphCorridors();
    testElevatorGeneration();
    testHullMesherGeneration();
    testCapitalShipGeneratorDeterminism();
    testCapitalShipGeneratorClassOverride();
    testCapitalShipGeneratorValidity();
    testShipDesignerOverride();
    testShipDesignerSaveLoad();
    testSnappableGridCreation();
    testSnappableGridPlacement();
    testSnappableGridSectorGeneration();
    testStationGeneratorBasic();
    testStationGeneratorDeterminism();
    testStationGeneratorPower();
    testSalvageFieldGeneration();
    testSalvageFieldDeterminism();
    testSalvageFieldHiddenNodes();
    testRoverGeneration();
    testRoverDeployDock();
    testRoverCargoCapacity();
    testCollisionManagerBasic();
    testCollisionManagerAABBQuery();
    testCollisionManagerRemove();
}
