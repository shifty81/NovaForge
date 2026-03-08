#pragma once

#include "handlers/message_handler.h"

namespace atlas {
namespace ecs { class World; }
namespace network { class TCPServer; class ProtocolHandler; }
namespace systems { class ChatSystem; }
namespace handlers {

/**
 * @brief Handles all chat-related messages: send, join, leave, history, admin commands.
 *
 * Validates incoming messages (rate limit, length, mute/ban), routes through
 * the ChatSystem ECS, and broadcasts to appropriate recipients.
 */
class ChatHandler : public IMessageHandler {
public:
    ChatHandler(ecs::World* world, network::TCPServer* tcp_server,
                network::ProtocolHandler* protocol, EntityLookupFn entity_lookup);

    bool canHandle(network::MessageType type) const override;
    void handle(network::MessageType type,
                const network::ClientConnection& client,
                const std::string& data) override;

    void setChatSystem(systems::ChatSystem* cs) { chat_system_ = cs; }

private:
    void handleChatSend(const network::ClientConnection& client, const std::string& data);
    void handleChatJoin(const network::ClientConnection& client, const std::string& data);
    void handleChatLeave(const network::ClientConnection& client, const std::string& data);
    void handleChatHistory(const network::ClientConnection& client, const std::string& data);
    void handleAdminCommand(const network::ClientConnection& client, const std::string& data);

    /// Validate message text: length, rate limit check
    bool validateMessageText(const std::string& text) const;

    ecs::World* world_;
    network::TCPServer* tcp_server_;
    network::ProtocolHandler* protocol_;
    EntityLookupFn entity_lookup_;
    systems::ChatSystem* chat_system_ = nullptr;
};

} // namespace handlers
} // namespace atlas
