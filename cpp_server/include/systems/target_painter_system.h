#ifndef NOVAFORGE_SYSTEMS_TARGET_PAINTER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TARGET_PAINTER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Target painter EWAR system — multiplicative signature-radius amplification.
 *
 * Manages target painters applied to an entity.  Each painter increases the
 * entity's effective signature radius multiplicatively by its strength factor,
 * making the entity easier to hit by weapons that scale with signature (missiles
 * especially).  Painters cycle independently; effective signature is recomputed
 * whenever painters are added, removed, or cycle.
 */
class TargetPainterSystem
    : public ecs::SingleComponentSystem<components::TargetPainterState> {
public:
    explicit TargetPainterSystem(ecs::World* world);
    ~TargetPainterSystem() override = default;

    std::string getName() const override { return "TargetPainterSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    float base_signature = 50.0f);

    // --- Painter management ---
    bool applyPainter(const std::string& entity_id,
                      const std::string& painter_id,
                      const std::string& source_id,
                      float strength,
                      float cycle_time);
    bool removePainter(const std::string& entity_id,
                       const std::string& painter_id);
    bool clearPainters(const std::string& entity_id);

    // --- Configuration ---
    bool setBaseSignature(const std::string& entity_id, float signature);

    // --- Queries ---
    float getEffectiveSignature(const std::string& entity_id) const;
    float getBaseSignature(const std::string& entity_id) const;
    int   getPainterCount(const std::string& entity_id) const;
    int   getActivePainterCount(const std::string& entity_id) const;
    bool  isPainted(const std::string& entity_id) const;
    int   getTotalPaintersApplied(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::TargetPainterState& comp,
                         float delta_time) override;

private:
    void recomputeSignature(components::TargetPainterState& comp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TARGET_PAINTER_SYSTEM_H
