#include "handlers/combat_handler.h"
#include "handlers/handler_utils.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include "systems/targeting_system.h"
#include "systems/combat_system.h"
#include <sstream>

namespace atlas {
namespace handlers {

CombatHandler::CombatHandler(ecs::World* world, network::TCPServer* tcp_server,
                             network::ProtocolHandler* protocol, EntityLookupFn entity_lookup)
    : world_(world), tcp_server_(tcp_server), protocol_(protocol),
      entity_lookup_(std::move(entity_lookup)) {}

bool CombatHandler::canHandle(network::MessageType type) const {
    switch (type) {
        case network::MessageType::TARGET_LOCK:
        case network::MessageType::TARGET_UNLOCK:
        case network::MessageType::MODULE_ACTIVATE:
        case network::MessageType::MODULE_DEACTIVATE:
            return true;
        default:
            return false;
    }
}

void CombatHandler::handle(network::MessageType type,
                           const network::ClientConnection& client,
                           const std::string& data) {
    switch (type) {
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
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// TARGET_LOCK
// ---------------------------------------------------------------------------

void CombatHandler::handleTargetLock(const network::ClientConnection& client,
                                     const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    std::string target_id = extractJsonString(data, "target_id");
    if (target_id.empty()) return;

    bool success = false;
    if (targeting_system_) {
        success = targeting_system_->startLock(entity_id, target_id);
    }

    std::ostringstream ack;
    ack << "{\"type\":\"target_lock_ack\",\"data\":{"
        << "\"success\":" << (success ? "true" : "false") << ","
        << "\"target_id\":\"" << escapeJsonString(target_id) << "\""
        << "}}";
    tcp_server_->sendToClient(client, ack.str());
}

// ---------------------------------------------------------------------------
// TARGET_UNLOCK
// ---------------------------------------------------------------------------

void CombatHandler::handleTargetUnlock(const network::ClientConnection& client,
                                       const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    std::string target_id = extractJsonString(data, "target_id");
    if (target_id.empty()) return;

    if (targeting_system_) {
        targeting_system_->unlockTarget(entity_id, target_id);
    }

    std::ostringstream ack;
    ack << "{\"type\":\"target_unlock_ack\",\"data\":{"
        << "\"target_id\":\"" << escapeJsonString(target_id) << "\""
        << "}}";
    tcp_server_->sendToClient(client, ack.str());
}

// ---------------------------------------------------------------------------
// MODULE_ACTIVATE
// ---------------------------------------------------------------------------

void CombatHandler::handleModuleActivate(const network::ClientConnection& client,
                                         const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    int slot_index = static_cast<int>(extractJsonFloat(data, "\"slot_index\":", -1.0f));
    std::string target_id = extractJsonString(data, "target_id");

    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* weapon = entity->getComponent<components::Weapon>();
    bool success = false;

    if (weapon && !target_id.empty()) {
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
// MODULE_DEACTIVATE
// ---------------------------------------------------------------------------

void CombatHandler::handleModuleDeactivate(const network::ClientConnection& client,
                                           const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    int slot_index = static_cast<int>(extractJsonFloat(data, "\"slot_index\":", -1.0f));

    std::ostringstream ack;
    ack << "{\"type\":\"module_deactivate_ack\",\"data\":{"
        << "\"slot_index\":" << slot_index
        << "}}";
    tcp_server_->sendToClient(client, ack.str());
}

} // namespace handlers
} // namespace atlas
