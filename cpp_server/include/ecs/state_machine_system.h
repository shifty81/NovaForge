#ifndef NOVAFORGE_ECS_STATE_MACHINE_SYSTEM_H
#define NOVAFORGE_ECS_STATE_MACHINE_SYSTEM_H

#include "single_component_system.h"
#include <string>

namespace atlas {
namespace ecs {

/**
 * @brief Template base for systems that manage a phase-driven state machine
 *        stored in a single component type.
 *
 * Extends SingleComponentSystem<C> with no additional virtual methods — the
 * value is semantic: it marks a system as owning a state-machine loop, and
 * it inherits the entity-iteration + getComponentFor() helpers that eliminate
 * the repeated entity-lookup boilerplate in query/command methods.
 *
 * Derived classes implement:
 *   - void updateComponent(Entity& entity, C& component, float delta_time)
 *     (the per-entity state machine tick)
 *   - std::string getName() const override
 *
 * The component type C is expected to have at least:
 *   - A phase/state enum field
 *   - A float phase_timer field (used for timed transitions)
 *
 * Example usage:
 * @code
 *   class CloakingSystem
 *       : public ecs::StateMachineSystem<components::CloakingState> {
 *   public:
 *       using StateMachineSystem::StateMachineSystem;
 *       std::string getName() const override { return "CloakingSystem"; }
 *   protected:
 *       void updateComponent(Entity& e, components::CloakingState& c,
 *                            float dt) override {
 *           switch (c.phase) { ... }
 *       }
 *   };
 * @endcode
 */
template<typename C>
class StateMachineSystem : public SingleComponentSystem<C> {
public:
    explicit StateMachineSystem(World* world) : SingleComponentSystem<C>(world) {}
    ~StateMachineSystem() override = default;
};

} // namespace ecs
} // namespace atlas

#endif // NOVAFORGE_ECS_STATE_MACHINE_SYSTEM_H
