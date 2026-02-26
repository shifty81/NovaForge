#pragma once
#include "DockNode.h"
#include <vector>
#include <memory>
#include <string>

namespace atlas { class AtlasContext; }

namespace atlas::editor {

class EditorLayout {
public:
    void RegisterPanel(EditorPanel* panel);
    void Draw();

    /** Set the UI context for all registered panels.
     *  Propagates the context pointer to every panel via
     *  EditorPanel::SetContext().  Pass nullptr to clear. */
    void SetContext(atlas::AtlasContext* ctx);

    /** Notify every registered panel that an asset changed on disk. */
    void BroadcastAssetReload(const std::string& assetId,
                              const std::string& path);

    DockNode& Root() { return m_root; }
    const std::vector<EditorPanel*>& Panels() const { return m_panels; }

    // ── Persistence ──────────────────────────────────────────────

    /** Serialise the dock layout tree to a JSON string. */
    std::string SerializeToJSON() const;

    /** Deserialise a dock layout tree from a JSON string.
     *  Panel pointers are resolved by name against registered panels. */
    bool DeserializeFromJSON(const std::string& json);

    /** Save the layout to a JSON file.  Returns true on success. */
    bool SaveToFile(const std::string& path) const;

    /** Load the layout from a JSON file.  Returns true on success. */
    bool LoadFromFile(const std::string& path);

private:
    DockNode m_root;
    std::vector<EditorPanel*> m_panels;

    static void DrawNode(DockNode& node);
    std::string serializeNode(const DockNode& node, int indent) const;
    void deserializeNode(DockNode& node, const std::string& json, size_t& pos);
    EditorPanel* findPanelByName(const std::string& name) const;
    static std::string extractString(const std::string& json, const std::string& key, size_t searchStart, size_t searchEnd);
    static float extractFloat(const std::string& json, const std::string& key, size_t searchStart, size_t searchEnd, float defaultVal);
    static int extractInt(const std::string& json, const std::string& key, size_t searchStart, size_t searchEnd, int defaultVal);
};

}
