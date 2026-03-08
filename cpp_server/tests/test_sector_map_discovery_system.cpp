// Tests for: Sector Map Discovery System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/sector_map_discovery_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Sector Map Discovery System Tests ====================

static void testSectorMapDiscoveryCreate() {
    std::cout << "\n=== SectorMapDiscovery: Create ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    assertTrue(sys.initialize("smd1"), "Init succeeds");
    assertTrue(sys.getSectorCount("smd1") == 0, "No sectors initially");
    assertTrue(sys.getFullyExploredCount("smd1") == 0, "No fully explored sectors");
    assertTrue(approxEqual(sys.getExplorationPercent("smd1"), 0.0f), "0% explored");
    assertTrue(sys.getTotalVisits("smd1") == 0, "0 total visits");
}

static void testSectorMapDiscoveryDiscover() {
    std::cout << "\n=== SectorMapDiscovery: Discover ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    sys.initialize("smd1");
    assertTrue(sys.discoverSector("smd1", "sec_a", "Mining Belt Alpha"), "Discover sector A");
    assertTrue(sys.discoverSector("smd1", "sec_b", "Trade Hub Beta"), "Discover sector B");
    assertTrue(sys.getSectorCount("smd1") == 2, "2 sectors discovered");
    assertTrue(sys.isSectorDiscovered("smd1", "sec_a"), "Sector A is discovered");
    assertTrue(!sys.isSectorDiscovered("smd1", "sec_c"), "Sector C is not discovered");
}

static void testSectorMapDiscoveryDuplicate() {
    std::cout << "\n=== SectorMapDiscovery: Duplicate ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    sys.initialize("smd1");
    sys.discoverSector("smd1", "sec_a", "Mining Belt Alpha");
    assertTrue(!sys.discoverSector("smd1", "sec_a", "Same sector"), "Duplicate sector rejected");
    assertTrue(sys.getSectorCount("smd1") == 1, "Still 1 sector");
}

static void testSectorMapDiscoveryVisit() {
    std::cout << "\n=== SectorMapDiscovery: Visit ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    sys.initialize("smd1");
    sys.discoverSector("smd1", "sec_a", "Mining Belt Alpha");
    assertTrue(sys.getVisibility("smd1", "sec_a") == 1, "Partial visibility on discovery");
    assertTrue(sys.visitSector("smd1", "sec_a"), "Visit sector A");
    assertTrue(sys.getVisibility("smd1", "sec_a") == 2, "Full visibility after visit");
    assertTrue(sys.getVisitCount("smd1", "sec_a") == 1, "1 visit to sector A");
    assertTrue(sys.visitSector("smd1", "sec_a"), "Visit sector A again");
    assertTrue(sys.getVisitCount("smd1", "sec_a") == 2, "2 visits to sector A");
    assertTrue(sys.getTotalVisits("smd1") == 2, "2 total visits");
}

static void testSectorMapDiscoveryVisibility() {
    std::cout << "\n=== SectorMapDiscovery: Visibility ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    sys.initialize("smd1");
    sys.discoverSector("smd1", "sec_a", "Mining Belt Alpha");
    assertTrue(sys.setVisibility("smd1", "sec_a", 0), "Set visibility to hidden");
    assertTrue(sys.getVisibility("smd1", "sec_a") == 0, "Visibility is 0");
    assertTrue(sys.setVisibility("smd1", "sec_a", 2), "Set visibility to full");
    assertTrue(sys.getVisibility("smd1", "sec_a") == 2, "Visibility is 2");
    assertTrue(!sys.setVisibility("smd1", "nonexistent", 1), "Missing sector fails");
}

static void testSectorMapDiscoveryRemove() {
    std::cout << "\n=== SectorMapDiscovery: Remove ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    sys.initialize("smd1");
    sys.discoverSector("smd1", "sec_a", "Mining Belt Alpha");
    assertTrue(sys.removeSector("smd1", "sec_a"), "Remove sector A");
    assertTrue(sys.getSectorCount("smd1") == 0, "0 sectors after remove");
    assertTrue(!sys.removeSector("smd1", "sec_a"), "Double remove fails");
}

