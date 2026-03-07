// Tests for: Fleet Command Terminal System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/fleet_command_terminal_system.h"

using namespace atlas;

// ==================== Fleet Command Terminal System Tests ====================

static void testFleetCommandTerminalCreate() {
    std::cout << "\n=== FleetCommandTerminal: Create ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    assertTrue(sys.initialize("term1", "player1", "station_alpha"), "Init succeeds");
    assertTrue(sys.getState("term1") == "Offline", "Initial state is Offline");
    assertTrue(sys.getActiveUser("term1").empty(), "No active user");
    assertTrue(approxEqual(sys.getIntegrity("term1"), 100.0f), "Full integrity");
    assertTrue(sys.getTotalOrdersIssued("term1") == 0, "No orders issued");
    assertTrue(sys.getTotalSessions("term1") == 0, "No sessions");
    assertTrue(sys.getOrderHistoryCount("term1") == 0, "No order history");
    assertTrue(!sys.isOperational("term1"), "Not operational (not placed)");
}

static void testFleetCommandTerminalPlaceAndBoot() {
    std::cout << "\n=== FleetCommandTerminal: PlaceAndBoot ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");

    assertTrue(!sys.powerOn("term1"), "Can't power on unplaced terminal");
    assertTrue(sys.placeTerminal("term1"), "Place terminal");
    assertTrue(!sys.placeTerminal("term1"), "Can't double-place");
    assertTrue(sys.powerOn("term1"), "Power on");
    assertTrue(sys.getState("term1") == "Booting", "State is Booting");
    assertTrue(approxEqual(sys.getBootProgress("term1"), 0.0f), "Boot progress 0");
}

static void testFleetCommandTerminalBootSequence() {
    std::cout << "\n=== FleetCommandTerminal: BootSequence ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");
    sys.placeTerminal("term1");
    sys.powerOn("term1");

    // Default boot_time is 3s
    sys.update(1.5f);
    assertTrue(sys.getState("term1") == "Booting", "Still booting at 1.5s");
    assertTrue(sys.getBootProgress("term1") > 0.4f, "Boot progress > 40%");

    sys.update(2.0f); // total 3.5s
    assertTrue(sys.getState("term1") == "Idle", "Idle after boot complete");
    assertTrue(approxEqual(sys.getBootProgress("term1"), 1.0f), "Boot progress 100%");
    assertTrue(sys.isOperational("term1"), "Now operational");
}

static void testFleetCommandTerminalLoginLogout() {
    std::cout << "\n=== FleetCommandTerminal: LoginLogout ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");
    sys.placeTerminal("term1");
    sys.powerOn("term1");
    sys.update(5.0f); // boot complete

    assertTrue(sys.loginUser("term1", "player1"), "Login succeeds");
    assertTrue(sys.getActiveUser("term1") == "player1", "Active user is player1");
    assertTrue(sys.getState("term1") == "Active", "State is Active");
    assertTrue(sys.getTotalSessions("term1") == 1, "1 session");

    // Can't login another user while occupied
    assertTrue(!sys.loginUser("term1", "player2"), "Can't login while occupied");

    assertTrue(sys.logoutUser("term1"), "Logout succeeds");
    assertTrue(sys.getActiveUser("term1").empty(), "No active user");
    assertTrue(sys.getState("term1") == "Idle", "Back to Idle after logout");
}

static void testFleetCommandTerminalCommandMode() {
    std::cout << "\n=== FleetCommandTerminal: CommandMode ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");
    sys.placeTerminal("term1");
    sys.powerOn("term1");
    sys.update(5.0f);
    sys.loginUser("term1", "player1");

    assertTrue(sys.enterCommandMode("term1", "fleet_alpha"), "Enter command mode");
    assertTrue(sys.getState("term1") == "CommandMode", "State is CommandMode");

    assertTrue(sys.exitCommandMode("term1"), "Exit command mode");
    assertTrue(sys.getState("term1") == "Active", "Back to Active");
}

static void testFleetCommandTerminalIssueOrders() {
    std::cout << "\n=== FleetCommandTerminal: IssueOrders ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");
    sys.placeTerminal("term1");
    sys.powerOn("term1");
    sys.update(5.0f);
    sys.loginUser("term1", "player1");
    sys.enterCommandMode("term1", "fleet_alpha");

    // Issue Engage order (enum value 2)
    assertTrue(sys.issueOrder("term1", 2, "target_enemy1"), "Issue Engage order");
    assertTrue(sys.getCurrentOrder("term1") == "Engage", "Current order is Engage");
    assertTrue(sys.getState("term1") == "Cooldown", "State is Cooldown after order");
    assertTrue(sys.getTotalOrdersIssued("term1") == 1, "1 order issued");
    assertTrue(sys.getOrderHistoryCount("term1") == 1, "1 order in history");

    // Can't issue during cooldown
    assertTrue(!sys.issueOrder("term1", 3, "target2"), "Can't issue during cooldown");
}

