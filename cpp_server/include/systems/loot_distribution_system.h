#ifndef NOVAFORGE_SYSTEMS_LOOT_DISTRIBUTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_LOOT_DISTRIBUTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Proportional loot and ISK distribution among kill participants
 *
 * When multiple pilots contribute damage to a kill, this system fairly splits
 * the ISK bounty and assigns loot items proportional to the damage each pilot
 * dealt.  The owner opens a distribution session, registers participants with
 * their damage totals and optionally an ISK pool and item list, then calls
 * distribute() to commit shares.  After distribution the session is immutable.
 */
class LootDistributionSystem
    : public ecs::SingleComponentSystem<components::LootDistribution> {
public:
    explicit LootDistributionSystem(ecs::World* world);
    ~LootDistributionSystem() override = default;

    std::string getName() const override { return "LootDistributionSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Distribution control ---
    bool openDistribution(const std::string& entity_id);
    bool distribute(const std::string& entity_id);

    // --- Participant management ---
    bool addParticipant(const std::string& entity_id,
                        const std::string& pilot_id,
                        float damage_dealt);
    bool removeParticipant(const std::string& entity_id,
                           const std::string& pilot_id);

    // --- Pool management ---
    bool setIskPool(const std::string& entity_id, float amount);
    bool addItem(const std::string& entity_id,
                 const std::string& item_id,
                 const std::string& item_name,
                 int quantity);

    // --- Queries ---
    components::LootDistribution::State
         getState(const std::string& entity_id) const;
    int  getParticipantCount(const std::string& entity_id) const;
    int  getItemCount(const std::string& entity_id) const;
    float getIskPool(const std::string& entity_id) const;
    float getParticipantShare(const std::string& entity_id,
                               const std::string& pilot_id) const;
    int  getTotalDistributions(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::LootDistribution& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_LOOT_DISTRIBUTION_SYSTEM_H
