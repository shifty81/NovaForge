#ifndef NOVAFORGE_SYSTEMS_FPS_CHARACTER_CONTROLLER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_CHARACTER_CONTROLLER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <tuple>

namespace atlas {
namespace systems {

/**
 * @brief First-person character movement controller for ship/station interiors
 *
 * Manages walking, sprinting, crouching, jumping, and gravity for players
 * in FPS mode inside ships and stations. Enforces movement bounds within
 * an interior volume and handles stamina for sprinting.
 */
class FPSCharacterControllerSystem : public ecs::SingleComponentSystem<components::FPSCharacterState> {
public:
    explicit FPSCharacterControllerSystem(ecs::World* world);
    ~FPSCharacterControllerSystem() override = default;

    std::string getName() const override { return "FPSCharacterControllerSystem"; }

    /**
     * @brief Spawn a character into an interior at the given position
     */
    bool spawnCharacter(const std::string& player_id,
                        const std::string& interior_id,
                        float x, float y, float z, float yaw = 0.0f);

    /**
     * @brief Remove a character from FPS mode
     */
    bool removeCharacter(const std::string& player_id);

    /**
     * @brief Set movement input direction (-1..1 each axis)
     */
    bool setMoveInput(const std::string& player_id, float move_x, float move_z);

    /**
     * @brief Set look direction
     */
    bool setLookDirection(const std::string& player_id, float yaw, float pitch);

    /**
     * @brief Request a jump (processed on next update if grounded)
     */
    bool requestJump(const std::string& player_id);

    /**
     * @brief Set character stance (standing / crouching / sprinting)
     */
    bool setStance(const std::string& player_id, int stance);

    /**
     * @brief Set the gravity for a character (0 for zero-g interiors)
     */
    bool setGravity(const std::string& player_id, float gravity);

    /**
     * @brief Get the current position of a character
     */
    std::tuple<float, float, float> getPosition(const std::string& player_id) const;

    /**
     * @brief Get the current yaw and pitch of a character
     */
    std::pair<float, float> getLookDirection(const std::string& player_id) const;

    /**
     * @brief Check if a character is grounded
     */
    bool isGrounded(const std::string& player_id) const;

    /**
     * @brief Get current stamina as fraction (0..1)
     */
    float getStaminaFraction(const std::string& player_id) const;

    /**
     * @brief Get the current stance of a character
     */
    int getStance(const std::string& player_id) const;

    /**
     * @brief Set the current room the character is in (for hazard scoping)
     */
    bool setCurrentRoom(const std::string& player_id, const std::string& room_id);

    /**
     * @brief Get the current room the character is in
     */
    std::string getCurrentRoom(const std::string& player_id) const;

    /**
     * @brief Get the name of a stance
     */
    static std::string stanceName(int stance);

protected:
    void updateComponent(ecs::Entity& entity, components::FPSCharacterState& state, float delta_time) override;

private:
    static constexpr const char* FPS_CHAR_PREFIX = "fpschar_";
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_CHARACTER_CONTROLLER_SYSTEM_H
