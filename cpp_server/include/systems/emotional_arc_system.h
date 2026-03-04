#ifndef NOVAFORGE_SYSTEMS_EMOTIONAL_ARC_SYSTEM_H
#define NOVAFORGE_SYSTEMS_EMOTIONAL_ARC_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class EmotionalArcSystem : public ecs::SingleComponentSystem<components::EmotionalState> {
public:
    explicit EmotionalArcSystem(ecs::World* world);
    ~EmotionalArcSystem() override = default;

    std::string getName() const override { return "EmotionalArcSystem"; }

    void onCombatVictory(const std::string& entity_id);
    void onCombatDefeat(const std::string& entity_id);
    void onRest(const std::string& entity_id);
    void onPlayerTrust(const std::string& entity_id);
    void onPlayerBetray(const std::string& entity_id);

    float getConfidence(const std::string& entity_id) const;
    float getTrust(const std::string& entity_id) const;
    float getFatigue(const std::string& entity_id) const;
    float getHope(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::EmotionalState& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_EMOTIONAL_ARC_SYSTEM_H
