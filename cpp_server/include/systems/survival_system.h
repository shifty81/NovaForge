#ifndef EVE_SYSTEMS_SURVIVAL_SYSTEM_H
#define EVE_SYSTEMS_SURVIVAL_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <tuple>

namespace atlas {
namespace systems {

class SurvivalSystem : public ecs::System {
public:
    explicit SurvivalSystem(ecs::World* world);
    ~SurvivalSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SurvivalSystem"; }

    float refillOxygen(const std::string& entity_id, float amount);
    float feed(const std::string& entity_id, float amount);
    float rest(const std::string& entity_id, float amount);
    bool isAlive(const std::string& entity_id) const;
    std::tuple<float, float, float> getNeeds(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SURVIVAL_SYSTEM_H
