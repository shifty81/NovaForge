#include "game_session.h"
#include <string>

namespace atlas {

// ---------------------------------------------------------------------------
// Lightweight JSON helpers (no external library required)
// ---------------------------------------------------------------------------

std::string GameSession::extractJsonString(const std::string& json,
                                           const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    // Skip past key and colon
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";

    // Skip whitespace
    pos = json.find('\"', pos + 1);
    if (pos == std::string::npos) return "";

    size_t end = json.find('\"', pos + 1);
    if (end == std::string::npos) return "";

    return json.substr(pos + 1, end - pos - 1);
}

float GameSession::extractJsonFloat(const std::string& json,
                                    const std::string& key,
                                    float fallback) {
    size_t pos = json.find(key);
    if (pos == std::string::npos) return fallback;

    pos += key.size();
    // Skip whitespace
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;

    try {
        size_t end = pos;
        while (end < json.size() &&
               (json[end] == '-' || json[end] == '.' ||
                (json[end] >= '0' && json[end] <= '9') ||
                json[end] == 'e' || json[end] == 'E' || json[end] == '+')) {
            ++end;
        }
        return std::stof(json.substr(pos, end - pos));
    } catch (...) {
        return fallback;
    }
}

} // namespace atlas
