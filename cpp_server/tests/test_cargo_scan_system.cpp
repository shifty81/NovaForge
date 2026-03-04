// Tests for: CargoScanSystem Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "components/navigation_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/cargo_scan_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== CargoScanSystem Tests ====================

static void testCargoScanInitiate() {
    std::cout << "\n=== Cargo Scan: Initiate ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    addComp<components::CargoScanState>(scanner);
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    assertTrue(sys.getPhase("scanner1") == "idle", "Initial phase is idle");
    assertTrue(sys.initiateScan("scanner1", "target1"), "Scan initiated");
    assertTrue(sys.getPhase("scanner1") == "scanning", "Phase is scanning");
    assertTrue(!sys.initiateScan("scanner1", "target1"), "Cannot initiate while scanning");
}

static void testCargoScanCancel() {
    std::cout << "\n=== Cargo Scan: Cancel ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    addComp<components::CargoScanState>(scanner);
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    sys.initiateScan("scanner1", "target1");
    assertTrue(sys.cancelScan("scanner1"), "Cancel scan success");
    assertTrue(sys.getPhase("scanner1") == "idle", "Phase back to idle");
    assertTrue(!sys.cancelScan("scanner1"), "Cannot cancel when idle");
}

static void testCargoScanComplete() {
    std::cout << "\n=== Cargo Scan: Complete Clean ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    auto* scan = addComp<components::CargoScanState>(scanner);
    scan->scan_time = 2.0f;
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    sys.initiateScan("scanner1", "target1");
    sys.update(2.0f);
    assertTrue(sys.getPhase("scanner1") == "complete", "Phase is complete");
    assertTrue(sys.getContrabandFound("scanner1") == 0, "No contraband on clean target");
    assertTrue(sys.getTotalScans("scanner1") == 1, "1 total scan");
}

static void testCargoScanContraband() {
    std::cout << "\n=== Cargo Scan: Contraband Found ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    auto* scan = addComp<components::CargoScanState>(scanner);
    scan->scan_time = 1.0f;
    scan->fine_per_contraband = 1000.0f;
    auto* target = world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Narcotics);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Weapons);
    sys.initiateScan("scanner1", "target1");
    sys.update(1.0f);
    assertTrue(sys.getPhase("scanner1") == "complete", "Scan complete");
    assertTrue(sys.getContrabandFound("scanner1") == 2, "Found 2 contraband items");
    assertTrue(sys.getTotalContrabandDetected("scanner1") == 2, "Lifetime 2 detected");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalFinesIssued("scanner1")), 2000.0f), "Fines: 2 * 1000");
}

static void testCargoScanProgress() {
    std::cout << "\n=== Cargo Scan: Progress ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    auto* scan = addComp<components::CargoScanState>(scanner);
    scan->scan_time = 4.0f;
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    assertTrue(approxEqual(sys.getScanProgress("scanner1"), 0.0f), "Progress 0 before scan");
    sys.initiateScan("scanner1", "target1");
    sys.update(2.0f);
    assertTrue(approxEqual(sys.getScanProgress("scanner1"), 0.5f), "Progress 0.5 at halfway");
}

static void testCargoScanCustoms() {
    std::cout << "\n=== Cargo Scan: Customs Scanner ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    addComp<components::CargoScanState>(scanner);

    systems::CargoScanSystem sys(&world);
    assertTrue(!sys.isCustomsScanner("scanner1"), "Not customs by default");
    assertTrue(sys.markAsCustomsScanner("scanner1", true), "Mark as customs");
    assertTrue(sys.isCustomsScanner("scanner1"), "Is now customs");
}

static void testCargoScanRemoveContraband() {
    std::cout << "\n=== Cargo Scan: Remove Contraband ===" << std::endl;
    ecs::World world;
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Narcotics);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Stolen);
    assertTrue(sys.removeContraband("target1", components::CargoScanState::ContrabandType::Narcotics), "Remove success");
    auto types = sys.getDetectedTypes("target1");
    assertTrue(types.size() == 1, "1 type remaining");
    assertTrue(types[0] == "stolen", "Remaining type is stolen");
}

static void testCargoScanDetectionChance() {
    std::cout << "\n=== Cargo Scan: Detection Chance ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    addComp<components::CargoScanState>(scanner);

    systems::CargoScanSystem sys(&world);
    assertTrue(sys.setDetectionChance("scanner1", 0.5f), "Set detection chance");
    assertTrue(!sys.setDetectionChance("nonexistent", 0.5f), "Set on missing fails");
}

static void testCargoScanMultipleScans() {
    std::cout << "\n=== Cargo Scan: Multiple Scans ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    auto* scan = addComp<components::CargoScanState>(scanner);
    scan->scan_time = 1.0f;
    scan->fine_per_contraband = 500.0f;
    world.createEntity("target1");
    world.createEntity("target2");

    systems::CargoScanSystem sys(&world);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Exotic);

    // Scan 1
    sys.initiateScan("scanner1", "target1");
    sys.update(1.0f);
    assertTrue(sys.getTotalScans("scanner1") == 1, "1 scan complete");
    assertTrue(sys.getContrabandFound("scanner1") == 1, "Found 1 contraband");

    // Reset to idle for next scan
    scan->phase = components::CargoScanState::ScanPhase::Idle;

    // Scan 2: clean target
    sys.initiateScan("scanner1", "target2");
    sys.update(1.0f);
    assertTrue(sys.getTotalScans("scanner1") == 2, "2 scans total");
    assertTrue(sys.getContrabandFound("scanner1") == 0, "No contraband on clean target");
    assertTrue(sys.getTotalContrabandDetected("scanner1") == 1, "Lifetime 1 detected");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalFinesIssued("scanner1")), 500.0f), "Total fines 500");
}

static void testCargoScanMissing() {
    std::cout << "\n=== Cargo Scan: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CargoScanSystem sys(&world);
    assertTrue(sys.getPhase("nonexistent") == "unknown", "Default phase for missing");
    assertTrue(approxEqual(sys.getScanProgress("nonexistent"), 0.0f), "Default progress for missing");
    assertTrue(sys.getContrabandFound("nonexistent") == 0, "Default contraband for missing");
    assertTrue(sys.getTotalScans("nonexistent") == 0, "Default scans for missing");
    assertTrue(sys.getTotalContrabandDetected("nonexistent") == 0, "Default lifetime for missing");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalFinesIssued("nonexistent")), 0.0f), "Default fines for missing");
    assertTrue(!sys.isCustomsScanner("nonexistent"), "Default not customs for missing");
}


void run_cargo_scan_system_tests() {
    testCargoScanInitiate();
    testCargoScanCancel();
    testCargoScanComplete();
    testCargoScanContraband();
    testCargoScanProgress();
    testCargoScanCustoms();
    testCargoScanRemoveContraband();
    testCargoScanDetectionChance();
    testCargoScanMultipleScans();
    testCargoScanMissing();
}
