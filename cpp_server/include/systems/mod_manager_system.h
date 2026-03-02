#ifndef NOVAFORGE_SYSTEMS_MOD_MANAGER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MOD_MANAGER_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Mod manager utility for installing, enabling, and ordering mods
 *
 * Manages a collection of mods with dependency tracking, conflict detection,
 * and automatic load-order calculation.
 */
class ModManagerSystem : public ecs::System {
public:
    explicit ModManagerSystem(ecs::World* world);
    ~ModManagerSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ModManagerSystem"; }

    bool createManager(const std::string& entity_id);
    bool installMod(const std::string& entity_id, const std::string& mod_id,
                    const std::string& name, const std::string& version,
                    const std::string& author);
    bool uninstallMod(const std::string& entity_id, const std::string& mod_id);
    bool enableMod(const std::string& entity_id, const std::string& mod_id);
    bool disableMod(const std::string& entity_id, const std::string& mod_id);
    bool addDependency(const std::string& entity_id, const std::string& mod_id,
                       const std::string& depends_on);
    bool addConflict(const std::string& entity_id, const std::string& mod_id_a,
                     const std::string& mod_id_b);
    bool hasConflict(const std::string& entity_id, const std::string& mod_id) const;
    int getModCount(const std::string& entity_id) const;
    int getEnabledCount(const std::string& entity_id) const;
    bool isInstalled(const std::string& entity_id, const std::string& mod_id) const;
    std::vector<std::string> getLoadOrder(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MOD_MANAGER_SYSTEM_H
