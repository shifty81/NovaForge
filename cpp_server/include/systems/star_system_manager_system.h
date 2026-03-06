#ifndef NOVAFORGE_SYSTEMS_STAR_SYSTEM_MANAGER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STAR_SYSTEM_MANAGER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Star system manager for vertical slice orchestration
 *
 * Manages a complete star system with celestial bodies, stations, gates,
 * belts, and NPC presence. Enables the full fly/fight/mine/trade/dock loop
 * in one star system end-to-end.
 */
class StarSystemManagerSystem : public ecs::SingleComponentSystem<components::StarSystemState> {
public:
    explicit StarSystemManagerSystem(ecs::World* world);
    ~StarSystemManagerSystem() override = default;

    std::string getName() const override { return "StarSystemManagerSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& system_name, float security);

    // Celestial management
    bool addCelestial(const std::string& entity_id, const std::string& body_id,
                      const std::string& name, const std::string& type,
                      float x, float y, float z, float radius);
    bool removeCelestial(const std::string& entity_id, const std::string& body_id);
    int getCelestialCount(const std::string& entity_id) const;

    // Station management
    bool addStation(const std::string& entity_id, const std::string& station_id,
                    const std::string& name, const std::string& faction,
                    float x, float y, float z);
    bool removeStation(const std::string& entity_id, const std::string& station_id);
    bool dockAtStation(const std::string& entity_id, const std::string& station_id);
    bool undockFromStation(const std::string& entity_id, const std::string& station_id);
    int getStationDockedCount(const std::string& entity_id, const std::string& station_id) const;
    int getStationCount(const std::string& entity_id) const;

    // Gate management
    bool addGate(const std::string& entity_id, const std::string& gate_id,
                 const std::string& destination, float x, float y, float z);
    bool useGate(const std::string& entity_id, const std::string& gate_id);
    int getGateCount(const std::string& entity_id) const;
    int getTotalJumps(const std::string& entity_id) const;

    // NPC presence
    bool addNPCPresence(const std::string& entity_id, const std::string& faction,
                        int ship_count, float threat_level, bool hostile);
    bool removeNPCPresence(const std::string& entity_id, const std::string& faction);
    int getNPCFactionCount(const std::string& entity_id) const;
    int getTotalNPCShips(const std::string& entity_id) const;

    // Queries
    float getSecurityStatus(const std::string& entity_id) const;
    int getTotalDockings(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::StarSystemState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STAR_SYSTEM_MANAGER_SYSTEM_H
