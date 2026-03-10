#ifndef NOVAFORGE_SYSTEMS_ALLIANCE_MANAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ALLIANCE_MANAGEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Alliance creation and membership management
 *
 * Models EVE Online alliance mechanics — a group of corporations under
 * one banner.  One corporation is designated the executor (leader).
 * Members can be added/removed, the executor can be changed, and the
 * alliance can be disbanded.  Member count is capped at max_members
 * (default 50).
 */
class AllianceManagementSystem
    : public ecs::SingleComponentSystem<components::AllianceState> {
public:
    explicit AllianceManagementSystem(ecs::World* world);
    ~AllianceManagementSystem() override = default;

    std::string getName() const override { return "AllianceManagementSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& alliance_id,
                    const std::string& alliance_name,
                    const std::string& ticker,
                    const std::string& executor_corp_id);

    // --- Membership ---
    bool addMember(const std::string& entity_id,
                   const std::string& corp_id,
                   const std::string& corp_name);
    bool removeMember(const std::string& entity_id,
                      const std::string& corp_id);
    bool setExecutor(const std::string& entity_id,
                     const std::string& corp_id);
    bool disbandAlliance(const std::string& entity_id);

    // --- Queries ---
    int         getMemberCount(const std::string& entity_id) const;
    std::string getExecutorCorpId(const std::string& entity_id) const;
    std::string getAllianceName(const std::string& entity_id) const;
    std::string getTicker(const std::string& entity_id) const;
    bool        isActive(const std::string& entity_id) const;
    bool        hasMember(const std::string& entity_id,
                          const std::string& corp_id) const;
    int         getTotalMembersJoined(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AllianceState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ALLIANCE_MANAGEMENT_SYSTEM_H
