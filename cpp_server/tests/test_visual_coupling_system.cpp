// Tests for: VisualCouplingSystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/visual_coupling_system.h"

using namespace atlas;

// ==================== VisualCouplingSystem Tests ====================

static void testVisualCouplingInitialize() {
    std::cout << "\n=== VisualCoupling: Initialize ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeCoupling("ship1", "frigate_01"), "Initialize coupling");
    assertTrue(sys.getCouplingCount("ship1") == 0, "No couplings initially");
    assertTrue(sys.getVisibleCount("ship1") == 0, "No visible initially");
}

static void testVisualCouplingDuplicateInitRejected() {
    std::cout << "\n=== VisualCoupling: DuplicateInitRejected ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.initializeCoupling("ship1", "frigate_01"), "First init ok");
    assertTrue(!sys.initializeCoupling("ship1", "cruiser_02"), "Duplicate init rejected");
}

static void testVisualCouplingAddCoupling() {
    std::cout << "\n=== VisualCoupling: AddCoupling ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    assertTrue(sys.addCoupling("ship1", "solar_mod_1",
               components::VisualCoupling::ExteriorFeature::SolarPanel, 1.0f),
               "Add solar panel coupling");
    assertTrue(sys.getCouplingCount("ship1") == 1, "Coupling count is 1");
    assertTrue(sys.getVisibleCount("ship1") == 1, "Visible count is 1");
}

static void testVisualCouplingAddMultiple() {
    std::cout << "\n=== VisualCoupling: AddMultiple ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    using EF = components::VisualCoupling::ExteriorFeature;
    sys.addCoupling("ship1", "solar_1", EF::SolarPanel, 1.0f);
    sys.addCoupling("ship1", "ore_1", EF::OreContainer, 1.5f);
    sys.addCoupling("ship1", "vent_1", EF::Vent, 0.8f);

    assertTrue(sys.getCouplingCount("ship1") == 3, "Coupling count is 3");
    assertTrue(sys.getVisibleCount("ship1") == 3, "All visible");
}

static void testVisualCouplingRejectDuplicate() {
    std::cout << "\n=== VisualCoupling: RejectDuplicate ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    using EF = components::VisualCoupling::ExteriorFeature;
    sys.addCoupling("ship1", "solar_1", EF::SolarPanel, 1.0f);
    assertTrue(!sys.addCoupling("ship1", "solar_1", EF::Antenna, 2.0f),
               "Duplicate module_id rejected");
    assertTrue(sys.getCouplingCount("ship1") == 1, "Still 1 coupling");
}

static void testVisualCouplingMaxEntriesRespected() {
    std::cout << "\n=== VisualCoupling: MaxEntriesRespected ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    auto* coupling = e->getComponent<components::VisualCoupling>();
    coupling->max_entries = 2;

    using EF = components::VisualCoupling::ExteriorFeature;
    sys.addCoupling("ship1", "mod_a", EF::SolarPanel, 1.0f);
    sys.addCoupling("ship1", "mod_b", EF::Vent, 1.0f);
    assertTrue(!sys.addCoupling("ship1", "mod_c", EF::Antenna, 1.0f),
               "Cannot exceed max entries");
    assertTrue(sys.getCouplingCount("ship1") == 2, "Still 2 entries");
}

static void testVisualCouplingRemove() {
    std::cout << "\n=== VisualCoupling: Remove ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    using EF = components::VisualCoupling::ExteriorFeature;
    sys.addCoupling("ship1", "solar_1", EF::SolarPanel, 1.0f);

    assertTrue(sys.removeCoupling("ship1", "solar_1"), "Remove coupling");
    assertTrue(sys.getCouplingCount("ship1") == 0, "Coupling count is 0");
}

static void testVisualCouplingRemoveNonexistent() {
    std::cout << "\n=== VisualCoupling: RemoveNonexistent ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    assertTrue(!sys.removeCoupling("ship1", "ghost_mod"), "Cannot remove nonexistent coupling");
}

static void testVisualCouplingSetVisibility() {
    std::cout << "\n=== VisualCoupling: SetVisibility ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    using EF = components::VisualCoupling::ExteriorFeature;
    sys.addCoupling("ship1", "solar_1", EF::SolarPanel, 1.0f);
    sys.addCoupling("ship1", "ore_1", EF::OreContainer, 1.5f);

    assertTrue(sys.setVisibility("ship1", "solar_1", false), "Set solar invisible");
    assertTrue(sys.getVisibleCount("ship1") == 1, "Only 1 visible");

    assertTrue(sys.setVisibility("ship1", "solar_1", true), "Set solar visible again");
    assertTrue(sys.getVisibleCount("ship1") == 2, "Both visible again");
}

