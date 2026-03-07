#include "handlers/mission_handler.h"
#include "handlers/handler_utils.h"
#include "systems/mission_system.h"
#include "systems/mission_generator_system.h"
#include <sstream>

namespace atlas {
namespace handlers {

MissionHandler::MissionHandler(network::TCPServer* tcp_server,
                               network::ProtocolHandler* protocol, EntityLookupFn entity_lookup)
    : tcp_server_(tcp_server), protocol_(protocol),
      entity_lookup_(std::move(entity_lookup)) {}

bool MissionHandler::canHandle(network::MessageType type) const {
    switch (type) {
        case network::MessageType::MISSION_LIST:
        case network::MessageType::ACCEPT_MISSION:
        case network::MessageType::ABANDON_MISSION:
        case network::MessageType::MISSION_PROGRESS:
            return true;
        default:
            return false;
    }
}

void MissionHandler::handle(network::MessageType type,
                            const network::ClientConnection& client,
                            const std::string& data) {
    switch (type) {
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

// ---------------------------------------------------------------------------
// MISSION_LIST
// ---------------------------------------------------------------------------

void MissionHandler::handleMissionList(const network::ClientConnection& client,
                                       const std::string& data) {
    if (!mission_generator_) {
        tcp_server_->sendToClient(client, protocol_->createError("Mission system not available"));
        return;
    }

    std::string system_id = extractJsonString(data, "system_id");
    if (system_id.empty()) {
        tcp_server_->sendToClient(client, protocol_->createError("No system_id provided"));
        return;
    }

    auto missions = mission_generator_->getAvailableMissions(system_id);

    std::ostringstream missions_json;
    missions_json << "[";
    for (size_t i = 0; i < missions.size(); ++i) {
        if (i > 0) missions_json << ",";
        const auto& m = missions[i].mission;
        missions_json << "{\"index\":" << i << ","
                      << "\"mission_id\":\"" << m.mission_id << "\","
                      << "\"name\":\"" << m.name << "\","
                      << "\"level\":" << m.level << ","
                      << "\"type\":\"" << m.type << "\","
                      << "\"isc_reward\":" << m.isc_reward << ","
                      << "\"standing_reward\":" << m.standing_reward << "}";
    }
    missions_json << "]";

    tcp_server_->sendToClient(client,
        protocol_->createMissionList(system_id, static_cast<int>(missions.size()),
                                     missions_json.str()));
}

// ---------------------------------------------------------------------------
// ACCEPT_MISSION
// ---------------------------------------------------------------------------

void MissionHandler::handleAcceptMission(const network::ClientConnection& client,
                                         const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!mission_generator_) {
        tcp_server_->sendToClient(client, protocol_->createError("Mission system not available"));
        return;
    }

    std::string system_id = extractJsonString(data, "system_id");
    int mission_index = static_cast<int>(extractJsonFloat(data, "mission_index", -1.0f));

    if (system_id.empty() || mission_index < 0) {
        tcp_server_->sendToClient(client,
            protocol_->createMissionResult(false, "", "accept", "Invalid system_id or mission_index"));
        return;
    }

    bool accepted = mission_generator_->offerMissionToPlayer(entity_id, system_id, mission_index);
    if (accepted) {
        tcp_server_->sendToClient(client,
            protocol_->createMissionResult(true, "", "accept", "Mission accepted"));
    } else {
        tcp_server_->sendToClient(client,
            protocol_->createMissionResult(false, "", "accept", "Failed to accept mission"));
    }
}

// ---------------------------------------------------------------------------
// ABANDON_MISSION
// ---------------------------------------------------------------------------

void MissionHandler::handleAbandonMission(const network::ClientConnection& client,
                                          const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!mission_system_) {
        tcp_server_->sendToClient(client, protocol_->createError("Mission system not available"));
        return;
    }

    std::string mission_id = extractJsonString(data, "mission_id");
    if (mission_id.empty()) {
        tcp_server_->sendToClient(client,
            protocol_->createMissionResult(false, "", "abandon", "No mission_id provided"));
        return;
    }

    mission_system_->abandonMission(entity_id, mission_id);
    tcp_server_->sendToClient(client,
        protocol_->createMissionResult(true, mission_id, "abandon", "Mission abandoned"));
}

// ---------------------------------------------------------------------------
// MISSION_PROGRESS
// ---------------------------------------------------------------------------

void MissionHandler::handleMissionProgress(const network::ClientConnection& client,
                                           const std::string& data) {
    std::string entity_id = entity_lookup_(static_cast<int>(client.socket));
    if (entity_id.empty()) return;

    if (!mission_system_) {
        tcp_server_->sendToClient(client, protocol_->createError("Mission system not available"));
        return;
    }

    std::string mission_id = extractJsonString(data, "mission_id");
    std::string objective_type = extractJsonString(data, "objective_type");
    std::string target = extractJsonString(data, "target");
    int count = static_cast<int>(extractJsonFloat(data, "count", 1.0f));

    if (mission_id.empty() || objective_type.empty()) {
        tcp_server_->sendToClient(client,
            protocol_->createMissionResult(false, mission_id, "progress", "Missing mission_id or objective_type"));
        return;
    }

    mission_system_->recordProgress(entity_id, mission_id, objective_type, target, count);
    tcp_server_->sendToClient(client,
        protocol_->createMissionResult(true, mission_id, "progress", "Progress recorded"));
}

} // namespace handlers
} // namespace atlas
