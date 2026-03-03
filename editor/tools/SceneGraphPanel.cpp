#include "SceneGraphPanel.h"
#include <algorithm>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

SceneGraphPanel::SceneGraphPanel() {
    log("Scene Graph initialized");
}

// ── Draw ───────────────────────────────────────────────────────────

void SceneGraphPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Scene Graph", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Node count
    atlas::label(ctx, {b.x + pad, y},
        "Nodes: " + std::to_string(m_nodes.size()),
        ctx.theme().textPrimary);
    y += rowH + pad;

    // Search filter
    if (!m_searchFilter.empty()) {
        atlas::label(ctx, {b.x + pad, y},
            "Filter: " + m_searchFilter + " (" + std::to_string(FilteredCount()) + " matches)",
            ctx.theme().textSecondary);
        y += rowH + pad;
    }

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Selected node info
    if (m_selectedId != 0) {
        const SceneNode* sel = findNode(m_selectedId);
        if (sel) {
            atlas::label(ctx, {b.x + pad, y},
                "Selected: " + sel->name + " [" + sel->type + "] depth=" + std::to_string(Depth(m_selectedId)),
                ctx.theme().textPrimary);
            y += rowH + pad;
        }
    }

    // Node list (flat with indentation)
    auto roots = RootNodes();
    for (uint32_t rootId : roots) {
        const SceneNode* n = findNode(rootId);
        if (!n) continue;
        if (!matchesSearch(*n)) continue;
        if (y + rowH > b.y + b.h - pad) break;
        std::string label = n->name + " [" + n->type + "]";
        if (n->id == m_selectedId) label = "> " + label;
        atlas::Rect row{b.x + pad, y, b.w - 2.0f * pad, rowH};
        if (atlas::button(ctx, label.c_str(), row)) {
            SelectNode(n->id);
        }
        y += rowH + pad;
    }

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Node management ────────────────────────────────────────────────

uint32_t SceneGraphPanel::AddNode(const std::string& name,
                                   const std::string& type,
                                   uint32_t parentId) {
    // Validate parent exists (if specified).
    if (parentId != 0 && !findNode(parentId)) {
        return 0;
    }

    SceneNode node;
    node.id       = m_nextId++;
    node.parentId = parentId;
    node.name     = name;
    node.type     = type;
    m_nodes.push_back(node);
    log("Added " + type + " '" + name + "' (id=" + std::to_string(node.id) + ")");
    return node.id;
}

bool SceneGraphPanel::RemoveNode(uint32_t id) {
    if (!findNode(id)) return false;
    const SceneNode* n = findNode(id);
    std::string removedName = n ? n->name : "";

    // Clear selection if removing selected node or its ancestor.
    if (m_selectedId == id || isDescendant(m_selectedId, id)) {
        m_selectedId = 0;
    }

    removeRecursive(id);
    log("Removed '" + removedName + "' and children");
    return true;
}

bool SceneGraphPanel::RenameNode(uint32_t id, const std::string& newName) {
    SceneNode* n = findNode(id);
    if (!n) return false;
    std::string oldName = n->name;
    n->name = newName;
    log("Renamed '" + oldName + "' -> '" + newName + "'");
    return true;
}

bool SceneGraphPanel::Reparent(uint32_t nodeId, uint32_t newParentId) {
    SceneNode* node = findNode(nodeId);
    if (!node) return false;
    if (nodeId == newParentId) return false;

    // Validate new parent exists (unless reparenting to root).
    if (newParentId != 0 && !findNode(newParentId)) return false;

    // Prevent creating cycles (can't make a node a child of its own descendant).
    if (newParentId != 0 && isDescendant(newParentId, nodeId)) return false;

    node->parentId = newParentId;
    log("Reparented '" + node->name + "' under parent " + std::to_string(newParentId));
    return true;
}

const SceneNode* SceneGraphPanel::GetNode(uint32_t id) const {
    return findNode(id);
}

std::vector<uint32_t> SceneGraphPanel::RootNodes() const {
    std::vector<uint32_t> roots;
    for (const auto& n : m_nodes) {
        if (n.parentId == 0) roots.push_back(n.id);
    }
    return roots;
}

