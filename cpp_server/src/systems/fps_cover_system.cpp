#include "systems/fps_cover_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FPSCoverSystem::FPSCoverSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FPSCoverSystem::updateComponent(ecs::Entity& /*entity*/,
    components::FPSCover& cover, float delta_time) {
    if (!cover.active) return;

    // Handle cover transition
    if (cover.state == components::FPSCover::Transitioning) {
        cover.transition_progress += delta_time / cover.transition_time;
        if (cover.transition_progress >= 1.0f) {
            cover.transition_progress = 0.0f;
            if (!cover.current_cover_id.empty()) {
                cover.state = components::FPSCover::InCover;
            } else {
                cover.state = components::FPSCover::Exposed;
            }
        }
    }

    // Track peek duration
    if (cover.state == components::FPSCover::Peeking) {
        cover.peek_duration += delta_time;
    }
}

bool FPSCoverSystem::addCoverPoint(const std::string& entity_id,
    const std::string& point_id, float x, float y, float z,
    int type, float facing_angle) {
    if (type < 0 || type > 3) return false;
    auto* cover = getComponentFor(entity_id);
    if (!cover) return false;

    for (const auto& p : cover->cover_points) {
        if (p.point_id == point_id) return false;
    }
    if (static_cast<int>(cover->cover_points.size()) >= cover->max_cover_points) return false;

    components::FPSCover::CoverPoint point;
    point.point_id = point_id;
    point.x = x;
    point.y = y;
    point.z = z;
    point.type = static_cast<components::FPSCover::CoverType>(type);
    point.facing_angle = facing_angle;
    cover->cover_points.push_back(point);
    return true;
}

bool FPSCoverSystem::removeCoverPoint(const std::string& entity_id,
    const std::string& point_id) {
    auto* cover = getComponentFor(entity_id);
    if (!cover) return false;

    auto it = std::remove_if(cover->cover_points.begin(), cover->cover_points.end(),
        [&point_id](const components::FPSCover::CoverPoint& p) {
            return p.point_id == point_id;
        });
    if (it == cover->cover_points.end()) return false;
    cover->cover_points.erase(it, cover->cover_points.end());
    if (cover->current_cover_id == point_id) {
        cover->current_cover_id.clear();
        cover->state = components::FPSCover::Exposed;
    }
    return true;
}

bool FPSCoverSystem::enterCover(const std::string& entity_id,
    const std::string& point_id) {
    auto* cover = getComponentFor(entity_id);
    if (!cover) return false;
    if (cover->state == components::FPSCover::InCover) return false;

    for (auto& p : cover->cover_points) {
        if (p.point_id == point_id) {
            if (p.is_occupied) return false;
            if (p.type == components::FPSCover::None) return false;
            p.is_occupied = true;
            cover->current_cover_id = point_id;
            cover->state = components::FPSCover::Transitioning;
            cover->transition_progress = 0.0f;
            cover->total_covers_used++;
            return true;
        }
    }
    return false;
}

bool FPSCoverSystem::leaveCover(const std::string& entity_id) {
    auto* cover = getComponentFor(entity_id);
    if (!cover) return false;
    if (cover->state != components::FPSCover::InCover &&
        cover->state != components::FPSCover::Peeking) return false;

    for (auto& p : cover->cover_points) {
        if (p.point_id == cover->current_cover_id) {
            p.is_occupied = false;
            break;
        }
    }
    cover->current_cover_id.clear();
    cover->state = components::FPSCover::Transitioning;
    cover->transition_progress = 0.0f;
    cover->peek_duration = 0.0f;
    return true;
}

bool FPSCoverSystem::startPeek(const std::string& entity_id) {
    auto* cover = getComponentFor(entity_id);
    if (!cover) return false;
    if (cover->state != components::FPSCover::InCover) return false;
    cover->state = components::FPSCover::Peeking;
    cover->peek_duration = 0.0f;
    return true;
}

bool FPSCoverSystem::stopPeek(const std::string& entity_id) {
    auto* cover = getComponentFor(entity_id);
    if (!cover) return false;
    if (cover->state != components::FPSCover::Peeking) return false;
    cover->state = components::FPSCover::InCover;
    cover->peek_duration = 0.0f;
    return true;
}

int FPSCoverSystem::getCoverState(const std::string& entity_id) const {
    auto* cover = getComponentFor(entity_id);
    return cover ? static_cast<int>(cover->state) : 0;
}

float FPSCoverSystem::getDamageReduction(const std::string& entity_id) const {
    auto* cover = getComponentFor(entity_id);
    if (!cover) return 0.0f;

    if (cover->state == components::FPSCover::Peeking) {
        return cover->damage_reduction_peek;
    }

    if (cover->state == components::FPSCover::InCover) {
        // Find current cover type
        for (const auto& p : cover->cover_points) {
            if (p.point_id == cover->current_cover_id) {
                if (p.type == components::FPSCover::FullCover) return cover->damage_reduction_full;
                if (p.type == components::FPSCover::HalfCover) return cover->damage_reduction_half;
                if (p.type == components::FPSCover::Destructible) return cover->damage_reduction_half;
                break;
            }
        }
    }

    return 0.0f;
}

bool FPSCoverSystem::damageCoverPoint(const std::string& entity_id,
    const std::string& point_id, float amount) {
    if (amount <= 0.0f) return false;
    auto* cover = getComponentFor(entity_id);
    if (!cover) return false;

    for (auto& p : cover->cover_points) {
        if (p.point_id == point_id && p.type == components::FPSCover::Destructible) {
            p.health -= amount;
            if (p.health <= 0.0f) {
                p.health = 0.0f;
                p.type = components::FPSCover::None;
                cover->total_covers_destroyed++;
                if (cover->current_cover_id == point_id) {
                    cover->current_cover_id.clear();
                    cover->state = components::FPSCover::Exposed;
                    p.is_occupied = false;
                }
            }
            return true;
        }
    }
    return false;
}

int FPSCoverSystem::getCoverPointCount(const std::string& entity_id) const {
    auto* cover = getComponentFor(entity_id);
    return cover ? static_cast<int>(cover->cover_points.size()) : 0;
}

int FPSCoverSystem::getTotalCoversUsed(const std::string& entity_id) const {
    auto* cover = getComponentFor(entity_id);
    return cover ? cover->total_covers_used : 0;
}

int FPSCoverSystem::getTotalCoversDestroyed(const std::string& entity_id) const {
    auto* cover = getComponentFor(entity_id);
    return cover ? cover->total_covers_destroyed : 0;
}

} // namespace systems
} // namespace atlas
