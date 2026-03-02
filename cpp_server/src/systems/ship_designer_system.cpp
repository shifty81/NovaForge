#include "systems/ship_designer_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

ShipDesignerSystem::ShipDesignerSystem(ecs::World* world)
    : System(world) {
}

void ShipDesignerSystem::update(float delta_time) {
    (void)delta_time;
    auto entities = world_->getEntities<components::ShipDesigner>();
    for (auto* entity : entities) {
        auto* sd = entity->getComponent<components::ShipDesigner>();
        if (!sd || !sd->active) continue;

        // Recalculate used_cpu and used_powergrid
        float cpu = 0.0f;
        float power = 0.0f;
        for (const auto& m : sd->fitted_modules) {
            cpu += m.cpu_cost;
            power += m.power_cost;
        }
        sd->used_cpu = cpu;
        sd->used_powergrid = power;

        // Validate
        sd->valid = !sd->blueprint_name.empty() &&
                    sd->used_cpu <= sd->total_cpu &&
                    sd->used_powergrid <= sd->total_powergrid;
    }
}

bool ShipDesignerSystem::createDesigner(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->getComponent<components::ShipDesigner>()) return false;

    auto comp = std::make_unique<components::ShipDesigner>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ShipDesignerSystem::setBlueprint(const std::string& entity_id, const std::string& name,
                                       const std::string& hull_type, const std::string& faction) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd) return false;

    sd->blueprint_name = name;
    sd->hull_type = hull_type;
    sd->faction = faction;
    sd->design_count++;
    return true;
}

bool ShipDesignerSystem::fitModule(const std::string& entity_id, const std::string& module_name,
                                    int slot_type, float cpu_cost, float power_cost) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd) return false;

    // Check slot availability
    int max_slots = 0;
    switch (slot_type) {
        case 0: max_slots = sd->high_slots; break;
        case 1: max_slots = sd->mid_slots; break;
        case 2: max_slots = sd->low_slots; break;
        case 3: max_slots = sd->rig_slots; break;
        default: return false;
    }

    int used_slots = 0;
    for (const auto& m : sd->fitted_modules) {
        if (m.slot_type == slot_type) used_slots++;
    }
    if (used_slots >= max_slots) return false;

    components::ShipDesigner::SlotEntry entry;
    entry.module_name = module_name;
    entry.slot_type = slot_type;
    entry.cpu_cost = cpu_cost;
    entry.power_cost = power_cost;
    sd->fitted_modules.push_back(entry);
    sd->used_cpu += cpu_cost;
    sd->used_powergrid += power_cost;
    return true;
}

bool ShipDesignerSystem::removeModule(const std::string& entity_id, const std::string& module_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd) return false;

    for (auto it = sd->fitted_modules.begin(); it != sd->fitted_modules.end(); ++it) {
        if (it->module_name == module_name) {
            sd->used_cpu -= it->cpu_cost;
            sd->used_powergrid -= it->power_cost;
            sd->fitted_modules.erase(it);
            return true;
        }
    }
    return false;
}

bool ShipDesignerSystem::validateDesign(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd) return false;

    sd->valid = !sd->blueprint_name.empty() &&
                sd->used_cpu <= sd->total_cpu &&
                sd->used_powergrid <= sd->total_powergrid;
    return sd->valid;
}

float ShipDesignerSystem::getCpuUsage(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd || sd->total_cpu <= 0.0f) return 0.0f;

    return sd->used_cpu / sd->total_cpu;
}

float ShipDesignerSystem::getPowerUsage(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd || sd->total_powergrid <= 0.0f) return 0.0f;

    return sd->used_powergrid / sd->total_powergrid;
}

int ShipDesignerSystem::getFittedCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd) return 0;

    return static_cast<int>(sd->fitted_modules.size());
}

int ShipDesignerSystem::getSlotsFree(const std::string& entity_id, int slot_type) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd) return 0;

    int max_slots = 0;
    switch (slot_type) {
        case 0: max_slots = sd->high_slots; break;
        case 1: max_slots = sd->mid_slots; break;
        case 2: max_slots = sd->low_slots; break;
        case 3: max_slots = sd->rig_slots; break;
        default: return 0;
    }

    int used = 0;
    for (const auto& m : sd->fitted_modules) {
        if (m.slot_type == slot_type) used++;
    }
    return max_slots - used;
}

bool ShipDesignerSystem::clearDesign(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd) return false;

    sd->fitted_modules.clear();
    sd->used_cpu = 0.0f;
    sd->used_powergrid = 0.0f;
    sd->valid = false;
    return true;
}

bool ShipDesignerSystem::isValid(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sd = entity->getComponent<components::ShipDesigner>();
    if (!sd) return false;

    return sd->valid;
}

} // namespace systems
} // namespace atlas