static void testSectorMapDiscoveryFullyExplored() {
    std::cout << "\n=== SectorMapDiscovery: FullyExplored ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    sys.initialize("smd1");
    sys.discoverSector("smd1", "sec_a", "Mining Belt Alpha");
    sys.discoverSector("smd1", "sec_b", "Trade Hub Beta");
    assertTrue(sys.getFullyExploredCount("smd1") == 0, "0 fully explored at discovery");
    sys.visitSector("smd1", "sec_a");
    assertTrue(sys.getFullyExploredCount("smd1") == 1, "1 fully explored after visit");
    sys.visitSector("smd1", "sec_b");
    assertTrue(sys.getFullyExploredCount("smd1") == 2, "2 fully explored after both visits");
}

static void testSectorMapDiscoveryExplorationPercent() {
    std::cout << "\n=== SectorMapDiscovery: ExplorationPercent ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    sys.initialize("smd1");

    auto* entity = world.getEntity("smd1");
    auto* smd = entity->getComponent<components::SectorMapDiscovery>();
    smd->max_sectors = 4;

    sys.discoverSector("smd1", "sec_a", "Alpha");
    assertTrue(approxEqual(sys.getExplorationPercent("smd1"), 0.25f), "25% explored (1/4)");
    sys.discoverSector("smd1", "sec_b", "Beta");
    assertTrue(approxEqual(sys.getExplorationPercent("smd1"), 0.50f), "50% explored (2/4)");
}

static void testSectorMapDiscoveryMaxSectors() {
    std::cout << "\n=== SectorMapDiscovery: MaxSectors ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    sys.initialize("smd1");

    auto* entity = world.getEntity("smd1");
    auto* smd = entity->getComponent<components::SectorMapDiscovery>();
    smd->max_sectors = 2;

    sys.discoverSector("smd1", "sec_a", "Alpha");
    sys.discoverSector("smd1", "sec_b", "Beta");
    assertTrue(!sys.discoverSector("smd1", "sec_c", "Gamma"), "Max sectors enforced");
    assertTrue(sys.getSectorCount("smd1") == 2, "Still 2 sectors");
}

static void testSectorMapDiscoveryUpdate() {
    std::cout << "\n=== SectorMapDiscovery: Update ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    world.createEntity("smd1");
    sys.initialize("smd1");
    sys.update(5.0f);
    sys.update(3.0f);
    // elapsed should advance but we verify through sector discovery time
    sys.discoverSector("smd1", "sec_a", "Alpha");
    assertTrue(sys.isSectorDiscovered("smd1", "sec_a"), "Sector discovered after update");
}

static void testSectorMapDiscoveryMissing() {
    std::cout << "\n=== SectorMapDiscovery: Missing ===" << std::endl;
    ecs::World world;
    systems::SectorMapDiscoverySystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.discoverSector("nonexistent", "s1", "Test"), "Discover fails on missing");
    assertTrue(!sys.visitSector("nonexistent", "s1"), "Visit fails on missing");
    assertTrue(!sys.setVisibility("nonexistent", "s1", 1), "SetVisibility fails on missing");
    assertTrue(!sys.removeSector("nonexistent", "s1"), "Remove fails on missing");
    assertTrue(sys.getSectorCount("nonexistent") == 0, "0 sectors on missing");
    assertTrue(sys.getVisibility("nonexistent", "s1") == 0, "0 visibility on missing");
    assertTrue(sys.getVisitCount("nonexistent", "s1") == 0, "0 visits on missing");
    assertTrue(!sys.isSectorDiscovered("nonexistent", "s1"), "Not discovered on missing");
    assertTrue(sys.getFullyExploredCount("nonexistent") == 0, "0 fully explored on missing");
    assertTrue(approxEqual(sys.getExplorationPercent("nonexistent"), 0.0f), "0% on missing");
    assertTrue(sys.getTotalVisits("nonexistent") == 0, "0 total visits on missing");
}


void run_sector_map_discovery_system_tests() {
    testSectorMapDiscoveryCreate();
    testSectorMapDiscoveryDiscover();
    testSectorMapDiscoveryDuplicate();
    testSectorMapDiscoveryVisit();
    testSectorMapDiscoveryVisibility();
    testSectorMapDiscoveryRemove();
    testSectorMapDiscoveryFullyExplored();
    testSectorMapDiscoveryExplorationPercent();
    testSectorMapDiscoveryMaxSectors();
    testSectorMapDiscoveryUpdate();
    testSectorMapDiscoveryMissing();
}
