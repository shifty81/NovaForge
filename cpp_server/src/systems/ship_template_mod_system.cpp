#include "systems/ship_template_mod_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>
#include <unordered_map>

namespace atlas {
namespace systems {

ShipTemplateModSystem::ShipTemplateModSystem(ecs::World* world)
    : System(world) {
}

void ShipTemplateModSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::ShipTemplateMod>();

    // Build lookup map for base template inheritance (O(n) instead of O(n²))
    std::unordered_map<std::string, components::ShipTemplateMod*> template_lookup;
    for (auto* entity : entities) {
        auto* mod = entity->getComponent<components::ShipTemplateMod>();
        if (mod && !mod->template_id.empty()) {
            template_lookup[mod->template_id] = mod;
        }
    }

    for (auto* entity : entities) {
        auto* mod = entity->getComponent<components::ShipTemplateMod>();
        if (!mod) continue;

        // Inherit from base template if needed
        if (!mod->base_template_id.empty() && mod->needs_inheritance) {
            auto it = template_lookup.find(mod->base_template_id);
            if (it != template_lookup.end()) {
                auto* baseMod = it->second;
                mod->hull_hp = baseMod->hull_hp;
                mod->shield_hp = baseMod->shield_hp;
                mod->armor_hp = baseMod->armor_hp;
                mod->max_velocity = baseMod->max_velocity;
                mod->agility = baseMod->agility;
                mod->high_slots = baseMod->high_slots;
                mod->mid_slots = baseMod->mid_slots;
                mod->low_slots = baseMod->low_slots;
                mod->needs_inheritance = false;
            }
        }

        // Auto-validate
        if (!mod->ship_name.empty() && !mod->ship_class.empty() &&
            !mod->faction.empty() && mod->hull_hp > 0.0f) {
            mod->validated = true;
        }
    }
}

bool ShipTemplateModSystem::registerTemplate(const std::string& entity_id,
                                              const std::string& template_id,
                                              const std::string& ship_name,
                                              const std::string& ship_class,
                                              const std::string& faction) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    if (entity->getComponent<components::ShipTemplateMod>()) return false;

    auto comp = std::make_unique<components::ShipTemplateMod>();
    comp->template_id = template_id;
    comp->ship_name = ship_name;
    comp->ship_class = ship_class;
    comp->faction = faction;
    entity->addComponent(std::move(comp));
    return true;
}

bool ShipTemplateModSystem::setBaseTemplate(const std::string& entity_id,
                                             const std::string& base_template_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* mod = entity->getComponent<components::ShipTemplateMod>();
    if (!mod) return false;

    mod->base_template_id = base_template_id;
    return true;
}

bool ShipTemplateModSystem::setModSource(const std::string& entity_id,
                                          const std::string& mod_source, int priority) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* mod = entity->getComponent<components::ShipTemplateMod>();
    if (!mod) return false;

    mod->mod_source = mod_source;
    mod->priority = priority;
    return true;
}

bool ShipTemplateModSystem::validateTemplate(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* mod = entity->getComponent<components::ShipTemplateMod>();
    if (!mod) return false;

    if (!mod->ship_name.empty() && !mod->ship_class.empty() &&
        !mod->faction.empty() && mod->hull_hp > 0.0f) {
        mod->validated = true;
        return true;
    }
    return false;
}

bool ShipTemplateModSystem::isValid(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* mod = entity->getComponent<components::ShipTemplateMod>();
    if (!mod) return false;

    return mod->validated;
}

std::string ShipTemplateModSystem::getTemplateId(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* mod = entity->getComponent<components::ShipTemplateMod>();
    if (!mod) return "";

    return mod->template_id;
}

std::string ShipTemplateModSystem::getShipClass(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* mod = entity->getComponent<components::ShipTemplateMod>();
    if (!mod) return "";

    return mod->ship_class;
}

int ShipTemplateModSystem::getHighestPriority() const {
    auto entities = world_->getEntities<components::ShipTemplateMod>();
    int highest = 0;
    for (auto* entity : entities) {
        auto* mod = entity->getComponent<components::ShipTemplateMod>();
        if (mod && mod->priority > highest) {
            highest = mod->priority;
        }
    }
    return highest;
}

int ShipTemplateModSystem::getTemplateCount() const {
    auto entities = world_->getEntities<components::ShipTemplateMod>();
    return static_cast<int>(entities.size());
}

} // namespace systems
} // namespace atlas
