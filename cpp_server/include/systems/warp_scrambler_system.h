#ifndef NOVAFORGE_SYSTEMS_WARP_SCRAMBLER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WARP_SCRAMBLER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Warp interdiction system — scrambler and disruptor point management.
 *
 * Manages warp scramblers and disruptors applied to an entity.  Each module
 * contributes scramble points to the target; while total_scramble_points > 0
 * the entity cannot enter warp.  Warp scramblers (2 pts, short range ~9 km)
 * also disable MWD; warp disruptors (1 pt, long range ~24 km) block warp only.
 * Modules cycle independently; scramble-point total is recomputed on every
 * add/remove/clear.
 */
class WarpScramblerSystem
    : public ecs::SingleComponentSystem<components::WarpScramblerState> {
public:
    explicit WarpScramblerSystem(ecs::World* world);
    ~WarpScramblerSystem() override = default;

    std::string getName() const override { return "WarpScramblerSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Module management ---
    bool applyScrambler(const std::string& entity_id,
                        const std::string& scrambler_id,
                        const std::string& source_id,
                        int   scramble_points,
                        float optimal_range,
                        float cycle_time,
                        bool  is_scrambler = false);
    bool removeScrambler(const std::string& entity_id,
                         const std::string& scrambler_id);
    bool clearScramblers(const std::string& entity_id);

    // --- Queries ---
    int  getTotalScramblerPoints(const std::string& entity_id) const;
    bool isWarpScrambled(const std::string& entity_id) const;
    int  getScramblerCount(const std::string& entity_id) const;
    int  getActiveScramblerCount(const std::string& entity_id) const;
    int  getTotalScrambles(const std::string& entity_id) const;
    bool isMwdDisabled(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::WarpScramblerState& comp,
                         float delta_time) override;

private:
    void recomputePoints(components::WarpScramblerState& comp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WARP_SCRAMBLER_SYSTEM_H
