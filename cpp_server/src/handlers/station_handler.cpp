#include "handlers/station_handler.h"
#include "handlers/handler_utils.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include "systems/station_system.h"
#include "utils/logger.h"

namespace atlas {
namespace handlers {

StationHandler::StationHandler(ecs::World* world, network::TCPServer* tcp_server,
                               network::ProtocolHandler* protocol, EntityLookupFn entity_lookup)
    : world_(world), tcp_server_(tcp_server), protocol_(protocol),
      entity_lookup_(std::move(entity_lookup)) {}

bool StationHandler::canHandle(network::MessageType type) const {
    switch (type) {
        case network::MessageType::DOCK_REQUEST:
        case network::MessageType::UNDOCK_REQUEST:
        case network::MessageType::REPAIR_REQUEST:
            return true;
        default:
            return false;
    }
}

void StationHandler::handle(network::MessageType type,
                            const network::ClientConnection& client,
                            const std::string& data) {
    switch (type) {
        case network::MessageType::DOCK_REQUEST:
            handleDockRequest(client, data);
            break;
        case network::MessageType::UNDOCK_REQUEST:
            handleUndockRequest(client, data);
            break;
        case network::MessageType::REPAIR_REQUEST:
            handleRepairRequest(client, data);
            break;
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// DOCK_REQUEST
// ---------------------------------------------------------------------------

void StationHandler::handleDockRequest(const network::ClientConnection& client,
                                       const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!station_system_) {
        tcp_server_->sendToClient(client, protocol_->createDockFailed("Station system not available"));
        return;
    }

    std::string station_id = extractJsonString(data, "station_id");
    if (station_id.empty()) {
        tcp_server_->sendToClient(client, protocol_->createDockFailed("No station_id provided"));
        return;
    }

    bool success = station_system_->dockAtStation(entity_id, station_id);
    if (success) {
        tcp_server_->sendToClient(client, protocol_->createDockSuccess(station_id));
        atlas::utils::Logger::instance().info(
            "[StationHandler] Player " + entity_id + " docked at " + station_id);
    } else {
        tcp_server_->sendToClient(client, protocol_->createDockFailed("Out of range or already docked"));
    }
}

// ---------------------------------------------------------------------------
// UNDOCK_REQUEST
// ---------------------------------------------------------------------------

void StationHandler::handleUndockRequest(const network::ClientConnection& client,
                                         const std::string& /*data*/) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!station_system_) {
        tcp_server_->sendToClient(client, protocol_->createError("Station system not available"));
        return;
    }

    bool success = station_system_->undockFromStation(entity_id);
    if (success) {
        tcp_server_->sendToClient(client, protocol_->createUndockSuccess());
        atlas::utils::Logger::instance().info(
            "[StationHandler] Player " + entity_id + " undocked");
    } else {
        tcp_server_->sendToClient(client, protocol_->createError("Not currently docked"));
    }
}

// ---------------------------------------------------------------------------
// REPAIR_REQUEST
// ---------------------------------------------------------------------------

void StationHandler::handleRepairRequest(const network::ClientConnection& client,
                                         const std::string& /*data*/) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!station_system_) {
        tcp_server_->sendToClient(client, protocol_->createError("Station system not available"));
        return;
    }

    auto* entity = world_->getEntity(entity_id);
    if (!entity) {
        tcp_server_->sendToClient(client, protocol_->createError("Entity not found"));
        return;
    }

    if (!station_system_->isDocked(entity_id)) {
        tcp_server_->sendToClient(client, protocol_->createError("Must be docked to repair"));
        return;
    }

    double cost = station_system_->repairShip(entity_id);

    auto* health = entity->getComponent<components::Health>();
    float shield_hp = health ? health->shield_hp : 0.0f;
    float armor_hp = health ? health->armor_hp : 0.0f;
    float hull_hp = health ? health->hull_hp : 0.0f;

    tcp_server_->sendToClient(client,
        protocol_->createRepairResult(static_cast<float>(cost), shield_hp, armor_hp, hull_hp));

    atlas::utils::Logger::instance().info(
        "[StationHandler] Player " + entity_id + " repaired for " +
        std::to_string(cost) + " Credits");
}

} // namespace handlers
} // namespace atlas
