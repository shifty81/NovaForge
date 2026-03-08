#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * @brief A combat log entry for analysis in the editor.
 */
struct CombatLogEntry {
    uint32_t entryId = 0;
    std::string attackerId;
    std::string defenderId;
    std::string damageType;      ///< "EM", "Thermal", "Kinetic", "Explosive"
    float damageAmount = 0.0f;
    std::string weaponType;
    bool hit = true;
    float timestamp = 0.0f;
};

/**
 * @brief An engagement summary for the panel.
 */
struct EngagementEntry {
    uint32_t engagementId = 0;
    std::string engagementName;
    float duration      = 0.0f;
    float totalDamage   = 0.0f;
    int   hits          = 0;
    int   misses        = 0;
    std::string outcome;         ///< "Victory", "Defeat", "Draw", "Ongoing"
};

/**
 * CombatLogPanel — Combat log analysis and replay dashboard.
 *
 * Designers can:
 *   - Load combat log entries for review and balance analysis.
 *   - Add individual damage events (attacker, defender, damage type, amount).
 *   - Create engagement summaries with hit/miss ratios and outcomes.
 *   - View aggregate damage by type for balance tuning.
 *   - Calculate DPS (damage per second) for engagements.
 *   - Export combat data to JSON for external analysis tools.
 *
 * Headless-safe: Draw() is a no-op when no AtlasContext is set.
 */
class CombatLogPanel : public EditorPanel {
public:
    CombatLogPanel();
    ~CombatLogPanel() override = default;

    const char* Name() const override { return "Combat Log Viewer"; }
    void Draw() override;

    // ── Entry management ─────────────────────────────────────────

    int AddEntry(const std::string& attackerId, const std::string& defenderId,
                 const std::string& damageType, float damageAmount,
                 const std::string& weaponType, bool hit);
    bool RemoveEntry(uint32_t entryId);
    int EntryCount() const { return static_cast<int>(m_entries.size()); }
    const std::vector<CombatLogEntry>& Entries() const { return m_entries; }

    // ── Engagement management ────────────────────────────────────

    int AddEngagement(const std::string& name, const std::string& outcome);
    bool RemoveEngagement(uint32_t engagementId);
    bool SetEngagementDuration(uint32_t engagementId, float duration);
    bool SetEngagementDamage(uint32_t engagementId, float damage);
    bool SetEngagementHits(uint32_t engagementId, int hits, int misses);
    int EngagementCount() const { return static_cast<int>(m_engagements.size()); }
    const std::vector<EngagementEntry>& Engagements() const { return m_engagements; }

    float GetEngagementDPS(uint32_t engagementId) const;
    float GetEngagementAccuracy(uint32_t engagementId) const;

    // ── Aggregate stats ──────────────────────────────────────────

    float TotalDamage() const;
    int TotalHits() const;
    int TotalMisses() const;

    // ── Export ────────────────────────────────────────────────────

    std::string ExportJSON() const;

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<CombatLogEntry>  m_entries;
    std::vector<EngagementEntry> m_engagements;

    static constexpr int kMaxEntries      = 200;
    static constexpr int kMaxEngagements  = 20;

    uint32_t m_nextEntryId      = 1;
    uint32_t m_nextEngagementId = 1;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    EngagementEntry* findEngagement(uint32_t engagementId);
    const EngagementEntry* findEngagementConst(uint32_t engagementId) const;
};

} // namespace atlas::editor
