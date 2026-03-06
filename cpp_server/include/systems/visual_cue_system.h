#ifndef NOVAFORGE_SYSTEMS_VISUAL_CUE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_VISUAL_CUE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class VisualCueSystem : public ecs::SingleComponentSystem<components::SimStarSystemState> {
public:
    explicit VisualCueSystem(ecs::World* world);
    ~VisualCueSystem() override = default;

    std::string getName() const override { return "VisualCueSystem"; }

    // --- API ---
    bool isLockdownActive(const std::string& system_id) const;
    float getTrafficDensity(const std::string& system_id) const;
    float getThreatGlow(const std::string& system_id) const;
    float getProsperityIndicator(const std::string& system_id) const;
    float getPirateWarning(const std::string& system_id) const;
    float getResourceHighlight(const std::string& system_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SimStarSystemState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_VISUAL_CUE_SYSTEM_H
