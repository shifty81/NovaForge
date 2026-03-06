#include "handlers/scanner_handler.h"
#include "handlers/handler_utils.h"
#include "systems/scanner_system.h"
#include "systems/anomaly_system.h"
#include "components/game_components.h"
#include <sstream>

namespace atlas {
namespace handlers {

namespace {

std::string serializeScanResults(
    const std::vector<components::Scanner::ScanResult>& results) {
    std::ostringstream json;
    json << "[";
    for (size_t i = 0; i < results.size(); ++i) {
        if (i > 0) json << ",";
        json << "{\"anomaly_id\":\"" << results[i].anomaly_id << "\","
             << "\"signal_strength\":" << results[i].signal_strength << ","
             << "\"deviation\":" << results[i].deviation << "}";
    }
    json << "]";
    return json.str();
}

} // anonymous namespace

ScannerHandler::ScannerHandler(network::TCPServer* tcp_server,
                               network::ProtocolHandler* protocol, EntityLookupFn entity_lookup)
    : tcp_server_(tcp_server), protocol_(protocol),
      entity_lookup_(std::move(entity_lookup)) {}

bool ScannerHandler::canHandle(network::MessageType type) const {
    switch (type) {
        case network::MessageType::SCAN_START:
        case network::MessageType::SCAN_STOP:
        case network::MessageType::ANOMALY_LIST:
            return true;
        default:
            return false;
    }
}

void ScannerHandler::handle(network::MessageType type,
                            const network::ClientConnection& client,
                            const std::string& data) {
    switch (type) {
        case network::MessageType::SCAN_START:
            handleScanStart(client, data);
            break;
        case network::MessageType::SCAN_STOP:
            handleScanStop(client, data);
            break;
        case network::MessageType::ANOMALY_LIST:
            handleAnomalyList(client, data);
            break;
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// SCAN_START
// ---------------------------------------------------------------------------

void ScannerHandler::handleScanStart(const network::ClientConnection& client,
                                     const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!scanner_system_) {
        tcp_server_->sendToClient(client, protocol_->createError("Scanner system not available"));
        return;
    }

    std::string system_id = extractJsonString(data, "system_id");
    if (system_id.empty()) {
        tcp_server_->sendToClient(client, protocol_->createError("No system_id provided"));
        return;
    }

    bool started = scanner_system_->startScan(entity_id, system_id);
    if (started) {
        auto results = scanner_system_->getScanResults(entity_id);
        std::string results_json = serializeScanResults(results);
        tcp_server_->sendToClient(client,
            protocol_->createScanResult(entity_id, static_cast<int>(results.size()),
                                        results_json));
    } else {
        tcp_server_->sendToClient(client, protocol_->createError("Failed to start scan"));
    }
}

// ---------------------------------------------------------------------------
// SCAN_STOP
// ---------------------------------------------------------------------------

void ScannerHandler::handleScanStop(const network::ClientConnection& client,
                                    const std::string& /*data*/) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!scanner_system_) {
        tcp_server_->sendToClient(client, protocol_->createError("Scanner system not available"));
        return;
    }

    scanner_system_->stopScan(entity_id);
    auto results = scanner_system_->getScanResults(entity_id);
    std::string results_json = serializeScanResults(results);
    tcp_server_->sendToClient(client,
        protocol_->createScanResult(entity_id, static_cast<int>(results.size()),
                                    results_json));
}

// ---------------------------------------------------------------------------
// ANOMALY_LIST
// ---------------------------------------------------------------------------

void ScannerHandler::handleAnomalyList(const network::ClientConnection& client,
                                       const std::string& data) {
    if (!anomaly_system_) {
        tcp_server_->sendToClient(client, protocol_->createError("Anomaly system not available"));
        return;
    }

    std::string system_id = extractJsonString(data, "system_id");
    if (system_id.empty()) {
        tcp_server_->sendToClient(client, protocol_->createError("No system_id provided"));
        return;
    }

    auto anomaly_ids = anomaly_system_->getAnomaliesInSystem(system_id);

    std::ostringstream anomalies_json;
    anomalies_json << "[";
    for (size_t i = 0; i < anomaly_ids.size(); ++i) {
        if (i > 0) anomalies_json << ",";
        anomalies_json << "\"" << anomaly_ids[i] << "\"";
    }
    anomalies_json << "]";

    tcp_server_->sendToClient(client,
        protocol_->createAnomalyList(system_id, static_cast<int>(anomaly_ids.size()),
                                     anomalies_json.str()));
}

} // namespace handlers
} // namespace atlas
