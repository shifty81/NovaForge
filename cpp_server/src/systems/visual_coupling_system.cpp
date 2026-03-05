#include "systems/visual_coupling_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

VisualCouplingSystem::VisualCouplingSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void VisualCouplingSystem::updateComponent(ecs::Entity& entity,
    components::VisualCoupling& coupling, float delta_time) {
    if (!coupling.auto_update) return;

    coupling.total_updates++;
}

bool VisualCouplingSystem::initializeCoupling(const std::string& entity_id,
                                               const std::string& ship_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::VisualCoupling>();
    if (existing) return false;

    auto comp = std::make_unique<components::VisualCoupling>();
    comp->ship_id = ship_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool VisualCouplingSystem::addCoupling(const std::string& entity_id,
                                        const std::string& module_id,
                                        components::VisualCoupling::ExteriorFeature feature,
                                        float scale) {
    auto* coupling = getComponentFor(entity_id);
    if (!coupling) return false;

    if (static_cast<int>(coupling->entries.size()) >= coupling->max_entries) return false;

    // Check for duplicate
    if (coupling->findEntry(module_id)) return false;

    components::VisualCoupling::CouplingEntry entry;
    entry.module_id = module_id;
    entry.feature = feature;
    entry.scale = scale;
    entry.visible = true;
    coupling->entries.push_back(entry);
    return true;
}

bool VisualCouplingSystem::removeCoupling(const std::string& entity_id,
                                           const std::string& module_id) {
    auto* coupling = getComponentFor(entity_id);
    if (!coupling) return false;

    auto it = std::remove_if(coupling->entries.begin(), coupling->entries.end(),
        [&](const components::VisualCoupling::CouplingEntry& e) {
            return e.module_id == module_id;
        });
    if (it == coupling->entries.end()) return false;
    coupling->entries.erase(it, coupling->entries.end());
    return true;
}

bool VisualCouplingSystem::setVisibility(const std::string& entity_id,
                                          const std::string& module_id,
                                          bool visible) {
    auto* coupling = getComponentFor(entity_id);
    if (!coupling) return false;

    auto* entry = coupling->findEntry(module_id);
    if (!entry) return false;

    entry->visible = visible;
    return true;
}

bool VisualCouplingSystem::setOffset(const std::string& entity_id,
                                      const std::string& module_id,
                                      float x, float y, float z) {
    auto* coupling = getComponentFor(entity_id);
    if (!coupling) return false;

    auto* entry = coupling->findEntry(module_id);
    if (!entry) return false;

    entry->x_offset = x;
    entry->y_offset = y;
    entry->z_offset = z;
    return true;
}

int VisualCouplingSystem::getCouplingCount(const std::string& entity_id) const {
    auto* coupling = getComponentFor(entity_id);
    if (!coupling) return 0;

    return static_cast<int>(coupling->entries.size());
}

int VisualCouplingSystem::getVisibleCount(const std::string& entity_id) const {
    auto* coupling = getComponentFor(entity_id);
    if (!coupling) return 0;

    return coupling->visibleCount();
}

int VisualCouplingSystem::getFeatureCount(const std::string& entity_id,
                                           components::VisualCoupling::ExteriorFeature feature) const {
    auto* coupling = getComponentFor(entity_id);
    if (!coupling) return 0;

    return coupling->countByFeature(feature);
}

std::string VisualCouplingSystem::getFeatureName(
        components::VisualCoupling::ExteriorFeature feature) const {
    using EF = components::VisualCoupling::ExteriorFeature;
    switch (feature) {
        case EF::SolarPanel: return "SolarPanel";
        case EF::OreContainer: return "OreContainer";
        case EF::Vent: return "Vent";
        case EF::Antenna: return "Antenna";
        case EF::WeaponMount: return "WeaponMount";
        case EF::ShieldEmitter: return "ShieldEmitter";
        case EF::EngineBooster: return "EngineBooster";
        case EF::CargoRack: return "CargoRack";
    }
    return "Unknown";
}

} // namespace systems
} // namespace atlas
