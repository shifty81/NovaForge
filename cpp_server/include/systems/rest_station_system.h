#ifndef NOVAFORGE_SYSTEMS_REST_STATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_REST_STATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages rest facilities (beds, quarters) for fatigue recovery
 *
 * Provides mechanics for characters to rest in beds, quarters, or other
 * rest facilities to recover fatigue. Supports quality levels affecting
 * recovery rate and comfort bonuses.
 */
class RestStationSystem : public ecs::SingleComponentSystem<components::RestingState> {
public:
    explicit RestStationSystem(ecs::World* world);
    ~RestStationSystem() override = default;

    std::string getName() const override { return "RestStationSystem"; }

    /**
     * @brief Start resting at a rest station
     * @param entity_id Character entity
     * @param station_id Rest station (bed/quarters) entity
     * @return true if rest started successfully
     */
    bool startResting(const std::string& entity_id, const std::string& station_id);

    /**
     * @brief Stop resting and get up
     * @param entity_id Character entity
     * @return Current fatigue level after stopping
     */
    float stopResting(const std::string& entity_id);

    /**
     * @brief Check if an entity is currently resting
     */
    bool isResting(const std::string& entity_id) const;

    /**
     * @brief Get current rest progress (0 = just started, 1 = fully rested)
     */
    float getRestProgress(const std::string& entity_id) const;

    /**
     * @brief Get time remaining until fully rested (fatigue = 0)
     * @return Seconds remaining, or 0 if not resting
     */
    float getTimeUntilFullyRested(const std::string& entity_id) const;

    /**
     * @brief Check if a rest station is available (not occupied)
     */
    bool isStationAvailable(const std::string& station_id) const;

    /**
     * @brief Get the quality level of a rest station
     * @return Quality from 0.5 (poor) to 2.0 (luxury)
     */
    float getStationQuality(const std::string& station_id) const;

    /**
     * @brief Get quality level name
     */
    static std::string getQualityName(float quality);

    /**
     * @brief Get count of entities currently resting
     */
    int getRestingCount() const;

private:
    // Base fatigue recovery rate per second (at quality 1.0)
    float base_recovery_rate_ = 1.0f;

protected:
    void updateComponent(ecs::Entity& entity, components::RestingState& resting, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_REST_STATION_SYSTEM_H
