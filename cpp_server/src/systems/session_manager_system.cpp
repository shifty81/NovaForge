#include "systems/session_manager_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

SessionManagerSystem::SessionManagerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void SessionManagerSystem::updateComponent(ecs::Entity& /*entity*/,
                                           components::SessionState& comp,
                                           float delta_time) {
    if (comp.phase != components::SessionState::Phase::Active) return;

    comp.session_duration += delta_time;
    comp.idle_timer       += delta_time;

    if (comp.idle_timer >= comp.idle_timeout) {
        comp.phase = components::SessionState::Phase::Disconnecting;
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool SessionManagerSystem::createSession(const std::string& entity_id,
                                         const std::string& player_id,
                                         const std::string& character_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SessionState>();
    comp->player_id      = player_id;
    comp->character_name  = character_name;
    entity->addComponent(std::move(comp));
    return true;
}

bool SessionManagerSystem::authenticate(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->phase != components::SessionState::Phase::Disconnected) return false;
    // Instant authentication — transition directly to Loading
    comp->phase = components::SessionState::Phase::Loading;
    return true;
}

bool SessionManagerSystem::activateSession(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->phase != components::SessionState::Phase::Loading) return false;
    comp->phase = components::SessionState::Phase::Active;
    comp->login_count++;
    return true;
}

bool SessionManagerSystem::beginDisconnect(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->phase != components::SessionState::Phase::Active) return false;
    comp->phase = components::SessionState::Phase::Disconnecting;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

components::SessionState::Phase
SessionManagerSystem::getPhase(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->phase : components::SessionState::Phase::Disconnected;
}

float SessionManagerSystem::getSessionDuration(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->session_duration : 0.0f;
}

int SessionManagerSystem::getLoginCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->login_count : 0;
}

// ---------------------------------------------------------------------------
// Heartbeat / idle
// ---------------------------------------------------------------------------

bool SessionManagerSystem::heartbeat(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->idle_timer = 0.0f;
    comp->last_heartbeat = comp->session_duration;
    return true;
}

bool SessionManagerSystem::isIdle(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->idle_timer >= comp->idle_timeout;
}

// ---------------------------------------------------------------------------
// Spawn location
// ---------------------------------------------------------------------------

bool SessionManagerSystem::setSpawnLocation(const std::string& entity_id,
                                            const std::string& location) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || location.empty()) return false;
    comp->spawn_location = location;
    return true;
}

std::string SessionManagerSystem::getSpawnLocation(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->spawn_location : std::string();
}

} // namespace systems
} // namespace atlas
