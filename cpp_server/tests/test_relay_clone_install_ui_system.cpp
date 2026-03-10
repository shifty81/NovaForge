// Tests for: RelayCloneInstallUiSystem
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/relay_clone_install_ui_system.h"

using namespace atlas;
using UiStep = components::RelayCloneInstallUiState::UiStep;

// ==================== RelayCloneInstallUiSystem Tests ====================

static void testRelayCloneUiInit() {
    std::cout << "\n=== RelayCloneInstallUi: Init ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    assertTrue(sys.initialize("char1", "pilot_alpha"), "Init succeeds");
    assertTrue(!sys.isPanelOpen("char1"), "Panel starts closed");
    assertTrue(sys.getCurrentStep("char1") == UiStep::Idle, "Starts in Idle");
    assertTrue(sys.getAvailableStationCount("char1") == 0, "0 stations");
    assertTrue(sys.getInstalledCloneCount("char1") == 0, "0 installed clones");
    assertTrue(sys.getTotalInstalls("char1") == 0, "0 installs");
    assertTrue(sys.getTotalCancels("char1") == 0, "0 cancels");

    // Verify character_id stored; defaults to entity_id when empty
    world.createEntity("char2");
    sys.initialize("char2", "");
    auto* e2 = world.getEntity("char2");
    auto* comp2 = e2->getComponent<components::RelayCloneInstallUiState>();
    assertTrue(comp2->character_id == "char2", "character_id defaults to entity_id");
    world.createEntity("char3");
    sys.initialize("char3", "override_id");
    auto* e3 = world.getEntity("char3");
    auto* comp3 = e3->getComponent<components::RelayCloneInstallUiState>();
    assertTrue(comp3->character_id == "override_id", "character_id stored when provided");
}

static void testRelayCloneUiOpenClose() {
    std::cout << "\n=== RelayCloneInstallUi: OpenClose ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");

    assertTrue(sys.openPanel("char1"), "Open panel");
    assertTrue(sys.isPanelOpen("char1"), "Panel is open");
    assertTrue(sys.getCurrentStep("char1") == UiStep::SelectStation, "Step = SelectStation");

    assertTrue(sys.closePanel("char1"), "Close panel");
    assertTrue(!sys.isPanelOpen("char1"), "Panel is closed");
    assertTrue(sys.getCurrentStep("char1") == UiStep::Idle, "Step = Idle");
}

static void testRelayCloneUiAddStation() {
    std::cout << "\n=== RelayCloneInstallUi: AddStation ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");

    assertTrue(sys.addStation("char1", "st_jita", "Jita 4-4", "The Forge", 50000.0f), "Add Jita");
    assertTrue(sys.addStation("char1", "st_amarr", "Amarr VIII", "Domain", 50000.0f), "Add Amarr");
    assertTrue(sys.getAvailableStationCount("char1") == 2, "2 stations");

    // Duplicate rejected
    assertTrue(!sys.addStation("char1", "st_jita", "Jita 4-4", "The Forge", 60000.0f), "Dup rejected");
    assertTrue(sys.getAvailableStationCount("char1") == 2, "Still 2 after dup");

    // Empty ID rejected
    assertTrue(!sys.addStation("char1", "", "Unknown", "Unknown", 0.0f), "Empty ID rejected");
}

static void testRelayCloneUiRemoveStation() {
    std::cout << "\n=== RelayCloneInstallUi: RemoveStation ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.addStation("char1", "st_jita", "Jita 4-4", "The Forge", 50000.0f);
    sys.addStation("char1", "st_amarr", "Amarr VIII", "Domain", 50000.0f);

    assertTrue(sys.removeStation("char1", "st_jita"), "Remove Jita");
    assertTrue(sys.getAvailableStationCount("char1") == 1, "1 station left");
    assertTrue(!sys.removeStation("char1", "st_jita"), "Double remove fails");
    assertTrue(!sys.removeStation("char1", "nonexistent"), "Missing remove fails");
}

