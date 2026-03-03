#include "AssetPalettePanel.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

AssetPalettePanel::AssetPalettePanel() {
    log("Asset Palette initialized — " + std::to_string(kCategoryCount)
        + " categories available");
}

// ── Draw ───────────────────────────────────────────────────────────

void AssetPalettePanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Asset Palette", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad  = ctx.theme().padding;
    const float rowH = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Category filter buttons
    atlas::label(ctx, {b.x + pad, y}, "Category:", ctx.theme().textPrimary);
    y += rowH;

    float btnX = b.x + pad;
    const float btnW = 80.0f;

    // "All" button to clear category filter
    if (atlas::button(ctx, "All", {btnX, y, btnW, rowH})) {
        SetCategoryFilter("");
    }
    btnX += btnW + pad;

    for (int i = 0; i < kCategoryCount; ++i) {
        if (btnX + btnW > b.x + b.w - pad) {
            btnX = b.x + pad;
            y += rowH + pad;
        }
        if (atlas::button(ctx, kCategories[i], {btnX, y, btnW, rowH})) {
            SetCategoryFilter(kCategories[i]);
        }
        btnX += btnW + pad;
    }
    y += rowH + pad + pad;

    // Asset count + filter info
    std::string info = "Assets: " + std::to_string(m_assets.size());
    if (!m_categoryFilter.empty() || !m_searchFilter.empty()) {
        info += " (filtered: " + std::to_string(FilteredCount()) + ")";
    }
    atlas::label(ctx, {b.x + pad, y}, info, ctx.theme().textSecondary);
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Selected asset details
    if (m_selectedIndex >= 0 &&
        m_selectedIndex < static_cast<int>(m_assets.size())) {
        const auto& a = m_assets[m_selectedIndex];
        atlas::label(ctx, {b.x + pad, y},
            "Selected: " + a.name, ctx.theme().textPrimary);
        y += rowH + pad;

        atlas::label(ctx, {b.x + pad, y},
            "  ID: " + a.assetId, ctx.theme().textSecondary);
        y += rowH;
        atlas::label(ctx, {b.x + pad, y},
            "  Category: " + a.category + " / " + a.subcategory,
            ctx.theme().textSecondary);
        y += rowH;
        atlas::label(ctx, {b.x + pad, y},
            "  Tags: " + a.tags, ctx.theme().textSecondary);
        y += rowH;
        atlas::label(ctx, {b.x + pad, y},
            std::string("  Prefab: ") + (a.isPrefab ? "yes" : "no"),
            ctx.theme().textSecondary);
        y += rowH;
        atlas::label(ctx, {b.x + pad, y},
            "  Scale: " + std::to_string(a.previewScale),
            ctx.theme().textSecondary);
        y += rowH + pad;
    }

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Asset management ──────────────────────────────────────────────

size_t AssetPalettePanel::AddAsset(const AssetEntry& entry) {
    m_assets.push_back(entry);
    log("Added asset: " + entry.name);
    return m_assets.size() - 1;
}

bool AssetPalettePanel::RemoveAsset(size_t index) {
    if (index >= m_assets.size()) return false;
    std::string id = m_assets[index].assetId;
    m_assets.erase(m_assets.begin() + static_cast<ptrdiff_t>(index));

    if (m_selectedIndex == static_cast<int>(index)) {
        m_selectedIndex = -1;
    } else if (m_selectedIndex > static_cast<int>(index)) {
        --m_selectedIndex;
    }

    log("Removed asset: " + id);
    return true;
}

// ── Selection ─────────────────────────────────────────────────────

void AssetPalettePanel::SelectAsset(int index) {
    if (index < 0 || index >= static_cast<int>(m_assets.size())) return;
    m_selectedIndex = index;
    log("Selected: " + m_assets[index].name);
}

void AssetPalettePanel::ClearSelection() {
    m_selectedIndex = -1;
}

// ── Filtering ─────────────────────────────────────────────────────

void AssetPalettePanel::SetCategoryFilter(const std::string& category) {
    m_categoryFilter = category;
}

void AssetPalettePanel::SetSearchFilter(const std::string& search) {
    m_searchFilter = search;
}

size_t AssetPalettePanel::FilteredCount() const {
    size_t count = 0;
    for (const auto& a : m_assets) {
        if (matchesFilters(a)) ++count;
    }
    return count;
}

