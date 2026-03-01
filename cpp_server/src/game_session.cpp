#include "game_session.h"
#include "game_session_internal.h"
#include "systems/snapshot_replication_system.h"
#include "utils/logger.h"
#include <sstream>
#include <mutex>

namespace atlas {

// ---------------------------------------------------------------------------
// Construction / Initialization
// ---------------------------------------------------------------------------

GameSession::GameSession(ecs::World* world, network::TCPServer* tcp_server,
                         const std::string& data_path)
    : world_(world)
    , tcp_server_(tcp_server) {
    // Load ship data from JSON
    ship_db_.loadFromDirectory(data_path);
}

void GameSession::initialize() {
    // Register the message handler on the TCP server
    tcp_server_->setMessageHandler(
        [this](const network::ClientConnection& client, const std::string& raw) {
            onClientMessage(client, raw);
        }
    );

    // Spawn a handful of NPC enemies so the world isn't empty
    spawnInitialNPCs();

    atlas::utils::Logger::instance().info(
        "[GameSession] Initialized – " + std::to_string(world_->getEntityCount()) +
        " entities in world, " + std::to_string(ship_db_.getShipCount()) +
        " ship templates loaded");
}

// ---------------------------------------------------------------------------
// Per-tick update
// ---------------------------------------------------------------------------

void GameSession::update(float /*delta_time*/) {
    std::lock_guard<std::mutex> lock(players_mutex_);

    if (snapshot_replication_) {
        // Use delta-compressed, per-client state updates
        for (const auto& kv : players_) {
            int client_fd = kv.first;
            uint64_t seq = snapshot_sequence_++;
            std::string state_msg =
                snapshot_replication_->buildDeltaUpdate(client_fd, seq);
            tcp_server_->sendToClient(kv.second.connection, state_msg);
        }
    } else {
        // Fallback: broadcast full state to every client
        std::string state_msg = buildStateUpdate();
        for (const auto& kv : players_) {
            tcp_server_->sendToClient(kv.second.connection, state_msg);
        }
    }
}

int GameSession::getPlayerCount() const {
    std::lock_guard<std::mutex> lock(players_mutex_);
    return static_cast<int>(players_.size());
}

std::vector<std::string> GameSession::getPlayerNames() const {
    std::lock_guard<std::mutex> lock(players_mutex_);
    std::vector<std::string> names;
    names.reserve(players_.size());
    for (const auto& kv : players_) {
        names.push_back(kv.second.character_name);
    }
    return names;
}

bool GameSession::kickPlayer(const std::string& character_name) {
    std::lock_guard<std::mutex> lock(players_mutex_);
    for (auto it = players_.begin(); it != players_.end(); ++it) {
        if (it->second.character_name == character_name) {
            // Remove entity from world.
            if (world_) {
                world_->destroyEntity(it->second.entity_id);
            }
            players_.erase(it);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Incoming message dispatch
// ---------------------------------------------------------------------------

void GameSession::onClientMessage(const network::ClientConnection& client,
                                  const std::string& raw) {
    network::MessageType type;
    std::string data;

    if (!protocol_.parseMessage(raw, type, data)) {
        atlas::utils::Logger::instance().warn(
            "[GameSession] Unrecognised message from " + client.address);
        return;
    }

    switch (type) {
        case network::MessageType::CONNECT:
            handleConnect(client, data);
            break;
        case network::MessageType::DISCONNECT:
            handleDisconnect(client);
            break;
        case network::MessageType::INPUT_MOVE:
            handleInputMove(client, data);
            break;
        case network::MessageType::CHAT:
            handleChat(client, data);
            break;
        case network::MessageType::TARGET_LOCK:
            handleTargetLock(client, data);
            break;
        case network::MessageType::TARGET_UNLOCK:
            handleTargetUnlock(client, data);
            break;
        case network::MessageType::MODULE_ACTIVATE:
            handleModuleActivate(client, data);
            break;
        case network::MessageType::MODULE_DEACTIVATE:
            handleModuleDeactivate(client, data);
            break;
        case network::MessageType::DOCK_REQUEST:
            handleDockRequest(client, data);
            break;
        case network::MessageType::UNDOCK_REQUEST:
            handleUndockRequest(client, data);
            break;
        case network::MessageType::REPAIR_REQUEST:
            handleRepairRequest(client, data);
            break;
        case network::MessageType::WARP_REQUEST:
            handleWarpRequest(client, data);
            break;
        case network::MessageType::APPROACH:
            handleApproach(client, data);
            break;
        case network::MessageType::ORBIT:
            handleOrbit(client, data);
            break;
        case network::MessageType::STOP:
            handleStop(client, data);
            break;
        case network::MessageType::SCAN_START:
            handleScanStart(client, data);
            break;
        case network::MessageType::SCAN_STOP:
            handleScanStop(client, data);
            break;
        case network::MessageType::ANOMALY_LIST:
            handleAnomalyList(client, data);
            break;
        case network::MessageType::MISSION_LIST:
            handleMissionList(client, data);
            break;
        case network::MessageType::ACCEPT_MISSION:
            handleAcceptMission(client, data);
            break;
        case network::MessageType::ABANDON_MISSION:
            handleAbandonMission(client, data);
            break;
        case network::MessageType::MISSION_PROGRESS:
            handleMissionProgress(client, data);
            break;
        default:
            break;
    }
}

} // namespace atlas
