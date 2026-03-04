#ifndef NOVAFORGE_SYSTEMS_CLONE_BAY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CLONE_BAY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Clone and medical bay management system
 *
 * Manages clone installation, activation, implant fitting, and
 * death processing with skill point loss based on clone grade.
 */
class CloneBaySystem : public ecs::SingleComponentSystem<components::CloneBay> {
public:
    explicit CloneBaySystem(ecs::World* world);
    ~CloneBaySystem() override = default;

    std::string getName() const override { return "CloneBaySystem"; }

    bool initialize(const std::string& entity_id, const std::string& clone_bay_id,
                    const std::string& station_id);
    bool addClone(const std::string& entity_id, const std::string& clone_id, int grade);
    bool removeClone(const std::string& entity_id, const std::string& clone_id);
    bool activateClone(const std::string& entity_id, const std::string& clone_id);
    bool installImplant(const std::string& entity_id, const std::string& implant_id,
                        int slot, const std::string& attribute, float bonus,
                        const std::string& clone_id);
    bool removeImplant(const std::string& entity_id, const std::string& implant_id);
    float processDeath(const std::string& entity_id, float skill_points);
    int getActiveClone(const std::string& entity_id) const;
    int getCloneCount(const std::string& entity_id) const;
    int getImplantCount(const std::string& entity_id) const;
    int getTotalDeaths(const std::string& entity_id) const;
    float getSkillPointsAtRisk(const std::string& entity_id, float skill_points) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CloneBay& bay, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CLONE_BAY_SYSTEM_H
