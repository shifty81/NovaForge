#ifndef NOVAFORGE_SYSTEMS_HANGAR_TRANSITION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_HANGAR_TRANSITION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * HangarTransitionSystem - animated docking/undocking transitions
 * (Elite Dangerous style).
 *
 * Reads/Writes HangarTransitionState component.
 *
 * Design:
 *   - Phase state machine: Idle -> DockApproach -> DockSequence -> DockComplete
 *                          DockComplete -> UndockSequence -> UndockLaunch -> UndockComplete -> Idle
 *   - Each phase has a configurable duration. animation_progress tracks 0→1.
 *   - Auto-advances phases when the timer exceeds the phase duration.
 */
class HangarTransitionSystem : public ecs::SingleComponentSystem<components::HangarTransitionState> {
public:
    explicit HangarTransitionSystem(ecs::World* world);
    ~HangarTransitionSystem() override = default;

    std::string getName() const override { return "HangarTransitionSystem"; }

    /// Begin docking from Idle state. Returns false if not idle.
    bool beginDock(const std::string& entity_id, const std::string& station_id);

    /// Begin undocking from DockComplete state. Returns false if not docked.
    bool beginUndock(const std::string& entity_id);

    /// Query helpers
    components::HangarTransitionState::TransitionPhase getPhase(const std::string& entity_id) const;
    float getAnimationProgress(const std::string& entity_id) const;
    bool isDocking(const std::string& entity_id) const;
    bool isUndocking(const std::string& entity_id) const;
    bool isIdle(const std::string& entity_id) const;
    bool isDocked(const std::string& entity_id) const;
    int getTotalDocks(const std::string& entity_id) const;
    int getTotalUndocks(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::HangarTransitionState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_HANGAR_TRANSITION_SYSTEM_H
