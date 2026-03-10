#ifndef NOVAFORGE_SYSTEMS_CITADEL_MANAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CITADEL_MANAGEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Player-owned Upwell structure (citadel) management
 *
 * Manages citadels of three sizes (Astrahus / Fortizar / Keepstar).
 * Each structure has services that consume fuel, a vulnerability
 * window during which it can be attacked, and a reinforcement timer
 * that delays destruction.  Fuel consumption is computed per-tick
 * based on active services; when fuel runs out, all services go
 * offline.  Taking damage during the vulnerability window triggers
 * reinforcement.
 */
class CitadelManagementSystem
    : public ecs::SingleComponentSystem<components::CitadelState> {
public:
    explicit CitadelManagementSystem(ecs::World* world);
    ~CitadelManagementSystem() override = default;

    std::string getName() const override { return "CitadelManagementSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& owner_corp_id,
                    const std::string& structure_name,
                    components::CitadelState::CitadelType type);

    // --- Ownership ---
    bool setOwner(const std::string& entity_id,
                  const std::string& new_owner_corp_id);

    // --- Services ---
    bool addService(const std::string& entity_id,
                    const std::string& service_id,
                    const std::string& service_name,
                    float fuel_per_hour);
    bool removeService(const std::string& entity_id,
                       const std::string& service_id);
    bool setServiceOnline(const std::string& entity_id,
                          const std::string& service_id, bool online);

    // --- Fuel ---
    bool addFuel(const std::string& entity_id, float amount);

    // --- Structure state ---
    bool setVulnerable(const std::string& entity_id);
    bool triggerReinforcement(const std::string& entity_id);
    bool repairStructure(const std::string& entity_id);
    bool applyDamage(const std::string& entity_id, float damage);

    // --- Queries ---
    components::CitadelState::StructureState
        getState(const std::string& entity_id) const;
    float getFuelRemaining(const std::string& entity_id) const;
    int   getActiveServiceCount(const std::string& entity_id) const;
    int   getServiceCount(const std::string& entity_id) const;
    float getShieldHp(const std::string& entity_id) const;
    float getArmorHp(const std::string& entity_id) const;
    float getHullHp(const std::string& entity_id) const;
    int   getTotalReinforcements(const std::string& entity_id) const;
    float getReinforcementTimer(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CitadelState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CITADEL_MANAGEMENT_SYSTEM_H
