#include "systems/corporation_logo_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CorporationLogoSystem::CorporationLogoSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void CorporationLogoSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CorporationLogo& comp,
        float delta_time) {
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool CorporationLogoSystem::initialize(const std::string& entity_id,
                                        const std::string& corp_id,
                                        const std::string& corp_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity || corp_id.empty() || corp_name.empty()) return false;
    auto comp = std::make_unique<components::CorporationLogo>();
    comp->corp_id   = corp_id;
    comp->corp_name = corp_name;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Layer management
// ---------------------------------------------------------------------------

bool CorporationLogoSystem::addLayer(
        const std::string& entity_id,
        const std::string& layer_id,
        const std::string& name,
        components::CorporationLogo::LayerType type,
        const std::string& color,
        float opacity,
        float scale) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->published) return false;
    if (layer_id.empty() || name.empty()) return false;
    if (opacity < 0.0f || opacity > 1.0f) return false;
    if (scale <= 0.0f) return false;
    if (static_cast<int>(comp->layers.size()) >= comp->max_layers) return false;

    for (const auto& l : comp->layers) {
        if (l.layer_id == layer_id) return false;
    }

    components::CorporationLogo::LogoLayer layer;
    layer.layer_id = layer_id;
    layer.name     = name;
    layer.type     = type;
    layer.color    = color;
    layer.opacity  = opacity;
    layer.scale    = scale;
    comp->layers.push_back(layer);
    comp->total_edits++;
    return true;
}

bool CorporationLogoSystem::removeLayer(const std::string& entity_id,
                                         const std::string& layer_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->published) return false;
    auto it = std::find_if(comp->layers.begin(), comp->layers.end(),
        [&](const components::CorporationLogo::LogoLayer& l) {
            return l.layer_id == layer_id;
        });
    if (it == comp->layers.end()) return false;

    if (comp->active_layer_id == layer_id) {
        comp->active_layer_id.clear();
    }
    comp->layers.erase(it);
    comp->total_edits++;
    return true;
}

bool CorporationLogoSystem::setActiveLayer(const std::string& entity_id,
                                            const std::string& layer_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->published) return false;

    for (const auto& l : comp->layers) {
        if (l.layer_id == layer_id) {
            comp->active_layer_id = layer_id;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Active layer editing
// ---------------------------------------------------------------------------

bool CorporationLogoSystem::setLayerColor(const std::string& entity_id,
                                           const std::string& color) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active_layer_id.empty()) return false;
    if (comp->published) return false;

    for (auto& l : comp->layers) {
        if (l.layer_id == comp->active_layer_id) {
            l.color = color;
            comp->total_edits++;
            return true;
        }
    }
    return false;
}

bool CorporationLogoSystem::setLayerOpacity(const std::string& entity_id,
                                             float opacity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active_layer_id.empty()) return false;
    if (comp->published) return false;
    if (opacity < 0.0f || opacity > 1.0f) return false;

    for (auto& l : comp->layers) {
        if (l.layer_id == comp->active_layer_id) {
            l.opacity = opacity;
            comp->total_edits++;
            return true;
        }
    }
    return false;
}

bool CorporationLogoSystem::setLayerScale(const std::string& entity_id,
                                           float scale) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active_layer_id.empty()) return false;
    if (comp->published) return false;
    if (scale <= 0.0f) return false;

    for (auto& l : comp->layers) {
        if (l.layer_id == comp->active_layer_id) {
            l.scale = scale;
            comp->total_edits++;
            return true;
        }
    }
    return false;
}

bool CorporationLogoSystem::setLayerOffset(const std::string& entity_id,
                                            float offset_x, float offset_y) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->active_layer_id.empty()) return false;
    if (comp->published) return false;

    for (auto& l : comp->layers) {
        if (l.layer_id == comp->active_layer_id) {
            l.offset_x = offset_x;
            l.offset_y = offset_y;
            comp->total_edits++;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Publication
// ---------------------------------------------------------------------------

bool CorporationLogoSystem::publishLogo(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->published) return false;
    if (comp->layers.empty()) return false;
    comp->published = true;
    return true;
}

bool CorporationLogoSystem::resetLogo(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->layers.clear();
    comp->active_layer_id.clear();
    comp->published = false;
    comp->total_edits++;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int CorporationLogoSystem::getLayerCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->layers.size()) : 0;
}

std::string CorporationLogoSystem::getActiveLayerId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active_layer_id : std::string();
}

bool CorporationLogoSystem::isPublished(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->published : false;
}

int CorporationLogoSystem::getTotalEdits(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_edits : 0;
}

bool CorporationLogoSystem::hasLayer(const std::string& entity_id,
                                      const std::string& layer_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& l : comp->layers) {
        if (l.layer_id == layer_id) return true;
    }
    return false;
}

std::string CorporationLogoSystem::getCorpName(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->corp_name : std::string();
}

} // namespace systems
} // namespace atlas
