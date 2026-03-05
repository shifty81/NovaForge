#ifndef NOVAFORGE_SYSTEMS_HANGAR_ENVIRONMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_HANGAR_ENVIRONMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Hangar environment system (Phase 14)
 *
 * Manages atmospheric hazards when a hangar is opened in unsafe environments,
 * including toxicity, corrosion, occupant damage, and alarm systems.
 */
class HangarEnvironmentSystem : public ecs::SingleComponentSystem<components::HangarEnvironment> {
public:
    explicit HangarEnvironmentSystem(ecs::World* world);
    ~HangarEnvironmentSystem() override = default;

    std::string getName() const override { return "HangarEnvironmentSystem"; }

    // Initialization
    bool initializeEnvironment(const std::string& entity_id,
                               components::HangarEnvironment::AtmosphereType atmosphere_type,
                               float external_temp, float external_pressure);

    // Hangar operations
    bool openHangar(const std::string& entity_id);
    bool closeHangar(const std::string& entity_id);

    // Occupant management
    bool addOccupant(const std::string& entity_id, const std::string& occupant_id,
                     bool has_suit, float suit_rating);
    bool removeOccupant(const std::string& entity_id, const std::string& occupant_id);

    // Environment control
    bool setAtmosphere(const std::string& entity_id,
                       components::HangarEnvironment::AtmosphereType atmosphere_type);

    // Query
    float getOccupantDamage(const std::string& entity_id, const std::string& occupant_id) const;
    float getToxicity(const std::string& entity_id) const;
    bool isAlarmActive(const std::string& entity_id) const;
    int getOccupantCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::HangarEnvironment& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_HANGAR_ENVIRONMENT_SYSTEM_H
