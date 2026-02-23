#ifndef EVE_SYSTEMS_RIG_SYSTEM_H
#define EVE_SYSTEMS_RIG_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class RigSystem : public ecs::System {
public:
    explicit RigSystem(ecs::World* world);
    ~RigSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "RigSystem"; }

    bool installModule(const std::string& entity_id, const std::string& module_entity_id);
    bool removeModule(const std::string& entity_id, const std::string& module_entity_id);
    int getRackSize(const std::string& entity_id) const;
    int getInstalledCount(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_RIG_SYSTEM_H
