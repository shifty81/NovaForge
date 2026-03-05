#ifndef NOVAFORGE_SYSTEMS_RUMOR_PROPAGATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_RUMOR_PROPAGATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/narrative_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Incomplete intel leaks that propagate across star systems
 *
 * Manages rumor creation, spread between systems, accuracy decay,
 * confirmation, and expiration. Rumors carry partial information
 * about events like titan assembly, pirate activity, and trade shifts.
 */
class RumorPropagationSystem : public ecs::SingleComponentSystem<components::RumorPropagation> {
public:
    explicit RumorPropagationSystem(ecs::World* world);
    ~RumorPropagationSystem() override = default;

    std::string getName() const override { return "RumorPropagationSystem"; }

    bool initializeNetwork(const std::string& entity_id);
    bool createRumor(const std::string& entity_id, const std::string& rumor_id,
                     const std::string& category, float accuracy);
    bool spreadRumor(const std::string& entity_id, const std::string& rumor_id,
                     const std::string& target_system);
    bool confirmRumor(const std::string& entity_id, const std::string& rumor_id);
    float getRumorAccuracy(const std::string& entity_id, const std::string& rumor_id) const;
    int getRumorCount(const std::string& entity_id) const;
    int getConfirmedCount(const std::string& entity_id) const;
    int getExpiredCount(const std::string& entity_id) const;
    int getSpreadCount(const std::string& entity_id, const std::string& rumor_id) const;
    bool isRumorActive(const std::string& entity_id, const std::string& rumor_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::RumorPropagation& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_RUMOR_PROPAGATION_SYSTEM_H
