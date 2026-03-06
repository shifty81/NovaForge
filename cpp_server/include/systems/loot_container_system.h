#ifndef NOVAFORGE_SYSTEMS_LOOT_CONTAINER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_LOOT_CONTAINER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages loot containers from wrecks and exploration sites
 *
 * Tracks container contents with expiry timers. Supports access control
 * via owner, abandonment, and locking mechanics.
 */
class LootContainerSystem : public ecs::SingleComponentSystem<components::LootContainer> {
public:
    explicit LootContainerSystem(ecs::World* world);
    ~LootContainerSystem() override = default;

    std::string getName() const override { return "LootContainerSystem"; }

    bool addItem(const std::string& entity_id, const std::string& item_id,
                 const std::string& name, const std::string& category,
                 int quantity, float volume, float value);
    bool removeItem(const std::string& entity_id, const std::string& item_id, int quantity);
    int getItemCount(const std::string& entity_id) const;
    float getTotalValue(const std::string& entity_id) const;
    float getTotalVolume(const std::string& entity_id) const;
    float getTimeRemaining(const std::string& entity_id) const;
    bool abandonContainer(const std::string& entity_id);
    bool lockContainer(const std::string& entity_id, bool locked);
    bool isAccessible(const std::string& entity_id, const std::string& accessor_id) const;
    int getTotalLooted(const std::string& entity_id) const;
    bool setOwner(const std::string& entity_id, const std::string& owner_id);

protected:
    void updateComponent(ecs::Entity& entity, components::LootContainer& container, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_LOOT_CONTAINER_SYSTEM_H
