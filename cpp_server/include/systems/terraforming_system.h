#ifndef NOVAFORGE_SYSTEMS_TERRAFORMING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TERRAFORMING_SYSTEM_H

#include "ecs/system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Long-term planet modification system (Phase 14)
 *
 * Manages terraforming projects that progress through stages:
 * Planning → Infrastructure → AtmosphereProcessing → TemperatureRegulation → BiomeSeeding → Complete.
 * Each stage takes time_per_stage seconds. Environment parameters move toward targets over the process.
 */
class TerraformingSystem : public ecs::System {
public:
    explicit TerraformingSystem(ecs::World* world);
    ~TerraformingSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "TerraformingSystem"; }

    // Commands
    bool startTerraforming(const std::string& entity_id, const std::string& planet_id);
    bool pauseTerraforming(const std::string& entity_id);
    bool resumeTerraforming(const std::string& entity_id);
    bool cancelTerraforming(const std::string& entity_id);
    bool setTargets(const std::string& entity_id, float atmosphere, float temperature, float water_coverage);
    bool advanceStage(const std::string& entity_id);

    // Query API
    std::string getStage(const std::string& entity_id) const;
    float getProgress(const std::string& entity_id) const;
    float getTotalProgress(const std::string& entity_id) const;
    bool isActive(const std::string& entity_id) const;
    double getTotalCreditsSpent(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TERRAFORMING_SYSTEM_H
