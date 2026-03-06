#include "systems/star_system_manager_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using SSS = components::StarSystemState;
}

StarSystemManagerSystem::StarSystemManagerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void StarSystemManagerSystem::updateComponent(ecs::Entity& entity,
    components::StarSystemState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool StarSystemManagerSystem::initialize(const std::string& entity_id,
    const std::string& system_name, float security) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::StarSystemState>();
    comp->system_id = entity_id;
    comp->system_name = system_name;
    comp->security_status = std::max(0.0f, std::min(1.0f, security));
    entity->addComponent(std::move(comp));
    return true;
}

bool StarSystemManagerSystem::addCelestial(const std::string& entity_id,
    const std::string& body_id, const std::string& name, const std::string& type,
    float x, float y, float z, float radius) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->celestials.size()) >= state->max_celestials) return false;
    for (const auto& c : state->celestials) {
        if (c.body_id == body_id) return false;
    }
    SSS::CelestialBody body;
    body.body_id = body_id;
    body.name = name;
    body.type = type;
    body.x = x;
    body.y = y;
    body.z = z;
    body.radius = radius;
    state->celestials.push_back(body);
    return true;
}

bool StarSystemManagerSystem::removeCelestial(const std::string& entity_id,
    const std::string& body_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->celestials.begin(), state->celestials.end(),
        [&](const SSS::CelestialBody& c) { return c.body_id == body_id; });
    if (it == state->celestials.end()) return false;
    state->celestials.erase(it);
    return true;
}

int StarSystemManagerSystem::getCelestialCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->celestials.size()) : 0;
}

bool StarSystemManagerSystem::addStation(const std::string& entity_id,
    const std::string& station_id, const std::string& name, const std::string& faction,
    float x, float y, float z) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->stations.size()) >= state->max_stations) return false;
    for (const auto& s : state->stations) {
        if (s.station_id == station_id) return false;
    }
    SSS::StationInfo info;
    info.station_id = station_id;
    info.name = name;
    info.owner_faction = faction;
    info.x = x;
    info.y = y;
    info.z = z;
    state->stations.push_back(info);
    return true;
}

bool StarSystemManagerSystem::removeStation(const std::string& entity_id,
    const std::string& station_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->stations.begin(), state->stations.end(),
        [&](const SSS::StationInfo& s) { return s.station_id == station_id; });
    if (it == state->stations.end()) return false;
    state->stations.erase(it);
    return true;
}

bool StarSystemManagerSystem::dockAtStation(const std::string& entity_id,
    const std::string& station_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& s : state->stations) {
        if (s.station_id == station_id) {
            if (!s.online) return false;
            if (s.docked_count >= s.max_docking) return false;
            s.docked_count++;
            state->total_dockings++;
            return true;
        }
    }
    return false;
}

bool StarSystemManagerSystem::undockFromStation(const std::string& entity_id,
    const std::string& station_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& s : state->stations) {
        if (s.station_id == station_id) {
            if (s.docked_count <= 0) return false;
            s.docked_count--;
            return true;
        }
    }
    return false;
}

int StarSystemManagerSystem::getStationDockedCount(const std::string& entity_id,
    const std::string& station_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    for (const auto& s : state->stations) {
        if (s.station_id == station_id) return s.docked_count;
    }
    return 0;
}

int StarSystemManagerSystem::getStationCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->stations.size()) : 0;
}

bool StarSystemManagerSystem::addGate(const std::string& entity_id,
    const std::string& gate_id, const std::string& destination,
    float x, float y, float z) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->gates.size()) >= state->max_gates) return false;
    for (const auto& g : state->gates) {
        if (g.gate_id == gate_id) return false;
    }
    SSS::GateLink link;
    link.gate_id = gate_id;
    link.destination_system = destination;
    link.x = x;
    link.y = y;
    link.z = z;
    state->gates.push_back(link);
    return true;
}

bool StarSystemManagerSystem::useGate(const std::string& entity_id,
    const std::string& gate_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& g : state->gates) {
        if (g.gate_id == gate_id) {
            if (!g.online) return false;
            g.total_jumps++;
            state->total_jumps++;
            return true;
        }
    }
    return false;
}

int StarSystemManagerSystem::getGateCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->gates.size()) : 0;
}

int StarSystemManagerSystem::getTotalJumps(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_jumps : 0;
}

bool StarSystemManagerSystem::addNPCPresence(const std::string& entity_id,
    const std::string& faction, int ship_count, float threat_level, bool hostile) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->npc_presence.size()) >= state->max_npc_factions) return false;
    for (const auto& n : state->npc_presence) {
        if (n.faction == faction) return false;
    }
    SSS::NPCPresence presence;
    presence.faction = faction;
    presence.ship_count = ship_count;
    presence.threat_level = std::max(0.0f, std::min(1.0f, threat_level));
    presence.hostile = hostile;
    state->npc_presence.push_back(presence);
    state->total_npc_spawns += ship_count;
    return true;
}

bool StarSystemManagerSystem::removeNPCPresence(const std::string& entity_id,
    const std::string& faction) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->npc_presence.begin(), state->npc_presence.end(),
        [&](const SSS::NPCPresence& n) { return n.faction == faction; });
    if (it == state->npc_presence.end()) return false;
    state->npc_presence.erase(it);
    return true;
}

int StarSystemManagerSystem::getNPCFactionCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->npc_presence.size()) : 0;
}

int StarSystemManagerSystem::getTotalNPCShips(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int total = 0;
    for (const auto& n : state->npc_presence) {
        total += n.ship_count;
    }
    return total;
}

float StarSystemManagerSystem::getSecurityStatus(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->security_status : 0.0f;
}

int StarSystemManagerSystem::getTotalDockings(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_dockings : 0;
}

} // namespace systems
} // namespace atlas
