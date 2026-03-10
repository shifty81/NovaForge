#include "systems/citadel_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CitadelManagementSystem::CitadelManagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void CitadelManagementSystem::updateComponent(ecs::Entity& /*entity*/,
                                               components::CitadelState& comp,
                                               float delta_time) {
    comp.elapsed += delta_time;

    // Consume fuel for active services
    if (comp.state != components::CitadelState::StructureState::Destroyed) {
        float total_fuel_per_second = 0.0f;
        for (const auto& svc : comp.services) {
            if (svc.online) {
                total_fuel_per_second += svc.fuel_per_hour / 3600.0f;
            }
        }
        comp.fuel_remaining -= total_fuel_per_second * delta_time;
        if (comp.fuel_remaining <= 0.0f) {
            comp.fuel_remaining = 0.0f;
            // Take all services offline
            for (auto& svc : comp.services) {
                svc.online = false;
            }
        }
    }

    // Count down reinforcement timer
    if (comp.state == components::CitadelState::StructureState::Reinforced) {
        comp.reinforcement_timer -= delta_time;
        if (comp.reinforcement_timer <= 0.0f) {
            comp.reinforcement_timer = 0.0f;
            comp.state = components::CitadelState::StructureState::Vulnerable;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool CitadelManagementSystem::initialize(
        const std::string& entity_id,
        const std::string& owner_corp_id,
        const std::string& structure_name,
        components::CitadelState::CitadelType type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity || owner_corp_id.empty() || structure_name.empty()) return false;
    auto comp = std::make_unique<components::CitadelState>();
    comp->owner_corp_id  = owner_corp_id;
    comp->structure_name = structure_name;
    comp->type           = type;

    // Set HP based on citadel type
    switch (type) {
        case components::CitadelState::CitadelType::Astrahus:
            comp->shield_hp_max = 10000.0f;
            comp->armor_hp_max  = 10000.0f;
            comp->hull_hp_max   = 10000.0f;
            comp->max_services  = 3;
            comp->fuel_capacity = 3000.0f;
            break;
        case components::CitadelState::CitadelType::Fortizar:
            comp->shield_hp_max = 30000.0f;
            comp->armor_hp_max  = 30000.0f;
            comp->hull_hp_max   = 30000.0f;
            comp->max_services  = 5;
            comp->fuel_capacity = 10000.0f;
            break;
        case components::CitadelState::CitadelType::Keepstar:
            comp->shield_hp_max = 100000.0f;
            comp->armor_hp_max  = 100000.0f;
            comp->hull_hp_max   = 100000.0f;
            comp->max_services  = 7;
            comp->fuel_capacity = 50000.0f;
            break;
    }
    comp->shield_hp = comp->shield_hp_max;
    comp->armor_hp  = comp->armor_hp_max;
    comp->hull_hp   = comp->hull_hp_max;
    comp->fuel_remaining = comp->fuel_capacity;

    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Ownership
// ---------------------------------------------------------------------------

bool CitadelManagementSystem::setOwner(const std::string& entity_id,
                                        const std::string& new_owner_corp_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (new_owner_corp_id.empty()) return false;
    if (comp->state == components::CitadelState::StructureState::Destroyed) return false;
    comp->owner_corp_id = new_owner_corp_id;
    return true;
}

// ---------------------------------------------------------------------------
// Services
// ---------------------------------------------------------------------------

bool CitadelManagementSystem::addService(const std::string& entity_id,
                                          const std::string& service_id,
                                          const std::string& service_name,
                                          float fuel_per_hour) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (service_id.empty() || service_name.empty()) return false;
    if (fuel_per_hour < 0.0f) return false;
    if (comp->state == components::CitadelState::StructureState::Destroyed) return false;
    if (static_cast<int>(comp->services.size()) >= comp->max_services) return false;

    // Duplicate prevention
    for (const auto& svc : comp->services) {
        if (svc.service_id == service_id) return false;
    }

    components::CitadelState::Service svc;
    svc.service_id    = service_id;
    svc.service_name  = service_name;
    svc.fuel_per_hour = fuel_per_hour;
    svc.online        = (comp->fuel_remaining > 0.0f);
    comp->services.push_back(svc);
    comp->total_services_installed++;
    return true;
}

bool CitadelManagementSystem::removeService(const std::string& entity_id,
                                             const std::string& service_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->services.begin(), comp->services.end(),
        [&](const components::CitadelState::Service& svc) {
            return svc.service_id == service_id;
        });
    if (it == comp->services.end()) return false;
    comp->services.erase(it);
    return true;
}

bool CitadelManagementSystem::setServiceOnline(const std::string& entity_id,
                                                const std::string& service_id,
                                                bool online) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (online && comp->fuel_remaining <= 0.0f) return false;
    for (auto& svc : comp->services) {
        if (svc.service_id == service_id) {
            svc.online = online;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Fuel
// ---------------------------------------------------------------------------

bool CitadelManagementSystem::addFuel(const std::string& entity_id, float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0.0f) return false;
    if (comp->state == components::CitadelState::StructureState::Destroyed) return false;
    comp->fuel_remaining = std::min(comp->fuel_remaining + amount, comp->fuel_capacity);
    return true;
}

// ---------------------------------------------------------------------------
// Structure state
// ---------------------------------------------------------------------------

bool CitadelManagementSystem::setVulnerable(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::CitadelState::StructureState::Online) return false;
    comp->state = components::CitadelState::StructureState::Vulnerable;
    return true;
}

bool CitadelManagementSystem::triggerReinforcement(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::CitadelState::StructureState::Vulnerable) return false;
    comp->state = components::CitadelState::StructureState::Reinforced;
    comp->reinforcement_timer = comp->reinforcement_duration;
    comp->total_reinforcements++;
    return true;
}

