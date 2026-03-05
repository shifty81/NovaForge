#ifndef NOVAFORGE_SYSTEMS_IMPERFECT_INFORMATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_IMPERFECT_INFORMATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/npc_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Imperfect information and intel decay system
 *
 * Manages entity scan data with confidence levels that decay over time.
 * Entities below ghost threshold become sensor ghosts (unreliable data).
 * Scan quality depends on sensor strength and distance.
 */
class ImperfectInformationSystem : public ecs::SingleComponentSystem<components::EntityIntel> {
public:
    explicit ImperfectInformationSystem(ecs::World* world);
    ~ImperfectInformationSystem() override = default;

    std::string getName() const override { return "ImperfectInformationSystem"; }

    // Commands
    bool recordIntel(const std::string& observer_id, const std::string& target_id,
                     float scan_quality, float distance);
    bool clearIntel(const std::string& observer_id, const std::string& target_id);
    bool clearAllIntel(const std::string& observer_id);
    bool setSensorStrength(const std::string& observer_id, float strength);

    // Query API
    float getConfidence(const std::string& observer_id, const std::string& target_id) const;
    bool hasIntel(const std::string& observer_id, const std::string& target_id) const;
    float getIntelAge(const std::string& observer_id, const std::string& target_id) const;
    bool isGhost(const std::string& observer_id, const std::string& target_id) const;
    int getIntelCount(const std::string& observer_id) const;
    int getTotalScans(const std::string& observer_id) const;
    float getSensorStrength(const std::string& observer_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::EntityIntel& intel, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_IMPERFECT_INFORMATION_SYSTEM_H
