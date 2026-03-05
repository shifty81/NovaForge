#ifndef NOVAFORGE_SYSTEMS_CHAT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CHAT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class ChatSystem : public ecs::SingleComponentSystem<components::ChatChannel> {
public:
    explicit ChatSystem(ecs::World* world);
    ~ChatSystem() override = default;

    std::string getName() const override { return "ChatSystem"; }

    // Channel management
    bool joinChannel(const std::string& channel_entity_id,
                     const std::string& player_id,
                     const std::string& player_name);

    bool leaveChannel(const std::string& channel_entity_id,
                      const std::string& player_id);

    // Messaging
    bool sendMessage(const std::string& channel_entity_id,
                     const std::string& sender_id,
                     const std::string& sender_name,
                     const std::string& content);

    // Moderation
    bool mutePlayer(const std::string& channel_entity_id,
                    const std::string& moderator_id,
                    const std::string& target_id);

    bool unmutePlayer(const std::string& channel_entity_id,
                      const std::string& moderator_id,
                      const std::string& target_id);

    bool setMotd(const std::string& channel_entity_id,
                 const std::string& setter_id,
                 const std::string& motd);

    // Queries
    int getMessageCount(const std::string& channel_entity_id);
    int getMemberCount(const std::string& channel_entity_id);
    bool isMember(const std::string& channel_entity_id,
                  const std::string& player_id);

protected:
    void updateComponent(ecs::Entity& entity, components::ChatChannel& channel, float delta_time) override;

private:
    int message_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CHAT_SYSTEM_H
