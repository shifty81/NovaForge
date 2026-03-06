#ifndef NOVAFORGE_SYSTEMS_FPS_ENEMY_AI_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_ENEMY_AI_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief NPC enemy AI for FPS interior combat
 *
 * Manages patrol routes, alert states, chase/attack behaviors.
 * Handles damage, death, and faction-based hostility.
 */
class FPSEnemyAISystem : public ecs::SingleComponentSystem<components::FPSEnemyAI> {
public:
    explicit FPSEnemyAISystem(ecs::World* world);
    ~FPSEnemyAISystem() override = default;

    std::string getName() const override { return "FPSEnemyAISystem"; }

    bool setState(const std::string& entity_id, int state);
    int getState(const std::string& entity_id) const;
    bool setTarget(const std::string& entity_id, const std::string& target_id);
    std::string getTarget(const std::string& entity_id) const;
    bool addWaypoint(const std::string& entity_id, const std::string& wp_id,
                     float x, float y, float z, float wait_time);
    int getWaypointCount(const std::string& entity_id) const;
    bool applyDamage(const std::string& entity_id, float amount);
    float getHealth(const std::string& entity_id) const;
    bool isDead(const std::string& entity_id) const;
    bool isHostile(const std::string& entity_id) const;
    float getDistanceToTarget(const std::string& entity_id, float target_x, float target_y, float target_z) const;
    bool canDetect(const std::string& entity_id, float target_x, float target_y, float target_z) const;
    int getTotalAttacks(const std::string& entity_id) const;
    int getTotalPatrolsCompleted(const std::string& entity_id) const;
    bool setPosition(const std::string& entity_id, float x, float y, float z);

protected:
    void updateComponent(ecs::Entity& entity, components::FPSEnemyAI& ai, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_ENEMY_AI_SYSTEM_H
