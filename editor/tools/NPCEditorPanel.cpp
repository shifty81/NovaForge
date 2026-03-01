#include "NPCEditorPanel.h"
#include <sstream>
#include <algorithm>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

NPCEditorPanel::NPCEditorPanel() {
    log("NPC Editor initialized");
}

// ── Draw ───────────────────────────────────────────────────────────

void NPCEditorPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    if (!atlas::panelBeginStateful(ctx, "NPC Editor", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad  = ctx.theme().padding;
    const float rowH = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // NPC count
    atlas::label(ctx, {b.x + pad, y},
        "NPCs: " + std::to_string(m_npcs.size()),
        ctx.theme().textPrimary);
    y += rowH + pad;

    // Filter info
    std::string filterInfo = "Filter: ";
    if (!m_factionFilter.empty()) filterInfo += "faction=" + m_factionFilter + " ";
    if (!m_archetypeFilter.empty()) filterInfo += "archetype=" + m_archetypeFilter;
    if (m_factionFilter.empty() && m_archetypeFilter.empty()) filterInfo += "(none)";
    atlas::label(ctx, {b.x + pad, y}, filterInfo, ctx.theme().textSecondary);
    y += rowH + pad;

    // Archetype filter buttons
    const float btnW = 60.0f;
    float btnX = b.x + pad;
    for (const char* a : {"pirate", "trader", "miner", "hauler", "patrol"}) {
        if (btnX + btnW > b.x + b.w - pad) {
            btnX = b.x + pad;
            y += rowH + pad;
        }
        if (atlas::button(ctx, a, {btnX, y, btnW, rowH})) {
            SetArchetypeFilter(m_archetypeFilter == a ? "" : a);
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

    // Selected NPC info
    if (m_selectedIndex >= 0 &&
        m_selectedIndex < static_cast<int>(m_npcs.size())) {
        const auto& n = m_npcs[m_selectedIndex];
        atlas::label(ctx, {b.x + pad, y},
            "Selected: " + n.name + " [" + n.npcId + "]",
            ctx.theme().textPrimary);
        y += rowH + pad;
        atlas::label(ctx, {b.x + pad, y},
            "  Type: " + n.type + "  Faction: " + n.faction
            + "  Archetype: " + n.archetype,
            ctx.theme().textSecondary);
        y += rowH + pad;
        atlas::label(ctx, {b.x + pad, y},
            "  HP: " + std::to_string(static_cast<int>(n.shieldHp)) + "S/"
            + std::to_string(static_cast<int>(n.armorHp)) + "A/"
            + std::to_string(static_cast<int>(n.hullHp)) + "H"
            + "  Vel: " + std::to_string(static_cast<int>(n.maxVelocity)),
            ctx.theme().textSecondary);
        y += rowH + pad;
        atlas::label(ctx, {b.x + pad, y},
            "  Bounty: " + std::to_string(static_cast<int>(n.bounty))
            + "  Wallet: " + std::to_string(static_cast<int>(n.startingWallet))
            + "  Ship: " + n.ownedShipType,
            ctx.theme().textSecondary);
        y += rowH + pad;
        atlas::label(ctx, {b.x + pad, y},
            "  Behavior: " + n.behavior
            + "  Awareness: " + std::to_string(static_cast<int>(n.awarenessRange))
            + "m  Flee: " + std::to_string(static_cast<int>(n.fleeThreshold * 100)) + "%",
            ctx.theme().textSecondary);
        y += rowH + pad;
        if (!n.haulStationId.empty()) {
            atlas::label(ctx, {b.x + pad, y},
                "  Haul Station: " + n.haulStationId, ctx.theme().textSecondary);
            y += rowH + pad;
        }
        atlas::label(ctx, {b.x + pad, y},
            "  Weapons: " + std::to_string(n.weapons.size())
            + "  Loot items: " + std::to_string(n.lootTable.size()),
            ctx.theme().textSecondary);
        y += rowH + pad;
    }

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── NPC management ────────────────────────────────────────────────

size_t NPCEditorPanel::AddNPC(const NPCEntry& entry) {
    m_npcs.push_back(entry);
    log("Added NPC: " + entry.name + " (" + entry.archetype + ")");
    return m_npcs.size() - 1;
}

bool NPCEditorPanel::RemoveNPC(size_t index) {
    if (index >= m_npcs.size()) return false;
    std::string name = m_npcs[index].name;
    m_npcs.erase(m_npcs.begin() + static_cast<ptrdiff_t>(index));

    if (m_selectedIndex == static_cast<int>(index)) {
        m_selectedIndex = -1;
    } else if (m_selectedIndex > static_cast<int>(index)) {
        --m_selectedIndex;
    }

    log("Removed NPC: " + name);
    return true;
}

bool NPCEditorPanel::UpdateNPC(size_t index, const NPCEntry& entry) {
    if (index >= m_npcs.size()) return false;
    m_npcs[index] = entry;
    log("Updated NPC: " + entry.name);
    return true;
}

// ── Selection ─────────────────────────────────────────────────────

void NPCEditorPanel::SelectNPC(int index) {
    if (index < 0 || index >= static_cast<int>(m_npcs.size())) return;
    m_selectedIndex = index;
    log("Selected NPC [" + std::to_string(index) + "]: "
        + m_npcs[index].name);
}

void NPCEditorPanel::ClearSelection() {
    m_selectedIndex = -1;
}

// ── Filtering ─────────────────────────────────────────────────────

void NPCEditorPanel::SetFactionFilter(const std::string& faction) {
    m_factionFilter = faction;
}

void NPCEditorPanel::SetArchetypeFilter(const std::string& archetype) {
    m_archetypeFilter = archetype;
}

size_t NPCEditorPanel::FilteredCount() const {
    size_t count = 0;
    for (const auto& n : m_npcs) {
        if (matchesFilter(n)) ++count;
    }
    return count;
}

bool NPCEditorPanel::matchesFilter(const NPCEntry& entry) const {
    if (!m_factionFilter.empty() && entry.faction != m_factionFilter)
        return false;
    if (!m_archetypeFilter.empty() && entry.archetype != m_archetypeFilter)
        return false;
    return true;
}

// ── Validation ────────────────────────────────────────────────────

bool NPCEditorPanel::ValidateNPC(const NPCEntry& entry, std::string& errorOut) {
    if (entry.npcId.empty()) {
        errorOut = "NPC ID is empty";
        return false;
    }
    if (entry.name.empty()) {
        errorOut = "NPC name is empty";
        return false;
    }
    if (entry.type.empty()) {
        errorOut = "NPC ship type is empty";
        return false;
    }
    if (entry.hullHp <= 0.0f && entry.armorHp <= 0.0f && entry.shieldHp <= 0.0f) {
        errorOut = "NPC has zero total HP";
        return false;
    }
    if (entry.archetype.empty()) {
        errorOut = "NPC archetype is empty";
        return false;
    }
    if (entry.archetype == "miner" || entry.archetype == "hauler") {
        if (entry.startingWallet <= 0.0) {
            errorOut = "Miner/hauler NPC needs a positive starting wallet";
            return false;
        }
    }
    errorOut.clear();
    return true;
}

size_t NPCEditorPanel::ValidateAll() {
    size_t invalid = 0;
    for (size_t i = 0; i < m_npcs.size(); ++i) {
        std::string err;
        if (!ValidateNPC(m_npcs[i], err)) {
            ++invalid;
            log("INVALID [" + std::to_string(i) + "] "
                + m_npcs[i].name + ": " + err);
        }
    }
    if (invalid == 0) {
        log("All " + std::to_string(m_npcs.size()) + " NPCs valid");
    } else {
        log(std::to_string(invalid) + " of "
            + std::to_string(m_npcs.size()) + " NPCs invalid");
    }
    return invalid;
}

// ── Export / Import ───────────────────────────────────────────────

static std::string jsonEscapeNPC(const std::string& s) {
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

std::string NPCEditorPanel::ExportToJson() const {
    std::ostringstream os;
    os << "{ \"npcs\": {\n";
    for (size_t i = 0; i < m_npcs.size(); ++i) {
        const auto& n = m_npcs[i];
        os << "  \"" << jsonEscapeNPC(n.npcId) << "\": {"
           << " \"name\": \"" << jsonEscapeNPC(n.name) << "\","
           << " \"type\": \"" << jsonEscapeNPC(n.type) << "\","
           << " \"faction\": \"" << jsonEscapeNPC(n.faction) << "\","
           << " \"behavior\": \"" << jsonEscapeNPC(n.behavior) << "\","
           << " \"archetype\": \"" << jsonEscapeNPC(n.archetype) << "\","
           << " \"hull_hp\": " << n.hullHp << ","
           << " \"armor_hp\": " << n.armorHp << ","
           << " \"shield_hp\": " << n.shieldHp << ","
           << " \"max_velocity\": " << n.maxVelocity << ","
           << " \"orbit_distance\": " << n.orbitDistance << ","
           << " \"bounty\": " << n.bounty << ","
           << " \"starting_wallet\": " << n.startingWallet << ","
           << " \"owned_ship_type\": \"" << jsonEscapeNPC(n.ownedShipType) << "\","
           << " \"ship_value\": " << n.shipValue << ","
           << " \"awareness_range\": " << n.awarenessRange << ","
           << " \"flee_threshold\": " << n.fleeThreshold << ","
           << " \"target_selection\": \"" << jsonEscapeNPC(n.targetSelection) << "\","
           << " \"haul_station_id\": \"" << jsonEscapeNPC(n.haulStationId) << "\","
           << " \"weapons\": [";
        for (size_t j = 0; j < n.weapons.size(); ++j) {
            os << "{ \"type\": \"" << jsonEscapeNPC(n.weapons[j].type) << "\","
               << " \"damage\": " << n.weapons[j].damage << " }";
            if (j + 1 < n.weapons.size()) os << ", ";
        }
        os << "],"
           << " \"loot_table\": [";
        for (size_t j = 0; j < n.lootTable.size(); ++j) {
            os << "\"" << jsonEscapeNPC(n.lootTable[j]) << "\"";
            if (j + 1 < n.lootTable.size()) os << ", ";
        }
        os << "] }";
        if (i + 1 < m_npcs.size()) os << ",";
        os << "\n";
    }
    os << "} }";
    return os.str();
}

size_t NPCEditorPanel::ImportFromJson(const std::string& json) {
    size_t imported = 0;
    size_t pos = 0;
    while ((pos = json.find("\"id\"", pos)) != std::string::npos) {
        size_t colon = json.find(':', pos);
        if (colon == std::string::npos) break;
        size_t qStart = json.find('"', colon + 1);
        if (qStart == std::string::npos) break;
        size_t qEnd = json.find('"', qStart + 1);
        if (qEnd == std::string::npos) break;

        NPCEntry entry;
        entry.npcId = json.substr(qStart + 1, qEnd - qStart - 1);

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

        if (entry.name.empty()) entry.name = entry.npcId;
        entry.type = "frigate";
        entry.archetype = "pirate";
        entry.behavior = "aggressive";
        entry.hullHp = 300.0f;
        entry.armorHp = 250.0f;
        entry.shieldHp = 350.0f;

        m_npcs.push_back(entry);
        ++imported;
        pos = qEnd + 1;
    }

    if (imported > 0)
        log("Imported " + std::to_string(imported) + " NPC(s) from JSON");
    return imported;
}

// ── Logging ───────────────────────────────────────────────────────

void NPCEditorPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
