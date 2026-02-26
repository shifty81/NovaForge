#ifndef NOVAFORGE_SYSTEMS_VIEW_MODE_TRANSITION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_VIEW_MODE_TRANSITION_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages seamless transitions between game view modes
 *
 * Handles the state machine for switching between Cockpit (piloting),
 * Interior (FPS walking inside ship), EVA (space walk), and
 * RTS Overlay (tactical fleet command) view modes.
 */
class ViewModeTransitionSystem : public ecs::System {
public:
    explicit ViewModeTransitionSystem(ecs::World* world);
    ~ViewModeTransitionSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ViewModeTransitionSystem"; }

    /**
     * @brief Initialize view mode state for a player (defaults to Cockpit)
     */
    bool initializePlayer(const std::string& player_id);

    /**
     * @brief Request a transition to a new view mode
     * @return true if transition was started (valid transition path)
     */
    bool requestTransition(const std::string& player_id, int target_mode);

    /**
     * @brief Get current view mode for a player
     */
    int getCurrentMode(const std::string& player_id) const;

    /**
     * @brief Check if a player is currently transitioning
     */
    bool isTransitioning(const std::string& player_id) const;

    /**
     * @brief Get the transition progress (0.0 - 1.0)
     */
    float getTransitionProgress(const std::string& player_id) const;

    /**
     * @brief Cancel an in-progress transition, reverting to previous mode
     */
    bool cancelTransition(const std::string& player_id);

    /**
     * @brief Check if a transition from current mode to target is valid
     */
    bool isValidTransition(int from_mode, int to_mode) const;

    /**
     * @brief Get the name of a view mode
     */
    static std::string getModeName(int mode);

private:
    static constexpr const char* VIEWMODE_ENTITY_PREFIX = "viewmode_";
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_VIEW_MODE_TRANSITION_SYSTEM_H
