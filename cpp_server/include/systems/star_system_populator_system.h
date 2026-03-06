#ifndef NOVAFORGE_SYSTEMS_STAR_SYSTEM_POPULATOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STAR_SYSTEM_POPULATOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Seeds a star system with content for the vertical slice
 *
 * Populates a star system entity with stations, asteroid belts, NPC
 * factions, jump gates, and points of interest required for end-to-end
 * gameplay: undock → fly → mine → sell → fit → fight → warp.
 */
class StarSystemPopulatorSystem : public ecs::SingleComponentSystem<components::StarSystemPopulation> {
public:
    explicit StarSystemPopulatorSystem(ecs::World* world);
    ~StarSystemPopulatorSystem() override = default;

    std::string getName() const override { return "StarSystemPopulatorSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& system_id,
                    const std::string& system_name, float security);

    // Station seeding
    bool addStation(const std::string& entity_id, const std::string& station_id,
                    const std::string& station_name, const std::string& station_type);
    int getStationCount(const std::string& entity_id) const;
    bool hasStation(const std::string& entity_id, const std::string& station_id) const;

    // Asteroid belt seeding
    bool addAsteroidBelt(const std::string& entity_id, const std::string& belt_id,
                         const std::string& ore_type, int richness);
    int getAsteroidBeltCount(const std::string& entity_id) const;
    bool hasAsteroidBelt(const std::string& entity_id, const std::string& belt_id) const;

    // NPC faction seeding
    bool addNPCFaction(const std::string& entity_id, const std::string& faction_id,
                       const std::string& faction_name, int spawn_count);
    int getNPCFactionCount(const std::string& entity_id) const;
    bool hasNPCFaction(const std::string& entity_id, const std::string& faction_id) const;

    // Jump gate seeding
    bool addJumpGate(const std::string& entity_id, const std::string& gate_id,
                     const std::string& destination_system);
    int getJumpGateCount(const std::string& entity_id) const;
    bool hasJumpGate(const std::string& entity_id, const std::string& gate_id) const;

    // Point of interest seeding
    bool addPointOfInterest(const std::string& entity_id, const std::string& poi_id,
                            const std::string& poi_type, const std::string& description);
    int getPointOfInterestCount(const std::string& entity_id) const;
    bool hasPointOfInterest(const std::string& entity_id, const std::string& poi_id) const;

    // Queries
    bool isPopulated(const std::string& entity_id) const;
    bool markPopulated(const std::string& entity_id, float timestamp);
    float getPopulationTime(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::StarSystemPopulation& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STAR_SYSTEM_POPULATOR_SYSTEM_H
