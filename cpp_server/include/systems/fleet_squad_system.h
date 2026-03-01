#ifndef NOVAFORGE_SYSTEMS_FLEET_SQUAD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_SQUAD_SYSTEM_H

#include "ecs/system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Group AI abstraction for fleet squads (Future Considerations)
 *
 * Manages squads within fleets: creation, membership, formations, roles.
 * Updates cohesion and effectiveness based on squad composition.
 */
class FleetSquadSystem : public ecs::System {
public:
    explicit FleetSquadSystem(ecs::World* world);
    ~FleetSquadSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FleetSquadSystem"; }

    // Commands
    bool createSquad(const std::string& entity_id, const std::string& squad_id,
                     const std::string& leader_id, components::FleetSquad::SquadRole role);
    bool dissolveSquad(const std::string& entity_id);
    bool addMember(const std::string& entity_id, const std::string& member_id);
    bool removeMember(const std::string& entity_id, const std::string& member_id);
    bool setFormation(const std::string& entity_id, components::FleetSquad::SquadFormation formation);
    bool setRole(const std::string& entity_id, components::FleetSquad::SquadRole role);
    bool setActive(const std::string& entity_id, bool active);

    // Query API
    int getMemberCount(const std::string& entity_id) const;
    std::string getLeaderId(const std::string& entity_id) const;
    std::string getRole(const std::string& entity_id) const;
    std::string getFormation(const std::string& entity_id) const;
    float getCohesion(const std::string& entity_id) const;
    float getEffectiveness(const std::string& entity_id) const;
    bool isSquadActive(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_SQUAD_SYSTEM_H
