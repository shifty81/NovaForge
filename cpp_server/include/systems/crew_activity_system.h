#ifndef NOVAFORGE_SYSTEMS_CREW_ACTIVITY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CREW_ACTIVITY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages crew member activity transitions based on ship state and needs
 *
 * Crew members transition between activities: Working, Walking, Resting,
 * Eating, Repairing, Manning based on fatigue, hunger, and ship damage.
 */
class CrewActivitySystem : public ecs::SingleComponentSystem<components::CrewActivity> {
public:
    explicit CrewActivitySystem(ecs::World* world);
    ~CrewActivitySystem() override = default;

    std::string getName() const override { return "CrewActivitySystem"; }

protected:
    void updateComponent(ecs::Entity& entity, components::CrewActivity& crew, float delta_time) override;

public:
    // --- API ---
    void assignRoom(const std::string& crew_entity_id, const std::string& room_id);
    void setShipDamaged(const std::string& crew_entity_id, bool damaged);
    std::string getActivity(const std::string& crew_entity_id) const;
    std::string getAssignedRoom(const std::string& crew_entity_id) const;
    float getFatigue(const std::string& crew_entity_id) const;
    float getHunger(const std::string& crew_entity_id) const;
    std::vector<std::string> getCrewInActivity(components::CrewActivity::Activity activity) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CREW_ACTIVITY_SYSTEM_H
