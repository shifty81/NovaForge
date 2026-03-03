#include "DeltaEditStore.h"
#include <sstream>

namespace atlas::ecs {

const char* DeltaEditTypeName(DeltaEditType type) {
    switch (type) {
        case DeltaEditType::AddObject:    return "AddObject";
        case DeltaEditType::RemoveObject: return "RemoveObject";
        case DeltaEditType::MoveObject:   return "MoveObject";
        case DeltaEditType::SetProperty:  return "SetProperty";
    }
    return "Unknown";
}

// ── DeltaEditStore ──────────────────────────────────────────────────

void DeltaEditStore::Record(const DeltaEdit& edit) {
    m_edits.push_back(edit);
}

void DeltaEditStore::Clear() {
    m_edits.clear();
}

// ── Minimal JSON serialization (no external dependency) ─────────────
//
// The format is intentionally simple so it can be read/written without
// nlohmann-json at the engine level (the engine has zero deps beyond
// the C++17 stdlib).  A more robust implementation could delegate to
// nlohmann-json in higher layers.

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

std::string DeltaEditStore::SerializeToJSON() const {
    std::ostringstream os;
    os << "{\n  \"worldSeed\": " << m_seed << ",\n  \"deltaEdits\": [";

    for (size_t i = 0; i < m_edits.size(); ++i) {
        const auto& e = m_edits[i];
        if (i > 0) os << ",";
        os << "\n    {";
        os << "\"type\":\"" << DeltaEditTypeName(e.type) << "\"";
        os << ",\"entityID\":" << e.entityID;
        if (!e.objectType.empty())
            os << ",\"objectType\":\"" << EscapeJSON(e.objectType) << "\"";
        os << ",\"position\":[" << e.position[0] << "," << e.position[1]
           << "," << e.position[2] << "]";
        if (!e.propertyName.empty())
            os << ",\"propertyName\":\"" << EscapeJSON(e.propertyName) << "\"";
        if (!e.propertyValue.empty())
            os << ",\"propertyValue\":\"" << EscapeJSON(e.propertyValue) << "\"";
        os << "}";
    }

    os << "\n  ]\n}";
    return os.str();
}

// ── Minimal JSON deserialization ────────────────────────────────────
//
// Hand-rolled tokenizer that handles the exact format produced by
// SerializeToJSON().  Production code would use a proper JSON library.

static void SkipWhitespace(const std::string& s, size_t& pos) {
    while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\n' ||
                               s[pos] == '\r' || s[pos] == '\t'))
        ++pos;
}

static bool Expect(const std::string& s, size_t& pos, char ch) {
    SkipWhitespace(s, pos);
    if (pos < s.size() && s[pos] == ch) { ++pos; return true; }
    return false;
}

static std::string ReadString(const std::string& s, size_t& pos) {
    SkipWhitespace(s, pos);
    if (pos >= s.size() || s[pos] != '"') return {};
    ++pos; // skip opening quote
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
    if (pos < s.size()) ++pos; // skip closing quote
    return out;
}

static double ReadNumber(const std::string& s, size_t& pos) {
    SkipWhitespace(s, pos);
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

static bool ParseEditType(const std::string& name, DeltaEditType& out) {
    if (name == "AddObject")    { out = DeltaEditType::AddObject;    return true; }
    if (name == "RemoveObject") { out = DeltaEditType::RemoveObject; return true; }
    if (name == "MoveObject")   { out = DeltaEditType::MoveObject;   return true; }
    if (name == "SetProperty")  { out = DeltaEditType::SetProperty;  return true; }
    return false; // unknown type → reject
}

bool DeltaEditStore::DeserializeFromJSON(const std::string& json) {
    m_edits.clear();
    m_seed = 0;

    size_t pos = 0;
    if (!Expect(json, pos, '{')) return false;

    // Read key-value pairs at the top level
    while (pos < json.size()) {
        SkipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == '}') { ++pos; break; }

        std::string key = ReadString(json, pos);
        if (key.empty()) return false;
        if (!Expect(json, pos, ':')) return false;

        if (key == "worldSeed") {
            m_seed = static_cast<uint64_t>(ReadNumber(json, pos));
        } else if (key == "deltaEdits") {
            if (!Expect(json, pos, '[')) return false;

            while (pos < json.size()) {
                SkipWhitespace(json, pos);
                if (pos < json.size() && json[pos] == ']') { ++pos; break; }
                if (pos < json.size() && json[pos] == ',') ++pos;

                if (!Expect(json, pos, '{')) return false;
                DeltaEdit edit{};
                edit.type = DeltaEditType::AddObject;

                while (pos < json.size()) {
                    SkipWhitespace(json, pos);
                    if (pos < json.size() && json[pos] == '}') { ++pos; break; }
                    if (pos < json.size() && json[pos] == ',') ++pos;

                    std::string field = ReadString(json, pos);
                    if (!Expect(json, pos, ':')) return false;

                    if (field == "type") {
                        std::string typeName = ReadString(json, pos);
                        if (!ParseEditType(typeName, edit.type))
                            return false;
                    } else if (field == "entityID") {
                        edit.entityID = static_cast<uint32_t>(ReadNumber(json, pos));
                    } else if (field == "objectType") {
                        edit.objectType = ReadString(json, pos);
                    } else if (field == "position") {
                        if (!Expect(json, pos, '[')) return false;
                        edit.position[0] = static_cast<float>(ReadNumber(json, pos));
                        Expect(json, pos, ',');
                        edit.position[1] = static_cast<float>(ReadNumber(json, pos));
                        Expect(json, pos, ',');
                        edit.position[2] = static_cast<float>(ReadNumber(json, pos));
                        Expect(json, pos, ']');
                    } else if (field == "propertyName") {
                        edit.propertyName = ReadString(json, pos);
                    } else if (field == "propertyValue") {
                        edit.propertyValue = ReadString(json, pos);
                    }
                }

                m_edits.push_back(edit);
            }
        }

        // skip comma between top-level pairs
        SkipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') ++pos;
    }

    return true;
}

} // namespace atlas::ecs
