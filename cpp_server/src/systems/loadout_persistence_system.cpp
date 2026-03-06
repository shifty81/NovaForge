#include "systems/loadout_persistence_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

LoadoutPersistenceSystem::LoadoutPersistenceSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void LoadoutPersistenceSystem::updateComponent(ecs::Entity& /*entity*/,
    components::SavedLoadout& sl, float delta_time) {
    if (!sl.active) return;
    sl.elapsed += delta_time;
}

bool LoadoutPersistenceSystem::saveLoadout(const std::string& entity_id,
    const std::string& loadout_id, const std::string& loadout_name,
    const std::string& ship_class) {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return false;

    // Check if already exists — overwrite
    for (auto& lo : sl->loadouts) {
        if (lo.loadout_id == loadout_id) {
            lo.loadout_name = loadout_name;
            lo.ship_class = ship_class;
            lo.saved_at = sl->elapsed;
            lo.modules.clear();
            return true;
        }
    }

    // Check max
    if (static_cast<int>(sl->loadouts.size()) >= sl->max_loadouts) return false;

    components::SavedLoadout::Loadout lo;
    lo.loadout_id = loadout_id;
    lo.loadout_name = loadout_name;
    lo.ship_class = ship_class;
    lo.saved_at = sl->elapsed;
    sl->loadouts.push_back(lo);
    return true;
}

bool LoadoutPersistenceSystem::addModuleToLoadout(const std::string& entity_id,
    const std::string& loadout_id, const std::string& module_id,
    const std::string& module_name, int slot_index, const std::string& slot_type) {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return false;

    for (auto& lo : sl->loadouts) {
        if (lo.loadout_id == loadout_id) {
            // Check for duplicate slot_index within same slot_type
            for (const auto& mod : lo.modules) {
                if (mod.slot_index == slot_index && mod.slot_type == slot_type) return false;
            }
            components::SavedLoadout::ModuleSlot ms;
            ms.module_id = module_id;
            ms.module_name = module_name;
            ms.slot_index = slot_index;
            ms.slot_type = slot_type;
            lo.modules.push_back(ms);
            return true;
        }
    }
    return false;
}

bool LoadoutPersistenceSystem::deleteLoadout(const std::string& entity_id,
    const std::string& loadout_id) {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return false;

    for (auto it = sl->loadouts.begin(); it != sl->loadouts.end(); ++it) {
        if (it->loadout_id == loadout_id) {
            sl->loadouts.erase(it);
            if (sl->active_loadout_id == loadout_id) {
                sl->active_loadout_id.clear();
            }
            return true;
        }
    }
    return false;
}

bool LoadoutPersistenceSystem::renameLoadout(const std::string& entity_id,
    const std::string& loadout_id, const std::string& new_name) {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return false;

    for (auto& lo : sl->loadouts) {
        if (lo.loadout_id == loadout_id) {
            lo.loadout_name = new_name;
            return true;
        }
    }
    return false;
}

bool LoadoutPersistenceSystem::setActiveLoadout(const std::string& entity_id,
    const std::string& loadout_id) {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return false;

    for (const auto& lo : sl->loadouts) {
        if (lo.loadout_id == loadout_id) {
            sl->active_loadout_id = loadout_id;
            return true;
        }
    }
    return false;
}

std::string LoadoutPersistenceSystem::getActiveLoadoutId(const std::string& entity_id) const {
    auto* sl = getComponentFor(entity_id);
    return sl ? sl->active_loadout_id : "";
}

int LoadoutPersistenceSystem::getLoadoutCount(const std::string& entity_id) const {
    auto* sl = getComponentFor(entity_id);
    return sl ? static_cast<int>(sl->loadouts.size()) : 0;
}

int LoadoutPersistenceSystem::getModuleCount(const std::string& entity_id,
    const std::string& loadout_id) const {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return 0;
    for (const auto& lo : sl->loadouts) {
        if (lo.loadout_id == loadout_id) return static_cast<int>(lo.modules.size());
    }
    return 0;
}

std::string LoadoutPersistenceSystem::getLoadoutName(const std::string& entity_id,
    const std::string& loadout_id) const {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return "";
    for (const auto& lo : sl->loadouts) {
        if (lo.loadout_id == loadout_id) return lo.loadout_name;
    }
    return "";
}

std::string LoadoutPersistenceSystem::getLoadoutShipClass(const std::string& entity_id,
    const std::string& loadout_id) const {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return "";
    for (const auto& lo : sl->loadouts) {
        if (lo.loadout_id == loadout_id) return lo.ship_class;
    }
    return "";
}

bool LoadoutPersistenceSystem::hasLoadout(const std::string& entity_id,
    const std::string& loadout_id) const {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return false;
    for (const auto& lo : sl->loadouts) {
        if (lo.loadout_id == loadout_id) return true;
    }
    return false;
}

bool LoadoutPersistenceSystem::hasModule(const std::string& entity_id,
    const std::string& loadout_id, const std::string& module_id) const {
    auto* sl = getComponentFor(entity_id);
    if (!sl) return false;
    for (const auto& lo : sl->loadouts) {
        if (lo.loadout_id == loadout_id) {
            for (const auto& mod : lo.modules) {
                if (mod.module_id == module_id) return true;
            }
            return false;
        }
    }
    return false;
}

} // namespace systems
} // namespace atlas
