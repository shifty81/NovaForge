#ifndef EVE_SYSTEMS_WAR_DECLARATION_SYSTEM_H
#define EVE_SYSTEMS_WAR_DECLARATION_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

class WarDeclarationSystem : public ecs::System {
public:
    explicit WarDeclarationSystem(ecs::World* world);
    ~WarDeclarationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "WarDeclarationSystem"; }

    std::string declareWar(const std::string& aggressor_id,
                           const std::string& defender_id,
                           double war_cost);

    bool activateWar(const std::string& war_entity_id);

    bool makeMutual(const std::string& war_entity_id,
                    const std::string& requester_id);

    bool surrender(const std::string& war_entity_id,
                   const std::string& requester_id);

    bool retractWar(const std::string& war_entity_id,
                    const std::string& requester_id);

    bool recordKill(const std::string& war_entity_id,
                    const std::string& killer_side,
                    double isk_value);

    std::string getWarStatus(const std::string& war_entity_id);

private:
    int war_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_WAR_DECLARATION_SYSTEM_H
