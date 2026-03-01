#include "ModuleEditorPanel.h"
#include <sstream>
#include <algorithm>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

ModuleEditorPanel::ModuleEditorPanel() {
    log("Module Editor initialized");
}

// ── Draw ───────────────────────────────────────────────────────────

void ModuleEditorPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    if (!atlas::panelBeginStateful(ctx, "Module Editor", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad  = ctx.theme().padding;
    const float rowH = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Module count
    atlas::label(ctx, {b.x + pad, y},
        "Modules: " + std::to_string(m_modules.size()),
        ctx.theme().textPrimary);
    y += rowH + pad;

    // Filter info
    std::string filterInfo = "Filter: ";
    if (!m_typeFilter.empty()) filterInfo += "type=" + m_typeFilter + " ";
    if (!m_slotFilter.empty()) filterInfo += "slot=" + m_slotFilter;
    if (m_typeFilter.empty() && m_slotFilter.empty()) filterInfo += "(none)";
    atlas::label(ctx, {b.x + pad, y}, filterInfo, ctx.theme().textSecondary);
    y += rowH + pad;

    // Type filter buttons
    const float btnW = 65.0f;
    float btnX = b.x + pad;
    for (const char* t : {"weapon", "defense", "utility", "mining"}) {
        if (atlas::button(ctx, t, {btnX, y, btnW, rowH})) {
            SetTypeFilter(m_typeFilter == t ? "" : t);
        }
        btnX += btnW + pad;
    }
    y += rowH + pad;

    // Validate button
    if (atlas::button(ctx, "Validate All", {b.x + pad, y, 100.0f, rowH + pad})) {
        ValidateAll();
    }
    y += rowH + pad + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Selected module info
    if (m_selectedIndex >= 0 &&
        m_selectedIndex < static_cast<int>(m_modules.size())) {
        const auto& m = m_modules[m_selectedIndex];
        atlas::label(ctx, {b.x + pad, y},
            "Selected: " + m.name + " [" + m.moduleId + "]",
            ctx.theme().textPrimary);
        y += rowH + pad;
        atlas::label(ctx, {b.x + pad, y},
            "  Type: " + m.type + "  Slot: " + m.slot
            + "  Meta: " + std::to_string(m.metaLevel),
            ctx.theme().textSecondary);
        y += rowH + pad;
        if (m.type == "weapon") {
            atlas::label(ctx, {b.x + pad, y},
                "  Dmg: " + std::to_string(static_cast<int>(m.damage))
                + " " + m.damageType
                + "  ROF: " + std::to_string(m.rateOfFire) + "s"
                + "  Opt: " + std::to_string(static_cast<int>(m.optimalRange))
                + "  Fall: " + std::to_string(static_cast<int>(m.falloffRange)),
                ctx.theme().textSecondary);
            y += rowH + pad;
        }
        atlas::label(ctx, {b.x + pad, y},
            "  CPU: " + std::to_string(m.cpu)
            + "  PG: " + std::to_string(m.powergrid)
            + "  Cap: " + std::to_string(m.capacitorUse),
            ctx.theme().textSecondary);
        y += rowH + pad;
    }

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Module management ─────────────────────────────────────────────

size_t ModuleEditorPanel::AddModule(const ModuleEntry& entry) {
    m_modules.push_back(entry);
    log("Added module: " + entry.name + " (" + entry.type + "/" + entry.slot + ")");
    return m_modules.size() - 1;
}

bool ModuleEditorPanel::RemoveModule(size_t index) {
    if (index >= m_modules.size()) return false;
    std::string name = m_modules[index].name;
    m_modules.erase(m_modules.begin() + static_cast<ptrdiff_t>(index));

    if (m_selectedIndex == static_cast<int>(index)) {
        m_selectedIndex = -1;
    } else if (m_selectedIndex > static_cast<int>(index)) {
        --m_selectedIndex;
    }

    log("Removed module: " + name);
    return true;
}

bool ModuleEditorPanel::UpdateModule(size_t index, const ModuleEntry& entry) {
    if (index >= m_modules.size()) return false;
    m_modules[index] = entry;
    log("Updated module: " + entry.name);
    return true;
}

// ── Selection ─────────────────────────────────────────────────────

void ModuleEditorPanel::SelectModule(int index) {
    if (index < 0 || index >= static_cast<int>(m_modules.size())) return;
    m_selectedIndex = index;
    log("Selected module [" + std::to_string(index) + "]: "
        + m_modules[index].name);
}

void ModuleEditorPanel::ClearSelection() {
    m_selectedIndex = -1;
}

// ── Filtering ─────────────────────────────────────────────────────

void ModuleEditorPanel::SetTypeFilter(const std::string& type) {
    m_typeFilter = type;
}

void ModuleEditorPanel::SetSlotFilter(const std::string& slot) {
    m_slotFilter = slot;
}

size_t ModuleEditorPanel::FilteredCount() const {
    size_t count = 0;
    for (const auto& m : m_modules) {
        if (matchesFilter(m)) ++count;
    }
    return count;
}

bool ModuleEditorPanel::matchesFilter(const ModuleEntry& entry) const {
    if (!m_typeFilter.empty() && entry.type != m_typeFilter) return false;
    if (!m_slotFilter.empty() && entry.slot != m_slotFilter) return false;
    return true;
}

// ── Validation ────────────────────────────────────────────────────

bool ModuleEditorPanel::ValidateModule(const ModuleEntry& entry,
                                        std::string& errorOut) {
    if (entry.moduleId.empty()) {
        errorOut = "Module ID is empty";
        return false;
    }
    if (entry.name.empty()) {
        errorOut = "Module name is empty";
        return false;
    }
    if (entry.type.empty()) {
        errorOut = "Module type is empty";
        return false;
    }
    if (entry.slot.empty()) {
        errorOut = "Slot type is empty";
        return false;
    }
    if (entry.cpu < 0) {
        errorOut = "CPU must be non-negative";
        return false;
    }
    if (entry.powergrid < 0) {
        errorOut = "Powergrid must be non-negative";
        return false;
    }
    if (entry.type == "weapon") {
        if (entry.damage < 0.0f) {
            errorOut = "Weapon damage must be non-negative";
            return false;
        }
        if (entry.rateOfFire <= 0.0f) {
            errorOut = "Rate of fire must be positive";
            return false;
        }
    }
    errorOut.clear();
    return true;
}

size_t ModuleEditorPanel::ValidateAll() {
    size_t invalid = 0;
    for (size_t i = 0; i < m_modules.size(); ++i) {
        std::string err;
        if (!ValidateModule(m_modules[i], err)) {
            ++invalid;
            log("INVALID [" + std::to_string(i) + "] "
                + m_modules[i].name + ": " + err);
        }
    }
    if (invalid == 0) {
        log("All " + std::to_string(m_modules.size()) + " modules valid");
    } else {
        log(std::to_string(invalid) + " of "
            + std::to_string(m_modules.size()) + " modules invalid");
    }
    return invalid;
}

// ── Export / Import ───────────────────────────────────────────────

static std::string jsonEscapeME(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) { /* skip */ }
            else { out += c; }
            break;
        }
    }
    return out;
}

