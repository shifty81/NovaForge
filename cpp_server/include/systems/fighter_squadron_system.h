#ifndef NOVAFORGE_SYSTEMS_FIGHTER_SQUADRON_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FIGHTER_SQUADRON_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Carrier fighter squadron management system
 *
 * Manages fighter squadrons launched from carriers and supercarriers.  Each
 * squadron has a type (Light / Support / Heavy), health, ammo, and launch
 * state.  Launching a squadron marks it active; recalling returns it to the
 * bay.  recordKill increments per-squadron and global kill counters.
 * applyDamage reduces health and auto-recalls at 0 HP.  resupplyAmmo and
 * repairSquadron restore ammo/health for non-launched squadrons.
 * max_squadrons (default 5) caps active squadrons in space.
 */
class FighterSquadronSystem
    : public ecs::SingleComponentSystem<components::FighterSquadronState> {
public:
    explicit FighterSquadronSystem(ecs::World* world);
    ~FighterSquadronSystem() override = default;

    std::string getName() const override { return "FighterSquadronSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Squadron management ---
    bool addSquadron(const std::string& entity_id,
                     const std::string& squadron_id,
                     const std::string& name,
                     components::FighterSquadronState::SquadronType type,
                     int max_health = 100,
                     int max_ammo   = 100);
    bool removeSquadron(const std::string& entity_id,
                        const std::string& squadron_id);
    bool clearSquadrons(const std::string& entity_id);

    // --- Launch / Recall ---
    bool launchSquadron(const std::string& entity_id,
                        const std::string& squadron_id);
    bool recallSquadron(const std::string& entity_id,
                        const std::string& squadron_id);
    bool recallAll(const std::string& entity_id);

    // --- Combat ---
    bool recordKill(const std::string& entity_id,
                    const std::string& squadron_id);
    bool applyDamage(const std::string& entity_id,
                     const std::string& squadron_id,
                     int damage);

    // --- Maintenance ---
    bool resupplyAmmo(const std::string& entity_id,
                      const std::string& squadron_id,
                      int amount);
    bool repairSquadron(const std::string& entity_id,
                        const std::string& squadron_id,
                        int amount);

    // --- Queries ---
    int  getSquadronCount(const std::string& entity_id) const;
    int  getLaunchedCount(const std::string& entity_id) const;
    bool isLaunched(const std::string& entity_id,
                    const std::string& squadron_id) const;
    int  getHealth(const std::string& entity_id,
                   const std::string& squadron_id) const;
    int  getAmmo(const std::string& entity_id,
                 const std::string& squadron_id) const;
    int  getTotalLaunched(const std::string& entity_id) const;
    int  getTotalRecalled(const std::string& entity_id) const;
    int  getTotalKills(const std::string& entity_id) const;
    bool hasSquadron(const std::string& entity_id,
                     const std::string& squadron_id) const;
    int  getCountByType(const std::string& entity_id,
                        components::FighterSquadronState::SquadronType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FighterSquadronState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FIGHTER_SQUADRON_SYSTEM_H
