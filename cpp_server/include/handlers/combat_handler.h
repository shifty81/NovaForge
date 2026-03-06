#pragma once

#include "handlers/message_handler.h"

namespace atlas {
namespace ecs { class World; }
namespace network { class TCPServer; class ProtocolHandler; }
namespace systems {
    class TargetingSystem;
    class CombatSystem;
}
namespace handlers {

/**
 * @brief Handles combat-related messages: target lock/unlock, module activate/deactivate.
 */
class CombatHandler : public IMessageHandler {
public:
    CombatHandler(ecs::World* world, network::TCPServer* tcp_server,
                  network::ProtocolHandler* protocol, EntityLookupFn entity_lookup);

    bool canHandle(network::MessageType type) const override;
    void handle(network::MessageType type,
                const network::ClientConnection& client,
                const std::string& data) override;

    void setTargetingSystem(systems::TargetingSystem* ts) { targeting_system_ = ts; }
    void setCombatSystem(systems::CombatSystem* cs) { combat_system_ = cs; }

private:
    void handleTargetLock(const network::ClientConnection& client, const std::string& data);
    void handleTargetUnlock(const network::ClientConnection& client, const std::string& data);
    void handleModuleActivate(const network::ClientConnection& client, const std::string& data);
    void handleModuleDeactivate(const network::ClientConnection& client, const std::string& data);

    ecs::World* world_;
    network::TCPServer* tcp_server_;
    network::ProtocolHandler* protocol_;
    EntityLookupFn entity_lookup_;
    systems::TargetingSystem* targeting_system_ = nullptr;
    systems::CombatSystem* combat_system_ = nullptr;
};

} // namespace handlers
} // namespace atlas
