#ifndef NOVAFORGE_SYSTEMS_EVA_AIRLOCK_EXIT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_EVA_AIRLOCK_EXIT_SYSTEM_H

#include "ecs/system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief EVA airlock exit system (Phase 13)
 *
 * Manages exiting to space when the ship is undocked. Checks docking state,
 * validates suit oxygen, handles tether range, and manages the exit/return lifecycle.
 */
class EVAAirlockExitSystem : public ecs::System {
public:
    explicit EVAAirlockExitSystem(ecs::World* world);
    ~EVAAirlockExitSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "EVAAirlockExitSystem"; }

    // Initialization
    bool createExitPoint(const std::string& entity_id, const std::string& ship_id,
                         float max_tether_range = 200.0f);

    // State management
    bool requestExit(const std::string& entity_id, const std::string& player_id,
                     float suit_oxygen);
    bool setDockState(const std::string& entity_id, bool docked);
    bool beginReturn(const std::string& entity_id);
    bool cancelExit(const std::string& entity_id);

    // EVA movement
    bool moveAway(const std::string& entity_id, float distance);

    // Query
    int getState(const std::string& entity_id) const;
    float getStateProgress(const std::string& entity_id) const;
    bool isExitBlocked(const std::string& entity_id) const;
    bool isInSpace(const std::string& entity_id) const;
    float getDistanceFromShip(const std::string& entity_id) const;
    bool isTetherActive(const std::string& entity_id) const;

    static std::string stateName(int state);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_EVA_AIRLOCK_EXIT_SYSTEM_H
