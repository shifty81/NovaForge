#include "ECSInspectorPanel.h"
#include <algorithm>
#include <sstream>

namespace atlas::editor {

static bool MatchesFilter(const ecs::World& world, ecs::EntityID id, const std::string& filter) {
    if (filter.empty()) return true;

    // Match against entity ID as string
    std::string idStr = std::to_string(id);
    if (idStr.find(filter) != std::string::npos) return true;

    // Match against component type names
    auto types = world.GetComponentTypes(id);
    for (const auto& t : types) {
        std::string name = t.name();
        if (name.find(filter) != std::string::npos) return true;
    }
    return false;
}

void ECSInspectorPanel::Draw() {
    // ATLAS_HEADLESS: state tracking only, no UI rendering
    auto entities = m_world.GetEntities();
    for (auto id : entities) {
        if (!MatchesFilter(m_world, id, m_searchFilter)) continue;
        // Visible entity — real rendering would happen here
        (void)id;
    }

    // Clear selection if the selected entity is no longer alive
    if (m_selectedEntity != 0 && !m_world.IsAlive(m_selectedEntity)) {
        m_selectedEntity = 0;
    }
}

void ECSInspectorPanel::SelectEntity(ecs::EntityID id) {
    if (m_world.IsAlive(id)) {
        m_selectedEntity = id;
    }
}

void ECSInspectorPanel::SetSearchFilter(const std::string& filter) {
    m_searchFilter = filter;
}

void ECSInspectorPanel::DestroySelectedEntity() {
    if (m_selectedEntity != 0) {
        m_world.DestroyEntity(m_selectedEntity);
        m_selectedEntity = 0;
    }
}

void ECSInspectorPanel::ClearSelection() {
    m_selectedEntity = 0;
}

size_t ECSInspectorPanel::EntityCountVisible() const {
    auto entities = m_world.GetEntities();
    if (m_searchFilter.empty()) return entities.size();

    size_t count = 0;
    for (auto id : entities) {
        if (MatchesFilter(m_world, id, m_searchFilter)) {
            ++count;
        }
    }
    return count;
}

}
