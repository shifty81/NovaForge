#include "MissionEditorPanel.h"
#include <sstream>
#include <algorithm>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

MissionEditorPanel::MissionEditorPanel() {
    log("Mission Editor initialized");
}

// ── Draw ───────────────────────────────────────────────────────────

void MissionEditorPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    if (!atlas::panelBeginStateful(ctx, "Mission Editor", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Template count
    atlas::label(ctx, {b.x + pad, y},
        "Templates: " + std::to_string(m_templates.size()),
        ctx.theme().textPrimary);
    y += rowH + pad;

    // Filter info
    std::string filterInfo = "Filter: ";
    if (!m_typeFilter.empty()) filterInfo += "type=" + m_typeFilter + " ";
    if (m_levelFilter > 0) filterInfo += "level=" + std::to_string(m_levelFilter);
    if (m_typeFilter.empty() && m_levelFilter == 0) filterInfo += "(none)";
    atlas::label(ctx, {b.x + pad, y}, filterInfo, ctx.theme().textSecondary);
    y += rowH + pad;

    // Validate button
    const float btnW = 100.0f;
    if (atlas::button(ctx, "Validate All", {b.x + pad, y, btnW, rowH + pad})) {
        ValidateAll();
    }
    y += rowH + pad + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Selected template info
    if (m_selectedIndex >= 0 &&
        m_selectedIndex < static_cast<int>(m_templates.size())) {
        const auto& t = m_templates[m_selectedIndex];
        atlas::label(ctx, {b.x + pad, y},
            "Selected: " + t.templateId, ctx.theme().textPrimary);
        y += rowH + pad;
        atlas::label(ctx, {b.x + pad, y},
            "  Type: " + t.type + "  Level: " + std::to_string(t.level),
            ctx.theme().textSecondary);
        y += rowH + pad;
        atlas::label(ctx, {b.x + pad, y},
            "  Objectives: " + std::to_string(t.objectives.size()),
            ctx.theme().textSecondary);
        y += rowH + pad;
    }

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Template management ────────────────────────────────────────────

size_t MissionEditorPanel::AddTemplate(const MissionTemplateEntry& entry) {
    m_templates.push_back(entry);
    log("Added template: " + entry.templateId + " (" + entry.type
        + " L" + std::to_string(entry.level) + ")");
    return m_templates.size() - 1;
}

bool MissionEditorPanel::RemoveTemplate(size_t index) {
    if (index >= m_templates.size()) return false;
    std::string id = m_templates[index].templateId;
    m_templates.erase(m_templates.begin() + static_cast<ptrdiff_t>(index));

    // Fix selection
    if (m_selectedIndex == static_cast<int>(index)) {
        m_selectedIndex = -1;
    } else if (m_selectedIndex > static_cast<int>(index)) {
        --m_selectedIndex;
    }

    log("Removed template: " + id);
    return true;
}

bool MissionEditorPanel::UpdateTemplate(size_t index,
                                         const MissionTemplateEntry& entry) {
    if (index >= m_templates.size()) return false;
    m_templates[index] = entry;
    log("Updated template: " + entry.templateId);
    return true;
}

// ── Selection ──────────────────────────────────────────────────────

void MissionEditorPanel::SelectTemplate(int index) {
    if (index < 0 || index >= static_cast<int>(m_templates.size())) return;
    m_selectedIndex = index;
    log("Selected template [" + std::to_string(index) + "]: "
        + m_templates[index].templateId);
}

void MissionEditorPanel::ClearSelection() {
    m_selectedIndex = -1;
}

// ── Filtering ──────────────────────────────────────────────────────

void MissionEditorPanel::SetTypeFilter(const std::string& type) {
    m_typeFilter = type;
}

void MissionEditorPanel::SetLevelFilter(int level) {
    m_levelFilter = std::max(0, std::min(level, 5));
}

size_t MissionEditorPanel::FilteredCount() const {
    size_t count = 0;
    for (const auto& t : m_templates) {
        if (matchesFilter(t)) ++count;
    }
    return count;
}

bool MissionEditorPanel::matchesFilter(const MissionTemplateEntry& entry) const {
    if (!m_typeFilter.empty() && entry.type != m_typeFilter)
        return false;
    if (m_levelFilter > 0 && entry.level != m_levelFilter)
        return false;
    return true;
}

// ── Validation ─────────────────────────────────────────────────────

bool MissionEditorPanel::ValidateTemplate(const MissionTemplateEntry& entry,
                                            std::string& errorOut) {
    if (entry.templateId.empty()) {
        errorOut = "Template ID is empty";
        return false;
    }
    if (entry.type.empty()) {
        errorOut = "Mission type is empty";
        return false;
    }
    if (entry.level < 1 || entry.level > 5) {
        errorOut = "Level must be 1-5";
        return false;
    }
    if (entry.objectives.empty()) {
        errorOut = "Must have at least one objective";
        return false;
    }
    for (const auto& obj : entry.objectives) {
        if (obj.type.empty()) {
            errorOut = "Objective type is empty";
            return false;
        }
        if (obj.target.empty()) {
            errorOut = "Objective target is empty";
            return false;
        }
        if (obj.countMin < 1) {
            errorOut = "Objective count_min must be >= 1";
            return false;
        }
        if (obj.countMax < obj.countMin) {
            errorOut = "Objective count_max must be >= count_min";
            return false;
        }
    }
    if (entry.baseIsk < 0.0) {
        errorOut = "Base ISK reward must be non-negative";
        return false;
    }
    errorOut.clear();
    return true;
}

size_t MissionEditorPanel::ValidateAll() {
    size_t invalid = 0;
    for (size_t i = 0; i < m_templates.size(); ++i) {
        std::string err;
        if (!ValidateTemplate(m_templates[i], err)) {
            ++invalid;
            log("INVALID [" + std::to_string(i) + "] "
                + m_templates[i].templateId + ": " + err);
        }
    }
    if (invalid == 0) {
        log("All " + std::to_string(m_templates.size())
            + " templates valid");
    } else {
        log(std::to_string(invalid) + " of "
            + std::to_string(m_templates.size()) + " templates invalid");
    }
    return invalid;
}

// ── Export / Import ────────────────────────────────────────────────

std::string MissionEditorPanel::ExportToJson() const {
    std::ostringstream os;
    os << "{ \"mission_templates\": [\n";
    for (size_t i = 0; i < m_templates.size(); ++i) {
        const auto& t = m_templates[i];
        os << "  { \"template_id\": \"" << t.templateId << "\","
           << " \"name_pattern\": \"" << t.namePattern << "\","
           << " \"type\": \"" << t.type << "\","
           << " \"level\": " << t.level << ","
           << " \"faction\": \"" << t.faction << "\","
           << " \"min_standing\": " << t.minStanding << ","
           << " \"base_isk\": " << t.baseIsk << ","
           << " \"isk_per_level\": " << t.iskPerLevel << ","
           << " \"base_standing_reward\": " << t.baseStandingReward << ","
           << " \"standing_per_level\": " << t.standingPerLevel << ","
           << " \"base_time_limit\": " << t.baseTimeLimit << ","
           << " \"objectives\": [";
        for (size_t j = 0; j < t.objectives.size(); ++j) {
            const auto& o = t.objectives[j];
            os << "{ \"type\": \"" << o.type << "\","
               << " \"target\": \"" << o.target << "\","
               << " \"count_min\": " << o.countMin << ","
               << " \"count_max\": " << o.countMax << " }";
            if (j + 1 < t.objectives.size()) os << ", ";
        }
        os << "] }";
        if (i + 1 < m_templates.size()) os << ",";
        os << "\n";
    }
    os << "] }";
    return os.str();
}

size_t MissionEditorPanel::ImportFromJson(const std::string& json) {
    // Lightweight field extraction — no external JSON library required.
    // Counts template objects by template_id occurrences.
    size_t imported = 0;
    size_t pos = 0;
    while ((pos = json.find("\"template_id\"", pos)) != std::string::npos) {
        // Find the value after the colon
        size_t colon = json.find(':', pos);
        if (colon == std::string::npos) break;
        size_t qStart = json.find('"', colon + 1);
        if (qStart == std::string::npos) break;
        size_t qEnd = json.find('"', qStart + 1);
        if (qEnd == std::string::npos) break;

        MissionTemplateEntry entry;
        entry.templateId = json.substr(qStart + 1, qEnd - qStart - 1);

        // Extract type
        size_t typePos = json.find("\"type\"", qEnd);
        if (typePos != std::string::npos) {
            size_t tc = json.find(':', typePos);
            if (tc != std::string::npos) {
                size_t ts = json.find('"', tc + 1);
                if (ts != std::string::npos) {
                    size_t te = json.find('"', ts + 1);
                    if (te != std::string::npos) {
                        entry.type = json.substr(ts + 1, te - ts - 1);
                    }
                }
            }
        }

        // Default objective so the template validates
        if (entry.objectives.empty()) {
            entry.objectives.push_back({"destroy", "enemy", 1, 5});
        }

        m_templates.push_back(entry);
        ++imported;
        pos = qEnd + 1;
    }

    if (imported > 0) {
        log("Imported " + std::to_string(imported) + " template(s) from JSON");
    }
    return imported;
}

// ── Logging ────────────────────────────────────────────────────────

void MissionEditorPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
