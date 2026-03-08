// Tests for: HullClassCapabilitySystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/hull_class_capability_system.h"

using namespace atlas;

// ==================== HullClassCapabilitySystem Tests ====================

static void testHullCapFrigateProfile() {
    std::cout << "\n=== HullCapability: FrigateProfile ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Frigate"), "Init Frigate");
    assertTrue(sys.getHullClass("ship1") == "Frigate", "Hull class is Frigate");
    assertTrue(sys.getRoverBayCount("ship1") == 0, "Frigate: no rover bay");
    assertTrue(sys.getGravBikeBayCount("ship1") == 0, "Frigate: no grav bike bay");
    assertTrue(sys.getShipHangarCount("ship1") == 0, "Frigate: no ship hangar");
    assertTrue(!sys.hasSurvivalModule("ship1"), "Frigate: no survival module");
    assertTrue(!sys.hasRigLocker("ship1"), "Frigate: no rig locker");
    assertTrue(sys.getMaxPowerGrid("ship1") == 50.0f, "Frigate: 50 MW power");
    assertTrue(sys.getMaxCPU("ship1") == 150.0f, "Frigate: 150 TF CPU");
    assertTrue(sys.getMaxModuleSlots("ship1") == 4, "Frigate: 4 module slots");
    assertTrue(sys.getMaxCargoVolume("ship1") == 500.0f, "Frigate: 500 m3 cargo");
}

static void testHullCapShuttleProfile() {
    std::cout << "\n=== HullCapability: ShuttleProfile ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Shuttle"), "Init Shuttle");
    assertTrue(sys.getHullClass("ship1") == "Shuttle", "Hull class is Shuttle");
    assertTrue(sys.getRoverBayCount("ship1") == 0, "Shuttle: no rover bay");
    assertTrue(sys.getShipHangarCount("ship1") == 0, "Shuttle: no hangar");
    assertTrue(!sys.hasSurvivalModule("ship1"), "Shuttle: no survival module");
}

static void testHullCapDestroyerProfile() {
    std::cout << "\n=== HullCapability: DestroyerProfile ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Destroyer"), "Init Destroyer");
    assertTrue(sys.getHullClass("ship1") == "Destroyer", "Hull class is Destroyer");

    // Destroyer: 1 rover bay (S), 1 grav bike bay (S), 1 ship hangar (S)
    assertTrue(sys.getRoverBayCount("ship1") == 1, "Destroyer: 1 rover bay");
    assertTrue(sys.getRoverBayClass("ship1") == "S", "Destroyer: rover bay class S");
    assertTrue(sys.getGravBikeBayCount("ship1") == 1, "Destroyer: 1 grav bike bay");
    assertTrue(sys.getGravBikeBayClass("ship1") == "S", "Destroyer: grav bike class S");
    assertTrue(sys.getShipHangarCount("ship1") == 1, "Destroyer: 1 ship hangar");
    assertTrue(sys.getShipHangarClass("ship1") == "S", "Destroyer: hangar class S");

    // Destroyer: survival module + rig locker
    assertTrue(sys.hasSurvivalModule("ship1"), "Destroyer: has survival module");
    assertTrue(sys.hasRigLocker("ship1"), "Destroyer: has rig locker");

    // Structural budgets
    assertTrue(sys.getMaxPowerGrid("ship1") == 200.0f, "Destroyer: 200 MW");
    assertTrue(sys.getMaxCPU("ship1") == 400.0f, "Destroyer: 400 TF");
    assertTrue(sys.getMaxModuleSlots("ship1") == 8, "Destroyer: 8 module slots");
    assertTrue(sys.getMaxCargoVolume("ship1") == 2000.0f, "Destroyer: 2000 m3");
}

