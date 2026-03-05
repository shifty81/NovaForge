#ifndef NOVAFORGE_GAME_SESSION_H
#define NOVAFORGE_GAME_SESSION_H

#include "ecs/world.h"
#include "network/tcp_server.h"
#include "network/protocol_handler.h"
#include "data/ship_database.h"
#include "handlers/message_handler.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <memory>

namespace atlas {

// Forward declarations — systems
namespace pcg { class PCGManager; }
namespace systems { 
    class TargetingSystem;
    class StationSystem;
    class MovementSystem;
    class CombatSystem;
    class ScannerSystem;
    class AnomalySystem;
    class MissionSystem;
    class MissionGeneratorSystem;
    class SnapshotReplicationSystem;
    class InterestManagementSystem;
}

// Forward declarations — domain handlers
namespace handlers {
    class CombatHandler;
    class StationHandler;
    class MovementHandler;
    class ScannerHandler;
    class MissionHandler;
}

/**
 * @brief Manages game sessions: connects networking to the ECS world
 *
 * Bridges TCP client connections with the game world by:
 * - Handling connect/disconnect messages
 * - Spawning player entities on connect
 * - Processing player input (movement, commands)
 * - Broadcasting entity state updates each tick
 * - Spawning NPC entities on startup
 */
class GameSession {
public:
    explicit GameSession(ecs::World* world, network::TCPServer* tcp_server,
                         const std::string& data_path = "../data");
    ~GameSession() = default;

    /// Initialize message handlers and spawn initial NPCs
    void initialize();

    /// Called each server tick to broadcast state to all clients
    void update(float delta_time);

    /// Get the number of connected players
    int getPlayerCount() const;

    /// Get list of connected player names
    std::vector<std::string> getPlayerNames() const;

    /// Kick a player by character name. Returns true if found and removed.
    bool kickPlayer(const std::string& character_name);

    /// System injection — forwarded to domain handlers
    void setTargetingSystem(systems::TargetingSystem* ts);
    void setStationSystem(systems::StationSystem* ss);
    void setMovementSystem(systems::MovementSystem* ms);
    void setCombatSystem(systems::CombatSystem* cs);
    void setScannerSystem(systems::ScannerSystem* ss);
    void setAnomalySystem(systems::AnomalySystem* as);
    void setMissionSystem(systems::MissionSystem* ms);
    void setMissionGeneratorSystem(systems::MissionGeneratorSystem* mg);

    /// Set pointer to the SnapshotReplicationSystem for delta state updates
    void setSnapshotReplicationSystem(systems::SnapshotReplicationSystem* srs) { snapshot_replication_ = srs; }

    /// Set pointer to the InterestManagementSystem for per-client entity filtering
    void setInterestManagementSystem(systems::InterestManagementSystem* ims) { interest_management_ = ims; }

    /// Set pointer to the PCGManager for procedural content generation
    void setPCGManager(pcg::PCGManager* mgr) { pcg_manager_ = mgr; }

    /// Get the ship database (read-only)
    const data::ShipDatabase& getShipDatabase() const { return ship_db_; }

private:
    // --- Message handlers ---
    /**
     * Routes incoming client messages to appropriate handlers
     * @param client Client connection info
     * @param raw Raw message string from network
     */
    void onClientMessage(const network::ClientConnection& client, const std::string& raw);
    
