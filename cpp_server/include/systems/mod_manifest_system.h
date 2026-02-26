#ifndef NOVAFORGE_SYSTEMS_MOD_MANIFEST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MOD_MANIFEST_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages mod manifests for data-driven modding support
 *
 * Handles mod.json manifest loading, validation, dependency ordering,
 * compatibility checking, and hot reload coordination. Mods are data-driven
 * (JSON files) and maintain deterministic simulation.
 */
class ModManifestSystem : public ecs::System {
public:
    explicit ModManifestSystem(ecs::World* world);
    ~ModManifestSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ModManifestSystem"; }

    /**
     * @brief Register a mod manifest
     * @return true if manifest is valid and registered
     */
    bool registerMod(const std::string& mod_id, const std::string& name,
                     const std::string& version, const std::string& author,
                     const std::vector<std::string>& dependencies = {});

    /**
     * @brief Unregister a mod
     */
    bool unregisterMod(const std::string& mod_id);

    /**
     * @brief Check if a mod is registered
     */
    bool isModRegistered(const std::string& mod_id) const;

    /**
     * @brief Get the number of registered mods
     */
    int getModCount() const;

    /**
     * @brief Validate all mods (check dependency resolution, version conflicts)
     * @return true if all mods are valid
     */
    bool validateAll() const;

    /**
     * @brief Check if a specific mod has all dependencies satisfied
     */
    bool areDependenciesMet(const std::string& mod_id) const;

    /**
     * @brief Get the load order (topological sort by dependencies)
     * @return Ordered list of mod_ids, or empty if circular dependency detected
     */
    std::vector<std::string> getLoadOrder() const;

    /**
     * @brief Enable or disable a mod
     */
    bool setModEnabled(const std::string& mod_id, bool enabled);

    /**
     * @brief Check if a mod is enabled
     */
    bool isModEnabled(const std::string& mod_id) const;

    /**
     * @brief Get count of enabled mods
     */
    int getEnabledModCount() const;

    /**
     * @brief Get the mod version string
     */
    std::string getModVersion(const std::string& mod_id) const;

private:
    /// @brief Get or create the singleton ModRegistry entity
    components::ModRegistry* getOrCreateRegistry();
    const components::ModRegistry* getRegistry() const;

    static constexpr const char* REGISTRY_ENTITY_ID = "__mod_registry__";
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MOD_MANIFEST_SYSTEM_H
