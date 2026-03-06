#ifndef NOVAFORGE_SYSTEMS_OVERVIEW_FILTER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_OVERVIEW_FILTER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Overview window entity filtering and column sorting
 *
 * Manages overview tab presets with type/distance/standing filters.
 * Supports column-based sorting and entry count limits.
 */
class OverviewFilterSystem : public ecs::SingleComponentSystem<components::OverviewFilter> {
public:
    explicit OverviewFilterSystem(ecs::World* world);
    ~OverviewFilterSystem() override = default;

    std::string getName() const override { return "OverviewFilterSystem"; }

    bool addPreset(const std::string& entity_id, const std::string& preset_id,
                   const std::string& name);
    bool removePreset(const std::string& entity_id, const std::string& preset_id);
    bool setActivePreset(const std::string& entity_id, const std::string& preset_id);
    bool addTypeToPreset(const std::string& entity_id, const std::string& preset_id,
                         const std::string& type);
    bool setPresetMaxDistance(const std::string& entity_id, const std::string& preset_id,
                             float max_distance);
    bool addEntry(const std::string& entity_id, const std::string& entry_entity_id,
                  const std::string& name, const std::string& type,
                  float distance, const std::string& standing);
    bool removeEntry(const std::string& entity_id, const std::string& entry_entity_id);
    int getEntryCount(const std::string& entity_id) const;
    int getPresetCount(const std::string& entity_id) const;
    bool setSortColumn(const std::string& entity_id, const std::string& column, bool ascending);
    int getFilteredEntryCount(const std::string& entity_id) const;
    int getTotalEntriesFiltered(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::OverviewFilter& overview, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_OVERVIEW_FILTER_SYSTEM_H
