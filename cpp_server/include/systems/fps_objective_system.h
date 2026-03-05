#ifndef NOVAFORGE_SYSTEMS_FPS_OBJECTIVE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_OBJECTIVE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief On-foot mission objective manager for FPS gameplay
 *
 * Creates and tracks objectives like boarding actions, VIP rescue,
 * sabotage, defend-the-point, and item retrieval.  Objectives advance
 * through Inactive → Active → Completed/Failed states.
 */
class FPSObjectiveSystem : public ecs::SingleComponentSystem<components::FPSObjective> {
public:
    explicit FPSObjectiveSystem(ecs::World* world);
    ~FPSObjectiveSystem() override = default;

    std::string getName() const override { return "FPSObjectiveSystem"; }

    // --- Setup ---

    /** Create a new objective */
    bool createObjective(const std::string& objective_id,
                         const std::string& interior_id,
                         const std::string& room_id,
                         const std::string& player_id,
                         components::FPSObjective::ObjectiveType type,
                         const std::string& description = "",
                         float time_limit = 0.0f);

    /** Activate an objective (transitions from Inactive to Active) */
    bool activateObjective(const std::string& objective_id);

    // --- Queries ---

    /** Get the state of an objective */
    int getObjectiveState(const std::string& objective_id) const;

    /** Get all active objectives for a player */
    std::vector<std::string> getPlayerObjectives(const std::string& player_id) const;

    /** Get progress (0.0 to 1.0) of an objective */
    float getProgress(const std::string& objective_id) const;

    /** Check if an objective is complete */
    bool isComplete(const std::string& objective_id) const;

    /** Check if an objective has failed */
    bool isFailed(const std::string& objective_id) const;

    // --- Actions ---

    /** Report a hostile kill (for EliminateHostiles objectives) */
    bool reportHostileKill(const std::string& objective_id);

    /** Report item collected (for RetrieveItem objectives) */
    bool reportItemCollected(const std::string& objective_id,
                              const std::string& item_id);

    /** Report sabotage complete (for Sabotage objectives) */
    bool reportSabotageComplete(const std::string& objective_id);

    /** Report player reached extraction (for Escape objectives) */
    bool reportExtraction(const std::string& objective_id);

    /** Report VIP rescued (for RescueVIP objectives) */
    bool reportVIPRescued(const std::string& objective_id);

    /** Report repair complete (for RepairSystem objectives) */
    bool reportRepairComplete(const std::string& objective_id);

    /** Force-fail an objective */
    bool failObjective(const std::string& objective_id);

    // --- Helpers ---

    static std::string objectiveTypeName(int type);
    static std::string stateName(int s);

    // --- Configuration for EliminateHostiles ---

    bool setHostileCount(const std::string& objective_id, int count);

    // --- Configuration for DefendPoint ---

    bool setDefendDuration(const std::string& objective_id, float seconds);

    // --- Configuration for RetrieveItem / Sabotage ---

    bool setTargetItem(const std::string& objective_id, const std::string& item_id);

protected:
    void updateComponent(ecs::Entity& entity, components::FPSObjective& obj, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_OBJECTIVE_SYSTEM_H