static void testVisualCouplingSetOffset() {
    std::cout << "\n=== VisualCoupling: SetOffset ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    using EF = components::VisualCoupling::ExteriorFeature;
    sys.addCoupling("ship1", "solar_1", EF::SolarPanel, 1.0f);

    assertTrue(sys.setOffset("ship1", "solar_1", 1.0f, 2.0f, 3.0f), "Set offset");

    auto* coupling = e->getComponent<components::VisualCoupling>();
    auto* entry = coupling->findEntry("solar_1");
    assertTrue(entry != nullptr, "Entry found");
    assertTrue(approxEqual(entry->x_offset, 1.0f), "X offset set");
    assertTrue(approxEqual(entry->y_offset, 2.0f), "Y offset set");
    assertTrue(approxEqual(entry->z_offset, 3.0f), "Z offset set");
}

static void testVisualCouplingSetOffsetNonexistent() {
    std::cout << "\n=== VisualCoupling: SetOffsetNonexistent ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    assertTrue(!sys.setOffset("ship1", "ghost", 1.0f, 2.0f, 3.0f), "Cannot set offset for nonexistent");
}

static void testVisualCouplingFeatureCount() {
    std::cout << "\n=== VisualCoupling: FeatureCount ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    using EF = components::VisualCoupling::ExteriorFeature;
    sys.addCoupling("ship1", "solar_1", EF::SolarPanel, 1.0f);
    sys.addCoupling("ship1", "solar_2", EF::SolarPanel, 1.0f);
    sys.addCoupling("ship1", "ore_1", EF::OreContainer, 1.5f);

    assertTrue(sys.getFeatureCount("ship1", EF::SolarPanel) == 2, "2 solar panels");
    assertTrue(sys.getFeatureCount("ship1", EF::OreContainer) == 1, "1 ore container");
    assertTrue(sys.getFeatureCount("ship1", EF::WeaponMount) == 0, "0 weapon mounts");
}

static void testVisualCouplingGetFeatureName() {
    std::cout << "\n=== VisualCoupling: GetFeatureName ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);

    using EF = components::VisualCoupling::ExteriorFeature;
    assertTrue(sys.getFeatureName(EF::SolarPanel) == "SolarPanel", "SolarPanel name");
    assertTrue(sys.getFeatureName(EF::OreContainer) == "OreContainer", "OreContainer name");
    assertTrue(sys.getFeatureName(EF::Vent) == "Vent", "Vent name");
    assertTrue(sys.getFeatureName(EF::Antenna) == "Antenna", "Antenna name");
    assertTrue(sys.getFeatureName(EF::WeaponMount) == "WeaponMount", "WeaponMount name");
    assertTrue(sys.getFeatureName(EF::ShieldEmitter) == "ShieldEmitter", "ShieldEmitter name");
    assertTrue(sys.getFeatureName(EF::EngineBooster) == "EngineBooster", "EngineBooster name");
    assertTrue(sys.getFeatureName(EF::CargoRack) == "CargoRack", "CargoRack name");
}

static void testVisualCouplingAutoUpdate() {
    std::cout << "\n=== VisualCoupling: AutoUpdate ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initializeCoupling("ship1", "frigate_01");

    auto* coupling = e->getComponent<components::VisualCoupling>();
    assertTrue(coupling->auto_update, "Auto update enabled by default");

    sys.update(0.016f);
    assertTrue(coupling->total_updates == 1, "Update counter incremented");

    coupling->auto_update = false;
    sys.update(0.016f);
    assertTrue(coupling->total_updates == 1, "No update when auto_update is false");
}

static void testVisualCouplingMissingEntity() {
    std::cout << "\n=== VisualCoupling: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);

    using EF = components::VisualCoupling::ExteriorFeature;
    assertTrue(!sys.initializeCoupling("ghost", "ship"), "Init fails for missing entity");
    assertTrue(!sys.addCoupling("ghost", "m", EF::SolarPanel, 1.0f), "Add fails for missing");
    assertTrue(!sys.removeCoupling("ghost", "m"), "Remove fails for missing");
    assertTrue(!sys.setVisibility("ghost", "m", true), "setVisibility fails for missing");
    assertTrue(!sys.setOffset("ghost", "m", 0, 0, 0), "setOffset fails for missing");
    assertTrue(sys.getCouplingCount("ghost") == 0, "Count 0 for missing");
    assertTrue(sys.getVisibleCount("ghost") == 0, "Visible 0 for missing");
    assertTrue(sys.getFeatureCount("ghost", EF::SolarPanel) == 0, "Feature count 0 for missing");
}

void run_visual_coupling_system_tests() {
    testVisualCouplingInitialize();
    testVisualCouplingDuplicateInitRejected();
    testVisualCouplingAddCoupling();
    testVisualCouplingAddMultiple();
    testVisualCouplingRejectDuplicate();
    testVisualCouplingMaxEntriesRespected();
    testVisualCouplingRemove();
    testVisualCouplingRemoveNonexistent();
    testVisualCouplingSetVisibility();
    testVisualCouplingSetOffset();
    testVisualCouplingSetOffsetNonexistent();
    testVisualCouplingFeatureCount();
    testVisualCouplingGetFeatureName();
    testVisualCouplingAutoUpdate();
    testVisualCouplingMissingEntity();
}
