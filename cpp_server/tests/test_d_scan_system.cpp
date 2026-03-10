// Tests for: DScanSystem (Directional Scanner)
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/d_scan_system.h"

using namespace atlas;

// ==================== DScanSystem Tests ====================

static void testDScanInit() {
    std::cout << "\n=== DScan: Init ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(!sys.isScanning("ship1"), "Not scanning initially");
    assertTrue(sys.getScanCount("ship1") == 0, "Scan count starts at 0");
    assertTrue(sys.getContactCount("ship1") == 0, "No contacts initially");
    assertTrue(approxEqual(sys.getScanRange("ship1"), 14.3f), "Default range is 14.3 AU");
    assertTrue(approxEqual(sys.getScanAngle("ship1"), 360.0f), "Default angle is 360");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testDScanStartScan() {
    std::cout << "\n=== DScan: StartScan ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.startScan("ship1", 5.0f, 60.0f), "Start scan succeeds");
    assertTrue(sys.isScanning("ship1"), "Is scanning after startScan");
    assertTrue(approxEqual(sys.getScanRange("ship1"), 5.0f), "Range updated to 5.0 AU");
    assertTrue(approxEqual(sys.getScanAngle("ship1"), 60.0f), "Angle updated to 60°");

    // Cannot start scan while already scanning
    assertTrue(!sys.startScan("ship1", 10.0f, 90.0f), "Start scan rejected while scanning");
}

static void testDScanStartScanInvalidParams() {
    std::cout << "\n=== DScan: StartScanInvalidParams ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(!sys.startScan("ship1", 0.0f, 90.0f), "Zero range rejected");
    assertTrue(!sys.startScan("ship1", 5.0f, 0.0f), "Zero angle rejected");
    assertTrue(!sys.startScan("nonexistent", 5.0f, 90.0f), "Fails on missing entity");
}

static void testDScanScanCompletion() {
    std::cout << "\n=== DScan: ScanCompletion ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Set a short scan duration
    auto* comp = world.getEntity("ship1")->getComponent<components::DScanState>();
    comp->scan_duration = 2.0f;

    sys.startScan("ship1", 14.3f, 360.0f);
    assertTrue(sys.isScanning("ship1"), "Scanning started");

    sys.update(1.0f);
    assertTrue(sys.isScanning("ship1"), "Still scanning after 1s");

    sys.update(1.5f);
    assertTrue(!sys.isScanning("ship1"), "Scan completed after 2s");
    assertTrue(sys.getScanCount("ship1") == 1, "Scan count incremented");
}

static void testDScanAddContact() {
    std::cout << "\n=== DScan: AddContact ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.startScan("ship1", 14.3f, 360.0f);

    assertTrue(sys.addContact("ship1", "enemy1", "Rifter", 0, 3.5f), "Add ship contact");
    assertTrue(sys.addContact("ship1", "str1", "Station", 1, 10.0f), "Add structure contact");
    assertTrue(sys.getContactCount("ship1") == 2, "2 contacts added");

    // Duplicate prevention
    assertTrue(!sys.addContact("ship1", "enemy1", "Rifter", 0, 3.5f), "Duplicate contact rejected");
    assertTrue(sys.getContactCount("ship1") == 2, "Count unchanged after duplicate");

    auto contacts = sys.getContacts("ship1");
    assertTrue(contacts[0].entity_id == "enemy1", "First contact entity_id");
    assertTrue(contacts[0].name == "Rifter", "First contact name");
    assertTrue(approxEqual(contacts[0].distance, 3.5f), "First contact distance");
    assertTrue(contacts[0].type == components::DScanState::ContactType::Ship,
               "First contact type is Ship");
}

static void testDScanClearContacts() {
    std::cout << "\n=== DScan: ClearContacts ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.startScan("ship1", 14.3f, 360.0f);
    sys.addContact("ship1", "enemy1", "Rifter", 0, 3.5f);
    sys.addContact("ship1", "enemy2", "Drake", 0, 7.0f);
    assertTrue(sys.getContactCount("ship1") == 2, "2 contacts before clear");

    assertTrue(sys.clearContacts("ship1"), "Clear contacts succeeds");
    assertTrue(sys.getContactCount("ship1") == 0, "0 contacts after clear");
    assertTrue(!sys.clearContacts("nonexistent"), "Clear fails on missing entity");
}

static void testDScanContactsClearedOnNewScan() {
    std::cout << "\n=== DScan: ContactsClearedOnNewScan ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // First scan with contact
    auto* comp = world.getEntity("ship1")->getComponent<components::DScanState>();
    comp->scan_duration = 1.0f;
    sys.startScan("ship1", 14.3f, 360.0f);
    sys.addContact("ship1", "enemy1", "Rifter", 0, 3.5f);
    sys.update(2.0f); // complete scan
    assertTrue(sys.getScanCount("ship1") == 1, "First scan complete");

    // Second scan — contacts cleared
    sys.startScan("ship1", 10.0f, 180.0f);
    assertTrue(sys.getContactCount("ship1") == 0, "Contacts cleared on new scan");
}

static void testDScanMaxContacts() {
    std::cout << "\n=== DScan: MaxContacts ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.startScan("ship1", 14.3f, 360.0f);

    auto* comp = world.getEntity("ship1")->getComponent<components::DScanState>();
    comp->max_contacts = 3;

    sys.addContact("ship1", "e1", "Ship1", 0, 1.0f);
    sys.addContact("ship1", "e2", "Ship2", 0, 2.0f);
    sys.addContact("ship1", "e3", "Ship3", 0, 3.0f);
    assertTrue(!sys.addContact("ship1", "e4", "Ship4", 0, 4.0f), "Max contacts enforced");
    assertTrue(sys.getContactCount("ship1") == 3, "Capped at max_contacts");
}

static void testDScanMissingEntity() {
    std::cout << "\n=== DScan: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);

    assertTrue(!sys.isScanning("nonexistent"), "Not scanning on missing");
    assertTrue(sys.getScanCount("nonexistent") == 0, "0 scans on missing");
    assertTrue(sys.getContactCount("nonexistent") == 0, "0 contacts on missing");
    assertTrue(approxEqual(sys.getScanRange("nonexistent"), 0.0f), "0 range on missing");
    assertTrue(approxEqual(sys.getScanAngle("nonexistent"), 0.0f), "0 angle on missing");
    assertTrue(sys.getContacts("nonexistent").empty(), "Empty contacts on missing");
    assertTrue(!sys.addContact("nonexistent", "e1", "Ship", 0, 1.0f), "AddContact fails on missing");
    assertTrue(!sys.clearContacts("nonexistent"), "ClearContacts fails on missing");
}

static void testDScanContactTypes() {
    std::cout << "\n=== DScan: ContactTypes ===" << std::endl;
    ecs::World world;
    systems::DScanSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.startScan("ship1", 14.3f, 360.0f);

    sys.addContact("ship1", "e_ship",      "Rifter",    0, 1.0f);
    sys.addContact("ship1", "e_structure", "Citadel",   1, 2.0f);
    sys.addContact("ship1", "e_probe",     "CoreProbe", 2, 3.0f);
    sys.addContact("ship1", "e_drone",     "HobGoblin", 3, 4.0f);
    sys.addContact("ship1", "e_other",     "Wreck",     4, 5.0f);

    auto contacts = sys.getContacts("ship1");
    assertTrue(contacts.size() == 5, "5 contacts of different types");
    assertTrue(contacts[1].type == components::DScanState::ContactType::Structure,
               "Structure type correct");
    assertTrue(contacts[2].type == components::DScanState::ContactType::Probe,
               "Probe type correct");
    assertTrue(contacts[3].type == components::DScanState::ContactType::Drone,
               "Drone type correct");
    assertTrue(contacts[4].type == components::DScanState::ContactType::Other,
               "Other type correct");
}

void run_d_scan_system_tests() {
    testDScanInit();
    testDScanStartScan();
    testDScanStartScanInvalidParams();
    testDScanScanCompletion();
    testDScanAddContact();
    testDScanClearContacts();
    testDScanContactsClearedOnNewScan();
    testDScanMaxContacts();
    testDScanMissingEntity();
    testDScanContactTypes();
}
