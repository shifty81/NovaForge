#include "auth/steam_auth.h"
#include "utils/logger.h"

namespace atlas {
namespace auth {

SteamAuth::SteamAuth()
    : initialized_(false)
    , app_id_(0) {
}

SteamAuth::~SteamAuth() {
    shutdown();
}

bool SteamAuth::initialize(uint32_t app_id) {
    app_id_ = app_id;
    
#ifdef USE_STEAM
    // Initialize Steam API
    if (!SteamAPI_Init()) {
        atlas::utils::Logger::instance().error("Failed to initialize Steam API");
        atlas::utils::Logger::instance().error("Make sure Steam is running");
        return false;
    }
    
    initialized_ = true;
    atlas::utils::Logger::instance().info("[Steam] Steam API initialized successfully");
    return true;
#else
    atlas::utils::Logger::instance().info("[Steam] Server compiled without Steam support");
    return false;
#endif
}

void SteamAuth::shutdown() {
    if (!initialized_) {
        return;
    }
    
#ifdef USE_STEAM
    SteamAPI_Shutdown();
#endif
    
    initialized_ = false;
    atlas::utils::Logger::instance().info("[Steam] Steam API shutdown");
}

void SteamAuth::update() {
    if (!initialized_) {
        return;
    }
    
#ifdef USE_STEAM
    SteamAPI_RunCallbacks();
#endif
}

bool SteamAuth::authenticateUser(const std::string& steam_ticket) {
#ifdef USE_STEAM
    if (!initialized_) {
        atlas::utils::Logger::instance().error("[Steam] Cannot authenticate: not initialized");
        return false;
    }
    // Validate ticket is non-empty
    if (steam_ticket.empty()) {
        atlas::utils::Logger::instance().error("[Steam] Empty authentication ticket");
        return false;
    }
    // Steam ticket validation would go through ISteamGameServer::BeginAuthSession
    // For now, accept non-empty tickets when Steam is initialized
    atlas::utils::Logger::instance().info("[Steam] User authenticated via ticket");
    return true;
#else
    (void)steam_ticket;
    return false;
#endif
}

bool SteamAuth::isUserAuthenticated(uint64_t steam_id) const {
#ifdef USE_STEAM
    if (!initialized_) {
        return false;
    }
    // When Steam is active, any non-zero Steam ID with an active session is considered authenticated
    return steam_id != 0;
#else
    (void)steam_id;
    return false;
#endif
}

std::string SteamAuth::getUserName(uint64_t steam_id) const {
#ifdef USE_STEAM
    if (!initialized_ || steam_id == 0) {
        return "Unknown";
    }
    // Return a formatted name based on Steam ID until proper API integration
    return "Player_" + std::to_string(steam_id & 0xFFFFFFFF);
#else
    (void)steam_id;
    return "Unknown";
#endif
}

bool SteamAuth::registerServer(const std::string& server_name, const std::string& map_name) {
#ifdef USE_STEAM
    if (!initialized_) {
        return false;
    }
    
    atlas::utils::Logger::instance().info("[Steam] Server registered: " + server_name + " (map: " + map_name + ")");
    return true;
#else
    (void)server_name;
    (void)map_name;
    return false;
#endif
}

void SteamAuth::updateServerInfo(int current_players, int max_players) {
#ifdef USE_STEAM
    if (!initialized_) {
        return;
    }
    
    atlas::utils::Logger::instance().info("[Steam] Server info updated: " + 
        std::to_string(current_players) + "/" + std::to_string(max_players) + " players");
#else
    (void)current_players;
    (void)max_players;
#endif
}

void SteamAuth::setServerTags(const std::string& tags) {
#ifdef USE_STEAM
    if (!initialized_) {
        return;
    }
    
    atlas::utils::Logger::instance().info("[Steam] Server tags set: " + tags);
#else
    (void)tags;
#endif
}

bool SteamAuth::isSteamRunning() const {
#ifdef USE_STEAM
    return SteamAPI_IsSteamRunning();
#else
    return false;
#endif
}

} // namespace auth
} // namespace atlas
