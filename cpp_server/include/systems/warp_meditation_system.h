#ifndef EVE_SYSTEMS_WARP_MEDITATION_SYSTEM_H
#define EVE_SYSTEMS_WARP_MEDITATION_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Updates WarpMeditationLayer and WarpAudioProgression based on WarpState
 *
 * Meditation audio fades in after a sustained warp cruise period.
 * Audio progression moves through Tension → Stabilize → Bloom → Meditative.
 */
class WarpMeditationSystem : public ecs::System {
public:
    explicit WarpMeditationSystem(ecs::World* world);
    ~WarpMeditationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "WarpMeditationSystem"; }

    // --- Query API ---
    float getMeditationVolume(const std::string& entity_id) const;
    bool isMeditationActive(const std::string& entity_id) const;
    components::WarpAudioProgression::Phase getAudioPhase(const std::string& entity_id) const;
    float getAudioProgression(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_WARP_MEDITATION_SYSTEM_H