static void testRelayCloneUiSearchFilter() {
    std::cout << "\n=== RelayCloneInstallUi: SearchFilter ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.addStation("char1", "st_jita", "Jita 4-4", "The Forge", 50000.0f);
    sys.addStation("char1", "st_amarr", "Amarr VIII", "Domain", 50000.0f);
    sys.addStation("char1", "st_dodixie", "Dodixie IX", "Sinq Laison", 50000.0f);

    // No filter: all stations visible
    assertTrue(sys.getFilteredStationCount("char1") == 3, "3 with no filter");

    // Filter by name substring
    assertTrue(sys.setSearchFilter("char1", "amarr"), "Set filter 'amarr'");
    assertTrue(sys.getFilteredStationCount("char1") == 1, "1 match 'amarr'");

    // Filter by region
    assertTrue(sys.setSearchFilter("char1", "forge"), "Set filter 'forge'");
    assertTrue(sys.getFilteredStationCount("char1") == 1, "1 match 'forge'");

    // Clear filter
    assertTrue(sys.setSearchFilter("char1", ""), "Clear filter");
    assertTrue(sys.getFilteredStationCount("char1") == 3, "3 after clear");

    assertTrue(sys.getSearchFilter("char1").empty(), "Filter is empty string");
}

static void testRelayCloneUiFullFlow() {
    std::cout << "\n=== RelayCloneInstallUi: FullFlow ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.addStation("char1", "st_jita", "Jita 4-4", "The Forge", 75000.0f);

    sys.openPanel("char1");
    assertTrue(sys.getCurrentStep("char1") == UiStep::SelectStation, "Step: SelectStation");

    // Select station
    assertTrue(sys.selectStation("char1", "st_jita"), "Select Jita");
    assertTrue(sys.getCurrentStep("char1") == UiStep::ConfirmCost, "Step: ConfirmCost");
    assertTrue(sys.getSelectedStationId("char1") == "st_jita", "Selected station is Jita");
    assertTrue(approxEqual(sys.getPendingCost("char1"), 75000.0f), "Cost is 75000");

    // Confirm
    assertTrue(sys.confirmInstall("char1"), "Confirm install");
    assertTrue(sys.getCurrentStep("char1") == UiStep::Pending, "Step: Pending");

    // Server acknowledges success
    assertTrue(sys.acknowledgeSuccess("char1", "clone_001"), "Acknowledge success");
    assertTrue(sys.getCurrentStep("char1") == UiStep::Success, "Step: Success");
    assertTrue(sys.getTotalInstalls("char1") == 1, "1 total install");
    assertTrue(sys.getInstalledCloneCount("char1") == 1, "1 installed clone");
}

static void testRelayCloneUiErrorFlow() {
    std::cout << "\n=== RelayCloneInstallUi: ErrorFlow ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.addStation("char1", "st_jita", "Jita 4-4", "The Forge", 75000.0f);

    sys.openPanel("char1");
    sys.selectStation("char1", "st_jita");
    sys.confirmInstall("char1");

    // Server returns error
    assertTrue(sys.acknowledgeError("char1", "Insufficient funds"), "Acknowledge error");
    assertTrue(sys.getCurrentStep("char1") == UiStep::Error, "Step: Error");
    assertTrue(sys.getLastError("char1") == "Insufficient funds", "Error message correct");
    assertTrue(sys.getTotalInstalls("char1") == 0, "0 installs on error");
}

static void testRelayCloneUiCancelFlow() {
    std::cout << "\n=== RelayCloneInstallUi: CancelFlow ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.addStation("char1", "st_jita", "Jita 4-4", "The Forge", 75000.0f);

    sys.openPanel("char1");
    sys.selectStation("char1", "st_jita");
    assertTrue(sys.getCurrentStep("char1") == UiStep::ConfirmCost, "At confirm step");

    // Cancel from ConfirmCost goes back to SelectStation
    assertTrue(sys.cancelInstall("char1"), "Cancel from ConfirmCost");
    assertTrue(sys.getCurrentStep("char1") == UiStep::SelectStation, "Back to SelectStation");
    assertTrue(sys.getTotalCancels("char1") == 1, "1 cancel recorded");
    assertTrue(sys.getSelectedStationId("char1").empty(), "Selection cleared");
}

