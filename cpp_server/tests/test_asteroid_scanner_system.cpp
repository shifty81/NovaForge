// Tests for: Asteroid Scanner System
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/asteroid_scanner_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Asteroid Scanner System Tests ====================

static void testAsteroidScannerCreate() {
    std::cout << "\n=== AsteroidScanner: Create ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    world.createEntity("as1");
    assertTrue(sys.initialize("as1", "rock_42"), "Init succeeds");
    assertTrue(!sys.isScanning("as1"), "Not scanning initially");
    assertTrue(!sys.isScanComplete("as1"), "Not complete initially");
    assertTrue(sys.getTotalScans("as1") == 0, "0 scans initially");
    assertTrue(sys.getReadingCount("as1") == 0, "0 readings initially");
    assertTrue(sys.getTargetAsteroid("as1") == "rock_42", "Target is rock_42");
}

static void testAsteroidScannerInitValidation() {
    std::cout << "\n=== AsteroidScanner: InitValidation ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    world.createEntity("as1");
    assertTrue(!sys.initialize("as1", ""), "Empty target rejected");
    assertTrue(!sys.initialize("nonexistent", "rock_1"), "Missing entity rejected");
}

static void testAsteroidScannerStartScan() {
    std::cout << "\n=== AsteroidScanner: StartScan ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    world.createEntity("as1");
    sys.initialize("as1", "rock_1");
    sys.setScanDuration("as1", 10.0f);
    assertTrue(sys.startScan("as1"), "Start scan");
    assertTrue(sys.isScanning("as1"), "Now scanning");
    sys.update(5.0f);
    float progress = sys.getScanProgress("as1");
    assertTrue(progress > 0.49f && progress < 0.51f, "50% after 5s of 10s scan");
    assertTrue(sys.isScanning("as1"), "Still scanning at 50%");
}

static void testAsteroidScannerCompleteScan() {
    std::cout << "\n=== AsteroidScanner: CompleteScan ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    world.createEntity("as1");
    sys.initialize("as1", "rock_1");
    sys.setScanDuration("as1", 5.0f);
    sys.addReading("as1", "Veldspar", 0.8f, 100.0f);
    sys.addReading("as1", "Scordite", 0.3f, 250.0f);
    sys.startScan("as1");
    sys.update(6.0f);
    assertTrue(sys.isScanComplete("as1"), "Scan complete");
    assertTrue(!sys.isScanning("as1"), "No longer scanning");
    assertTrue(sys.getTotalScans("as1") == 1, "1 scan completed");
    assertTrue(approxEqual(sys.getTotalValueScanned("as1"), 350.0f), "Value = 350");
}

static void testAsteroidScannerCancelScan() {
    std::cout << "\n=== AsteroidScanner: CancelScan ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    world.createEntity("as1");
    sys.initialize("as1", "rock_1");
    sys.startScan("as1");
    sys.update(2.0f);
    assertTrue(sys.cancelScan("as1"), "Cancel scan");
    assertTrue(!sys.isScanning("as1"), "No longer scanning");
    assertTrue(approxEqual(sys.getScanProgress("as1"), 0.0f), "Progress reset");
    assertTrue(!sys.cancelScan("as1"), "Cannot cancel when not scanning");
}

static void testAsteroidScannerAddReadings() {
    std::cout << "\n=== AsteroidScanner: AddReadings ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    world.createEntity("as1");
    sys.initialize("as1", "rock_1");
    assertTrue(sys.addReading("as1", "Veldspar", 0.9f, 100.0f), "Add Veldspar");
    assertTrue(sys.addReading("as1", "Pyroxeres", 0.4f, 500.0f), "Add Pyroxeres");
    assertTrue(sys.getReadingCount("as1") == 2, "2 readings");
    assertTrue(!sys.addReading("as1", "", 0.5f, 100.0f), "Empty ore type rejected");
}

static void testAsteroidScannerRemoveReading() {
    std::cout << "\n=== AsteroidScanner: RemoveReading ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    world.createEntity("as1");
    sys.initialize("as1", "rock_1");
    sys.addReading("as1", "Veldspar", 0.9f, 100.0f);
    sys.addReading("as1", "Scordite", 0.3f, 200.0f);
    assertTrue(sys.removeReading("as1", "Veldspar"), "Remove Veldspar");
    assertTrue(sys.getReadingCount("as1") == 1, "1 reading left");
    assertTrue(!sys.removeReading("as1", "Nonexistent"), "Cannot remove nonexistent");
}

static void testAsteroidScannerMaxReadings() {
    std::cout << "\n=== AsteroidScanner: MaxReadings ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    world.createEntity("as1");
    sys.initialize("as1", "rock_1");
    for (int i = 0; i < 10; ++i) {
        assertTrue(sys.addReading("as1", "ore_" + std::to_string(i), 0.5f, 100.0f),
                   ("Add reading " + std::to_string(i)).c_str());
    }
    assertTrue(!sys.addReading("as1", "ore_overflow", 0.5f, 100.0f), "11th reading rejected");
    assertTrue(sys.getReadingCount("as1") == 10, "10 readings max");
}

static void testAsteroidScannerDurationClamp() {
    std::cout << "\n=== AsteroidScanner: DurationClamp ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    world.createEntity("as1");
    sys.initialize("as1", "rock_1");
    sys.setScanDuration("as1", 0.1f);
    sys.startScan("as1");
    sys.update(1.5f);
    assertTrue(sys.isScanComplete("as1"), "Scan completed with clamped 1s duration");
    sys.setScanResolution("as1", 100.0f);
    // Resolution is clamped to 5.0
}

static void testAsteroidScannerMissing() {
    std::cout << "\n=== AsteroidScanner: Missing ===" << std::endl;
    ecs::World world;
    systems::AsteroidScannerSystem sys(&world);
    assertTrue(!sys.startScan("nonexistent"), "Start fails on missing");
    assertTrue(!sys.cancelScan("nonexistent"), "Cancel fails on missing");
    assertTrue(!sys.addReading("nonexistent", "ore", 0.5f, 100.0f), "Add reading fails on missing");
    assertTrue(!sys.removeReading("nonexistent", "ore"), "Remove reading fails on missing");
    assertTrue(!sys.setScanDuration("nonexistent", 5.0f), "Duration fails on missing");
    assertTrue(!sys.setScanResolution("nonexistent", 2.0f), "Resolution fails on missing");
    assertTrue(approxEqual(sys.getScanProgress("nonexistent"), 0.0f), "0 progress on missing");
    assertTrue(!sys.isScanning("nonexistent"), "Not scanning on missing");
    assertTrue(!sys.isScanComplete("nonexistent"), "Not complete on missing");
    assertTrue(sys.getReadingCount("nonexistent") == 0, "0 readings on missing");
    assertTrue(sys.getTotalScans("nonexistent") == 0, "0 scans on missing");
    assertTrue(approxEqual(sys.getTotalValueScanned("nonexistent"), 0.0f), "0 value on missing");
    assertTrue(sys.getTargetAsteroid("nonexistent").empty(), "Empty target on missing");
}

void run_asteroid_scanner_system_tests() {
    testAsteroidScannerCreate();
    testAsteroidScannerInitValidation();
    testAsteroidScannerStartScan();
    testAsteroidScannerCompleteScan();
    testAsteroidScannerCancelScan();
    testAsteroidScannerAddReadings();
    testAsteroidScannerRemoveReading();
    testAsteroidScannerMaxReadings();
    testAsteroidScannerDurationClamp();
    testAsteroidScannerMissing();
}
