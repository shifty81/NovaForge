#ifndef NOVAFORGE_SYSTEMS_RESOURCE_PRODUCTION_CHAIN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_RESOURCE_PRODUCTION_CHAIN_SYSTEM_H

#include "ecs/system.h"
#include "components/economy_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages end-to-end production chains (mining → refining → manufacturing → market)
 *
 * Tracks each stage of production and computes overall efficiency, bottlenecks,
 * and throughput. Enables the living economy where everything is produced,
 * transported, consumed, or destroyed — no fake NPC market orders.
 */
class ResourceProductionChainSystem : public ecs::System {
public:
    explicit ResourceProductionChainSystem(ecs::World* world);
    ~ResourceProductionChainSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ResourceProductionChainSystem"; }

    // Chain management
    bool createChain(const std::string& entity_id, const std::string& chain_id);
    bool addStage(const std::string& entity_id, const std::string& stage_name,
                  const std::string& input_resource, const std::string& output_resource,
                  float conversion_rate);
    bool removeStage(const std::string& entity_id, const std::string& stage_name);
    bool setStageEfficiency(const std::string& entity_id, const std::string& stage_name, float efficiency);
    bool setChainActive(const std::string& entity_id, bool active);

    // Query API
    float getOverallEfficiency(const std::string& entity_id) const;
    float getTotalOutput(const std::string& entity_id) const;
    float getStageThroughput(const std::string& entity_id, const std::string& stage_name) const;
    float getBottleneckFactor(const std::string& entity_id, const std::string& stage_name) const;
    int getStageCount(const std::string& entity_id) const;
    bool isChainActive(const std::string& entity_id) const;
    float getUptime(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_RESOURCE_PRODUCTION_CHAIN_SYSTEM_H
