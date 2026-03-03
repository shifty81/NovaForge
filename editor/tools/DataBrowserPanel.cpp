#include "DataBrowserPanel.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

DataBrowserPanel::DataBrowserPanel() {
    log("Data Browser initialized — " + std::to_string(kCategoryCount)
        + " categories available");
}

// ── Draw ───────────────────────────────────────────────────────────

void DataBrowserPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Data Browser", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad  = ctx.theme().padding;
    const float rowH = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Category selector
    atlas::label(ctx, {b.x + pad, y}, "Category:", ctx.theme().textPrimary);
    y += rowH;
    if (m_currentCategory.empty()) {
        atlas::label(ctx, {b.x + pad, y}, "(none loaded)",
                     ctx.theme().textSecondary);
    } else {
        atlas::label(ctx, {b.x + pad, y}, m_currentCategory,
                     ctx.theme().textPrimary);
    }

    // Category buttons row
    y += rowH + pad;
    float btnX = b.x + pad;
    const float btnW = 80.0f;
    for (int i = 0; i < kCategoryCount; ++i) {
        if (btnX + btnW > b.x + b.w - pad) {
            btnX = b.x + pad;
            y += rowH + pad;
        }
        if (atlas::button(ctx, kCategories[i], {btnX, y, btnW, rowH})) {
            LoadCategory(kCategories[i]);
        }
        btnX += btnW + pad;
    }
    y += rowH + pad + pad;

    // Entry count + filter info
    atlas::label(ctx, {b.x + pad, y},
        "Entries: " + std::to_string(m_entries.size())
        + (m_filter.empty() ? "" : " (filtered: "
           + std::to_string(FilteredCount()) + ")"),
        ctx.theme().textSecondary);
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Selected entry details
    if (m_selectedIndex >= 0 &&
        m_selectedIndex < static_cast<int>(m_entries.size())) {
        const auto& e = m_entries[m_selectedIndex];
        atlas::label(ctx, {b.x + pad, y},
            "Selected: " + e.id, ctx.theme().textPrimary);
        y += rowH + pad;

        // Show fields (up to available space)
        int fieldsShown = 0;
        for (const auto& f : e.fields) {
            if (y + rowH > b.y + b.h - pad * 4) break;
            std::string line = "  " + f.key + ": " + f.value;
            atlas::label(ctx, {b.x + pad, y}, line, ctx.theme().textSecondary);
            y += rowH;
            ++fieldsShown;
        }
        if (fieldsShown < static_cast<int>(e.fields.size())) {
            atlas::label(ctx, {b.x + pad, y},
                "  ... +" + std::to_string(e.fields.size() - fieldsShown)
                + " more fields",
                ctx.theme().textSecondary);
            y += rowH;
        }
    }

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Category loading ──────────────────────────────────────────────

void DataBrowserPanel::LoadCategory(const std::string& category) {
    m_currentCategory = category;
    m_entries.clear();
    m_selectedIndex = -1;
    log("Loaded category: " + category);
}

// ── Entry management ──────────────────────────────────────────────

size_t DataBrowserPanel::AddEntry(const DataEntry& entry) {
    m_entries.push_back(entry);
    log("Added entry: " + entry.id);
    return m_entries.size() - 1;
}

bool DataBrowserPanel::RemoveEntry(size_t index) {
    if (index >= m_entries.size()) return false;
    std::string id = m_entries[index].id;
    m_entries.erase(m_entries.begin() + static_cast<ptrdiff_t>(index));

    if (m_selectedIndex == static_cast<int>(index)) {
        m_selectedIndex = -1;
    } else if (m_selectedIndex > static_cast<int>(index)) {
        --m_selectedIndex;
    }

    log("Removed entry: " + id);
    return true;
}

bool DataBrowserPanel::UpdateEntryField(size_t entryIndex, size_t fieldIndex,
                                         const std::string& newValue) {
    if (entryIndex >= m_entries.size()) return false;
    auto& entry = m_entries[entryIndex];
    if (fieldIndex >= entry.fields.size()) return false;

    std::string old = entry.fields[fieldIndex].value;
    entry.fields[fieldIndex].value = newValue;
    log("Updated " + entry.id + "." + entry.fields[fieldIndex].key
        + ": " + old + " -> " + newValue);
    return true;
}

// ── Selection ─────────────────────────────────────────────────────

void DataBrowserPanel::SelectEntry(int index) {
    if (index < 0 || index >= static_cast<int>(m_entries.size())) return;
    m_selectedIndex = index;
    log("Selected: " + m_entries[index].id);
}

void DataBrowserPanel::ClearSelection() {
    m_selectedIndex = -1;
}

// ── Filtering ─────────────────────────────────────────────────────

void DataBrowserPanel::SetFilter(const std::string& filter) {
    m_filter = filter;
}

