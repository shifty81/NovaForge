#ifndef NOVAFORGE_SYSTEMS_DENSITY_FIELD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DENSITY_FIELD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Voxel density field for procedural hull shaping
 *
 * Provides a 3-D scalar field of density values that can be sculpted by
 * the player or procedural generation.  Supports iso-surface extraction
 * (marching-cubes style), per-axis symmetry, and simple smoothing.
 */
class DensityFieldSystem
    : public ecs::SingleComponentSystem<components::DensityFieldState> {
public:
    explicit DensityFieldSystem(ecs::World* world);
    ~DensityFieldSystem() override = default;

    std::string getName() const override { return "DensityFieldSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Voxel operations ---
    bool  set_voxel(const std::string& entity_id, int x, int y, int z,
                    float density);
    float get_voxel(const std::string& entity_id, int x, int y, int z) const;
    bool  clear_field(const std::string& entity_id);
    bool  apply_smooth(const std::string& entity_id);

    // --- Configuration ---
    bool set_iso_value(const std::string& entity_id, float value);
    bool set_symmetry(const std::string& entity_id, bool x, bool y, bool z);

    // --- Queries ---
    int   get_voxel_count(const std::string& entity_id) const;
    float get_iso_value(const std::string& entity_id) const;
    bool  is_symmetric_x(const std::string& entity_id) const;
    int   get_total_updates(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::DensityFieldState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DENSITY_FIELD_SYSTEM_H
