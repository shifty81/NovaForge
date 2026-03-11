#include "core/application.h"
#include "rendering/window.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "core/game_client.h"
#include "core/entity.h"
#include "core/embedded_server.h"
#include "core/session_manager.h"
#include "core/solar_system_scene.h"
#include "core/ship_physics.h"
#include "ui/input_handler.h"
#include "ui/entity_picker.h"
#include "ui/context_menu.h"
#include "ui/radial_menu.h"
#include <GLFW/glfw3.h>
#include "ui/atlas/atlas_context.h"
#include "ui/atlas/atlas_hud.h"
#include "ui/atlas/atlas_console.h"
#include "ui/atlas/atlas_pause_menu.h"
#include "ui/atlas/atlas_title_screen.h"
#ifdef NOVAFORGE_EDITOR_TOOLS
#include "editor/editor_tool_layer.h"
#endif
#include <iostream>
#include <algorithm>

namespace atlas {

Application* Application::s_instance = nullptr;

Application::Application(const std::string& title, int width, int height)
    : m_running(false)
    , m_lastFrameTime(0.0f)
    , m_currentTargetIndex(-1)
{
    if (s_instance != nullptr) {
        throw std::runtime_error("Application already exists");
    }
    s_instance = this;
    
    std::cout << "Creating application: " << title << std::endl;
    
    // Create window
    m_window = std::make_unique<Window>(title, width, height);
    
    // Create subsystems
    m_renderer = std::make_unique<Renderer>();
    m_gameClient = std::make_unique<GameClient>();
    m_inputHandler = std::make_unique<InputHandler>();
    m_camera = std::make_unique<Camera>(DEFAULT_CAMERA_FOV, static_cast<float>(width) / height, DEFAULT_CAMERA_NEAR_PLANE, DEFAULT_CAMERA_FAR_PLANE);
    m_embeddedServer = std::make_unique<EmbeddedServer>();
    m_sessionManager = std::make_unique<SessionManager>();
    m_entityPicker = std::make_unique<UI::EntityPicker>();
    m_solarSystem = std::make_unique<SolarSystemScene>();
    m_shipPhysics = std::make_unique<ShipPhysics>();
    m_atlasCtx = std::make_unique<atlas::AtlasContext>();
    m_atlasHUD = std::make_unique<atlas::AtlasHUD>();
    m_console = std::make_unique<atlas::AtlasConsole>();
    m_pauseMenu = std::make_unique<atlas::AtlasPauseMenu>();
    m_titleScreen = std::make_unique<atlas::AtlasTitleScreen>();
    m_contextMenu = std::make_unique<UI::ContextMenu>();
    m_radialMenu = std::make_unique<UI::RadialMenu>();

#ifdef NOVAFORGE_EDITOR_TOOLS
    m_editorToolLayer = std::make_unique<atlas::editor::EditorToolLayer>();
#endif
    
    // Initialize
    initialize();
}

Application::~Application() {
    cleanup();
    s_instance = nullptr;
    std::cout << "Application destroyed" << std::endl;
}

void Application::run() {
    std::cout << "Starting main loop..." << std::endl;
    m_running = true;
    m_lastFrameTime = static_cast<float>(glfwGetTime());
    
    // Main game loop
    while (m_running && !m_window->shouldClose()) {
        // Calculate delta time
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
        m_deltaTime = deltaTime;

        // Reset per-frame input state before polling events
        m_inputHandler->beginFrame();

        // Poll events so transient input flags (clicked, released) are
        // available during update and render within the same frame
        m_window->pollEvents();
        
        // Update
        update(deltaTime);
        
        // Render
        render();
        
        // Present the frame
        m_window->swapBuffers();
    }
    
    std::cout << "Main loop ended" << std::endl;
}

void Application::shutdown() {
    std::cout << "Shutdown requested" << std::endl;
    m_running = false;
}

void Application::initialize() {
    std::cout << "Initializing application..." << std::endl;
    
    // Initialize renderer
    if (!m_renderer->initialize()) {
        throw std::runtime_error("Failed to initialize renderer");
    }
    
    // Initialize Atlas UI context
    m_atlasCtx->init();
    m_atlasHUD->init(m_window->getWidth(), m_window->getHeight());
    
    // Wire Atlas sidebar icon callbacks so clicking sidebar opens panels
    m_atlasHUD->setSidebarCallback([this](int icon) {
        std::cout << "[Nexcom] Sidebar icon " << icon << " clicked" << std::endl;
        switch (icon) {
            case 0:
                std::cout << "[Nexcom] Toggle Inventory" << std::endl;
                m_atlasHUD->toggleInventory();
                break;
            case 1:
                std::cout << "[Nexcom] Toggle Fitting" << std::endl;
                m_atlasHUD->toggleFitting();
                break;
            case 2:
                std::cout << "[Nexcom] Toggle Market" << std::endl;
                m_atlasHUD->toggleMarket();
                break;
            case 3:
                std::cout << "[Nexcom] Toggle Missions" << std::endl;
                m_atlasHUD->toggleMission();
                break;
            case 4:
                std::cout << "[Nexcom] Toggle Proxscan" << std::endl;
                m_atlasHUD->toggleProxscan();
                break;
            case 5:
                std::cout << "[Nexcom] Toggle Overview" << std::endl;
                m_atlasHUD->toggleOverview();
                break;
            case 6:
                std::cout << "[Nexcom] Toggle Chat" << std::endl;
                m_atlasHUD->toggleChat();
                break;
            case 7:
                std::cout << "[Nexcom] Toggle Drones" << std::endl;
                m_atlasHUD->toggleDronePanel();
                break;
        }
    });
    
    // Wire console callbacks
    m_console->setQuitCallback([this]() { shutdown(); });
    m_console->setSaveCallback([this]() {
        std::cout << "[Console] Force save requested" << std::endl;
        if (m_embeddedServer && m_embeddedServer->isRunning()) {
            std::cout << "[Console] Requesting world save from embedded server..." << std::endl;
        } else {
            auto* networkMgr = m_gameClient->getNetworkManager();
            if (networkMgr && networkMgr->isConnected()) {
                // Use chat command to request save from remote server
                // (server console processes /save as admin command)
                networkMgr->sendChat("/save");
                std::cout << "[Console] Save request sent to remote server" << std::endl;
            } else {
                std::cout << "[Console] No server connection available for save" << std::endl;
            }
        }
    });

    // Wire pause menu callbacks
    m_pauseMenu->setResumeCallback([this]() {
        std::cout << "[PauseMenu] Resumed" << std::endl;
        // Re-capture FPS cursor so mouse look resumes immediately
        if (isInFPSMode()) {
            captureFPSCursor();
        }
    });
    m_pauseMenu->setSaveCallback([this]() {
        std::cout << "[PauseMenu] Save requested" << std::endl;
    });
    m_pauseMenu->setQuitCallback([this]() { shutdown(); });

    // Wire title screen callbacks
    m_titleScreen->setPlayCallback([this]() {
        std::cout << "[TitleScreen] Entering game..." << std::endl;

        // ── Spawn player docked at the nearest station ─────────────
        // The title screen's "Enter Hangar" flow ends here.  Place the
        // player at a station so they start the game inside the hangar
        // rather than floating in open space.
        if (m_solarSystem) {
            bool docked = false;
            for (const auto& c : m_solarSystem->getCelestials()) {
                if (c.type != atlas::Celestial::Type::STATION) continue;
                m_dockedStationId = c.id;
                auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
                if (playerEntity) {
                    Health currentHealth = playerEntity->getHealth();
                    m_gameClient->getEntityManager().updateEntityState(
                        m_localPlayerId, c.position, glm::vec3(0.0f), 0.0f, currentHealth);
                }
                // Go directly to Docked state (bypass the normal
                // InSpace→Docking→Docked transition since we are spawning)
                m_gameState = GameState::Docked;
                m_camera->setViewMode(atlas::ViewMode::FPS);
                // Place the player at eye level on the hangar catwalk,
                // looking toward the landing pad and ship.
                m_camera->setFPSPosition(glm::vec3(-30.0f, 1.8f, 0.0f),
                                          glm::vec3(1.0f, 0.0f, 0.0f));
                m_activeModeText = "DOCKED";
                std::cout << "[Hangar] Player spawned docked at " << c.name << std::endl;
                docked = true;
                break;
            }
            if (!docked) {
                std::cerr << "[Hangar] WARNING: No station found in solar system — "
                             "player remains in space" << std::endl;
            }
        }
    });
    m_titleScreen->setQuitCallback([this]() { shutdown(); });
    
    // Set up input callbacks — Astralis style controls
    // Left-click: select/target, Double-click: approach
    // Right-click: context menu
    // Left-drag: nothing (UI uses it for interaction)
    // Right-drag: orbit camera around ship
    // Scroll: zoom camera
    m_window->setKeyCallback([this](int key, int, int action, int mods) {
#ifdef NOVAFORGE_EDITOR_TOOLS
        // F12 toggles the editor tool overlay
        if (key == GLFW_KEY_F12 && action == GLFW_PRESS) {
            if (m_editorToolLayer) {
                m_editorToolLayer->toggle();
                std::cout << "[ToolLayer] Editor overlay "
                          << (m_editorToolLayer->isActive() ? "ON" : "OFF")
                          << std::endl;
            }
            return;
        }

        // Forward key presses to the editor tool layer when active
        if (m_editorToolLayer && m_editorToolLayer->isActive() && action == GLFW_PRESS) {
            m_editorToolLayer->handleKeyPress(key, mods);
        }
#endif

        // Backtick (`) toggles developer console
        if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
            m_console->toggle();
            return;
        }

        // Console eats all key input when open
        if (m_console && m_console->isOpen()) {
            m_console->handleKey(key, action);
            return;
        }

        // Title screen forwards key events to its character-name input field
        if (m_titleScreen && m_titleScreen->isActive()) {
            m_titleScreen->handleKey(key, action);
            return;
        }

        // ESC toggles pause menu (instead of quitting)
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            if (m_pauseMenu->isOpen()) {
                // Closing the pause menu
                m_pauseMenu->toggle();
                // Re-capture FPS cursor so mouse look resumes immediately
                if (isInFPSMode()) {
                    captureFPSCursor();
                }
            } else {
                // Opening the pause menu
                // In FPS mode: release the cursor so the pause menu (and
                // any other UI) is immediately interactive.  Warp the tracked
                // mouse position to the screen centre so that the Atlas UI
                // receives a valid absolute coordinate on the very next frame.
                if (isInFPSMode() && m_fpsCursorCaptured) {
                    releaseFPSCursor();
                    if (m_window && m_window->getHandle()) {
                        int w = m_window->getWidth();
                        int h = m_window->getHeight();
                        double cx = w * 0.5;
                        double cy = h * 0.5;
                        glfwSetCursorPos(m_window->getHandle(), cx, cy);
                        m_inputHandler->warpMousePosition(cx, cy);
                    }
                }
                m_pauseMenu->toggle();
            }
            return;
        }

        // Don't forward keys when pause menu is active
        if (m_pauseMenu && m_pauseMenu->isOpen()) {
            return;
        }

        m_inputHandler->handleKey(key, action, mods);
    });

    m_window->setCharCallback([this](unsigned int codepoint) {
        // Forward character input to console when open
        if (m_console && m_console->isOpen()) {
            m_console->handleChar(codepoint);
            return;
        }
        // Forward to title screen name input when active
        if (m_titleScreen && m_titleScreen->isActive()) {
            m_titleScreen->handleChar(codepoint);
        }
    });
    
    m_window->setMouseCallback([this](double xpos, double ypos) {
        m_inputHandler->handleMouse(xpos, ypos);
    });
    
    m_window->setMouseButtonCallback([this](int button, int action, int mods) {
        double x = m_inputHandler->getMouseX();
        double y = m_inputHandler->getMouseY();
        m_inputHandler->handleMouseButton(button, action, mods, x, y);
    });
    
    // Scroll callback — Astralis uses mousewheel for camera zoom
    m_window->setScrollCallback([this](double xoffset, double yoffset) {
        m_inputHandler->handleScroll(xoffset, yoffset);
        handleScroll(xoffset, yoffset);
    });
    
    m_window->setResizeCallback([this](int width, int height) {
        m_renderer->setViewport(0, 0, width, height);
    });
    
    // Register input handler callbacks
    m_inputHandler->setKeyCallback([this](int key, int action, int mods) {
        handleKeyInput(key, action, mods);
    });
    
    m_inputHandler->setMouseButtonCallback([this](int button, int action, int mods, double x, double y) {
        handleMouseButton(button, action, mods, x, y);
    });
    
    m_inputHandler->setMouseMoveCallback([this](double x, double y, double deltaX, double deltaY) {
        handleMouseMove(x, y, deltaX, deltaY);
    });
    
    // Set initial viewport
    m_renderer->setViewport(0, 0, m_window->getWidth(), m_window->getHeight());
    
    // Set up entity event callbacks
    m_gameClient->setOnEntitySpawned([this](const std::shared_ptr<Entity>& entity) {
        std::cout << "Application: Entity spawned event received" << std::endl;
        m_renderer->createEntityVisual(entity);
    });
    
    m_gameClient->setOnEntityDestroyed([this](const std::shared_ptr<Entity>& entity) {
        std::cout << "Application: Entity destroyed event received" << std::endl;
        m_renderer->removeEntityVisual(entity->getId());
    });
    
    // Setup UI callbacks for network integration
    setupUICallbacks();
    
    // Spawn local player entity so ship is always visible (PVE mode)
    spawnLocalPlayerEntity();
    spawnDemoNPCEntities();
    
    // Load the solar system (with sun, planets, stations etc.)
    m_solarSystem->loadTestSystem();
    
    // Set up sun rendering from solar system data
    const auto* sun = m_solarSystem->getSun();
    if (sun) {
        m_renderer->setSunState(sun->position, sun->lightColor, sun->radius);
        std::cout << "[PVE] Sun configured at origin with radius " << sun->radius << "m" << std::endl;
    }

    // Set system info in HUD
    m_atlasHUD->setSystemInfo(m_solarSystem->getSystemName(),
                              m_solarSystem->getSecurityLevel());

    // Wire warp callback so UI and renderer are updated when a warp begins
    m_solarSystem->setWarpCallback([this](const std::string& celestialId) {
        std::cout << "[Warp] Warp initiated to celestial: " << celestialId << std::endl;

        // Send warp request to server if connected
        auto* networkMgr = m_gameClient->getNetworkManager();
        if (networkMgr && networkMgr->isConnected()) {
            networkMgr->sendDockRequest(celestialId);  // reuse dock channel until dedicated warp msg
        }
    });
    
    // Set initial camera to orbit around player
    m_camera->setDistance(DEFAULT_CAMERA_DISTANCE);
    m_camera->rotate(DEFAULT_CAMERA_PITCH, 0.0f);

