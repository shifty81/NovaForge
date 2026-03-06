#ifndef NOVAFORGE_SYSTEMS_PLAYER_SESSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_SESSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Player session management for vertical slice game loop
 *
 * Manages player connections, character selection, session state transitions,
 * heartbeat monitoring, and reconnection handling.  Enables the connect →
 * select character → play → disconnect lifecycle.
 */
class PlayerSessionSystem : public ecs::SingleComponentSystem<components::PlayerSession> {
public:
    explicit PlayerSessionSystem(ecs::World* world);
    ~PlayerSessionSystem() override = default;

    std::string getName() const override { return "PlayerSessionSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id,
                    const std::string& display_name);

    // Connection lifecycle
    bool connect(const std::string& entity_id);
    bool disconnect(const std::string& entity_id);
    bool reconnect(const std::string& entity_id);

    // Character management
    bool addCharacterSlot(const std::string& entity_id, const std::string& character_id,
                          const std::string& character_name, const std::string& ship_type,
                          const std::string& location);
    bool removeCharacterSlot(const std::string& entity_id, const std::string& character_id);
    bool selectCharacter(const std::string& entity_id, const std::string& character_id);
    bool enterGame(const std::string& entity_id);

    // Heartbeat
    bool heartbeat(const std::string& entity_id, float current_time);

    // Queries
    int getCharacterSlotCount(const std::string& entity_id) const;
    int getSessionState(const std::string& entity_id) const;
    float getSessionDuration(const std::string& entity_id) const;
    float getTotalPlayTime(const std::string& entity_id) const;
    int getLoginCount(const std::string& entity_id) const;
    int getDisconnectCount(const std::string& entity_id) const;
    int getReconnectCount(const std::string& entity_id) const;
    std::string getSelectedCharacter(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::PlayerSession& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_SESSION_SYSTEM_H
