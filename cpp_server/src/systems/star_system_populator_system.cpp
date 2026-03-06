#include "systems/star_system_populator_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using SSP = components::StarSystemPopulation;
}

StarSystemPopulatorSystem::StarSystemPopulatorSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void StarSystemPopulatorSystem::updateComponent(ecs::Entity& entity,
    components::StarSystemPopulation& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool StarSystemPopulatorSystem::initialize(const std::string& entity_id,
    const std::string& system_id, const std::string& system_name, float security) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::StarSystemPopulation>();
    comp->system_id = system_id;
    comp->system_name = system_name;
    comp->security_status = security;
    entity->addComponent(std::move(comp));
    return true;
}

bool StarSystemPopulatorSystem::addStation(const std::string& entity_id,
    const std::string& station_id, const std::string& station_name,
    const std::string& station_type) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->stations.size()) >= state->max_stations) return false;
    for (const auto& s : state->stations) {
        if (s.station_id == station_id) return false;
    }
    SSP::StationSeed seed;
    seed.station_id = station_id;
    seed.station_name = station_name;
    seed.station_type = station_type;
    state->stations.push_back(seed);
    return true;
}

int StarSystemPopulatorSystem::getStationCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->stations.size()) : 0;
}

bool StarSystemPopulatorSystem::hasStation(const std::string& entity_id,
    const std::string& station_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& s : state->stations) {
        if (s.station_id == station_id) return true;
    }
    return false;
}

bool StarSystemPopulatorSystem::addAsteroidBelt(const std::string& entity_id,
    const std::string& belt_id, const std::string& ore_type, int richness) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->asteroid_belts.size()) >= state->max_asteroid_belts) return false;
    for (const auto& b : state->asteroid_belts) {
        if (b.belt_id == belt_id) return false;
    }
    SSP::AsteroidBeltSeed seed;
    seed.belt_id = belt_id;
    seed.ore_type = ore_type;
    seed.richness = richness;
    state->asteroid_belts.push_back(seed);
    return true;
}

int StarSystemPopulatorSystem::getAsteroidBeltCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->asteroid_belts.size()) : 0;
}

bool StarSystemPopulatorSystem::hasAsteroidBelt(const std::string& entity_id,
    const std::string& belt_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& b : state->asteroid_belts) {
        if (b.belt_id == belt_id) return true;
    }
    return false;
}

bool StarSystemPopulatorSystem::addNPCFaction(const std::string& entity_id,
    const std::string& faction_id, const std::string& faction_name, int spawn_count) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->npc_factions.size()) >= state->max_npc_factions) return false;
    for (const auto& f : state->npc_factions) {
        if (f.faction_id == faction_id) return false;
    }
    SSP::NPCFactionSeed seed;
    seed.faction_id = faction_id;
    seed.faction_name = faction_name;
    seed.spawn_count = spawn_count;
    state->npc_factions.push_back(seed);
    return true;
}

int StarSystemPopulatorSystem::getNPCFactionCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->npc_factions.size()) : 0;
}

bool StarSystemPopulatorSystem::hasNPCFaction(const std::string& entity_id,
    const std::string& faction_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& f : state->npc_factions) {
        if (f.faction_id == faction_id) return true;
    }
    return false;
}

bool StarSystemPopulatorSystem::addJumpGate(const std::string& entity_id,
    const std::string& gate_id, const std::string& destination_system) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->jump_gates.size()) >= state->max_jump_gates) return false;
    for (const auto& g : state->jump_gates) {
        if (g.gate_id == gate_id) return false;
    }
    SSP::JumpGateSeed seed;
    seed.gate_id = gate_id;
    seed.destination_system = destination_system;
    state->jump_gates.push_back(seed);
    return true;
}

int StarSystemPopulatorSystem::getJumpGateCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->jump_gates.size()) : 0;
}

bool StarSystemPopulatorSystem::hasJumpGate(const std::string& entity_id,
    const std::string& gate_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& g : state->jump_gates) {
        if (g.gate_id == gate_id) return true;
    }
    return false;
}

bool StarSystemPopulatorSystem::addPointOfInterest(const std::string& entity_id,
    const std::string& poi_id, const std::string& poi_type,
    const std::string& description) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->points_of_interest.size()) >= state->max_pois) return false;
    for (const auto& p : state->points_of_interest) {
        if (p.poi_id == poi_id) return false;
    }
    SSP::PointOfInterestSeed seed;
    seed.poi_id = poi_id;
    seed.poi_type = poi_type;
    seed.description = description;
    state->points_of_interest.push_back(seed);
    return true;
}

int StarSystemPopulatorSystem::getPointOfInterestCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->points_of_interest.size()) : 0;
}

bool StarSystemPopulatorSystem::hasPointOfInterest(const std::string& entity_id,
    const std::string& poi_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& p : state->points_of_interest) {
        if (p.poi_id == poi_id) return true;
    }
    return false;
}

bool StarSystemPopulatorSystem::isPopulated(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->populated : false;
}

bool StarSystemPopulatorSystem::markPopulated(const std::string& entity_id, float timestamp) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->populated) return false;
    state->populated = true;
    state->population_time = timestamp;
    return true;
}

float StarSystemPopulatorSystem::getPopulationTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->population_time : 0.0f;
}

} // namespace systems
} // namespace atlas
