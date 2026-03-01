#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * @brief Represents a single game module (weapon, defense, utility, mining).
 *
 * Maps to the JSON module definitions in data/modules/.
 */
struct ModuleEntry {
    std::string moduleId;
    std::string name;
    std::string type;           ///< "weapon", "defense", "utility", "mining"
    std::string slot;           ///< "high", "mid", "low"
    std::string weaponType;     ///< "projectile", "hybrid", "energy", "missile" (weapons only)
    std::string damageType;     ///< "em", "thermal", "kinetic", "explosive"

    float damage        = 0.0f;
    float rateOfFire    = 0.0f; ///< seconds between shots
    float optimalRange  = 0.0f;
    float falloffRange  = 0.0f;
    float tracking      = 0.0f;

    int   cpu           = 0;
    int   powergrid     = 0;
    int   capacitorUse  = 0;
    int   metaLevel     = 0;
    int   techLevel     = 1;

    // Defense-specific
    float shieldBonus   = 0.0f;
    float armorBonus    = 0.0f;
    float resistBonus   = 0.0f;

    // Utility-specific
    float bonusValue    = 0.0f;
    std::string bonusTarget;    ///< e.g. "velocity", "capacitor", "signature"
};

/**
 * @brief ModuleEditorPanel — editor panel for authoring weapon / defense / utility modules.
 *
 * Allows designers to:
 *   - Create / edit / delete module definitions
 *   - Configure damage, fitting cost (CPU/PG), ranges, tracking
 *   - Filter by type (weapon, defense, utility, mining) and slot
 *   - Validate module balance (e.g. CPU cost vs meta level)
 *   - Export / import module data as JSON
 */
class ModuleEditorPanel : public EditorPanel {
public:
    ModuleEditorPanel();
    ~ModuleEditorPanel() override = default;

    const char* Name() const override { return "Module Editor"; }
    void Draw() override;

    // ── Module management ────────────────────────────────────────

    size_t AddModule(const ModuleEntry& entry);
    bool   RemoveModule(size_t index);
    bool   UpdateModule(size_t index, const ModuleEntry& entry);
    size_t ModuleCount() const { return m_modules.size(); }

    const ModuleEntry& GetModule(size_t index) const { return m_modules[index]; }
    const std::vector<ModuleEntry>& Modules() const { return m_modules; }

    // ── Selection ────────────────────────────────────────────────

    void SelectModule(int index);
    void ClearSelection();
    int  SelectedModule() const { return m_selectedIndex; }

    // ── Filtering ────────────────────────────────────────────────

    void SetTypeFilter(const std::string& type);
    const std::string& TypeFilter() const { return m_typeFilter; }

    void SetSlotFilter(const std::string& slot);
    const std::string& SlotFilter() const { return m_slotFilter; }

    size_t FilteredCount() const;

    // ── Validation ───────────────────────────────────────────────

    static bool ValidateModule(const ModuleEntry& entry, std::string& errorOut);
    size_t ValidateAll();

    // ── Export ────────────────────────────────────────────────────

    std::string ExportToJson() const;
    size_t ImportFromJson(const std::string& json);

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<ModuleEntry> m_modules;
    int m_selectedIndex = -1;
    std::string m_typeFilter;
    std::string m_slotFilter;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    bool matchesFilter(const ModuleEntry& entry) const;
};

} // namespace atlas::editor
