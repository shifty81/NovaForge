#ifndef NOVAFORGE_SYSTEMS_PLAYER_STANDING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_STANDING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Aggregate faction standings and compute reputation ranks
 *
 * Tracks per-faction standings, computes rank thresholds,
 * and generates notifications when rank changes occur.
 */
class PlayerStandingSystem : public ecs::SingleComponentSystem<components::PlayerStanding> {
public:
    explicit PlayerStandingSystem(ecs::World* world);
    ~PlayerStandingSystem() override = default;

    std::string getName() const override { return "PlayerStandingSystem"; }

    bool addFaction(const std::string& entity_id, const std::string& faction_id,
                    const std::string& faction_name);
    bool modifyStanding(const std::string& entity_id, const std::string& faction_id,
                        double delta);
    double getStanding(const std::string& entity_id, const std::string& faction_id) const;
    int getRank(const std::string& entity_id, const std::string& faction_id) const;
    int getFactionCount(const std::string& entity_id) const;
    int getNotificationCount(const std::string& entity_id) const;
    bool hasFaction(const std::string& entity_id, const std::string& faction_id) const;
    std::string getRankName(int rank) const;

protected:
    void updateComponent(ecs::Entity& entity, components::PlayerStanding& ps, float delta_time) override;

private:
    static int computeRank(double standing);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_STANDING_SYSTEM_H