static void testHullCapCruiserProfile() {
    std::cout << "\n=== HullCapability: CruiserProfile ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Cruiser"), "Init Cruiser");
    assertTrue(sys.getRoverBayCount("ship1") == 1, "Cruiser: 1 rover bay");
    assertTrue(sys.getRoverBayClass("ship1") == "M", "Cruiser: rover bay class M");
    assertTrue(sys.getGravBikeBayCount("ship1") == 1, "Cruiser: 1 grav bike bay");
    assertTrue(sys.getGravBikeBayClass("ship1") == "S", "Cruiser: grav bike class S");
    assertTrue(sys.getShipHangarCount("ship1") == 1, "Cruiser: 1 ship hangar");
    assertTrue(sys.getShipHangarClass("ship1") == "S", "Cruiser: hangar class S");
    assertTrue(sys.hasSurvivalModule("ship1"), "Cruiser: survival module");
    assertTrue(sys.hasRigLocker("ship1"), "Cruiser: rig locker");
    assertTrue(sys.getMaxPowerGrid("ship1") == 400.0f, "Cruiser: 400 MW");
    assertTrue(sys.getMaxModuleSlots("ship1") == 12, "Cruiser: 12 slots");
    assertTrue(sys.getMaxCargoVolume("ship1") == 5000.0f, "Cruiser: 5000 m3");
}

static void testHullCapBattlecruiserProfile() {
    std::cout << "\n=== HullCapability: BattlecruiserProfile ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Battlecruiser"), "Init Battlecruiser");
    assertTrue(sys.getRoverBayCount("ship1") == 1, "BC: 1 rover bay");
    assertTrue(sys.getRoverBayClass("ship1") == "M", "BC: rover bay class M");
    assertTrue(sys.getGravBikeBayCount("ship1") == 1, "BC: 1 grav bike bay");
    assertTrue(sys.getGravBikeBayClass("ship1") == "M", "BC: grav bike class M");
    assertTrue(sys.getShipHangarCount("ship1") == 1, "BC: 1 ship hangar");
    assertTrue(sys.getShipHangarClass("ship1") == "M", "BC: hangar class M");
    assertTrue(sys.getMaxPowerGrid("ship1") == 600.0f, "BC: 600 MW");
    assertTrue(sys.getMaxModuleSlots("ship1") == 14, "BC: 14 slots");
}

static void testHullCapBattleshipProfile() {
    std::cout << "\n=== HullCapability: BattleshipProfile ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Battleship"), "Init Battleship");
    assertTrue(sys.getRoverBayCount("ship1") == 1, "BS: 1 rover bay");
    assertTrue(sys.getRoverBayClass("ship1") == "M", "BS: rover bay class M");
    assertTrue(sys.getGravBikeBayCount("ship1") == 1, "BS: 1 grav bike bay");
    assertTrue(sys.getGravBikeBayClass("ship1") == "M", "BS: grav bike class M");
    assertTrue(sys.getShipHangarCount("ship1") == 1, "BS: 1 ship hangar");
    assertTrue(sys.getShipHangarClass("ship1") == "M", "BS: hangar class M");
    assertTrue(sys.hasSurvivalModule("ship1"), "BS: survival module");
    assertTrue(sys.getMaxPowerGrid("ship1") == 800.0f, "BS: 800 MW");
    assertTrue(sys.getMaxModuleSlots("ship1") == 16, "BS: 16 slots");
    assertTrue(sys.getMaxCargoVolume("ship1") == 10000.0f, "BS: 10000 m3");
}

