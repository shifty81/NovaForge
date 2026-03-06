#include "systems/spatial_partition_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

SpatialPartitionSystem::SpatialPartitionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SpatialPartitionSystem::computeCell(float x, float y, float z, float cell_size,
    int& cx, int& cy, int& cz) const {
    cx = static_cast<int>(std::floor(x / cell_size));
    cy = static_cast<int>(std::floor(y / cell_size));
    cz = static_cast<int>(std::floor(z / cell_size));
}

void SpatialPartitionSystem::updateComponent(ecs::Entity& entity,
    components::SpatialPartition& sp, float delta_time) {
    if (!sp.active) return;

    sp.rebuild_timer += delta_time;
    if (sp.rebuild_timer >= sp.rebuild_interval) {
        sp.rebuild_timer -= sp.rebuild_interval;
        // Recompute cell assignments for all entries
        for (auto& entry : sp.entries) {
            computeCell(entry.x, entry.y, entry.z, sp.cell_size,
                        entry.cell_x, entry.cell_y, entry.cell_z);
        }
        sp.total_rebuilds++;
    }
}

bool SpatialPartitionSystem::initialize(const std::string& entity_id, float cell_size) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SpatialPartition>();
    comp->cell_size = std::max(1.0f, cell_size);
    entity->addComponent(std::move(comp));
    return true;
}

bool SpatialPartitionSystem::insertEntity(const std::string& partition_id,
    const std::string& entity_id, float x, float y, float z) {
    auto* sp = getComponentFor(partition_id);
    if (!sp) return false;
    if (static_cast<int>(sp->entries.size()) >= sp->max_entries) return false;

    for (const auto& e : sp->entries) {
        if (e.entity_id == entity_id) return false;
    }

    components::SpatialPartition::GridEntry entry;
    entry.entity_id = entity_id;
    entry.x = x;
    entry.y = y;
    entry.z = z;
    computeCell(x, y, z, sp->cell_size, entry.cell_x, entry.cell_y, entry.cell_z);
    sp->entries.push_back(entry);
    sp->total_inserts++;
    return true;
}

bool SpatialPartitionSystem::removeEntity(const std::string& partition_id,
    const std::string& entity_id) {
    auto* sp = getComponentFor(partition_id);
    if (!sp) return false;

    auto it = std::find_if(sp->entries.begin(), sp->entries.end(),
        [&](const components::SpatialPartition::GridEntry& e) {
            return e.entity_id == entity_id;
        });
    if (it == sp->entries.end()) return false;
    sp->entries.erase(it);
    sp->total_removals++;
    return true;
}

bool SpatialPartitionSystem::updatePosition(const std::string& partition_id,
    const std::string& entity_id, float x, float y, float z) {
    auto* sp = getComponentFor(partition_id);
    if (!sp) return false;

    for (auto& entry : sp->entries) {
        if (entry.entity_id == entity_id) {
            entry.x = x;
            entry.y = y;
            entry.z = z;
            computeCell(x, y, z, sp->cell_size, entry.cell_x, entry.cell_y, entry.cell_z);
            return true;
        }
    }
    return false;
}

int SpatialPartitionSystem::getEntityCount(const std::string& partition_id) const {
    auto* sp = getComponentFor(partition_id);
    return sp ? static_cast<int>(sp->entries.size()) : 0;
}

std::vector<std::string> SpatialPartitionSystem::getEntitiesInCell(
    const std::string& partition_id, int cell_x, int cell_y, int cell_z) const {
    std::vector<std::string> result;
    auto* sp = getComponentFor(partition_id);
    if (!sp) return result;

    const_cast<components::SpatialPartition*>(sp)->total_queries++;
    for (const auto& entry : sp->entries) {
        if (entry.cell_x == cell_x && entry.cell_y == cell_y && entry.cell_z == cell_z) {
            result.push_back(entry.entity_id);
        }
    }
    return result;
}

std::vector<std::string> SpatialPartitionSystem::getEntitiesInRadius(
    const std::string& partition_id, float x, float y, float z, float radius) const {
    std::vector<std::string> result;
    auto* sp = getComponentFor(partition_id);
    if (!sp) return result;

    const_cast<components::SpatialPartition*>(sp)->total_queries++;
    float radius_sq = radius * radius;
    for (const auto& entry : sp->entries) {
        float dx = entry.x - x;
        float dy = entry.y - y;
        float dz = entry.z - z;
        float dist_sq = dx * dx + dy * dy + dz * dz;
        if (dist_sq <= radius_sq) {
            result.push_back(entry.entity_id);
        }
    }
    return result;
}

int SpatialPartitionSystem::getEntitiesInRadiusCount(const std::string& partition_id,
    float x, float y, float z, float radius) const {
    return static_cast<int>(getEntitiesInRadius(partition_id, x, y, z, radius).size());
}

int SpatialPartitionSystem::getCellEntityCount(const std::string& partition_id,
    int cell_x, int cell_y, int cell_z) const {
    return static_cast<int>(getEntitiesInCell(partition_id, cell_x, cell_y, cell_z).size());
}

int SpatialPartitionSystem::getTotalQueries(const std::string& partition_id) const {
    auto* sp = getComponentFor(partition_id);
    return sp ? sp->total_queries : 0;
}

int SpatialPartitionSystem::getTotalRebuilds(const std::string& partition_id) const {
    auto* sp = getComponentFor(partition_id);
    return sp ? sp->total_rebuilds : 0;
}

} // namespace systems
} // namespace atlas
