#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_DEPARTURE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_DEPARTURE_SYSTEM_H

#include "ecs/state_machine_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages organic captain departure flow
 *
 * Captains accumulate disagreement which causes them to grumble,
 * then formally request departure, and eventually leave.
 */
class CaptainDepartureSystem : public ecs::StateMachineSystem<components::CaptainDepartureState> {
public:
    explicit CaptainDepartureSystem(ecs::World* world);
    ~CaptainDepartureSystem() override = default;

    std::string getName() const override { return "CaptainDepartureSystem"; }

    // --- API ---
    void addDisagreement(const std::string& entity_id, float amount);
    components::CaptainDepartureState::DeparturePhase getDeparturePhase(const std::string& entity_id) const;
    void acknowledgeRequest(const std::string& entity_id);
    std::vector<std::string> getDepartingCaptains() const;

protected:
    void updateComponent(ecs::Entity& entity, components::CaptainDepartureState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_DEPARTURE_SYSTEM_H
