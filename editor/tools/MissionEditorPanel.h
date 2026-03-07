#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_server/include/components/game_components.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/// Maximum mission level (1-based).
static constexpr int kMaxMissionLevel = 5;

/**
 * Represents a single mission template being authored in the editor.
 * Maps closely to the server-side MissionTemplate component but lives
 * entirely in the editor for design-time editing.
 */
struct MissionTemplateEntry {
    std::string templateId;
    std::string namePattern;       ///< e.g. "Pirate Clearance: {system}"
    std::string type;              ///< "combat", "mining", "courier", "trade", "exploration"
    int         level      = 1;    ///< 1-5
    std::string faction;           ///< required faction ("" = any)
    float       minStanding = 0.0f;

    struct ObjectiveEntry {
        std::string type;          ///< "destroy", "mine", "deliver", "reach"
        std::string target;        ///< target type (e.g. "pirate_frigate")
        int countMin = 1;
        int countMax = 5;
    };
    std::vector<ObjectiveEntry> objectives;

    // Reward scaling
    double baseIsc           = 100000.0;
    double iscPerLevel       = 50000.0;
    float  baseStandingReward = 0.1f;
    float  standingPerLevel   = 0.05f;
    float  baseTimeLimit      = 3600.0f; ///< seconds, -1 = no limit
};

/**
 * @brief MissionEditorPanel — editor panel for authoring mission templates.
 *
 * Allows designers to:
 *   - Create / edit / delete mission templates
 *   - Add objectives with type, target, and count range
 *   - Configure reward scaling (ISC, standing)
 *   - Filter templates by type or level
 *   - Validate templates before export
 *   - Export templates to JSON for the mission system
 *
 * This panel is headless-safe: Draw() is a no-op without a UI context.
 */
class MissionEditorPanel : public EditorPanel {
public:
    MissionEditorPanel();
    ~MissionEditorPanel() override = default;

    const char* Name() const override { return "Mission Editor"; }
    void Draw() override;

    // ── Template management ──────────────────────────────────────

    /** Add a new template.  Returns the index. */
    size_t AddTemplate(const MissionTemplateEntry& entry);

    /** Remove the template at the given index.  Returns true if removed. */
    bool RemoveTemplate(size_t index);

    /** Replace the template at the given index.  Returns true on success. */
    bool UpdateTemplate(size_t index, const MissionTemplateEntry& entry);

    /** Get the number of templates. */
    size_t TemplateCount() const { return m_templates.size(); }

    /** Access a template by index. */
    const MissionTemplateEntry& GetTemplate(size_t index) const {
        return m_templates[index];
    }

    /** Read-only access to all templates. */
    const std::vector<MissionTemplateEntry>& Templates() const {
        return m_templates;
    }

    // ── Selection ────────────────────────────────────────────────

    void SelectTemplate(int index);
    void ClearSelection();
    int  SelectedTemplate() const { return m_selectedIndex; }

    // ── Filtering ────────────────────────────────────────────────

    /** Set type filter ("" clears filter). */
    void SetTypeFilter(const std::string& type);
    const std::string& TypeFilter() const { return m_typeFilter; }

    /** Set level filter (0 = show all). */
    void SetLevelFilter(int level);
    int LevelFilter() const { return m_levelFilter; }

    /** Count of templates matching the current filter. */
    size_t FilteredCount() const;

    // ── Validation ───────────────────────────────────────────────

    /**
     * Validate a template.  Returns true if valid.
     * If invalid, populates the error string.
     */
    static bool ValidateTemplate(const MissionTemplateEntry& entry,
                                 std::string& errorOut);

    /** Validate all templates.  Returns count of invalid ones. */
    size_t ValidateAll();

    // ── Export ────────────────────────────────────────────────────

    /** Serialise all templates to a JSON string. */
    std::string ExportToJson() const;

    /** Import templates from a JSON string.  Returns count imported. */
    size_t ImportFromJson(const std::string& json);

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<MissionTemplateEntry> m_templates;
    int         m_selectedIndex = -1;
    std::string m_typeFilter;
    int         m_levelFilter   = 0;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);

    bool matchesFilter(const MissionTemplateEntry& entry) const;
};

} // namespace atlas::editor
