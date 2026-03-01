#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * @brief Represents a single NPC definition in the editor.
 *
 * Maps to the JSON NPC definitions in data/npcs/ and integrates
 * with the AI economic actor / intent / behavior tree systems.
 */
struct NPCEntry {
    std::string npcId;
    std::string name;
    std::string type;           ///< "frigate", "cruiser", "battleship", etc.
    std::string faction;        ///< "venom_syndicate", "iron_corsairs", etc.
    std::string behavior;       ///< "aggressive", "defensive", "passive", "flee"
    std::string archetype;      ///< "trader", "pirate", "patrol", "miner", "hauler"

    // Combat stats
    float hullHp    = 0.0f;
    float armorHp   = 0.0f;
    float shieldHp  = 0.0f;
    float maxVelocity = 0.0f;
    float orbitDistance = 0.0f;

    // Economic stats (ties into AIEconomicActor)
    double bounty        = 0.0;
    double startingWallet = 10000.0;
    std::string ownedShipType;
    double shipValue     = 0.0;

    // AI behavior
    float awarenessRange = 50000.0f;
    float fleeThreshold  = 0.25f;
    std::string targetSelection;    ///< "closest", "lowest_hp", "highest_threat"
    std::string haulStationId;      ///< station for hauling (miner/hauler NPCs)

    // Loot
    std::vector<std::string> lootTable;

    // Weapons
    struct WeaponSlot {
        std::string type;
        float damage = 0.0f;
    };
    std::vector<WeaponSlot> weapons;
};

/**
 * @brief NPCEditorPanel — editor panel for authoring NPC definitions.
 *
 * Allows designers to:
 *   - Create / edit / delete NPC definitions
 *   - Configure combat stats, AI behavior, and economic parameters
 *   - Set archetype (trader, pirate, miner, hauler) for AI intent system
 *   - Configure loot tables and weapon loadouts
 *   - Filter by faction, type, or archetype
 *   - Validate NPC definitions
 *   - Export / import NPC data as JSON
 */
class NPCEditorPanel : public EditorPanel {
public:
    NPCEditorPanel();
    ~NPCEditorPanel() override = default;

    const char* Name() const override { return "NPC Editor"; }
    void Draw() override;

    // ── NPC management ───────────────────────────────────────────

    size_t AddNPC(const NPCEntry& entry);
    bool   RemoveNPC(size_t index);
    bool   UpdateNPC(size_t index, const NPCEntry& entry);
    size_t NPCCount() const { return m_npcs.size(); }

    const NPCEntry& GetNPC(size_t index) const { return m_npcs[index]; }
    const std::vector<NPCEntry>& NPCs() const { return m_npcs; }

    // ── Selection ────────────────────────────────────────────────

    void SelectNPC(int index);
    void ClearSelection();
    int  SelectedNPC() const { return m_selectedIndex; }

    // ── Filtering ────────────────────────────────────────────────

    void SetFactionFilter(const std::string& faction);
    const std::string& FactionFilter() const { return m_factionFilter; }

    void SetArchetypeFilter(const std::string& archetype);
    const std::string& ArchetypeFilter() const { return m_archetypeFilter; }

    size_t FilteredCount() const;

    // ── Validation ───────────────────────────────────────────────

    static bool ValidateNPC(const NPCEntry& entry, std::string& errorOut);
    size_t ValidateAll();

    // ── Export ────────────────────────────────────────────────────

    std::string ExportToJson() const;
    size_t ImportFromJson(const std::string& json);

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<NPCEntry> m_npcs;
    int m_selectedIndex = -1;
    std::string m_factionFilter;
    std::string m_archetypeFilter;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    bool matchesFilter(const NPCEntry& entry) const;
};

} // namespace atlas::editor
