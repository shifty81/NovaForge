#include "editor/scene_bookmark_manager.h"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace atlas::editor {

// ── Minimal JSON helpers (no external dependency) ───────────────────

static std::string EscapeJSON(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

std::string SceneBookmarkManager::SerializeToJSON() const {
    std::ostringstream os;
    os << "{\n  \"bookmarks\": [";

    for (size_t i = 0; i < m_bookmarks.size(); ++i) {
        const auto& bm = m_bookmarks[i];
        if (i > 0) os << ",";
        os << "\n    {";
        os << "\"label\":\"" << EscapeJSON(bm.label) << "\"";
        os << ",\"camX\":" << bm.camX;
        os << ",\"camY\":" << bm.camY;
        os << ",\"camZ\":" << bm.camZ;
        os << ",\"lookX\":" << bm.lookX;
        os << ",\"lookY\":" << bm.lookY;
        os << ",\"lookZ\":" << bm.lookZ;
        if (!bm.selectedEntities.empty()) {
            os << ",\"selectedEntities\":[";
            for (size_t j = 0; j < bm.selectedEntities.size(); ++j) {
                if (j > 0) os << ",";
                os << bm.selectedEntities[j];
            }
            os << "]";
        }
        os << "}";
    }

    os << "\n  ]\n}";
    return os.str();
}

// ── Minimal JSON deserialization ────────────────────────────────────

static void SkipWS(const std::string& s, size_t& pos) {
    while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\n' ||
                               s[pos] == '\r' || s[pos] == '\t'))
        ++pos;
}

static bool Expect(const std::string& s, size_t& pos, char ch) {
    SkipWS(s, pos);
    if (pos < s.size() && s[pos] == ch) { ++pos; return true; }
    return false;
}

static std::string ReadStr(const std::string& s, size_t& pos) {
    SkipWS(s, pos);
    if (pos >= s.size() || s[pos] != '"') return {};
    ++pos;
    std::string out;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\' && pos + 1 < s.size()) {
            ++pos;
            switch (s[pos]) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                default:   out += s[pos]; break;
            }
        } else {
            out += s[pos];
        }
        ++pos;
    }
    if (pos < s.size()) ++pos;
    return out;
}

static double ReadNum(const std::string& s, size_t& pos) {
    SkipWS(s, pos);
    size_t start = pos;
    if (pos < s.size() && (s[pos] == '-' || s[pos] == '+')) ++pos;
    bool hasDot = false;
    while (pos < s.size()) {
        if (std::isdigit(static_cast<unsigned char>(s[pos]))) {
            ++pos;
        } else if (s[pos] == '.' && !hasDot) {
            hasDot = true;
            ++pos;
        } else {
            break;
        }
    }
    if (pos == start) return 0.0;
    return std::stod(s.substr(start, pos - start));
}

bool SceneBookmarkManager::DeserializeFromJSON(const std::string& json) {
    m_bookmarks.clear();

    size_t pos = 0;
    if (!Expect(json, pos, '{')) return false;

    while (pos < json.size()) {
        SkipWS(json, pos);
        if (pos < json.size() && json[pos] == '}') { ++pos; break; }

        std::string key = ReadStr(json, pos);
        if (key.empty()) return false;
        if (!Expect(json, pos, ':')) return false;

        if (key == "bookmarks") {
            if (!Expect(json, pos, '[')) return false;

            while (pos < json.size()) {
                SkipWS(json, pos);
                if (pos < json.size() && json[pos] == ']') { ++pos; break; }
                if (pos < json.size() && json[pos] == ',') ++pos;

                if (!Expect(json, pos, '{')) return false;
                SceneBookmark bm;

                while (pos < json.size()) {
                    SkipWS(json, pos);
                    if (pos < json.size() && json[pos] == '}') { ++pos; break; }
                    if (pos < json.size() && json[pos] == ',') ++pos;

                    std::string field = ReadStr(json, pos);
                    if (!Expect(json, pos, ':')) return false;

                    if (field == "label") {
                        bm.label = ReadStr(json, pos);
                    } else if (field == "camX") {
                        bm.camX = static_cast<float>(ReadNum(json, pos));
                    } else if (field == "camY") {
                        bm.camY = static_cast<float>(ReadNum(json, pos));
                    } else if (field == "camZ") {
                        bm.camZ = static_cast<float>(ReadNum(json, pos));
                    } else if (field == "lookX") {
                        bm.lookX = static_cast<float>(ReadNum(json, pos));
                    } else if (field == "lookY") {
                        bm.lookY = static_cast<float>(ReadNum(json, pos));
                    } else if (field == "lookZ") {
                        bm.lookZ = static_cast<float>(ReadNum(json, pos));
                    } else if (field == "selectedEntities") {
                        if (!Expect(json, pos, '[')) return false;
                        while (pos < json.size()) {
                            SkipWS(json, pos);
                            if (pos < json.size() && json[pos] == ']') { ++pos; break; }
                            if (pos < json.size() && json[pos] == ',') ++pos;
                            bm.selectedEntities.push_back(
                                static_cast<uint32_t>(ReadNum(json, pos)));
                        }
                    }
                }

                m_bookmarks.push_back(std::move(bm));
            }
        }

        SkipWS(json, pos);
        if (pos < json.size() && json[pos] == ',') ++pos;
    }

    return true;
}

// ── File I/O ────────────────────────────────────────────────────

bool SceneBookmarkManager::SaveToFile(const std::string& path) const {
    std::filesystem::path fspath(path);
    if (fspath.has_parent_path()) {
        std::filesystem::create_directories(fspath.parent_path());
    }

    std::ofstream out(path);
    if (!out.is_open()) return false;

    out << SerializeToJSON();
    out.close();
    return !out.fail();
}

bool SceneBookmarkManager::LoadFromFile(const std::string& path) {
    if (!std::filesystem::exists(path)) return false;

    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string json((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    in.close();

    return DeserializeFromJSON(json);
}

} // namespace atlas::editor