static void testHullCapCarrierProfile() {
    std::cout << "\n=== HullCapability: CarrierProfile ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Carrier"), "Init Carrier");
    assertTrue(sys.getRoverBayCount("ship1") == 2, "Carrier: 2 rover bays");
    assertTrue(sys.getRoverBayClass("ship1") == "L", "Carrier: rover bay class L");
    assertTrue(sys.getGravBikeBayCount("ship1") == 2, "Carrier: 2 grav bike bays");
    assertTrue(sys.getGravBikeBayClass("ship1") == "M", "Carrier: grav bike class M");
    assertTrue(sys.getShipHangarCount("ship1") == 3, "Carrier: 3 ship hangars");
    assertTrue(sys.getShipHangarClass("ship1") == "L", "Carrier: hangar class L");
    assertTrue(sys.hasSurvivalModule("ship1"), "Carrier: survival module");
    assertTrue(sys.hasRigLocker("ship1"), "Carrier: rig locker");
    assertTrue(sys.getMaxPowerGrid("ship1") == 1500.0f, "Carrier: 1500 MW");
    assertTrue(sys.getMaxModuleSlots("ship1") == 20, "Carrier: 20 slots");
    assertTrue(sys.getMaxCargoVolume("ship1") == 25000.0f, "Carrier: 25000 m3");
}

static void testHullCapDreadnoughtProfile() {
    std::cout << "\n=== HullCapability: DreadnoughtProfile ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Dreadnought"), "Init Dreadnought");
    assertTrue(sys.getRoverBayCount("ship1") == 2, "Dread: 2 rover bays");
    assertTrue(sys.getRoverBayClass("ship1") == "L", "Dread: rover bay class L");
    assertTrue(sys.getGravBikeBayCount("ship1") == 2, "Dread: 2 grav bike bays");
    assertTrue(sys.getGravBikeBayClass("ship1") == "L", "Dread: grav bike class L");
    assertTrue(sys.getShipHangarCount("ship1") == 2, "Dread: 2 ship hangars");
    assertTrue(sys.getShipHangarClass("ship1") == "L", "Dread: hangar class L");
    assertTrue(sys.getMaxPowerGrid("ship1") == 2000.0f, "Dread: 2000 MW");
    assertTrue(sys.getMaxModuleSlots("ship1") == 24, "Dread: 24 slots");
    assertTrue(sys.getMaxCargoVolume("ship1") == 35000.0f, "Dread: 35000 m3");
}

static void testHullCapTitanProfile() {
    std::cout << "\n=== HullCapability: TitanProfile ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Titan"), "Init Titan");
    assertTrue(sys.getRoverBayCount("ship1") == 3, "Titan: 3 rover bays");
    assertTrue(sys.getRoverBayClass("ship1") == "XL", "Titan: rover bay class XL");
    assertTrue(sys.getGravBikeBayCount("ship1") == 3, "Titan: 3 grav bike bays");
    assertTrue(sys.getGravBikeBayClass("ship1") == "L", "Titan: grav bike class L");
    assertTrue(sys.getShipHangarCount("ship1") == 4, "Titan: 4 ship hangars");
    assertTrue(sys.getShipHangarClass("ship1") == "L", "Titan: hangar class L");
    assertTrue(sys.hasSurvivalModule("ship1"), "Titan: survival module");
    assertTrue(sys.hasRigLocker("ship1"), "Titan: rig locker");
    assertTrue(sys.getMaxPowerGrid("ship1") == 5000.0f, "Titan: 5000 MW");
    assertTrue(sys.getMaxCPU("ship1") == 5000.0f, "Titan: 5000 TF CPU");
    assertTrue(sys.getMaxModuleSlots("ship1") == 32, "Titan: 32 slots");
    assertTrue(sys.getMaxCargoVolume("ship1") == 100000.0f, "Titan: 100000 m3");
}

