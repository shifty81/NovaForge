#ifndef NOVAFORGE_SYSTEMS_TECH2_MODULE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TECH2_MODULE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tech II / Faction / Deadspace module catalogue system
 *
 * Maintains a per-owner catalogue of advanced module variants.
 * Supports:
 *  - Registering Tech II modules (manufactured via Invention)
 *  - Recording Faction module drops (from faction NPCs)
 *  - Recording Deadspace module drops (from exploration sites)
 *  - Loot-table driven acquisition of faction/deadspace items
 */
class Tech2ModuleSystem
    : public ecs::SingleComponentSystem<components::Tech2ModuleState> {
public:
    using Category = components::Tech2ModuleState::ModuleCategory;

    explicit Tech2ModuleSystem(ecs::World* world);
    ~Tech2ModuleSystem() override = default;

    std::string getName() const override { return "Tech2ModuleSystem"; }

    // --- public API ---

    /** Attach a Tech2ModuleState component to an existing entity. */
    bool initialize(const std::string& entity_id, const std::string& owner_id);

    /** Register a module definition in the catalogue. */
    bool registerModule(const std::string& entity_id,
                        const std::string& module_id,
                        const std::string& base_module_id,
                        Category category,
                        int meta_level,
                        float stat_multiplier);

    /** Add a loot-table entry so the system can simulate drops. */
    bool addLootEntry(const std::string& entity_id,
                      const std::string& source_site,
                      const std::string& module_id,
                      float drop_chance);

    /**
     * Simulate a loot roll from a specific site type.
     * Returns the module_id that dropped, or "" if nothing dropped.
     * Uses a seeded random for deterministic testing.
     */
    std::string rollLoot(const std::string& entity_id,
                         const std::string& source_site,
                         float random_value_0_1);

    /** Grant a module directly (e.g. after a confirmed drop). */
    bool acquireModule(const std::string& entity_id, const std::string& module_id);

    int getModuleCount(const std::string& entity_id) const;
    int getOwnedCount(const std::string& entity_id,
                      const std::string& module_id) const;
    int getTotalTech2Acquired(const std::string& entity_id) const;
    int getTotalFactionAcquired(const std::string& entity_id) const;
    int getTotalDeadspaceAcquired(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::Tech2ModuleState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TECH2_MODULE_SYSTEM_H
