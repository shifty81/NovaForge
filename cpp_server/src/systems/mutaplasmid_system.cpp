#include "systems/mutaplasmid_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <numeric>

namespace atlas {
namespace systems {

MutaplasmidSystem::MutaplasmidSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MutaplasmidSystem::updateComponent(ecs::Entity& /*entity*/,
    components::MutaplasmidState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    // Mutation is applied via explicit API; nothing to tick here.
}

// ---------------------------------------------------------------------------
// Static helper
// ---------------------------------------------------------------------------

float MutaplasmidSystem::computeQuality(
    const std::vector<components::MutaplasmidState::StatRoll>& rolls) {
    if (rolls.empty()) return 0.0f;
    float total = 0.0f;
    int counted = 0;
    for (const auto& r : rolls) {
        float range = r.max_multiplier - r.min_multiplier;
        if (range > 0.0f) {
            total += (r.rolled_value - r.min_multiplier) / range;
            counted++;
        }
    }
    return counted > 0 ? total / static_cast<float>(counted) : 0.0f;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool MutaplasmidSystem::initialize(const std::string& entity_id,
    const std::string& facility_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MutaplasmidState>();
    comp->facility_id = facility_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool MutaplasmidSystem::queueMutation(const std::string& entity_id,
    const std::string& module_id,
    components::MutaplasmidState::Grade grade,
    const std::vector<components::MutaplasmidState::StatRoll>& stats) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->mutations.size()) >= comp->max_pending) return false;

    // Reject duplicate module_id
    for (const auto& m : comp->mutations) {
        if (m.original_module_id == module_id) return false;
    }

    components::MutaplasmidState::MutatedModule mm;
    mm.original_module_id = module_id;
    mm.mutated_module_id  = module_id + "_mutated";
    mm.grade = grade;
    mm.stat_rolls = stats;
    comp->mutations.push_back(mm);
    comp->total_attempted++;
    return true;
}

bool MutaplasmidSystem::applyRoll(const std::string& entity_id,
    const std::string& module_id, int stat_index, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& mm : comp->mutations) {
        if (mm.original_module_id == module_id && !mm.created) {
            if (stat_index < 0 ||
                stat_index >= static_cast<int>(mm.stat_rolls.size()))
                return false;
            auto& r = mm.stat_rolls[stat_index];
            r.rolled_value = (std::max)(r.min_multiplier,
                             (std::min)(r.max_multiplier, value));
            r.rolled = true;
            return true;
        }
    }
    return false;
}

bool MutaplasmidSystem::finalizeMutation(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& mm : comp->mutations) {
        if (mm.original_module_id == module_id && !mm.created) {
            // Ensure all stats have been rolled
            for (const auto& r : mm.stat_rolls) {
                if (!r.rolled) return false;
            }
            mm.overall_quality = computeQuality(mm.stat_rolls);
            mm.created = true;
            comp->total_created++;
            return true;
        }
    }
    return false;
}

int MutaplasmidSystem::getMutationCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->mutations.size()) : 0;
}

int MutaplasmidSystem::getTotalAttempted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_attempted : 0;
}

int MutaplasmidSystem::getTotalCreated(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_created : 0;
}

float MutaplasmidSystem::getOverallQuality(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& mm : comp->mutations) {
        if (mm.original_module_id == module_id) return mm.overall_quality;
    }
    return 0.0f;
}

bool MutaplasmidSystem::isMutationCreated(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& mm : comp->mutations) {
        if (mm.original_module_id == module_id) return mm.created;
    }
    return false;
}

} // namespace systems
} // namespace atlas
