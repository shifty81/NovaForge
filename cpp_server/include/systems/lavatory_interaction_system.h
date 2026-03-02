#ifndef NOVAFORGE_SYSTEMS_LAVATORY_INTERACTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_LAVATORY_INTERACTION_SYSTEM_H

#include "ecs/system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Lavatory interaction system (Phase 13)
 *
 * Manages FPS → 3rd-person door transition with audio for lavatory use.
 * Handles the multi-phase interaction: approach, door open, camera switch,
 * use facility, camera return, door close.
 */
class LavatoryInteractionSystem : public ecs::System {
public:
    explicit LavatoryInteractionSystem(ecs::World* world);
    ~LavatoryInteractionSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "LavatoryInteractionSystem"; }

    // Initialization
    bool createLavatory(const std::string& entity_id, const std::string& room_id,
                        float use_duration = 5.0f, float hygiene_bonus = 15.0f);

    // Interaction
    bool beginInteraction(const std::string& entity_id, const std::string& user_id);
    bool cancelInteraction(const std::string& entity_id);

    // Query
    int getPhase(const std::string& entity_id) const;
    float getPhaseProgress(const std::string& entity_id) const;
    bool isOccupied(const std::string& entity_id) const;
    bool isDoorOpen(const std::string& entity_id) const;
    bool isInThirdPerson(const std::string& entity_id) const;
    bool isAudioPlaying(const std::string& entity_id) const;
    float getHygieneBonus(const std::string& entity_id) const;

    static std::string phaseName(int phase);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_LAVATORY_INTERACTION_SYSTEM_H
