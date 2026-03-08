#include "handlers/chat_handler.h"
#include "handlers/handler_utils.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/social_components.h"
#include "systems/chat_system.h"
#include "network/tcp_server.h"
#include "network/protocol_handler.h"
#include <sstream>

namespace atlas {
namespace handlers {

ChatHandler::ChatHandler(ecs::World* world, network::TCPServer* tcp_server,
                         network::ProtocolHandler* protocol, EntityLookupFn entity_lookup)
    : world_(world), tcp_server_(tcp_server), protocol_(protocol),
      entity_lookup_(std::move(entity_lookup)) {}

bool ChatHandler::canHandle(network::MessageType type) const {
    switch (type) {
        case network::MessageType::CHAT:
        case network::MessageType::CHAT_SEND:
        case network::MessageType::CHAT_JOIN:
        case network::MessageType::CHAT_LEAVE:
        case network::MessageType::CHAT_HISTORY:
        case network::MessageType::ADMIN_COMMAND_SEND:
            return true;
        default:
            return false;
    }
}

void ChatHandler::handle(network::MessageType type,
                         const network::ClientConnection& client,
                         const std::string& data) {
    switch (type) {
        case network::MessageType::CHAT:
        case network::MessageType::CHAT_SEND:
            handleChatSend(client, data);
            break;
        case network::MessageType::CHAT_JOIN:
            handleChatJoin(client, data);
            break;
        case network::MessageType::CHAT_LEAVE:
            handleChatLeave(client, data);
            break;
        case network::MessageType::CHAT_HISTORY:
            handleChatHistory(client, data);
            break;
        case network::MessageType::ADMIN_COMMAND_SEND:
            handleAdminCommand(client, data);
            break;
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// CHAT_SEND — validate, route, broadcast
// ---------------------------------------------------------------------------

void ChatHandler::handleChatSend(const network::ClientConnection& client,
                                 const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    std::string text = extractJsonString(data, "message");
    std::string channel_id = extractJsonString(data, "channel_id");
    std::string client_msg_id = extractJsonString(data, "client_msg_id");

    // Validate text
    if (!validateMessageText(text)) {
        // Send error result back to sender
        if (tcp_server_ && protocol_) {
            std::string error_msg = protocol_->createError("Message too long or empty");
            tcp_server_->sendToClient(client, error_msg);
        }
        return;
    }

    // Route through ChatSystem
    if (chat_system_ && !channel_id.empty()) {
        // Look up sender name from entity
        std::string sender_name = entity_id; // Fallback to entity ID
        auto* entity = world_->getEntity(entity_id);
        if (entity) {
            auto* sheet = entity->getComponent<components::CharacterSheet>();
            if (sheet) {
                sender_name = sheet->character_name;
            }
        }

        bool sent = chat_system_->sendMessage(channel_id, entity_id, sender_name, text);

        if (!sent && tcp_server_ && protocol_) {
            std::string error_msg = protocol_->createError("Failed to send message");
            tcp_server_->sendToClient(client, error_msg);
        }
    }
}

// ---------------------------------------------------------------------------
// CHAT_JOIN
// ---------------------------------------------------------------------------

void ChatHandler::handleChatJoin(const network::ClientConnection& client,
                                 const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    std::string channel_id = extractJsonString(data, "channel_id");
    if (channel_id.empty()) return;

    if (chat_system_) {
        std::string player_name = entity_id;
        auto* entity = world_->getEntity(entity_id);
        if (entity) {
            auto* sheet = entity->getComponent<components::CharacterSheet>();
            if (sheet) player_name = sheet->character_name;
        }

        chat_system_->joinChannel(channel_id, entity_id, player_name);
    }
}

// ---------------------------------------------------------------------------
// CHAT_LEAVE
// ---------------------------------------------------------------------------

void ChatHandler::handleChatLeave(const network::ClientConnection& client,
                                  const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    std::string channel_id = extractJsonString(data, "channel_id");
    if (channel_id.empty()) return;

    if (chat_system_) {
        chat_system_->leaveChannel(channel_id, entity_id);
    }
}

// ---------------------------------------------------------------------------
// CHAT_HISTORY — send last N messages for a channel
// ---------------------------------------------------------------------------

void ChatHandler::handleChatHistory(const network::ClientConnection& client,
                                    const std::string& data) {
    (void)client;
    (void)data;
    // History retrieval would query ChatSystem's message buffer
    // and send a ChatHistoryChunk back. Implementation depends on
    // persistence layer (database or in-memory).
}

// ---------------------------------------------------------------------------
// ADMIN_COMMAND_SEND
// ---------------------------------------------------------------------------

void ChatHandler::handleAdminCommand(const network::ClientConnection& client,
                                     const std::string& data) {
    (void)client;
    (void)data;
    // Admin command execution requires role verification.
    // For v1, this is a stub that logs the command.
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

bool ChatHandler::validateMessageText(const std::string& text) const {
    if (text.empty()) return false;
    if (text.size() > 512) return false;  // Max 512 UTF-8 bytes
    return true;
}

} // namespace handlers
} // namespace atlas
