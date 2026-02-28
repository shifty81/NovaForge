#include "ECSInspectorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
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

static constexpr float kDefaultPanelWidth  = 260.0f;
static constexpr float kDefaultPanelX      = 20.0f;
static constexpr float kDefaultPanelY      = 20.0f;
static constexpr float kDefaultPanelHeight = 400.0f;
static constexpr float kListViewHeight     = 200.0f;
static constexpr float kScrollbarWidth     = 8.0f;
static constexpr float kScrollbarPad       = 10.0f;  // offset from right edge

void ECSInspectorPanel::Draw() {
    atlas::AtlasContext* ctx = GetContext();

    if (ctx) {
        using namespace atlas;
        const float rowH = ctx->theme().rowHeight;
        const float panelW = m_panelState.bounds.w > 0
                                 ? m_panelState.bounds.w
                                 : kDefaultPanelWidth;

        if (m_panelState.bounds.w == 0) {
            m_panelState.bounds = {kDefaultPanelX, kDefaultPanelY,
                                   panelW, kDefaultPanelHeight};
        }

        if (panelBeginStateful(*ctx, "ECS Inspector", m_panelState)) {
            float x = m_panelState.bounds.x + 4;
            float y = m_panelState.bounds.y + 28;
            float contentW = panelW - 8;

            // Search text input
            Rect searchRect{x, y, contentW, rowH + 4};
            if (textInput(*ctx, "##search", searchRect, m_searchInput, "Search entities...")) {
                m_searchFilter = m_searchInput.text;
            }
            y += rowH + 8;

            separator(*ctx, {x, y}, contentW);
            y += 4;

            // Scrollable entity list
            auto entities = m_world.GetEntities();
            std::vector<ecs::EntityID> visible;
            for (auto id : entities) {
                if (MatchesFilter(m_world, id, m_searchFilter))
                    visible.push_back(id);
            }

            float listTop = y;
            float viewHeight = kListViewHeight;
            float contentHeight = static_cast<float>(visible.size()) * rowH;

            for (size_t i = 0; i < visible.size(); ++i) {
                float rowY = listTop + i * rowH - m_scrollOffset;
                if (rowY + rowH < listTop || rowY > listTop + viewHeight)
                    continue;

                Rect rowRect{x, rowY, contentW - kScrollbarPad - 2, rowH};
                ecs::EntityID id = visible[i];
                bool selected = (id == m_selectedEntity);

                // Highlight selected row
                if (selected) {
                    ctx->renderer().drawRect(rowRect, ctx->theme().accentPrimary);
                }

                std::string rowText = "Entity " + std::to_string(id);
                Color textColor = selected ? Color{255, 255, 255, 255}
                                           : ctx->theme().textPrimary;
                label(*ctx, {rowRect.x + 4, rowRect.y + 1}, rowText, textColor);

                // Click to select
                if (ctx->isHovered(rowRect) && ctx->isMouseClicked()) {
                    m_selectedEntity = id;
                }
            }

            // Scrollbar
            Rect trackRect{x + contentW - kScrollbarPad, listTop,
                           kScrollbarWidth, viewHeight};
            scrollbar(*ctx, trackRect, m_scrollOffset, contentHeight, viewHeight);
            y = listTop + viewHeight + 4;

            separator(*ctx, {x, y}, contentW);
            y += 4;

            // Entity count label
            std::string countText = std::to_string(visible.size()) + " entities";
            label(*ctx, {x, y}, countText);
            y += rowH + 4;

            // Component detail section (when an entity is selected)
            if (m_selectedEntity != 0) {
                label(*ctx, {x, y}, "Entity " + std::to_string(m_selectedEntity) + " Components:",
                      ctx->theme().textPrimary);
                y += rowH + 4;

                auto types = m_world.GetComponentTypes(m_selectedEntity);
                if (types.empty()) {
                    label(*ctx, {x + 8, y}, "(no components)", ctx->theme().textSecondary);
                    y += rowH + 4;
                } else {
                    for (const auto& t : types) {
                        std::string typeName = t.name();
                        label(*ctx, {x + 8, y}, typeName, ctx->theme().textSecondary);
                        y += rowH + 2;
                    }
                }
                y += 4;

                separator(*ctx, {x, y}, contentW);
                y += 4;

                Rect destroyRect{x, y, contentW, rowH + 4};
                if (button(*ctx, "Destroy", destroyRect)) {
                    DestroySelectedEntity();
                }
                y += rowH + 8;
            }

            // Clear Selection button
            Rect clearRect{x, y, contentW, rowH + 4};
            if (button(*ctx, "Clear Selection", clearRect)) {
                ClearSelection();
            }
        }
        panelEnd(*ctx);
    }

    // Always run headless logic: clear selection if entity is dead
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
