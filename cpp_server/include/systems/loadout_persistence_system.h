#ifndef NOVAFORGE_SYSTEMS_LOADOUT_PERSISTENCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_LOADOUT_PERSISTENCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Save/restore ship fittings between sessions
 *
 * Manages named loadout slots with module configurations.
 * Supports save, load, delete, rename, and quick-swap operations.
 */
class LoadoutPersistenceSystem : public ecs::SingleComponentSystem<components::SavedLoadout> {
public:
    explicit LoadoutPersistenceSystem(ecs::World* world);
    ~LoadoutPersistenceSystem() override = default;

    std::string getName() const override { return "LoadoutPersistenceSystem"; }

    bool saveLoadout(const std::string& entity_id, const std::string& loadout_id,
                     const std::string& loadout_name, const std::string& ship_class);
    bool addModuleToLoadout(const std::string& entity_id, const std::string& loadout_id,
                            const std::string& module_id, const std::string& module_name,
                            int slot_index, const std::string& slot_type);
    bool deleteLoadout(const std::string& entity_id, const std::string& loadout_id);
    bool renameLoadout(const std::string& entity_id, const std::string& loadout_id,
                       const std::string& new_name);
    bool setActiveLoadout(const std::string& entity_id, const std::string& loadout_id);
    std::string getActiveLoadoutId(const std::string& entity_id) const;
    int getLoadoutCount(const std::string& entity_id) const;
    int getModuleCount(const std::string& entity_id, const std::string& loadout_id) const;
    std::string getLoadoutName(const std::string& entity_id, const std::string& loadout_id) const;
    std::string getLoadoutShipClass(const std::string& entity_id, const std::string& loadout_id) const;
    bool hasLoadout(const std::string& entity_id, const std::string& loadout_id) const;
    bool hasModule(const std::string& entity_id, const std::string& loadout_id,
                   const std::string& module_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SavedLoadout& loadout, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_LOADOUT_PERSISTENCE_SYSTEM_H
