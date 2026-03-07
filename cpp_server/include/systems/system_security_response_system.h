#ifndef NOVAFORGE_SYSTEMS_SYSTEM_SECURITY_RESPONSE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SYSTEM_SECURITY_RESPONSE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Security force response to criminal activity in a star system
 *
 * Offences reported raise the response level.  The system auto-transitions
 * through Idle → Alerted → Responding → Engaged as thresholds are crossed.
 * Response level decays passively when no offences are active.  In
 * HighSec the decay is slower and thresholds are lower, making response
 * faster.
 */
class SystemSecurityResponseSystem : public ecs::SingleComponentSystem<components::SystemSecurityResponse> {
public:
    explicit SystemSecurityResponseSystem(ecs::World* world);
    ~SystemSecurityResponseSystem() override = default;

    std::string getName() const override { return "SystemSecurityResponseSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& security_level);
    bool reportOffence(const std::string& entity_id, const std::string& offender_id,
                       const std::string& offence_type, float severity);
    bool dispatchResponse(const std::string& entity_id, int ship_count);
    bool clearOffences(const std::string& entity_id);

    int getOffenceCount(const std::string& entity_id) const;
    float getResponseLevel(const std::string& entity_id) const;
    std::string getState(const std::string& entity_id) const;
    int getResponseShipsDispatched(const std::string& entity_id) const;
    int getTotalResponses(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SystemSecurityResponse& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SYSTEM_SECURITY_RESPONSE_SYSTEM_H
