#pragma once
/**
 * @file System.h
 * @brief Base interface for ECS systems and a registry to tick them.
 *
 * Every gameplay system (AI, physics, economy, etc.) can implement ISystem
 * so the engine ticks them in a deterministic, priority-ordered sequence.
 * SystemRegistry owns the registered systems and drives Update each frame.
 */

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace atlas::ecs {

class World;

/**
 * Abstract base for any system that operates on the ECS World each tick.
 *
 * Systems are sorted by priority (lower runs first).  Two systems with
 * the same priority run in registration order.
 */
class ISystem {
public:
    virtual ~ISystem() = default;

    /** Human-readable name for logging / debug UI. */
    virtual const char* Name() const = 0;

    /** Lower values run earlier.  Default is 100. */
    virtual int Priority() const { return 100; }

    /** Called once when the system is first registered. */
    virtual void Init(World& /*world*/) {}

    /** Called every tick.  @p dt is the frame delta in seconds. */
    virtual void Execute(World& world, float dt) = 0;

    /** Called once when the system is removed or the registry shuts down. */
    virtual void Shutdown(World& /*world*/) {}

    /** Whether the system is enabled.  Disabled systems are skipped. */
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }

private:
    bool m_enabled = true;
};

/**
 * Owns a set of ISystem instances and ticks them each frame.
 *
 * Usage:
 *   SystemRegistry reg;
 *   reg.Register(std::make_unique<MyPhysicsSystem>());
 *   reg.Register(std::make_unique<MyAISystem>());
 *   reg.InitAll(world);
 *   // ... each frame:
 *   reg.UpdateAll(world, dt);
 *   // ... on shutdown:
 *   reg.ShutdownAll(world);
 */
class SystemRegistry {
public:
    /** Register a system.  The registry takes ownership. */
    void Register(std::unique_ptr<ISystem> system);

    /** Remove a system by pointer.  Returns true if found and removed. */
    bool Unregister(ISystem* system);

    /** Call Init on every registered system (in priority order). */
    void InitAll(World& world);

    /** Tick every enabled system (in priority order). */
    void UpdateAll(World& world, float dt);

    /** Call Shutdown on every registered system (reverse priority order). */
    void ShutdownAll(World& world);

    /** Number of registered systems. */
    size_t Count() const { return m_systems.size(); }

    /** Find a system by name.  Returns nullptr if not found. */
    ISystem* FindByName(const char* name) const;

private:
    void SortByPriority();

    struct Entry {
        std::unique_ptr<ISystem> system;
        bool initialized = false;
    };
    std::vector<Entry> m_systems;
    bool m_sorted = false;
};

} // namespace atlas::ecs
