#pragma once

#include "network/protocol_handler.h"
#include "network/tcp_server.h"
#include <functional>
#include <string>

namespace atlas {
namespace handlers {

/// Callback to resolve a client socket fd to a player entity ID.
using EntityLookupFn = std::function<std::string(int socket_fd)>;

/**
 * @brief Interface for domain-specific message handlers.
 *
 * GameSession dispatches incoming client messages to registered handlers.
 * Each handler owns the system pointers it needs and implements the message
 * processing logic for its domain (combat, stations, movement, etc.).
 */
class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    /// Return true if this handler processes the given message type.
    virtual bool canHandle(network::MessageType type) const = 0;

    /// Process an incoming message.
    virtual void handle(network::MessageType type,
                        const network::ClientConnection& client,
                        const std::string& data) = 0;
};

} // namespace handlers
} // namespace atlas
