#include "systems/mod_manager_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>
#include <set>

namespace atlas {
namespace systems {

ModManagerSystem::ModManagerSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void ModManagerSystem::updateComponent(ecs::Entity& /*entity*/, components::ModManager& comp, float /*delta_time*/) {
    if (!comp.active) return;

    // Recalculate load_order based on dependencies (topological sort)
    int n = static_cast<int>(comp.mods.size());
    // Build index map
    std::map<std::string, int> id_to_idx;
    for (int i = 0; i < n; i++) {
        id_to_idx[comp.mods[i].mod_id] = i;
    }

    // Simple topological assignment: iterate and assign order
    std::vector<int> order(n, 0);
    // For each mod, its order = max(order of dependencies) + 1
    // Repeat n times to propagate
    for (int pass = 0; pass < n; pass++) {
        for (int i = 0; i < n; i++) {
            for (const auto& dep : comp.mods[i].dependencies) {
                auto it = id_to_idx.find(dep);
                if (it != id_to_idx.end()) {
                    if (order[i] <= order[it->second]) {
                        order[i] = order[it->second] + 1;
                    }
                }
            }
        }
    }
    for (int i = 0; i < n; i++) {
        comp.mods[i].load_order = order[i];
    }

    // Update total_enabled count
    int enabled = 0;
    for (const auto& mod : comp.mods) {
        if (mod.enabled) enabled++;
    }
    comp.total_enabled = enabled;
}

bool ModManagerSystem::createManager(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->getComponent<components::ModManager>()) return false;

    auto comp = std::make_unique<components::ModManager>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ModManagerSystem::installMod(const std::string& entity_id, const std::string& mod_id,
                                   const std::string& name, const std::string& version,
                                   const std::string& author) {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return false;

    // Check max_mods limit
    if (static_cast<int>(mgr->mods.size()) >= mgr->max_mods) return false;

    // Check for duplicate
    for (const auto& mod : mgr->mods) {
        if (mod.mod_id == mod_id) return false;
    }

    components::ModManager::ModEntry entry;
    entry.mod_id = mod_id;
    entry.name = name;
    entry.version = version;
    entry.author = author;
    entry.installed = true;
    entry.enabled = false;
    entry.load_order = static_cast<int>(mgr->mods.size());
    mgr->mods.push_back(entry);
    mgr->total_installed = static_cast<int>(mgr->mods.size());
    return true;
}

bool ModManagerSystem::uninstallMod(const std::string& entity_id, const std::string& mod_id) {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return false;

    for (auto it = mgr->mods.begin(); it != mgr->mods.end(); ++it) {
        if (it->mod_id == mod_id) {
            mgr->mods.erase(it);
            mgr->total_installed = static_cast<int>(mgr->mods.size());
            return true;
        }
    }
    return false;
}

bool ModManagerSystem::enableMod(const std::string& entity_id, const std::string& mod_id) {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return false;

    for (auto& mod : mgr->mods) {
        if (mod.mod_id == mod_id) {
            if (mod.enabled) return false;
            mod.enabled = true;
            mgr->total_enabled++;
            return true;
        }
    }
    return false;
}

bool ModManagerSystem::disableMod(const std::string& entity_id, const std::string& mod_id) {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return false;

    for (auto& mod : mgr->mods) {
        if (mod.mod_id == mod_id) {
            if (!mod.enabled) return false;
            mod.enabled = false;
            mgr->total_enabled--;
            return true;
        }
    }
    return false;
}

bool ModManagerSystem::addDependency(const std::string& entity_id, const std::string& mod_id,
                                      const std::string& depends_on) {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return false;

    for (auto& mod : mgr->mods) {
        if (mod.mod_id == mod_id) {
            mod.dependencies.push_back(depends_on);
            return true;
        }
    }
    return false;
}

bool ModManagerSystem::addConflict(const std::string& entity_id, const std::string& mod_id_a,
                                    const std::string& mod_id_b) {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return false;

    std::string key = mod_id_a + ":" + mod_id_b;
    mgr->conflicts.push_back(key);
    return true;
}

bool ModManagerSystem::hasConflict(const std::string& entity_id, const std::string& mod_id) const {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return false;

    // Collect enabled mod IDs
    std::set<std::string> enabled_ids;
    for (const auto& mod : mgr->mods) {
        if (mod.enabled) enabled_ids.insert(mod.mod_id);
    }

    for (const auto& conflict : mgr->conflicts) {
        auto sep = conflict.find(':');
        if (sep == std::string::npos) continue;
        std::string a = conflict.substr(0, sep);
        std::string b = conflict.substr(sep + 1);
        if (a == mod_id && enabled_ids.count(b)) return true;
        if (b == mod_id && enabled_ids.count(a)) return true;
    }
    return false;
}

int ModManagerSystem::getModCount(const std::string& entity_id) const {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return 0;

    return static_cast<int>(mgr->mods.size());
}

int ModManagerSystem::getEnabledCount(const std::string& entity_id) const {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return 0;

    return mgr->total_enabled;
}

bool ModManagerSystem::isInstalled(const std::string& entity_id, const std::string& mod_id) const {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return false;

    for (const auto& mod : mgr->mods) {
        if (mod.mod_id == mod_id) return true;
    }
    return false;
}

std::vector<std::string> ModManagerSystem::getLoadOrder(const std::string& entity_id) const {
    auto* mgr = getComponentFor(entity_id);
    if (!mgr) return {};

    // Sort mods by load_order
    std::vector<const components::ModManager::ModEntry*> sorted;
    for (const auto& mod : mgr->mods) {
        sorted.push_back(&mod);
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto* a, const auto* b) {
        return a->load_order < b->load_order;
    });

    std::vector<std::string> result;
    for (const auto* mod : sorted) {
        result.push_back(mod->mod_id);
    }
    return result;
}

} // namespace systems
} // namespace atlas
