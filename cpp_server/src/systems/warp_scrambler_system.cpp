#include "systems/warp_scrambler_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

WarpScramblerSystem::WarpScramblerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void WarpScramblerSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::WarpScramblerState& comp,
        float delta_time) {
    comp.elapsed += delta_time;

    for (auto& s : comp.scramblers) {
        s.cycle_elapsed += delta_time;
        if (s.cycle_elapsed >= s.cycle_time) {
            s.cycle_elapsed -= s.cycle_time;
        }
    }
    recomputePoints(comp);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool WarpScramblerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::WarpScramblerState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Module management
// ---------------------------------------------------------------------------

bool WarpScramblerSystem::applyScrambler(const std::string& entity_id,
                                          const std::string& scrambler_id,
                                          const std::string& source_id,
                                          int   scramble_points,
                                          float optimal_range,
                                          float cycle_time,
                                          bool  is_scrambler) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (scrambler_id.empty() || source_id.empty()) return false;
    if (scramble_points <= 0) return false;
    if (optimal_range <= 0.0f) return false;
    if (cycle_time <= 0.0f) return false;
    if (static_cast<int>(comp->scramblers.size()) >= comp->max_scramblers) return false;

    for (const auto& s : comp->scramblers) {
        if (s.scrambler_id == scrambler_id) return false;
    }

    components::WarpScramblerState::Scrambler scr;
    scr.scrambler_id    = scrambler_id;
    scr.source_id       = source_id;
    scr.scramble_points = scramble_points;
    scr.optimal_range   = optimal_range;
    scr.cycle_time      = cycle_time;
    scr.active          = true;
    scr.is_scrambler    = is_scrambler;
    comp->scramblers.push_back(scr);
    comp->total_scrambles_applied++;
    recomputePoints(*comp);
    return true;
}

bool WarpScramblerSystem::removeScrambler(const std::string& entity_id,
                                           const std::string& scrambler_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->scramblers.begin(), comp->scramblers.end(),
        [&](const components::WarpScramblerState::Scrambler& s) {
            return s.scrambler_id == scrambler_id;
        });
    if (it == comp->scramblers.end()) return false;
    comp->scramblers.erase(it);
    recomputePoints(*comp);
    return true;
}

bool WarpScramblerSystem::clearScramblers(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->scramblers.clear();
    recomputePoints(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int WarpScramblerSystem::getTotalScramblerPoints(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_scramble_points : 0;
}

bool WarpScramblerSystem::isWarpScrambled(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->is_warp_scrambled : false;
}

int WarpScramblerSystem::getScramblerCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->scramblers.size()) : 0;
}

int WarpScramblerSystem::getActiveScramblerCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->scramblers) {
        if (s.active) count++;
    }
    return count;
}

int WarpScramblerSystem::getTotalScrambles(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_scrambles_applied : 0;
}

bool WarpScramblerSystem::isMwdDisabled(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->scramblers) {
        if (s.active && s.is_scrambler) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void WarpScramblerSystem::recomputePoints(
        components::WarpScramblerState& comp) {
    int total = 0;
    for (const auto& s : comp.scramblers) {
        if (s.active) total += s.scramble_points;
    }
    comp.total_scramble_points = total;
    comp.is_warp_scrambled     = (total > 0);
}

} // namespace systems
} // namespace atlas
