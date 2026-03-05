#include "systems/player_progression_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

PlayerProgressionSystem::PlayerProgressionSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

float PlayerProgressionSystem::xpForLevel(int level) {
    // XP curve: 100 × level^1.5 (accelerating difficulty)
    return 100.0f * std::pow(static_cast<float>(level), 1.5f);
}

void PlayerProgressionSystem::updateComponent(ecs::Entity& /*entity*/,
                                               components::PlayerProgression& prog,
                                               float /*delta_time*/) {
    // Recalculate total XP
    prog.total_xp = prog.combat_xp + prog.mining_xp + prog.exploration_xp
                    + prog.industry_xp + prog.social_xp;

    // Level calculation: accumulate XP thresholds
    float xp_accumulated = 0.0f;
    int new_level = 1;
    while (xp_accumulated + xpForLevel(new_level) <= prog.total_xp) {
        xp_accumulated += xpForLevel(new_level);
        new_level++;
        if (new_level > 100) break; // cap at level 100
    }
    prog.level = new_level;

    // Progress to next level
    float xp_needed = xpForLevel(new_level);
    float xp_into_level = prog.total_xp - xp_accumulated;
    prog.xp_to_next_level = xp_needed;
    prog.level_progress = (xp_needed > 0.0f)
        ? std::clamp(xp_into_level / xp_needed, 0.0f, 1.0f)
        : 0.0f;

    // Check milestones
    int achieved = 0;
    for (auto& milestone : prog.milestones) {
        if (!milestone.achieved) {
            float cat_xp = 0.0f;
            if (milestone.category == "combat") cat_xp = prog.combat_xp;
            else if (milestone.category == "mining") cat_xp = prog.mining_xp;
            else if (milestone.category == "exploration") cat_xp = prog.exploration_xp;
            else if (milestone.category == "industry") cat_xp = prog.industry_xp;
            else if (milestone.category == "social") cat_xp = prog.social_xp;
            else cat_xp = prog.total_xp; // "total" or unknown category uses total

            if (cat_xp >= milestone.xp_required) {
                milestone.achieved = true;
            }
        }
        if (milestone.achieved) achieved++;
    }
    prog.milestones_achieved = achieved;
}

bool PlayerProgressionSystem::awardXP(const std::string& player_id, const std::string& category, float amount) {
    auto* prog = getComponentFor(player_id);
    if (!prog) return false;

    float scaled_amount = amount * prog->prestige_multiplier;

    if (category == "combat") prog->combat_xp += scaled_amount;
    else if (category == "mining") prog->mining_xp += scaled_amount;
    else if (category == "exploration") prog->exploration_xp += scaled_amount;
    else if (category == "industry") prog->industry_xp += scaled_amount;
    else if (category == "social") prog->social_xp += scaled_amount;
    else return false; // unknown category

    return true;
}

bool PlayerProgressionSystem::initProgression(const std::string& player_id) {
    auto* entity = world_->getEntity(player_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::PlayerProgression>();
    if (existing) return false; // already initialized

    entity->addComponent(std::make_unique<components::PlayerProgression>());
    return true;
}

bool PlayerProgressionSystem::addMilestone(const std::string& player_id, const std::string& name,
                                            const std::string& category, float xp_required) {
    auto* prog = getComponentFor(player_id);
    if (!prog) return false;

    components::PlayerProgression::Milestone milestone;
    milestone.name = name;
    milestone.category = category;
    milestone.xp_required = xp_required;
    milestone.achieved = false;
    prog->milestones.push_back(milestone);
    return true;
}

bool PlayerProgressionSystem::prestige(const std::string& player_id) {
    auto* prog = getComponentFor(player_id);
    if (!prog) return false;

    // Must be at least level 50 to prestige
    if (prog->level < 50) return false;

    // Reset XP but increment prestige
    prog->prestige_level++;
    prog->prestige_multiplier = 1.0f + (prog->prestige_level * 0.1f); // +10% per prestige
    prog->combat_xp = 0.0f;
    prog->mining_xp = 0.0f;
    prog->exploration_xp = 0.0f;
    prog->industry_xp = 0.0f;
    prog->social_xp = 0.0f;
    prog->total_xp = 0.0f;
    prog->level = 1;

    // Reset milestones
    for (auto& m : prog->milestones) {
        m.achieved = false;
    }
    prog->milestones_achieved = 0;

    return true;
}

int PlayerProgressionSystem::getLevel(const std::string& player_id) const {
    const auto* prog = getComponentFor(player_id);
    if (!prog) return 0;

    return prog->level;
}

float PlayerProgressionSystem::getTotalXP(const std::string& player_id) const {
    const auto* prog = getComponentFor(player_id);
    if (!prog) return 0.0f;

    return prog->total_xp;
}

float PlayerProgressionSystem::getCategoryXP(const std::string& player_id, const std::string& category) const {
    const auto* prog = getComponentFor(player_id);
    if (!prog) return 0.0f;

    if (category == "combat") return prog->combat_xp;
    if (category == "mining") return prog->mining_xp;
    if (category == "exploration") return prog->exploration_xp;
    if (category == "industry") return prog->industry_xp;
    if (category == "social") return prog->social_xp;
    return 0.0f;
}

float PlayerProgressionSystem::getLevelProgress(const std::string& player_id) const {
    const auto* prog = getComponentFor(player_id);
    if (!prog) return 0.0f;

    return prog->level_progress;
}

int PlayerProgressionSystem::getMilestonesAchieved(const std::string& player_id) const {
    const auto* prog = getComponentFor(player_id);
    if (!prog) return 0;

    return prog->milestones_achieved;
}

int PlayerProgressionSystem::getPrestigeLevel(const std::string& player_id) const {
    const auto* prog = getComponentFor(player_id);
    if (!prog) return 0;

    return prog->prestige_level;
}

float PlayerProgressionSystem::getPrestigeMultiplier(const std::string& player_id) const {
    const auto* prog = getComponentFor(player_id);
    if (!prog) return 1.0f;

    return prog->prestige_multiplier;
}

} // namespace systems
} // namespace atlas
