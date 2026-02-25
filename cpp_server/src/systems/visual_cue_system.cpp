#include "systems/visual_cue_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

VisualCueSystem::VisualCueSystem(ecs::World* world)
    : System(world) {
}

void VisualCueSystem::update(float /*delta_time*/) {
    auto entities = world_->getEntities<components::SimStarSystemState>();
    for (auto* entity : entities) {
        auto* state = entity->getComponent<components::SimStarSystemState>();
        auto* cue = entity->getComponent<components::VisualCue>();
        if (!state || !cue) continue;

        // Lockdown: active if threat > 0.8 or security < 0.2
        cue->lockdown_active = (state->threat_level > 0.8f || state->security_level < 0.2f);
        cue->lockdown_intensity = std::clamp(state->threat_level, 0.0f, 1.0f);

        // Traffic density
        cue->traffic_density = std::clamp(state->traffic_level, 0.0f, 1.0f);
        cue->traffic_ship_count = static_cast<int>(state->traffic_level * 100.0f);

        // Threat glow
        cue->threat_glow = std::clamp(state->threat_level, 0.0f, 1.0f);

        // Prosperity from economy
        cue->prosperity_indicator = std::clamp(state->economic_index, 0.0f, 1.0f);

        // Pirate warning
        cue->pirate_warning = std::clamp(state->pirate_activity, 0.0f, 1.0f);

        // Resource highlight
        cue->resource_highlight = std::clamp(state->resource_availability, 0.0f, 1.0f);

        // Faction influence: find dominant faction
        float best = 0.0f;
        std::string best_faction;
        for (const auto& pair : state->faction_influence) {
            if (pair.second > best) {
                best = pair.second;
                best_faction = pair.first;
            }
        }
        cue->dominant_faction = best_faction;
        cue->faction_influence_strength = best;
    }
}

// --- Query API ---

bool VisualCueSystem::isLockdownActive(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return false;
    auto* cue = entity->getComponent<components::VisualCue>();
    return cue ? cue->lockdown_active : false;
}

float VisualCueSystem::getTrafficDensity(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0.0f;
    auto* cue = entity->getComponent<components::VisualCue>();
    return cue ? cue->traffic_density : 0.0f;
}

float VisualCueSystem::getThreatGlow(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0.0f;
    auto* cue = entity->getComponent<components::VisualCue>();
    return cue ? cue->threat_glow : 0.0f;
}

float VisualCueSystem::getProsperityIndicator(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0.5f;
    auto* cue = entity->getComponent<components::VisualCue>();
    return cue ? cue->prosperity_indicator : 0.5f;
}

float VisualCueSystem::getPirateWarning(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0.0f;
    auto* cue = entity->getComponent<components::VisualCue>();
    return cue ? cue->pirate_warning : 0.0f;
}

float VisualCueSystem::getResourceHighlight(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0.5f;
    auto* cue = entity->getComponent<components::VisualCue>();
    return cue ? cue->resource_highlight : 0.5f;
}

} // namespace systems
} // namespace atlas
