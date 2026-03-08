#ifndef NOVAFORGE_SYSTEMS_SHIP_LOADOUT_PRESET_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_LOADOUT_PRESET_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Save/load ship fitting presets for quick loadout swaps
 *
 * Manages named presets of module configurations that players can save
 * and apply to ships. Each preset stores a list of modules with slot
 * assignments. Supports rename, delete, and slot-count validation.
 */
class ShipLoadoutPresetSystem : public ecs::SingleComponentSystem<components::ShipLoadoutPresets> {
public:
    explicit ShipLoadoutPresetSystem(ecs::World* world);
    ~ShipLoadoutPresetSystem() override = default;

    std::string getName() const override { return "ShipLoadoutPresetSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool savePreset(const std::string& entity_id, const std::string& preset_name,
                    const std::string& ship_type);
    bool addModuleToPreset(const std::string& entity_id, const std::string& preset_name,
                           const std::string& module_name, const std::string& slot);
    bool removePreset(const std::string& entity_id, const std::string& preset_name);
    bool renamePreset(const std::string& entity_id, const std::string& old_name,
                      const std::string& new_name);
    int getPresetCount(const std::string& entity_id) const;
    int getModuleCount(const std::string& entity_id, const std::string& preset_name) const;
    bool hasPreset(const std::string& entity_id, const std::string& preset_name) const;
    std::string getPresetShipType(const std::string& entity_id,
                                   const std::string& preset_name) const;
    int getTotalPresetsSaved(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ShipLoadoutPresets& slp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_LOADOUT_PRESET_SYSTEM_H
