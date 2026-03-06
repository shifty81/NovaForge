#include "systems/fps_terminal_hack_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FPSTerminalHackSystem::FPSTerminalHackSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FPSTerminalHackSystem::updateComponent(ecs::Entity& /*entity*/,
    components::FPSTerminalHack& hack, float delta_time) {
    if (!hack.active) return;

    if (hack.state == components::FPSTerminalHack::Hacking) {
        // Security level reduces hack speed
        float effective_speed = hack.hack_speed * (1.0f + hack.skill_bonus) /
                                static_cast<float>(hack.security_level);
        hack.hack_progress += effective_speed * delta_time;
        hack.time_remaining -= delta_time;

        if (hack.hack_progress >= 1.0f) {
            hack.hack_progress = 1.0f;
            hack.state = components::FPSTerminalHack::Success;
            hack.total_hacks_succeeded++;
        } else if (hack.time_remaining <= 0.0f) {
            hack.time_remaining = 0.0f;
            hack.hack_progress = 0.0f;
            hack.state = components::FPSTerminalHack::Failed;
            if (hack.triggers_alarm_on_fail) {
                hack.state = components::FPSTerminalHack::Alarmed;
                hack.total_alarms_triggered++;
            }
        }
    }
}

bool FPSTerminalHackSystem::startHack(const std::string& entity_id, float skill_bonus) {
    auto* hack = getComponentFor(entity_id);
    if (!hack) return false;
    if (hack->state == components::FPSTerminalHack::Hacking) return false;
    if (hack->state == components::FPSTerminalHack::Success) return false;
    if (hack->attempts_used >= hack->max_attempts) return false;

    hack->state = components::FPSTerminalHack::Hacking;
    hack->hack_progress = 0.0f;
    hack->time_remaining = hack->time_limit / static_cast<float>(hack->security_level);
    hack->skill_bonus = std::max(0.0f, std::min(1.0f, skill_bonus));
    hack->attempts_used++;
    hack->total_hacks_attempted++;
    return true;
}

bool FPSTerminalHackSystem::cancelHack(const std::string& entity_id) {
    auto* hack = getComponentFor(entity_id);
    if (!hack) return false;
    if (hack->state != components::FPSTerminalHack::Hacking) return false;
    hack->state = components::FPSTerminalHack::Locked;
    hack->hack_progress = 0.0f;
    return true;
}

int FPSTerminalHackSystem::getState(const std::string& entity_id) const {
    auto* hack = getComponentFor(entity_id);
    return hack ? static_cast<int>(hack->state) : 0;
}

float FPSTerminalHackSystem::getProgress(const std::string& entity_id) const {
    auto* hack = getComponentFor(entity_id);
    return hack ? hack->hack_progress : 0.0f;
}

float FPSTerminalHackSystem::getTimeRemaining(const std::string& entity_id) const {
    auto* hack = getComponentFor(entity_id);
    return hack ? hack->time_remaining : 0.0f;
}

int FPSTerminalHackSystem::getAttemptsRemaining(const std::string& entity_id) const {
    auto* hack = getComponentFor(entity_id);
    return hack ? (hack->max_attempts - hack->attempts_used) : 0;
}

bool FPSTerminalHackSystem::isAlarmed(const std::string& entity_id) const {
    auto* hack = getComponentFor(entity_id);
    return hack ? hack->state == components::FPSTerminalHack::Alarmed : false;
}

bool FPSTerminalHackSystem::setSecurityLevel(const std::string& entity_id, int level) {
    if (level < 1 || level > 5) return false;
    auto* hack = getComponentFor(entity_id);
    if (!hack) return false;
    hack->security_level = level;
    return true;
}

int FPSTerminalHackSystem::getTotalHacksAttempted(const std::string& entity_id) const {
    auto* hack = getComponentFor(entity_id);
    return hack ? hack->total_hacks_attempted : 0;
}

int FPSTerminalHackSystem::getTotalHacksSucceeded(const std::string& entity_id) const {
    auto* hack = getComponentFor(entity_id);
    return hack ? hack->total_hacks_succeeded : 0;
}

int FPSTerminalHackSystem::getTotalAlarmsTriggered(const std::string& entity_id) const {
    auto* hack = getComponentFor(entity_id);
    return hack ? hack->total_alarms_triggered : 0;
}

bool FPSTerminalHackSystem::resetTerminal(const std::string& entity_id) {
    auto* hack = getComponentFor(entity_id);
    if (!hack) return false;
    hack->state = components::FPSTerminalHack::Locked;
    hack->hack_progress = 0.0f;
    hack->time_remaining = hack->time_limit;
    hack->attempts_used = 0;
    return true;
}

} // namespace systems
} // namespace atlas
