// Tests for: RespawnSelectionSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/respawn_selection_system.h"

using namespace atlas;

// ==================== RespawnSelectionSystem Tests ====================

static void testRespawnSelectionInit() {
    std::cout << "\n=== RespawnSelection: Init ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1"), "Init succeeds");
    assertTrue(!sys.isOpen("p1"), "Panel closed initially");
    assertTrue(sys.getSelectedLocation("p1").empty(), "No location selected initially");
    assertTrue(sys.getLocationCount("p1") == 0, "No locations initially");
    assertTrue(sys.getTotalSelections("p1") == 0, "Zero selections initially");
}

static void testRespawnSelectionInitFails() {
    std::cout << "\n=== RespawnSelection: InitFails ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testRespawnSelectionOpenClose() {
    std::cout << "\n=== RespawnSelection: OpenClose ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addLocation("p1", "station_alpha", "Station Alpha", 0.5f, true);

    assertTrue(sys.openSelection("p1"), "Open panel succeeds");
    assertTrue(sys.isOpen("p1"), "Panel is open");
    assertTrue(sys.selectLocation("p1", "station_alpha"), "Select location succeeds");
    assertTrue(sys.getSelectedLocation("p1") == "station_alpha", "Location selected");
    assertTrue(sys.confirmSelection("p1"), "Confirm succeeds with selection");
    assertTrue(!sys.isOpen("p1"), "Panel closed after confirm");
    assertTrue(sys.getTotalSelections("p1") == 1, "Selection counter incremented");
}

static void testRespawnSelectionOpenRejectDouble() {
    std::cout << "\n=== RespawnSelection: OpenRejectDouble ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.openSelection("p1"), "First open succeeds");
    assertTrue(!sys.openSelection("p1"), "Second open rejected while open");
    assertTrue(sys.isOpen("p1"), "Panel still open");
}

static void testRespawnSelectionAddLocation() {
    std::cout << "\n=== RespawnSelection: AddLocation ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.addLocation("p1", "loc1", "Location 1", 1.0f, false), "Add first location");
    assertTrue(sys.addLocation("p1", "loc2", "Location 2", 2.5f, true), "Add second location");
    assertTrue(sys.getLocationCount("p1") == 2, "Two locations stored");
    assertTrue(!sys.addLocation("p1", "loc1", "Duplicate", 0.0f, false), "Duplicate rejected");
    assertTrue(!sys.addLocation("p1", "", "Empty ID", 0.0f, false), "Empty ID rejected");
    assertTrue(sys.getLocationCount("p1") == 2, "Count unchanged after rejections");
}

static void testRespawnSelectionRemoveLocation() {
    std::cout << "\n=== RespawnSelection: RemoveLocation ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addLocation("p1", "loc1", "Location 1", 1.0f, false);
    sys.addLocation("p1", "loc2", "Location 2", 2.0f, false);

    assertTrue(sys.removeLocation("p1", "loc1"), "Remove existing location");
    assertTrue(sys.getLocationCount("p1") == 1, "Count decremented");
    assertTrue(!sys.removeLocation("p1", "loc1"), "Remove nonexistent fails");
    assertTrue(!sys.removeLocation("p1", "nonexistent"), "Remove unknown fails");
}

static void testRespawnSelectionSelectValidation() {
    std::cout << "\n=== RespawnSelection: SelectValidation ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addLocation("p1", "loc1", "Location 1", 1.0f, false);

    // Cannot select while panel is closed
    assertTrue(!sys.selectLocation("p1", "loc1"), "Select fails when panel closed");
    sys.openSelection("p1");
    assertTrue(!sys.selectLocation("p1", "unknown"), "Select nonexistent fails");
    assertTrue(sys.selectLocation("p1", "loc1"), "Select known location succeeds");
    assertTrue(sys.getSelectedLocation("p1") == "loc1", "Location stored");
}

static void testRespawnSelectionConfirmNoLocation() {
    std::cout << "\n=== RespawnSelection: ConfirmNoLocation ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.openSelection("p1");

    assertTrue(!sys.confirmSelection("p1"), "Confirm fails without selection");
    assertTrue(sys.isOpen("p1"), "Panel still open after failed confirm");
    assertTrue(sys.getTotalSelections("p1") == 0, "Counter unchanged");
}

static void testRespawnSelectionAutoSelect() {
    std::cout << "\n=== RespawnSelection: AutoSelect ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addLocation("p1", "home", "Home Station", 0.0f, true);
    sys.setAutoSelectDuration("p1", 5.0f);
    sys.openSelection("p1");

    sys.update(3.0f);
    assertTrue(sys.isOpen("p1"), "Still open after 3s");

    sys.update(3.0f);
    assertTrue(!sys.isOpen("p1"), "Auto-closed after timeout");
    assertTrue(sys.getSelectedLocation("p1") == "home", "Default location auto-selected");
    assertTrue(sys.getTotalSelections("p1") == 1, "Selection counted");
}

static void testRespawnSelectionSetAutoSelectDuration() {
    std::cout << "\n=== RespawnSelection: SetAutoSelectDuration ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.setAutoSelectDuration("p1", 60.0f), "Set duration to 60s");
    auto* comp = world.getEntity("p1")->getComponent<components::RespawnSelection>();
    assertTrue(approxEqual(comp->auto_select_duration, 60.0f), "Duration stored");
    assertTrue(!sys.setAutoSelectDuration("p1", -1.0f), "Negative duration rejected");
    assertTrue(!sys.setAutoSelectDuration("nonexistent", 10.0f), "Fails on missing entity");
}

static void testRespawnSelectionMissing() {
    std::cout << "\n=== RespawnSelection: Missing ===" << std::endl;
    ecs::World world;
    systems::RespawnSelectionSystem sys(&world);

    assertTrue(!sys.openSelection("nonexistent"), "Open fails on missing");
    assertTrue(!sys.confirmSelection("nonexistent"), "Confirm fails on missing");
    assertTrue(!sys.addLocation("nonexistent", "l", "L", 0.0f, false), "AddLocation fails");
    assertTrue(!sys.removeLocation("nonexistent", "l"), "RemoveLocation fails");
    assertTrue(!sys.selectLocation("nonexistent", "l"), "SelectLocation fails");
    assertTrue(!sys.isOpen("nonexistent"), "Not open on missing");
    assertTrue(sys.getSelectedLocation("nonexistent").empty(), "Empty location on missing");
    assertTrue(sys.getLocationCount("nonexistent") == 0, "Zero count on missing");
    assertTrue(sys.getTotalSelections("nonexistent") == 0, "Zero selections on missing");
}

void run_respawn_selection_system_tests() {
    testRespawnSelectionInit();
    testRespawnSelectionInitFails();
    testRespawnSelectionOpenClose();
    testRespawnSelectionOpenRejectDouble();
    testRespawnSelectionAddLocation();
    testRespawnSelectionRemoveLocation();
    testRespawnSelectionSelectValidation();
    testRespawnSelectionConfirmNoLocation();
    testRespawnSelectionAutoSelect();
    testRespawnSelectionSetAutoSelectDuration();
    testRespawnSelectionMissing();
}
