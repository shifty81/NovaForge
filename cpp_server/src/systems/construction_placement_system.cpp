#include "systems/construction_placement_system.h"
#include "ecs/world.h"
#include <cmath>

namespace atlas {
namespace systems {

ConstructionPlacementSystem::ConstructionPlacementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ConstructionPlacementSystem::updateComponent(ecs::Entity& /*entity*/,
    components::ConstructionPlacementState& /*state*/, float /*delta_time*/) {
    // Event-driven system — no per-tick update needed.
}

float ConstructionPlacementSystem::snapToGrid(float value, float grid_size) {
    if (grid_size <= 0.0f) return value;
    return std::round(value / grid_size) * grid_size;
}

bool ConstructionPlacementSystem::isGridAligned(float x, float y, float z, float grid_size) {
    if (grid_size <= 0.0f) return true;
    auto aligned = [grid_size](float v) {
        float snapped = std::round(v / grid_size) * grid_size;
        return std::fabs(v - snapped) < 0.001f;
    };
    return aligned(x) && aligned(y) && aligned(z);
}

bool ConstructionPlacementSystem::placeModule(const std::string& entity_id,
    const std::string& module_id, float x, float y, float z) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (state->occupied_sockets >= state->max_sockets) {
        state->placement_valid = false;
        return false;
    }

    if (state->snap_to_grid) {
        x = snapToGrid(x, state->grid_size);
        y = snapToGrid(y, state->grid_size);
        z = snapToGrid(z, state->grid_size);
    }

    state->placement_x = x;
    state->placement_y = y;
    state->placement_z = z;
    state->selected_module_id = module_id;
    state->placement_valid = true;
    state->occupied_sockets++;
    state->total_placements++;
    return true;
}

bool ConstructionPlacementSystem::removeModule(const std::string& entity_id, int socket_index) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (socket_index < 0 || socket_index >= state->occupied_sockets) return false;

    state->occupied_sockets--;
    state->total_removals++;
    return true;
}

bool ConstructionPlacementSystem::setContext(const std::string& entity_id,
    components::ConstructionPlacementState::BuildContext ctx) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->context = ctx;
    return true;
}

bool ConstructionPlacementSystem::selectModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->selected_module_id = module_id;
    return true;
}

bool ConstructionPlacementSystem::validatePlacement(const std::string& entity_id,
    float x, float y, float z) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (state->occupied_sockets >= state->max_sockets) return false;
    if (state->snap_to_grid && !isGridAligned(x, y, z, state->grid_size)) return false;
    return true;
}

int ConstructionPlacementSystem::getOccupiedSockets(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->occupied_sockets : 0;
}

int ConstructionPlacementSystem::getAvailableSockets(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? (state->max_sockets - state->occupied_sockets) : 0;
}

} // namespace systems
} // namespace atlas
