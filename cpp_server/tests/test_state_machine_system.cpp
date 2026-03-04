// Tests for: StateMachineSystem template base
#include "test_log.h"
#include "ecs/state_machine_system.h"
#include "components/ship_components.h"
#include "components/navigation_components.h"

using namespace atlas;

// ==================== StateMachineSystem Tests ====================

static void testStateMachineSystemInheritsIteration() {
    std::cout << "\n=== StateMachineSystem Inherits Iteration ===" << std::endl;

    // Verify that StateMachineSystem properly inherits the entity-iteration
    // loop from SingleComponentSystem via CloakingState components
    ecs::World world;

    // A concrete test system using StateMachineSystem
    class TestPhaseSystem : public ecs::StateMachineSystem<components::CloakingState> {
    public:
        using StateMachineSystem::StateMachineSystem;
        std::string getName() const override { return "TestPhaseSystem"; }
        int calls = 0;
    protected:
        void updateComponent(ecs::Entity& /*entity*/, components::CloakingState& cloak, float dt) override {
            cloak.phase_timer += dt;
            ++calls;
        }
    };

    TestPhaseSystem sys(&world);

    auto* e1 = world.createEntity("ship1");
    auto* c1 = addComp<components::CloakingState>(e1);
    c1->phase_timer = 0.0f;

    auto* e2 = world.createEntity("ship2");
    auto* c2 = addComp<components::CloakingState>(e2);
    c2->phase_timer = 1.0f;

    // Entity without CloakingState — should be skipped
    world.createEntity("ship_no_cloak");

    sys.update(0.5f);

    assertTrue(sys.calls == 2, "StateMachineSystem iterates only matching entities");
    assertTrue(approxEqual(c1->phase_timer, 0.5f), "ship1 phase_timer advanced");
    assertTrue(approxEqual(c2->phase_timer, 1.5f), "ship2 phase_timer advanced");
}

static void testStateMachineSystemGetComponentFor() {
    std::cout << "\n=== StateMachineSystem getComponentFor ===" << std::endl;

    // Verify that getComponentFor works for state machine systems
    // Tested indirectly through CloakingSystem
    ecs::World world;

    class TestQuerySystem : public ecs::StateMachineSystem<components::CloakingState> {
    public:
        using StateMachineSystem::StateMachineSystem;
        std::string getName() const override { return "TestQuerySystem"; }

        // Expose getComponentFor for testing
        components::CloakingState* lookup(const std::string& id) {
            return getComponentFor(id);
        }
        const components::CloakingState* lookupConst(const std::string& id) const {
            return getComponentFor(id);
        }
    protected:
        void updateComponent(ecs::Entity&, components::CloakingState&, float) override {}
    };

    TestQuerySystem sys(&world);

    auto* e1 = world.createEntity("ship_a");
    auto* c1 = addComp<components::CloakingState>(e1);
    c1->fuel_per_second = 5.0f;

    auto* found = sys.lookup("ship_a");
    assertTrue(found != nullptr, "getComponentFor finds existing entity");
    assertTrue(approxEqual(found->fuel_per_second, 5.0f), "getComponentFor returns correct component");

    auto* missing = sys.lookup("nonexistent");
    assertTrue(missing == nullptr, "getComponentFor returns nullptr for missing entity");

    const auto* constFound = sys.lookupConst("ship_a");
    assertTrue(constFound != nullptr, "const getComponentFor works");
}

static void testStateMachineSystemEmptyWorld() {
    std::cout << "\n=== StateMachineSystem Empty World ===" << std::endl;

    ecs::World world;

    class TestEmptySystem : public ecs::StateMachineSystem<components::JumpDriveState> {
    public:
        using StateMachineSystem::StateMachineSystem;
        std::string getName() const override { return "TestEmptySystem"; }
        int calls = 0;
    protected:
        void updateComponent(ecs::Entity&, components::JumpDriveState&, float) override {
            ++calls;
        }
    };

    TestEmptySystem sys(&world);
    sys.update(1.0f);
    assertTrue(sys.calls == 0, "No calls on empty world");
}

static void testStateMachineSystemGetName() {
    std::cout << "\n=== StateMachineSystem getName ===" << std::endl;

    ecs::World world;

    class TestNamedSystem : public ecs::StateMachineSystem<components::CloakingState> {
    public:
        using StateMachineSystem::StateMachineSystem;
        std::string getName() const override { return "TestNamedStateMachine"; }
    protected:
        void updateComponent(ecs::Entity&, components::CloakingState&, float) override {}
    };

    TestNamedSystem sys(&world);
    assertTrue(sys.getName() == "TestNamedStateMachine", "getName returns derived class name");
}


void run_state_machine_system_tests() {
    testStateMachineSystemInheritsIteration();
    testStateMachineSystemGetComponentFor();
    testStateMachineSystemEmptyWorld();
    testStateMachineSystemGetName();
}
