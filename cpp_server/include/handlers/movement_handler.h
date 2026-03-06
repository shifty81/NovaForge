#pragma once

#include "handlers/message_handler.h"

namespace atlas {
namespace network { class TCPServer; class ProtocolHandler; }
namespace systems { class MovementSystem; }
namespace handlers {

/**
 * @brief Handles movement-related messages: warp, approach, orbit, stop.
 */
class MovementHandler : public IMessageHandler {
public:
    MovementHandler(network::TCPServer* tcp_server,
                    network::ProtocolHandler* protocol, EntityLookupFn entity_lookup);

    bool canHandle(network::MessageType type) const override;
    void handle(network::MessageType type,
                const network::ClientConnection& client,
                const std::string& data) override;

    void setMovementSystem(systems::MovementSystem* ms) { movement_system_ = ms; }

private:
    void handleWarpRequest(const network::ClientConnection& client, const std::string& data);
    void handleApproach(const network::ClientConnection& client, const std::string& data);
    void handleOrbit(const network::ClientConnection& client, const std::string& data);
    void handleStop(const network::ClientConnection& client, const std::string& data);

    network::TCPServer* tcp_server_;
    network::ProtocolHandler* protocol_;
    EntityLookupFn entity_lookup_;
    systems::MovementSystem* movement_system_ = nullptr;
};

} // namespace handlers
} // namespace atlas