std::vector<uint32_t> SceneGraphPanel::Children(uint32_t parentId) const {
    std::vector<uint32_t> kids;
    for (const auto& n : m_nodes) {
        if (n.parentId == parentId) kids.push_back(n.id);
    }
    return kids;
}

int SceneGraphPanel::Depth(uint32_t id) const {
    int depth = 0;
    const SceneNode* n = findNode(id);
    while (n && n->parentId != 0) {
        ++depth;
        n = findNode(n->parentId);
    }
    return depth;
}

// ── Selection ──────────────────────────────────────────────────────

void SceneGraphPanel::SelectNode(uint32_t id) {
    if (!findNode(id)) return;
    const SceneNode* n = findNode(id);
    if (n->locked) return;
    m_selectedId = id;
    log("Selected '" + n->name + "'");
}

void SceneGraphPanel::ClearSelection() {
    m_selectedId = 0;
}

// ── Visibility / Lock ──────────────────────────────────────────────

bool SceneGraphPanel::SetVisible(uint32_t id, bool visible) {
    SceneNode* n = findNode(id);
    if (!n) return false;
    n->visible = visible;
    return true;
}

bool SceneGraphPanel::SetLocked(uint32_t id, bool locked) {
    SceneNode* n = findNode(id);
    if (!n) return false;
    n->locked = locked;
    // If locking the currently selected node, deselect it.
    if (locked && m_selectedId == id) {
        m_selectedId = 0;
    }
    return true;
}

// ── Expand / Collapse ──────────────────────────────────────────────

bool SceneGraphPanel::ToggleExpanded(uint32_t id) {
    SceneNode* n = findNode(id);
    if (!n) return false;
    n->expanded = !n->expanded;
    return true;
}

void SceneGraphPanel::ExpandAll() {
    for (auto& n : m_nodes) n.expanded = true;
}

void SceneGraphPanel::CollapseAll() {
    for (auto& n : m_nodes) n.expanded = false;
}

// ── Search ─────────────────────────────────────────────────────────

void SceneGraphPanel::SetSearchFilter(const std::string& filter) {
    m_searchFilter = filter;
}

size_t SceneGraphPanel::FilteredCount() const {
    if (m_searchFilter.empty()) return m_nodes.size();
    size_t count = 0;
    for (const auto& n : m_nodes) {
        if (matchesSearch(n)) ++count;
    }
    return count;
}

// ── Internals ──────────────────────────────────────────────────────

SceneNode* SceneGraphPanel::findNode(uint32_t id) {
    for (auto& n : m_nodes) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

const SceneNode* SceneGraphPanel::findNode(uint32_t id) const {
    for (const auto& n : m_nodes) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

bool SceneGraphPanel::isDescendant(uint32_t nodeId, uint32_t ancestorId) const {
    const SceneNode* n = findNode(nodeId);
    while (n && n->parentId != 0) {
        if (n->parentId == ancestorId) return true;
        n = findNode(n->parentId);
    }
    return false;
}

void SceneGraphPanel::removeRecursive(uint32_t id) {
    // Collect all children first.
    std::vector<uint32_t> childIds;
    for (const auto& n : m_nodes) {
        if (n.parentId == id) childIds.push_back(n.id);
    }
    for (uint32_t childId : childIds) {
        removeRecursive(childId);
    }
    m_nodes.erase(
        std::remove_if(m_nodes.begin(), m_nodes.end(),
            [id](const SceneNode& n) { return n.id == id; }),
        m_nodes.end());
}

bool SceneGraphPanel::matchesSearch(const SceneNode& node) const {
    if (m_searchFilter.empty()) return true;
    // Case-insensitive name and type search.
    auto toLower = [](const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    };
    std::string lowerFilter = toLower(m_searchFilter);
    return toLower(node.name).find(lowerFilter) != std::string::npos ||
           toLower(node.type).find(lowerFilter) != std::string::npos;
}

void SceneGraphPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
