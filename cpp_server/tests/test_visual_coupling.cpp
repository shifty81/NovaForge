// Tests for: VisualCoupling Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/visual_coupling_system.h"

using namespace atlas;

// ==================== VisualCoupling Tests ====================

static void testVisualCouplingInit() {
    std::cout << "\n=== VisualCoupling: Init ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    assertTrue(sys.initializeCoupling("ship_1", "frigate_01"), "Coupling initialized");
    assertTrue(sys.getCouplingCount("ship_1") == 0, "No couplings initially");
    assertTrue(!sys.initializeCoupling("ship_1", "frigate_01"), "Duplicate init fails");
}

static void testVisualCouplingAdd() {
    std::cout << "\n=== VisualCoupling: Add ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "frigate_01");
    assertTrue(sys.addCoupling("ship_1", "solar_1",
        components::VisualCoupling::ExteriorFeature::SolarPanel, 1.0f), "Solar panel added");
    assertTrue(sys.getCouplingCount("ship_1") == 1, "1 coupling");
    assertTrue(!sys.addCoupling("ship_1", "solar_1",
        components::VisualCoupling::ExteriorFeature::SolarPanel, 1.0f), "Duplicate rejected");
}

static void testVisualCouplingRemove() {
    std::cout << "\n=== VisualCoupling: Remove ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "frigate_01");
    sys.addCoupling("ship_1", "vent_1",
        components::VisualCoupling::ExteriorFeature::Vent, 0.5f);
    assertTrue(sys.getCouplingCount("ship_1") == 1, "1 before remove");
    assertTrue(sys.removeCoupling("ship_1", "vent_1"), "Removed");
    assertTrue(sys.getCouplingCount("ship_1") == 0, "0 after remove");
    assertTrue(!sys.removeCoupling("ship_1", "vent_1"), "Double remove fails");
}

static void testVisualCouplingVisibility() {
    std::cout << "\n=== VisualCoupling: Visibility ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "frigate_01");
    sys.addCoupling("ship_1", "ore_1",
        components::VisualCoupling::ExteriorFeature::OreContainer, 1.5f);
    assertTrue(sys.getVisibleCount("ship_1") == 1, "1 visible initially");
    assertTrue(sys.setVisibility("ship_1", "ore_1", false), "Set invisible");
    assertTrue(sys.getVisibleCount("ship_1") == 0, "0 visible after hide");
    assertTrue(sys.setVisibility("ship_1", "ore_1", true), "Set visible again");
    assertTrue(sys.getVisibleCount("ship_1") == 1, "1 visible after show");
}

static void testVisualCouplingOffset() {
    std::cout << "\n=== VisualCoupling: Offset ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "cruiser_01");
    sys.addCoupling("ship_1", "antenna_1",
        components::VisualCoupling::ExteriorFeature::Antenna, 0.8f);
    assertTrue(sys.setOffset("ship_1", "antenna_1", 1.0f, 2.0f, 3.0f), "Offset set");
    assertTrue(!sys.setOffset("ship_1", "nonexistent", 0, 0, 0), "Offset fails for missing");
}

static void testVisualCouplingFeatureCount() {
    std::cout << "\n=== VisualCoupling: Feature Count ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "battleship_01");
    sys.addCoupling("ship_1", "solar_1",
        components::VisualCoupling::ExteriorFeature::SolarPanel, 1.0f);
    sys.addCoupling("ship_1", "solar_2",
        components::VisualCoupling::ExteriorFeature::SolarPanel, 1.0f);
    sys.addCoupling("ship_1", "weapon_1",
        components::VisualCoupling::ExteriorFeature::WeaponMount, 1.0f);
    assertTrue(sys.getFeatureCount("ship_1",
        components::VisualCoupling::ExteriorFeature::SolarPanel) == 2, "2 solar panels");
    assertTrue(sys.getFeatureCount("ship_1",
        components::VisualCoupling::ExteriorFeature::WeaponMount) == 1, "1 weapon mount");
    assertTrue(sys.getFeatureCount("ship_1",
        components::VisualCoupling::ExteriorFeature::Vent) == 0, "0 vents");
}

static void testVisualCouplingFeatureNames() {
    std::cout << "\n=== VisualCoupling: Feature Names ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    assertTrue(sys.getFeatureName(components::VisualCoupling::ExteriorFeature::SolarPanel) == "SolarPanel", "SolarPanel name");
    assertTrue(sys.getFeatureName(components::VisualCoupling::ExteriorFeature::OreContainer) == "OreContainer", "OreContainer name");
    assertTrue(sys.getFeatureName(components::VisualCoupling::ExteriorFeature::CargoRack) == "CargoRack", "CargoRack name");
}

static void testVisualCouplingUpdate() {
    std::cout << "\n=== VisualCoupling: Update ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "frigate_01");
    sys.addCoupling("ship_1", "mod_1",
        components::VisualCoupling::ExteriorFeature::EngineBooster, 1.0f);
    sys.update(0.016f);
    sys.update(0.016f);
    auto* entity = world.getEntity("ship_1");
    auto* coupling = entity->getComponent<components::VisualCoupling>();
    assertTrue(coupling->total_updates == 2, "Update counter incremented");
}

static void testVisualCouplingMaxEntries() {
    std::cout << "\n=== VisualCoupling: Max Entries ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "titan_01");
    auto* entity = world.getEntity("ship_1");
    auto* coupling = entity->getComponent<components::VisualCoupling>();
    coupling->max_entries = 3;
    sys.addCoupling("ship_1", "a", components::VisualCoupling::ExteriorFeature::Vent, 1.0f);
    sys.addCoupling("ship_1", "b", components::VisualCoupling::ExteriorFeature::Vent, 1.0f);
    sys.addCoupling("ship_1", "c", components::VisualCoupling::ExteriorFeature::Vent, 1.0f);
    assertTrue(sys.getCouplingCount("ship_1") == 3, "3 at max");
    assertTrue(!sys.addCoupling("ship_1", "d",
        components::VisualCoupling::ExteriorFeature::Vent, 1.0f), "4th rejected");
}

static void testVisualCouplingMissing() {
    std::cout << "\n=== VisualCoupling: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    assertTrue(!sys.initializeCoupling("nonexistent", "s"), "Init fails on missing");
    assertTrue(!sys.addCoupling("nonexistent", "m",
        components::VisualCoupling::ExteriorFeature::Vent, 1.0f), "Add fails on missing");
    assertTrue(sys.getCouplingCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getVisibleCount("nonexistent") == 0, "Visible 0 on missing");
}


void run_visual_coupling_tests() {
    testVisualCouplingInit();
    testVisualCouplingAdd();
    testVisualCouplingRemove();
    testVisualCouplingVisibility();
    testVisualCouplingOffset();
    testVisualCouplingFeatureCount();
    testVisualCouplingFeatureNames();
    testVisualCouplingUpdate();
    testVisualCouplingMaxEntries();
    testVisualCouplingMissing();
}
