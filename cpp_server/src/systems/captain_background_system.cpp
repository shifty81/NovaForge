#include "systems/captain_background_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CaptainBackgroundSystem::CaptainBackgroundSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void CaptainBackgroundSystem::updateComponent(ecs::Entity& /*entity*/, components::CaptainBackground& /*bg*/, float /*delta_time*/) {
    // Captain backgrounds are static data — no per-tick processing needed
}

void CaptainBackgroundSystem::applyBackgroundDefaults(components::CaptainBackground* bg) {
    switch (bg->background) {
        case components::CaptainBackground::BackgroundType::FormerMiner:
            bg->aggression_modifier = -0.1f;
            bg->loyalty_modifier = 0.1f;
            bg->professionalism_modifier = 0.0f;
            bg->preferred_role = "Mining";
            bg->dialogue_flavor = "gruff";
            bg->skill_bonus = 0.15f;
            bg->skill_category = "mining";
            break;
        case components::CaptainBackground::BackgroundType::ExMilitary:
            bg->aggression_modifier = 0.2f;
            bg->loyalty_modifier = 0.15f;
            bg->professionalism_modifier = 0.2f;
            bg->preferred_role = "Combat";
            bg->dialogue_flavor = "formal";
            bg->skill_bonus = 0.15f;
            bg->skill_category = "combat";
            break;
        case components::CaptainBackground::BackgroundType::Smuggler:
            bg->aggression_modifier = 0.1f;
            bg->loyalty_modifier = -0.15f;
            bg->professionalism_modifier = -0.1f;
            bg->preferred_role = "Scout";
            bg->dialogue_flavor = "sly";
            bg->skill_bonus = 0.15f;
            bg->skill_category = "exploration";
            break;
        case components::CaptainBackground::BackgroundType::Scientist:
            bg->aggression_modifier = -0.2f;
            bg->loyalty_modifier = 0.0f;
            bg->professionalism_modifier = 0.15f;
            bg->preferred_role = "Support";
            bg->dialogue_flavor = "analytical";
            bg->skill_bonus = 0.2f;
            bg->skill_category = "exploration";
            break;
        case components::CaptainBackground::BackgroundType::Noble:
            bg->aggression_modifier = 0.0f;
            bg->loyalty_modifier = 0.05f;
            bg->professionalism_modifier = 0.25f;
            bg->preferred_role = "Command";
            bg->dialogue_flavor = "eloquent";
            bg->skill_bonus = 0.1f;
            bg->skill_category = "social";
            break;
        case components::CaptainBackground::BackgroundType::Colonist:
            bg->aggression_modifier = -0.05f;
            bg->loyalty_modifier = 0.2f;
            bg->professionalism_modifier = -0.05f;
            bg->preferred_role = "Logistics";
            bg->dialogue_flavor = "practical";
            bg->skill_bonus = 0.15f;
            bg->skill_category = "industry";
            break;
        case components::CaptainBackground::BackgroundType::BountyHunter:
            bg->aggression_modifier = 0.25f;
            bg->loyalty_modifier = -0.1f;
            bg->professionalism_modifier = 0.1f;
            bg->preferred_role = "Combat";
            bg->dialogue_flavor = "terse";
            bg->skill_bonus = 0.15f;
            bg->skill_category = "combat";
            break;
        case components::CaptainBackground::BackgroundType::Trader:
            bg->aggression_modifier = -0.1f;
            bg->loyalty_modifier = 0.0f;
            bg->professionalism_modifier = 0.1f;
            bg->preferred_role = "Logistics";
            bg->dialogue_flavor = "smooth";
            bg->skill_bonus = 0.15f;
            bg->skill_category = "industry";
            break;
    }
}

bool CaptainBackgroundSystem::assignBackground(const std::string& entity_id,
                                                const std::string& background_type) {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return false;

    bg->background = components::CaptainBackground::stringToType(background_type);
    applyBackgroundDefaults(bg);
    return true;
}

bool CaptainBackgroundSystem::generateBackground(const std::string& entity_id, uint32_t seed) {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return false;

    // Deterministic background selection from seed
    int type_index = seed % 8;
    bg->background = static_cast<components::CaptainBackground::BackgroundType>(type_index);
    applyBackgroundDefaults(bg);

    // Deterministic experience from seed
    bg->years_experience = static_cast<int>((seed >> 8) % 30) + 1;  // 1-30 years

    // Deterministic origin system
    static const char* systems[] = {
        "Solari Prime", "Veyren Station", "Aurelian Reach", "Keldari Belt",
        "Frontier-7", "Outer Rim", "Core Worlds", "Drift Nebula"
    };
    bg->origin_system = systems[(seed >> 16) % 8];

    return true;
}

bool CaptainBackgroundSystem::setOriginSystem(const std::string& entity_id,
                                               const std::string& system_name) {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return false;

    bg->origin_system = system_name;
    return true;
}

bool CaptainBackgroundSystem::setExperience(const std::string& entity_id, int years) {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return false;

    bg->years_experience = std::max(0, years);
    return true;
}

std::string CaptainBackgroundSystem::getBackground(const std::string& entity_id) const {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return "Unknown";

    return components::CaptainBackground::typeToString(bg->background);
}

std::string CaptainBackgroundSystem::getPreferredRole(const std::string& entity_id) const {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return "";

    return bg->preferred_role;
}

std::string CaptainBackgroundSystem::getDialogueFlavor(const std::string& entity_id) const {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return "";

    return bg->dialogue_flavor;
}

float CaptainBackgroundSystem::getSkillBonus(const std::string& entity_id) const {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return 0.0f;

    return bg->skill_bonus;
}

std::string CaptainBackgroundSystem::getSkillCategory(const std::string& entity_id) const {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return "";

    return bg->skill_category;
}

float CaptainBackgroundSystem::getAggressionModifier(const std::string& entity_id) const {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return 0.0f;

    return bg->aggression_modifier;
}

float CaptainBackgroundSystem::getLoyaltyModifier(const std::string& entity_id) const {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return 0.0f;

    return bg->loyalty_modifier;
}

int CaptainBackgroundSystem::getExperience(const std::string& entity_id) const {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return 0;

    return bg->years_experience;
}

std::string CaptainBackgroundSystem::getOriginSystem(const std::string& entity_id) const {
    auto* bg = getComponentFor(entity_id);
    if (!bg) return "";

    return bg->origin_system;
}

} // namespace systems
} // namespace atlas
