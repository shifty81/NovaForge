#ifndef EVE_SYSTEMS_ALLIANCE_SYSTEM_H
#define EVE_SYSTEMS_ALLIANCE_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

class AllianceSystem : public ecs::System {
public:
    explicit AllianceSystem(ecs::World* world);
    ~AllianceSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "AllianceSystem"; }

    bool createAlliance(const std::string& corp_entity_id,
                        const std::string& alliance_name,
                        const std::string& ticker);

    bool joinAlliance(const std::string& corp_entity_id,
                      const std::string& alliance_entity_id);

    bool leaveAlliance(const std::string& corp_entity_id,
                       const std::string& alliance_entity_id);

    bool setExecutor(const std::string& alliance_entity_id,
                     const std::string& new_executor_corp_id,
                     const std::string& requester_corp_id);

    int getMemberCorpCount(const std::string& alliance_entity_id);

    bool isCorpInAlliance(const std::string& corp_entity_id,
                          const std::string& alliance_entity_id);

    bool disbandAlliance(const std::string& alliance_entity_id,
                         const std::string& requester_corp_id);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_ALLIANCE_SYSTEM_H
