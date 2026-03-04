#ifndef NOVAFORGE_ECS_SINGLE_COMPONENT_SYSTEM_H
#define NOVAFORGE_ECS_SINGLE_COMPONENT_SYSTEM_H

#include "system.h"
#include "world.h"
#include "entity.h"
#include <string>
#include <vector>

namespace atlas {
namespace ecs {

/**
 * @brief Template base for systems that operate on a single component type.
 *
 * Provides the standard entity-query + null-check loop in update(), delegating
 * per-entity logic to the pure-virtual updateComponent().  Also provides a
 * helper to look up a component by entity ID (getComponentFor).
 *
 * Derived classes must implement:
 *   - void updateComponent(Entity& entity, C& component, float delta_time)
 *   - std::string getName() const override
 *
 * Example usage:
 * @code
 *   class CapacitorSystem : public ecs::SingleComponentSystem<components::Capacitor> {
 *   public:
 *       using SingleComponentSystem::SingleComponentSystem;
 *       std::string getName() const override { return "CapacitorSystem"; }
 *   protected:
 *       void updateComponent(Entity& entity, components::Capacitor& cap, float dt) override {
 *           if (cap.capacitor < cap.capacitor_max) {
 *               cap.capacitor = std::min(cap.capacitor + cap.recharge_rate * dt, cap.capacitor_max);
 *           }
 *       }
 *   };
 * @endcode
 */
template<typename C>
class SingleComponentSystem : public System {
public:
    explicit SingleComponentSystem(World* world) : System(world) {}
    ~SingleComponentSystem() override = default;

    void update(float delta_time) override {
        auto entities = world_->getEntities<C>();
        for (auto* entity : entities) {
            auto* comp = entity->template getComponent<C>();
            if (!comp) continue;
            updateComponent(*entity, *comp, delta_time);
        }
    }

protected:
    /**
     * @brief Per-entity update logic — override in derived classes.
     * @param entity  The entity owning the component
     * @param component  Reference to the component (guaranteed non-null)
     * @param delta_time  Time elapsed since last tick (seconds)
     */
    virtual void updateComponent(Entity& entity, C& component, float delta_time) = 0;

    /**
     * @brief Look up a component by entity ID.
     * @return Pointer to the component, or nullptr if entity or component not found.
     */
    C* getComponentFor(const std::string& entity_id) {
        auto* entity = world_->getEntity(entity_id);
        if (!entity) return nullptr;
        return entity->template getComponent<C>();
    }

    const C* getComponentFor(const std::string& entity_id) const {
        auto* entity = world_->getEntity(entity_id);
        if (!entity) return nullptr;
        return entity->template getComponent<C>();
    }
};

} // namespace ecs
} // namespace atlas

#endif // NOVAFORGE_ECS_SINGLE_COMPONENT_SYSTEM_H
