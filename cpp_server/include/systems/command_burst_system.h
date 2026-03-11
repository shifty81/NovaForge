#ifndef NOVAFORGE_SYSTEMS_COMMAND_BURST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COMMAND_BURST_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Fleet command burst module system.
 *
 * Command burst modules are fitted to command ships and project a fleet-wide
 * stat boost within a configurable radius.  Each burst has a type (Shield,
 * Armor, Navigation, Sensor, Mining), a strength multiplier, and a cycle
 * time.  Bursts are activated manually and deactivate after one cycle.
 * Multiple burst types can be fitted simultaneously up to max_bursts.
 */
class CommandBurstSystem
    : public ecs::SingleComponentSystem<components::CommandBurstState> {
public:
    explicit CommandBurstSystem(ecs::World* world);
    ~CommandBurstSystem() override = default;

    std::string getName() const override { return "CommandBurstSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& commander_id);

    // --- Burst management ---
    bool addBurst(const std::string& entity_id,
                  const std::string& burst_id,
                  components::CommandBurstState::BurstType type,
                  float strength,
                  float radius,
                  float cycle_time);
    bool removeBurst(const std::string& entity_id,
                     const std::string& burst_id);
    bool activateBurst(const std::string& entity_id,
                       const std::string& burst_id);
    bool deactivateBurst(const std::string& entity_id,
                         const std::string& burst_id);

    // --- Queries ---
    int  getBurstCount(const std::string& entity_id) const;
    int  getActiveBurstCount(const std::string& entity_id) const;
    bool hasBurst(const std::string& entity_id,
                  const std::string& burst_id) const;
    bool isBurstActive(const std::string& entity_id,
                       const std::string& burst_id) const;
    bool isAnyActive(const std::string& entity_id) const;
    int  getTotalActivations(const std::string& entity_id) const;
    int  getTotalCycles(const std::string& entity_id) const;
    std::string getCommanderId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CommandBurstState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COMMAND_BURST_SYSTEM_H
