#ifndef NOVAFORGE_SYSTEMS_VISUAL_CUE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_VISUAL_CUE_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class VisualCueSystem : public ecs::System {
public:
    explicit VisualCueSystem(ecs::World* world);
    ~VisualCueSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "VisualCueSystem"; }

    // --- API ---
    bool isLockdownActive(const std::string& system_id) const;
    float getTrafficDensity(const std::string& system_id) const;
    float getThreatGlow(const std::string& system_id) const;
    float getProsperityIndicator(const std::string& system_id) const;
    float getPirateWarning(const std::string& system_id) const;
    float getResourceHighlight(const std::string& system_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_VISUAL_CUE_SYSTEM_H
