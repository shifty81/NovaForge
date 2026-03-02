#include "systems/fleet_positional_audio_system.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

FleetPositionalAudioSystem::FleetPositionalAudioSystem(ecs::World* world)
    : ecs::System(world) {}

void FleetPositionalAudioSystem::update(float delta_time) {
    if (!world_) return;

    for (auto* entity : world_->getEntities()) {
        auto* audio     = entity->getComponent<components::PositionalAudioSource>();
        auto* formation = entity->getComponent<components::FleetFormation>();
        auto* warp      = entity->getComponent<components::FleetWarpState>();
        if (!audio || !formation) continue;

        // Compute 3D position from formation offset
        computeAudioPosition(
            audio->listener_x, audio->listener_y, audio->listener_z,
            formation->offset_x, formation->offset_y, formation->offset_z,
            audio->source_x, audio->source_y, audio->source_z);

        // Compute distance and attenuation
        float dx = audio->source_x - audio->listener_x;
        float dy = audio->source_y - audio->listener_y;
        float dz = audio->source_z - audio->listener_z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        audio->attenuation = computeAttenuation(dist, audio->min_range, audio->max_range);

        // Warp reverb
        bool in_warp = warp ? warp->in_fleet_warp : false;
        // Default fleet warp speed used for reverb when no per-entity speed is available
        static constexpr float kDefaultFleetWarpSpeed = 3.0f;
        float warp_speed = in_warp ? kDefaultFleetWarpSpeed : 0.0f;
        computeWarpReverb(in_warp, warp_speed, audio->reverb_wet_mix, audio->reverb_decay);
    }
}

} // namespace systems
} // namespace atlas
