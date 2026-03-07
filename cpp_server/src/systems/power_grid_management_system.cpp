#include "systems/power_grid_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PowerGridManagementSystem::PowerGridManagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PowerGridManagementSystem::updateComponent(ecs::Entity& entity,
    components::PowerGridState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Recalculate total draw
    float draw = 0.0f;
    for (const auto& m : comp.modules) {
        if (m.online) draw += m.power_draw;
    }
    comp.total_draw = draw;

    // Auto-offline lowest-priority module when overloaded
    if (comp.total_draw > comp.total_output) {
        comp.total_overloads++;

        // Find lowest-priority online module
        int lowest_idx = -1;
        int lowest_pri = 11;
        for (int i = 0; i < static_cast<int>(comp.modules.size()); i++) {
            if (comp.modules[i].online && comp.modules[i].priority < lowest_pri) {
                lowest_pri = comp.modules[i].priority;
                lowest_idx = i;
            }
        }
        if (lowest_idx >= 0) {
            comp.modules[lowest_idx].online = false;
            comp.total_offlined++;
            // Recalculate draw after offline
            draw = 0.0f;
            for (const auto& m : comp.modules) {
                if (m.online) draw += m.power_draw;
            }
            comp.total_draw = draw;
        }
    }
}

bool PowerGridManagementSystem::initialize(const std::string& entity_id,
    float total_output) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PowerGridState>();
    comp->total_output = total_output;
    entity->addComponent(std::move(comp));
    return true;
}

bool PowerGridManagementSystem::addModule(const std::string& entity_id,
    const std::string& module_id, float power_draw, int priority) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->modules.size()) >= comp->max_modules) return false;

    // No duplicate module IDs
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return false;
    }

    components::PowerGridState::FittedModule mod;
    mod.module_id = module_id;
    mod.power_draw = power_draw;
    mod.priority = std::max(1, std::min(10, priority));
    mod.online = false;
    comp->modules.push_back(mod);
    return true;
}

bool PowerGridManagementSystem::onlineModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (m.online) return false;
            // Check if there's enough power
            if (comp->total_draw + m.power_draw > comp->total_output) return false;
            m.online = true;
            comp->total_draw += m.power_draw;
            comp->total_onlined++;
            return true;
        }
    }
    return false;
}

bool PowerGridManagementSystem::offlineModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (!m.online) return false;
            m.online = false;
            comp->total_draw -= m.power_draw;
            comp->total_offlined++;
            return true;
        }
    }
    return false;
}

bool PowerGridManagementSystem::removeModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->modules.begin(); it != comp->modules.end(); ++it) {
        if (it->module_id == module_id) {
            if (it->online) {
                comp->total_draw -= it->power_draw;
                comp->total_offlined++;
            }
            comp->modules.erase(it);
            return true;
        }
    }
    return false;
}

bool PowerGridManagementSystem::setTotalOutput(const std::string& entity_id,
    float output) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->total_output = std::max(0.0f, output);
    return true;
}

int PowerGridManagementSystem::getModuleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->modules.size()) : 0;
}

int PowerGridManagementSystem::getOnlineCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(std::count_if(comp->modules.begin(), comp->modules.end(),
        [](const components::PowerGridState::FittedModule& m) { return m.online; }));
}

float PowerGridManagementSystem::getTotalDraw(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_draw : 0.0f;
}

float PowerGridManagementSystem::getTotalOutput(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_output : 0.0f;
}

float PowerGridManagementSystem::getAvailablePower(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->total_output - comp->total_draw;
}

bool PowerGridManagementSystem::isOverloaded(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->total_draw > comp->total_output;
}

int PowerGridManagementSystem::getTotalOverloads(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_overloads : 0;
}

} // namespace systems
} // namespace atlas
