#ifndef NOVAFORGE_SYSTEMS_TRACTOR_BEAM_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TRACTOR_BEAM_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Pulls loot / cargo containers toward the ship
 *
 * Locks a target within range and decreases distance each tick.
 * When the target reaches collection distance the item is auto-collected
 * and the beam unlocks, ready for the next target.
 */
class TractorBeamSystem : public ecs::SingleComponentSystem<components::TractorBeam> {
public:
    explicit TractorBeamSystem(ecs::World* world);
    ~TractorBeamSystem() override = default;

    std::string getName() const override { return "TractorBeamSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool lockTarget(const std::string& entity_id, const std::string& target_id, float distance);
    bool unlockTarget(const std::string& entity_id);
    bool setRange(const std::string& entity_id, float range);
    bool setPullSpeed(const std::string& entity_id, float speed);

    bool isLocked(const std::string& entity_id) const;
    float getCurrentDistance(const std::string& entity_id) const;
    int getItemsCollected(const std::string& entity_id) const;
    int getItemsFailed(const std::string& entity_id) const;
    std::string getTargetId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::TractorBeam& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TRACTOR_BEAM_SYSTEM_H
