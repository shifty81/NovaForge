#pragma once

#include "handlers/message_handler.h"

namespace atlas {
namespace network { class TCPServer; class ProtocolHandler; }
namespace systems {
    class MissionSystem;
    class MissionGeneratorSystem;
}
namespace handlers {

/**
 * @brief Handles mission messages: list, accept, abandon, progress.
 */
class MissionHandler : public IMessageHandler {
public:
    MissionHandler(network::TCPServer* tcp_server,
                   network::ProtocolHandler* protocol, EntityLookupFn entity_lookup);

    bool canHandle(network::MessageType type) const override;
    void handle(network::MessageType type,
                const network::ClientConnection& client,
                const std::string& data) override;

    void setMissionSystem(systems::MissionSystem* ms) { mission_system_ = ms; }
    void setMissionGeneratorSystem(systems::MissionGeneratorSystem* mg) { mission_generator_ = mg; }

private:
    void handleMissionList(const network::ClientConnection& client, const std::string& data);
    void handleAcceptMission(const network::ClientConnection& client, const std::string& data);
    void handleAbandonMission(const network::ClientConnection& client, const std::string& data);
    void handleMissionProgress(const network::ClientConnection& client, const std::string& data);

    network::TCPServer* tcp_server_;
    network::ProtocolHandler* protocol_;
    EntityLookupFn entity_lookup_;
    systems::MissionSystem* mission_system_ = nullptr;
    systems::MissionGeneratorSystem* mission_generator_ = nullptr;
};

} // namespace handlers
} // namespace atlas
