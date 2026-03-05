#include "systems/procedural_mission_generator_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/mission_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {

using GM = components::ProceduralMissionGenerator::GeneratedMission;

GM* findMission(components::ProceduralMissionGenerator* gen, const std::string& mission_id) {
    for (auto& m : gen->available_missions) {
        if (m.mission_id == mission_id) return &m;
    }
    return nullptr;
}

const GM* findMissionConst(const components::ProceduralMissionGenerator* gen,
                           const std::string& mission_id) {
    for (const auto& m : gen->available_missions) {
        if (m.mission_id == mission_id) return &m;
    }
    return nullptr;
}

float calculateReward(int difficulty, float multiplier) {
    // Base 10000 + 15000 per difficulty level, scaled by multiplier
    return (10000.0f + 15000.0f * static_cast<float>(difficulty - 1)) * multiplier;
}

float calculateStanding(int difficulty) {
    return 0.05f + 0.05f * static_cast<float>(difficulty);
}

int calculateObjectives(int difficulty) {
    return std::min(difficulty, 5);
}

float calculateTimeLimit(int difficulty) {
    // Higher difficulty = more time (but also harder)
    return 1800.0f + 600.0f * static_cast<float>(difficulty);
}

} // anonymous namespace

ProceduralMissionGeneratorSystem::ProceduralMissionGeneratorSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ProceduralMissionGeneratorSystem::updateComponent(ecs::Entity& /*entity*/, components::ProceduralMissionGenerator& gen, float delta_time) {
    if (!gen.active) return;

    // Countdown generation cooldown
    if (gen.generation_cooldown > 0.0f) {
        gen.generation_cooldown -= delta_time;
        if (gen.generation_cooldown < 0.0f) gen.generation_cooldown = 0.0f;
    }

    // Check for expired accepted missions with time limits
    for (auto& m : gen.available_missions) {
        if (m.accepted && !m.completed && !m.expired && m.time_limit > 0.0f) {
            m.time_limit -= delta_time;
            if (m.time_limit <= 0.0f) {
                m.time_limit = 0.0f;
                m.expired = true;
                gen.total_expired++;
            }
        }
    }
}

bool ProceduralMissionGeneratorSystem::initialize(const std::string& entity_id,
    const std::string& generator_id, const std::string& faction_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ProceduralMissionGenerator>();
    comp->generator_id = generator_id;
    comp->faction_id = faction_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool ProceduralMissionGeneratorSystem::generateMission(const std::string& entity_id,
    const std::string& mission_id, const std::string& type, int difficulty,
    const std::string& target_system) {
    auto* gen = getComponentFor(entity_id);
    if (!gen) return false;
    if (static_cast<int>(gen->available_missions.size()) >= gen->max_available) return false;

    // Check duplicate
    if (findMission(gen, mission_id)) return false;

    // Clamp difficulty
    int eff_diff = std::max(1, std::min(5, difficulty + gen->difficulty_bias));

    components::ProceduralMissionGenerator::GeneratedMission m;
    m.mission_id = mission_id;
    m.title = type + " Mission L" + std::to_string(eff_diff);
    m.type = type;
    m.difficulty = eff_diff;
    m.reward_credits = calculateReward(eff_diff, gen->reward_multiplier);
    m.reward_standing = calculateStanding(eff_diff);
    m.objective_count = calculateObjectives(eff_diff);
    m.time_limit = calculateTimeLimit(eff_diff);
    m.target_system = target_system;
    gen->available_missions.push_back(m);
    gen->total_generated++;
    return true;
}

bool ProceduralMissionGeneratorSystem::acceptMission(const std::string& entity_id,
    const std::string& mission_id) {
    auto* gen = getComponentFor(entity_id);
    if (!gen) return false;

    auto* m = findMission(gen, mission_id);
    if (!m || m->accepted || m->expired) return false;
    m->accepted = true;
    return true;
}

bool ProceduralMissionGeneratorSystem::completeMission(const std::string& entity_id,
    const std::string& mission_id) {
    auto* gen = getComponentFor(entity_id);
    if (!gen) return false;

    auto* m = findMission(gen, mission_id);
    if (!m || !m->accepted || m->completed || m->expired) return false;
    m->completed = true;
    gen->total_completed++;

    // Move to completed list
    gen->completed_missions.push_back(*m);
    return true;
}

bool ProceduralMissionGeneratorSystem::expireMission(const std::string& entity_id,
    const std::string& mission_id) {
    auto* gen = getComponentFor(entity_id);
    if (!gen) return false;

    auto* m = findMission(gen, mission_id);
    if (!m || m->expired || m->completed) return false;
    m->expired = true;
    gen->total_expired++;
    return true;
}

bool ProceduralMissionGeneratorSystem::removeMission(const std::string& entity_id,
    const std::string& mission_id) {
    auto* gen = getComponentFor(entity_id);
    if (!gen) return false;

    auto it = std::remove_if(gen->available_missions.begin(), gen->available_missions.end(),
        [&](const components::ProceduralMissionGenerator::GeneratedMission& m) {
            return m.mission_id == mission_id;
        });
    if (it == gen->available_missions.end()) return false;
    gen->available_missions.erase(it, gen->available_missions.end());
    return true;
}

int ProceduralMissionGeneratorSystem::getAvailableCount(const std::string& entity_id) const {
    const auto* gen = getComponentFor(entity_id);
    return gen ? static_cast<int>(gen->available_missions.size()) : 0;
}

int ProceduralMissionGeneratorSystem::getCompletedCount(const std::string& entity_id) const {
    const auto* gen = getComponentFor(entity_id);
    return gen ? gen->total_completed : 0;
}

float ProceduralMissionGeneratorSystem::getMissionReward(const std::string& entity_id,
    const std::string& mission_id) const {
    const auto* gen = getComponentFor(entity_id);
    if (!gen) return 0.0f;
    auto* m = findMissionConst(gen, mission_id);
    return m ? m->reward_credits : 0.0f;
}

int ProceduralMissionGeneratorSystem::getMissionDifficulty(const std::string& entity_id,
    const std::string& mission_id) const {
    const auto* gen = getComponentFor(entity_id);
    if (!gen) return 0;
    auto* m = findMissionConst(gen, mission_id);
    return m ? m->difficulty : 0;
}

bool ProceduralMissionGeneratorSystem::isMissionAccepted(const std::string& entity_id,
    const std::string& mission_id) const {
    const auto* gen = getComponentFor(entity_id);
    if (!gen) return false;
    auto* m = findMissionConst(gen, mission_id);
    return m ? m->accepted : false;
}

int ProceduralMissionGeneratorSystem::getTotalGenerated(const std::string& entity_id) const {
    const auto* gen = getComponentFor(entity_id);
    return gen ? gen->total_generated : 0;
}

} // namespace systems
} // namespace atlas