static void testFleetCommandTerminalCooldown() {
    std::cout << "\n=== FleetCommandTerminal: Cooldown ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");
    sys.placeTerminal("term1");
    sys.powerOn("term1");
    sys.update(5.0f);
    sys.loginUser("term1", "player1");
    sys.enterCommandMode("term1", "fleet_alpha");
    sys.issueOrder("term1", 1, ""); // Hold order

    sys.update(1.0f);
    assertTrue(sys.getState("term1") == "Cooldown", "Still in cooldown at 1s");

    sys.update(2.0f); // total 3s, past default 2s cooldown
    assertTrue(sys.getState("term1") == "CommandMode", "Back to CommandMode after cooldown");

    // Can issue another order now
    assertTrue(sys.issueOrder("term1", 4, ""), "Issue Retreat order after cooldown");
    assertTrue(sys.getCurrentOrder("term1") == "Retreat", "Current order is Retreat");
    assertTrue(sys.getTotalOrdersIssued("term1") == 2, "2 orders issued");
}

static void testFleetCommandTerminalDamage() {
    std::cout << "\n=== FleetCommandTerminal: Damage ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");
    sys.placeTerminal("term1");
    sys.powerOn("term1");
    sys.update(5.0f);
    sys.loginUser("term1", "player1");

    assertTrue(sys.damageTerminal("term1", 30.0f), "Damage terminal");
    assertTrue(approxEqual(sys.getIntegrity("term1"), 70.0f), "Integrity 70%");
    assertTrue(sys.isOperational("term1"), "Still operational at 70%");

    assertTrue(sys.damageTerminal("term1", 25.0f), "More damage");
    sys.update(0.1f); // trigger damage check
    assertTrue(approxEqual(sys.getIntegrity("term1"), 45.0f), "Integrity 45%");
    assertTrue(sys.getState("term1") == "Damaged", "State is Damaged below threshold");
    assertTrue(!sys.isOperational("term1"), "Not operational when damaged");
    assertTrue(sys.getActiveUser("term1").empty(), "User kicked when damaged");
}

static void testFleetCommandTerminalRepair() {
    std::cout << "\n=== FleetCommandTerminal: Repair ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");
    sys.placeTerminal("term1");
    sys.powerOn("term1");
    sys.update(5.0f);

    sys.damageTerminal("term1", 60.0f); // integrity = 40
    sys.update(0.1f); // triggers Damaged state
    assertTrue(sys.getState("term1") == "Damaged", "Damaged state");

    sys.repairTerminal("term1", 20.0f); // integrity = 60, above threshold
    assertTrue(approxEqual(sys.getIntegrity("term1"), 60.0f), "Integrity 60%");
    assertTrue(sys.getState("term1") == "Offline", "Repaired terminal goes Offline (needs reboot)");

    // Can reboot after repair
    assertTrue(sys.powerOn("term1"), "Can power on after repair");
    assertTrue(sys.getState("term1") == "Booting", "Rebooting after repair");
}

static void testFleetCommandTerminalFleetInfo() {
    std::cout << "\n=== FleetCommandTerminal: FleetInfo ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");

    assertTrue(sys.updateFleetInfo("term1", 12, 0.85f, 75.0f), "Update fleet info");
    assertTrue(sys.getFleetShipCount("term1") == 12, "12 ships");
    assertTrue(approxEqual(sys.getFleetReadiness("term1"), 0.85f), "Readiness 0.85");
    assertTrue(approxEqual(sys.getFleetMorale("term1"), 75.0f), "Morale 75");
}

static void testFleetCommandTerminalPowerOff() {
    std::cout << "\n=== FleetCommandTerminal: PowerOff ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");
    sys.placeTerminal("term1");
    sys.powerOn("term1");
    sys.update(5.0f);
    sys.loginUser("term1", "player1");

    assertTrue(sys.powerOff("term1"), "Power off");
    assertTrue(sys.getState("term1") == "Offline", "State is Offline");
    assertTrue(sys.getActiveUser("term1").empty(), "User disconnected on power off");
}

