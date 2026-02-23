#ifndef EVE_SYSTEMS_CREW_SYSTEM_H
#define EVE_SYSTEMS_CREW_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class CrewSystem : public ecs::System {
public:
    explicit CrewSystem(ecs::World* world);
    ~CrewSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CrewSystem"; }

    bool assignCrew(const std::string& ship_entity_id, const std::string& crew_entity_id, const std::string& room_id);
    bool removeCrew(const std::string& ship_entity_id, const std::string& crew_entity_id);
    int getCrewCount(const std::string& ship_entity_id) const;
    float getOverallEfficiency(const std::string& ship_entity_id) const;
    void setActivity(const std::string& crew_entity_id, components::CrewMember::Activity activity);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_CREW_SYSTEM_H
