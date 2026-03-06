#pragma once

#include "handlers/message_handler.h"

namespace atlas {
namespace ecs { class World; }
namespace network { class TCPServer; class ProtocolHandler; }
namespace systems { class StationSystem; }
namespace handlers {

/**
 * @brief Handles station-related messages: dock, undock, repair.
 */
class StationHandler : public IMessageHandler {
public:
    StationHandler(ecs::World* world, network::TCPServer* tcp_server,
                   network::ProtocolHandler* protocol, EntityLookupFn entity_lookup);

    bool canHandle(network::MessageType type) const override;
    void handle(network::MessageType type,
                const network::ClientConnection& client,
                const std::string& data) override;

    void setStationSystem(systems::StationSystem* ss) { station_system_ = ss; }

private:
    void handleDockRequest(const network::ClientConnection& client, const std::string& data);
    void handleUndockRequest(const network::ClientConnection& client, const std::string& data);
    void handleRepairRequest(const network::ClientConnection& client, const std::string& data);

    ecs::World* world_;
    network::TCPServer* tcp_server_;
    network::ProtocolHandler* protocol_;
    EntityLookupFn entity_lookup_;
    systems::StationSystem* station_system_ = nullptr;
};

} // namespace handlers
} // namespace atlas
