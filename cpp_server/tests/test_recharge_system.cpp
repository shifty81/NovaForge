// Tests for: RechargeSystem template base
#include "test_log.h"
#include "ecs/recharge_system.h"
#include "components/core_components.h"

using namespace atlas;

// ==================== Concrete test system using RechargeSystem ====================

namespace {

/// Simple recharge system operating on the Capacitor component.
class TestRechargeCapSystem
    : public ecs::RechargeSystem<components::Capacitor> {
public:
    using RechargeSystem::RechargeSystem;
    std::string getName() const override { return "TestRechargeCapSystem"; }
    int afterCalls = 0;
protected:
    float& current(components::Capacitor& c) override { return c.capacitor; }
    float max(const components::Capacitor& c) const override { return c.capacitor_max; }
    float rate(const components::Capacitor& c) const override { return c.recharge_rate; }
    void onAfterRecharge(ecs::Entity& /*e*/, components::Capacitor& /*c*/, float /*dt*/) override {
        ++afterCalls;
    }
};

} // anonymous namespace

// ==================== RechargeSystem Tests ====================

static void testRechargeSystemAppliesRecharge() {
    std::cout << "\n=== RechargeSystem Applies Recharge ===" << std::endl;

    ecs::World world;
    TestRechargeCapSystem sys(&world);

    auto* e = world.createEntity("ship1");
    auto* cap = addComp<components::Capacitor>(e);
    cap->capacitor = 50.0f;
    cap->capacitor_max = 100.0f;
    cap->recharge_rate = 10.0f;

    sys.update(1.0f);  // 10 * 1.0 = +10

    assertTrue(approxEqual(cap->capacitor, 60.0f), "Capacitor recharged by rate*dt");
}

static void testRechargeSystemClampsToMax() {
    std::cout << "\n=== RechargeSystem Clamps To Max ===" << std::endl;

    ecs::World world;
    TestRechargeCapSystem sys(&world);

    auto* e = world.createEntity("ship1");
    auto* cap = addComp<components::Capacitor>(e);
    cap->capacitor = 95.0f;
    cap->capacitor_max = 100.0f;
    cap->recharge_rate = 20.0f;

    sys.update(1.0f);  // Would be 115, but clamped to 100

    assertTrue(approxEqual(cap->capacitor, 100.0f), "Capacitor clamped to max");
}

static void testRechargeSystemSkipsFullCapacitor() {
    std::cout << "\n=== RechargeSystem Skips Full Capacitor ===" << std::endl;

    ecs::World world;
    TestRechargeCapSystem sys(&world);

    auto* e = world.createEntity("ship1");
    auto* cap = addComp<components::Capacitor>(e);
    cap->capacitor = 100.0f;
    cap->capacitor_max = 100.0f;
    cap->recharge_rate = 10.0f;

    sys.update(1.0f);

    assertTrue(approxEqual(cap->capacitor, 100.0f), "Full capacitor unchanged");
    assertTrue(sys.afterCalls == 1, "onAfterRecharge still called even when full");
}

static void testRechargeSystemCallsAfterHook() {
    std::cout << "\n=== RechargeSystem Calls onAfterRecharge ===" << std::endl;

    ecs::World world;
    TestRechargeCapSystem sys(&world);

    auto* e1 = world.createEntity("s1");
    auto* c1 = addComp<components::Capacitor>(e1);
    c1->capacitor = 10.0f; c1->capacitor_max = 100.0f; c1->recharge_rate = 5.0f;

    auto* e2 = world.createEntity("s2");
    auto* c2 = addComp<components::Capacitor>(e2);
    c2->capacitor = 20.0f; c2->capacitor_max = 100.0f; c2->recharge_rate = 5.0f;

    sys.update(1.0f);

    assertTrue(sys.afterCalls == 2, "onAfterRecharge called once per entity");
}

static void testRechargeSystemEmptyWorld() {
    std::cout << "\n=== RechargeSystem Empty World ===" << std::endl;

    ecs::World world;
    TestRechargeCapSystem sys(&world);

    sys.update(1.0f);
    assertTrue(sys.afterCalls == 0, "No calls when world is empty");
}

static void testRechargeSystemGetName() {
    std::cout << "\n=== RechargeSystem getName ===" << std::endl;

    ecs::World world;
    TestRechargeCapSystem sys(&world);
    assertTrue(sys.getName() == "TestRechargeCapSystem", "getName returns derived class name");
}


void run_recharge_system_tests() {
    testRechargeSystemAppliesRecharge();
    testRechargeSystemClampsToMax();
    testRechargeSystemSkipsFullCapacitor();
    testRechargeSystemCallsAfterHook();
    testRechargeSystemEmptyWorld();
    testRechargeSystemGetName();
}
