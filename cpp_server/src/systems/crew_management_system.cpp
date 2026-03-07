#include "systems/crew_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using CM = components::CrewManagement;

const char* moraleToString(CM::MoraleLevel m) {
    switch (m) {
        case CM::MoraleLevel::Mutinous:    return "Mutinous";
        case CM::MoraleLevel::Low:         return "Low";
        case CM::MoraleLevel::Normal:      return "Normal";
        case CM::MoraleLevel::High:        return "High";
        case CM::MoraleLevel::Exceptional: return "Exceptional";
    }
    return "Unknown";
}

CM::MoraleLevel moraleFromFloat(float m) {
    if (m < 0.15f) return CM::MoraleLevel::Mutinous;
    if (m < 0.35f) return CM::MoraleLevel::Low;
    if (m < 0.65f) return CM::MoraleLevel::Normal;
    if (m < 0.85f) return CM::MoraleLevel::High;
    return CM::MoraleLevel::Exceptional;
}

CM::CrewRole roleFromString(const std::string& s) {
    if (s == "Engineer")       return CM::CrewRole::Engineer;
    if (s == "Gunner")         return CM::CrewRole::Gunner;
    if (s == "Navigator")      return CM::CrewRole::Navigator;
    if (s == "Medic")          return CM::CrewRole::Medic;
    if (s == "ScienceOfficer") return CM::CrewRole::ScienceOfficer;
    return CM::CrewRole::Pilot;
}

void recalcAverages(CM& comp) {
    if (comp.crew.empty()) {
        comp.average_morale = 0.5f;
        comp.efficiency_multiplier = 1.0f;
        return;
    }
    float sum_morale = 0.0f;
    float sum_skill = 0.0f;
    int assigned = 0;
    for (const auto& c : comp.crew) {
        sum_morale += c.morale;
        if (c.assigned) {
            sum_skill += static_cast<float>(c.skill_level);
            assigned++;
        }
    }
    comp.average_morale = sum_morale / static_cast<float>(comp.crew.size());
    float morale_factor = 0.5f + comp.average_morale; // 0.5..1.5
    float skill_factor = assigned > 0
        ? (sum_skill / static_cast<float>(assigned)) / 10.0f  // normalise to 0..1
        : 0.0f;
    comp.efficiency_multiplier = morale_factor * (0.5f + skill_factor); // 0.25..2.25
}

} // anonymous namespace

CrewManagementSystem::CrewManagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CrewManagementSystem::updateComponent(ecs::Entity& entity,
    components::CrewManagement& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Pay salaries at intervals
    comp.salary_timer += delta_time;
    if (comp.salary_timer >= comp.salary_interval) {
        comp.salary_timer -= comp.salary_interval;
        double pay = 0.0;
        for (const auto& c : comp.crew) {
            if (c.assigned) pay += static_cast<double>(c.salary_per_hour);
        }
        comp.total_salary_paid += pay;
    }

    recalcAverages(comp);
}

bool CrewManagementSystem::initialize(const std::string& entity_id, int max_crew) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CrewManagement>();
    comp->max_crew = max_crew;
    entity->addComponent(std::move(comp));
    return true;
}

bool CrewManagementSystem::hireCrew(const std::string& entity_id,
    const std::string& name, const std::string& role, int skill_level, float salary) {
    auto* cm = getComponentFor(entity_id);
    if (!cm) return false;
    if (static_cast<int>(cm->crew.size()) >= cm->max_crew) return false;
    for (const auto& c : cm->crew) {
        if (c.name == name) return false; // duplicate
    }
    CM::CrewMember member;
    member.name = name;
    member.role = roleFromString(role);
    member.skill_level = std::max(1, std::min(skill_level, 10));
    member.salary_per_hour = salary;
    member.morale = 0.5f;
    member.assigned = false;
    cm->crew.push_back(member);
    cm->total_hired++;
    recalcAverages(*cm);
    return true;
}

bool CrewManagementSystem::dismissCrew(const std::string& entity_id,
    const std::string& name) {
    auto* cm = getComponentFor(entity_id);
    if (!cm) return false;
    auto it = std::find_if(cm->crew.begin(), cm->crew.end(),
        [&](const CM::CrewMember& c) { return c.name == name; });
    if (it == cm->crew.end()) return false;
    cm->crew.erase(it);
    cm->total_dismissed++;
    recalcAverages(*cm);
    return true;
}

bool CrewManagementSystem::assignCrew(const std::string& entity_id,
    const std::string& name) {
    auto* cm = getComponentFor(entity_id);
    if (!cm) return false;
    for (auto& c : cm->crew) {
        if (c.name == name) {
            if (c.assigned) return false;
            c.assigned = true;
            recalcAverages(*cm);
            return true;
        }
    }
    return false;
}

bool CrewManagementSystem::unassignCrew(const std::string& entity_id,
    const std::string& name) {
    auto* cm = getComponentFor(entity_id);
    if (!cm) return false;
    for (auto& c : cm->crew) {
        if (c.name == name) {
            if (!c.assigned) return false;
            c.assigned = false;
            recalcAverages(*cm);
            return true;
        }
    }
    return false;
}

bool CrewManagementSystem::adjustMorale(const std::string& entity_id,
    const std::string& name, float delta) {
    auto* cm = getComponentFor(entity_id);
    if (!cm) return false;
    for (auto& c : cm->crew) {
        if (c.name == name) {
            c.morale = std::max(0.0f, std::min(c.morale + delta, 1.0f));
            recalcAverages(*cm);
            return true;
        }
    }
    return false;
}

int CrewManagementSystem::getCrewCount(const std::string& entity_id) const {
    auto* cm = getComponentFor(entity_id);
    return cm ? static_cast<int>(cm->crew.size()) : 0;
}

int CrewManagementSystem::getAssignedCount(const std::string& entity_id) const {
    auto* cm = getComponentFor(entity_id);
    if (!cm) return 0;
    int count = 0;
    for (const auto& c : cm->crew) {
        if (c.assigned) count++;
    }
    return count;
}

float CrewManagementSystem::getAverageMorale(const std::string& entity_id) const {
    auto* cm = getComponentFor(entity_id);
    return cm ? cm->average_morale : 0.0f;
}

float CrewManagementSystem::getEfficiencyMultiplier(const std::string& entity_id) const {
    auto* cm = getComponentFor(entity_id);
    return cm ? cm->efficiency_multiplier : 0.0f;
}

double CrewManagementSystem::getTotalSalaryPaid(const std::string& entity_id) const {
    auto* cm = getComponentFor(entity_id);
    return cm ? cm->total_salary_paid : 0.0;
}

int CrewManagementSystem::getTotalHired(const std::string& entity_id) const {
    auto* cm = getComponentFor(entity_id);
    return cm ? cm->total_hired : 0;
}

int CrewManagementSystem::getTotalDismissed(const std::string& entity_id) const {
    auto* cm = getComponentFor(entity_id);
    return cm ? cm->total_dismissed : 0;
}

std::string CrewManagementSystem::getMoraleLevel(const std::string& entity_id) const {
    auto* cm = getComponentFor(entity_id);
    if (!cm) return "Unknown";
    return moraleToString(moraleFromFloat(cm->average_morale));
}

} // namespace systems
} // namespace atlas
