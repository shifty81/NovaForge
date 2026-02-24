#include "PCGOverrideStore.h"
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstring>

namespace atlas::editor {

// ── Override management ──────────────────────────────────────────

void PCGOverrideStore::ImportFromViewport(
        const std::vector<ViewportChange>& changes,
        const std::vector<ViewportObject>& objects,
        uint64_t seed, uint32_t version) {

    for (const auto& c : changes) {
        PCGOverride ov;
        ov.objectId = c.objectId;
        ov.field    = c.field;
        ov.seed     = seed;
        ov.version  = version;
        std::memcpy(ov.values, c.newValues, sizeof(ov.values));

        // Resolve name / type from viewport objects
        for (const auto& obj : objects) {
            if (obj.id == c.objectId) {
                ov.objectName = obj.name;
                ov.objectType = obj.type;
                break;
            }
        }

        // Merge: replace existing override for same object+field, or append.
        bool merged = false;
        for (auto& existing : m_overrides) {
            if (existing.objectId == ov.objectId &&
                existing.field    == ov.field &&
                existing.seed     == ov.seed) {
                existing = ov;
                merged = true;
                break;
            }
        }
        if (!merged) {
            m_overrides.push_back(ov);
        }
    }

    if (!changes.empty()) {
        m_dirty = true;
        std::ostringstream oss;
        oss << "[Overrides] Imported " << changes.size()
            << " change(s) from viewport";
        log(oss.str());
    }
}

void PCGOverrideStore::Add(const PCGOverride& ov) {
    m_overrides.push_back(ov);
    m_dirty = true;
}

void PCGOverrideStore::RemoveByObject(uint32_t objectId) {
    auto it = std::remove_if(m_overrides.begin(), m_overrides.end(),
        [objectId](const PCGOverride& o) { return o.objectId == objectId; });
    if (it != m_overrides.end()) {
        m_overrides.erase(it, m_overrides.end());
        m_dirty = true;
    }
}

void PCGOverrideStore::Clear() {
    m_overrides.clear();
    m_dirty = false;
    log("[Overrides] Cleared all overrides");
}

// ── Serialisation ────────────────────────────────────────────────
//
// Minimal hand-written JSON to avoid pulling in a JSON library into
// the editor.  The format is:
//
//   { "overrides": [ { "objectId":1, "objectName":"…", … }, … ] }

std::string PCGOverrideStore::SerializeToJSON() const {
    std::ostringstream os;
    os << "{\n  \"overrides\": [\n";
    for (size_t i = 0; i < m_overrides.size(); ++i) {
        const auto& o = m_overrides[i];
        os << "    {\n"
           << "      \"objectId\": "   << o.objectId   << ",\n"
           << "      \"objectName\": \"" << o.objectName << "\",\n"
           << "      \"objectType\": \"" << o.objectType << "\",\n"
           << "      \"field\": \""    << o.field      << "\",\n"
           << "      \"values\": [" << o.values[0] << ", "
                                    << o.values[1] << ", "
                                    << o.values[2] << "],\n"
           << "      \"seed\": "       << o.seed       << ",\n"
           << "      \"version\": "    << o.version    << "\n"
           << "    }";
        if (i + 1 < m_overrides.size()) os << ",";
        os << "\n";
    }
    os << "  ]\n}\n";
    return os.str();
}

bool PCGOverrideStore::DeserializeFromJSON(const std::string& json) {
    // Minimal parser: find each override block by searching for "objectId"
    // This is intentionally simple — a real implementation would use
    // nlohmann/json, but we keep editor dependencies light.
    m_overrides.clear();

    size_t pos = 0;
    while ((pos = json.find("\"objectId\"", pos)) != std::string::npos) {
        PCGOverride ov;

        // objectId
        size_t colon = json.find(':', pos);
        if (colon == std::string::npos) break;
        ov.objectId = static_cast<uint32_t>(std::stoul(json.substr(colon + 1)));

        // objectName
        size_t nameKey = json.find("\"objectName\"", pos);
        if (nameKey != std::string::npos) {
            size_t q1 = json.find('\"', json.find(':', nameKey) + 1);
            size_t q2 = json.find('\"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos)
                ov.objectName = json.substr(q1 + 1, q2 - q1 - 1);
        }

        // objectType
        size_t typeKey = json.find("\"objectType\"", pos);
        if (typeKey != std::string::npos) {
            size_t q1 = json.find('\"', json.find(':', typeKey) + 1);
            size_t q2 = json.find('\"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos)
                ov.objectType = json.substr(q1 + 1, q2 - q1 - 1);
        }

        // field
        size_t fieldKey = json.find("\"field\"", pos);
        if (fieldKey != std::string::npos) {
            size_t q1 = json.find('\"', json.find(':', fieldKey) + 1);
            size_t q2 = json.find('\"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos)
                ov.field = json.substr(q1 + 1, q2 - q1 - 1);
        }

        // values
        size_t valKey = json.find("\"values\"", pos);
        if (valKey != std::string::npos) {
            size_t bracket = json.find('[', valKey);
            if (bracket != std::string::npos) {
                char* end = nullptr;
                const char* start = json.c_str() + bracket + 1;
                ov.values[0] = std::strtof(start, &end);
                if (end) { start = end + 1; ov.values[1] = std::strtof(start, &end); }
                if (end) { start = end + 1; ov.values[2] = std::strtof(start, &end); }
            }
        }

        // seed
        size_t seedKey = json.find("\"seed\"", pos);
        if (seedKey != std::string::npos) {
            size_t sc = json.find(':', seedKey);
            if (sc != std::string::npos)
                ov.seed = std::stoull(json.substr(sc + 1));
        }

        // version
        size_t verKey = json.find("\"version\"", pos);
        if (verKey != std::string::npos) {
            size_t vc = json.find(':', verKey);
            if (vc != std::string::npos)
                ov.version = static_cast<uint32_t>(std::stoul(json.substr(vc + 1)));
        }

        m_overrides.push_back(ov);

        // Move past this block
        pos = json.find('}', pos);
        if (pos == std::string::npos) break;
        ++pos;
    }

    m_dirty = false;
    std::ostringstream oss;
    oss << "[Overrides] Loaded " << m_overrides.size() << " override(s)";
    log(oss.str());
    return true;
}

// ── File I/O ─────────────────────────────────────────────────────

bool PCGOverrideStore::SaveToFile(const std::string& path) {
    // Create parent directories if they don't exist
    std::filesystem::path fspath(path);
    if (fspath.has_parent_path()) {
        std::filesystem::create_directories(fspath.parent_path());
    }

    std::ofstream out(path);
    if (!out.is_open()) {
        log("[Overrides] ERROR: Could not open " + path + " for writing");
        return false;
    }

    out << SerializeToJSON();
    out.close();
    m_dirty = false;

    std::ostringstream oss;
    oss << "[Overrides] Saved " << m_overrides.size()
        << " override(s) to " << path;
    log(oss.str());
    std::cout << oss.str() << std::endl;
    return true;
}

bool PCGOverrideStore::LoadFromFile(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        log("[Overrides] File not found: " + path);
        return false;
    }

    std::ifstream in(path);
    if (!in.is_open()) {
        log("[Overrides] ERROR: Could not open " + path + " for reading");
        return false;
    }

    std::string json((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    in.close();

    return DeserializeFromJSON(json);
}

// ── Logging ──────────────────────────────────────────────────────

void PCGOverrideStore::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
