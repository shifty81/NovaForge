#ifndef NOVAFORGE_SYSTEMS_ABYSSAL_ESCALATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ABYSSAL_ESCALATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Abyssal Deadspace pocket escalation system
 *
 * Each Abyssal pocket contains three waves of Triglavian NPCs:
 *   Wave 1 – scout units
 *   Wave 2 – reinforcements with higher HP/DPS
 *   Boss    – named Triglavian with unique abilities; unlocks exit
 *
 * NPC stats and loot value scale with filament tier.  The boss is
 * only spawned after Wave 2 is fully cleared.
 */
class AbyssalEscalationSystem
    : public ecs::SingleComponentSystem<components::AbyssalEscalationState> {
public:
    explicit AbyssalEscalationSystem(ecs::World* world);
    ~AbyssalEscalationSystem() override = default;

    std::string getName() const override { return "AbyssalEscalationSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id,
                    const std::string& pocket_id = "",
                    int tier = 1);
    bool completeWave(const std::string& entity_id);
    bool spawnBoss(const std::string& entity_id);
    bool killBoss(const std::string& entity_id);
    bool applyDamage(const std::string& entity_id, float amount);
    bool recordKill(const std::string& entity_id);

    components::AbyssalEscalationState::EscalationPhase
        getCurrentPhase(const std::string& entity_id) const;
    int  getEnemiesKilled(const std::string& entity_id) const;
    bool isBossSpawned(const std::string& entity_id) const;
    bool isBossKilled(const std::string& entity_id) const;
    bool isRunCompleted(const std::string& entity_id) const;
    float getDpsReceived(const std::string& entity_id) const;
    int  getTotalLootValue(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AbyssalEscalationState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ABYSSAL_ESCALATION_SYSTEM_H
