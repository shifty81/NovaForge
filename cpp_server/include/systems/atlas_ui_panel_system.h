#ifndef NOVAFORGE_SYSTEMS_ATLAS_UI_PANEL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ATLAS_UI_PANEL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Atlas UI panel system (Phase 15)
 *
 * Manages dockable UI panels (inventory, fitting, market, overview,
 * chat, drone) with item management, filtering, and sorting.
 */
class AtlasUIPanelSystem : public ecs::SingleComponentSystem<components::AtlasUIPanel> {
public:
    explicit AtlasUIPanelSystem(ecs::World* world);
    ~AtlasUIPanelSystem() override = default;

    std::string getName() const override { return "AtlasUIPanelSystem"; }

    // Initialization
    bool initializePanel(const std::string& entity_id, const std::string& owner_id,
                         components::AtlasUIPanel::PanelType type);

    // Panel state
    bool openPanel(const std::string& entity_id);
    bool closePanel(const std::string& entity_id);
    bool togglePanel(const std::string& entity_id);

    // Item management
    bool addItem(const std::string& entity_id, const std::string& item_id,
                 const std::string& name, int quantity, float value);
    bool removeItem(const std::string& entity_id, const std::string& item_id);

    // Filtering and sorting
    bool setFilter(const std::string& entity_id, const std::string& filter_text);
    bool setSort(const std::string& entity_id, const std::string& field, bool ascending);
    bool selectItem(const std::string& entity_id, int index);

    // Query
    std::string getPanelType(const std::string& entity_id) const;
    bool isOpen(const std::string& entity_id) const;
    int getItemCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::AtlasUIPanel& panel, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ATLAS_UI_PANEL_SYSTEM_H
