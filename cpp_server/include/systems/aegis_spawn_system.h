#ifndef NOVAFORGE_SYSTEMS_AEGIS_SPAWN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_AEGIS_SPAWN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief AEGIS security force spawning system
 *
 * Spawns CONCORD-like NPC response fleets when criminal activity
 * is detected in high-security space.  Response time scales inversely
 * with system security level.  Squads warp in, engage the criminal,
 * and withdraw once the engagement timer expires.
 */
class AegisSpawnSystem : public ecs::SingleComponentSystem<components::AegisSpawnState> {
public:
    explicit AegisSpawnSystem(ecs::World* world);
    ~AegisSpawnSystem() override = default;

    std::string getName() const override { return "AegisSpawnSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& system_id = "",
                    float security_level = 1.0f);
    bool reportCriminal(const std::string& entity_id, const std::string& criminal_id,
                        int ship_count = 3, float dps_per_ship = 200.0f);
    bool withdrawSquad(const std::string& entity_id, const std::string& squad_id);
    bool setSecurityLevel(const std::string& entity_id, float level);

    int  getSquadCount(const std::string& entity_id) const;
    int  getActiveSquadCount(const std::string& entity_id) const;
    int  getTotalDispatched(const std::string& entity_id) const;
    int  getTotalKills(const std::string& entity_id) const;
    float getSecurityLevel(const std::string& entity_id) const;
    float getResponseTime(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AegisSpawnState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_AEGIS_SPAWN_SYSTEM_H
