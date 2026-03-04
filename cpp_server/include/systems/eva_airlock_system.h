#ifndef NOVAFORGE_SYSTEMS_EVA_AIRLOCK_SYSTEM_H
#define NOVAFORGE_SYSTEMS_EVA_AIRLOCK_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages EVA airlock sequences for exiting/entering ships and stations
 *
 * Handles the multi-step depressurization cycle for EVA transitions:
 * inner seal → depressurize → outer open → EVA, and the reverse
 * for re-entry. Checks suit oxygen before allowing EVA.
 */
class EVAAirlockSystem : public ecs::SingleComponentSystem<components::EVAAirlockState> {
public:
    explicit EVAAirlockSystem(ecs::World* world);
    ~EVAAirlockSystem() override = default;

    std::string getName() const override { return "EVAAirlockSystem"; }

    /**
     * @brief Create an airlock entity on a ship/station
     */
    bool createAirlock(const std::string& airlock_id,
                       const std::string& ship_id,
                       float phase_duration = 2.0f);

    /**
     * @brief Begin the EVA exit sequence
     * @param suit_oxygen Current oxygen level of the player's suit
     * @return true if the sequence was started
     */
    bool beginEVA(const std::string& airlock_id,
                  const std::string& player_id,
                  float suit_oxygen);

    /**
     * @brief Begin the re-entry sequence (EVA → interior)
     */
    bool beginReentry(const std::string& airlock_id,
                      const std::string& player_id);

    /**
     * @brief Abort the current airlock sequence (returns to Idle if possible)
     */
    bool abortSequence(const std::string& airlock_id);

    /**
     * @brief Get the current phase of the airlock
     */
    int getPhase(const std::string& airlock_id) const;

    /**
     * @brief Get the progress through the current phase (0..1)
     */
    float getPhaseProgress(const std::string& airlock_id) const;

    /**
     * @brief Get the chamber pressure (0..1)
     */
    float getChamberPressure(const std::string& airlock_id) const;

    /**
     * @brief Check if the player is currently in EVA
     */
    bool isInEVA(const std::string& airlock_id) const;

    /**
     * @brief Get the name of an EVA phase
     */
    static std::string phaseName(int phase);

protected:
    void updateComponent(ecs::Entity& entity, components::EVAAirlockState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_EVA_AIRLOCK_SYSTEM_H
