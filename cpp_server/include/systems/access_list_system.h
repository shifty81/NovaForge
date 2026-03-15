#ifndef NOVAFORGE_SYSTEMS_ACCESS_LIST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ACCESS_LIST_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Access Control List management system
 *
 * Manages ACL entries for player-owned structures.  Each entry grants or
 * blocks a specific member.  Entries with a positive TTL count down per
 * tick and are auto-removed on expiry.  A default_policy (Allow / Block)
 * governs members not explicitly listed.  max_entries caps the list size
 * (default 50).
 */
class AccessListSystem
    : public ecs::SingleComponentSystem<components::AccessListState> {
public:
    explicit AccessListSystem(ecs::World* world);
    ~AccessListSystem() override = default;

    std::string getName() const override { return "AccessListSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Entry management ---
    bool addEntry(const std::string& entity_id,
                  const std::string& entry_id,
                  const std::string& member_id,
                  components::AccessListState::Permission permission,
                  float ttl = 0.0f);
    bool removeEntry(const std::string& entity_id,
                     const std::string& entry_id);
    bool clearEntries(const std::string& entity_id);

    // --- Operations ---
    bool checkAccess(const std::string& entity_id,
                     const std::string& member_id);
    bool setPermission(const std::string& entity_id,
                       const std::string& entry_id,
                       components::AccessListState::Permission permission);

    // --- Configuration ---
    bool setOwnerId(const std::string& entity_id,
                    const std::string& owner_id);
    bool setStructureId(const std::string& entity_id,
                        const std::string& structure_id);
    bool setDefaultPolicy(const std::string& entity_id,
                          components::AccessListState::Permission policy);
    bool setMaxEntries(const std::string& entity_id, int max_entries);

    // --- Queries ---
    int   getEntryCount(const std::string& entity_id) const;
    bool  hasEntry(const std::string& entity_id,
                   const std::string& entry_id) const;
    bool  hasMember(const std::string& entity_id,
                    const std::string& member_id) const;
    std::string getOwnerId(const std::string& entity_id) const;
    std::string getStructureId(const std::string& entity_id) const;
    int   getTotalEntriesAdded(const std::string& entity_id) const;
    int   getTotalEntriesExpired(const std::string& entity_id) const;
    int   getTotalAccessChecks(const std::string& entity_id) const;
    float getEntryTtl(const std::string& entity_id,
                      const std::string& entry_id) const;
    bool  isDefaultAllow(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AccessListState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ACCESS_LIST_SYSTEM_H
