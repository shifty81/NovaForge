#include "systems/ship_loadout_preset_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using SLP = components::ShipLoadoutPresets;
using Preset = components::ShipLoadoutPresets::LoadoutPreset;

Preset* findPreset(SLP* slp, const std::string& preset_name) {
    for (auto& p : slp->presets) {
        if (p.preset_name == preset_name) return &p;
    }
    return nullptr;
}

const Preset* findPresetConst(const SLP* slp, const std::string& preset_name) {
    for (const auto& p : slp->presets) {
        if (p.preset_name == preset_name) return &p;
    }
    return nullptr;
}

} // anonymous namespace

ShipLoadoutPresetSystem::ShipLoadoutPresetSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ShipLoadoutPresetSystem::updateComponent(ecs::Entity& entity,
    components::ShipLoadoutPresets& slp, float delta_time) {
    if (!slp.active) return;
    slp.elapsed += delta_time;
}

bool ShipLoadoutPresetSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ShipLoadoutPresets>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ShipLoadoutPresetSystem::savePreset(const std::string& entity_id,
    const std::string& preset_name, const std::string& ship_type) {
    auto* slp = getComponentFor(entity_id);
    if (!slp) return false;
    if (static_cast<int>(slp->presets.size()) >= slp->max_presets) return false;
    if (findPreset(slp, preset_name)) return false;
    if (preset_name.empty() || ship_type.empty()) return false;

    Preset p;
    p.preset_name = preset_name;
    p.ship_type = ship_type;
    slp->presets.push_back(p);
    slp->total_presets_saved++;
    return true;
}

bool ShipLoadoutPresetSystem::addModuleToPreset(const std::string& entity_id,
    const std::string& preset_name, const std::string& module_name,
    const std::string& slot) {
    auto* slp = getComponentFor(entity_id);
    if (!slp) return false;

    auto* preset = findPreset(slp, preset_name);
    if (!preset) return false;
    if (static_cast<int>(preset->modules.size()) >= slp->max_modules_per_preset) return false;
    if (module_name.empty() || slot.empty()) return false;

    Preset::ModuleSlot ms;
    ms.module_name = module_name;
    ms.slot = slot;
    preset->modules.push_back(ms);
    return true;
}

bool ShipLoadoutPresetSystem::removePreset(const std::string& entity_id,
    const std::string& preset_name) {
    auto* slp = getComponentFor(entity_id);
    if (!slp) return false;

    auto it = std::find_if(slp->presets.begin(), slp->presets.end(),
        [&](const Preset& p) { return p.preset_name == preset_name; });
    if (it == slp->presets.end()) return false;
    slp->presets.erase(it);
    return true;
}

bool ShipLoadoutPresetSystem::renamePreset(const std::string& entity_id,
    const std::string& old_name, const std::string& new_name) {
    auto* slp = getComponentFor(entity_id);
    if (!slp) return false;
    if (new_name.empty()) return false;
    if (findPreset(slp, new_name)) return false;

    auto* preset = findPreset(slp, old_name);
    if (!preset) return false;

    preset->preset_name = new_name;
    return true;
}

int ShipLoadoutPresetSystem::getPresetCount(const std::string& entity_id) const {
    auto* slp = getComponentFor(entity_id);
    return slp ? static_cast<int>(slp->presets.size()) : 0;
}

int ShipLoadoutPresetSystem::getModuleCount(const std::string& entity_id,
    const std::string& preset_name) const {
    auto* slp = getComponentFor(entity_id);
    if (!slp) return 0;
    const auto* preset = findPresetConst(slp, preset_name);
    return preset ? static_cast<int>(preset->modules.size()) : 0;
}

bool ShipLoadoutPresetSystem::hasPreset(const std::string& entity_id,
    const std::string& preset_name) const {
    auto* slp = getComponentFor(entity_id);
    if (!slp) return false;
    return findPresetConst(slp, preset_name) != nullptr;
}

std::string ShipLoadoutPresetSystem::getPresetShipType(const std::string& entity_id,
    const std::string& preset_name) const {
    auto* slp = getComponentFor(entity_id);
    if (!slp) return "";
    const auto* preset = findPresetConst(slp, preset_name);
    return preset ? preset->ship_type : "";
}

int ShipLoadoutPresetSystem::getTotalPresetsSaved(const std::string& entity_id) const {
    auto* slp = getComponentFor(entity_id);
    return slp ? slp->total_presets_saved : 0;
}

} // namespace systems
} // namespace atlas