bool AssetPalettePanel::matchesFilters(const AssetEntry& entry) const {
    // Category filter: exact match when set
    if (!m_categoryFilter.empty() && entry.category != m_categoryFilter)
        return false;

    // Search filter: case-insensitive substring on name, tags, subcategory
    if (!m_searchFilter.empty()) {
        auto toLower = [](const std::string& s) {
            std::string out = s;
            std::transform(out.begin(), out.end(), out.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            return out;
        };
        std::string lowerSearch = toLower(m_searchFilter);
        if (toLower(entry.name).find(lowerSearch) == std::string::npos &&
            toLower(entry.tags).find(lowerSearch) == std::string::npos &&
            toLower(entry.subcategory).find(lowerSearch) == std::string::npos)
            return false;
    }

    return true;
}

// ── Prefab support ────────────────────────────────────────────────

bool AssetPalettePanel::SaveAsPrefab(size_t index) {
    if (index >= m_assets.size()) return false;
    m_assets[index].isPrefab = true;
    log("Saved as prefab: " + m_assets[index].name);
    return true;
}

size_t AssetPalettePanel::PrefabCount() const {
    size_t count = 0;
    for (const auto& a : m_assets) {
        if (a.isPrefab) ++count;
    }
    return count;
}

// ── Export / Import ───────────────────────────────────────────────

static std::string jsonEscapeAP(const std::string& s) {
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

std::string AssetPalettePanel::ExportToJson() const {
    std::ostringstream os;
    os << "{ \"assetPalette\": {\n";
    for (size_t i = 0; i < m_assets.size(); ++i) {
        const auto& a = m_assets[i];
        os << "  \"" << jsonEscapeAP(a.assetId) << "\": {";
        os << " \"name\": \"" << jsonEscapeAP(a.name) << "\",";
        os << " \"category\": \"" << jsonEscapeAP(a.category) << "\",";
        os << " \"subcategory\": \"" << jsonEscapeAP(a.subcategory) << "\",";
        os << " \"tags\": \"" << jsonEscapeAP(a.tags) << "\",";
        os << " \"isPrefab\": " << (a.isPrefab ? "true" : "false") << ",";
        os << " \"previewScale\": " << a.previewScale;
        os << " }";
        if (i + 1 < m_assets.size()) os << ",";
        os << "\n";
    }
    os << "} }";
    return os.str();
}

size_t AssetPalettePanel::ImportFromJson(const std::string& json) {
    size_t imported = 0;
    size_t pos = 0;

    // Skip past the wrapper object ("assetPalette")
    size_t firstBrace = json.find('{');
    if (firstBrace == std::string::npos) return 0;
    size_t secondBrace = json.find('{', firstBrace + 1);
    if (secondBrace == std::string::npos) secondBrace = firstBrace;

    pos = secondBrace + 1;
    while (pos < json.size()) {
        // Find next quoted key (asset id)
        size_t qStart = json.find('"', pos);
        if (qStart == std::string::npos) break;
        size_t qEnd = json.find('"', qStart + 1);
        if (qEnd == std::string::npos) break;

        std::string assetId = json.substr(qStart + 1, qEnd - qStart - 1);
        if (assetId.empty()) { pos = qEnd + 1; continue; }

        // Find the object body
        size_t objStart = json.find('{', qEnd);
        if (objStart == std::string::npos) break;

        // Simple brace matching
        int depth = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < json.size() && depth > 0) {
            if (json[objEnd] == '{') ++depth;
            else if (json[objEnd] == '}') --depth;
            ++objEnd;
        }

        std::string body = json.substr(objStart + 1, objEnd - objStart - 2);

        AssetEntry entry;
        entry.assetId = assetId;

        // Extract known fields from the object body
        size_t fpos = 0;
        while (fpos < body.size()) {
            size_t fks = body.find('"', fpos);
            if (fks == std::string::npos) break;
            size_t fke = body.find('"', fks + 1);
            if (fke == std::string::npos) break;
            std::string fkey = body.substr(fks + 1, fke - fks - 1);

            size_t colon = body.find(':', fke);
            if (colon == std::string::npos) break;

            size_t vs = colon + 1;
            while (vs < body.size() && body[vs] == ' ') ++vs;

            if (vs < body.size() && body[vs] == '"') {
                // String value
                size_t ve = body.find('"', vs + 1);
                if (ve == std::string::npos) break;
                std::string val = body.substr(vs + 1, ve - vs - 1);

                if (fkey == "name")        entry.name = val;
                else if (fkey == "category")    entry.category = val;
                else if (fkey == "subcategory") entry.subcategory = val;
                else if (fkey == "tags")        entry.tags = val;

                fpos = ve + 1;
            } else {
                // Number or boolean
                size_t ve = vs;
                while (ve < body.size() && body[ve] != ',' && body[ve] != '}')
                    ++ve;
                std::string raw = body.substr(vs, ve - vs);
                while (!raw.empty() && (raw.back() == ' ' || raw.back() == '\n'))
                    raw.pop_back();

                if (fkey == "isPrefab") {
                    entry.isPrefab = (raw == "true");
                } else if (fkey == "previewScale") {
                    try { entry.previewScale = std::stof(raw); }
                    catch (...) { entry.previewScale = 1.0f; }
                }

                fpos = ve;
            }
        }

        m_assets.push_back(entry);
        ++imported;
        pos = objEnd;
    }

    if (imported > 0) {
        log("Imported " + std::to_string(imported) + " assets from JSON");
    }
    return imported;
}

// ── Logging ───────────────────────────────────────────────────────

void AssetPalettePanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
