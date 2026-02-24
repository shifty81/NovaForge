#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ecs/ECS.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>

namespace atlas::editor {

class ECSInspectorPanel : public EditorPanel {
public:
    explicit ECSInspectorPanel(ecs::World& world) : m_world(world) {}

    const char* Name() const override { return "ECS Inspector"; }
    void Draw() override;

    ecs::EntityID SelectedEntity() const { return m_selectedEntity; }
    const std::string& SearchFilter() const { return m_searchFilter; }
    size_t EntityCountVisible() const;

    void SelectEntity(ecs::EntityID id);
    void SetSearchFilter(const std::string& filter);
    void DestroySelectedEntity();
    void ClearSelection();

private:
    ecs::World& m_world;
    ecs::EntityID m_selectedEntity = 0;
    std::string m_searchFilter;
    atlas::PanelState     m_panelState;
    atlas::TextInputState m_searchInput;
    float                 m_scrollOffset = 0.0f;
};

}
