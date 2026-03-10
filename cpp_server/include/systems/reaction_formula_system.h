#ifndef NOVAFORGE_SYSTEMS_REACTION_FORMULA_SYSTEM_H
#define NOVAFORGE_SYSTEMS_REACTION_FORMULA_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Reaction formula system for moon material processing
 *
 * Manages reaction jobs that convert moon goo (raw moon materials) into
 * intermediate and advanced materials.  Each job tracks input materials,
 * a single output, and progress scaled by an efficiency multiplier
 * from facility rigs / bonuses.
 */
class ReactionFormulaSystem : public ecs::SingleComponentSystem<components::ReactionFormulaState> {
public:
    explicit ReactionFormulaSystem(ecs::World* world);
    ~ReactionFormulaSystem() override = default;

    std::string getName() const override { return "ReactionFormulaSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& facility_id = "");
    bool startReaction(const std::string& entity_id, const std::string& job_id,
                       const std::string& formula_id,
                       float time_required = 3600.0f,
                       const std::string& output_material = "",
                       int output_quantity = 1);
    bool addInput(const std::string& entity_id, const std::string& job_id,
                  const std::string& material_id, int quantity);
    bool cancelReaction(const std::string& entity_id, const std::string& job_id);
    bool setEfficiency(const std::string& entity_id, float efficiency);

    int   getActiveReactionCount(const std::string& entity_id) const;
    bool  isReactionCompleted(const std::string& entity_id,
                              const std::string& job_id) const;
    float getReactionProgress(const std::string& entity_id,
                              const std::string& job_id) const;
    int   getTotalCompleted(const std::string& entity_id) const;
    float getEfficiency(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::ReactionFormulaState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_REACTION_FORMULA_SYSTEM_H
