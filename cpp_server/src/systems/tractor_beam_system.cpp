#include "systems/tractor_beam_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

TractorBeamSystem::TractorBeamSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void TractorBeamSystem::updateComponent(ecs::Entity& entity,
    components::TractorBeam& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (!comp.locked) return;

    // Pull the target closer
    float pull = comp.pull_speed * delta_time;
    comp.current_distance = std::max(0.0f, comp.current_distance - pull);

    // Auto-collect when close enough
    if (comp.current_distance <= comp.collection_distance) {
        comp.items_collected++;
        comp.locked = false;
        comp.target_id.clear();
        comp.current_distance = 0.0f;
    }
}

bool TractorBeamSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::TractorBeam>();
    entity->addComponent(std::move(comp));
    return true;
}

bool TractorBeamSystem::lockTarget(const std::string& entity_id,
    const std::string& target_id, float distance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->locked) return false;
    if (distance > comp->range) {
        comp->items_failed++;
        return false;
    }
    comp->target_id = target_id;
    comp->current_distance = distance;
    comp->locked = true;
    return true;
}

bool TractorBeamSystem::unlockTarget(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->locked) return false;
    comp->locked = false;
    comp->target_id.clear();
    comp->current_distance = 0.0f;
    return true;
}

bool TractorBeamSystem::setRange(const std::string& entity_id, float range) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (range <= 0.0f) return false;
    comp->range = range;
    return true;
}

bool TractorBeamSystem::setPullSpeed(const std::string& entity_id, float speed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (speed <= 0.0f) return false;
    comp->pull_speed = speed;
    return true;
}

bool TractorBeamSystem::isLocked(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->locked : false;
}

float TractorBeamSystem::getCurrentDistance(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_distance : 0.0f;
}

int TractorBeamSystem::getItemsCollected(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->items_collected : 0;
}

int TractorBeamSystem::getItemsFailed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->items_failed : 0;
}

std::string TractorBeamSystem::getTargetId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->target_id : "";
}

} // namespace systems
} // namespace atlas
