#ifndef NOVAFORGE_SYSTEMS_SNAPSHOT_REPLICATION_SYSTEM2_H
#define NOVAFORGE_SYSTEMS_SNAPSHOT_REPLICATION_SYSTEM2_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Server-side state snapshot management for network replication
 *
 * Captures entity states at regular intervals, maintains a history buffer,
 * computes delta snapshots between frames, tracks per-client acknowledgment,
 * and manages snapshot lifecycle for bandwidth-efficient state synchronization.
 */
class SnapshotReplicationSystem2 : public ecs::System {
public:
    explicit SnapshotReplicationSystem2(ecs::World* world);
    ~SnapshotReplicationSystem2() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SnapshotReplicationSystem2"; }

    bool initialize(const std::string& entity_id, const std::string& server_id);
    bool captureSnapshot(const std::string& entity_id);
    bool addEntityToSnapshot(const std::string& entity_id, const std::string& target_entity_id,
                             float x, float y, float z, float health, float shield, float velocity);
    bool registerClient(const std::string& entity_id, const std::string& client_id);
    bool unregisterClient(const std::string& entity_id, const std::string& client_id);
    bool acknowledgeFrame(const std::string& entity_id, const std::string& client_id,
                          uint32_t frame_number);
    int getDeltaEntityCount(const std::string& entity_id, const std::string& client_id) const;
    uint32_t getCurrentFrame(const std::string& entity_id) const;
    int getHistorySize(const std::string& entity_id) const;
    int getClientCount(const std::string& entity_id) const;
    int getTotalSnapshotsSent(const std::string& entity_id) const;
    uint32_t getClientLastAck(const std::string& entity_id, const std::string& client_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SNAPSHOT_REPLICATION_SYSTEM2_H
