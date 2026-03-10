#ifndef NOVAFORGE_SYSTEMS_SLEEPER_CACHE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SLEEPER_CACHE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Sleeper cache exploration site system
 *
 * Manages hidden wormhole sites containing valuable Sleeper technology.
 * Sites have multiple rooms with hackable containers and sentry turrets.
 * Players must hack containers before the site timer expires.
 */
class SleeperCacheSystem : public ecs::SingleComponentSystem<components::SleeperCacheState> {
public:
    explicit SleeperCacheSystem(ecs::World* world);
    ~SleeperCacheSystem() override = default;

    std::string getName() const override { return "SleeperCacheSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& site_id = "",
                    components::SleeperCacheState::CacheTier tier =
                        components::SleeperCacheState::CacheTier::Limited);
    bool addRoom(const std::string& entity_id, const std::string& room_id,
                 int sentry_count = 2, float sentry_dps = 100.0f);
    bool addContainer(const std::string& entity_id, const std::string& room_id,
                      const std::string& container_id, float difficulty = 40.0f,
                      float loot_value = 1000000.0f);
    bool openRoom(const std::string& entity_id, const std::string& room_id);
    bool hackContainer(const std::string& entity_id, const std::string& room_id,
                       const std::string& container_id, float hack_amount);
    bool failContainer(const std::string& entity_id, const std::string& room_id,
                       const std::string& container_id);
    bool destroySentries(const std::string& entity_id, const std::string& room_id);

    int  getRoomCount(const std::string& entity_id) const;
    int  getContainersHacked(const std::string& entity_id) const;
    int  getContainersFailed(const std::string& entity_id) const;
    float getTotalLootValue(const std::string& entity_id) const;
    float getTimeRemaining(const std::string& entity_id) const;
    bool isExpired(const std::string& entity_id) const;
    bool isRoomOpen(const std::string& entity_id, const std::string& room_id) const;
    bool isContainerHacked(const std::string& entity_id, const std::string& room_id,
                           const std::string& container_id) const;
    components::SleeperCacheState::CacheTier getCacheTier(
        const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SleeperCacheState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SLEEPER_CACHE_SYSTEM_H
