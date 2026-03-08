#ifndef NOVAFORGE_SYSTEMS_MINING_YIELD_OPTIMIZER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MINING_YIELD_OPTIMIZER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Calculates mining yield bonuses from skills, modules, environment
 *
 * Aggregates mining yield multipliers from skill levels, mining module
 * bonuses, and environmental factors (belt richness, security status).
 * Outputs a final yield multiplier used by the mining system to scale
 * ore extraction rates. Ties together skills, modules, and mining loops.
 */
class MiningYieldOptimizerSystem : public ecs::SingleComponentSystem<components::MiningYieldState> {
public:
    explicit MiningYieldOptimizerSystem(ecs::World* world);
    ~MiningYieldOptimizerSystem() override = default;

    std::string getName() const override { return "MiningYieldOptimizerSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool setSkillBonus(const std::string& entity_id, float bonus);
    bool setModuleBonus(const std::string& entity_id, float bonus);
    bool setEnvironmentBonus(const std::string& entity_id, float bonus);
    bool setSecurityModifier(const std::string& entity_id, float security);
    bool recordCycle(const std::string& entity_id, float base_yield);
    float getSkillBonus(const std::string& entity_id) const;
    float getModuleBonus(const std::string& entity_id) const;
    float getEnvironmentBonus(const std::string& entity_id) const;
    float getFinalMultiplier(const std::string& entity_id) const;
    float getTotalYield(const std::string& entity_id) const;
    int getCycleCount(const std::string& entity_id) const;
    float getAverageYieldPerCycle(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::MiningYieldState& mys, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MINING_YIELD_OPTIMIZER_SYSTEM_H
