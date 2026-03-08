#ifndef NOVAFORGE_SYSTEMS_CHAT_ROUTER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CHAT_ROUTER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * ChatRouterSystem - server-side chat stream routing with rate limiting
 * and per-stream sequence tracking.
 *
 * Reads/Writes ChatRouterState component.
 *
 * Design:
 *   - Routes messages to global, local, party, guild, and system streams.
 *   - Enforces per-entity rate limiting (max_messages_per_window in rate_window_seconds).
 *   - Rejects messages that exceed max_message_length or are empty.
 *   - Each stream maintains an independent sequence counter.
 */
class ChatRouterSystem : public ecs::SingleComponentSystem<components::ChatRouterState> {
public:
    explicit ChatRouterSystem(ecs::World* world);
    ~ChatRouterSystem() override = default;

    std::string getName() const override { return "ChatRouterSystem"; }

    /// Route a message to the given stream. Returns false if rate limited, too long, or empty.
    bool routeMessage(const std::string& entity_id, const std::string& stream_type, const std::string& text);

    /// Get the current sequence number for a stream on this entity.
    uint64_t getNextSeq(const std::string& entity_id, const std::string& stream_type) const;

    /// Check if entity is currently rate limited.
    bool isRateLimited(const std::string& entity_id) const;

    /// Query total routed / rejected message counts.
    int getTotalRouted(const std::string& entity_id) const;
    int getTotalRejected(const std::string& entity_id) const;

    /// Reset the rate-limit window for an entity.
    void resetRateWindow(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::ChatRouterState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CHAT_ROUTER_SYSTEM_H