    /**
     * Handle client connection
     * 
     * Creates a player entity and spawns them in the game world.
     * Expected message format: {"type":"connect","character_name":"PlayerName"}
     * 
     * @param client Client connection info
     * @param data JSON message data
     */
    void handleConnect(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle client disconnection
     * 
     * Removes player entity from world and cleans up player info.
     * 
     * @param client Client connection info
     */
    void handleDisconnect(const network::ClientConnection& client);
    
    /**
     * Handle player movement input
     * 
     * Expected message format: {"type":"input_move","forward":1.0,"strafe":0.5}
     * Values range from -1.0 to 1.0 for each axis.
     * 
     * @param client Client connection info
     * @param data JSON message data with forward/strafe values
     */
    void handleInputMove(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle chat message
     * 
     * Broadcasts chat message to all connected clients.
     * Expected format: {"type":"chat","message":"Hello world"}
     * 
     * @param client Client connection info
     * @param data JSON message data with message content
     */
    void handleChat(const network::ClientConnection& client, const std::string& data);

    // --- State broadcast ---
    /**
     * Build full state update message
     * 
     * Creates JSON message with all entity states including:
     * - Position, velocity, rotation
     * - Health (shield, armor, hull)
     * - Target locks
     * - Active modules
     * 
     * @return JSON string with format: {"type":"state_update","entities":[...]}
     */
    std::string buildStateUpdate() const;
    
    /**
     * Build entity spawn notification
     * 
     * Creates JSON message to notify clients when a new entity appears.
     * Includes full entity data (type, ship, faction, initial stats).
     * 
     * @param entity_id ID of entity to spawn
     * @return JSON string with format: {"type":"spawn_entity","entity":{...}}
     */
    std::string buildSpawnEntity(const std::string& entity_id) const;

    // --- NPC management ---
    void spawnInitialNPCs();
    void spawnNPC(const std::string& id, const std::string& name, const std::string& ship,
                  const std::string& faction, float x, float y, float z);

    // --- Player entity helpers ---
    std::string createPlayerEntity(const std::string& player_id,
                                   const std::string& character_name,
                                   const std::string& ship_type = "rifter");

    // --- Helpers ---
    /**
     * Extract a string value from a simple JSON object (lightweight parser)
     * 
     * Simple JSON parser for extracting string fields. Looks for "key":"value"
     * pattern in the JSON string. Not a full JSON parser - only handles simple
     * key-value pairs.
     * 
     * @param json JSON string to parse
     * @param key Key name to extract
     * @return Extracted string value, or empty string if not found
     */
    static std::string extractJsonString(const std::string& json, const std::string& key);
    
    /**
     * Extract a float value from a simple JSON object
     * 
     * Simple JSON parser for extracting numeric fields. Looks for "key":value
     * pattern (no quotes around value).
     * 
     * @param json JSON string to parse
     * @param key Key name to extract
     * @param fallback Default value if key not found or parsing fails
     * @return Extracted float value, or fallback if not found
     */
    static float extractJsonFloat(const std::string& json, const std::string& key, float fallback = 0.0f);

    ecs::World* world_;
    network::TCPServer* tcp_server_;
    network::ProtocolHandler protocol_;
    data::ShipDatabase ship_db_;

    // Systems kept on GameSession (used in core connect/disconnect/update)
    systems::SnapshotReplicationSystem* snapshot_replication_ = nullptr;
    systems::InterestManagementSystem* interest_management_ = nullptr;
    pcg::PCGManager* pcg_manager_ = nullptr;

    // Domain message handlers
    std::vector<std::unique_ptr<handlers::IMessageHandler>> handlers_;
    handlers::CombatHandler* combat_handler_ = nullptr;
    handlers::StationHandler* station_handler_ = nullptr;
    handlers::MovementHandler* movement_handler_ = nullptr;
    handlers::ScannerHandler* scanner_handler_ = nullptr;
    handlers::MissionHandler* mission_handler_ = nullptr;

    // Map socket → entity_id for connected players
    struct PlayerInfo {
        std::string entity_id;
        std::string character_name;
        network::ClientConnection connection;
    };

    std::unordered_map<int, PlayerInfo> players_;  // keyed by socket fd
    mutable std::mutex players_mutex_;

    std::atomic<uint32_t> next_entity_id_{1};
    mutable std::atomic<uint64_t> snapshot_sequence_{0};  // Sequence number for snapshots
};

} // namespace atlas

#endif // NOVAFORGE_GAME_SESSION_H
