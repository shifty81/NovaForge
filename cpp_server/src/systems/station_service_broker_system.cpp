#include "systems/station_service_broker_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {
using SSS = components::StationServiceState;
using Service = SSS::Service;

Service* findService(SSS* sss, const std::string& service_id) {
    for (auto& s : sss->services) {
        if (s.service_id == service_id) return &s;
    }
    return nullptr;
}

const Service* findServiceConst(const SSS* sss, const std::string& service_id) {
    for (const auto& s : sss->services) {
        if (s.service_id == service_id) return &s;
    }
    return nullptr;
}
} // anonymous namespace

StationServiceBrokerSystem::StationServiceBrokerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void StationServiceBrokerSystem::updateComponent(ecs::Entity& entity,
    components::StationServiceState& sss, float delta_time) {
    if (!sss.active) return;

    // Normalize demand modifiers toward 1.0 over time (mean reversion)
    for (auto& svc : sss.services) {
        if (svc.demand_modifier > 1.0f) {
            svc.demand_modifier = std::max(1.0f, svc.demand_modifier - 0.01f * delta_time);
        } else if (svc.demand_modifier < 1.0f) {
            svc.demand_modifier = std::min(1.0f, svc.demand_modifier + 0.01f * delta_time);
        }
    }
    sss.elapsed += delta_time;
}

bool StationServiceBrokerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::StationServiceState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool StationServiceBrokerSystem::addService(const std::string& entity_id,
    const std::string& service_id, float base_cost) {
    auto* sss = getComponentFor(entity_id);
    if (!sss) return false;
    if (service_id.empty() || base_cost < 0.0f) return false;
    if (static_cast<int>(sss->services.size()) >= sss->max_services) return false;
    if (findService(sss, service_id)) return false; // duplicate

    Service svc;
    svc.service_id = service_id;
    svc.base_cost = base_cost;
    svc.demand_modifier = 1.0f;
    svc.available = true;
    sss->services.push_back(svc);
    return true;
}

bool StationServiceBrokerSystem::removeService(const std::string& entity_id,
    const std::string& service_id) {
    auto* sss = getComponentFor(entity_id);
    if (!sss) return false;
    auto it = std::find_if(sss->services.begin(), sss->services.end(),
        [&](const Service& s) { return s.service_id == service_id; });
    if (it == sss->services.end()) return false;
    sss->services.erase(it);
    return true;
}

bool StationServiceBrokerSystem::setServiceAvailable(const std::string& entity_id,
    const std::string& service_id, bool available) {
    auto* sss = getComponentFor(entity_id);
    if (!sss) return false;
    auto* svc = findService(sss, service_id);
    if (!svc) return false;
    svc->available = available;
    return true;
}

bool StationServiceBrokerSystem::setDemandModifier(const std::string& entity_id,
    const std::string& service_id, float modifier) {
    auto* sss = getComponentFor(entity_id);
    if (!sss) return false;
    auto* svc = findService(sss, service_id);
    if (!svc) return false;
    svc->demand_modifier = std::max(0.1f, std::min(5.0f, modifier));
    return true;
}

bool StationServiceBrokerSystem::setTaxRate(const std::string& entity_id, float rate) {
    auto* sss = getComponentFor(entity_id);
    if (!sss) return false;
    sss->tax_rate = std::max(0.0f, std::min(0.5f, rate));
    return true;
}

bool StationServiceBrokerSystem::setStandingDiscount(const std::string& entity_id, float discount) {
    auto* sss = getComponentFor(entity_id);
    if (!sss) return false;
    sss->standing_discount = std::max(0.0f, std::min(0.5f, discount));
    return true;
}

bool StationServiceBrokerSystem::processTransaction(const std::string& entity_id,
    const std::string& service_id) {
    auto* sss = getComponentFor(entity_id);
    if (!sss) return false;
    auto* svc = findService(sss, service_id);
    if (!svc || !svc->available) return false;

    float cost = svc->base_cost * svc->demand_modifier * (1.0f + sss->tax_rate) * (1.0f - sss->standing_discount);
    sss->total_revenue += cost;
    sss->total_transactions++;
    // Increase demand slightly after transaction
    svc->demand_modifier = std::min(5.0f, svc->demand_modifier + 0.05f);
    return true;
}

float StationServiceBrokerSystem::getServiceCost(const std::string& entity_id,
    const std::string& service_id) const {
    auto* sss = getComponentFor(entity_id);
    if (!sss) return 0.0f;
    auto* svc = findServiceConst(sss, service_id);
    if (!svc) return 0.0f;
    return svc->base_cost * svc->demand_modifier * (1.0f + sss->tax_rate) * (1.0f - sss->standing_discount);
}

bool StationServiceBrokerSystem::isServiceAvailable(const std::string& entity_id,
    const std::string& service_id) const {
    auto* sss = getComponentFor(entity_id);
    if (!sss) return false;
    auto* svc = findServiceConst(sss, service_id);
    return svc ? svc->available : false;
}

float StationServiceBrokerSystem::getTaxRate(const std::string& entity_id) const {
    auto* sss = getComponentFor(entity_id);
    return sss ? sss->tax_rate : 0.0f;
}

float StationServiceBrokerSystem::getStandingDiscount(const std::string& entity_id) const {
    auto* sss = getComponentFor(entity_id);
    return sss ? sss->standing_discount : 0.0f;
}

float StationServiceBrokerSystem::getTotalRevenue(const std::string& entity_id) const {
    auto* sss = getComponentFor(entity_id);
    return sss ? sss->total_revenue : 0.0f;
}

int StationServiceBrokerSystem::getTotalTransactions(const std::string& entity_id) const {
    auto* sss = getComponentFor(entity_id);
    return sss ? sss->total_transactions : 0;
}

int StationServiceBrokerSystem::getServiceCount(const std::string& entity_id) const {
    auto* sss = getComponentFor(entity_id);
    return sss ? static_cast<int>(sss->services.size()) : 0;
}

} // namespace systems
} // namespace atlas
