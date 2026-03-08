#ifndef NOVAFORGE_SYSTEMS_GAMEPLAY_LOOP_TRACKER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_GAMEPLAY_LOOP_TRACKER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * GameplayLoopTrackerSystem - tracks player progression through the core
 * gameplay loop: Docked -> Undocking -> Flying -> Mining -> Trading -> Combat -> Docking.
 *
 * Reads/Writes GameplayLoopTrackerState component.
 *
 * Design:
 *   - Maintains phase state machine with valid transitions.
 *   - Accumulates time-in-phase statistics.
 *   - Counts full loop completions (undock -> any activities -> dock).
 *   - Tracks milestone flags (has_undocked, has_mined, has_traded, has_fought).
 */
class GameplayLoopTrackerSystem : public ecs::SingleComponentSystem<components::GameplayLoopTrackerState> {
public:
    explicit GameplayLoopTrackerSystem(ecs::World* world);
    ~GameplayLoopTrackerSystem() override = default;

    std::string getName() const override { return "GameplayLoopTrackerSystem"; }

    /// Transition to a new phase. Returns false if transition is invalid.
    bool transitionTo(const std::string& entity_id, components::GameplayLoopTrackerState::LoopPhase phase);

    /// Query helpers
    components::GameplayLoopTrackerState::LoopPhase getCurrentPhase(const std::string& entity_id) const;
    int getLoopsCompleted(const std::string& entity_id) const;
    float getTimeInCurrentPhase(const std::string& entity_id) const;
    bool hasCompletedAllActivities(const std::string& entity_id) const;
    int getTotalUndocks(const std::string& entity_id) const;

    /// Check if a phase transition is valid
    static bool isValidTransition(components::GameplayLoopTrackerState::LoopPhase from,
                                  components::GameplayLoopTrackerState::LoopPhase to);

protected:
    void updateComponent(ecs::Entity& entity, components::GameplayLoopTrackerState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_GAMEPLAY_LOOP_TRACKER_SYSTEM_H
