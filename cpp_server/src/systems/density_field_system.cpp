#include "systems/density_field_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

DensityFieldSystem::DensityFieldSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void DensityFieldSystem::updateComponent(ecs::Entity& /*entity*/,
                                          components::DensityFieldState& comp,
                                          float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool DensityFieldSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DensityFieldState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Voxel operations
// ---------------------------------------------------------------------------

bool DensityFieldSystem::set_voxel(const std::string& entity_id,
                                    int x, int y, int z, float density) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    // Clamp density to [0,1]
    density = std::max(0.0f, std::min(1.0f, density));

    // Update existing voxel if present
    for (auto& v : comp->voxels) {
        if (v.x == x && v.y == y && v.z == z) {
            v.density = density;
            comp->total_updates++;
            return true;
        }
    }

    // Add new voxel
    if (static_cast<int>(comp->voxels.size()) >= comp->max_voxels) return false;

    components::DensityFieldState::Voxel vox;
    vox.x       = x;
    vox.y       = y;
    vox.z       = z;
    vox.density = density;
    comp->voxels.push_back(vox);
    comp->total_updates++;
    return true;
}

float DensityFieldSystem::get_voxel(const std::string& entity_id,
                                     int x, int y, int z) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;

    for (const auto& v : comp->voxels) {
        if (v.x == x && v.y == y && v.z == z) return v.density;
    }
    return 0.0f;
}

bool DensityFieldSystem::clear_field(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->voxels.clear();
    return true;
}

bool DensityFieldSystem::apply_smooth(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->voxels.empty()) return true;

    // Build a copy of densities, then average each with its neighbors
    std::vector<float> smoothed(comp->voxels.size());

    for (size_t i = 0; i < comp->voxels.size(); ++i) {
        float sum   = comp->voxels[i].density;
        int   count = 1;

        for (size_t j = 0; j < comp->voxels.size(); ++j) {
            if (i == j) continue;
            int dx = std::abs(comp->voxels[j].x - comp->voxels[i].x);
            int dy = std::abs(comp->voxels[j].y - comp->voxels[i].y);
            int dz = std::abs(comp->voxels[j].z - comp->voxels[i].z);
            if (dx <= 1 && dy <= 1 && dz <= 1) {
                sum += comp->voxels[j].density;
                count++;
            }
        }
        smoothed[i] = sum / static_cast<float>(count);
    }

    for (size_t i = 0; i < comp->voxels.size(); ++i) {
        comp->voxels[i].density = smoothed[i];
    }
    comp->total_updates++;
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool DensityFieldSystem::set_iso_value(const std::string& entity_id,
                                        float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->iso_value = value;
    return true;
}

bool DensityFieldSystem::set_symmetry(const std::string& entity_id,
                                       bool x, bool y, bool z) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->symmetry_x = x;
    comp->symmetry_y = y;
    comp->symmetry_z = z;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int DensityFieldSystem::get_voxel_count(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->voxels.size());
}

float DensityFieldSystem::get_iso_value(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->iso_value;
}

bool DensityFieldSystem::is_symmetric_x(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->symmetry_x;
}

int DensityFieldSystem::get_total_updates(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_updates;
}

} // namespace systems
} // namespace atlas
