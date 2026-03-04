// Tests for: SingleComponentSystem template base
#include "test_log.h"
#include "ecs/single_component_system.h"
#include "components/core_components.h"
#include "systems/capacitor_system.h"

using namespace atlas;

// ==================== Test system using SingleComponentSystem ====================

namespace {

/// Simple system that doubles a component field each tick — for testing the template.
class TestDoublerSystem : public ecs::SingleComponentSystem<components::Capacitor> {
public:
    using SingleComponentSystem::SingleComponentSystem;
    std::string getName() const override { return "TestDoublerSystem"; }
    int updateCalls = 0;
protected:
    void updateComponent(ecs::Entity& /*entity*/, components::Capacitor& cap, float /*dt*/) override {
        cap.capacitor *= 2.0f;
        ++updateCalls;
    }
};

} // anonymous namespace

// ==================== SingleComponentSystem Tests ====================

static void testSingleComponentSystemIterates() {
    std::cout << "\n=== SingleComponentSystem Iterates ===" << std::endl;

    ecs::World world;
    TestDoublerSystem sys(&world);

    auto* e1 = world.createEntity("ship1");
    auto* c1 = addComp<components::Capacitor>(e1);
    c1->capacitor = 10.0f;

    auto* e2 = world.createEntity("ship2");
    auto* c2 = addComp<components::Capacitor>(e2);
    c2->capacitor = 5.0f;

    sys.update(1.0f);

    assertTrue(approxEqual(c1->capacitor, 20.0f), "ship1 capacitor doubled to 20");
    assertTrue(approxEqual(c2->capacitor, 10.0f), "ship2 capacitor doubled to 10");
    assertTrue(sys.updateCalls == 2, "updateComponent called once per matching entity");
}

static void testSingleComponentSystemSkipsNonMatching() {
    std::cout << "\n=== SingleComponentSystem Skips Non-Matching ===" << std::endl;

    ecs::World world;
    TestDoublerSystem sys(&world);

    auto* e1 = world.createEntity("ship_with_cap");
    auto* c1 = addComp<components::Capacitor>(e1);
    c1->capacitor = 7.0f;

    // Entity without Capacitor component
    world.createEntity("ship_without_cap");

    sys.update(0.5f);

    assertTrue(approxEqual(c1->capacitor, 14.0f), "Only entity with Capacitor is updated");
    assertTrue(sys.updateCalls == 1, "updateComponent called once (only matching entity)");
}

static void testSingleComponentSystemGetComponentFor() {
    std::cout << "\n=== SingleComponentSystem getComponentFor ===" << std::endl;

    ecs::World world;
    TestDoublerSystem sys(&world);

    auto* e1 = world.createEntity("ship_a");
    auto* c1 = addComp<components::Capacitor>(e1);
    c1->capacitor = 42.0f;

    // Access through the system's protected helper (exposed via public test system)
    // We test indirectly through the actual migrated systems:
    systems::CapacitorSystem capSys(&world);
    float pct = capSys.getCapacitorPercentage("ship_a");
    // capacitor=42, max=100 (default) → 42%
    assertTrue(pct > 0.0f, "getCapacitorPercentage works via getComponentFor");

    float missing = capSys.getCapacitorPercentage("nonexistent");
    assertTrue(missing < 0.0f, "getComponentFor returns nullptr for missing entity");
}

static void testSingleComponentSystemEmptyWorld() {
    std::cout << "\n=== SingleComponentSystem Empty World ===" << std::endl;

    ecs::World world;
    TestDoublerSystem sys(&world);

    // Should not crash on empty world
    sys.update(1.0f);
    assertTrue(sys.updateCalls == 0, "No calls when world is empty");
}

static void testSingleComponentSystemGetName() {
    std::cout << "\n=== SingleComponentSystem getName ===" << std::endl;

    ecs::World world;
    TestDoublerSystem sys(&world);

    assertTrue(sys.getName() == "TestDoublerSystem", "getName returns derived class name");
}


void run_single_component_system_tests() {
    testSingleComponentSystemIterates();
    testSingleComponentSystemSkipsNonMatching();
    testSingleComponentSystemGetComponentFor();
    testSingleComponentSystemEmptyWorld();
    testSingleComponentSystemGetName();
}
