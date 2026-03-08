#ifndef NOVAFORGE_SYSTEMS_CAPACITOR_WARFARE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPACITOR_WARFARE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Energy neutralizer/nosferatu warfare system
 *
 * Manages capacitor warfare modules: neutralizers (drain target and
 * self equally) and nosferatu (drain target, transfer to self).
 * Tracks drain resistance, energy drained/received, and total
 * warfare statistics.  Key for the combat depth vertical slice.
 */
class CapacitorWarfareSystem : public ecs::SingleComponentSystem<components::CapacitorWarfareState> {
public:
    explicit CapacitorWarfareSystem(ecs::World* world);
    ~CapacitorWarfareSystem() override = default;

    std::string getName() const override { return "CapacitorWarfareSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id);

    // Module management
    bool addModule(const std::string& entity_id, const std::string& module_id,
                   const std::string& module_type, float drain_rate,
                   float optimal_range, float cycle_time);
    bool removeModule(const std::string& entity_id, const std::string& module_id);

    // Operations
    bool activateModule(const std::string& entity_id, const std::string& module_id,
                        const std::string& target_id);
    bool deactivateModule(const std::string& entity_id, const std::string& module_id);
    bool setDrainResistance(const std::string& entity_id, float resistance);

    // Queries
    int getModuleCount(const std::string& entity_id) const;
    int getActiveModuleCount(const std::string& entity_id) const;
    float getDrainResistance(const std::string& entity_id) const;
    float getTotalEnergyDrained(const std::string& entity_id) const;
    float getTotalEnergyReceived(const std::string& entity_id) const;
    float getTotalEnergyLost(const std::string& entity_id) const;
    int getTotalCyclesCompleted(const std::string& entity_id) const;
    int getTotalTargetsCapped(const std::string& entity_id) const;
    bool isModuleActive(const std::string& entity_id, const std::string& module_id) const;
    float getModuleCycleProgress(const std::string& entity_id,
                                  const std::string& module_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CapacitorWarfareState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPACITOR_WARFARE_SYSTEM_H
