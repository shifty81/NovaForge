#include "systems/fleet_readiness_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using FR = components::FleetReadinessState;
}

FleetReadinessSystem::FleetReadinessSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetReadinessSystem::updateComponent(ecs::Entity& entity,
    components::FleetReadinessState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool FleetReadinessSystem::initialize(const std::string& entity_id,
    const std::string& fleet_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetReadinessState>();
    comp->fleet_id = fleet_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetReadinessSystem::addMember(const std::string& entity_id,
    const std::string& member_id, const std::string& ship_name,
    float dps, float ehp, float capacitor) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    // Duplicate check
    for (const auto& m : state->members) {
        if (m.member_id == member_id) return false;
    }
    if (static_cast<int>(state->members.size()) >= state->max_members) return false;
    FR::FleetMember mem;
    mem.member_id = member_id;
    mem.ship_name = ship_name;
    mem.dps = dps;
    mem.ehp = ehp;
    mem.capacitor = capacitor;
    mem.ready = false;
    state->members.push_back(mem);
    return true;
}

bool FleetReadinessSystem::removeMember(const std::string& entity_id,
    const std::string& member_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto it = state->members.begin(); it != state->members.end(); ++it) {
        if (it->member_id == member_id) {
            state->members.erase(it);
            return true;
        }
    }
    return false;
}

int FleetReadinessSystem::getMemberCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->members.size()) : 0;
}

bool FleetReadinessSystem::setMemberReady(const std::string& entity_id,
    const std::string& member_id, bool ready) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& m : state->members) {
        if (m.member_id == member_id) {
            m.ready = ready;
            return true;
        }
    }
    return false;
}

bool FleetReadinessSystem::isMemberReady(const std::string& entity_id,
    const std::string& member_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& m : state->members) {
        if (m.member_id == member_id) return m.ready;
    }
    return false;
}

int FleetReadinessSystem::getReadyCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& m : state->members) {
        if (m.ready) count++;
    }
    return count;
}

int FleetReadinessSystem::getNotReadyCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& m : state->members) {
        if (!m.ready) count++;
    }
    return count;
}

float FleetReadinessSystem::getFleetDPS(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float total = 0.0f;
    for (const auto& m : state->members) {
        total += m.dps;
    }
    return total;
}

float FleetReadinessSystem::getFleetEHP(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float total = 0.0f;
    for (const auto& m : state->members) {
        total += m.ehp;
    }
    return total;
}

float FleetReadinessSystem::getFleetCapacitor(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float total = 0.0f;
    for (const auto& m : state->members) {
        total += m.capacitor;
    }
    return total;
}

bool FleetReadinessSystem::isFleetReady(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->members.empty()) return false;
    for (const auto& m : state->members) {
        if (!m.ready) return false;
    }
    return true;
}

float FleetReadinessSystem::getReadinessPercentage(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    if (state->members.empty()) return 0.0f;
    int ready = 0;
    for (const auto& m : state->members) {
        if (m.ready) ready++;
    }
    return static_cast<float>(ready) / static_cast<float>(state->members.size()) * 100.0f;
}

bool FleetReadinessSystem::setSupplyLevel(const std::string& entity_id,
    const std::string& supply_type, float level) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& s : state->supplies) {
        if (s.supply_type == supply_type) {
            s.level = level;
            return true;
        }
    }
    if (static_cast<int>(state->supplies.size()) >= state->max_supplies) return false;
    FR::SupplyStatus ss;
    ss.supply_type = supply_type;
    ss.level = level;
    state->supplies.push_back(ss);
    return true;
}

float FleetReadinessSystem::getSupplyLevel(const std::string& entity_id,
    const std::string& supply_type) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    for (const auto& s : state->supplies) {
        if (s.supply_type == supply_type) return s.level;
    }
    return 0.0f;
}

bool FleetReadinessSystem::isSupplyAdequate(const std::string& entity_id, float threshold) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->supplies.empty()) return false;
    for (const auto& s : state->supplies) {
        if (s.level < threshold) return false;
    }
    return true;
}

} // namespace systems
} // namespace atlas
