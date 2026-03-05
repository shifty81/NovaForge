#include "handlers/movement_handler.h"
#include "handlers/handler_utils.h"
#include "systems/movement_system.h"
#include "utils/logger.h"

namespace atlas {
namespace handlers {

MovementHandler::MovementHandler(network::TCPServer* tcp_server,
                                 network::ProtocolHandler* protocol, EntityLookupFn entity_lookup)
    : tcp_server_(tcp_server), protocol_(protocol),
      entity_lookup_(std::move(entity_lookup)) {}

bool MovementHandler::canHandle(network::MessageType type) const {
    switch (type) {
        case network::MessageType::WARP_REQUEST:
        case network::MessageType::APPROACH:
        case network::MessageType::ORBIT:
        case network::MessageType::STOP:
            return true;
        default:
            return false;
    }
}

void MovementHandler::handle(network::MessageType type,
                             const network::ClientConnection& client,
                             const std::string& data) {
    switch (type) {
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
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// WARP_REQUEST
// ---------------------------------------------------------------------------

void MovementHandler::handleWarpRequest(const network::ClientConnection& client,
                                        const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!movement_system_) {
        tcp_server_->sendToClient(client,
            protocol_->createWarpResult(false, "Movement system not available"));
        return;
    }

    float dest_x = extractJsonFloat(data, "\"dest_x\":");
    float dest_y = extractJsonFloat(data, "\"dest_y\":");
    float dest_z = extractJsonFloat(data, "\"dest_z\":");

    bool success = movement_system_->commandWarp(entity_id, dest_x, dest_y, dest_z);
    if (success) {
        tcp_server_->sendToClient(client, protocol_->createWarpResult(true));
        atlas::utils::Logger::instance().info(
            "[MovementHandler] Player " + entity_id + " warping to (" +
            std::to_string(dest_x) + ", " + std::to_string(dest_y) + ", " +
            std::to_string(dest_z) + ")");
    } else {
        std::string reason = movement_system_->isWarpDisrupted(entity_id)
                                 ? "Warp drive disrupted"
                                 : "Destination too close (min 150km)";
        tcp_server_->sendToClient(client, protocol_->createWarpResult(false, reason));
    }
}

// ---------------------------------------------------------------------------
// APPROACH
// ---------------------------------------------------------------------------

void MovementHandler::handleApproach(const network::ClientConnection& client,
                                     const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!movement_system_) {
        tcp_server_->sendToClient(client, protocol_->createMovementAck("approach", false));
        return;
    }

    std::string target_id = extractJsonString(data, "target_id");
    if (target_id.empty()) {
        tcp_server_->sendToClient(client, protocol_->createMovementAck("approach", false));
        return;
    }

    movement_system_->commandApproach(entity_id, target_id);
    tcp_server_->sendToClient(client, protocol_->createMovementAck("approach", true));
}

// ---------------------------------------------------------------------------
// ORBIT
// ---------------------------------------------------------------------------

void MovementHandler::handleOrbit(const network::ClientConnection& client,
                                  const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!movement_system_) {
        tcp_server_->sendToClient(client, protocol_->createMovementAck("orbit", false));
        return;
    }

    std::string target_id = extractJsonString(data, "target_id");
    float distance = extractJsonFloat(data, "\"distance\":", 5000.0f);
    if (target_id.empty()) {
        tcp_server_->sendToClient(client, protocol_->createMovementAck("orbit", false));
        return;
    }

    movement_system_->commandOrbit(entity_id, target_id, distance);
    tcp_server_->sendToClient(client, protocol_->createMovementAck("orbit", true));
}

// ---------------------------------------------------------------------------
// STOP
// ---------------------------------------------------------------------------

void MovementHandler::handleStop(const network::ClientConnection& client,
                                 const std::string& /*data*/) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!movement_system_) {
        tcp_server_->sendToClient(client, protocol_->createMovementAck("stop", false));
        return;
    }

    movement_system_->commandStop(entity_id);
    tcp_server_->sendToClient(client, protocol_->createMovementAck("stop", true));
}

} // namespace handlers
} // namespace atlas
