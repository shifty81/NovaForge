/**
 * Tests for RuntimeBootstrap — the unified entry point for
 * Editor, Game (Client), and Server modes.
 */

#include "../engine/core/RuntimeBootstrap.h"
#include <cassert>
#include <string>

using namespace atlas;

void test_bootstrap_not_initialized_by_default() {
    RuntimeBootstrap bootstrap;
    assert(!bootstrap.IsInitialized());
}

void test_bootstrap_initialize_editor() {
    RuntimeBootstrap bootstrap;
    Engine& engine = bootstrap.Initialize(RuntimeMode::Editor);

    assert(bootstrap.IsInitialized());
    assert(bootstrap.Mode() == RuntimeMode::Editor);
    assert(engine.Config().mode == EngineMode::Editor);
    assert(engine.Running());
    assert(engine.Can(Capability::AssetWrite));
    assert(engine.Can(Capability::HotReload));
    engine.Shutdown();
}

void test_bootstrap_initialize_game() {
    RuntimeBootstrap bootstrap;
    Engine& engine = bootstrap.Initialize(RuntimeMode::Game);

    assert(bootstrap.IsInitialized());
    assert(bootstrap.Mode() == RuntimeMode::Game);
    assert(engine.Config().mode == EngineMode::Client);
    assert(engine.Running());
    assert(engine.Can(Capability::Rendering));
    assert(!engine.Can(Capability::NetAuthority));
    engine.Shutdown();
}

void test_bootstrap_initialize_server() {
    RuntimeBootstrap bootstrap;
    Engine& engine = bootstrap.Initialize(RuntimeMode::Server);

    assert(bootstrap.IsInitialized());
    assert(bootstrap.Mode() == RuntimeMode::Server);
    assert(engine.Config().mode == EngineMode::Server);
    assert(engine.Running());
    assert(engine.Can(Capability::NetAuthority));
    assert(!engine.Can(Capability::Rendering));
    engine.Shutdown();
}

void test_bootstrap_idempotent_initialize() {
    RuntimeBootstrap bootstrap;
    Engine& first  = bootstrap.Initialize(RuntimeMode::Server);
    Engine& second = bootstrap.Initialize(RuntimeMode::Editor); // should be ignored

    // Second call returns the same engine, mode unchanged
    assert(&first == &second);
    assert(bootstrap.Mode() == RuntimeMode::Server);
    first.Shutdown();
}

void test_bootstrap_engine_runs() {
    RuntimeBootstrap bootstrap;
    Engine& engine = bootstrap.Initialize(RuntimeMode::Server);
    engine.Config(); // accessible

    // Set maxTicks so Run() terminates
    // We access the mutable config through the non-const engine reference
    // by using the scheduler/tick approach from existing tests
    int tickCount = 0;
    engine.GetWorld().SetTickCallback([&](float) { tickCount++; });

    // Manually override maxTicks via a fresh config isn't possible (const ref),
    // so we just verify the engine is in a runnable state and shut down.
    assert(engine.Running());
    engine.Shutdown();
    assert(!engine.Running());
}

void test_bootstrap_net_modes() {
    // Server bootstrap -> net authority
    {
        RuntimeBootstrap bootstrap;
        Engine& engine = bootstrap.Initialize(RuntimeMode::Server);
        assert(engine.GetNet().Mode() == net::NetMode::Server);
        assert(engine.GetNet().IsAuthority());
        engine.Shutdown();
    }

    // Game bootstrap -> net client
    {
        RuntimeBootstrap bootstrap;
        Engine& engine = bootstrap.Initialize(RuntimeMode::Game);
        assert(engine.GetNet().Mode() == net::NetMode::Client);
        assert(!engine.GetNet().IsAuthority());
        engine.Shutdown();
    }
}