#ifdef NOVAFORGE_EDITOR_TOOLS
    // Initialize the editor tool layer (all editor panels as a client overlay)
    if (m_editorToolLayer) {
        m_editorToolLayer->init();
        std::cout << "[ToolLayer] Editor tools available — press F12 to toggle"
                  << std::endl;
    }
#endif
    
    std::cout << "Application initialized successfully" << std::endl;
}

void Application::setupUICallbacks() {
    std::cout << "Setting up UI callbacks for network integration..." << std::endl;
    
    // Get network manager from game client
    auto* networkMgr = m_gameClient->getNetworkManager();
    if (!networkMgr) {
        std::cout << "NetworkManager not available yet, skipping UI callback setup" << std::endl;
        return;
    }
    
    // === Setup Response Callbacks (Network → UI) ===
    networkMgr->setInventoryCallback([](const atlas::InventoryResponse& response) {
        if (response.success) {
            std::cout << "✓ Inventory operation succeeded: " << response.message << std::endl;
        } else {
            std::cerr << "✗ Inventory operation failed: " << response.message << std::endl;
        }
    });

    networkMgr->setFittingCallback([](const atlas::FittingResponse& response) {
        if (response.success) {
            std::cout << "✓ Fitting operation succeeded: " << response.message << std::endl;
        } else {
            std::cerr << "✗ Fitting operation failed: " << response.message << std::endl;
        }
    });

    networkMgr->setMarketCallback([](const atlas::MarketResponse& response) {
        if (response.success) {
            std::cout << "✓ Market transaction succeeded: " << response.message << std::endl;
        } else {
            std::cerr << "✗ Market transaction failed: " << response.message << std::endl;
        }
    });
    
    // Error response callback (general errors)
    networkMgr->setErrorCallback([](const std::string& message) {
        std::cerr << "✗ Server error: " << message << std::endl;
        // TODO: Could show a general error dialog here
    });
    
    std::cout << "  - Response callbacks wired for all panels" << std::endl;
    
    // === Setup Context Menu Callbacks ===
    m_contextMenu->SetApproachCallback([this](const std::string& entityId) {
        commandApproach(entityId);
    });
    
    m_contextMenu->SetOrbitCallback([this](const std::string& entityId, int distance_m) {
        commandOrbit(entityId, static_cast<float>(distance_m));
    });
    
    m_contextMenu->SetKeepAtRangeCallback([this](const std::string& entityId, int distance_m) {
        commandKeepAtRange(entityId, static_cast<float>(distance_m));
    });
    
    m_contextMenu->SetWarpToCallback([this](const std::string& entityId, int distance_m) {
        // For now, warp just treats it as approach
        std::cout << "[Movement] Warp to " << entityId << " at " << distance_m << "m distance" << std::endl;
        commandWarpTo(entityId);
    });
    
    m_contextMenu->SetLockTargetCallback([this](const std::string& entityId) {
        if (std::find(m_targetList.begin(), m_targetList.end(), entityId) == m_targetList.end()) {
            m_targetList.push_back(entityId);
            std::cout << "[Targeting] Locked target: " << entityId << std::endl;
            // Send target lock to server
            auto* networkMgr = m_gameClient->getNetworkManager();
            if (networkMgr && networkMgr->isConnected()) {
                networkMgr->sendTargetLock(entityId);
            }
        }
    });
    
    m_contextMenu->SetUnlockTargetCallback([this](const std::string& entityId) {
        auto it = std::find(m_targetList.begin(), m_targetList.end(), entityId);
        if (it != m_targetList.end()) {
            m_targetList.erase(it);
            std::cout << "[Targeting] Unlocked target: " << entityId << std::endl;
            // Send target unlock to server
            auto* networkMgr = m_gameClient->getNetworkManager();
            if (networkMgr && networkMgr->isConnected()) {
                networkMgr->sendTargetUnlock(entityId);
            }
        }
    });
    
    m_contextMenu->SetLookAtCallback([this](const std::string& entityId) {
        auto entity = m_gameClient->getEntityManager().getEntity(entityId);
        if (entity) {
            m_camera->setTarget(entity->getPosition());
            std::cout << "[Camera] Looking at entity: " << entityId << std::endl;
        } else if (m_solarSystem) {
            // Check if it's a celestial
            const auto* cel = m_solarSystem->findCelestial(entityId);
            if (cel) {
                m_camera->setTarget(cel->position);
                std::cout << "[Camera] Looking at celestial: " << cel->name << std::endl;
            }
        }
    });
    
    m_contextMenu->SetShowInfoCallback([this](const std::string& entityId) {
        std::cout << "[Info] Show info for: " << entityId << std::endl;
        openInfoPanelForEntity(entityId);
    });

    m_contextMenu->SetJumpCallback([this](const std::string& entityId) {
        commandJump(entityId);
    });

    m_contextMenu->SetAlignToCallback([this](const std::string& entityId) {
        commandAlignTo(entityId);
    });

    m_contextMenu->SetNavigateToCallback([this](float x, float y, float z) {
        std::cout << "[Navigate] Align to position (" << x << ", " << y << ", " << z << ")" << std::endl;
    });

    m_contextMenu->SetBookmarkCallback([this](float x, float y, float z) {
        std::cout << "[Bookmark] Saved location (" << x << ", " << y << ", " << z << ")" << std::endl;
    });
    
    std::cout << "  - Context menu callbacks wired" << std::endl;
    
    // === Setup Radial Menu Callbacks ===
    m_radialMenu->SetActionCallback([this](UI::RadialMenu::Action action, const std::string& entityId) {
        switch (action) {
            case UI::RadialMenu::Action::APPROACH:
                commandApproach(entityId);
                break;
            case UI::RadialMenu::Action::ORBIT:
                commandOrbit(entityId, DEFAULT_ORBIT_DISTANCE);  // Default orbit distance
                break;
            case UI::RadialMenu::Action::KEEP_AT_RANGE:
                commandKeepAtRange(entityId, DEFAULT_KEEP_AT_RANGE);  // Default range
                break;
            case UI::RadialMenu::Action::WARP_TO:
                commandWarpTo(entityId);
                break;
            case UI::RadialMenu::Action::LOCK_TARGET:
                if (std::find(m_targetList.begin(), m_targetList.end(), entityId) == m_targetList.end()) {
                    m_targetList.push_back(entityId);
                    std::cout << "[Targeting] Locked target: " << entityId << std::endl;
                }
                break;
            case UI::RadialMenu::Action::ALIGN_TO:
                commandAlignTo(entityId);
                break;
            case UI::RadialMenu::Action::LOOK_AT:
                {
                    auto entity = m_gameClient->getEntityManager().getEntity(entityId);
                    if (entity) {
                        m_camera->setTarget(entity->getPosition());
                        std::cout << "[Camera] Looking at entity: " << entityId << std::endl;
                    } else if (m_solarSystem) {
                        const auto* cel = m_solarSystem->findCelestial(entityId);
                        if (cel) {
                            m_camera->setTarget(cel->position);
                            std::cout << "[Camera] Looking at celestial: " << cel->name << std::endl;
                        }
                    }
                }
                break;
            case UI::RadialMenu::Action::SHOW_INFO:
                {
                    std::cout << "[Info] Show info for: " << entityId << std::endl;
                    openInfoPanelForEntity(entityId);
                }
                break;
            default:
                break;
        }

        // ── FPS mode actions ───────────────────────────────────────
        // These are dispatched to the server via network messages.
        // For now, log the action; full server integration follows.
        switch (action) {
            case UI::RadialMenu::Action::FPS_USE:
                std::cout << "[FPS] Use: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_OPEN:
                std::cout << "[FPS] Open: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_CLOSE:
                std::cout << "[FPS] Close: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_LOCK:
                std::cout << "[FPS] Lock: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_UNLOCK:
                std::cout << "[FPS] Unlock: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_HACK:
                std::cout << "[FPS] Hack: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_LOOT_ALL:
                std::cout << "[FPS] Loot All: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_SEARCH:
                std::cout << "[FPS] Search: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_REPAIR:
                std::cout << "[FPS] Repair: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_HEAL:
                std::cout << "[FPS] Heal: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_RESTOCK:
                std::cout << "[FPS] Restock: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_CRAFT:
                std::cout << "[FPS] Craft: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_EVA_BEGIN:
                std::cout << "[FPS] Begin EVA: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_EVA_ABORT:
                std::cout << "[FPS] Abort EVA: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_ACCESS_TERMINAL:
                std::cout << "[FPS] Access Terminal: " << entityId << std::endl;
                break;
            case UI::RadialMenu::Action::FPS_EXAMINE:
                std::cout << "[FPS] Examine: " << entityId << std::endl;
                break;
            default:
                break;
        }
    });
    
    std::cout << "  - Radial menu callbacks wired" << std::endl;
    
    // === Setup Selected Item Panel Callbacks ===
    m_atlasHUD->setSelectedItemOrbitCb([this]() {
        if (!m_currentTargetId.empty()) {
            commandOrbit(m_currentTargetId);
        }
    });
    m_atlasHUD->setSelectedItemApproachCb([this]() {
        if (!m_currentTargetId.empty()) {
            commandApproach(m_currentTargetId);
        }
    });
    m_atlasHUD->setSelectedItemWarpCb([this]() {
        if (!m_currentTargetId.empty()) {
            commandWarpTo(m_currentTargetId);
        }
    });
    m_atlasHUD->setSelectedItemInfoCb([this]() {
        if (!m_currentTargetId.empty()) {
            openInfoPanelForEntity(m_currentTargetId);
        }
    });
    std::cout << "  - Selected item panel callbacks wired" << std::endl;

    // === Setup Station Panel Callbacks ===
    m_atlasHUD->setStationDockCb([this]() { requestDock(); });
    m_atlasHUD->setStationUndockCb([this]() { requestUndock(); });
    m_atlasHUD->setStationRepairCb([this]() {
        std::cout << "[Station] Repair requested" << std::endl;
    });
    std::cout << "  - Station panel callbacks wired" << std::endl;

    // === Setup Overview Interaction Callbacks ===
    m_atlasHUD->setOverviewSelectCb([this](const std::string& entityId) {
        targetEntity(entityId, false);
        std::cout << "[Overview] Selected entity: " << entityId << std::endl;
    });

    m_atlasHUD->setOverviewRightClickCb([this](const std::string& entityId, float screenX, float screenY) {
        bool isLocked = std::find(m_targetList.begin(), m_targetList.end(), entityId) != m_targetList.end();
        bool isStargate = false;
        if (m_solarSystem) {
            const auto* cel = m_solarSystem->findCelestial(entityId);
            if (cel && cel->type == atlas::Celestial::Type::STARGATE)
                isStargate = true;
        }
        m_contextMenu->ShowEntityMenu(entityId, isLocked, isStargate);
        m_contextMenu->SetScreenPosition(screenX, screenY);
        std::cout << "[Overview] Right-click context menu for: " << entityId << std::endl;
    });

    m_atlasHUD->setOverviewBgRightClickCb([this](float screenX, float screenY) {
        m_contextMenu->ShowEmptySpaceMenu(0.0f, 0.0f, 0.0f);
        m_contextMenu->SetScreenPosition(screenX, screenY);
        std::cout << "[Overview] Right-click empty space context menu" << std::endl;
    });

    // Ctrl+Click on overview row = lock target (Astralis standard)
    m_atlasHUD->setOverviewCtrlClickCb([this](const std::string& entityId) {
        targetEntity(entityId, true);  // addToTargets = true for lock
        std::cout << "[Overview] Ctrl+Click lock target: " << entityId << std::endl;
    });

    std::cout << "  - Overview interaction callbacks wired" << std::endl;
    
    std::cout << "UI callbacks setup complete" << std::endl;
}

