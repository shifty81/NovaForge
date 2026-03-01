#include "game_session.h"
#include "game_session_internal.h"
#include "systems/mission_system.h"
#include "systems/mission_generator_system.h"
#include <sstream>
#include <mutex>

namespace atlas {

// ---------------------------------------------------------------------------
// MISSION LIST handler
// ---------------------------------------------------------------------------

void GameSession::handleMissionList(const network::ClientConnection& client,
                                     const std::string& data) {
    if (!mission_generator_) {
        tcp_server_->sendToClient(client, protocol_.createError("Mission system not available"));
        return;
    }

    std::string system_id = extractJsonString(data, "system_id");
    if (system_id.empty()) {
        tcp_server_->sendToClient(client, protocol_.createError("No system_id provided"));
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
                      << "\"isk_reward\":" << m.isk_reward << ","
                      << "\"standing_reward\":" << m.standing_reward << "}";
    }
    missions_json << "]";

    tcp_server_->sendToClient(client,
        protocol_.createMissionList(system_id, static_cast<int>(missions.size()),
                                    missions_json.str()));
}

// ---------------------------------------------------------------------------
// ACCEPT MISSION handler
// ---------------------------------------------------------------------------

void GameSession::handleAcceptMission(const network::ClientConnection& client,
                                       const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!mission_generator_) {
        tcp_server_->sendToClient(client, protocol_.createError("Mission system not available"));
        return;
    }

    std::string system_id = extractJsonString(data, "system_id");
    int mission_index = static_cast<int>(extractJsonFloat(data, "mission_index", -1.0f));

    if (system_id.empty() || mission_index < 0) {
        tcp_server_->sendToClient(client,
            protocol_.createMissionResult(false, "", "accept", "Invalid system_id or mission_index"));
        return;
    }

    bool accepted = mission_generator_->offerMissionToPlayer(entity_id, system_id, mission_index);
    if (accepted) {
        tcp_server_->sendToClient(client,
            protocol_.createMissionResult(true, "", "accept", "Mission accepted"));
    } else {
        tcp_server_->sendToClient(client,
            protocol_.createMissionResult(false, "", "accept", "Failed to accept mission"));
    }
}

// ---------------------------------------------------------------------------
// ABANDON MISSION handler
// ---------------------------------------------------------------------------

void GameSession::handleAbandonMission(const network::ClientConnection& client,
                                        const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!mission_system_) {
        tcp_server_->sendToClient(client, protocol_.createError("Mission system not available"));
        return;
    }

    std::string mission_id = extractJsonString(data, "mission_id");
    if (mission_id.empty()) {
        tcp_server_->sendToClient(client,
            protocol_.createMissionResult(false, "", "abandon", "No mission_id provided"));
        return;
    }

    mission_system_->abandonMission(entity_id, mission_id);
    tcp_server_->sendToClient(client,
        protocol_.createMissionResult(true, mission_id, "abandon", "Mission abandoned"));
}

// ---------------------------------------------------------------------------
// MISSION PROGRESS handler
// ---------------------------------------------------------------------------

void GameSession::handleMissionProgress(const network::ClientConnection& client,
                                         const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    if (!mission_system_) {
        tcp_server_->sendToClient(client, protocol_.createError("Mission system not available"));
        return;
    }

    std::string mission_id = extractJsonString(data, "mission_id");
    std::string objective_type = extractJsonString(data, "objective_type");
    std::string target = extractJsonString(data, "target");
    int count = static_cast<int>(extractJsonFloat(data, "count", 1.0f));

    if (mission_id.empty() || objective_type.empty()) {
        tcp_server_->sendToClient(client,
            protocol_.createMissionResult(false, mission_id, "progress", "Missing mission_id or objective_type"));
        return;
    }

    mission_system_->recordProgress(entity_id, mission_id, objective_type, target, count);
    tcp_server_->sendToClient(client,
        protocol_.createMissionResult(true, mission_id, "progress", "Progress recorded"));
}

} // namespace atlas
