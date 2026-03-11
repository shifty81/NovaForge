#include "systems/target_painter_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

TargetPainterSystem::TargetPainterSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void TargetPainterSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::TargetPainterState& comp,
        float delta_time) {
    comp.elapsed += delta_time;

    for (auto& p : comp.painters) {
        p.cycle_elapsed += delta_time;
        if (p.cycle_elapsed >= p.cycle_time) {
            p.cycle_elapsed -= p.cycle_time;
        }
    }
    comp.is_painted = !comp.painters.empty();
    recomputeSignature(comp);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool TargetPainterSystem::initialize(const std::string& entity_id,
                                      float base_signature) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (base_signature <= 0.0f) return false;
    auto comp = std::make_unique<components::TargetPainterState>();
    comp->base_signature      = base_signature;
    comp->effective_signature = base_signature;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Painter management
// ---------------------------------------------------------------------------

bool TargetPainterSystem::applyPainter(const std::string& entity_id,
                                        const std::string& painter_id,
                                        const std::string& source_id,
                                        float strength,
                                        float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (painter_id.empty() || source_id.empty()) return false;
    if (strength <= 0.0f || strength > 1.0f) return false;
    if (cycle_time <= 0.0f) return false;
    if (static_cast<int>(comp->painters.size()) >= comp->max_painters) return false;

    for (const auto& p : comp->painters) {
        if (p.painter_id == painter_id) return false;
    }

    components::TargetPainterState::Painter painter;
    painter.painter_id    = painter_id;
    painter.source_id     = source_id;
    painter.strength      = strength;
    painter.cycle_time    = cycle_time;
    painter.active        = true;
    comp->painters.push_back(painter);
    comp->total_painters_applied++;
    comp->is_painted = true;
    recomputeSignature(*comp);
    return true;
}

bool TargetPainterSystem::removePainter(const std::string& entity_id,
                                         const std::string& painter_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->painters.begin(), comp->painters.end(),
        [&](const components::TargetPainterState::Painter& p) {
            return p.painter_id == painter_id;
        });
    if (it == comp->painters.end()) return false;
    comp->painters.erase(it);
    comp->is_painted = !comp->painters.empty();
    recomputeSignature(*comp);
    return true;
}

bool TargetPainterSystem::clearPainters(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->painters.clear();
    comp->is_painted = false;
    recomputeSignature(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool TargetPainterSystem::setBaseSignature(const std::string& entity_id,
                                            float signature) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (signature <= 0.0f) return false;
    comp->base_signature = signature;
    recomputeSignature(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

float TargetPainterSystem::getEffectiveSignature(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_signature : 0.0f;
}

float TargetPainterSystem::getBaseSignature(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->base_signature : 0.0f;
}

int TargetPainterSystem::getPainterCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->painters.size()) : 0;
}

int TargetPainterSystem::getActivePainterCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& p : comp->painters) {
        if (p.active) count++;
    }
    return count;
}

bool TargetPainterSystem::isPainted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->is_painted : false;
}

int TargetPainterSystem::getTotalPaintersApplied(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_painters_applied : 0;
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void TargetPainterSystem::recomputeSignature(
        components::TargetPainterState& comp) {
    float factor = 1.0f;
    for (const auto& p : comp.painters) {
        factor *= (1.0f + p.strength);
    }
    comp.effective_signature = comp.base_signature * factor;
}

} // namespace systems
} // namespace atlas
