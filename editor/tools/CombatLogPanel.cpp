#include "CombatLogPanel.h"

#include <algorithm>
#include <sstream>

namespace atlas::editor {

// ── Construction ─────────────────────────────────────────────────────

CombatLogPanel::CombatLogPanel() = default;

// ── Internal helpers ─────────────────────────────────────────────────

EngagementEntry* CombatLogPanel::findEngagement(uint32_t engagementId) {
    for (auto& e : m_engagements) {
        if (e.engagementId == engagementId) return &e;
    }
    return nullptr;
}

const EngagementEntry* CombatLogPanel::findEngagementConst(uint32_t engagementId) const {
    for (const auto& e : m_engagements) {
        if (e.engagementId == engagementId) return &e;
    }
    return nullptr;
}

// ── Entry management ─────────────────────────────────────────────────

int CombatLogPanel::AddEntry(const std::string& attackerId,
                              const std::string& defenderId,
                              const std::string& damageType,
                              float damageAmount,
                              const std::string& weaponType,
                              bool hit) {
    if (static_cast<int>(m_entries.size()) >= kMaxEntries) return -1;
    if (attackerId.empty() || defenderId.empty()) return -1;

    // Validate damage type
    if (damageType != "EM" && damageType != "Thermal" &&
        damageType != "Kinetic" && damageType != "Explosive") return -1;

    CombatLogEntry entry;
    entry.entryId      = m_nextEntryId++;
    entry.attackerId   = attackerId;
    entry.defenderId   = defenderId;
    entry.damageType   = damageType;
    entry.damageAmount = std::max(0.0f, damageAmount);
    entry.weaponType   = weaponType;
    entry.hit          = hit;
    m_entries.push_back(entry);

    log("Entry #" + std::to_string(entry.entryId) + " " + attackerId + "→"
        + defenderId + " " + damageType + " " + std::to_string(static_cast<int>(damageAmount))
        + (hit ? " HIT" : " MISS"));
    return static_cast<int>(entry.entryId);
}

bool CombatLogPanel::RemoveEntry(uint32_t entryId) {
    auto it = std::find_if(m_entries.begin(), m_entries.end(),
        [entryId](const CombatLogEntry& e) { return e.entryId == entryId; });
    if (it == m_entries.end()) return false;
    log("Removed entry #" + std::to_string(entryId));
    m_entries.erase(it);
    return true;
}

// ── Engagement management ────────────────────────────────────────────

int CombatLogPanel::AddEngagement(const std::string& name,
                                    const std::string& outcome) {
    if (static_cast<int>(m_engagements.size()) >= kMaxEngagements) return -1;
    if (name.empty()) return -1;

    // Validate outcome
    if (outcome != "Victory" && outcome != "Defeat" &&
        outcome != "Draw" && outcome != "Ongoing") return -1;

    EngagementEntry eng;
    eng.engagementId   = m_nextEngagementId++;
    eng.engagementName = name;
    eng.outcome        = outcome;
    m_engagements.push_back(eng);

    log("Engagement #" + std::to_string(eng.engagementId) + " \"" + name + "\" " + outcome);
    return static_cast<int>(eng.engagementId);
}

bool CombatLogPanel::RemoveEngagement(uint32_t engagementId) {
    auto it = std::find_if(m_engagements.begin(), m_engagements.end(),
        [engagementId](const EngagementEntry& e) { return e.engagementId == engagementId; });
    if (it == m_engagements.end()) return false;
    log("Removed engagement #" + std::to_string(engagementId));
    m_engagements.erase(it);
    return true;
}

bool CombatLogPanel::SetEngagementDuration(uint32_t engagementId, float duration) {
    auto* e = findEngagement(engagementId);
    if (!e) return false;
    e->duration = std::max(0.0f, duration);
    return true;
}

bool CombatLogPanel::SetEngagementDamage(uint32_t engagementId, float damage) {
    auto* e = findEngagement(engagementId);
    if (!e) return false;
    e->totalDamage = std::max(0.0f, damage);
    return true;
}

bool CombatLogPanel::SetEngagementHits(uint32_t engagementId, int hits, int misses) {
    auto* e = findEngagement(engagementId);
    if (!e) return false;
    e->hits   = std::max(0, hits);
    e->misses = std::max(0, misses);
    return true;
}

float CombatLogPanel::GetEngagementDPS(uint32_t engagementId) const {
    const auto* e = findEngagementConst(engagementId);
    if (!e || e->duration <= 0.0f) return 0.0f;
    return e->totalDamage / e->duration;
}

float CombatLogPanel::GetEngagementAccuracy(uint32_t engagementId) const {
    const auto* e = findEngagementConst(engagementId);
    if (!e) return 0.0f;
    int total = e->hits + e->misses;
    if (total == 0) return 0.0f;
    return static_cast<float>(e->hits) / static_cast<float>(total);
}

// ── Aggregate stats ──────────────────────────────────────────────────

float CombatLogPanel::TotalDamage() const {
    float total = 0.0f;
    for (const auto& e : m_entries) {
        if (e.hit) total += e.damageAmount;
    }
    return total;
}

int CombatLogPanel::TotalHits() const {
    int count = 0;
    for (const auto& e : m_entries) {
        if (e.hit) ++count;
    }
    return count;
}

int CombatLogPanel::TotalMisses() const {
    int count = 0;
    for (const auto& e : m_entries) {
        if (!e.hit) ++count;
    }
    return count;
}

// ── Export ────────────────────────────────────────────────────────────

std::string CombatLogPanel::ExportJSON() const {
    std::ostringstream os;
    os << "{\n";
    os << "  \"entries\": [\n";
    for (size_t i = 0; i < m_entries.size(); ++i) {
        const auto& e = m_entries[i];
        os << "    {\"id\": " << e.entryId
           << ", \"attacker\": \"" << e.attackerId << "\""
           << ", \"defender\": \"" << e.defenderId << "\""
           << ", \"damageType\": \"" << e.damageType << "\""
           << ", \"damage\": " << e.damageAmount
           << ", \"weapon\": \"" << e.weaponType << "\""
           << ", \"hit\": " << (e.hit ? "true" : "false") << "}";
        if (i + 1 < m_entries.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";

    os << "  \"engagements\": [\n";
    for (size_t i = 0; i < m_engagements.size(); ++i) {
        const auto& eng = m_engagements[i];
        os << "    {\"id\": " << eng.engagementId
           << ", \"name\": \"" << eng.engagementName << "\""
           << ", \"duration\": " << eng.duration
           << ", \"totalDamage\": " << eng.totalDamage
           << ", \"hits\": " << eng.hits
           << ", \"misses\": " << eng.misses
           << ", \"outcome\": \"" << eng.outcome << "\"}";
        if (i + 1 < m_engagements.size()) os << ",";
        os << "\n";
    }
    os << "  ]\n";

    os << "}";
    return os.str();
}

// ── Draw ──────────────────────────────────────────────────────────────

void CombatLogPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Combat Log Viewer", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad    = ctx.theme().padding;
    const float rowH   = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH  = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Summary
    std::string summary = "Entries: " + std::to_string(EntryCount())
        + "  Engagements: " + std::to_string(EngagementCount())
        + "  Total Damage: " + std::to_string(static_cast<int>(TotalDamage()))
        + "  Hits: " + std::to_string(TotalHits())
        + "  Misses: " + std::to_string(TotalMisses());
    atlas::label(ctx, {b.x + pad, y}, summary, ctx.theme().textPrimary);
    y += rowH + pad;

    // Engagement listing
    for (int i = 0; i < EngagementCount() && i < 8; ++i) {
        const auto& eng = m_engagements[i];
        std::string info = "#" + std::to_string(eng.engagementId)
            + " " + eng.engagementName + " " + eng.outcome
            + " DPS:" + std::to_string(static_cast<int>(
                eng.duration > 0 ? eng.totalDamage / eng.duration : 0));
        atlas::label(ctx, {b.x + pad, y}, info, ctx.theme().textPrimary);
        y += rowH;
    }
    y += pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Helpers ───────────────────────────────────────────────────────────

void CombatLogPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
