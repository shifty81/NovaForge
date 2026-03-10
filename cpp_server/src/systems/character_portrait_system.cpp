#include "systems/character_portrait_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CharacterPortraitSystem::CharacterPortraitSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void CharacterPortraitSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CharacterPortrait& comp,
        float delta_time) {
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool CharacterPortraitSystem::initialize(const std::string& entity_id,
                                          const std::string& character_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity || character_name.empty()) return false;
    auto comp = std::make_unique<components::CharacterPortrait>();
    comp->character_name = character_name;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Preset management
// ---------------------------------------------------------------------------

bool CharacterPortraitSystem::addPreset(
        const std::string& entity_id,
        const std::string& preset_id,
        const std::string& name,
        const std::string& background,
        const std::string& lighting,
        const std::string& pose,
        const std::string& expression,
        float camera_angle) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (preset_id.empty() || name.empty()) return false;
    if (static_cast<int>(comp->presets.size()) >= comp->max_presets) return false;

    // Duplicate prevention
    for (const auto& p : comp->presets) {
        if (p.preset_id == preset_id) return false;
    }

    components::CharacterPortrait::PortraitPreset preset;
    preset.preset_id    = preset_id;
    preset.name         = name;
    preset.background   = background;
    preset.lighting     = lighting;
    preset.pose         = pose;
    preset.expression   = expression;
    preset.camera_angle = camera_angle;
    comp->presets.push_back(preset);
    return true;
}

bool CharacterPortraitSystem::removePreset(const std::string& entity_id,
                                            const std::string& preset_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->presets.begin(), comp->presets.end(),
        [&](const components::CharacterPortrait::PortraitPreset& p) {
            return p.preset_id == preset_id;
        });
    if (it == comp->presets.end()) return false;

    // If removing the active portrait, clear it
    if (comp->active_preset_id == preset_id) {
        comp->active_preset_id.clear();
    }
    comp->presets.erase(it);
    return true;
}

bool CharacterPortraitSystem::setActivePortrait(const std::string& entity_id,
                                                 const std::string& preset_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    // Validate preset exists
    auto it = std::find_if(comp->presets.begin(), comp->presets.end(),
        [&](const components::CharacterPortrait::PortraitPreset& p) {
            return p.preset_id == preset_id;
        });
    if (it == comp->presets.end()) return false;

    comp->active_preset_id = preset_id;
    return true;
}

// ---------------------------------------------------------------------------
// Active portrait editing
// ---------------------------------------------------------------------------

bool CharacterPortraitSystem::setBackground(const std::string& entity_id,
                                             const std::string& bg) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active_preset_id.empty()) return false;

    for (auto& p : comp->presets) {
        if (p.preset_id == comp->active_preset_id) {
            p.background = bg;
            comp->total_updates++;
            return true;
        }
    }
    return false;
}

bool CharacterPortraitSystem::setLighting(const std::string& entity_id,
                                           const std::string& lighting) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active_preset_id.empty()) return false;

    for (auto& p : comp->presets) {
        if (p.preset_id == comp->active_preset_id) {
            p.lighting = lighting;
            comp->total_updates++;
            return true;
        }
    }
    return false;
}

bool CharacterPortraitSystem::setPose(const std::string& entity_id,
                                       const std::string& pose) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active_preset_id.empty()) return false;

    for (auto& p : comp->presets) {
        if (p.preset_id == comp->active_preset_id) {
            p.pose = pose;
            comp->total_updates++;
            return true;
        }
    }
    return false;
}

bool CharacterPortraitSystem::setExpression(const std::string& entity_id,
                                             const std::string& expr) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active_preset_id.empty()) return false;

    for (auto& p : comp->presets) {
        if (p.preset_id == comp->active_preset_id) {
            p.expression = expr;
            comp->total_updates++;
            return true;
        }
    }
    return false;
}

bool CharacterPortraitSystem::setCameraAngle(const std::string& entity_id,
                                              float angle) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active_preset_id.empty()) return false;

    for (auto& p : comp->presets) {
        if (p.preset_id == comp->active_preset_id) {
            p.camera_angle = angle;
            comp->total_updates++;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int CharacterPortraitSystem::getPresetCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->presets.size()) : 0;
}

std::string CharacterPortraitSystem::getActivePresetId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active_preset_id : std::string();
}

std::string CharacterPortraitSystem::getCharacterName(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->character_name : std::string();
}

int CharacterPortraitSystem::getTotalUpdates(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_updates : 0;
}

bool CharacterPortraitSystem::hasPreset(const std::string& entity_id,
                                         const std::string& preset_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& p : comp->presets) {
        if (p.preset_id == preset_id) return true;
    }
    return false;
}

} // namespace systems
} // namespace atlas
