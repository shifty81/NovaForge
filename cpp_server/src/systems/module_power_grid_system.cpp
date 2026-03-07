#include "systems/module_power_grid_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ModulePowerGridSystem::ModulePowerGridSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

bool ModulePowerGridSystem::initializePowerGrid(const std::string& entity_id,
                                                 float total_cpu,
                                                 float total_pg) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->hasComponent<components::ModulePowerGrid>()) return false;

    auto mpg = std::make_unique<components::ModulePowerGrid>();
    mpg->total_cpu = total_cpu;
    mpg->total_pg = total_pg;
    entity->addComponent(std::move(mpg));
    return true;
}

bool ModulePowerGridSystem::fitModule(const std::string& entity_id,
                                       const std::string& module_id,
                                       const std::string& module_name,
                                       float cpu, float pg) {
    auto* mpg = getComponentFor(entity_id);
    if (!mpg || !mpg->active) return false;
    if (module_id.empty() || cpu < 0.0f || pg < 0.0f) return false;

    // Check for duplicate module_id
    for (const auto& m : mpg->modules) {
        if (m.module_id == module_id) return false;
    }

    // Check if fitting online would exceed budget
    if (mpg->cpu_used + cpu > mpg->total_cpu || mpg->pg_used + pg > mpg->total_pg) {
        return false;
    }

    components::ModulePowerGrid::FittedModule mod;
    mod.module_id = module_id;
    mod.module_name = module_name;
    mod.cpu_usage = cpu;
    mod.pg_usage = pg;
    mod.online = true;
    mpg->modules.push_back(mod);
    recalculateUsage(*mpg);
    return true;
}

bool ModulePowerGridSystem::setModuleOnline(const std::string& entity_id,
                                             const std::string& module_id,
                                             bool online) {
    auto* mpg = getComponentFor(entity_id);
    if (!mpg || !mpg->active) return false;

    for (auto& m : mpg->modules) {
        if (m.module_id == module_id) {
            if (online && !m.online) {
                // Check budget before bringing online
                if (mpg->cpu_used + m.cpu_usage > mpg->total_cpu ||
                    mpg->pg_used + m.pg_usage > mpg->total_pg) {
                    return false;
                }
            }
            m.online = online;
            recalculateUsage(*mpg);
            return true;
        }
    }
    return false;
}

bool ModulePowerGridSystem::removeModule(const std::string& entity_id,
                                          const std::string& module_id) {
    auto* mpg = getComponentFor(entity_id);
    if (!mpg) return false;

    for (auto it = mpg->modules.begin(); it != mpg->modules.end(); ++it) {
        if (it->module_id == module_id) {
            mpg->modules.erase(it);
            recalculateUsage(*mpg);
            return true;
        }
    }
    return false;
}

bool ModulePowerGridSystem::setCapacity(const std::string& entity_id,
                                         float cpu, float pg) {
    auto* mpg = getComponentFor(entity_id);
    if (!mpg || cpu < 0.0f || pg < 0.0f) return false;
    mpg->total_cpu = cpu;
    mpg->total_pg = pg;
    return true;
}

float ModulePowerGridSystem::getCpuUsed(const std::string& entity_id) const {
    auto* mpg = getComponentFor(entity_id);
    return mpg ? mpg->cpu_used : 0.0f;
}

float ModulePowerGridSystem::getPgUsed(const std::string& entity_id) const {
    auto* mpg = getComponentFor(entity_id);
    return mpg ? mpg->pg_used : 0.0f;
}

float ModulePowerGridSystem::getCpuFree(const std::string& entity_id) const {
    auto* mpg = getComponentFor(entity_id);
    return mpg ? mpg->total_cpu - mpg->cpu_used : 0.0f;
}

float ModulePowerGridSystem::getPgFree(const std::string& entity_id) const {
    auto* mpg = getComponentFor(entity_id);
    return mpg ? mpg->total_pg - mpg->pg_used : 0.0f;
}

int ModulePowerGridSystem::getModuleCount(const std::string& entity_id) const {
    auto* mpg = getComponentFor(entity_id);
    return mpg ? static_cast<int>(mpg->modules.size()) : 0;
}

int ModulePowerGridSystem::getOnlineCount(const std::string& entity_id) const {
    auto* mpg = getComponentFor(entity_id);
    if (!mpg) return 0;
    int count = 0;
    for (const auto& m : mpg->modules) {
        if (m.online) ++count;
    }
    return count;
}

int ModulePowerGridSystem::getModulesForcedOffline(const std::string& entity_id) const {
    auto* mpg = getComponentFor(entity_id);
    return mpg ? mpg->modules_forced_offline : 0;
}

void ModulePowerGridSystem::recalculateUsage(components::ModulePowerGrid& mpg) {
    mpg.cpu_used = 0.0f;
    mpg.pg_used = 0.0f;
    for (const auto& m : mpg.modules) {
        if (m.online) {
            mpg.cpu_used += m.cpu_usage;
            mpg.pg_used += m.pg_usage;
        }
    }
}

void ModulePowerGridSystem::enforceCapacity(components::ModulePowerGrid& mpg) {
    // Force modules offline from back to front until within budget
    while ((mpg.cpu_used > mpg.total_cpu || mpg.pg_used > mpg.total_pg) &&
           !mpg.modules.empty()) {
        bool forced = false;
        for (int i = static_cast<int>(mpg.modules.size()) - 1; i >= 0; --i) {
            if (mpg.modules[i].online) {
                mpg.modules[i].online = false;
                mpg.modules_forced_offline++;
                mpg.total_overload_events++;
                recalculateUsage(mpg);
                forced = true;
                break;
            }
        }
        if (!forced) break;  // No online modules left
    }
}

void ModulePowerGridSystem::updateComponent(ecs::Entity& /*entity*/,
                                             components::ModulePowerGrid& mpg,
                                             float delta_time) {
    if (!mpg.active) return;
    mpg.elapsed += delta_time;
    enforceCapacity(mpg);
}

} // namespace systems
} // namespace atlas
