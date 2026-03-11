#ifndef NOVAFORGE_SYSTEMS_STASIS_WEB_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STASIS_WEB_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Stasis webifier EWAR system — multiplicative velocity reduction.
 *
 * Manages stasis webifiers applied to an entity.  Each web reduces the
 * target's velocity multiplicatively by its strength factor.  Webs cycle
 * independently; when a cycle expires the web deactivates until the next
 * tick processes it, keeping the system stateless between updates.
 * Effective velocity is recomputed whenever webs are added or removed.
 */
class StasisWebSystem
    : public ecs::SingleComponentSystem<components::StasisWebState> {
public:
    explicit StasisWebSystem(ecs::World* world);
    ~StasisWebSystem() override = default;

    std::string getName() const override { return "StasisWebSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    float base_velocity = 1000.0f);

    // --- Web management ---
    bool applyWeb(const std::string& entity_id,
                  const std::string& web_id,
                  const std::string& source_id,
                  float strength,
                  float cycle_time);
    bool removeWeb(const std::string& entity_id,
                   const std::string& web_id);
    bool clearWebs(const std::string& entity_id);

    // --- Configuration ---
    bool setBaseVelocity(const std::string& entity_id, float velocity);

    // --- Queries ---
    float getEffectiveVelocity(const std::string& entity_id) const;
    float getBaseVelocity(const std::string& entity_id) const;
    int   getWebCount(const std::string& entity_id) const;
    int   getActiveWebCount(const std::string& entity_id) const;
    bool  isWebbed(const std::string& entity_id) const;
    int   getTotalWebsApplied(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::StasisWebState& comp,
                         float delta_time) override;

private:
    void recomputeVelocity(components::StasisWebState& comp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STASIS_WEB_SYSTEM_H
