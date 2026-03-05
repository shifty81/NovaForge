#include "systems/wing_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

WingManagementSystem::WingManagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void WingManagementSystem::updateComponent(ecs::Entity& /*entity*/,
    components::WingState& ws, float /*delta_time*/) {
    for (auto& wing : ws.wings) {
        if (wing.members.empty()) {
            wing.morale = 50.0f;
            continue;
        }

        float sum = 0.0f;
        int count = 0;
        for (const auto& member_id : wing.members) {
            auto* member = world_->getEntity(member_id);
            if (!member) continue;
            auto* morale_comp = member->getComponent<components::FleetMorale>();
            if (morale_comp) {
                // Map morale_score (-100..+100) to 0..100 range
                sum += (morale_comp->morale_score + 100.0f) / 2.0f;
            } else {
                sum += 50.0f;
            }
            count++;
        }
        wing.morale = (count > 0) ? std::clamp(sum / static_cast<float>(count), 0.0f, 100.0f) : 50.0f;
    }
}

bool WingManagementSystem::createWing(const std::string& fleet_id, const std::string& wing_id, const std::string& role) {
    if (role != "Mining" && role != "Combat" && role != "Logistics" &&
        role != "Salvage" && role != "Construction") {
        return false;
    }

    auto* entity = world_->getEntity(fleet_id);
    if (!entity) return false;

    auto* ws = entity->getComponent<components::WingState>();
    if (!ws) {
        entity->addComponent(std::make_unique<components::WingState>());
        ws = entity->getComponent<components::WingState>();
    }

    if (ws->getWing(wing_id)) return false;

    components::WingState::Wing wing;
    wing.wing_id = wing_id;
    wing.role = role;
    ws->wings.push_back(wing);
    return true;
}

bool WingManagementSystem::dissolveWing(const std::string& fleet_id, const std::string& wing_id) {
    auto* ws = getComponentFor(fleet_id);
    if (!ws) return false;

    auto it = std::find_if(ws->wings.begin(), ws->wings.end(),
        [&wing_id](const components::WingState::Wing& w) { return w.wing_id == wing_id; });

    if (it == ws->wings.end()) return false;

    ws->wings.erase(it);
    return true;
}

bool WingManagementSystem::assignToWing(const std::string& fleet_id, const std::string& wing_id, const std::string& member_id) {
    auto* ws = getComponentFor(fleet_id);
    if (!ws) return false;

    auto* wing = ws->getWing(wing_id);
    if (!wing) return false;

    if (static_cast<int>(wing->members.size()) >= max_members_per_wing) return false;

    // Avoid duplicate
    if (std::find(wing->members.begin(), wing->members.end(), member_id) != wing->members.end()) {
        return false;
    }

    wing->members.push_back(member_id);
    return true;
}

bool WingManagementSystem::removeFromWing(const std::string& fleet_id, const std::string& wing_id, const std::string& member_id) {
    auto* ws = getComponentFor(fleet_id);
    if (!ws) return false;

    auto* wing = ws->getWing(wing_id);
    if (!wing) return false;

    auto it = std::find(wing->members.begin(), wing->members.end(), member_id);
    if (it == wing->members.end()) return false;

    wing->members.erase(it);

    // Clear commander if removed
    if (wing->commander_id == member_id) {
        wing->commander_id.clear();
    }
    return true;
}

bool WingManagementSystem::setWingCommander(const std::string& fleet_id, const std::string& wing_id, const std::string& commander_id) {
    auto* ws = getComponentFor(fleet_id);
    if (!ws) return false;

    auto* wing = ws->getWing(wing_id);
    if (!wing) return false;

    // Auto-add commander as member if not already present
    if (std::find(wing->members.begin(), wing->members.end(), commander_id) == wing->members.end()) {
        if (static_cast<int>(wing->members.size()) >= max_members_per_wing) return false;
        wing->members.push_back(commander_id);
    }

    wing->commander_id = commander_id;
    return true;
}

std::string WingManagementSystem::getWingRole(const std::string& fleet_id, const std::string& wing_id) const {
    auto* ws = getComponentFor(fleet_id);
    if (!ws) return "";

    const auto* wing = ws->getWing(wing_id);
    return wing ? wing->role : "";
}

std::vector<std::string> WingManagementSystem::getWingMembers(const std::string& fleet_id, const std::string& wing_id) const {
    auto* ws = getComponentFor(fleet_id);
    if (!ws) return {};

    const auto* wing = ws->getWing(wing_id);
    return wing ? wing->members : std::vector<std::string>{};
}

std::string WingManagementSystem::getWingCommander(const std::string& fleet_id, const std::string& wing_id) const {
    auto* ws = getComponentFor(fleet_id);
    if (!ws) return "";

    const auto* wing = ws->getWing(wing_id);
    return wing ? wing->commander_id : "";
}

float WingManagementSystem::getWingMorale(const std::string& fleet_id, const std::string& wing_id) const {
    auto* ws = getComponentFor(fleet_id);
    if (!ws) return 50.0f;

    const auto* wing = ws->getWing(wing_id);
    return wing ? wing->morale : 50.0f;
}

int WingManagementSystem::getWingCount(const std::string& fleet_id) const {
    auto* ws = getComponentFor(fleet_id);
    if (!ws) return 0;

    return static_cast<int>(ws->wings.size());
}

} // namespace systems
} // namespace atlas
