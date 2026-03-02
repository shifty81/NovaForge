#ifndef NOVAFORGE_SYSTEMS_DOCK_NODE_LAYOUT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DOCK_NODE_LAYOUT_SYSTEM_H

#include "ecs/system.h"
#include "components/ui_components.h"
#include <string>
#include <tuple>

namespace atlas {
namespace systems {

/**
 * @brief Dock node layout system (Phase 15)
 *
 * Retained-mode UI window framework with DockNode tree for managing
 * dockable window panels. Server-side layout calculation engine.
 */
class DockNodeLayoutSystem : public ecs::System {
public:
    explicit DockNodeLayoutSystem(ecs::World* world);
    ~DockNodeLayoutSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "DockNodeLayoutSystem"; }

    // Initialization
    bool initializeLayout(const std::string& entity_id, const std::string& owner_id,
                          float width, float height);

    // Window management
    bool addWindow(const std::string& entity_id, const std::string& window_id);
    bool removeWindow(const std::string& entity_id, const std::string& window_id);
    bool splitNode(const std::string& entity_id, const std::string& node_id,
                   components::DockNodeLayout::SplitDirection direction, float ratio);

    // Docking
    bool dockWindow(const std::string& entity_id, const std::string& window_id,
                    const std::string& target_node_id,
                    components::DockNodeLayout::SplitDirection direction);
    bool undockWindow(const std::string& entity_id, const std::string& window_id);

    // Query
    int getWindowCount(const std::string& entity_id) const;
    std::string getNodeType(const std::string& entity_id, const std::string& node_id) const;
    std::tuple<float, float, float, float> getWindowBounds(const std::string& entity_id,
                                                            const std::string& window_id) const;

private:
    void recalculateLayout(components::DockNodeLayout* layout,
                           components::DockNodeLayout::DockNode* node);
    std::string generateNodeId(components::DockNodeLayout* layout);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DOCK_NODE_LAYOUT_SYSTEM_H
