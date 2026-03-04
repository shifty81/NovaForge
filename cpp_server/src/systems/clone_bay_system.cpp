#include "systems/clone_bay_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using CB = components::CloneBay;
using Clone = components::CloneBay::Clone;
using Implant = components::CloneBay::Implant;

Clone* findClone(CB* bay, const std::string& clone_id) {
    for (auto& c : bay->clones) {
        if (c.clone_id == clone_id) return &c;
    }
    return nullptr;
}

const Clone* findCloneConst(const CB* bay, const std::string& clone_id) {
    for (const auto& c : bay->clones) {
        if (c.clone_id == clone_id) return &c;
    }
    return nullptr;
}

Implant* findImplant(CB* bay, const std::string& implant_id) {
    for (auto& i : bay->implants) {
        if (i.implant_id == implant_id) return &i;
    }
    return nullptr;
}

int countImplantsInClone(const CB* bay, const std::string& clone_id) {
    int count = 0;
    for (const auto& i : bay->implants) {
        if (i.installed_in_clone == clone_id) count++;
    }
    return count;
}

} // anonymous namespace

CloneBaySystem::CloneBaySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CloneBaySystem::updateComponent(ecs::Entity& /*entity*/, components::CloneBay& /*bay*/, float /*delta_time*/) {
    // No per-tick logic needed
}

bool CloneBaySystem::initialize(const std::string& entity_id,
    const std::string& clone_bay_id, const std::string& station_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CloneBay>();
    comp->clone_bay_id = clone_bay_id;
    comp->station_id = station_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool CloneBaySystem::addClone(const std::string& entity_id,
    const std::string& clone_id, int grade) {
    auto* bay = getComponentFor(entity_id);
    if (!bay) return false;
    if (static_cast<int>(bay->clones.size()) >= bay->max_clones) return false;
    if (findClone(bay, clone_id)) return false;

    Clone c;
    c.clone_id = clone_id;
    c.grade = grade;
    c.sp_limit = static_cast<float>(grade) * 900000.0f;
    c.implant_slots = grade;
    c.cost = static_cast<float>(grade) * 5000000.0f;
    c.installed = true;
    bay->clones.push_back(c);
    return true;
}

bool CloneBaySystem::removeClone(const std::string& entity_id,
    const std::string& clone_id) {
    auto* bay = getComponentFor(entity_id);
    if (!bay) return false;

    auto it = std::remove_if(bay->clones.begin(), bay->clones.end(),
        [&](const Clone& c) { return c.clone_id == clone_id; });
    if (it == bay->clones.end()) return false;
    bay->clones.erase(it, bay->clones.end());
    return true;
}

bool CloneBaySystem::activateClone(const std::string& entity_id,
    const std::string& clone_id) {
    auto* bay = getComponentFor(entity_id);
    if (!bay) return false;

    auto* target = findClone(bay, clone_id);
    if (!target) return false;

    // Deactivate current active clone
    for (auto& c : bay->clones) {
        c.active = false;
    }
    target->active = true;
    bay->total_activations++;
    return true;
}

bool CloneBaySystem::installImplant(const std::string& entity_id,
    const std::string& implant_id, int slot, const std::string& attribute,
    float bonus, const std::string& clone_id) {
    auto* bay = getComponentFor(entity_id);
    if (!bay) return false;
    if (static_cast<int>(bay->implants.size()) >= bay->max_implants) return false;
    if (findImplant(bay, implant_id)) return false;

    // Check clone exists and has free slots
    auto* clone = findClone(bay, clone_id);
    if (!clone) return false;
    if (countImplantsInClone(bay, clone_id) >= clone->implant_slots) return false;

    Implant imp;
    imp.implant_id = implant_id;
    imp.slot = slot;
    imp.attribute = attribute;
    imp.bonus = bonus;
    imp.installed_in_clone = clone_id;
    bay->implants.push_back(imp);
    return true;
}

bool CloneBaySystem::removeImplant(const std::string& entity_id,
    const std::string& implant_id) {
    auto* bay = getComponentFor(entity_id);
    if (!bay) return false;

    auto it = std::remove_if(bay->implants.begin(), bay->implants.end(),
        [&](const Implant& i) { return i.implant_id == implant_id; });
    if (it == bay->implants.end()) return false;
    bay->implants.erase(it, bay->implants.end());
    return true;
}

float CloneBaySystem::processDeath(const std::string& entity_id, float skill_points) {
    auto* bay = getComponentFor(entity_id);
    if (!bay) return 0.0f;

    bay->total_deaths++;

    // Find active clone
    float sp_limit = 0.0f;
    for (const auto& c : bay->clones) {
        if (c.active) { sp_limit = c.sp_limit; break; }
    }

    float sp_loss = 0.0f;
    if (skill_points > sp_limit) {
        sp_loss = skill_points - sp_limit;
    }
    bay->skill_points_at_risk = sp_loss;
    return sp_loss;
}

int CloneBaySystem::getActiveClone(const std::string& entity_id) const {
    const auto* bay = getComponentFor(entity_id);
    if (!bay) return 0;

    for (const auto& c : bay->clones) {
        if (c.active) return c.grade;
    }
    return 0;
}

int CloneBaySystem::getCloneCount(const std::string& entity_id) const {
    const auto* bay = getComponentFor(entity_id);
    return bay ? static_cast<int>(bay->clones.size()) : 0;
}

int CloneBaySystem::getImplantCount(const std::string& entity_id) const {
    const auto* bay = getComponentFor(entity_id);
    return bay ? static_cast<int>(bay->implants.size()) : 0;
}

int CloneBaySystem::getTotalDeaths(const std::string& entity_id) const {
    const auto* bay = getComponentFor(entity_id);
    return bay ? bay->total_deaths : 0;
}

float CloneBaySystem::getSkillPointsAtRisk(const std::string& entity_id,
    float skill_points) const {
    const auto* bay = getComponentFor(entity_id);
    if (!bay) return 0.0f;

    float sp_limit = 0.0f;
    for (const auto& c : bay->clones) {
        if (c.active) { sp_limit = c.sp_limit; break; }
    }
    return (skill_points > sp_limit) ? (skill_points - sp_limit) : 0.0f;
}

} // namespace systems
} // namespace atlas
