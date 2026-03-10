#ifndef NOVAFORGE_SYSTEMS_SESSION_MANAGER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SESSION_MANAGER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Player session lifecycle management system
 *
 * Manages the full session lifecycle: login, authentication, loading,
 * active gameplay, and disconnection.  While a session is Active the
 * system accumulates session_duration and idle_timer each tick.  If
 * idle_timer reaches idle_timeout the session is automatically moved
 * to the Disconnecting phase.
 */
class SessionManagerSystem
    : public ecs::SingleComponentSystem<components::SessionState> {
public:
    explicit SessionManagerSystem(ecs::World* world);
    ~SessionManagerSystem() override = default;

    std::string getName() const override { return "SessionManagerSystem"; }

    // --- Lifecycle ---
    bool createSession(const std::string& entity_id,
                       const std::string& player_id,
                       const std::string& character_name);
    bool authenticate(const std::string& entity_id);
    bool activateSession(const std::string& entity_id);
    bool beginDisconnect(const std::string& entity_id);

    // --- Queries ---
    components::SessionState::Phase getPhase(const std::string& entity_id) const;
    float getSessionDuration(const std::string& entity_id) const;
    int   getLoginCount(const std::string& entity_id) const;

    // --- Heartbeat / idle ---
    bool heartbeat(const std::string& entity_id);
    bool isIdle(const std::string& entity_id) const;

    // --- Spawn location ---
    bool setSpawnLocation(const std::string& entity_id,
                          const std::string& location);
    std::string getSpawnLocation(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SessionState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SESSION_MANAGER_SYSTEM_H
