#include "systems/structure_skin_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

StructureSkinSystem::StructureSkinSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void StructureSkinSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::StructureSkinCollection& comp,
        float delta_time) {
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool StructureSkinSystem::initialize(const std::string& entity_id,
                                      const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity || owner_id.empty()) return false;
    auto comp = std::make_unique<components::StructureSkinCollection>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Collection management
// ---------------------------------------------------------------------------

bool StructureSkinSystem::addSkin(
        const std::string& entity_id,
        const std::string& skin_id,
        const std::string& name,
        components::StructureSkinCollection::StructureType structure_type,
        components::StructureSkinCollection::Rarity rarity,
        const std::string& color_primary,
        const std::string& color_secondary) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (skin_id.empty() || name.empty()) return false;
    if (static_cast<int>(comp->skins.size()) >= comp->max_skins) return false;

    for (const auto& s : comp->skins) {
        if (s.skin_id == skin_id) return false;
    }

    components::StructureSkinCollection::StructureSkin skin;
    skin.skin_id        = skin_id;
    skin.name           = name;
    skin.structure_type = structure_type;
    skin.rarity         = rarity;
    skin.color_primary  = color_primary;
    skin.color_secondary = color_secondary;
    comp->skins.push_back(skin);
    comp->total_acquired++;
    return true;
}

bool StructureSkinSystem::removeSkin(const std::string& entity_id,
                                      const std::string& skin_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->skins.begin(), comp->skins.end(),
        [&](const components::StructureSkinCollection::StructureSkin& s) {
            return s.skin_id == skin_id;
        });
    if (it == comp->skins.end()) return false;
    comp->skins.erase(it);
    return true;
}

// ---------------------------------------------------------------------------
// Application
// ---------------------------------------------------------------------------

bool StructureSkinSystem::applySkin(const std::string& entity_id,
                                     const std::string& skin_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    // Find the target skin
    auto* target = static_cast<components::StructureSkinCollection::StructureSkin*>(nullptr);
    for (auto& s : comp->skins) {
        if (s.skin_id == skin_id) {
            target = &s;
            break;
        }
    }
    if (!target) return false;

    // Unapply any currently applied skin
    for (auto& s : comp->skins) {
        s.applied = false;
    }
    target->applied = true;
    return true;
}

bool StructureSkinSystem::unapplySkin(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    bool any = false;
    for (auto& s : comp->skins) {
        if (s.applied) { s.applied = false; any = true; }
    }
    return any;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int StructureSkinSystem::getSkinCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->skins.size()) : 0;
}

std::string StructureSkinSystem::getAppliedSkinId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return std::string();
    for (const auto& s : comp->skins) {
        if (s.applied) return s.skin_id;
    }
    return std::string();
}

int StructureSkinSystem::getSkinCountByType(
        const std::string& entity_id,
        components::StructureSkinCollection::StructureType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->skins) {
        if (s.structure_type == type) count++;
    }
    return count;
}

int StructureSkinSystem::getTotalAcquired(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_acquired : 0;
}

bool StructureSkinSystem::hasSkin(const std::string& entity_id,
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
