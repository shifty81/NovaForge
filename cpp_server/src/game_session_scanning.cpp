#include "game_session.h"
#include "game_session_internal.h"
#include "systems/scanner_system.h"
#include "systems/anomaly_system.h"
#include <sstream>
#include <mutex>

namespace atlas {

// ---------------------------------------------------------------------------
// SCAN START handler
// ---------------------------------------------------------------------------

void GameSession::handleScanStart(const network::ClientConnection& client,
                                   const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!scanner_system_) {
        tcp_server_->sendToClient(client, protocol_.createError("Scanner system not available"));
        return;
    }

    std::string system_id = extractJsonString(data, "system_id");
    if (system_id.empty()) {
        tcp_server_->sendToClient(client, protocol_.createError("No system_id provided"));
        return;
    }

    bool started = scanner_system_->startScan(entity_id, system_id);
    if (started) {
        auto results = scanner_system_->getScanResults(entity_id);
        std::ostringstream results_json;
        results_json << "[";
        for (size_t i = 0; i < results.size(); ++i) {
            if (i > 0) results_json << ",";
            results_json << "{\"anomaly_id\":\"" << results[i].anomaly_id << "\","
                         << "\"signal_strength\":" << results[i].signal_strength << ","
                         << "\"deviation\":" << results[i].deviation << "}";
        }
        results_json << "]";
        tcp_server_->sendToClient(client,
            protocol_.createScanResult(entity_id, static_cast<int>(results.size()),
                                       results_json.str()));
    } else {
        tcp_server_->sendToClient(client, protocol_.createError("Failed to start scan"));
    }
}

// ---------------------------------------------------------------------------
// SCAN STOP handler
// ---------------------------------------------------------------------------

void GameSession::handleScanStop(const network::ClientConnection& client,
                                  const std::string& /*data*/) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!scanner_system_) {
        tcp_server_->sendToClient(client, protocol_.createError("Scanner system not available"));
        return;
    }

    scanner_system_->stopScan(entity_id);
    auto results = scanner_system_->getScanResults(entity_id);
    std::ostringstream results_json;
    results_json << "[";
    for (size_t i = 0; i < results.size(); ++i) {
        if (i > 0) results_json << ",";
        results_json << "{\"anomaly_id\":\"" << results[i].anomaly_id << "\","
                     << "\"signal_strength\":" << results[i].signal_strength << ","
                     << "\"deviation\":" << results[i].deviation << "}";
    }
    results_json << "]";
    tcp_server_->sendToClient(client,
        protocol_.createScanResult(entity_id, static_cast<int>(results.size()),
                                   results_json.str()));
}

// ---------------------------------------------------------------------------
// ANOMALY LIST handler
// ---------------------------------------------------------------------------

void GameSession::handleAnomalyList(const network::ClientConnection& client,
                                     const std::string& data) {
    if (!anomaly_system_) {
        tcp_server_->sendToClient(client, protocol_.createError("Anomaly system not available"));
        return;
    }

    std::string system_id = extractJsonString(data, "system_id");
    if (system_id.empty()) {
        tcp_server_->sendToClient(client, protocol_.createError("No system_id provided"));
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
        protocol_.createAnomalyList(system_id, static_cast<int>(anomaly_ids.size()),
                                    anomalies_json.str()));
}

} // namespace atlas
