#include "game_session.h"
#include "game_session_internal.h"
#include "handlers/combat_handler.h"
#include "handlers/station_handler.h"
#include "handlers/movement_handler.h"
#include "handlers/scanner_handler.h"
#include "handlers/mission_handler.h"
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

    // Entity lookup callback shared by all handlers
    auto lookup = [this](int fd) -> std::string {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(fd);
        return (it != players_.end()) ? it->second.entity_id : "";
    };

    // Create domain handlers
    auto combat = std::make_unique<handlers::CombatHandler>(
        world_, tcp_server_, &protocol_, lookup);
    combat_handler_ = combat.get();
    handlers_.push_back(std::move(combat));

    auto station = std::make_unique<handlers::StationHandler>(
        world_, tcp_server_, &protocol_, lookup);
    station_handler_ = station.get();
    handlers_.push_back(std::move(station));

    auto movement = std::make_unique<handlers::MovementHandler>(
        tcp_server_, &protocol_, lookup);
    movement_handler_ = movement.get();
    handlers_.push_back(std::move(movement));

    auto scanner = std::make_unique<handlers::ScannerHandler>(
        tcp_server_, &protocol_, lookup);
    scanner_handler_ = scanner.get();
    handlers_.push_back(std::move(scanner));

    auto mission = std::make_unique<handlers::MissionHandler>(
        tcp_server_, &protocol_, lookup);
    mission_handler_ = mission.get();
    handlers_.push_back(std::move(mission));
}

// ---------------------------------------------------------------------------
// System injection — forwarded to domain handlers
// ---------------------------------------------------------------------------

void GameSession::setTargetingSystem(systems::TargetingSystem* ts) {
    combat_handler_->setTargetingSystem(ts);
}

void GameSession::setStationSystem(systems::StationSystem* ss) {
    station_handler_->setStationSystem(ss);
}

void GameSession::setMovementSystem(systems::MovementSystem* ms) {
    movement_handler_->setMovementSystem(ms);
}

void GameSession::setCombatSystem(systems::CombatSystem* cs) {
    combat_handler_->setCombatSystem(cs);
}

void GameSession::setScannerSystem(systems::ScannerSystem* ss) {
    scanner_handler_->setScannerSystem(ss);
}

void GameSession::setAnomalySystem(systems::AnomalySystem* as) {
    scanner_handler_->setAnomalySystem(as);
}

void GameSession::setMissionSystem(systems::MissionSystem* ms) {
    mission_handler_->setMissionSystem(ms);
}

void GameSession::setMissionGeneratorSystem(systems::MissionGeneratorSystem* mg) {
    mission_handler_->setMissionGeneratorSystem(mg);
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
        // Core session messages handled directly
        case network::MessageType::CONNECT:
            handleConnect(client, data);
            return;
        case network::MessageType::DISCONNECT:
            handleDisconnect(client);
            return;
        case network::MessageType::INPUT_MOVE:
            handleInputMove(client, data);
            return;
        case network::MessageType::CHAT:
            handleChat(client, data);
            return;
        default:
            break;
    }

    // Delegate to domain handlers
    for (auto& handler : handlers_) {
        if (handler->canHandle(type)) {
            handler->handle(type, client, data);
            return;
        }
    }
}

} // namespace atlas
