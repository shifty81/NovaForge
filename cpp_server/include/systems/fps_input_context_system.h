#ifndef NOVAFORGE_SYSTEMS_FPS_INPUT_CONTEXT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_INPUT_CONTEXT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>
#include <unordered_map>

namespace atlas {
namespace systems {

/**
 * @brief Manages input context switching between FPS and tactical/fleet modes
 *
 * When a player transitions between game states (InSpace, Docked, StationInterior,
 * ShipInterior, Cockpit), the active input context determines which key bindings
 * are active. This system tracks per-player input context and provides methods
 * to query what actions a given key should perform in the current context.
 *
 * Contexts:
 *   - Tactical: Ship piloting and fleet command (Q=Approach, W=Orbit, etc.)
 *   - FPS:      First-person movement (WASD, Space=Jump, Shift=Sprint, etc.)
 *   - Cockpit:  Hybrid — ship controls with limited head-look
 */
class FPSInputContextSystem : public ecs::SingleComponentSystem<components::FPSCharacterState> {
public:
    enum class InputContext {
        Tactical = 0,  // Space flight / fleet command
        FPS      = 1,  // On-foot first-person
        Cockpit  = 2   // Seated ship controls
    };

    /**
     * FPS key actions that can be triggered in first-person mode.
     */
    enum class FPSAction {
        None = 0,
        MoveForward,
        MoveBackward,
        MoveLeft,
        MoveRight,
        Jump,
        Crouch,
        Sprint,
        Interact,
        Reload,
        ToggleFlashlight,
        Inventory,
        PrimaryFire,
        SecondaryFire,
        BoardShip,       // Context-sensitive: enter ship from station
        ExitToStation,   // Context-sensitive: leave ship to station
        ToggleViewMode
    };

    explicit FPSInputContextSystem(ecs::World* world);
    ~FPSInputContextSystem() override = default;

    std::string getName() const override { return "FPSInputContextSystem"; }

    /**
     * @brief Set the active input context for a player
     */
    bool setContext(const std::string& player_id, InputContext context);

    /**
     * @brief Get the active input context for a player
     */
    InputContext getContext(const std::string& player_id) const;

    /**
     * @brief Register a player into the context system (defaults to Tactical)
     */
    bool registerPlayer(const std::string& player_id);

    /**
     * @brief Remove a player from the context system
     */
    bool unregisterPlayer(const std::string& player_id);

    /**
     * @brief Check if the player's current context is FPS
     */
    bool isInFPSContext(const std::string& player_id) const;

    /**
     * @brief Check if the player's current context is Tactical
     */
    bool isInTacticalContext(const std::string& player_id) const;

    /**
     * @brief Check if the player's current context is Cockpit
     */
    bool isInCockpitContext(const std::string& player_id) const;

    /**
     * @brief Get the FPS action mapped to a given key code in FPS context
     *
     * Returns FPSAction::None if the key has no FPS binding.
     * Key codes use GLFW conventions (e.g. 87 = 'W').
     */
    FPSAction getFPSActionForKey(int key_code) const;

    /**
     * @brief Check if a key should be consumed by the current context
     *
     * When in FPS context, WASD/Space/Shift/etc. should be consumed and not
     * passed through to tactical handlers. When in Tactical context, these
     * keys map to fleet commands instead.
     */
    bool shouldConsumeKey(const std::string& player_id, int key_code) const;

    /**
     * @brief Whether mouse should be captured (hidden + locked) in the current context
     */
    bool shouldCaptureMouse(const std::string& player_id) const;

    /**
     * @brief Whether free mouse look is active (no button hold required)
     */
    bool isFreeLookActive(const std::string& player_id) const;

    /**
     * @brief Get context name string
     */
    static std::string contextName(InputContext ctx);

    /**
     * @brief Get FPS action name string
     */
    static std::string actionName(FPSAction action);

    /**
     * @brief Get number of registered players
     */
    size_t playerCount() const { return m_contexts.size(); }

protected:
    void updateComponent(ecs::Entity& entity, components::FPSCharacterState& state, float delta_time) override;

private:
    std::unordered_map<std::string, InputContext> m_contexts;

    // GLFW key codes for FPS bindings (default layout)
    // W=87, A=65, S=83, D=68, Space=32, LShift=340, C=67, E=69,
    // R=82, F=70, Tab=258, V=86
    static constexpr int KEY_W     = 87;
    static constexpr int KEY_A     = 65;
    static constexpr int KEY_S     = 83;
    static constexpr int KEY_D     = 68;
    static constexpr int KEY_SPACE = 32;
    static constexpr int KEY_LSHIFT = 340;
    static constexpr int KEY_C     = 67;
    static constexpr int KEY_E     = 69;
    static constexpr int KEY_R     = 82;
    static constexpr int KEY_F     = 70;
    static constexpr int KEY_TAB   = 258;
    static constexpr int KEY_V     = 86;
    static constexpr int KEY_LCTRL = 341;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_INPUT_CONTEXT_SYSTEM_H
