// Tests for: Interior-Exterior Coupling Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Interior-Exterior Coupling Tests ====================

static void testInteriorExteriorDefaults() {
    std::cout << "\n=== Interior-Exterior Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("link1");
    auto* link = addComp<components::InteriorExteriorLink>(e);
    assertTrue(link->effects.empty(), "No effects");
    assertTrue(link->visible_module_count == 0, "No visible modules");
    assertTrue(approxEqual(link->total_hull_deformation, 0.0f), "No hull deformation");
}

static void testInteriorExteriorAddEffect() {
    std::cout << "\n=== Interior-Exterior Add Effect ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("link2");
    auto* link = addComp<components::InteriorExteriorLink>(e);
    link->addEffect("refinery", 0.3f, true, 1.5f);
    link->addEffect("cargo_rack", 0.1f, false, 1.0f);
    assertTrue(static_cast<int>(link->effects.size()) == 2, "Two effects");
    assertTrue(link->visible_module_count == 1, "One visible module");
    assertTrue(approxEqual(link->total_hull_deformation, 0.4f), "Total deformation 0.4");
}

static void testInteriorExteriorClear() {
    std::cout << "\n=== Interior-Exterior Clear ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("link3");
    auto* link = addComp<components::InteriorExteriorLink>(e);
    link->addEffect("solar_panel", 0.2f, true, 1.0f);
    link->clearEffects();
    assertTrue(link->effects.empty(), "Effects cleared");
    assertTrue(link->visible_module_count == 0, "Visible count reset");
    assertTrue(approxEqual(link->total_hull_deformation, 0.0f), "Deformation reset");
}


void run_interior_exterior_coupling_tests() {
    testInteriorExteriorDefaults();
    testInteriorExteriorAddEffect();
    testInteriorExteriorClear();
}
