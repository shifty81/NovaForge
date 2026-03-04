#ifndef NOVAFORGE_SYSTEMS_SPACE_PLANET_TRANSITION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SPACE_PLANET_TRANSITION_SYSTEM_H

#include "ecs/state_machine_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Space-planet transition system (Phase 14)
 *
 * Manages seamless zoom from orbit to surface with multi-phase transition
 * state machine, atmospheric heating, and gravity changes.
 */
class SpacePlanetTransitionSystem : public ecs::StateMachineSystem<components::SpacePlanetTransition> {
public:
    explicit SpacePlanetTransitionSystem(ecs::World* world);
    ~SpacePlanetTransitionSystem() override = default;

    std::string getName() const override { return "SpacePlanetTransitionSystem"; }

    // Initialization
    bool initializeTransition(const std::string& entity_id, const std::string& planet_id,
                              bool has_atmosphere);

    // Transition control
    bool beginDescent(const std::string& entity_id);
    bool beginLaunch(const std::string& entity_id);
    bool abortTransition(const std::string& entity_id);
    bool setAutopilot(const std::string& entity_id, bool enabled);
    bool setLandingTarget(const std::string& entity_id, float x, float y);

    // Query
    float getAltitude(const std::string& entity_id) const;
    float getHeatLevel(const std::string& entity_id) const;
    std::string getTransitionState(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SpacePlanetTransition& tr, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SPACE_PLANET_TRANSITION_SYSTEM_H
