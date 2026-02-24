#ifndef EVE_SYSTEMS_SOVEREIGNTY_SYSTEM_H
#define EVE_SYSTEMS_SOVEREIGNTY_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

class SovereigntySystem : public ecs::System {
public:
    explicit SovereigntySystem(ecs::World* world);
    ~SovereigntySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SovereigntySystem"; }

    bool claimSovereignty(const std::string& system_entity_id,
                          const std::string& owner_id,
                          const std::string& system_name);

    bool relinquishSovereignty(const std::string& system_entity_id,
                               const std::string& requester_id);

    bool contestSovereignty(const std::string& system_entity_id);

    bool updateIndices(const std::string& system_entity_id,
                       float military_delta,
                       float industrial_delta);

    float getControlLevel(const std::string& system_entity_id);

    std::string getOwner(const std::string& system_entity_id);

    bool upgradeInfrastructure(const std::string& system_entity_id,
                               const std::string& requester_id);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SOVEREIGNTY_SYSTEM_H
