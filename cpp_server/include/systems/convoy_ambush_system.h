#ifndef NOVAFORGE_SYSTEMS_CONVOY_AMBUSH_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CONVOY_AMBUSH_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class ConvoyAmbushSystem : public ecs::System {
public:
    explicit ConvoyAmbushSystem(ecs::World* world);
    ~ConvoyAmbushSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ConvoyAmbushSystem"; }

    // Register a trade route; returns route entity id
    std::string registerRoute(const std::string& origin,
                              const std::string& destination,
                              const std::string& cargo_type,
                              double cargo_value,
                              float security_level);

    // Pirate plans an ambush on a route; returns ambush entity id or ""
    std::string planAmbush(const std::string& pirate_entity_id,
                           const std::string& route_entity_id);

    // Execute a planned ambush — success depends on security level
    bool executeAmbush(const std::string& ambush_entity_id);

    // Disperse an active ambush (security response)
    bool disperseAmbush(const std::string& ambush_entity_id);

    // Get current ambush state string
    std::string getAmbushState(const std::string& ambush_entity_id) const;

    // Get pirate interest for a route (0.0-1.0)
    float getRouteRisk(const std::string& route_entity_id) const;

    // List all planned ambushes
    std::vector<std::string> getPlannedAmbushes() const;

private:
    int route_counter_ = 0;
    int ambush_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CONVOY_AMBUSH_SYSTEM_H