static void testFleetCommandTerminalOrderHistory() {
    std::cout << "\n=== FleetCommandTerminal: OrderHistory ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    world.createEntity("term1");
    sys.initialize("term1", "player1", "station_alpha");
    sys.placeTerminal("term1");
    sys.powerOn("term1");
    sys.update(5.0f);
    sys.loginUser("term1", "player1");
    sys.enterCommandMode("term1", "fleet_alpha");

    auto* entity = world.getEntity("term1");
    auto* comp = entity->getComponent<components::FleetCommandTerminal>();
    comp->max_orders_history = 3;
    comp->cooldown_time = 0.1f; // fast cooldown for testing

    sys.issueOrder("term1", 1, "");    // Hold
    sys.update(0.2f);
    sys.issueOrder("term1", 2, "e1");  // Engage
    sys.update(0.2f);
    sys.issueOrder("term1", 3, "e2");  // FocusFire
    sys.update(0.2f);
    sys.issueOrder("term1", 5, "");    // Regroup — evicts oldest
    assertTrue(sys.getOrderHistoryCount("term1") == 3, "History capped at 3");
    assertTrue(sys.getTotalOrdersIssued("term1") == 4, "4 total orders issued");
}

static void testFleetCommandTerminalMissing() {
    std::cout << "\n=== FleetCommandTerminal: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetCommandTerminalSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "p1", "s1"), "Init fails on missing");
    assertTrue(!sys.placeTerminal("nonexistent"), "Place fails on missing");
    assertTrue(!sys.powerOn("nonexistent"), "PowerOn fails on missing");
    assertTrue(!sys.powerOff("nonexistent"), "PowerOff fails on missing");
    assertTrue(!sys.loginUser("nonexistent", "p1"), "Login fails on missing");
    assertTrue(!sys.logoutUser("nonexistent"), "Logout fails on missing");
    assertTrue(!sys.enterCommandMode("nonexistent", "f1"), "EnterCmd fails on missing");
    assertTrue(!sys.exitCommandMode("nonexistent"), "ExitCmd fails on missing");
    assertTrue(!sys.issueOrder("nonexistent", 1, ""), "IssueOrder fails on missing");
    assertTrue(!sys.damageTerminal("nonexistent", 10.0f), "Damage fails on missing");
    assertTrue(!sys.repairTerminal("nonexistent", 10.0f), "Repair fails on missing");
    assertTrue(!sys.updateFleetInfo("nonexistent", 5, 0.5f, 50.0f), "FleetInfo fails on missing");
    assertTrue(sys.getState("nonexistent") == "Unknown", "Unknown state on missing");
    assertTrue(sys.getActiveUser("nonexistent").empty(), "Empty user on missing");
    assertTrue(sys.getCurrentOrder("nonexistent") == "None", "None order on missing");
    assertTrue(approxEqual(sys.getIntegrity("nonexistent"), 0.0f), "0 integrity on missing");
    assertTrue(approxEqual(sys.getBootProgress("nonexistent"), 0.0f), "0 boot on missing");
    assertTrue(sys.getOrderHistoryCount("nonexistent") == 0, "0 history on missing");
    assertTrue(sys.getTotalOrdersIssued("nonexistent") == 0, "0 orders on missing");
    assertTrue(sys.getTotalSessions("nonexistent") == 0, "0 sessions on missing");
    assertTrue(sys.getFleetShipCount("nonexistent") == 0, "0 ships on missing");
    assertTrue(approxEqual(sys.getFleetReadiness("nonexistent"), 0.0f), "0 readiness on missing");
    assertTrue(approxEqual(sys.getFleetMorale("nonexistent"), 0.0f), "0 morale on missing");
    assertTrue(!sys.isOperational("nonexistent"), "Not operational on missing");
}


void run_fleet_command_terminal_system_tests() {
    testFleetCommandTerminalCreate();
    testFleetCommandTerminalPlaceAndBoot();
    testFleetCommandTerminalBootSequence();
    testFleetCommandTerminalLoginLogout();
    testFleetCommandTerminalCommandMode();
    testFleetCommandTerminalIssueOrders();
    testFleetCommandTerminalCooldown();
    testFleetCommandTerminalDamage();
    testFleetCommandTerminalRepair();
    testFleetCommandTerminalFleetInfo();
    testFleetCommandTerminalPowerOff();
    testFleetCommandTerminalOrderHistory();
    testFleetCommandTerminalMissing();
}
