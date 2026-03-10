#ifndef NOVAFORGE_SYSTEMS_TUTORIAL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TUTORIAL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tutorial guidance and progression system
 *
 * Steps are registered with addStep() before the tutorial starts.  Once
 * startTutorial() is called the system increments elapsed each tick.
 * completeStep() marks the current step done and advances to the next;
 * when the last step is completed is_complete is set to true.
 * skipTutorial() marks all steps done immediately.
 */
class TutorialSystem
    : public ecs::SingleComponentSystem<components::TutorialState> {
public:
    explicit TutorialSystem(ecs::World* world);
    ~TutorialSystem() override = default;

    std::string getName() const override { return "TutorialSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Step management ---
    bool addStep(const std::string& entity_id,
                 const std::string& step_id,
                 const std::string& description);
    bool startTutorial(const std::string& entity_id);
    bool completeStep(const std::string& entity_id,
                      const std::string& step_id);
    bool skipTutorial(const std::string& entity_id);
    bool resetTutorial(const std::string& entity_id);

    // --- Queries ---
    bool isActive(const std::string& entity_id) const;
    bool isComplete(const std::string& entity_id) const;
    std::string getCurrentStepId(const std::string& entity_id) const;
    int  getCompletedStepCount(const std::string& entity_id) const;
    int  getTotalStepCount(const std::string& entity_id) const;
    float getElapsed(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::TutorialState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TUTORIAL_SYSTEM_H
