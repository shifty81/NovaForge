#include "systems/system_security_response_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SystemSecurityResponseSystem::SystemSecurityResponseSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SystemSecurityResponseSystem::updateComponent(ecs::Entity& entity,
    components::SystemSecurityResponse& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Decay response level over time
    if (comp.response_level > 0.0f) {
        comp.response_level = std::max(0.0f, comp.response_level - comp.decay_rate * delta_time);
    }

    // Update state based on thresholds
    if (comp.response_level >= comp.respond_threshold) {
        if (comp.state != components::SystemSecurityResponse::ResponseState::Responding &&
            comp.state != components::SystemSecurityResponse::ResponseState::Engaged) {
            comp.state = components::SystemSecurityResponse::ResponseState::Responding;
        }
    } else if (comp.response_level >= comp.alert_threshold) {
        if (comp.state == components::SystemSecurityResponse::ResponseState::Idle) {
            comp.state = components::SystemSecurityResponse::ResponseState::Alerted;
        }
    } else {
        if (comp.state != components::SystemSecurityResponse::ResponseState::Engaged) {
            comp.state = components::SystemSecurityResponse::ResponseState::Idle;
        }
    }
}

bool SystemSecurityResponseSystem::initialize(const std::string& entity_id,
    const std::string& security_level) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SystemSecurityResponse>();
    comp->system_id = entity_id;
    if (security_level == "highsec") {
        comp->security_level = components::SystemSecurityResponse::SecurityLevel::HighSec;
        comp->alert_threshold = 10.0f;
        comp->respond_threshold = 30.0f;
        comp->decay_rate = 1.0f;
    } else if (security_level == "lowsec") {
        comp->security_level = components::SystemSecurityResponse::SecurityLevel::LowSec;
        comp->alert_threshold = 30.0f;
        comp->respond_threshold = 60.0f;
        comp->decay_rate = 3.0f;
    } else {
        comp->security_level = components::SystemSecurityResponse::SecurityLevel::NullSec;
        comp->alert_threshold = 50.0f;
        comp->respond_threshold = 80.0f;
        comp->decay_rate = 5.0f;
    }
    entity->addComponent(std::move(comp));
    return true;
}

bool SystemSecurityResponseSystem::reportOffence(const std::string& entity_id,
    const std::string& offender_id, const std::string& offence_type, float severity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    severity = std::max(0.0f, std::min(10.0f, severity));

    // Evict oldest if at capacity
    if (static_cast<int>(comp->offences.size()) >= comp->max_offences) {
        comp->offences.erase(comp->offences.begin());
    }

    components::SystemSecurityResponse::Offence offence;
    offence.offender_id = offender_id;
    offence.offence_type = offence_type;
    offence.severity = severity;
    offence.timestamp = comp->elapsed;
    comp->offences.push_back(offence);

    comp->response_level = std::min(100.0f, comp->response_level + severity * 5.0f);
    return true;
}

bool SystemSecurityResponseSystem::dispatchResponse(const std::string& entity_id,
    int ship_count) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::SystemSecurityResponse::ResponseState::Responding) return false;

    comp->response_ships_dispatched += ship_count;
    comp->total_responses++;
    comp->state = components::SystemSecurityResponse::ResponseState::Engaged;
    return true;
}

bool SystemSecurityResponseSystem::clearOffences(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->offences.clear();
    comp->response_level = 0.0f;
    comp->state = components::SystemSecurityResponse::ResponseState::Idle;
    comp->response_ships_dispatched = 0;
    return true;
}

int SystemSecurityResponseSystem::getOffenceCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->offences.size()) : 0;
}

float SystemSecurityResponseSystem::getResponseLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->response_level : 0.0f;
}

std::string SystemSecurityResponseSystem::getState(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "unknown";
    switch (comp->state) {
        case components::SystemSecurityResponse::ResponseState::Idle: return "idle";
        case components::SystemSecurityResponse::ResponseState::Alerted: return "alerted";
        case components::SystemSecurityResponse::ResponseState::Responding: return "responding";
        case components::SystemSecurityResponse::ResponseState::Engaged: return "engaged";
    }
    return "unknown";
}

int SystemSecurityResponseSystem::getResponseShipsDispatched(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->response_ships_dispatched : 0;
}

int SystemSecurityResponseSystem::getTotalResponses(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_responses : 0;
}

} // namespace systems
} // namespace atlas
