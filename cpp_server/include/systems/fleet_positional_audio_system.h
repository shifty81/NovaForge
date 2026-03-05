#ifndef NOVAFORGE_SYSTEMS_FLEET_POSITIONAL_AUDIO_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_POSITIONAL_AUDIO_SYSTEM_H

#include "ecs/single_component_system.h"
#include <string>
#include <algorithm>
#include <cmath>

namespace atlas {

namespace components {
struct PositionalAudioSource;
}

namespace systems {

/**
 * FleetPositionalAudioSystem — computes 3D audio source positions for
 * fleet member voices based on their formation offset, and applies
 * warp tunnel reverb when the fleet is in warp.
 *
 * Reads: FleetFormation (offset_x/y/z), FleetWarpState (in_fleet_warp),
 *        PositionalAudioSource (base settings).
 * Writes: PositionalAudioSource (computed 3D position, reverb params).
 *
 * Design:
 *   - Each fleet member's voice originates from their formation position.
 *   - During warp, a reverb effect is applied (wet mix, decay) to simulate
 *     the enclosed warp tunnel environment.
 *   - Attenuation follows inverse-distance with a min/max range.
 */
class FleetPositionalAudioSystem : public ecs::SingleComponentSystem<components::PositionalAudioSource> {
public:
    explicit FleetPositionalAudioSystem(ecs::World* world);
    ~FleetPositionalAudioSystem() override = default;

    std::string getName() const override { return "FleetPositionalAudioSystem"; }

    /**
     * Compute 3D audio position from fleet formation offset and
     * the fleet commander's world position.
     */
    static inline void computeAudioPosition(float cmd_x, float cmd_y, float cmd_z,
                                            float off_x, float off_y, float off_z,
                                            float& out_x, float& out_y, float& out_z) {
        out_x = cmd_x + off_x;
        out_y = cmd_y + off_y;
        out_z = cmd_z + off_z;
    }

    /**
     * Compute attenuation factor based on distance from listener.
     * Uses inverse-distance model clamped to [min_range, max_range].
     * @return Attenuation factor 0.0–1.0
     */
    static inline float computeAttenuation(float distance, float min_range, float max_range) {
        if (min_range < 0.0f) min_range = 0.0f;
        if (max_range <= min_range) return 0.0f;
        if (distance <= min_range) return 1.0f;
        if (distance >= max_range) return 0.0f;

        return 1.0f - (distance - min_range) / (max_range - min_range);
    }

    /**
     * Compute warp tunnel reverb parameters.
     * During warp, voices get reverb to simulate the enclosed tunnel.
     */
    static inline void computeWarpReverb(bool in_warp, float warp_speed,
                                         float& wet_mix, float& decay) {
        if (!in_warp) {
            wet_mix = 0.0f;
            decay   = 0.0f;
            return;
        }

        static constexpr float kBaseWetMix = 0.3f;
        static constexpr float kBaseDecay  = 1.5f;

        float speed_factor = std::clamp(warp_speed / 10.0f, 0.0f, 1.0f);

        wet_mix = kBaseWetMix + 0.3f * speed_factor;
        decay   = kBaseDecay  + 1.0f * speed_factor;

        wet_mix = std::clamp(wet_mix, 0.0f, 1.0f);
        decay   = std::max(decay, 0.0f);
    }

protected:
    void updateComponent(ecs::Entity& entity, components::PositionalAudioSource& audio, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_POSITIONAL_AUDIO_SYSTEM_H
