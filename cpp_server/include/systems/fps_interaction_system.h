#ifndef NOVAFORGE_SYSTEMS_FPS_INTERACTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_INTERACTION_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Proximity-based interaction system for FPS characters
 *
 * Detects interactable objects (doors, airlocks, terminals, loot containers)
 * near FPS characters and processes interaction requests.  Bridges the
 * FPSCharacterControllerSystem to InteriorDoorSystem, EVAAirlockSystem,
 * SurvivalSystem and other interior-facing systems.
 */
class FPSInteractionSystem : public ecs::System {
public:
    explicit FPSInteractionSystem(ecs::World* world);
    ~FPSInteractionSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FPSInteractionSystem"; }

    // --- Setup ---

    /** Register an interactable in the world */
    bool createInteractable(const std::string& interactable_id,
                            const std::string& interior_id,
                            const std::string& linked_entity_id,
                            components::FPSInteractable::InteractionType type,
                            float x, float y, float z,
                            float range = 2.0f,
                            const std::string& display_name = "");

    /** Enable or disable an interactable */
    bool setEnabled(const std::string& interactable_id, bool enabled);

    // --- Queries ---

    /** Find the nearest interactable within range of a player */
    std::string findNearestInteractable(const std::string& player_id) const;

    /** Get the distance from a player to an interactable */
    float getDistanceTo(const std::string& player_id,
                        const std::string& interactable_id) const;

    /** Check if a player is within interaction range of an interactable */
    bool isInRange(const std::string& player_id,
                   const std::string& interactable_id) const;

    // --- Actions ---

    /** Player interacts with the nearest interactable (or a specific one) */
    bool interact(const std::string& player_id,
                  const std::string& interactable_id = "",
                  const std::string& player_access = "");

    // --- Helpers ---

    static std::string typeName(int type);

private:
    static constexpr const char* FPS_CHAR_PREFIX = "fpschar_";
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_INTERACTION_SYSTEM_H
