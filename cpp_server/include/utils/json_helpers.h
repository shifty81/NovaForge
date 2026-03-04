#pragma once

// Lightweight JSON extraction helpers — no external library required.
//
// These free functions operate on raw JSON strings with simple key
// lookups.  They are intentionally minimal; for complex parsing
// consider a full JSON library.

#include <string>
#include <vector>
#include <stdexcept>

namespace atlas {
namespace json {

/// Escape a string for safe embedding in JSON values.
inline std::string escapeString(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '\"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Skip other control characters
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

/// Extract a string value for a given key: "key":"value"
inline std::string extractString(const std::string& json,
                                 const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";

    pos = json.find('\"', pos + 1);
    if (pos == std::string::npos) return "";

    // Handle escaped quotes inside the value
    size_t end = pos + 1;
    while (end < json.size()) {
        if (json[end] == '\\') { end += 2; continue; }
        if (json[end] == '\"') break;
        ++end;
    }
    if (end >= json.size()) return "";

    return json.substr(pos + 1, end - pos - 1);
}

/// Extract a float value for a given key: "key":123.4
inline float extractFloat(const std::string& json,
                          const std::string& key,
                          float fallback = 0.0f) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return fallback;

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return fallback;
    ++pos;

    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r'))
        ++pos;

    try {
        size_t end = pos;
        while (end < json.size() &&
               (json[end] == '-' || json[end] == '.' ||
                (json[end] >= '0' && json[end] <= '9') ||
                json[end] == 'e' || json[end] == 'E' || json[end] == '+')) {
            ++end;
        }
        return std::stof(json.substr(pos, end - pos));
    } catch (const std::exception&) {
        return fallback;
    }
}

/// Extract an integer value for a given key: "key":42
inline int extractInt(const std::string& json,
                      const std::string& key,
                      int fallback = 0) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return fallback;

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return fallback;
    ++pos;

    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r'))
        ++pos;

    try {
        size_t end = pos;
        while (end < json.size() &&
               (json[end] == '-' || (json[end] >= '0' && json[end] <= '9'))) {
            ++end;
        }
        return std::stoi(json.substr(pos, end - pos));
    } catch (const std::exception&) {
        return fallback;
    }
}

/// Extract a double value for a given key: "key":123.456789
inline double extractDouble(const std::string& json,
                            const std::string& key,
                            double fallback = 0.0) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return fallback;

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return fallback;
    ++pos;

    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r'))
        ++pos;

    try {
        size_t end = pos;
        while (end < json.size() &&
               (json[end] == '-' || json[end] == '.' ||
                (json[end] >= '0' && json[end] <= '9') ||
                json[end] == 'e' || json[end] == 'E' || json[end] == '+')) {
            ++end;
        }
        return std::stod(json.substr(pos, end - pos));
    } catch (const std::exception&) {
        return fallback;
    }
}

/// Extract a boolean value for a given key: "key":true
inline bool extractBool(const std::string& json,
                        const std::string& key,
                        bool fallback = false) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return fallback;

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return fallback;
    ++pos;

    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t'))
        ++pos;

    if (pos + 4 <= json.size() && json.substr(pos, 4) == "true")  return true;
    if (pos + 5 <= json.size() && json.substr(pos, 5) == "false") return false;
    return fallback;
}

/// Find the position of the closing delimiter that matches the opening
/// delimiter at @p start.  Handles nested braces/brackets and respects
/// JSON string boundaries (including escaped quotes).
///
/// @param json       Input JSON text.
/// @param start      Index of the opening '{' or '['.
/// @param open_char  The opening delimiter ('{' or '[').
/// @param close_char The matching closing delimiter ('}' or ']').
/// @return Position of the matching closing delimiter, or
///         std::string::npos if not found.
inline size_t findBlockEnd(const std::string& json, size_t start,
                           char open_char, char close_char) {
    if (start >= json.size() || json[start] != open_char) return std::string::npos;
    int depth = 0;
    bool in_str = false;
    for (size_t i = start; i < json.size(); ++i) {
        char c = json[i];
        if (c == '\\' && in_str) { ++i; continue; }
        if (c == '\"') { in_str = !in_str; continue; }
        if (in_str) continue;
        if (c == open_char) ++depth;
        else if (c == close_char) {
            --depth;
            if (depth == 0) return i;
        }
    }
    return std::string::npos;
}

/// Extract a nested JSON object for a given key: "key":{...}
inline std::string extractObject(const std::string& json,
                                 const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find('{', pos + search.size());
    if (pos == std::string::npos) return "";

    size_t end = findBlockEnd(json, pos, '{', '}');
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos + 1);
}

/// Extract a JSON array for a given key: "key":[...]
inline std::string extractArray(const std::string& json,
                                const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find('[', pos + search.size());
    if (pos == std::string::npos) return "";

    size_t end = findBlockEnd(json, pos, '[', ']');
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos + 1);
}

/// Parse a JSON array of strings: ["a","b","c"]
inline std::vector<std::string> parseStringArray(const std::string& arr) {
    std::vector<std::string> result;
    size_t pos = 0;
    while (pos < arr.size()) {
        size_t qs = arr.find('\"', pos);
        if (qs == std::string::npos) break;

        // Find closing quote, skipping escaped quotes
        size_t qe = qs + 1;
        while (qe < arr.size()) {
            if (arr[qe] == '\\') { qe += 2; continue; }
            if (arr[qe] == '\"') break;
            ++qe;
        }
        if (qe >= arr.size()) break;

        result.push_back(arr.substr(qs + 1, qe - qs - 1));
        pos = qe + 1;
    }
    return result;
}

} // namespace json
} // namespace atlas
