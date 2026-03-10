#include "systems/meta_level_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

MetaLevelSystem::MetaLevelSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MetaLevelSystem::updateComponent(ecs::Entity& entity,
    components::MetaLevelState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool MetaLevelSystem::initialize(const std::string& entity_id,
    const std::string& fitting_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MetaLevelState>();
    comp->fitting_id = fitting_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool MetaLevelSystem::addModule(const std::string& entity_id,
    const std::string& module_id, const std::string& base_type,
    int meta_level, float stat_mult, float cpu_mult, float pg_mult) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->modules.size()) >= comp->max_modules) return false;

    components::MetaLevelState::ModuleEntry m;
    m.module_id = module_id;
    m.base_item_type = base_type;
    m.meta_level = (std::max)(0, meta_level);
    m.stat_multiplier = stat_mult;
    m.cpu_multiplier = cpu_mult;
    m.powergrid_multiplier = pg_mult;
    comp->modules.push_back(m);
    return true;
}

bool MetaLevelSystem::removeModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->modules.begin(), comp->modules.end(),
        [&](const components::MetaLevelState::ModuleEntry& m) {
            return m.module_id == module_id;
        });
    if (it == comp->modules.end()) return false;
    comp->modules.erase(it);
    return true;
}

bool MetaLevelSystem::upgradeModule(const std::string& entity_id,
    const std::string& module_id, int new_meta_level) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            int clamped = (std::max)(0, new_meta_level);
            // Adjust multipliers proportionally to meta level change
            float factor = 1.0f + 0.05f * static_cast<float>(clamped - m.meta_level);
            factor = (std::max)(0.1f, factor);
            m.stat_multiplier *= factor;
            m.cpu_multiplier *= factor;
            m.powergrid_multiplier *= factor;
            m.meta_level = clamped;
            return true;
        }
    }
    return false;
}

bool MetaLevelSystem::setDropRate(const std::string& entity_id,
    const std::string& module_id, float drop_rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            m.drop_rate = (std::min)((std::max)(0.0f, drop_rate), 1.0f);
            return true;
        }
    }
    return false;
}

int MetaLevelSystem::getModuleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->modules.size()) : 0;
}

int MetaLevelSystem::getMetaLevel(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return -1;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.meta_level;
    }
    return -1;
}

float MetaLevelSystem::getAverageMetaLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->modules.empty()) return 0.0f;
    float sum = 0.0f;
    for (const auto& m : comp->modules) {
        sum += static_cast<float>(m.meta_level);
    }
    return sum / static_cast<float>(comp->modules.size());
}

float MetaLevelSystem::getStatMultiplier(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.stat_multiplier;
    }
    return 0.0f;
}

float MetaLevelSystem::getCPUMultiplier(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.cpu_multiplier;
    }
    return 0.0f;
}

float MetaLevelSystem::getPowerGridMultiplier(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.powergrid_multiplier;
    }
    return 0.0f;
}

int MetaLevelSystem::getTechIICount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->modules) {
        if (m.meta_level == 5) count++;
    }
    return count;
}

int MetaLevelSystem::getFactionCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->modules) {
        if (m.meta_level >= 6) count++;
    }
    return count;
}

} // namespace systems
} // namespace atlas
