#ifndef NOVAFORGE_COMMON_CHAT_TYPES_H
#define NOVAFORGE_COMMON_CHAT_TYPES_H

#include <string>
#include <cstdint>
#include <vector>

namespace novaforge {
namespace chat {

/**
 * @brief Chat stream types matching the tab model.
 *
 * Each stream type maps to a UI tab and a server-side routing rule.
 */
enum class StreamType {
    Global,         // All players on shard
    Local,          // Same solar system
    Party,          // Party members
    Guild,          // Guild/corp members
    Whisper,        // 1:1 DM thread
    System,         // Server announcements, moderation, errors
    AdminConsole    // Admin-only command console
};

/**
 * @brief System message sub-categories for the System tab.
 */
enum class SystemCategory {
    All,            // Default — shows everything
    Moderation,     // Mute/ban warnings, admin actions
    Errors,         // Rate-limit, delivery failures
    Announcements   // Server-wide broadcasts
};

/**
 * @brief Error codes for ChatSendResult.
 */
enum class ChatError {
    None,
    RateLimited,
    Muted,
    TooLong,
    InvalidChannel,
    NotMember,
    Banned,
    ServerError
};

/**
 * @brief Message flags (bitfield).
 */
enum ChatMessageFlags : uint32_t {
    FLAG_NONE       = 0,
    FLAG_SYSTEM     = 1 << 0,
    FLAG_MODERATION = 1 << 1,
    FLAG_BROADCAST  = 1 << 2,
    FLAG_WHISPER    = 1 << 3,
    FLAG_EMOJI      = 1 << 4
};

/**
 * @brief A single chat message as stored and transmitted.
 */
struct ChatMessage {
    StreamType stream_type = StreamType::Global;
    std::string stream_key;                  // solar_system_id, party_id, thread_id, etc.
    uint64_t server_seq = 0;                 // Monotonic per stream
    std::string server_msg_id;
    uint64_t timestamp_utc_ms = 0;
    std::string sender_player_id;
    std::string sender_character_id;
    std::string sender_character_name;
    std::string text;                        // UTF-8, may include emoji/shortcodes
    uint32_t flags = FLAG_NONE;
    SystemCategory system_category = SystemCategory::All;
};

/**
 * @brief Result sent back to the sender after a ChatSend.
 */
struct ChatSendResult {
    std::string client_msg_id;
    bool success = false;
    ChatError error_code = ChatError::None;
    std::string server_msg_id;
};

/**
 * @brief Chat constants (rate limits, max lengths, retention).
 */
struct ChatConstants {
    static constexpr int MAX_MESSAGE_LENGTH = 512;       // UTF-8 bytes
    static constexpr int RATE_LIMIT_COUNT = 3;           // messages per window
    static constexpr float RATE_LIMIT_WINDOW = 5.0f;     // seconds
    static constexpr int CLIENT_SCROLLBACK = 1000;       // messages per stream
    static constexpr int SERVER_RETENTION_DAYS = 90;
    static constexpr int HISTORY_CHUNK_SIZE = 50;        // messages per history request
};

/**
 * @brief Per-tab UI state for flash/unread tracking.
 */
struct TabState {
    StreamType stream_type = StreamType::Global;
    std::string stream_key;
    int unread_count = 0;
    bool has_mention = false;
    bool flash_state = false;
    uint64_t last_activity_time = 0;
    bool is_focused = false;
};

/**
 * @brief Helper to convert StreamType to string.
 */
inline const char* streamTypeToString(StreamType type) {
    switch (type) {
        case StreamType::Global:       return "global";
        case StreamType::Local:        return "local";
        case StreamType::Party:        return "party";
        case StreamType::Guild:        return "guild";
        case StreamType::Whisper:      return "whisper";
        case StreamType::System:       return "system";
        case StreamType::AdminConsole: return "admin_console";
    }
    return "unknown";
}

/**
 * @brief Helper to convert string to StreamType.
 */
inline StreamType stringToStreamType(const std::string& str) {
    if (str == "global")        return StreamType::Global;
    if (str == "local")         return StreamType::Local;
    if (str == "party")         return StreamType::Party;
    if (str == "guild")         return StreamType::Guild;
    if (str == "whisper")       return StreamType::Whisper;
    if (str == "system")        return StreamType::System;
    if (str == "admin_console") return StreamType::AdminConsole;
    return StreamType::Global;
}

/**
 * @brief Helper to convert ChatError to string.
 */
inline const char* chatErrorToString(ChatError err) {
    switch (err) {
        case ChatError::None:           return "none";
        case ChatError::RateLimited:    return "rate_limited";
        case ChatError::Muted:          return "muted";
        case ChatError::TooLong:        return "too_long";
        case ChatError::InvalidChannel: return "invalid_channel";
        case ChatError::NotMember:      return "not_member";
        case ChatError::Banned:         return "banned";
        case ChatError::ServerError:    return "server_error";
    }
    return "unknown";
}

/**
 * @brief Compute a stable DM thread ID from two character IDs.
 *
 * Ensures both parties derive the same thread key.
 */
inline std::string computeWhisperThreadId(const std::string& char_a,
                                           const std::string& char_b) {
    if (char_a < char_b) return char_a + ":" + char_b;
    return char_b + ":" + char_a;
}

} // namespace chat
} // namespace novaforge

#endif // NOVAFORGE_COMMON_CHAT_TYPES_H