static void testHullCapScalingProgression() {
    std::cout << "\n=== HullCapability: ScalingProgression ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);

    // Verify capability scaling across hull classes
    world.createEntity("frigate");
    world.createEntity("destroyer");
    world.createEntity("cruiser");
    world.createEntity("titan");
    sys.initializeProfile("frigate", "Frigate");
    sys.initializeProfile("destroyer", "Destroyer");
    sys.initializeProfile("cruiser", "Cruiser");
    sys.initializeProfile("titan", "Titan");

    // Power grid scales up
    assertTrue(sys.getMaxPowerGrid("frigate") < sys.getMaxPowerGrid("destroyer"), "Frigate < Destroyer power");
    assertTrue(sys.getMaxPowerGrid("destroyer") < sys.getMaxPowerGrid("cruiser"), "Destroyer < Cruiser power");
    assertTrue(sys.getMaxPowerGrid("cruiser") < sys.getMaxPowerGrid("titan"), "Cruiser < Titan power");

    // Module slots scale up
    assertTrue(sys.getMaxModuleSlots("frigate") < sys.getMaxModuleSlots("destroyer"), "Frigate < Destroyer slots");
    assertTrue(sys.getMaxModuleSlots("destroyer") < sys.getMaxModuleSlots("cruiser"), "Destroyer < Cruiser slots");
    assertTrue(sys.getMaxModuleSlots("cruiser") < sys.getMaxModuleSlots("titan"), "Cruiser < Titan slots");

    // Hangar count scales up
    assertTrue(sys.getShipHangarCount("frigate") < sys.getShipHangarCount("destroyer"), "Frigate < Destroyer hangars");
    assertTrue(sys.getShipHangarCount("destroyer") <= sys.getShipHangarCount("titan"), "Destroyer <= Titan hangars");
}

static void testHullCapDestroyerIsMinimumForBays() {
    std::cout << "\n=== HullCapability: DestroyerIsMinimumForBays ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("frigate");
    world.createEntity("destroyer");
    sys.initializeProfile("frigate", "Frigate");
    sys.initializeProfile("destroyer", "Destroyer");

    // Frigate has NO bays
    assertTrue(sys.getRoverBayCount("frigate") == 0, "Frigate: 0 rover bays");
    assertTrue(sys.getGravBikeBayCount("frigate") == 0, "Frigate: 0 grav bike bays");
    assertTrue(sys.getShipHangarCount("frigate") == 0, "Frigate: 0 hangars");
    assertTrue(!sys.hasSurvivalModule("frigate"), "Frigate: no survival");

    // Destroyer is the minimum for all bays
    assertTrue(sys.getRoverBayCount("destroyer") >= 1, "Destroyer: >= 1 rover bay");
    assertTrue(sys.getGravBikeBayCount("destroyer") >= 1, "Destroyer: >= 1 grav bike bay");
    assertTrue(sys.getShipHangarCount("destroyer") >= 1, "Destroyer: >= 1 hangar");
    assertTrue(sys.hasSurvivalModule("destroyer"), "Destroyer: has survival");
}

static void testHullCapRigLockerLinkedToRoverBay() {
    std::cout << "\n=== HullCapability: RigLockerLinkedToRoverBay ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProfile("ship1", "Destroyer");

    // Rig locker present when rover bay exists
    assertTrue(sys.getRoverBayCount("ship1") > 0, "Has rover bay");
    assertTrue(sys.hasRigLocker("ship1"), "Rig locker with rover bay");

    // Remove rover bays — rig locker should auto-remove
    assertTrue(sys.setRoverBayCount("ship1", 0), "Set rover bays to 0");
    assertTrue(!sys.hasRigLocker("ship1"), "No rig locker without rover bay");

    // Re-add rover bay — rig locker returns
    assertTrue(sys.setRoverBayCount("ship1", 1), "Set rover bays to 1");
    assertTrue(sys.hasRigLocker("ship1"), "Rig locker returns with rover bay");
}

static void testHullCapOverrideBayCounts() {
    std::cout << "\n=== HullCapability: OverrideBayCounts ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProfile("ship1", "Cruiser");

    assertTrue(sys.setRoverBayCount("ship1", 3), "Override rover bay count");
    assertTrue(sys.getRoverBayCount("ship1") == 3, "Rover bay count is 3");

    assertTrue(sys.setGravBikeBayCount("ship1", 2), "Override grav bike count");
    assertTrue(sys.getGravBikeBayCount("ship1") == 2, "Grav bike count is 2");

    assertTrue(sys.setShipHangarCount("ship1", 5), "Override hangar count");
    assertTrue(sys.getShipHangarCount("ship1") == 5, "Hangar count is 5");

    // Negative clamped to 0
    assertTrue(sys.setRoverBayCount("ship1", -1), "Negative clamped");
    assertTrue(sys.getRoverBayCount("ship1") == 0, "Clamped to 0");
}

