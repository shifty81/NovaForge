#include "systems/d_scan_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

DScanSystem::DScanSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void DScanSystem::updateComponent(ecs::Entity& /*entity*/,
                                  components::DScanState& comp,
                                  float delta_time) {
    if (!comp.active) return;
    if (!comp.is_scanning) return;

    comp.scan_timer -= delta_time;
    if (comp.scan_timer <= 0.0f) {
        comp.scan_timer = 0.0f;
        comp.is_scanning = false;
        comp.scan_count++;
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool DScanSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DScanState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Scanning
// ---------------------------------------------------------------------------

bool DScanSystem::startScan(const std::string& entity_id,
                             float range_au, float angle_deg) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->is_scanning) return false;
    if (range_au <= 0.0f || angle_deg <= 0.0f) return false;

    comp->scan_range   = range_au;
    comp->scan_angle   = angle_deg;
    comp->is_scanning  = true;
    comp->scan_timer   = comp->scan_duration;
    comp->contacts.clear();
    return true;
}

bool DScanSystem::addContact(const std::string& entity_id,
                              const std::string& contact_entity_id,
                              const std::string& name,
                              int contact_type,
                              float distance_au) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->contacts.size()) >= comp->max_contacts) return false;

    // Duplicate prevention
    for (const auto& c : comp->contacts) {
        if (c.entity_id == contact_entity_id) return false;
    }

    components::DScanState::Contact contact;
    contact.entity_id   = contact_entity_id;
    contact.name        = name;
    contact.type        = static_cast<components::DScanState::ContactType>(contact_type);
    contact.distance    = distance_au;
    comp->contacts.push_back(contact);
    return true;
}

bool DScanSystem::clearContacts(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->contacts.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool DScanSystem::isScanning(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp && comp->is_scanning;
}

int DScanSystem::getScanCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->scan_count : 0;
}

int DScanSystem::getContactCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->contacts.size()) : 0;
}

float DScanSystem::getScanRange(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->scan_range : 0.0f;
}

float DScanSystem::getScanAngle(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->scan_angle : 0.0f;
}

std::vector<components::DScanState::Contact>
DScanSystem::getContacts(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return {};
    return comp->contacts;
}

} // namespace systems
} // namespace atlas
