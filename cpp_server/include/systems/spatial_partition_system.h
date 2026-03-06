#ifndef NOVAFORGE_SYSTEMS_SPATIAL_PARTITION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SPATIAL_PARTITION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Grid-based spatial partitioning system
 *
 * Divides 3D space into grid cells for O(1) neighbor lookups,
 * enabling efficient entity queries at 500+ entity scale.
 * Supports the client polish & optimization target.
 */
class SpatialPartitionSystem : public ecs::SingleComponentSystem<components::SpatialPartition> {
public:
    explicit SpatialPartitionSystem(ecs::World* world);
    ~SpatialPartitionSystem() override = default;

    std::string getName() const override { return "SpatialPartitionSystem"; }

public:
    bool initialize(const std::string& entity_id, float cell_size);

    // Entity management
    bool insertEntity(const std::string& partition_id, const std::string& entity_id,
                      float x, float y, float z);
    bool removeEntity(const std::string& partition_id, const std::string& entity_id);
    bool updatePosition(const std::string& partition_id, const std::string& entity_id,
                        float x, float y, float z);

    // Queries
    int getEntityCount(const std::string& partition_id) const;
    std::vector<std::string> getEntitiesInCell(const std::string& partition_id,
                                                int cell_x, int cell_y, int cell_z) const;
    std::vector<std::string> getEntitiesInRadius(const std::string& partition_id,
                                                  float x, float y, float z, float radius) const;
    int getEntitiesInRadiusCount(const std::string& partition_id,
                                  float x, float y, float z, float radius) const;
    int getCellEntityCount(const std::string& partition_id,
                           int cell_x, int cell_y, int cell_z) const;
    int getTotalQueries(const std::string& partition_id) const;
    int getTotalRebuilds(const std::string& partition_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SpatialPartition& sp, float delta_time) override;

private:
    void computeCell(float x, float y, float z, float cell_size,
                     int& cx, int& cy, int& cz) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SPATIAL_PARTITION_SYSTEM_H
