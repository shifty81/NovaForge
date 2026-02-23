#ifndef EVE_SYSTEMS_CAPTAIN_DEPARTURE_SYSTEM_H
#define EVE_SYSTEMS_CAPTAIN_DEPARTURE_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
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
class CaptainDepartureSystem : public ecs::System {
public:
    explicit CaptainDepartureSystem(ecs::World* world);
    ~CaptainDepartureSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CaptainDepartureSystem"; }

    // --- API ---
    void addDisagreement(const std::string& entity_id, float amount);
    components::CaptainDepartureState::DeparturePhase getDeparturePhase(const std::string& entity_id) const;
    void acknowledgeRequest(const std::string& entity_id);
    std::vector<std::string> getDepartingCaptains() const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_CAPTAIN_DEPARTURE_SYSTEM_H
