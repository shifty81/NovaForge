#ifndef NOVAFORGE_SYSTEMS_ONBOARDING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ONBOARDING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tutorial / onboarding system for new player experience
 *
 * Manages tutorial phase progression, objective tracking, and hint
 * display for the vertical-slice first play-through:
 * undock → fly → mine → sell → fit → fight → warp.
 */
class OnboardingSystem : public ecs::SingleComponentSystem<components::OnboardingState> {
public:
    explicit OnboardingSystem(ecs::World* world);
    ~OnboardingSystem() override = default;

    std::string getName() const override { return "OnboardingSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id);

    // Phase management
    bool startTutorial(const std::string& entity_id, float start_time);
    bool advancePhase(const std::string& entity_id);
    bool skipTutorial(const std::string& entity_id);
    int getCurrentPhase(const std::string& entity_id) const;

    // Objective management
    bool addObjective(const std::string& entity_id, const std::string& objective_id,
                      const std::string& description, int phase);
    bool completeObjective(const std::string& entity_id, const std::string& objective_id,
                           float completed_at);
    bool isObjectiveComplete(const std::string& entity_id,
                             const std::string& objective_id) const;
    int getObjectiveCount(const std::string& entity_id) const;
    int getCompletedObjectiveCount(const std::string& entity_id) const;

    // Hint management
    bool addHint(const std::string& entity_id, const std::string& hint_id,
                 const std::string& text);
    bool showHint(const std::string& entity_id, const std::string& hint_id,
                  float shown_at);
    bool isHintShown(const std::string& entity_id, const std::string& hint_id) const;
    int getHintCount(const std::string& entity_id) const;
    int getShownHintCount(const std::string& entity_id) const;

    // Queries
    bool isTutorialComplete(const std::string& entity_id) const;
    bool isTutorialSkipped(const std::string& entity_id) const;
    float getCompletionTime(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::OnboardingState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ONBOARDING_SYSTEM_H
