#include "systems/mod_manifest_system.h"
#include "ecs/world.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace atlas {
namespace systems {

ModManifestSystem::ModManifestSystem(ecs::World* world)
    : System(world) {
}

void ModManifestSystem::update(float /*delta_time*/) {
    // Mod management is event-driven, not tick-based
}

components::ModRegistry* ModManifestSystem::getOrCreateRegistry() {
    auto* entity = world_->getEntity(REGISTRY_ENTITY_ID);
    if (!entity) {
        entity = world_->createEntity(REGISTRY_ENTITY_ID);
    }
    auto* reg = entity->getComponent<components::ModRegistry>();
    if (!reg) {
        auto r = std::make_unique<components::ModRegistry>();
        reg = r.get();
        entity->addComponent(std::move(r));
    }
    return reg;
}

const components::ModRegistry* ModManifestSystem::getRegistry() const {
    auto* entity = world_->getEntity(REGISTRY_ENTITY_ID);
    if (!entity) return nullptr;
    return entity->getComponent<components::ModRegistry>();
}

bool ModManifestSystem::registerMod(const std::string& mod_id, const std::string& name,
                                     const std::string& version, const std::string& author,
                                     const std::vector<std::string>& dependencies) {
    if (mod_id.empty()) return false;

    auto* reg = getOrCreateRegistry();
    if (reg->findMod(mod_id)) return false;
    if (static_cast<int>(reg->mods.size()) >= reg->max_mods) return false;

    components::ModRegistry::ModInfo info;
    info.mod_id = mod_id;
    info.name = name;
    info.version = version;
    info.author = author;
    info.dependencies = dependencies;
    info.enabled = true;
    reg->mods.push_back(info);
    return true;
}

bool ModManifestSystem::unregisterMod(const std::string& mod_id) {
    auto* reg = getOrCreateRegistry();
    return reg->removeMod(mod_id);
}

bool ModManifestSystem::isModRegistered(const std::string& mod_id) const {
    const auto* reg = getRegistry();
    if (!reg) return false;
    return reg->findMod(mod_id) != nullptr;
}

int ModManifestSystem::getModCount() const {
    const auto* reg = getRegistry();
    if (!reg) return 0;
    return static_cast<int>(reg->mods.size());
}

bool ModManifestSystem::validateAll() const {
    const auto* reg = getRegistry();
    if (!reg) return true;

    for (const auto& mod : reg->mods) {
        for (const auto& dep : mod.dependencies) {
            if (!reg->findMod(dep)) return false;
        }
    }
    return true;
}

bool ModManifestSystem::areDependenciesMet(const std::string& mod_id) const {
    const auto* reg = getRegistry();
    if (!reg) return false;
    const auto* mod = reg->findMod(mod_id);
    if (!mod) return false;

    for (const auto& dep : mod->dependencies) {
        if (!reg->findMod(dep)) return false;
    }
    return true;
}

std::vector<std::string> ModManifestSystem::getLoadOrder() const {
    const auto* reg = getRegistry();
    if (!reg) return {};
    if (reg->mods.empty()) return {};

    // Kahn's algorithm (BFS-based topological sort)
    std::unordered_map<std::string, int> in_degree;
    std::unordered_map<std::string, std::vector<std::string>> dependents;
    std::unordered_set<std::string> all_ids;

    for (const auto& mod : reg->mods) {
        all_ids.insert(mod.mod_id);
        if (in_degree.find(mod.mod_id) == in_degree.end()) {
            in_degree[mod.mod_id] = 0;
        }
        for (const auto& dep : mod.dependencies) {
            if (reg->findMod(dep)) {
                dependents[dep].push_back(mod.mod_id);
                in_degree[mod.mod_id]++;
            }
        }
    }

    std::queue<std::string> queue;
    for (const auto& id : all_ids) {
        if (in_degree[id] == 0) {
            queue.push(id);
        }
    }

    std::vector<std::string> order;
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        order.push_back(current);

        auto it = dependents.find(current);
        if (it != dependents.end()) {
            for (const auto& dep : it->second) {
                in_degree[dep]--;
                if (in_degree[dep] == 0) {
                    queue.push(dep);
                }
            }
        }
    }

    // Circular dependency detected if not all mods are in order
    if (order.size() != all_ids.size()) {
        return {};
    }
    return order;
}

bool ModManifestSystem::setModEnabled(const std::string& mod_id, bool enabled) {
    auto* reg = getOrCreateRegistry();
    auto* mod = reg->findMod(mod_id);
    if (!mod) return false;
    mod->enabled = enabled;
    return true;
}

bool ModManifestSystem::isModEnabled(const std::string& mod_id) const {
    const auto* reg = getRegistry();
    if (!reg) return false;
    const auto* mod = reg->findMod(mod_id);
    if (!mod) return false;
    return mod->enabled;
}

int ModManifestSystem::getEnabledModCount() const {
    const auto* reg = getRegistry();
    if (!reg) return 0;
    int count = 0;
    for (const auto& mod : reg->mods) {
        if (mod.enabled) ++count;
    }
    return count;
}

std::string ModManifestSystem::getModVersion(const std::string& mod_id) const {
    const auto* reg = getRegistry();
    if (!reg) return "";
    const auto* mod = reg->findMod(mod_id);
    if (!mod) return "";
    return mod->version;
}

} // namespace systems
} // namespace atlas
