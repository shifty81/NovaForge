#include "game_session.h"
#include "game_session_internal.h"
#include "components/game_components.h"
#include "systems/station_system.h"
#include "utils/logger.h"
#include <mutex>

namespace atlas {

// ---------------------------------------------------------------------------
// DOCK_REQUEST handler
// ---------------------------------------------------------------------------

void GameSession::handleDockRequest(const network::ClientConnection& client,
                                   const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!station_system_) {
        tcp_server_->sendToClient(client, protocol_.createDockFailed("Station system not available"));
        return;
    }

    std::string station_id = extractJsonString(data, "station_id");
    if (station_id.empty()) {
        tcp_server_->sendToClient(client, protocol_.createDockFailed("No station_id provided"));
        return;
    }

    bool success = station_system_->dockAtStation(entity_id, station_id);
    if (success) {
        tcp_server_->sendToClient(client, protocol_.createDockSuccess(station_id));
        atlas::utils::Logger::instance().info(
            "[GameSession] Player " + entity_id + " docked at " + station_id);
    } else {
        tcp_server_->sendToClient(client, protocol_.createDockFailed("Out of range or already docked"));
    }
}

// ---------------------------------------------------------------------------
// UNDOCK_REQUEST handler
// ---------------------------------------------------------------------------

void GameSession::handleUndockRequest(const network::ClientConnection& client,
                                     const std::string& /*data*/) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!station_system_) {
        tcp_server_->sendToClient(client, protocol_.createError("Station system not available"));
        return;
    }

    bool success = station_system_->undockFromStation(entity_id);
    if (success) {
        tcp_server_->sendToClient(client, protocol_.createUndockSuccess());
        atlas::utils::Logger::instance().info(
            "[GameSession] Player " + entity_id + " undocked");
    } else {
        tcp_server_->sendToClient(client, protocol_.createError("Not currently docked"));
    }
}

// ---------------------------------------------------------------------------
// REPAIR_REQUEST handler
// ---------------------------------------------------------------------------

void GameSession::handleRepairRequest(const network::ClientConnection& client,
                                     const std::string& /*data*/) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!station_system_) {
        tcp_server_->sendToClient(client, protocol_.createError("Station system not available"));
        return;
    }

    auto* entity = world_->getEntity(entity_id);
    if (!entity) {
        tcp_server_->sendToClient(client, protocol_.createError("Entity not found"));
        return;
    }

    // Check if player is docked
    if (!station_system_->isDocked(entity_id)) {
        tcp_server_->sendToClient(client, protocol_.createError("Must be docked to repair"));
        return;
    }

    // Perform repair and get cost
    double cost = station_system_->repairShip(entity_id);

    // Get current HP values after repair
    auto* health = entity->getComponent<components::Health>();
    float shield_hp = health ? health->shield_hp : 0.0f;
    float armor_hp = health ? health->armor_hp : 0.0f;
    float hull_hp = health ? health->hull_hp : 0.0f;

    // Send repair result to client
    tcp_server_->sendToClient(client, 
        protocol_.createRepairResult(static_cast<float>(cost), shield_hp, armor_hp, hull_hp));
    
    atlas::utils::Logger::instance().info(
        "[GameSession] Player " + entity_id + " repaired for " +
        std::to_string(cost) + " Credits");
}

} // namespace atlas