static void testHullCapDuplicateInitRejected() {
    std::cout << "\n=== HullCapability: DuplicateInitRejected ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Destroyer"), "First init ok");
    assertTrue(!sys.initializeProfile("ship1", "Titan"), "Duplicate init rejected");
    assertTrue(sys.getHullClass("ship1") == "Destroyer", "Still Destroyer");
}

static void testHullCapUnknownHullClass() {
    std::cout << "\n=== HullCapability: UnknownHullClass ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeProfile("ship1", "Corvette"), "Init unknown class");
    assertTrue(sys.getHullClass("ship1") == "Corvette", "Hull class stored");
    assertTrue(sys.getRoverBayCount("ship1") == 0, "Unknown: 0 rover bays");
    assertTrue(sys.getShipHangarCount("ship1") == 0, "Unknown: 0 hangars");
    assertTrue(sys.getMaxPowerGrid("ship1") == 50.0f, "Unknown: Frigate-like power");
}

static void testHullCapMissingEntity() {
    std::cout << "\n=== HullCapability: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::HullClassCapabilitySystem sys(&world);

    assertTrue(!sys.initializeProfile("ghost", "Frigate"), "Init fails for missing entity");
    assertTrue(sys.getHullClass("ghost") == "", "Hull class empty for missing");
    assertTrue(sys.getRoverBayCount("ghost") == 0, "Rover bay 0 for missing");
    assertTrue(sys.getGravBikeBayCount("ghost") == 0, "Grav bike 0 for missing");
    assertTrue(sys.getShipHangarCount("ghost") == 0, "Hangar 0 for missing");
    assertTrue(sys.getRoverBayClass("ghost") == "", "Rover class empty for missing");
    assertTrue(sys.getGravBikeBayClass("ghost") == "", "Grav bike class empty for missing");
    assertTrue(sys.getShipHangarClass("ghost") == "", "Hangar class empty for missing");
    assertTrue(!sys.hasSurvivalModule("ghost"), "No survival for missing");
    assertTrue(!sys.hasRigLocker("ghost"), "No rig locker for missing");
    assertTrue(sys.getMaxPowerGrid("ghost") == 0.0f, "Power 0 for missing");
    assertTrue(sys.getMaxCPU("ghost") == 0.0f, "CPU 0 for missing");
    assertTrue(sys.getMaxModuleSlots("ghost") == 0, "Slots 0 for missing");
    assertTrue(sys.getMaxCargoVolume("ghost") == 0.0f, "Cargo 0 for missing");
    assertTrue(!sys.setRoverBayCount("ghost", 1), "Set rover fails for missing");
    assertTrue(!sys.setGravBikeBayCount("ghost", 1), "Set grav bike fails for missing");
    assertTrue(!sys.setShipHangarCount("ghost", 1), "Set hangar fails for missing");
}

void run_hull_class_capability_system_tests() {
    testHullCapFrigateProfile();
    testHullCapShuttleProfile();
    testHullCapDestroyerProfile();
    testHullCapCruiserProfile();
    testHullCapBattlecruiserProfile();
    testHullCapBattleshipProfile();
    testHullCapCarrierProfile();
    testHullCapDreadnoughtProfile();
    testHullCapTitanProfile();
    testHullCapScalingProgression();
    testHullCapDestroyerIsMinimumForBays();
    testHullCapRigLockerLinkedToRoverBay();
    testHullCapOverrideBayCounts();
    testHullCapDuplicateInitRejected();
    testHullCapUnknownHullClass();
    testHullCapMissingEntity();
}
