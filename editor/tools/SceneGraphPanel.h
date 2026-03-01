#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * Represents a node in the hierarchical scene graph.
 * Nodes can be entities, groups, or transform containers.
 */
struct SceneNode {
    uint32_t    id       = 0;
    uint32_t    parentId = 0;       ///< 0 = root-level node
    std::string name;
    std::string type;               ///< "Entity", "Group", "Light", "Camera"
    bool        expanded = false;   ///< Expanded in tree view
    bool        visible  = true;
    bool        locked   = false;   ///< Prevent selection/editing
};

/**
 * @brief SceneGraphPanel — hierarchical tree view of the scene.
 *
 * Provides:
 *   - Tree display of all scene entities with parent-child relationships
 *   - Reparenting (drag-drop / programmatic)
 *   - Search/filter by name
 *   - Visibility and lock toggles per node
 *   - Node CRUD operations
 *   - Depth calculation and child enumeration
 *
 * Headless-safe: Draw() is a no-op without a UI context.
 */
class SceneGraphPanel : public EditorPanel {
public:
    SceneGraphPanel();
    ~SceneGraphPanel() override = default;

    const char* Name() const override { return "Scene Graph"; }
    void Draw() override;

    // ── Node management ──────────────────────────────────────────

    /** Add a new node.  Returns its assigned ID. */
    uint32_t AddNode(const std::string& name, const std::string& type,
                     uint32_t parentId = 0);

    /** Remove a node and all its children recursively. */
    bool RemoveNode(uint32_t id);

    /** Rename a node. */
    bool RenameNode(uint32_t id, const std::string& newName);

    /** Reparent a node under a new parent (0 = root). */
    bool Reparent(uint32_t nodeId, uint32_t newParentId);

    /** Get the total number of nodes. */
    size_t NodeCount() const { return m_nodes.size(); }

    /** Access a node by ID.  Returns nullptr if not found. */
    const SceneNode* GetNode(uint32_t id) const;

    /** Get all root-level nodes (parentId == 0). */
    std::vector<uint32_t> RootNodes() const;

    /** Get the direct children of a node. */
    std::vector<uint32_t> Children(uint32_t parentId) const;

    /** Compute the depth of a node in the tree (root = 0). */
    int Depth(uint32_t id) const;

    // ── Selection ────────────────────────────────────────────────

    void SelectNode(uint32_t id);
    void ClearSelection();
    uint32_t SelectedNode() const { return m_selectedId; }

    // ── Visibility / Lock ────────────────────────────────────────

    using EditorPanel::SetVisible;   // inherit SetVisible(bool) from base
    bool SetVisible(uint32_t id, bool visible);
    bool SetLocked(uint32_t id, bool locked);

    // ── Expand / Collapse ────────────────────────────────────────

    bool ToggleExpanded(uint32_t id);
    void ExpandAll();
    void CollapseAll();

    // ── Search ───────────────────────────────────────────────────

    void SetSearchFilter(const std::string& filter);
    const std::string& SearchFilter() const { return m_searchFilter; }

    /** Count of nodes matching the current search filter. */
    size_t FilteredCount() const;

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    SceneNode*       findNode(uint32_t id);
    const SceneNode* findNode(uint32_t id) const;
    bool isDescendant(uint32_t nodeId, uint32_t ancestorId) const;
    void removeRecursive(uint32_t id);
    bool matchesSearch(const SceneNode& node) const;
    void log(const std::string& msg);

    std::vector<SceneNode> m_nodes;
    uint32_t m_nextId      = 1;
    uint32_t m_selectedId  = 0;
    std::string m_searchFilter;
    std::vector<std::string> m_log;

    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;
};

} // namespace atlas::editor
