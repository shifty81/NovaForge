#ifndef NOVAFORGE_SYSTEMS_ECONOMIC_FLOW_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ECONOMIC_FLOW_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <map>

namespace atlas {
namespace systems {

/**
 * @brief Economy coordinator tracking the production→transport→consumption→destruction lifecycle
 *
 * Phase 2 economic flow system. Every commodity in the game is produced,
 * transported, consumed, or destroyed — no fake NPC market orders.
 * Each star system with an EconomicFlowState component has per-commodity
 * flow rates that are updated each tick and decay over time.
 */
class EconomicFlowSystem : public ecs::System {
public:
    explicit EconomicFlowSystem(ecs::World* world);
    ~EconomicFlowSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "EconomicFlowSystem"; }

    // --- Recording API ---

    /** Record goods produced in a system (NPC or player) */
    void recordProduction(const std::string& system_id, const std::string& commodity, float amount);

    /** Record goods consumed in a system (NPCs/players) */
    void recordConsumption(const std::string& system_id, const std::string& commodity, float amount);

    /** Record goods transported between systems (hauler moved goods) */
    void recordTransport(const std::string& from_system, const std::string& to_system,
                         const std::string& commodity, float amount);

    /** Record goods destroyed (combat, piracy, decay) */
    void recordDestruction(const std::string& system_id, const std::string& commodity, float amount);

    // --- Query API ---

    float getEconomicHealth(const std::string& system_id) const;
    float getProductionRate(const std::string& system_id, const std::string& commodity) const;
    float getConsumptionRate(const std::string& system_id, const std::string& commodity) const;
    float getNetFlow(const std::string& system_id, const std::string& commodity) const;
    float getTotalProduction(const std::string& system_id) const;
    float getTotalConsumption(const std::string& system_id) const;

    // --- Configuration ---
    float flow_decay_rate = 0.1f;     // rates decay towards 0 over time
    float health_smoothing = 0.05f;   // smoothing factor for economic_health
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ECONOMIC_FLOW_SYSTEM_H
