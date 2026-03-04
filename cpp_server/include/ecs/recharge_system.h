#ifndef NOVAFORGE_ECS_RECHARGE_SYSTEM_H
#define NOVAFORGE_ECS_RECHARGE_SYSTEM_H

#include "single_component_system.h"
#include <string>

namespace atlas {
namespace ecs {

/**
 * @brief Template base for systems that recharge a numeric resource over time.
 *
 * Extends SingleComponentSystem<C> for the common pattern:
 *   current = min(current + rate * dt, max)
 *
 * Derived classes must implement:
 *   - float& current(C& component)  — mutable reference to the resource value
 *   - float max(const C& component) — current maximum (cap)
 *   - float rate(const C& component) — recharge rate per second
 *   - std::string getName() const override
 *
 * Optionally override updateComponent() for additional per-tick logic.
 *
 * Example usage:
 * @code
 *   class FuelSystem : public ecs::RechargeSystem<components::FuelTank> {
 *   public:
 *       using RechargeSystem::RechargeSystem;
 *       std::string getName() const override { return "FuelSystem"; }
 *   protected:
 *       float& current(components::FuelTank& c) override { return c.fuel; }
 *       float max(const components::FuelTank& c) const override { return c.fuel_max; }
 *       float rate(const components::FuelTank& c) const override { return c.regen_rate; }
 *   };
 * @endcode
 */
template<typename C>
class RechargeSystem : public SingleComponentSystem<C> {
public:
    explicit RechargeSystem(World* world) : SingleComponentSystem<C>(world) {}
    ~RechargeSystem() override = default;

protected:
    /**
     * @brief Per-entity recharge tick.  May be overridden for extra logic.
     */
    void updateComponent(Entity& entity, C& component, float delta_time) override {
        float& cur = current(component);
        float cap = max(component);
        if (cur < cap) {
            cur = std::min(cur + rate(component) * delta_time, cap);
        }
        onAfterRecharge(entity, component, delta_time);
    }

    /** @brief Mutable reference to the resource being recharged. */
    virtual float& current(C& component) = 0;

    /** @brief Upper bound for the resource. */
    virtual float max(const C& component) const = 0;

    /** @brief Per-second recharge rate. */
    virtual float rate(const C& component) const = 0;

    /**
     * @brief Optional hook called after recharge is applied each tick.
     *        Default does nothing.
     */
    virtual void onAfterRecharge(Entity& /*entity*/, C& /*component*/, float /*delta_time*/) {}
};

} // namespace ecs
} // namespace atlas

#endif // NOVAFORGE_ECS_RECHARGE_SYSTEM_H