std::string ModuleEditorPanel::ExportToJson() const {
    std::ostringstream os;
    os << "{ \"modules\": {\n";
    for (size_t i = 0; i < m_modules.size(); ++i) {
        const auto& m = m_modules[i];
        os << "  \"" << jsonEscapeME(m.moduleId) << "\": {"
           << " \"name\": \"" << jsonEscapeME(m.name) << "\","
           << " \"type\": \"" << jsonEscapeME(m.type) << "\","
           << " \"slot\": \"" << jsonEscapeME(m.slot) << "\","
           << " \"cpu\": " << m.cpu << ","
           << " \"powergrid\": " << m.powergrid << ","
           << " \"capacitor_use\": " << m.capacitorUse << ","
           << " \"meta_level\": " << m.metaLevel << ","
           << " \"tech_level\": " << m.techLevel;
        if (m.type == "weapon") {
            os << ", \"weapon_type\": \"" << jsonEscapeME(m.weaponType) << "\""
               << ", \"damage\": " << m.damage
               << ", \"damage_type\": \"" << jsonEscapeME(m.damageType) << "\""
               << ", \"rate_of_fire\": " << m.rateOfFire
               << ", \"optimal_range\": " << m.optimalRange
               << ", \"falloff_range\": " << m.falloffRange
               << ", \"tracking\": " << m.tracking;
        }
        if (m.type == "defense") {
            os << ", \"shield_bonus\": " << m.shieldBonus
               << ", \"armor_bonus\": " << m.armorBonus
               << ", \"resist_bonus\": " << m.resistBonus;
        }
        os << " }";
        if (i + 1 < m_modules.size()) os << ",";
        os << "\n";
    }
    os << "} }";
    return os.str();
}

size_t ModuleEditorPanel::ImportFromJson(const std::string& json) {
    size_t imported = 0;
    size_t pos = 0;
    while ((pos = json.find("\"id\"", pos)) != std::string::npos) {
        size_t colon = json.find(':', pos);
        if (colon == std::string::npos) break;
        size_t qStart = json.find('"', colon + 1);
        if (qStart == std::string::npos) break;
        size_t qEnd = json.find('"', qStart + 1);
        if (qEnd == std::string::npos) break;

        ModuleEntry entry;
        entry.moduleId = json.substr(qStart + 1, qEnd - qStart - 1);

        // Extract name
        size_t nPos = json.find("\"name\"", qEnd);
        if (nPos != std::string::npos && nPos < qEnd + 500) {
            size_t nc = json.find(':', nPos);
            if (nc != std::string::npos) {
                size_t ns = json.find('"', nc + 1);
                if (ns != std::string::npos) {
                    size_t ne = json.find('"', ns + 1);
                    if (ne != std::string::npos)
                        entry.name = json.substr(ns + 1, ne - ns - 1);
                }
            }
        }

        // Extract type
        size_t tPos = json.find("\"type\"", qEnd);
        if (tPos != std::string::npos && tPos < qEnd + 500) {
            size_t tc = json.find(':', tPos);
            if (tc != std::string::npos) {
                size_t ts = json.find('"', tc + 1);
                if (ts != std::string::npos) {
                    size_t te = json.find('"', ts + 1);
                    if (te != std::string::npos)
                        entry.type = json.substr(ts + 1, te - ts - 1);
                }
            }
        }

        if (entry.name.empty()) entry.name = entry.moduleId;
        if (entry.type.empty()) entry.type = "utility";
        if (entry.slot.empty()) entry.slot = "mid";

        m_modules.push_back(entry);
        ++imported;
        pos = qEnd + 1;
    }

    if (imported > 0)
        log("Imported " + std::to_string(imported) + " module(s) from JSON");
    return imported;
}

// ── Logging ───────────────────────────────────────────────────────

void ModuleEditorPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
