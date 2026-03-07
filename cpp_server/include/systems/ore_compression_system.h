#ifndef NOVAFORGE_SYSTEMS_ORE_COMPRESSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ORE_COMPRESSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ore compression facility system
 *
 * Compresses raw ore into compact blocks at a per-type ratio, reducing
 * cargo volume for transport.  Processing takes time and charges an ISC
 * fee.  Supports multiple ore types with independent ratios and costs.
 */
class OreCompressionSystem : public ecs::SingleComponentSystem<components::OreCompression> {
public:
    explicit OreCompressionSystem(ecs::World* world);
    ~OreCompressionSystem() override = default;

    std::string getName() const override { return "OreCompressionSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addOreType(const std::string& entity_id, const std::string& ore_name,
                    float compression_ratio, float process_time, double cost_per_batch);
    bool startCompression(const std::string& entity_id, const std::string& ore_name,
                          int raw_units);
    bool cancelCompression(const std::string& entity_id);
    bool collectCompressed(const std::string& entity_id);
    int getOreTypeCount(const std::string& entity_id) const;
    std::string getCompressionState(const std::string& entity_id) const;
    std::string getCurrentOre(const std::string& entity_id) const;
    int getRawQueued(const std::string& entity_id) const;
    int getCompressedProduced(const std::string& entity_id) const;
    float getProcessTimer(const std::string& entity_id) const;
    double getTotalIscSpent(const std::string& entity_id) const;
    int getTotalBatchesProcessed(const std::string& entity_id) const;
    int getTotalRawConsumed(const std::string& entity_id) const;
    int getTotalCompressedProduced(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::OreCompression& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ORE_COMPRESSION_SYSTEM_H
