#ifndef NOVAFORGE_SYSTEMS_GRID_CONSTRUCTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_GRID_CONSTRUCTION_SYSTEM_H

#include "ecs/system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Grid construction system (Phase 14)
 *
 * Manages snappable grid-based construction for habitats with power
 * networking, structural integrity, and module placement.
 */
class GridConstructionSystem : public ecs::System {
public:
    explicit GridConstructionSystem(ecs::World* world);
    ~GridConstructionSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "GridConstructionSystem"; }

    // Initialization
    bool initializeGrid(const std::string& entity_id, const std::string& owner_id,
                        int width, int height);

    // Module operations
    bool placeModule(const std::string& entity_id, int x, int y,
                     components::GridConstruction::ModuleType module_type);
    bool removeModule(const std::string& entity_id, int x, int y);

    // Query
    std::string getModuleAt(const std::string& entity_id, int x, int y) const;
    float getModuleHealth(const std::string& entity_id, int x, int y) const;
    int getModuleCount(const std::string& entity_id) const;
    int getPoweredCount(const std::string& entity_id) const;
    int getGridWidth(const std::string& entity_id) const;
    int getGridHeight(const std::string& entity_id) const;

    // Module health
    bool damageModule(const std::string& entity_id, int x, int y, float amount);
    bool repairModule(const std::string& entity_id, int x, int y, float amount);

    // Power and integrity
    float calculateIntegrity(const std::string& entity_id);
    float calculatePower(const std::string& entity_id);
    bool setPowerEnabled(const std::string& entity_id, bool enabled);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_GRID_CONSTRUCTION_SYSTEM_H
