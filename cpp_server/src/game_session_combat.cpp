#include "game_session.h"
#include "game_session_internal.h"
#include "components/game_components.h"
#include "systems/targeting_system.h"
#include "systems/combat_system.h"
#include <sstream>
#include <mutex>

namespace atlas {

// ---------------------------------------------------------------------------
// TARGET_LOCK handler
// ---------------------------------------------------------------------------

void GameSession::handleTargetLock(const network::ClientConnection& client,
                                   const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    std::string target_id = extractJsonString(data, "target_id");
    if (target_id.empty()) return;

    bool success = false;
    if (targeting_system_) {
        success = targeting_system_->startLock(entity_id, target_id);
    }

    // Send acknowledgement to the requesting client
    std::ostringstream ack;
    ack << "{\"type\":\"target_lock_ack\",\"data\":{"
        << "\"success\":" << (success ? "true" : "false") << ","
        << "\"target_id\":\"" << escapeJsonString(target_id) << "\""
        << "}}";
    tcp_server_->sendToClient(client, ack.str());
}

// ---------------------------------------------------------------------------
// TARGET_UNLOCK handler
// ---------------------------------------------------------------------------

void GameSession::handleTargetUnlock(const network::ClientConnection& client,
                                     const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    std::string target_id = extractJsonString(data, "target_id");
    if (target_id.empty()) return;

    if (targeting_system_) {
        targeting_system_->unlockTarget(entity_id, target_id);
    }

    // Send acknowledgement
    std::ostringstream ack;
    ack << "{\"type\":\"target_unlock_ack\",\"data\":{"
        << "\"target_id\":\"" << escapeJsonString(target_id) << "\""
        << "}}";
    tcp_server_->sendToClient(client, ack.str());
}

// ---------------------------------------------------------------------------
// MODULE_ACTIVATE handler
// ---------------------------------------------------------------------------

void GameSession::handleModuleActivate(const network::ClientConnection& client,
                                       const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    int slot_index = static_cast<int>(extractJsonFloat(data, "\"slot_index\":", -1.0f));
    std::string target_id = extractJsonString(data, "target_id");

    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    // For now, module activation triggers the weapon system for high-slot weapons.
    // Future: full module system with mid/low slot modules, heat, etc.
    auto* weapon = entity->getComponent<components::Weapon>();
    bool success = false;

    if (weapon && !target_id.empty()) {
        // Validate and fire the weapon through the CombatSystem
        if (weapon->cooldown <= 0.0f && weapon->ammo_count > 0) {
            auto* cap = entity->getComponent<components::Capacitor>();
            if (!cap || cap->capacitor >= weapon->capacitor_cost) {
                if (combat_system_) {
                    success = combat_system_->fireWeapon(entity_id, target_id);
                    if (success && cap) {
                        cap->capacitor -= weapon->capacitor_cost;
                    }
                }
            }
        }
    }

    std::ostringstream ack;
    ack << "{\"type\":\"module_activate_ack\",\"data\":{"
        << "\"success\":" << (success ? "true" : "false") << ","
        << "\"slot_index\":" << slot_index
        << "}}";
    tcp_server_->sendToClient(client, ack.str());
}

// ---------------------------------------------------------------------------
// MODULE_DEACTIVATE handler
// ---------------------------------------------------------------------------

void GameSession::handleModuleDeactivate(const network::ClientConnection& client,
                                         const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    int slot_index = static_cast<int>(extractJsonFloat(data, "\"slot_index\":", -1.0f));

    // Module deactivation acknowledged (future: stop active module cycles)
    std::ostringstream ack;
    ack << "{\"type\":\"module_deactivate_ack\",\"data\":{"
        << "\"slot_index\":" << slot_index
        << "}}";
    tcp_server_->sendToClient(client, ack.str());
}

} // namespace atlas
