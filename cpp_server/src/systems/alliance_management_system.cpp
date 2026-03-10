#include "systems/alliance_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AllianceManagementSystem::AllianceManagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void AllianceManagementSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::AllianceState& comp,
        float delta_time) {
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool AllianceManagementSystem::initialize(
        const std::string& entity_id,
        const std::string& alliance_id,
        const std::string& alliance_name,
        const std::string& ticker,
        const std::string& executor_corp_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (alliance_id.empty() || alliance_name.empty() || ticker.empty() ||
        executor_corp_id.empty())
        return false;

    auto comp = std::make_unique<components::AllianceState>();
    comp->alliance_id      = alliance_id;
    comp->alliance_name    = alliance_name;
    comp->ticker           = ticker;
    comp->executor_corp_id = executor_corp_id;
    comp->state            = components::AllianceState::State::Active;

    // Auto-add executor as first member
    components::AllianceState::AllianceMember exec;
    exec.corp_id     = executor_corp_id;
    exec.corp_name   = executor_corp_id; // name defaults to ID
    exec.is_executor = true;
    comp->members.push_back(exec);
    comp->total_members_joined = 1;

    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Membership
// ---------------------------------------------------------------------------

bool AllianceManagementSystem::addMember(const std::string& entity_id,
                                          const std::string& corp_id,
                                          const std::string& corp_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::AllianceState::State::Active) return false;
    if (corp_id.empty() || corp_name.empty()) return false;
    if (static_cast<int>(comp->members.size()) >= comp->max_members) return false;

    // Duplicate prevention
    for (const auto& m : comp->members) {
        if (m.corp_id == corp_id) return false;
    }

    components::AllianceState::AllianceMember member;
    member.corp_id   = corp_id;
    member.corp_name = corp_name;
    comp->members.push_back(member);
    comp->total_members_joined++;
    return true;
}

bool AllianceManagementSystem::removeMember(const std::string& entity_id,
                                             const std::string& corp_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::AllianceState::State::Active) return false;

    // Cannot remove executor
    if (corp_id == comp->executor_corp_id) return false;

    auto it = std::find_if(comp->members.begin(), comp->members.end(),
        [&](const components::AllianceState::AllianceMember& m) {
            return m.corp_id == corp_id;
        });
    if (it == comp->members.end()) return false;
    comp->members.erase(it);
    return true;
}

bool AllianceManagementSystem::setExecutor(const std::string& entity_id,
                                            const std::string& corp_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::AllianceState::State::Active) return false;

    // Must be an existing member
    bool found = false;
    for (auto& m : comp->members) {
        if (m.corp_id == corp_id) {
            found = true;
        }
    }
    if (!found) return false;

    // Update executor flags
    for (auto& m : comp->members) {
        m.is_executor = (m.corp_id == corp_id);
    }
    comp->executor_corp_id = corp_id;
    return true;
}

bool AllianceManagementSystem::disbandAlliance(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::AllianceState::State::Active) return false;

    comp->state = components::AllianceState::State::Disbanded;
    comp->members.clear();
    comp->executor_corp_id.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int AllianceManagementSystem::getMemberCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->members.size()) : 0;
}

std::string AllianceManagementSystem::getExecutorCorpId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->executor_corp_id : std::string();
}

std::string AllianceManagementSystem::getAllianceName(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->alliance_name : std::string();
}

std::string AllianceManagementSystem::getTicker(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->ticker : std::string();
}

bool AllianceManagementSystem::isActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? (comp->state == components::AllianceState::State::Active) : false;
}

bool AllianceManagementSystem::hasMember(const std::string& entity_id,
                                          const std::string& corp_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->members) {
        if (m.corp_id == corp_id) return true;
    }
    return false;
}

int AllianceManagementSystem::getTotalMembersJoined(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_members_joined : 0;
}

} // namespace systems
} // namespace atlas
