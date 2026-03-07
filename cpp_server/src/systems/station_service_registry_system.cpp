#include "systems/station_service_registry_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using SSR = components::StationServiceRegistry;
using Service = components::StationServiceRegistry::ServiceEntry;

Service* findService(SSR* ssr, const std::string& service_id) {
    for (auto& s : ssr->services) {
        if (s.service_id == service_id) return &s;
    }
    return nullptr;
}

const Service* findServiceConst(const SSR* ssr, const std::string& service_id) {
    for (const auto& s : ssr->services) {
        if (s.service_id == service_id) return &s;
    }
    return nullptr;
}

} // anonymous namespace

StationServiceRegistrySystem::StationServiceRegistrySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void StationServiceRegistrySystem::updateComponent(ecs::Entity& entity,
    components::StationServiceRegistry& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Tick down cooldowns
    for (auto& s : comp.services) {
        if (s.cooldown_remaining > 0.0f) {
            s.cooldown_remaining -= delta_time;
            if (s.cooldown_remaining < 0.0f) {
                s.cooldown_remaining = 0.0f;
            }
        }
    }
}

bool StationServiceRegistrySystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::StationServiceRegistry>();
    entity->addComponent(std::move(comp));
    return true;
}

bool StationServiceRegistrySystem::registerService(const std::string& entity_id,
    const std::string& service_id, const std::string& name,
    components::StationServiceRegistry::ServiceCategory category,
    float cost, float cooldown) {
    auto* ssr = getComponentFor(entity_id);
    if (!ssr) return false;
    if (static_cast<int>(ssr->services.size()) >= ssr->max_services) return false;
    if (findService(ssr, service_id)) return false; // duplicate

    Service s;
    s.service_id = service_id;
    s.name = name;
    s.category = category;
    s.cost = cost;
    s.cooldown = cooldown;
    ssr->services.push_back(s);
    return true;
}

bool StationServiceRegistrySystem::removeService(const std::string& entity_id,
    const std::string& service_id) {
    auto* ssr = getComponentFor(entity_id);
    if (!ssr) return false;
    auto it = std::find_if(ssr->services.begin(), ssr->services.end(),
        [&](const Service& s) { return s.service_id == service_id; });
    if (it == ssr->services.end()) return false;
    ssr->services.erase(it);
    return true;
}

bool StationServiceRegistrySystem::useService(const std::string& entity_id,
    const std::string& service_id) {
    auto* ssr = getComponentFor(entity_id);
    if (!ssr) return false;

    auto* svc = findService(ssr, service_id);
    if (!svc || !svc->available) return false;
    if (svc->cooldown_remaining > 0.0f) return false; // on cooldown

    svc->times_used++;
    svc->cooldown_remaining = svc->cooldown;
    ssr->total_uses++;
    ssr->total_revenue += svc->cost;
    return true;
}

bool StationServiceRegistrySystem::setAvailability(const std::string& entity_id,
    const std::string& service_id, bool available) {
    auto* ssr = getComponentFor(entity_id);
    if (!ssr) return false;

    auto* svc = findService(ssr, service_id);
    if (!svc) return false;
    svc->available = available;
    return true;
}

int StationServiceRegistrySystem::getServiceCount(const std::string& entity_id) const {
    auto* ssr = getComponentFor(entity_id);
    return ssr ? static_cast<int>(ssr->services.size()) : 0;
}

int StationServiceRegistrySystem::getAvailableCount(const std::string& entity_id) const {
    auto* ssr = getComponentFor(entity_id);
    if (!ssr) return 0;
    int count = 0;
    for (const auto& s : ssr->services) {
        if (s.available) count++;
    }
    return count;
}

float StationServiceRegistrySystem::getServiceCost(const std::string& entity_id,
    const std::string& service_id) const {
    auto* ssr = getComponentFor(entity_id);
    if (!ssr) return 0.0f;
    const auto* svc = findServiceConst(ssr, service_id);
    return svc ? svc->cost : 0.0f;
}

int StationServiceRegistrySystem::getTimesUsed(const std::string& entity_id,
    const std::string& service_id) const {
    auto* ssr = getComponentFor(entity_id);
    if (!ssr) return 0;
    const auto* svc = findServiceConst(ssr, service_id);
    return svc ? svc->times_used : 0;
}

bool StationServiceRegistrySystem::isServiceAvailable(const std::string& entity_id,
    const std::string& service_id) const {
    auto* ssr = getComponentFor(entity_id);
    if (!ssr) return false;
    const auto* svc = findServiceConst(ssr, service_id);
    return svc ? svc->available : false;
}

bool StationServiceRegistrySystem::isOnCooldown(const std::string& entity_id,
    const std::string& service_id) const {
    auto* ssr = getComponentFor(entity_id);
    if (!ssr) return false;
    const auto* svc = findServiceConst(ssr, service_id);
    return svc ? (svc->cooldown_remaining > 0.0f) : false;
}

int StationServiceRegistrySystem::getTotalUses(const std::string& entity_id) const {
    auto* ssr = getComponentFor(entity_id);
    return ssr ? ssr->total_uses : 0;
}

float StationServiceRegistrySystem::getTotalRevenue(const std::string& entity_id) const {
    auto* ssr = getComponentFor(entity_id);
    return ssr ? ssr->total_revenue : 0.0f;
}

} // namespace systems
} // namespace atlas
