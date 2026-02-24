#ifndef EVE_SYSTEMS_AI_ECONOMIC_ACTOR_SYSTEM_H
#define EVE_SYSTEMS_AI_ECONOMIC_ACTOR_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages NPC economic actors with wallets, ship ownership, and permanent death
 *
 * AI economic actors are real participants in the economy:
 * - They own ships (tracked by type and value)
 * - They have wallets that change from trade, mining, bounties
 * - They pay for repairs and replacement ships on destruction
 * - They can permanently die if they cannot afford a new ship
 *
 * This transforms NPCs from disposable spawns into persistent economic agents.
 */
class AIEconomicActorSystem : public ecs::System {
public:
    explicit AIEconomicActorSystem(ecs::World* world);
    ~AIEconomicActorSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "AIEconomicActorSystem"; }

    // --- API ---

    /** Record an NPC earning ISK (from trade, mining, bounty) */
    void earnISK(const std::string& entity_id, double amount);

    /** Record an NPC spending ISK (on repairs, modules, etc.) */
    bool spendISK(const std::string& entity_id, double amount);

    /** Handle ship destruction — NPC tries to buy replacement or permanently dies */
    bool handleShipDestruction(const std::string& entity_id);

    /** Check if an NPC can afford a replacement ship */
    bool canAffordReplacement(const std::string& entity_id) const;

    /** Get all permanently dead actors */
    std::vector<std::string> getDeadActors() const;

    /** Get all actors with wallet below threshold */
    std::vector<std::string> getLowFundsActors(double threshold) const;

    /** Get total economy ISK across all actors */
    double getTotalEconomyISK() const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_AI_ECONOMIC_ACTOR_SYSTEM_H