bool CitadelManagementSystem::repairStructure(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state == components::CitadelState::StructureState::Destroyed) return false;
    comp->shield_hp = comp->shield_hp_max;
    comp->armor_hp  = comp->armor_hp_max;
    comp->hull_hp   = comp->hull_hp_max;
    comp->state     = components::CitadelState::StructureState::Online;
    return true;
}

bool CitadelManagementSystem::applyDamage(const std::string& entity_id,
                                           float damage) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (damage <= 0.0f) return false;
    if (comp->state == components::CitadelState::StructureState::Destroyed) return false;
    if (comp->state == components::CitadelState::StructureState::Reinforced) return false;

    float remaining = damage;

    // Shield layer
    if (comp->shield_hp > 0.0f) {
        float absorbed = std::min(remaining, comp->shield_hp);
        comp->shield_hp -= absorbed;
        remaining -= absorbed;
    }

    // Armor layer
    if (remaining > 0.0f && comp->armor_hp > 0.0f) {
        float absorbed = std::min(remaining, comp->armor_hp);
        comp->armor_hp -= absorbed;
        remaining -= absorbed;
    }

    // Hull layer
    if (remaining > 0.0f && comp->hull_hp > 0.0f) {
        float absorbed = std::min(remaining, comp->hull_hp);
        comp->hull_hp -= absorbed;
        remaining -= absorbed;
    }

    // Check destruction
    if (comp->hull_hp <= 0.0f) {
        comp->hull_hp = 0.0f;
        comp->state = components::CitadelState::StructureState::Destroyed;
    }

    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

components::CitadelState::StructureState
CitadelManagementSystem::getState(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->state : components::CitadelState::StructureState::Online;
}

float CitadelManagementSystem::getFuelRemaining(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fuel_remaining : 0.0f;
}

int CitadelManagementSystem::getActiveServiceCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& svc : comp->services) {
        if (svc.online) count++;
    }
    return count;
}

int CitadelManagementSystem::getServiceCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->services.size()) : 0;
}

float CitadelManagementSystem::getShieldHp(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->shield_hp : 0.0f;
}

float CitadelManagementSystem::getArmorHp(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->armor_hp : 0.0f;
}

float CitadelManagementSystem::getHullHp(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->hull_hp : 0.0f;
}

int CitadelManagementSystem::getTotalReinforcements(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_reinforcements : 0;
}

float CitadelManagementSystem::getReinforcementTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->reinforcement_timer : 0.0f;
}

} // namespace systems
} // namespace atlas
