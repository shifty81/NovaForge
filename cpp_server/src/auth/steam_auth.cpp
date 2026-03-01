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
    // TODO: Implement Steam authentication ticket validation
    // This would use ISteamGameServer::BeginAuthSession
    return true;
#else
    return false;
#endif
}

bool SteamAuth::isUserAuthenticated(uint64_t steam_id) const {
#ifdef USE_STEAM
    // TODO: Track authenticated users
    return true;
#else
    return false;
#endif
}

std::string SteamAuth::getUserName(uint64_t steam_id) const {
#ifdef USE_STEAM
    // TODO: Get Steam user name from Steam ID
    return "SteamUser";
#else
    return "Unknown";
#endif
}

bool SteamAuth::registerServer(const std::string& server_name, const std::string& map_name) {
#ifdef USE_STEAM
    if (!initialized_) {
        return false;
    }
    
    // TODO: Register with Steam game server API
    // This would use ISteamGameServer::LogOnAnonymous() and SetServerName()
    atlas::utils::Logger::instance().info("[Steam] Server registered: " + server_name);
    return true;
#else
    return false;
#endif
}

void SteamAuth::updateServerInfo(int current_players, int max_players) {
#ifdef USE_STEAM
    if (!initialized_) {
        return;
    }
    
    // TODO: Update server info with ISteamGameServer::SetMaxPlayerCount()
    // and ISteamGameServer::SetPasswordProtected()
#endif
}

void SteamAuth::setServerTags(const std::string& tags) {
#ifdef USE_STEAM
    if (!initialized_) {
        return;
    }
    
    // TODO: Set server tags with ISteamGameServer::SetGameTags()
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
