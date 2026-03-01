#ifndef NOVAFORGE_SYSTEMS_ENVIRONMENTAL_HAZARD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ENVIRONMENTAL_HAZARD_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Environmental hazard management for ship/station interiors
 *
 * Creates and manages hazards (hull breaches, fires, radiation, etc.)
 * that affect rooms in a ship interior.  Hazards damage FPS characters
 * in the affected room, can spread to adjacent rooms over time, and
 * can be repaired by player interaction.
 *
 * @note Current limitation: damage is applied to all FPS characters
 * sharing the same interior_id, not just those in the affected room,
 * because FPSCharacterState does not yet have room-level spatial
 * tracking.  A room-to-characters index should be added when
 * FPSCharacterState gains a current_room_id field.
 */
class EnvironmentalHazardSystem : public ecs::System {
public:
    explicit EnvironmentalHazardSystem(ecs::World* world);
    ~EnvironmentalHazardSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "EnvironmentalHazardSystem"; }

    // --- Setup ---

    /** Create a new hazard in a specific room */
    bool createHazard(const std::string& hazard_id,
                      const std::string& room_id,
                      const std::string& interior_id,
                      components::EnvironmentalHazard::HazardType type,
                      components::EnvironmentalHazard::Severity severity);

    /** Remove a hazard (fully repaired or extinguished) */
    bool removeHazard(const std::string& hazard_id);

    // --- Queries ---

    /** Get hazards affecting a specific room */
    std::vector<std::string> getHazardsInRoom(const std::string& room_id) const;

    /** Get all active hazards in an interior */
    std::vector<std::string> getActiveHazards(const std::string& interior_id) const;

    /** Check if a room is safe (no active hazards) */
    bool isRoomSafe(const std::string& room_id) const;

    /** Get total DPS in a room from all active hazards */
    float getRoomDPS(const std::string& room_id) const;

    // --- Actions ---

    /** Start repairing a hazard */
    bool startRepair(const std::string& hazard_id);

    /** Stop repairing a hazard */
    bool stopRepair(const std::string& hazard_id);

    /** Get repair progress (0.0 to 1.0) */
    float getRepairProgress(const std::string& hazard_id) const;

    // --- Helpers ---

    static std::string hazardTypeName(int type);
    static std::string severityName(int sev);

private:
    float getDPSForSeverity(int severity) const;
    float getSpreadIntervalForSeverity(int severity) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ENVIRONMENTAL_HAZARD_SYSTEM_H
