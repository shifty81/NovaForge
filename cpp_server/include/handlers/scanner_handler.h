#pragma once

#include "handlers/message_handler.h"

namespace atlas {
namespace network { class TCPServer; class ProtocolHandler; }
namespace systems {
    class ScannerSystem;
    class AnomalySystem;
}
namespace handlers {

/**
 * @brief Handles scanning messages: scan start/stop, anomaly list.
 */
class ScannerHandler : public IMessageHandler {
public:
    ScannerHandler(network::TCPServer* tcp_server,
                   network::ProtocolHandler* protocol, EntityLookupFn entity_lookup);

    bool canHandle(network::MessageType type) const override;
    void handle(network::MessageType type,
                const network::ClientConnection& client,
                const std::string& data) override;

    void setScannerSystem(systems::ScannerSystem* ss) { scanner_system_ = ss; }
    void setAnomalySystem(systems::AnomalySystem* as) { anomaly_system_ = as; }

private:
    void handleScanStart(const network::ClientConnection& client, const std::string& data);
    void handleScanStop(const network::ClientConnection& client, const std::string& data);
    void handleAnomalyList(const network::ClientConnection& client, const std::string& data);

    network::TCPServer* tcp_server_;
    network::ProtocolHandler* protocol_;
    EntityLookupFn entity_lookup_;
    systems::ScannerSystem* scanner_system_ = nullptr;
    systems::AnomalySystem* anomaly_system_ = nullptr;
};

} // namespace handlers
} // namespace atlas
