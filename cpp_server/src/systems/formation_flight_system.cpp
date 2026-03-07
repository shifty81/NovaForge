#include "systems/formation_flight_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/fleet_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {

using FF = components::FormationFlight;

const char* formationToString(FF::FormationType f) {
    switch (f) {
        case FF::FormationType::Line:    return "Line";
        case FF::FormationType::Wedge:   return "Wedge";
        case FF::FormationType::Sphere:  return "Sphere";
        case FF::FormationType::Wall:    return "Wall";
        case FF::FormationType::Echelon: return "Echelon";
    }
    return "Unknown";
}

FF::FormationType stringToFormation(const std::string& s) {
    if (s == "Line")    return FF::FormationType::Line;
    if (s == "Sphere")  return FF::FormationType::Sphere;
    if (s == "Wall")    return FF::FormationType::Wall;
    if (s == "Echelon") return FF::FormationType::Echelon;
    return FF::FormationType::Wedge;
}

const char* statusToString(FF::SlotStatus s) {
    switch (s) {
        case FF::SlotStatus::Holding:   return "Holding";
        case FF::SlotStatus::Drifting:  return "Drifting";
        case FF::SlotStatus::Broken:    return "Broken";
        case FF::SlotStatus::Reforming: return "Reforming";
    }
    return "Unknown";
}

double computeDrift(const FF& comp) {
    double dx = comp.actual_x - comp.offset_x;
    double dy = comp.actual_y - comp.offset_y;
    double dz = comp.actual_z - comp.offset_z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

void recalcCohesion(FF& comp) {
    double drift = computeDrift(comp);
    if (comp.max_drift <= 0.0f) {
        comp.cohesion = 1.0f;
    } else {
        comp.cohesion = std::max(0.0f,
            1.0f - static_cast<float>(drift / comp.max_drift));
    }

    // Status based on cohesion
    if (comp.status == FF::SlotStatus::Reforming) {
        if (comp.cohesion >= 0.8f) {
            comp.status = FF::SlotStatus::Holding;
            comp.formation_reforms++;
        }
    } else if (comp.cohesion >= 0.8f) {
        comp.status = FF::SlotStatus::Holding;
    } else if (comp.cohesion >= 0.3f) {
        comp.status = FF::SlotStatus::Drifting;
    } else {
        if (comp.status != FF::SlotStatus::Broken) {
            comp.formation_breaks++;
        }
        comp.status = FF::SlotStatus::Broken;
    }

    // Cohesion bonus: full bonus at 1.0, zero at < 0.5
    comp.cohesion_bonus = (comp.cohesion >= 0.5f)
        ? (comp.cohesion - 0.5f) * 0.2f  // 0.0–0.1 bonus
        : 0.0f;
}

} // anonymous namespace

FormationFlightSystem::FormationFlightSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FormationFlightSystem::updateComponent(ecs::Entity& entity,
    components::FormationFlight& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    recalcCohesion(comp);
}

bool FormationFlightSystem::initialize(const std::string& entity_id,
    const std::string& fleet_id, const std::string& leader_id,
    int slot_index) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FormationFlight>();
    comp->fleet_id = fleet_id;
    comp->leader_id = leader_id;
    comp->slot_index = slot_index;
    entity->addComponent(std::move(comp));
    return true;
}

bool FormationFlightSystem::setFormationType(const std::string& entity_id,
    const std::string& formation) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->formation = stringToFormation(formation);
    return true;
}

bool FormationFlightSystem::setSlotOffset(const std::string& entity_id,
    double x, double y, double z) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->offset_x = x;
    comp->offset_y = y;
    comp->offset_z = z;
    return true;
}

bool FormationFlightSystem::updateActualPosition(const std::string& entity_id,
    double x, double y, double z) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->actual_x = x;
    comp->actual_y = y;
    comp->actual_z = z;
    recalcCohesion(*comp);
    return true;
}

bool FormationFlightSystem::setMaxDrift(const std::string& entity_id,
    float max_drift) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->max_drift = max_drift;
    return true;
}

bool FormationFlightSystem::reformFormation(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->status = FF::SlotStatus::Reforming;
    return true;
}

std::string FormationFlightSystem::getFormationType(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    return formationToString(comp->formation);
}

std::string FormationFlightSystem::getSlotStatus(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    return statusToString(comp->status);
}

float FormationFlightSystem::getCohesion(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cohesion : 0.0f;
}

float FormationFlightSystem::getCohesionBonus(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cohesion_bonus : 0.0f;
}

int FormationFlightSystem::getFormationBreaks(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->formation_breaks : 0;
}

int FormationFlightSystem::getFormationReforms(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->formation_reforms : 0;
}

double FormationFlightSystem::getDriftDistance(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0;
    return computeDrift(*comp);
}

} // namespace systems
} // namespace atlas
