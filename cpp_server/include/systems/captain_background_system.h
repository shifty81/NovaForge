#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_BACKGROUND_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_BACKGROUND_SYSTEM_H

#include "ecs/system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Captain background/backstory management system
 *
 * Assigns and manages captain backgrounds (archetypes) that affect
 * personality modifiers, preferred fleet roles, skill bonuses, and
 * dialogue flavor. Supports deterministic generation from seed.
 */
class CaptainBackgroundSystem : public ecs::System {
public:
    explicit CaptainBackgroundSystem(ecs::World* world);
    ~CaptainBackgroundSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CaptainBackgroundSystem"; }

    // Commands
    bool assignBackground(const std::string& entity_id, const std::string& background_type);
    bool generateBackground(const std::string& entity_id, uint32_t seed);
    bool setOriginSystem(const std::string& entity_id, const std::string& system_name);
    bool setExperience(const std::string& entity_id, int years);

    // Query API
    std::string getBackground(const std::string& entity_id) const;
    std::string getPreferredRole(const std::string& entity_id) const;
    std::string getDialogueFlavor(const std::string& entity_id) const;
    float getSkillBonus(const std::string& entity_id) const;
    std::string getSkillCategory(const std::string& entity_id) const;
    float getAggressionModifier(const std::string& entity_id) const;
    float getLoyaltyModifier(const std::string& entity_id) const;
    int getExperience(const std::string& entity_id) const;
    std::string getOriginSystem(const std::string& entity_id) const;

private:
    void applyBackgroundDefaults(components::CaptainBackground* bg);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_BACKGROUND_SYSTEM_H
