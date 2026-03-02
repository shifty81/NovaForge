#include "systems/rig_locker_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

static constexpr float BASE_MODULE_MASS_KG = 1.5f;

RigLockerSystem::RigLockerSystem(ecs::World* world)
    : System(world) {
}

std::string RigLockerSystem::generatePresetId(components::RigLockerPreset* locker) {
    return "preset_" + std::to_string(locker->next_preset_seq++);
}

float RigLockerSystem::calculateMass(const std::vector<std::string>& module_ids) const {
    return static_cast<float>(module_ids.size()) * BASE_MODULE_MASS_KG;
}

void RigLockerSystem::update(float delta_time) {
    // Rig locker is event-driven, no per-tick processing needed
}

bool RigLockerSystem::initializeLocker(const std::string& entity_id,
                                        const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::RigLockerPreset>();
    if (existing) return false;

    auto comp = std::make_unique<components::RigLockerPreset>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool RigLockerSystem::savePreset(const std::string& entity_id, const std::string& name,
                                  const std::vector<std::string>& module_ids) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* locker = entity->getComponent<components::RigLockerPreset>();
    if (!locker) return false;

    if (static_cast<int>(locker->presets.size()) >= locker->max_presets) return false;

    components::RigLockerPreset::Preset preset;
    preset.preset_id = generatePresetId(locker);
    preset.name = name;
    preset.module_ids = module_ids;
    preset.total_mass = calculateMass(module_ids);
    preset.slot_count = static_cast<int>(module_ids.size());
    locker->presets.push_back(preset);
    return true;
}

bool RigLockerSystem::deletePreset(const std::string& entity_id,
                                    const std::string& preset_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* locker = entity->getComponent<components::RigLockerPreset>();
    if (!locker) return false;

    auto it = std::remove_if(locker->presets.begin(), locker->presets.end(),
        [&](const components::RigLockerPreset::Preset& p) {
            return p.preset_id == preset_id;
        });
    if (it == locker->presets.end()) return false;
    locker->presets.erase(it, locker->presets.end());

    if (locker->active_preset_id == preset_id) {
        locker->active_preset_id.clear();
    }
    return true;
}

bool RigLockerSystem::renamePreset(const std::string& entity_id,
                                    const std::string& preset_id,
                                    const std::string& new_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* locker = entity->getComponent<components::RigLockerPreset>();
    if (!locker) return false;

    auto* preset = locker->findPreset(preset_id);
    if (!preset) return false;

    preset->name = new_name;
    return true;
}

bool RigLockerSystem::equipPreset(const std::string& entity_id,
                                   const std::string& preset_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* locker = entity->getComponent<components::RigLockerPreset>();
    if (!locker) return false;

    auto* preset = locker->findPreset(preset_id);
    if (!preset) return false;

    locker->active_preset_id = preset_id;
    locker->total_equips++;
    return true;
}

bool RigLockerSystem::toggleFavorite(const std::string& entity_id,
                                      const std::string& preset_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* locker = entity->getComponent<components::RigLockerPreset>();
    if (!locker) return false;

    auto* preset = locker->findPreset(preset_id);
    if (!preset) return false;

    preset->is_favorite = !preset->is_favorite;
    return true;
}

int RigLockerSystem::getPresetCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* locker = entity->getComponent<components::RigLockerPreset>();
    if (!locker) return 0;

    return static_cast<int>(locker->presets.size());
}

std::string RigLockerSystem::getActivePreset(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* locker = entity->getComponent<components::RigLockerPreset>();
    if (!locker) return "";

    return locker->active_preset_id;
}

int RigLockerSystem::getFavoriteCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* locker = entity->getComponent<components::RigLockerPreset>();
    if (!locker) return 0;

    return locker->favoriteCount();
}

float RigLockerSystem::getPresetMass(const std::string& entity_id,
                                      const std::string& preset_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* locker = entity->getComponent<components::RigLockerPreset>();
    if (!locker) return 0.0f;

    const auto* preset = locker->findPreset(preset_id);
    if (!preset) return 0.0f;

    return preset->total_mass;
}

} // namespace systems
} // namespace atlas
