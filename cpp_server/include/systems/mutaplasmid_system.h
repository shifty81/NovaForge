#ifndef NOVAFORGE_SYSTEMS_MUTAPLASMID_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MUTAPLASMID_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Abyssal mutaplasmid module mutation system
 *
 * Allows players to apply a mutaplasmid to an eligible module,
 * producing a "mutated" module with randomly rolled stat multipliers.
 * Each stat is rolled between [min_multiplier, max_multiplier].
 * The overall_quality score reflects how close to the maximum
 * roll each stat achieved (0.0 = all minimums, 1.0 = all maximums).
 *
 * Grade affects the breadth of the roll range:
 *   Unstable   – wide range, moderate base
 *   Decayed    – narrow range below baseline (mostly worse)
 *   Gravid     – narrow range above baseline (mostly better)
 *   Overloaded – very wide range (high risk, high reward)
 */
class MutaplasmidSystem
    : public ecs::SingleComponentSystem<components::MutaplasmidState> {
public:
    explicit MutaplasmidSystem(ecs::World* world);
    ~MutaplasmidSystem() override = default;

    std::string getName() const override { return "MutaplasmidSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& facility_id = "");

    /**
     * @brief Queue a mutation of @p module_id using a mutaplasmid of @p grade.
     *        Stat names and roll ranges are provided by the caller.
     * @return true if accepted (under max_pending limit)
     */
    bool queueMutation(const std::string& entity_id,
                       const std::string& module_id,
                       components::MutaplasmidState::Grade grade,
                       const std::vector<components::MutaplasmidState::StatRoll>& stats);

    /**
     * @brief Apply deterministic rolls to a pending mutation (useful in tests).
     *        Each stat_index receives @p value clamped to [min, max].
     */
    bool applyRoll(const std::string& entity_id,
                   const std::string& module_id,
                   int stat_index,
                   float value);

    /**
     * @brief Finalize a mutation once all stats have been rolled.
     * @return true if mutation was completed successfully
     */
    bool finalizeMutation(const std::string& entity_id,
                          const std::string& module_id);

    int   getMutationCount(const std::string& entity_id) const;
    int   getTotalAttempted(const std::string& entity_id) const;
    int   getTotalCreated(const std::string& entity_id) const;
    float getOverallQuality(const std::string& entity_id,
                            const std::string& module_id) const;
    bool  isMutationCreated(const std::string& entity_id,
                            const std::string& module_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::MutaplasmidState& comp,
                         float delta_time) override;

private:
    static float computeQuality(
        const std::vector<components::MutaplasmidState::StatRoll>& rolls);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MUTAPLASMID_SYSTEM_H
