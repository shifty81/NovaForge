#ifndef NOVAFORGE_SYSTEMS_CREW_MANAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CREW_MANAGEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship crew management — hiring, dismissal, morale and efficiency
 *
 * Manages a ship's crew roster.  Crew members have roles, skill levels,
 * and morale that fluctuates over time.  Average morale drives an
 * efficiency multiplier affecting ship performance.  Salaries are paid
 * periodically from the ship owner's wallet.
 */
class CrewManagementSystem : public ecs::SingleComponentSystem<components::CrewManagement> {
public:
    explicit CrewManagementSystem(ecs::World* world);
    ~CrewManagementSystem() override = default;

    std::string getName() const override { return "CrewManagementSystem"; }

public:
    bool initialize(const std::string& entity_id, int max_crew);
    bool hireCrew(const std::string& entity_id, const std::string& name,
                  const std::string& role, int skill_level, float salary);
    bool dismissCrew(const std::string& entity_id, const std::string& name);
    bool assignCrew(const std::string& entity_id, const std::string& name);
    bool unassignCrew(const std::string& entity_id, const std::string& name);
    bool adjustMorale(const std::string& entity_id, const std::string& name, float delta);
    int getCrewCount(const std::string& entity_id) const;
    int getAssignedCount(const std::string& entity_id) const;
    float getAverageMorale(const std::string& entity_id) const;
    float getEfficiencyMultiplier(const std::string& entity_id) const;
    double getTotalSalaryPaid(const std::string& entity_id) const;
    int getTotalHired(const std::string& entity_id) const;
    int getTotalDismissed(const std::string& entity_id) const;
    std::string getMoraleLevel(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CrewManagement& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CREW_MANAGEMENT_SYSTEM_H
