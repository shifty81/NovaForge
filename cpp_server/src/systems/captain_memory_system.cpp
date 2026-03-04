#include "systems/captain_memory_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"

namespace atlas {
namespace systems {

CaptainMemorySystem::CaptainMemorySystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void CaptainMemorySystem::updateComponent(ecs::Entity& /*entity*/, components::CaptainMemory& /*memory*/, float /*delta_time*/) {
    // Memories are event-driven — nothing to tick.
}

void CaptainMemorySystem::recordMemory(const std::string& entity_id,
                                        const std::string& event_type,
                                        const std::string& context,
                                        float timestamp,
                                        float weight) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* mem = entity->getComponent<components::CaptainMemory>();
    if (!mem) {
        entity->addComponent(std::make_unique<components::CaptainMemory>());
        mem = entity->getComponent<components::CaptainMemory>();
    }

    mem->addMemory(event_type, context, timestamp, weight);
}

int CaptainMemorySystem::countMemories(const std::string& entity_id,
                                        const std::string& event_type) const {
    auto* mem = getComponentFor(entity_id);
    if (!mem) return 0;

    return mem->countByType(event_type);
}

float CaptainMemorySystem::averageEmotionalWeight(const std::string& entity_id) const {
    auto* mem = getComponentFor(entity_id);
    if (!mem) return 0.0f;

    return mem->averageWeight();
}

int CaptainMemorySystem::totalMemories(const std::string& entity_id) const {
    auto* mem = getComponentFor(entity_id);
    if (!mem) return 0;

    return static_cast<int>(mem->memories.size());
}

std::string CaptainMemorySystem::mostRecentEvent(const std::string& entity_id) const {
    auto* mem = getComponentFor(entity_id);
    if (!mem || mem->memories.empty()) return "";

    return mem->memories.back().event_type;
}

} // namespace systems
} // namespace atlas
