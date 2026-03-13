// Tests for: DensityFieldSystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/density_field_system.h"

using namespace atlas;

// ==================== DensityFieldSystem Tests ====================

static void testDensityFieldInit() {
    std::cout << "\n=== DensityField: Init ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.get_voxel_count("e1") == 0, "No voxels initially");
    assertTrue(approxEqual(sys.get_iso_value("e1"), 0.5f), "Default iso 0.5");
    assertTrue(sys.is_symmetric_x("e1"), "Default symmetry_x is true");
    assertTrue(sys.get_total_updates("e1") == 0, "Zero updates");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testDensityFieldSetGet() {
    std::cout << "\n=== DensityField: SetGet ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.set_voxel("e1", 0, 0, 0, 0.8f), "Set voxel at origin");
    assertTrue(approxEqual(sys.get_voxel("e1", 0, 0, 0), 0.8f), "Get returns 0.8");
    assertTrue(sys.get_voxel_count("e1") == 1, "One voxel");
    assertTrue(sys.get_total_updates("e1") == 1, "One update");

    // Set another
    assertTrue(sys.set_voxel("e1", 1, 0, 0, 0.3f), "Set voxel at (1,0,0)");
    assertTrue(approxEqual(sys.get_voxel("e1", 1, 0, 0), 0.3f), "Get 0.3");
    assertTrue(sys.get_voxel_count("e1") == 2, "Two voxels");
}

static void testDensityFieldClamp() {
    std::cout << "\n=== DensityField: Clamp ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.set_voxel("e1", 0, 0, 0, 2.0f), "Set >1 clamped");
    assertTrue(approxEqual(sys.get_voxel("e1", 0, 0, 0), 1.0f), "Clamped to 1.0");
    assertTrue(sys.set_voxel("e1", 1, 0, 0, -0.5f), "Set <0 clamped");
    assertTrue(approxEqual(sys.get_voxel("e1", 1, 0, 0), 0.0f), "Clamped to 0.0");
}

static void testDensityFieldUpdate() {
    std::cout << "\n=== DensityField: Update ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Update existing voxel
    sys.set_voxel("e1", 0, 0, 0, 0.5f);
    assertTrue(sys.set_voxel("e1", 0, 0, 0, 0.9f), "Update existing voxel");
    assertTrue(approxEqual(sys.get_voxel("e1", 0, 0, 0), 0.9f), "Updated to 0.9");
    assertTrue(sys.get_voxel_count("e1") == 1, "Still one voxel (updated, not added)");
    assertTrue(sys.get_total_updates("e1") == 2, "Two updates total");
}

static void testDensityFieldClear() {
    std::cout << "\n=== DensityField: Clear ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.set_voxel("e1", 0, 0, 0, 0.5f);
    sys.set_voxel("e1", 1, 0, 0, 0.6f);

    assertTrue(sys.clear_field("e1"), "Clear field");
    assertTrue(sys.get_voxel_count("e1") == 0, "Zero voxels after clear");
    assertTrue(approxEqual(sys.get_voxel("e1", 0, 0, 0), 0.0f), "Returns 0.0 after clear");
}

static void testDensityFieldIsoSymmetry() {
    std::cout << "\n=== DensityField: IsoSymmetry ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.set_iso_value("e1", 0.7f), "Set iso 0.7");
    assertTrue(approxEqual(sys.get_iso_value("e1"), 0.7f), "Iso is 0.7");

    assertTrue(sys.set_symmetry("e1", false, true, true), "Set symmetry");
    assertTrue(!sys.is_symmetric_x("e1"), "Symmetry X off");
}

static void testDensityFieldSmooth() {
    std::cout << "\n=== DensityField: Smooth ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.set_voxel("e1", 0, 0, 0, 1.0f);
    sys.set_voxel("e1", 1, 0, 0, 0.0f);
    assertTrue(sys.apply_smooth("e1"), "Smooth succeeds");

    // After smoothing, neighbors should average each other
    float v0 = sys.get_voxel("e1", 0, 0, 0);
    float v1 = sys.get_voxel("e1", 1, 0, 0);
    assertTrue(approxEqual(v0, 0.5f), "Origin smoothed to ~0.5");
    assertTrue(approxEqual(v1, 0.5f), "Neighbor smoothed to ~0.5");
}

static void testDensityFieldSmoothEmpty() {
    std::cout << "\n=== DensityField: SmoothEmpty ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.apply_smooth("e1"), "Smooth on empty field succeeds");
}

static void testDensityFieldGetNonexistent() {
    std::cout << "\n=== DensityField: GetNonexistent ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(approxEqual(sys.get_voxel("e1", 99, 99, 99), 0.0f), "Returns 0 for nonexistent");
}

static void testDensityFieldTick() {
    std::cout << "\n=== DensityField: Tick ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.set_voxel("e1", 0, 0, 0, 0.5f);
    sys.update(0.016f);
    assertTrue(approxEqual(sys.get_voxel("e1", 0, 0, 0), 0.5f), "Voxel unchanged by tick");
}

static void testDensityFieldMissing() {
    std::cout << "\n=== DensityField: Missing ===" << std::endl;
    ecs::World world;
    systems::DensityFieldSystem sys(&world);

    assertTrue(!sys.set_voxel("no", 0, 0, 0, 0.5f), "set_voxel fails");
    assertTrue(approxEqual(sys.get_voxel("no", 0, 0, 0), 0.0f), "get_voxel default");
    assertTrue(!sys.clear_field("no"), "clear_field fails");
    assertTrue(!sys.apply_smooth("no"), "apply_smooth fails");
    assertTrue(!sys.set_iso_value("no", 0.5f), "set_iso_value fails");
    assertTrue(!sys.set_symmetry("no", true, true, true), "set_symmetry fails");
    assertTrue(sys.get_voxel_count("no") == 0, "get_voxel_count default");
    assertTrue(approxEqual(sys.get_iso_value("no"), 0.0f), "get_iso_value default");
    assertTrue(!sys.is_symmetric_x("no"), "is_symmetric_x default");
    assertTrue(sys.get_total_updates("no") == 0, "get_total_updates default");
}

void run_density_field_system_tests() {
    testDensityFieldInit();
    testDensityFieldSetGet();
    testDensityFieldClamp();
    testDensityFieldUpdate();
    testDensityFieldClear();
    testDensityFieldIsoSymmetry();
    testDensityFieldSmooth();
    testDensityFieldSmoothEmpty();
    testDensityFieldGetNonexistent();
    testDensityFieldTick();
    testDensityFieldMissing();
}
