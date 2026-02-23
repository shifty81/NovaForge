#ifndef EVE_SYSTEMS_WRECK_PERSISTENCE_SYSTEM_H
#define EVE_SYSTEMS_WRECK_PERSISTENCE_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages wreck lifetime and salvage NPC assignment
 */
class WreckPersistenceSystem : public ecs::System {
public:
    explicit WreckPersistenceSystem(ecs::World* world);
    ~WreckPersistenceSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "WreckPersistenceSystem"; }

    // --- Query API ---
    bool isExpired(const std::string& entity_id) const;
    float getRemainingLifetime(const std::string& entity_id) const;
    void assignSalvageNPC(const std::string& wreck_id, const std::string& npc_id);
    std::vector<std::string> getExpiredWrecks() const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_WRECK_PERSISTENCE_SYSTEM_H
