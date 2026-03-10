#ifndef NOVAFORGE_SYSTEMS_META_LEVEL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_META_LEVEL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Module meta level management system
 *
 * Manages per-fitting module meta level data.  Meta levels classify
 * modules from Tech I (0) through Named variants (1-4), Tech II (5),
 * and Faction / Deadspace / Officer (6+).  The system tracks stat,
 * CPU, and powergrid multipliers for each module and supports
 * upgrading modules between meta levels.
 */
class MetaLevelSystem : public ecs::SingleComponentSystem<components::MetaLevelState> {
public:
    explicit MetaLevelSystem(ecs::World* world);
    ~MetaLevelSystem() override = default;

    std::string getName() const override { return "MetaLevelSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& fitting_id = "");
    bool addModule(const std::string& entity_id, const std::string& module_id,
                   const std::string& base_type, int meta_level,
                   float stat_mult = 1.0f, float cpu_mult = 1.0f,
                   float pg_mult = 1.0f);
    bool removeModule(const std::string& entity_id, const std::string& module_id);
    bool upgradeModule(const std::string& entity_id, const std::string& module_id,
                       int new_meta_level);
    bool setDropRate(const std::string& entity_id, const std::string& module_id,
                     float drop_rate);

    int   getModuleCount(const std::string& entity_id) const;
    int   getMetaLevel(const std::string& entity_id,
                       const std::string& module_id) const;
    float getAverageMetaLevel(const std::string& entity_id) const;
    float getStatMultiplier(const std::string& entity_id,
                            const std::string& module_id) const;
    float getCPUMultiplier(const std::string& entity_id,
                           const std::string& module_id) const;
    float getPowerGridMultiplier(const std::string& entity_id,
                                 const std::string& module_id) const;
    int   getTechIICount(const std::string& entity_id) const;
    int   getFactionCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::MetaLevelState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_META_LEVEL_SYSTEM_H
