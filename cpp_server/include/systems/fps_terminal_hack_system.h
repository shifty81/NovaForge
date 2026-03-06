#ifndef NOVAFORGE_SYSTEMS_FPS_TERMINAL_HACK_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_TERMINAL_HACK_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Terminal hacking mini-game with difficulty, time limits, and failure
 *
 * Manages hack attempts on security terminals, doors, and data cores.
 * Difficulty scales with security level; failure can trigger alarms.
 */
class FPSTerminalHackSystem : public ecs::SingleComponentSystem<components::FPSTerminalHack> {
public:
    explicit FPSTerminalHackSystem(ecs::World* world);
    ~FPSTerminalHackSystem() override = default;

    std::string getName() const override { return "FPSTerminalHackSystem"; }

    bool startHack(const std::string& entity_id, float skill_bonus);
    bool cancelHack(const std::string& entity_id);
    int getState(const std::string& entity_id) const;
    float getProgress(const std::string& entity_id) const;
    float getTimeRemaining(const std::string& entity_id) const;
    int getAttemptsRemaining(const std::string& entity_id) const;
    bool isAlarmed(const std::string& entity_id) const;
    bool setSecurityLevel(const std::string& entity_id, int level);
    int getTotalHacksAttempted(const std::string& entity_id) const;
    int getTotalHacksSucceeded(const std::string& entity_id) const;
    int getTotalAlarmsTriggered(const std::string& entity_id) const;
    bool resetTerminal(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::FPSTerminalHack& hack, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_TERMINAL_HACK_SYSTEM_H
