#include "systems/warp_bubble_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

WarpBubbleSystem::WarpBubbleSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void WarpBubbleSystem::updateComponent(ecs::Entity& entity,
    components::WarpBubbleState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Tick down bubble lifetimes
    for (auto& b : comp.bubbles) {
        if (!b.expired) {
            b.remaining -= delta_time;
            if (b.remaining <= 0.0f) {
                b.remaining = 0.0f;
                b.expired = true;
            }
        }
    }

    // Remove expired bubbles
    comp.bubbles.erase(
        std::remove_if(comp.bubbles.begin(), comp.bubbles.end(),
            [](const components::WarpBubbleState::Bubble& b) {
                return b.expired;
            }),
        comp.bubbles.end());
}

bool WarpBubbleSystem::initialize(const std::string& entity_id,
    const std::string& system_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::WarpBubbleState>();
    comp->system_id = system_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool WarpBubbleSystem::deployBubble(const std::string& entity_id,
    const std::string& deployer_id, float radius, float lifetime,
    float x, float y, float z) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->bubbles.size()) >= comp->max_bubbles) return false;

    components::WarpBubbleState::Bubble b;
    b.bubble_id = "bubble_" + std::to_string(comp->total_deployed);
    b.deployer_id = deployer_id;
    b.radius = radius;
    b.lifetime = lifetime;
    b.remaining = lifetime;
    b.x = x;
    b.y = y;
    b.z = z;
    comp->bubbles.push_back(b);
    comp->total_deployed++;
    return true;
}

bool WarpBubbleSystem::removeBubble(const std::string& entity_id,
    const std::string& bubble_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->bubbles.begin(), comp->bubbles.end(),
        [&](const components::WarpBubbleState::Bubble& b) {
            return b.bubble_id == bubble_id;
        });
    if (it == comp->bubbles.end()) return false;
    comp->bubbles.erase(it);
    return true;
}

bool WarpBubbleSystem::catchShip(const std::string& entity_id,
    const std::string& bubble_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& b : comp->bubbles) {
        if (b.bubble_id == bubble_id && !b.expired) {
            b.ships_caught++;
            comp->total_ships_caught++;
            return true;
        }
    }
    return false;
}

int WarpBubbleSystem::getBubbleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->bubbles.size()) : 0;
}

int WarpBubbleSystem::getActiveBubbleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& b : comp->bubbles) {
        if (!b.expired) count++;
    }
    return count;
}

int WarpBubbleSystem::getTotalDeployed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_deployed : 0;
}

int WarpBubbleSystem::getTotalShipsCaught(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_ships_caught : 0;
}

float WarpBubbleSystem::getBubbleRemaining(const std::string& entity_id,
    const std::string& bubble_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& b : comp->bubbles) {
        if (b.bubble_id == bubble_id) return b.remaining;
    }
    return 0.0f;
}

bool WarpBubbleSystem::isBubbleExpired(const std::string& entity_id,
    const std::string& bubble_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return true;
    for (const auto& b : comp->bubbles) {
        if (b.bubble_id == bubble_id) return b.expired;
    }
    return true;
}

} // namespace systems
} // namespace atlas
