#include "game_session.h"
#include "game_session_internal.h"
#include "components/game_components.h"
#include "systems/snapshot_replication_system.h"
#include "systems/interest_management_system.h"
#include "utils/logger.h"
#include <sstream>
#include <mutex>

namespace atlas {

// ---------------------------------------------------------------------------
// CONNECT handler
// ---------------------------------------------------------------------------

void GameSession::handleConnect(const network::ClientConnection& client,
                                const std::string& data) {
    // Reject duplicate connections from the same socket
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        if (players_.find(static_cast<int>(client.socket)) != players_.end()) {
            atlas::utils::Logger::instance().warn(
                "[GameSession] Duplicate connect from " + client.address + ", ignoring");
            return;
        }
    }

    std::string player_id   = extractJsonString(data, "player_id");
    std::string char_name   = extractJsonString(data, "character_name");

    if (player_id.empty()) {
        player_id = "player_" + std::to_string(client.socket);
    }
    if (char_name.empty()) {
        char_name = "Pilot";
    }

    // Enforce length limits
    if (char_name.size() > MAX_CHARACTER_NAME_LEN) {
        char_name.resize(MAX_CHARACTER_NAME_LEN);
    }

    // Create the player's ship entity in the game world
    std::string entity_id = createPlayerEntity(player_id, char_name);

    // Record the mapping and snapshot other players for notification
    std::vector<PlayerInfo> others;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        PlayerInfo info;
        info.entity_id      = entity_id;
        info.character_name  = char_name;
        info.connection      = client;
        players_[static_cast<int>(client.socket)] = info;

        for (const auto& kv : players_) {
            if (kv.first != static_cast<int>(client.socket)) {
                others.push_back(kv.second);
            }
        }
    }

    // Escape char_name for safe JSON embedding
    std::string safe_name = escapeJsonString(char_name);

    // Send connect_ack with the player's entity id
    std::ostringstream ack;
    ack << "{\"type\":\"connect_ack\","
        << "\"data\":{"
        << "\"success\":true,"
        << "\"player_entity_id\":\"" << entity_id << "\","
        << "\"message\":\"Welcome, " << safe_name << "!\""
        << "}}";
    tcp_server_->sendToClient(client, ack.str());

    // Send spawn_entity messages for every existing entity
    for (auto* entity : world_->getAllEntities()) {
        std::string spawn_msg = buildSpawnEntity(entity->getId());
        tcp_server_->sendToClient(client, spawn_msg);
    }

    atlas::utils::Logger::instance().info(
        "[GameSession] Player connected: " + char_name + " (entity " + entity_id + ")");

    // Notify other clients about the new player entity
    std::string new_spawn = buildSpawnEntity(entity_id);
    for (const auto& other : others) {
        tcp_server_->sendToClient(other.connection, new_spawn);
    }

    // Register with interest management / snapshot replication
    int fd = static_cast<int>(client.socket);
    if (interest_management_) {
        interest_management_->registerClient(fd, entity_id);
    }
}

// ---------------------------------------------------------------------------
// DISCONNECT handler
// ---------------------------------------------------------------------------

void GameSession::handleDisconnect(const network::ClientConnection& client) {
    int fd = static_cast<int>(client.socket);
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(fd);
        if (it != players_.end()) {
            entity_id = it->second.entity_id;
            atlas::utils::Logger::instance().info(
                "[GameSession] Player disconnected: " + it->second.character_name);
            players_.erase(it);
        }
    }

    // Clean up replication / interest state for this client
    if (snapshot_replication_) {
        snapshot_replication_->clearClient(fd);
    }
    if (interest_management_) {
        interest_management_->unregisterClient(fd);
    }

    if (!entity_id.empty()) {
        world_->destroyEntity(entity_id);

        // Tell remaining clients to remove the entity
        std::ostringstream msg;
        msg << "{\"type\":\"destroy_entity\","
            << "\"data\":{\"entity_id\":\"" << entity_id << "\"}}";
        std::string destroy_msg = msg.str();

        std::lock_guard<std::mutex> lock(players_mutex_);
        for (const auto& kv : players_) {
            tcp_server_->sendToClient(kv.second.connection, destroy_msg);
        }
    }
}

// ---------------------------------------------------------------------------
// INPUT_MOVE handler
// ---------------------------------------------------------------------------

void GameSession::handleInputMove(const network::ClientConnection& client,
                                  const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* vel = entity->getComponent<components::Velocity>();
    if (!vel) return;

    // Parse velocity – the client sends {"velocity":{"x":..,"y":..,"z":..}}
    // Our lightweight parser operates on the inner data block.
    float vx = extractJsonFloat(data, "\"x\":", 0.0f);
    float vy = extractJsonFloat(data, "\"y\":", 0.0f);
    float vz = extractJsonFloat(data, "\"z\":", 0.0f);

    vel->vx = vx;
    vel->vy = vy;
    vel->vz = vz;
}

// ---------------------------------------------------------------------------
// CHAT handler
// ---------------------------------------------------------------------------

void GameSession::handleChat(const network::ClientConnection& client,
                             const std::string& data) {
    std::string sender;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it != players_.end()) {
            sender = it->second.character_name;
        }
    }

    std::string message = extractJsonString(data, "message");

    // Enforce message length limit
    if (message.size() > MAX_CHAT_MESSAGE_LEN) {
        message.resize(MAX_CHAT_MESSAGE_LEN);
    }

    // Escape for safe JSON embedding
    std::string chat_msg = protocol_.createChatMessage(
        escapeJsonString(sender), escapeJsonString(message));

    // Broadcast chat to everyone
    tcp_server_->broadcastToAll(chat_msg);
}

} // namespace atlas
