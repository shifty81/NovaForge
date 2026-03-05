#ifndef NOVAFORGE_SYSTEMS_COMMANDER_DISAGREEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COMMANDER_DISAGREEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Wing commander disagreement management system
 *
 * Manages disagreements between wing commanders. Disagreements arise
 * from personality conflicts, escalate over time if unresolved,
 * and impact fleet morale. Resolution methods: Vote, AuthorityOverride,
 * Compromise, or Escalation.
 */
class CommanderDisagreementSystem : public ecs::SingleComponentSystem<components::CommanderDisagreement> {
public:
    explicit CommanderDisagreementSystem(ecs::World* world);
    ~CommanderDisagreementSystem() override = default;

    std::string getName() const override { return "CommanderDisagreementSystem"; }

    // Commands
    bool raiseDisagreement(const std::string& fleet_id,
                           const std::string& commander_a,
                           const std::string& commander_b,
                           const std::string& topic);
    bool resolveDisagreement(const std::string& fleet_id, int index,
                             const std::string& resolution);
    bool dismissDisagreement(const std::string& fleet_id, int index);

    // Query API
    int getActiveCount(const std::string& fleet_id) const;
    float getFleetTension(const std::string& fleet_id) const;
    int getTotalDisagreements(const std::string& fleet_id) const;
    int getTotalResolved(const std::string& fleet_id) const;
    std::string getSeverity(const std::string& fleet_id, int index) const;
    std::string getResolution(const std::string& fleet_id, int index) const;
    float getMoraleImpact(const std::string& fleet_id, int index) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CommanderDisagreement& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COMMANDER_DISAGREEMENT_SYSTEM_H
