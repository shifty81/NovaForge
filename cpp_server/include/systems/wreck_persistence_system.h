#ifndef NOVAFORGE_SYSTEMS_WRECK_PERSISTENCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WRECK_PERSISTENCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages wreck lifetime and salvage NPC assignment
 */
class WreckPersistenceSystem : public ecs::SingleComponentSystem<components::WreckPersistence> {
public:
    explicit WreckPersistenceSystem(ecs::World* world);
    ~WreckPersistenceSystem() override = default;

    std::string getName() const override { return "WreckPersistenceSystem"; }

    // --- Query API ---
    bool isExpired(const std::string& entity_id) const;
    float getRemainingLifetime(const std::string& entity_id) const;
    void assignSalvageNPC(const std::string& wreck_id, const std::string& npc_id);
    std::vector<std::string> getExpiredWrecks() const;

protected:
    void updateComponent(ecs::Entity& entity, components::WreckPersistence& wreck, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WRECK_PERSISTENCE_SYSTEM_H
