#include "game_session.h"
#include "game_session_internal.h"
#include "systems/movement_system.h"
#include "utils/logger.h"
#include <mutex>

namespace atlas {

// ---------------------------------------------------------------------------
// WARP_REQUEST handler
// ---------------------------------------------------------------------------

void GameSession::handleWarpRequest(const network::ClientConnection& client,
                                    const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!movement_system_) {
        tcp_server_->sendToClient(client,
            protocol_.createWarpResult(false, "Movement system not available"));
        return;
    }

    float dest_x = extractJsonFloat(data, "\"dest_x\":");
    float dest_y = extractJsonFloat(data, "\"dest_y\":");
    float dest_z = extractJsonFloat(data, "\"dest_z\":");

    bool success = movement_system_->commandWarp(entity_id, dest_x, dest_y, dest_z);
    if (success) {
        tcp_server_->sendToClient(client, protocol_.createWarpResult(true));
        atlas::utils::Logger::instance().info(
            "[GameSession] Player " + entity_id + " warping to (" +
            std::to_string(dest_x) + ", " + std::to_string(dest_y) + ", " +
            std::to_string(dest_z) + ")");
    } else {
        std::string reason = movement_system_->isWarpDisrupted(entity_id)
                                 ? "Warp drive disrupted"
                                 : "Destination too close (min 150km)";
        tcp_server_->sendToClient(client, protocol_.createWarpResult(false, reason));
    }
}

// ---------------------------------------------------------------------------
// APPROACH handler
// ---------------------------------------------------------------------------

void GameSession::handleApproach(const network::ClientConnection& client,
                                 const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!movement_system_) {
        tcp_server_->sendToClient(client, protocol_.createMovementAck("approach", false));
        return;
    }

    std::string target_id = extractJsonString(data, "target_id");
    if (target_id.empty()) {
        tcp_server_->sendToClient(client, protocol_.createMovementAck("approach", false));
        return;
    }

    movement_system_->commandApproach(entity_id, target_id);
    tcp_server_->sendToClient(client, protocol_.createMovementAck("approach", true));
}

// ---------------------------------------------------------------------------
// ORBIT handler
// ---------------------------------------------------------------------------

void GameSession::handleOrbit(const network::ClientConnection& client,
                              const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!movement_system_) {
        tcp_server_->sendToClient(client, protocol_.createMovementAck("orbit", false));
        return;
    }

    std::string target_id = extractJsonString(data, "target_id");
    float distance = extractJsonFloat(data, "\"distance\":", 5000.0f);
    if (target_id.empty()) {
        tcp_server_->sendToClient(client, protocol_.createMovementAck("orbit", false));
        return;
    }

    movement_system_->commandOrbit(entity_id, target_id, distance);
    tcp_server_->sendToClient(client, protocol_.createMovementAck("orbit", true));
}

// ---------------------------------------------------------------------------
// STOP handler
// ---------------------------------------------------------------------------

void GameSession::handleStop(const network::ClientConnection& client,
                             const std::string& /*data*/) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!movement_system_) {
        tcp_server_->sendToClient(client, protocol_.createMovementAck("stop", false));
        return;
    }

    movement_system_->commandStop(entity_id);
    tcp_server_->sendToClient(client, protocol_.createMovementAck("stop", true));
}

} // namespace atlas
