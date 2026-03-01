#pragma once

#include <string>

namespace atlas {

static constexpr float NPC_AWARENESS_RANGE = 50000.0f;
static constexpr float PLAYER_SPAWN_SPACING_X = 50.0f;
static constexpr float PLAYER_SPAWN_SPACING_Z = 30.0f;
static constexpr size_t MAX_CHARACTER_NAME_LEN = 32;
static constexpr size_t MAX_CHAT_MESSAGE_LEN = 256;

inline std::string escapeJsonString(const std::string& input) {
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

} // namespace atlas
