#ifndef NOVAFORGE_SYSTEMS_ANCIENT_MODULE_DISCOVERY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ANCIENT_MODULE_DISCOVERY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ancient module discovery system (Phase 13)
 *
 * Manages finding repairable ancient tech in ruins. Handles scanning for
 * hidden modules, discovering them, and extracting them for repair.
 */
class AncientModuleDiscoverySystem : public ecs::SingleComponentSystem<components::AncientModuleDiscovery> {
public:
    explicit AncientModuleDiscoverySystem(ecs::World* world);
    ~AncientModuleDiscoverySystem() override = default;

    std::string getName() const override { return "AncientModuleDiscoverySystem"; }

    // Initialization
    bool initializeSite(const std::string& entity_id, const std::string& site_id,
                        const std::string& explorer_id, float scan_range = 50.0f);

    // Module management
    bool addHiddenModule(const std::string& entity_id, const std::string& module_id,
                         const std::string& tech_type, float repair_difficulty,
                         float extract_required, float estimated_value);

    // Scanning & extraction
    bool beginScan(const std::string& entity_id, const std::string& module_id);
    bool beginExtraction(const std::string& entity_id, const std::string& module_id);
    bool analyzeModule(const std::string& entity_id, const std::string& module_id);
    bool setActive(const std::string& entity_id, bool active);

    // Query
    int getModuleState(const std::string& entity_id, const std::string& module_id) const;
    int getDiscoveredCount(const std::string& entity_id) const;
    int getExtractedCount(const std::string& entity_id) const;
    int getTotalModules(const std::string& entity_id) const;
    float getScanRange(const std::string& entity_id) const;

    static std::string stateName(int state);

protected:
    void updateComponent(ecs::Entity& entity, components::AncientModuleDiscovery& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ANCIENT_MODULE_DISCOVERY_SYSTEM_H
