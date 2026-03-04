#ifndef NOVAFORGE_SYSTEMS_RIG_LOCKER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_RIG_LOCKER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Rig locker system (Phase 13)
 *
 * Manages suit/rig preset saving, loading, and equipping.
 * Server-side preset management for the rig locker dressing room.
 */
class RigLockerSystem : public ecs::SingleComponentSystem<components::RigLockerPreset> {
public:
    explicit RigLockerSystem(ecs::World* world);
    ~RigLockerSystem() override = default;

    std::string getName() const override { return "RigLockerSystem"; }

    // Initialization
    bool initializeLocker(const std::string& entity_id, const std::string& owner_id);

    // Preset management
    bool savePreset(const std::string& entity_id, const std::string& name,
                    const std::vector<std::string>& module_ids);
    bool deletePreset(const std::string& entity_id, const std::string& preset_id);
    bool renamePreset(const std::string& entity_id, const std::string& preset_id,
                      const std::string& new_name);

    // Equip / favorite
    bool equipPreset(const std::string& entity_id, const std::string& preset_id);
    bool toggleFavorite(const std::string& entity_id, const std::string& preset_id);

    // Query
    int getPresetCount(const std::string& entity_id) const;
    std::string getActivePreset(const std::string& entity_id) const;
    int getFavoriteCount(const std::string& entity_id) const;
    float getPresetMass(const std::string& entity_id, const std::string& preset_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::RigLockerPreset& locker, float delta_time) override;

private:
    std::string generatePresetId(components::RigLockerPreset* locker);
    float calculateMass(const std::vector<std::string>& module_ids) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_RIG_LOCKER_SYSTEM_H