static void testRelayCloneUiCancelInPending() {
    std::cout << "\n=== RelayCloneInstallUi: CancelInPending ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.addStation("char1", "st_jita", "Jita 4-4", "The Forge", 50000.0f);

    sys.openPanel("char1");
    sys.selectStation("char1", "st_jita");
    sys.confirmInstall("char1");

    // Cannot cancel while Pending (request in flight)
    assertTrue(!sys.cancelInstall("char1"), "Cancel blocked while Pending");
    assertTrue(sys.getCurrentStep("char1") == UiStep::Pending, "Still Pending");
}

static void testRelayCloneUiTimeout() {
    std::cout << "\n=== RelayCloneInstallUi: Timeout ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");
    sys.addStation("char1", "st_jita", "Jita 4-4", "The Forge", 50000.0f);

    sys.openPanel("char1");
    sys.selectStation("char1", "st_jita");
    sys.confirmInstall("char1");
    assertTrue(sys.getCurrentStep("char1") == UiStep::Pending, "Pending before timeout");

    // Tick past timeout (default 10s)
    sys.update(11.0f);
    assertTrue(sys.getCurrentStep("char1") == UiStep::Error, "Timeout causes Error");
    assertTrue(sys.getLastError("char1") == "Request timed out", "Timeout message set");
}

static void testRelayCloneUiSelectUnknownStation() {
    std::cout << "\n=== RelayCloneInstallUi: SelectUnknownStation ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);
    world.createEntity("char1");
    sys.initialize("char1");

    sys.openPanel("char1");
    assertTrue(!sys.selectStation("char1", "nonexistent_station"), "Select nonexistent fails");
    assertTrue(sys.getCurrentStep("char1") == UiStep::SelectStation, "Still SelectStation");
}

static void testRelayCloneUiMissing() {
    std::cout << "\n=== RelayCloneInstallUi: Missing ===" << std::endl;
    ecs::World world;
    systems::RelayCloneInstallUiSystem sys(&world);

    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.openPanel("nonexistent"), "OpenPanel fails");
    assertTrue(!sys.closePanel("nonexistent"), "ClosePanel fails");
    assertTrue(!sys.isPanelOpen("nonexistent"), "IsPanelOpen false");
    assertTrue(!sys.addStation("nonexistent", "s1", "S", "R", 100.0f), "AddStation fails");
    assertTrue(!sys.removeStation("nonexistent", "s1"), "RemoveStation fails");
    assertTrue(sys.getAvailableStationCount("nonexistent") == 0, "0 stations");
    assertTrue(!sys.setSearchFilter("nonexistent", "jita"), "SetFilter fails");
    assertTrue(sys.getSearchFilter("nonexistent").empty(), "Empty filter on missing");
    assertTrue(sys.getFilteredStationCount("nonexistent") == 0, "0 filtered");
    assertTrue(!sys.selectStation("nonexistent", "s1"), "SelectStation fails");
    assertTrue(!sys.confirmInstall("nonexistent"), "Confirm fails");
    assertTrue(!sys.cancelInstall("nonexistent"), "Cancel fails");
    assertTrue(!sys.acknowledgeSuccess("nonexistent", "c1"), "AckSuccess fails");
    assertTrue(!sys.acknowledgeError("nonexistent", "err"), "AckError fails");
    assertTrue(sys.getInstalledCloneCount("nonexistent") == 0, "0 clones");
    assertTrue(sys.getCurrentStep("nonexistent") == UiStep::Idle, "Idle on missing");
    assertTrue(sys.getTotalInstalls("nonexistent") == 0, "0 installs");
    assertTrue(sys.getTotalCancels("nonexistent") == 0, "0 cancels");
}

void run_relay_clone_install_ui_system_tests() {
    testRelayCloneUiInit();
    testRelayCloneUiOpenClose();
    testRelayCloneUiAddStation();
    testRelayCloneUiRemoveStation();
    testRelayCloneUiSearchFilter();
    testRelayCloneUiFullFlow();
    testRelayCloneUiErrorFlow();
    testRelayCloneUiCancelFlow();
    testRelayCloneUiCancelInPending();
    testRelayCloneUiTimeout();
    testRelayCloneUiSelectUnknownStation();
    testRelayCloneUiMissing();
}
