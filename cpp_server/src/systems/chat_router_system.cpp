#include "systems/chat_router_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

ChatRouterSystem::ChatRouterSystem(ecs::World* world) : SingleComponentSystem(world) {}

void ChatRouterSystem::updateComponent(ecs::Entity& /*entity*/,
    components::ChatRouterState& state, float delta_time) {
    if (!state.active) return;

    state.rate_window_timer -= delta_time;
    if (state.rate_window_timer <= 0.0f) {
        state.rate_window_timer = 0.0f;
        state.messages_in_window = 0;
    }
}

bool ChatRouterSystem::routeMessage(const std::string& entity_id,
    const std::string& stream_type, const std::string& text) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    // Reject empty messages
    if (text.empty()) {
        state->total_messages_rejected++;
        return false;
    }

    // Reject messages over max length
    if (static_cast<int>(text.size()) > state->max_message_length) {
        state->total_messages_rejected++;
        return false;
    }

    // Check rate limit
    if (state->messages_in_window >= state->max_messages_per_window) {
        state->rate_limit_violations++;
        state->total_messages_rejected++;
        return false;
    }

    // Route to the appropriate stream and increment its sequence counter
    uint64_t* seq = nullptr;
    if (stream_type == "global") seq = &state->next_global_seq;
    else if (stream_type == "local") seq = &state->next_local_seq;
    else if (stream_type == "party") seq = &state->next_party_seq;
    else if (stream_type == "guild") seq = &state->next_guild_seq;
    else if (stream_type == "system") seq = &state->next_system_seq;
    else {
        state->total_messages_rejected++;
        return false;
    }

    (*seq)++;
    state->total_messages_routed++;
    state->messages_in_window++;
    if (state->rate_window_timer <= 0.0f) {
        state->rate_window_timer = state->rate_window_seconds;
    }

    return true;
}

uint64_t ChatRouterSystem::getNextSeq(const std::string& entity_id,
    const std::string& stream_type) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return 0;

    if (stream_type == "global") return state->next_global_seq;
    if (stream_type == "local") return state->next_local_seq;
    if (stream_type == "party") return state->next_party_seq;
    if (stream_type == "guild") return state->next_guild_seq;
    if (stream_type == "system") return state->next_system_seq;
    return 0;
}

bool ChatRouterSystem::isRateLimited(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return false;
    return state->messages_in_window >= state->max_messages_per_window;
}

int ChatRouterSystem::getTotalRouted(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->total_messages_routed : 0;
}

int ChatRouterSystem::getTotalRejected(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->total_messages_rejected : 0;
}

void ChatRouterSystem::resetRateWindow(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return;
    state->rate_window_timer = 0.0f;
    state->messages_in_window = 0;
}

} // namespace systems
} // namespace atlas
