#include "systems/ship_capability_rating_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ShipCapabilityRatingSystem::ShipCapabilityRatingSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

namespace {
// Convert a raw value to 0-5 star rating using thresholds
float toStars(float value, float one_star, float five_stars) {
    if (value <= 0.0f) return 0.0f;
    if (value >= five_stars) return 5.0f;
    return std::min(5.0f, std::max(0.0f, (value / five_stars) * 5.0f));
}
} // anonymous namespace

void ShipCapabilityRatingSystem::updateComponent(ecs::Entity& /*entity*/, components::ShipCapabilityRating& scr, float /*delta_time*/) {
    if (!scr.active || !scr.needs_recalculation) return;

    // Combat: based on weapon count (0-8 weapons = 0-5 stars)
    scr.combat_score = toStars(static_cast<float>(scr.weapon_count), 1.0f, 8.0f);

    // Mining: based on mining modules (0-5 modules = 0-5 stars)
    scr.mining_score = toStars(static_cast<float>(scr.mining_module_count), 1.0f, 5.0f);

    // Exploration: based on scanners (0-4 scanners = 0-5 stars)
    scr.exploration_score = toStars(static_cast<float>(scr.scanner_count), 1.0f, 4.0f);

    // Cargo: based on capacity (0-50000 m3 = 0-5 stars)
    scr.cargo_score = toStars(scr.cargo_capacity, 1000.0f, 50000.0f);

    // Defense: based on effective HP (0-100000 EHP = 0-5 stars)
    scr.defense_score = toStars(scr.total_ehp, 5000.0f, 100000.0f);

    // Fabrication: based on industry modules (0-5 modules = 0-5 stars)
    scr.fabrication_score = toStars(static_cast<float>(scr.industry_module_count), 1.0f, 5.0f);

    // Overall = average of all 6
    scr.overall_rating = (scr.combat_score + scr.mining_score + scr.exploration_score
                         + scr.cargo_score + scr.defense_score + scr.fabrication_score) / 6.0f;

    scr.needs_recalculation = false;
}

bool ShipCapabilityRatingSystem::initializeRating(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ShipCapabilityRating>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ShipCapabilityRatingSystem::setWeaponCount(const std::string& entity_id, int count) {
    auto* scr = getComponentFor(entity_id);
    if (!scr) return false;
    scr->weapon_count = std::max(0, count);
    scr->needs_recalculation = true;
    return true;
}

bool ShipCapabilityRatingSystem::setMiningModuleCount(const std::string& entity_id, int count) {
    auto* scr = getComponentFor(entity_id);
    if (!scr) return false;
    scr->mining_module_count = std::max(0, count);
    scr->needs_recalculation = true;
    return true;
}

bool ShipCapabilityRatingSystem::setScannerCount(const std::string& entity_id, int count) {
    auto* scr = getComponentFor(entity_id);
    if (!scr) return false;
    scr->scanner_count = std::max(0, count);
    scr->needs_recalculation = true;
    return true;
}

bool ShipCapabilityRatingSystem::setCargoCapacity(const std::string& entity_id, float capacity_m3) {
    auto* scr = getComponentFor(entity_id);
    if (!scr) return false;
    scr->cargo_capacity = std::max(0.0f, capacity_m3);
    scr->needs_recalculation = true;
    return true;
}

bool ShipCapabilityRatingSystem::setTotalEHP(const std::string& entity_id, float ehp) {
    auto* scr = getComponentFor(entity_id);
    if (!scr) return false;
    scr->total_ehp = std::max(0.0f, ehp);
    scr->needs_recalculation = true;
    return true;
}

bool ShipCapabilityRatingSystem::setIndustryModuleCount(const std::string& entity_id, int count) {
    auto* scr = getComponentFor(entity_id);
    if (!scr) return false;
    scr->industry_module_count = std::max(0, count);
    scr->needs_recalculation = true;
    return true;
}

bool ShipCapabilityRatingSystem::recalculate(const std::string& entity_id) {
    auto* scr = getComponentFor(entity_id);
    if (!scr) return false;
    scr->needs_recalculation = true;
    return true;
}

float ShipCapabilityRatingSystem::getCombatRating(const std::string& entity_id) const {
    const auto* scr = getComponentFor(entity_id);
    if (!scr) return 0.0f;
    return scr->combat_score;
}

float ShipCapabilityRatingSystem::getMiningRating(const std::string& entity_id) const {
    const auto* scr = getComponentFor(entity_id);
    if (!scr) return 0.0f;
    return scr->mining_score;
}

float ShipCapabilityRatingSystem::getExplorationRating(const std::string& entity_id) const {
    const auto* scr = getComponentFor(entity_id);
    if (!scr) return 0.0f;
    return scr->exploration_score;
}

float ShipCapabilityRatingSystem::getCargoRating(const std::string& entity_id) const {
    const auto* scr = getComponentFor(entity_id);
    if (!scr) return 0.0f;
    return scr->cargo_score;
}

float ShipCapabilityRatingSystem::getDefenseRating(const std::string& entity_id) const {
    const auto* scr = getComponentFor(entity_id);
    if (!scr) return 0.0f;
    return scr->defense_score;
}

float ShipCapabilityRatingSystem::getFabricationRating(const std::string& entity_id) const {
    const auto* scr = getComponentFor(entity_id);
    if (!scr) return 0.0f;
    return scr->fabrication_score;
}

float ShipCapabilityRatingSystem::getOverallRating(const std::string& entity_id) const {
    const auto* scr = getComponentFor(entity_id);
    if (!scr) return 0.0f;
    return scr->overall_rating;
}

} // namespace systems
} // namespace atlas
