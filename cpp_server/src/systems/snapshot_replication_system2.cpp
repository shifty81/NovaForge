#include "systems/snapshot_replication_system2.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

SnapshotReplicationSystem2::SnapshotReplicationSystem2(ecs::World* world) : System(world) {}

void SnapshotReplicationSystem2::update(float delta_time) {
    auto entities = world_->getEntities<components::SnapshotReplication>();
    for (auto* entity : entities) {
        auto* sr = entity->getComponent<components::SnapshotReplication>();
        if (!sr || !sr->active) continue;

        sr->time_accumulator += delta_time;
        if (sr->time_accumulator >= sr->snapshot_interval) {
            sr->time_accumulator -= sr->snapshot_interval;
            // Auto-capture: advance frame
            sr->current_frame++;
            components::SnapshotReplication::SnapshotFrame frame;
            frame.frame_number = sr->current_frame;
            frame.timestamp = delta_time; // placeholder
            sr->history.push_back(frame);
            sr->total_snapshots_sent++;

            // Trim history
            while (static_cast<int>(sr->history.size()) > sr->max_history) {
                sr->history.erase(sr->history.begin());
            }
        }
    }
}

bool SnapshotReplicationSystem2::initialize(const std::string& entity_id,
    const std::string& server_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SnapshotReplication>();
    comp->server_id = server_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool SnapshotReplicationSystem2::captureSnapshot(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    if (!sr) return false;

    sr->current_frame++;
    components::SnapshotReplication::SnapshotFrame frame;
    frame.frame_number = sr->current_frame;
    sr->history.push_back(frame);
    sr->total_snapshots_sent++;

    while (static_cast<int>(sr->history.size()) > sr->max_history) {
        sr->history.erase(sr->history.begin());
    }
    return true;
}

bool SnapshotReplicationSystem2::addEntityToSnapshot(const std::string& entity_id,
    const std::string& target_entity_id,
    float x, float y, float z, float health, float shield, float velocity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    if (!sr || sr->history.empty()) return false;

    auto& current = sr->history.back();
    // Check duplicate
    for (const auto& es : current.entities) {
        if (es.entity_id == target_entity_id) return false;
    }

    components::SnapshotReplication::EntitySnapshot snap;
    snap.entity_id = target_entity_id;
    snap.x = x; snap.y = y; snap.z = z;
    snap.health = health; snap.shield = shield; snap.velocity = velocity;
    snap.frame_number = current.frame_number;
    snap.dirty = true;
    current.entities.push_back(snap);
    return true;
}

bool SnapshotReplicationSystem2::registerClient(const std::string& entity_id,
    const std::string& client_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    if (!sr) return false;
    if (static_cast<int>(sr->client_acks.size()) >= sr->max_clients) return false;

    // Check duplicate
    for (const auto& ca : sr->client_acks) {
        if (ca.client_id == client_id) return false;
    }

    components::SnapshotReplication::ClientAck ack;
    ack.client_id = client_id;
    ack.last_acked_frame = 0;
    sr->client_acks.push_back(ack);
    return true;
}

bool SnapshotReplicationSystem2::unregisterClient(const std::string& entity_id,
    const std::string& client_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    if (!sr) return false;

    auto it = std::remove_if(sr->client_acks.begin(), sr->client_acks.end(),
        [&](const components::SnapshotReplication::ClientAck& ca) {
            return ca.client_id == client_id;
        });
    if (it == sr->client_acks.end()) return false;
    sr->client_acks.erase(it, sr->client_acks.end());
    return true;
}

bool SnapshotReplicationSystem2::acknowledgeFrame(const std::string& entity_id,
    const std::string& client_id, uint32_t frame_number) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    if (!sr) return false;

    for (auto& ca : sr->client_acks) {
        if (ca.client_id == client_id) {
            if (frame_number > ca.last_acked_frame) {
                ca.last_acked_frame = frame_number;
            }
            return true;
        }
    }
    return false;
}

int SnapshotReplicationSystem2::getDeltaEntityCount(const std::string& entity_id,
    const std::string& client_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    if (!sr || sr->history.empty()) return 0;

    uint32_t last_acked = 0;
    bool found = false;
    for (const auto& ca : sr->client_acks) {
        if (ca.client_id == client_id) {
            last_acked = ca.last_acked_frame;
            found = true;
            break;
        }
    }
    if (!found) return 0;

    // Count entities that changed since last_acked
    const auto& latest = sr->history.back();
    if (last_acked == 0) return static_cast<int>(latest.entities.size()); // full snapshot needed

    // Find the acked frame
    const components::SnapshotReplication::SnapshotFrame* acked_frame = nullptr;
    for (const auto& f : sr->history) {
        if (f.frame_number == last_acked) { acked_frame = &f; break; }
    }
    if (!acked_frame) return static_cast<int>(latest.entities.size()); // full snapshot needed

    // Count entities in latest that differ from acked
    int delta = 0;
    for (const auto& es : latest.entities) {
        bool changed = true;
        for (const auto& oes : acked_frame->entities) {
            if (oes.entity_id == es.entity_id) {
                if (oes.x == es.x && oes.y == es.y && oes.z == es.z &&
                    oes.health == es.health && oes.shield == es.shield &&
                    oes.velocity == es.velocity) {
                    changed = false;
                }
                break;
            }
        }
        if (changed) delta++;
    }
    return delta;
}

uint32_t SnapshotReplicationSystem2::getCurrentFrame(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    return sr ? sr->current_frame : 0;
}

int SnapshotReplicationSystem2::getHistorySize(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    return sr ? static_cast<int>(sr->history.size()) : 0;
}

int SnapshotReplicationSystem2::getClientCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    return sr ? static_cast<int>(sr->client_acks.size()) : 0;
}

int SnapshotReplicationSystem2::getTotalSnapshotsSent(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    return sr ? sr->total_snapshots_sent : 0;
}

uint32_t SnapshotReplicationSystem2::getClientLastAck(const std::string& entity_id,
    const std::string& client_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    if (!sr) return 0;
    for (const auto& ca : sr->client_acks) {
        if (ca.client_id == client_id) return ca.last_acked_frame;
    }
    return 0;
}

} // namespace systems
} // namespace atlas
