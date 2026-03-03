#include "RuntimeBootstrap.h"
#include "Logger.h"

namespace atlas {

RuntimeBootstrap::RuntimeBootstrap() = default;
RuntimeBootstrap::~RuntimeBootstrap() = default;

Engine& RuntimeBootstrap::Initialize(RuntimeMode mode) {
    if (m_initialized) {
        return *m_engine;
    }

    m_mode = mode;

    // Create the engine with the translated mode
    EngineConfig cfg;
    cfg.mode = ToEngineMode(mode);
    m_engine = std::make_unique<Engine>(cfg);

    LoadCoreSystems();
    LoadPlatformSystems(mode);
    LoadGameModules();

    m_initialized = true;
    Logger::Info("RuntimeBootstrap: initialization complete");
    return *m_engine;
}

Engine& RuntimeBootstrap::GetEngine() {
    return *m_engine;
}

// ─────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────

void RuntimeBootstrap::LoadCoreSystems() {
    // Core systems shared by every mode: logging, ECS, event bus
    m_engine->InitCore();
    m_engine->InitECS();
    Logger::Info("RuntimeBootstrap: core systems loaded");
}

void RuntimeBootstrap::LoadPlatformSystems(RuntimeMode mode) {
    // Rendering and UI are skipped for Server mode (handled inside
    // Engine::InitRender / InitUI), but we call them uniformly so
    // the engine can make the decision internally.
    m_engine->InitRender();
    m_engine->InitUI();
    m_engine->InitNetworking();

    if (mode == RuntimeMode::Editor) {
        m_engine->InitEditor();
    }

    Logger::Info("RuntimeBootstrap: platform systems loaded");
}

void RuntimeBootstrap::LoadGameModules() {
    // Hook point for game-specific module initialisation (ships, PCG,
    // AI, story systems).  Currently a no-op — concrete game code
    // should extend or call into its own module loader after
    // Initialize() returns.
    Logger::Info("RuntimeBootstrap: game modules loaded");
}

EngineMode RuntimeBootstrap::ToEngineMode(RuntimeMode mode) {
    switch (mode) {
        case RuntimeMode::Editor: return EngineMode::Editor;
        case RuntimeMode::Game:   return EngineMode::Client;
        case RuntimeMode::Server: return EngineMode::Server;
    }
    return EngineMode::Client;
}

} // namespace atlas
