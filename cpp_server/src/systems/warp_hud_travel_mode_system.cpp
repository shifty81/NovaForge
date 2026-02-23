#include "systems/warp_hud_travel_mode_system.h"
#include "components/game_components.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

WarpHUDTravelModeSystem::WarpHUDTravelModeSystem(ecs::World* world)
    : ecs::System(world) {}

void WarpHUDTravelModeSystem::computeTargets(int phase,
                                              float& target_edge_softness,
                                              float& target_desaturation,
                                              float& target_warning_mute,
                                              float& target_safe_area_padding,
                                              float& target_hud_scale) {
    // Default: everything normal
    target_edge_softness     = 0.0f;
    target_desaturation      = 0.0f;
    target_warning_mute      = 0.0f;
    target_safe_area_padding = 0.0f;
    target_hud_scale         = 1.0f;

    // Phase mapping:
    //   0 = None     → normal HUD
    //   1 = Align    → slight softening begins
    //   2 = Entry    → partial softening
    //   3 = Cruise   → full travel mode
    //   4 = Event    → slightly heightened (anomaly awareness)
    //   5 = Exit     → fading back to normal
    switch (phase) {
        case 0: // None
            break;
        case 1: // Align
            target_edge_softness     = 0.1f;
            target_desaturation      = 0.05f;
            target_warning_mute      = 0.0f;
            target_safe_area_padding = 8.0f;
            target_hud_scale         = 0.99f;
            break;
        case 2: // Entry
            target_edge_softness     = 0.35f;
            target_desaturation      = 0.15f;
            target_warning_mute      = 0.3f;
            target_safe_area_padding = 24.0f;
            target_hud_scale         = 0.97f;
            break;
        case 3: // Cruise
            target_edge_softness     = 0.6f;
            target_desaturation      = 0.3f;
            target_warning_mute      = 0.7f;
            target_safe_area_padding = 40.0f;
            target_hud_scale         = 0.95f;
            break;
        case 4: // Event (anomaly — reduce muting so alerts show)
            target_edge_softness     = 0.4f;
            target_desaturation      = 0.2f;
            target_warning_mute      = 0.3f;
            target_safe_area_padding = 32.0f;
            target_hud_scale         = 0.96f;
            break;
        case 5: // Exit
            target_edge_softness     = 0.15f;
            target_desaturation      = 0.05f;
            target_warning_mute      = 0.0f;
            target_safe_area_padding = 10.0f;
            target_hud_scale         = 0.99f;
            break;
        default:
            break;
    }
}

void WarpHUDTravelModeSystem::computeUIFlair(float time, float bass_level,
                                              float& bracket_anim, float& glow,
                                              float& parallax) {
    // Bracket animation: slow oscillation (~0.15 Hz)
    bracket_anim = 0.5f + 0.5f * std::sin(time * 0.15f * 2.0f * 3.14159265f);

    // Glow synced to engine bass (0–1)
    glow = std::clamp(bass_level * 0.8f, 0.0f, 1.0f);

    // Parallax: gentle sinusoidal shift (~0.1 Hz, max ±3 pixels)
    parallax = 3.0f * std::sin(time * 0.1f * 2.0f * 3.14159265f);
}

// Smoothly move current toward target by at most ramp_speed * dt
static float rampToward(float current, float target, float ramp_speed, float dt) {
    float diff = target - current;
    float step = ramp_speed * dt;
    if (std::abs(diff) <= step) return target;
    return current + (diff > 0.0f ? step : -step);
}

void WarpHUDTravelModeSystem::update(float delta_time) {
    if (!world_) return;

    for (auto* entity : world_->getEntities()) {
        auto* warpState = entity->getComponent<components::WarpState>();
        auto* hudMode   = entity->getComponent<components::WarpHUDTravelMode>();
        if (!warpState || !hudMode) continue;

        int phase = static_cast<int>(warpState->phase);

        float te, td, tw, tp, ts;
        computeTargets(phase, te, td, tw, tp, ts);

        float ramp = kDefaultRampSpeed;

        // Ramp current values toward targets
        hudMode->edge_softness      = rampToward(hudMode->edge_softness,      te, ramp, delta_time);
        hudMode->color_desaturation = rampToward(hudMode->color_desaturation, td, ramp, delta_time);
        hudMode->warning_mute       = rampToward(hudMode->warning_mute,       tw, ramp, delta_time);
        hudMode->safe_area_padding  = rampToward(hudMode->safe_area_padding,  tp, ramp * 30.0f, delta_time);
        hudMode->hud_scale          = rampToward(hudMode->hud_scale,          ts, ramp * 0.05f, delta_time);

        // Optional UI flair
        if (hudMode->ui_flair_enabled && phase >= 2 && phase <= 4) {
            float bass_level = 0.0f;
            auto* audioCfg = entity->getComponent<components::WarpAudioProfile>();
            if (audioCfg) bass_level = audioCfg->engine_core_volume;

            computeUIFlair(warpState->warp_time, bass_level,
                           hudMode->bracket_animation,
                           hudMode->ui_glow_intensity,
                           hudMode->hud_parallax_offset);
        } else {
            hudMode->bracket_animation   = 0.0f;
            hudMode->ui_glow_intensity   = 0.0f;
            hudMode->hud_parallax_offset = 0.0f;
        }
    }
}

} // namespace systems
} // namespace atlas
