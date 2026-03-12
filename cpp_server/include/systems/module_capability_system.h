#ifndef NOVAFORGE_SYSTEMS_MODULE_CAPABILITY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MODULE_CAPABILITY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks capabilities provided by installed ship modules
 *
 * Each installed module can expose zero or more named capabilities
 * (e.g. "shield_boost", "ecm_jam", "mining_yield").  The system
 * aggregates capability strength across all modules for quick queries.
 */
class ModuleCapabilitySystem
    : public ecs::SingleComponentSystem<components::ModuleCapabilityState> {
public:
    explicit ModuleCapabilitySystem(ecs::World* world);
    ~ModuleCapabilitySystem() override = default;

    std::string getName() const override { return "ModuleCapabilitySystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Module management ---
    bool add_module(const std::string& entity_id,
                    const std::string& module_id,
                    const std::string& module_type,
                    int size);
    bool remove_module(const std::string& entity_id,
                       const std::string& module_id);
    bool clear_modules(const std::string& entity_id);

    // --- Capability management ---
    bool add_capability(const std::string& entity_id,
                        const std::string& module_id,
                        const std::string& capability_id,
                        const std::string& capability_type,
                        float strength);
    bool remove_capability(const std::string& entity_id,
                           const std::string& module_id,
                           const std::string& capability_id);
    bool set_capability_enabled(const std::string& entity_id,
                                const std::string& module_id,
                                const std::string& capability_id,
                                bool enabled);

    // --- Queries ---
    bool  has_capability_type(const std::string& entity_id,
                              const std::string& capability_type) const;
    float get_total_capability_strength(const std::string& entity_id,
                                        const std::string& capability_type) const;
    int   get_module_count(const std::string& entity_id) const;
    int   get_capability_count(const std::string& entity_id,
                               const std::string& module_id) const;
    bool  is_capability_enabled(const std::string& entity_id,
                                const std::string& module_id,
                                const std::string& capability_id) const;
    int   get_total_capabilities_registered(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::ModuleCapabilityState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MODULE_CAPABILITY_SYSTEM_H