void Application::update(float deltaTime) {
    // Update FPS for console display
    if (m_console) {
        m_console->setFPS(deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f);
    }

    // Skip game logic when title screen is active
    if (m_titleScreen && m_titleScreen->isActive()) {
        return;
    }

    // Skip game logic when paused (pause menu is open)
    if (m_pauseMenu && m_pauseMenu->isOpen()) {
        return;
    }

    // Update embedded server if running
    if (m_embeddedServer) {
        m_embeddedServer->update(deltaTime);
    }
    
    // Update session manager
    if (m_sessionManager) {
        m_sessionManager->update(deltaTime);
    }
    
    // Update game client
    m_gameClient->update(deltaTime);
    
    // Update local movement (PVE mode — Astralis-style movement commands)
    // Only process flight movement when in space or docking
    if (m_gameState == GameState::InSpace) {
        updateLocalMovement(deltaTime);
    }

    // Update FPS character movement when on foot
    if (isInFPSMode()) {
        updateFPSMovement(deltaTime);
    }

    // Docking animation timer
    if (m_gameState == GameState::Docking) {
        m_dockingTimer -= deltaTime;
        if (m_dockingTimer <= 0.0f) {
            m_dockingTimer = 0.0f;
            requestStateTransition(GameState::Docked);
            std::cout << "[Docking] Docking complete — entered hangar" << std::endl;
        }
    }
    
    // Update solar system scene (engine trail, warp visual state)
    if (m_solarSystem && m_shipPhysics) {
        m_solarSystem->update(deltaTime, m_shipPhysics.get());

        // Feed engine trail state into renderer
        const auto& trail = m_solarSystem->getEngineTrailState();
        m_renderer->setEngineTrailState(trail.emitting, trail.intensity,
                                        trail.position, trail.velocity);
    }
    
    // Update ship status in the HUD
    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    if (playerEntity) {
        // Update player position for UI calculations (e.g., distance in overview/targets)
        const auto playerPosition = playerEntity->getPosition();
        updateTargetListUi(playerPosition);

        // Camera follows player ship
        m_camera->setTarget(playerPosition);
    }
}

void Application::updateTargetListUi(const glm::vec3& /*playerPosition*/) {
    // Atlas HUD target cards are built in render(); nothing to do here.
}

void Application::cleanup() {
    std::cout << "Cleaning up application..." << std::endl;

#ifdef NOVAFORGE_EDITOR_TOOLS
    // Shutdown editor tool layer before UI context
    if (m_editorToolLayer) {
        m_editorToolLayer->shutdown();
    }
#endif
    
    // Shutdown Atlas UI
    if (m_atlasCtx) {
        m_atlasCtx->shutdown();
    }
    
    // Leave session and stop server if hosting
    if (m_sessionManager) {
        m_sessionManager->leaveSession();
    }
    
    if (m_embeddedServer && m_embeddedServer->isRunning()) {
        m_embeddedServer->stop();
    }
    
    // Disconnect from server if connected
    if (m_gameClient) {
        m_gameClient->disconnect();
    }
    
    std::cout << "Cleanup complete" << std::endl;
}

} // namespace atlas
