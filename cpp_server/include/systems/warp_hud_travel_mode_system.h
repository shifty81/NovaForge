#ifndef NOVAFORGE_SYSTEMS_WARP_HUD_TRAVEL_MODE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WARP_HUD_TRAVEL_MODE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * WarpHUDTravelModeSystem — manages HUD softening during warp.
 *
 * Reads WarpState (phase) and writes WarpHUDTravelMode:
 *   - During cruise: ramps edge_softness, desaturation, padding, scale
 *   - On exit: ramps everything back to normal
 *   - If ui_flair_enabled: computes bracket animation, glow, parallax
 *
 * Transition speed is configurable via ramp_speed (per-second).
 */
class WarpHUDTravelModeSystem : public ecs::SingleComponentSystem<components::WarpHUDTravelMode> {
public:
    explicit WarpHUDTravelModeSystem(ecs::World* world);
    ~WarpHUDTravelModeSystem() override = default;

    std::string getName() const override { return "WarpHUDTravelModeSystem"; }

    /**
     * Compute HUD travel mode targets for a given warp phase.
     * Static so it can be tested independently.
     * @param phase     Warp phase enum ordinal (0=None..5=Exit)
     * @param[out] target_edge_softness      Target edge softness (0–1)
     * @param[out] target_desaturation       Target color desaturation (0–1)
     * @param[out] target_warning_mute       Target warning muting (0–1)
     * @param[out] target_safe_area_padding  Target safe-area padding in pixels
     * @param[out] target_hud_scale          Target HUD scale factor
     */
    static void computeTargets(int phase,
                               float& target_edge_softness,
                               float& target_desaturation,
                               float& target_warning_mute,
                               float& target_safe_area_padding,
                               float& target_hud_scale);

    /**
     * Compute UI flair values for optional animated effects during warp.
     * @param time         Accumulated warp time in seconds
     * @param bass_level   Current engine bass level (0–1), from WarpAudioProfile
     * @param[out] bracket_anim   Bracket animation offset (0–1)
     * @param[out] glow           Glow intensity synced to bass (0–1)
     * @param[out] parallax       Parallax offset in pixels
     */
    static void computeUIFlair(float time, float bass_level,
                               float& bracket_anim, float& glow,
                               float& parallax);

    /** Per-second ramp speed for soft transitions (default 1.5). */
    static constexpr float kDefaultRampSpeed = 1.5f;

protected:
    void updateComponent(ecs::Entity& entity, components::WarpHUDTravelMode& hudMode, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WARP_HUD_TRAVEL_MODE_SYSTEM_H