size_t DataBrowserPanel::FilteredCount() const {
    size_t count = 0;
    for (const auto& e : m_entries) {
        if (matchesFilter(e)) ++count;
    }
    return count;
}

bool DataBrowserPanel::matchesFilter(const DataEntry& entry) const {
    if (m_filter.empty()) return true;
    // Case-insensitive substring match on entry id
    std::string lower_id = entry.id;
    std::string lower_filter = m_filter;
    std::transform(lower_id.begin(), lower_id.end(), lower_id.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    std::transform(lower_filter.begin(), lower_filter.end(), lower_filter.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lower_id.find(lower_filter) != std::string::npos;
}

// ── Export / Import ───────────────────────────────────────────────

static std::string jsonEscapeDB(const std::string& s) {
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

std::string DataBrowserPanel::ExportToJson() const {
    std::ostringstream os;
    os << "{ \"" << jsonEscapeDB(m_currentCategory) << "\": {\n";
    for (size_t i = 0; i < m_entries.size(); ++i) {
        const auto& e = m_entries[i];
        os << "  \"" << jsonEscapeDB(e.id) << "\": {";
        for (size_t j = 0; j < e.fields.size(); ++j) {
            const auto& f = e.fields[j];
            os << " \"" << jsonEscapeDB(f.key) << "\": ";
            if (f.fieldType == DataField::Type::Number ||
                f.fieldType == DataField::Type::Boolean) {
                os << f.value;
            } else {
                os << "\"" << jsonEscapeDB(f.value) << "\"";
            }
            if (j + 1 < e.fields.size()) os << ",";
        }
        os << " }";
        if (i + 1 < m_entries.size()) os << ",";
        os << "\n";
    }
    os << "} }";
    return os.str();
}

size_t DataBrowserPanel::ImportFromJson(const std::string& json) {
    // Lightweight parser: find quoted keys at the top object level
    size_t imported = 0;
    size_t pos = 0;

    // Skip past the category wrapper object
    size_t firstBrace = json.find('{');
    if (firstBrace == std::string::npos) return 0;
    size_t secondBrace = json.find('{', firstBrace + 1);
    if (secondBrace == std::string::npos) secondBrace = firstBrace;

    pos = secondBrace + 1;
    while (pos < json.size()) {
        // Find next quoted key (entry id)
        size_t qStart = json.find('"', pos);
        if (qStart == std::string::npos) break;
        size_t qEnd = json.find('"', qStart + 1);
        if (qEnd == std::string::npos) break;

        std::string entryId = json.substr(qStart + 1, qEnd - qStart - 1);
        if (entryId.empty()) { pos = qEnd + 1; continue; }

        // Find the object body
        size_t objStart = json.find('{', qEnd);
        if (objStart == std::string::npos) break;

        // Simple brace matching for the object
        int depth = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < json.size() && depth > 0) {
            if (json[objEnd] == '{') ++depth;
            else if (json[objEnd] == '}') --depth;
            ++objEnd;
        }

        DataEntry entry;
        entry.id = entryId;

        // Extract key-value pairs from the object body
        std::string body = json.substr(objStart + 1, objEnd - objStart - 2);
        size_t fpos = 0;
        while (fpos < body.size()) {
            size_t fks = body.find('"', fpos);
            if (fks == std::string::npos) break;
            size_t fke = body.find('"', fks + 1);
            if (fke == std::string::npos) break;
            std::string fkey = body.substr(fks + 1, fke - fks - 1);

            size_t colon = body.find(':', fke);
            if (colon == std::string::npos) break;

            // Determine value
            size_t vs = colon + 1;
            while (vs < body.size() && body[vs] == ' ') ++vs;

            DataField field;
            field.key = fkey;

            if (vs < body.size() && body[vs] == '"') {
                // String value
                size_t ve = body.find('"', vs + 1);
                if (ve == std::string::npos) break;
                field.value = body.substr(vs + 1, ve - vs - 1);
                field.fieldType = DataField::Type::String;
                fpos = ve + 1;
            } else {
                // Number or boolean
                size_t ve = vs;
                while (ve < body.size() && body[ve] != ',' && body[ve] != '}')
                    ++ve;
                std::string raw = body.substr(vs, ve - vs);
                // Trim
                while (!raw.empty() && (raw.back() == ' ' || raw.back() == '\n'))
                    raw.pop_back();

                if (raw == "true" || raw == "false") {
                    field.fieldType = DataField::Type::Boolean;
                } else {
                    field.fieldType = DataField::Type::Number;
                }
                field.value = raw;
                fpos = ve;
            }

            entry.fields.push_back(field);
        }

        m_entries.push_back(entry);
        ++imported;
        pos = objEnd;
    }

    if (imported > 0) {
        log("Imported " + std::to_string(imported) + " entries from JSON");
    }
    return imported;
}

// ── Logging ───────────────────────────────────────────────────────

void DataBrowserPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
