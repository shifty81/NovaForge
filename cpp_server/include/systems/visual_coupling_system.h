#ifndef NOVAFORGE_SYSTEMS_VISUAL_COUPLING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_VISUAL_COUPLING_SYSTEM_H

#include "ecs/system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Visual coupling system (Phase 13)
 *
 * Maps interior modules to exterior visual features on the ship hull.
 * Solar panels, ore containers, vents, and other features become visible
 * on the exterior when corresponding interior modules are installed.
 */
class VisualCouplingSystem : public ecs::System {
public:
    explicit VisualCouplingSystem(ecs::World* world);
    ~VisualCouplingSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "VisualCouplingSystem"; }

    // Initialization
    bool initializeCoupling(const std::string& entity_id, const std::string& ship_id);

    // Module coupling
    bool addCoupling(const std::string& entity_id, const std::string& module_id,
                     components::VisualCoupling::ExteriorFeature feature, float scale);
    bool removeCoupling(const std::string& entity_id, const std::string& module_id);
    bool setVisibility(const std::string& entity_id, const std::string& module_id, bool visible);
    bool setOffset(const std::string& entity_id, const std::string& module_id,
                   float x, float y, float z);

    // Query
    int getCouplingCount(const std::string& entity_id) const;
    int getVisibleCount(const std::string& entity_id) const;
    int getFeatureCount(const std::string& entity_id,
                        components::VisualCoupling::ExteriorFeature feature) const;
    std::string getFeatureName(components::VisualCoupling::ExteriorFeature feature) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_VISUAL_COUPLING_SYSTEM_H
