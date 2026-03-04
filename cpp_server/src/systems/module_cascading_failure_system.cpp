#include "systems/module_cascading_failure_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::ModuleCascadingFailure::ModuleState* findModule(
    components::ModuleCascadingFailure* mcf, const std::string& module_id) {
    for (auto& m : mcf->modules) {
        if (m.module_id == module_id) return &m;
    }
    return nullptr;
}

const components::ModuleCascadingFailure::ModuleState* findModuleConst(
    const components::ModuleCascadingFailure* mcf, const std::string& module_id) {
    for (const auto& m : mcf->modules) {
        if (m.module_id == module_id) return &m;
    }
    return nullptr;
}
} // anonymous namespace

ModuleCascadingFailureSystem::ModuleCascadingFailureSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void ModuleCascadingFailureSystem::updateComponent(ecs::Entity& /*entity*/, components::ModuleCascadingFailure& mcf, float /*delta_time*/) {
    if (!mcf.active) return;

    // Propagate cascading failures: if a dependency is destroyed/offline,
    // the dependent module goes offline too
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& mod : mcf.modules) {
            if (mod.destroyed || !mod.online) continue;
            for (const auto& dep_id : mod.depends_on) {
                auto* dep = findModule(&mcf, dep_id);
                if (dep && (dep->destroyed || !dep->online)) {
                    mod.online = false;
                    mcf.cascade_events++;
                    changed = true;
                    break;
                }
            }
        }
    }
}

bool ModuleCascadingFailureSystem::initializeShip(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ModuleCascadingFailure>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ModuleCascadingFailureSystem::addModule(const std::string& entity_id,
    const std::string& module_id, const std::string& module_type, float max_hp) {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return false;
    if (static_cast<int>(mcf->modules.size()) >= mcf->max_modules) return false;
    if (findModule(mcf, module_id)) return false; // duplicate

    components::ModuleCascadingFailure::ModuleState mod;
    mod.module_id = module_id;
    mod.module_type = module_type;
    mod.hp = max_hp;
    mod.max_hp = max_hp;
    mcf->modules.push_back(mod);
    return true;
}

bool ModuleCascadingFailureSystem::addDependency(const std::string& entity_id,
    const std::string& module_id, const std::string& depends_on_id) {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return false;
    auto* mod = findModule(mcf, module_id);
    if (!mod) return false;
    // Verify dependency target exists
    if (!findModule(mcf, depends_on_id)) return false;
    // Prevent duplicate dependencies
    for (const auto& d : mod->depends_on) {
        if (d == depends_on_id) return false;
    }
    mod->depends_on.push_back(depends_on_id);
    return true;
}

bool ModuleCascadingFailureSystem::damageModule(const std::string& entity_id,
    const std::string& module_id, float damage) {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return false;
    auto* mod = findModule(mcf, module_id);
    if (!mod || mod->destroyed) return false;

    mod->hp = std::max(0.0f, mod->hp - damage);
    if (mod->hp <= 0.0f) {
        mod->destroyed = true;
        mod->online = false;
        mcf->total_failures++;
    }
    return true;
}

bool ModuleCascadingFailureSystem::repairModule(const std::string& entity_id,
    const std::string& module_id, float amount) {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return false;
    auto* mod = findModule(mcf, module_id);
    if (!mod) return false;

    mod->hp = std::min(mod->max_hp, mod->hp + amount);
    if (mod->hp > 0.0f && mod->destroyed) {
        mod->destroyed = false;
        mod->online = true;
    }
    return true;
}

bool ModuleCascadingFailureSystem::isModuleOnline(const std::string& entity_id,
    const std::string& module_id) const {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return false;
    auto* mod = findModuleConst(mcf, module_id);
    if (!mod) return false;
    return mod->online;
}

bool ModuleCascadingFailureSystem::isModuleDestroyed(const std::string& entity_id,
    const std::string& module_id) const {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return false;
    auto* mod = findModuleConst(mcf, module_id);
    if (!mod) return false;
    return mod->destroyed;
}

float ModuleCascadingFailureSystem::getModuleHP(const std::string& entity_id,
    const std::string& module_id) const {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return 0.0f;
    auto* mod = findModuleConst(mcf, module_id);
    if (!mod) return 0.0f;
    return mod->hp;
}

int ModuleCascadingFailureSystem::getOnlineModuleCount(const std::string& entity_id) const {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return 0;
    int count = 0;
    for (const auto& m : mcf->modules) {
        if (m.online) count++;
    }
    return count;
}

int ModuleCascadingFailureSystem::getTotalFailures(const std::string& entity_id) const {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return 0;
    return mcf->total_failures;
}

int ModuleCascadingFailureSystem::getCascadeEvents(const std::string& entity_id) const {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return 0;
    return mcf->cascade_events;
}

int ModuleCascadingFailureSystem::getModuleCount(const std::string& entity_id) const {
    auto* mcf = getComponentFor(entity_id);
    if (!mcf) return 0;
    return static_cast<int>(mcf->modules.size());
}

} // namespace systems
} // namespace atlas
