#ifndef NOVAFORGE_SYSTEMS_OUTER_RIM_LOGISTICS_DISTORTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_OUTER_RIM_LOGISTICS_DISTORTION_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Trade route distortion from titan assembly pressure in the outer rim
 *
 * Manages route efficiency, threat-based distortion, price impact calculation,
 * and recovery when threat subsides.
 */
class OuterRimLogisticsDistortionSystem : public ecs::System {
public:
    explicit OuterRimLogisticsDistortionSystem(ecs::World* world);
    ~OuterRimLogisticsDistortionSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "OuterRimLogisticsDistortionSystem"; }

    bool initializeRegion(const std::string& entity_id, const std::string& region_id);
    bool addRoute(const std::string& entity_id, const std::string& route_id);
    bool setGlobalThreat(const std::string& entity_id, float threat);
    bool setRouteThreat(const std::string& entity_id, const std::string& route_id, float threat);
    float getRouteEfficiency(const std::string& entity_id, const std::string& route_id) const;
    float getRoutePriceImpact(const std::string& entity_id, const std::string& route_id) const;
    float getGlobalThreat(const std::string& entity_id) const;
    int getDisruptedRouteCount(const std::string& entity_id) const;
    float getTotalPriceImpact(const std::string& entity_id) const;
    int getRouteCount(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_OUTER_RIM_LOGISTICS_DISTORTION_SYSTEM_H
