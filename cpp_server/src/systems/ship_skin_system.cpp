#include "systems/ship_skin_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ShipSkinSystem::ShipSkinSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void ShipSkinSystem::updateComponent(ecs::Entity& /*entity*/,
                                      components::ShipSkinCollection& comp,
                                      float delta_time) {
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool ShipSkinSystem::initialize(const std::string& entity_id,
                                 const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity || owner_id.empty()) return false;
    auto comp = std::make_unique<components::ShipSkinCollection>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Skin management
// ---------------------------------------------------------------------------

bool ShipSkinSystem::addSkin(const std::string& entity_id,
                              const std::string& skin_id,
                              const std::string& name,
                              const std::string& ship_type,
                              components::ShipSkinCollection::Rarity rarity,
                              const std::string& color_primary,
                              const std::string& color_secondary) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (skin_id.empty() || name.empty()) return false;
    if (static_cast<int>(comp->skins.size()) >= comp->max_skins) return false;

    // Duplicate prevention
    for (const auto& s : comp->skins) {
        if (s.skin_id == skin_id) return false;
    }

    components::ShipSkinCollection::Skin skin;
    skin.skin_id         = skin_id;
    skin.name            = name;
    skin.ship_type       = ship_type;
    skin.rarity          = rarity;
    skin.color_primary   = color_primary;
    skin.color_secondary = color_secondary;
    comp->skins.push_back(skin);
    comp->total_acquired++;
    return true;
}

bool ShipSkinSystem::removeSkin(const std::string& entity_id,
                                 const std::string& skin_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->skins.begin(), comp->skins.end(),
        [&](const components::ShipSkinCollection::Skin& s) {
            return s.skin_id == skin_id;
        });
    if (it == comp->skins.end()) return false;
    comp->skins.erase(it);
    return true;
}

bool ShipSkinSystem::equipSkin(const std::string& entity_id,
                                const std::string& skin_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->skins.begin(), comp->skins.end(),
        [&](const components::ShipSkinCollection::Skin& s) {
            return s.skin_id == skin_id;
        });
    if (it == comp->skins.end()) return false;

    // Unequip any currently equipped skin
    for (auto& s : comp->skins) {
        s.equipped = false;
    }
    it->equipped = true;
    return true;
}

bool ShipSkinSystem::unequipSkin(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    bool had_equipped = false;
    for (auto& s : comp->skins) {
        if (s.equipped) had_equipped = true;
        s.equipped = false;
    }
    return had_equipped;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int ShipSkinSystem::getSkinCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->skins.size()) : 0;
}

std::string ShipSkinSystem::getEquippedSkinId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return std::string();
    for (const auto& s : comp->skins) {
        if (s.equipped) return s.skin_id;
    }
    return std::string();
}

int ShipSkinSystem::getSkinCountByRarity(
        const std::string& entity_id,
        components::ShipSkinCollection::Rarity rarity) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->skins) {
        if (s.rarity == rarity) count++;
    }
    return count;
}

int ShipSkinSystem::getTotalAcquired(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_acquired : 0;
}

bool ShipSkinSystem::hasSkin(const std::string& entity_id,
                              const std::string& skin_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->skins) {
        if (s.skin_id == skin_id) return true;
    }
    return false;
}

} // namespace systems
} // namespace atlas
