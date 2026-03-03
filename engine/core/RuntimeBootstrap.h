#pragma once
/**
 * @file RuntimeBootstrap.h
 * @brief Unified entry point for Editor, Game (Client), and Server modes.
 *
 * RuntimeBootstrap consolidates the common engine initialization sequence
 * so that all three runtime modes share the same core boot logic with
 * zero duplication.  Mode-specific differences are isolated in
 * LoadPlatformSystems().
 *
 * Usage:
 *   atlas::RuntimeBootstrap bootstrap;
 *   atlas::Engine& engine = bootstrap.Initialize(atlas::RuntimeMode::Editor);
 *   // ... set up mode-specific resources ...
 *   engine.Run();
 */

#include "Engine.h"
#include <memory>

namespace atlas {

/** Runtime modes that RuntimeBootstrap understands. */
enum class RuntimeMode {
    Editor,
    Game,   // Client / standalone gameplay
    Server
};

/**
 * Single entry point that boots the engine in the requested mode.
 *
 * Initialization order:
 *   1. LoadCoreSystems()          — logger, config, ECS, event bus
 *   2. LoadPlatformSystems(mode)  — rendering, UI, networking, editor
 *   3. LoadGameModules()          — game-specific module hooks
 */
class RuntimeBootstrap {
public:
    RuntimeBootstrap();
    ~RuntimeBootstrap();

    // Non-copyable
    RuntimeBootstrap(const RuntimeBootstrap&) = delete;
    RuntimeBootstrap& operator=(const RuntimeBootstrap&) = delete;

    /**
     * Perform the full initialization sequence and return a reference
     * to the ready-to-run Engine.
     *
     * May only be called once; subsequent calls return the same engine.
     */
    Engine& Initialize(RuntimeMode mode);

    /** Whether Initialize() has been called. */
    bool IsInitialized() const { return m_initialized; }

    /** The mode that was passed to Initialize(). */
    RuntimeMode Mode() const { return m_mode; }

    /** Access the underlying Engine (valid after Initialize). */
    Engine& GetEngine();

private:
    void LoadCoreSystems();
    void LoadPlatformSystems(RuntimeMode mode);
    void LoadGameModules();

    static EngineMode ToEngineMode(RuntimeMode mode);

    bool m_initialized = false;
    RuntimeMode m_mode = RuntimeMode::Game;
    std::unique_ptr<Engine> m_engine;
};

} // namespace atlas
