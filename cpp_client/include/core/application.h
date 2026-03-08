#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace atlas {

// Forward declarations
class Window;
class GameClient;
class Renderer;
class InputHandler;
class Camera;
class EmbeddedServer;
class SessionManager;
class SolarSystemScene;
class ShipPhysics;

} // namespace atlas

namespace UI {
    class EntityPicker;
    class ContextMenu;
    class RadialMenu;
}

namespace atlas {
    class AtlasHUD;
    class AtlasContext;
    class AtlasConsole;
    class AtlasPauseMenu;
    class AtlasTitleScreen;
}

#ifdef NOVAFORGE_EDITOR_TOOLS
namespace atlas::editor { class EditorToolLayer; }
#endif

namespace atlas {

/**
 * Main application class for Nova Forge client
 * 
 * Manages the game loop, window, and all core systems including:
 * - Rendering (3D graphics, UI)
 * - Input handling (keyboard, mouse)
 * - Networking (client-server or embedded server)
 * - Camera control (Astralis-style right-click camera, FPS, cockpit)
 * - Entity management and targeting
 * - Session management (singleplayer/multiplayer)
 * - Game state management (space flight, docking, interiors)
 * 
 * Lifecycle:
 * 1. Constructor: Create window and systems
 * 2. initialize(): Set up OpenGL, load resources, connect subsystems
 * 3. run(): Main game loop (update → render → present)
 * 4. cleanup(): Shutdown systems and free resources
 * 
 * Supports two modes:
 * - **Local Mode**: Single-player with demo NPCs for testing
 * - **Multiplayer Mode**: Connect to dedicated server or host embedded server
 *
 * Game states:
 * - **InSpace**: Flying in a solar system (orbit camera)
 * - **Docking**: Playing the docking approach animation
 * - **Docked**: Inside a station hangar (orbit camera on hangar)
 * - **StationInterior**: Walking inside the station (FPS camera)
 * - **ShipInterior**: Walking inside the ship while docked (FPS camera)
 * - **Cockpit**: Seated in the ship cockpit (cockpit camera)
 *
 * View mode transitions (V key):
 * - InSpace: Orbit ↔ Cockpit
 * - Docked/ShipInterior/StationInterior: FPS ↔ Cockpit (if in ship)
 */
class Application {
public:
    /**
     * High-level game state describing where the player currently is.
     */
    enum class GameState {
        InSpace,          // Flying in a solar system
        Docking,          // Docking approach animation
        Docked,           // Inside station hangar view
        StationInterior,  // Walking inside the station (FPS)
        ShipInterior,     // Walking inside the ship (FPS)
        Cockpit           // Seated in the ship cockpit
    };

    Application(const std::string& title, int width, int height);
    ~Application();

    // Delete copy constructor and assignment
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
     * Run the main game loop
     */
    void run();

    /**
     * Request application shutdown
     */
    void shutdown();

    /**
     * Get application instance
     */
    static Application* getInstance() { return s_instance; }

    /**
     * Get embedded server (nullptr if not hosting)
     */
    EmbeddedServer* getEmbeddedServer() { return m_embeddedServer.get(); }

    /**
     * Get session manager
     */
    SessionManager* getSessionManager() { return m_sessionManager.get(); }

    /**
     * Host a multiplayer game
     * 
     * Starts an embedded server and connects the client to it. The server
     * runs in a separate thread and accepts connections from other clients.
     * 
     * @param sessionName Name for the multiplayer session
     * @param maxPlayers Maximum number of players (default 20)
     * @return true if server started successfully
     */
    bool hostMultiplayerGame(const std::string& sessionName, int maxPlayers = 20);

    /**
     * Join a multiplayer game
     * 
     * Connect to a remote dedicated server. Disconnects from any existing
     * session first.
     * 
     * @param host Server hostname or IP address
     * @param port Server port (default 7777)
     * @return true if connection successful
     */
    bool joinMultiplayerGame(const std::string& host, int port);

    /**
     * Check if hosting a game
     */
    bool isHosting() const;
    
    /**
     * Target an entity by ID
     */
    void targetEntity(const std::string& entityId, bool addToTargets = false);
    
    /**
     * Clear current target
     */
    void clearTarget();
    
    /**
     * Cycle to next target
     */
    void cycleTarget();
    
    /**
     * Activate module by slot (F1-F8)
     */
    void activateModule(int slotNumber);

    /**
     * Get the current game state.
     */
    GameState getGameState() const { return m_gameState; }

    /**
     * Get the name of the current game state (for display / logging).
     */
    static const char* gameStateName(GameState state);

    /**
     * Request a game-state transition (e.g. InSpace → Docking → Docked).
     * Invalid transitions are logged and ignored.
     */
    void requestStateTransition(GameState newState);

    /**
     * Toggle the camera view mode (V key).
     *
     * Behaviour depends on current GameState:
     * - InSpace:  ORBIT ↔ COCKPIT
     * - Docked/ShipInterior: FPS ↔ COCKPIT
     * - StationInterior: stays FPS (must board ship first)
     */
    void toggleViewMode();

    /**
     * Attempt to dock at the nearest station (within docking range).
     */
    void requestDock();

    /**
     * Undock from the current station and return to space.
     */
    void requestUndock();

    /**
     * Transition from docked state to walking inside the station (FPS).
     */
    void enterStationInterior();

    /**
     * Transition from station/docked to walking inside the ship (FPS).
     */
    void enterShipInterior();

    /**
     * Transition from ship interior to the cockpit view.
     */
    void enterCockpit();

    /**
     * Return to the hangar (orbit) view from an interior view.
     */
    void returnToHangar();

private:
    /**
     * Initialize all subsystems
     * 
     * Sets up:
     * - OpenGL context and renderer
     * - Input handlers
     * - UI manager and panels
     * - Camera
     * - Network client (if connecting to server)
     * - Local player entity (if in demo mode)
     */
    void initialize();
    
    /**
     * Wire UI callbacks to network commands
     * 
     * Connects UI panel buttons and interactions to the appropriate
     * network message handlers (target lock, module activate, etc.)
     */
    void setupUICallbacks();
    
    /**
     * Update game state
     * 
     * Called each frame to:
     * - Update camera position
     * - Process network messages
     * - Update entity states
     * - Handle movement commands
     * - Update UI state
     * 
     * @param deltaTime Time since last frame in seconds
     */
    void update(float deltaTime);
    
    /**
     * Render the current frame
     * 
     * Rendering order:
     * 1. Clear screen
     * 2. Render 3D scene (starfield, entities, health bars)
     * 3. Render UI overlays (panels, menus, HUD)
     * 4. Present frame to window
     */
    void render();
    
    /**
     * Cleanup and shutdown
     * 
     * Shuts down all subsystems in reverse initialization order:
     * - Disconnect from network
     * - Stop embedded server (if hosting)
     * - Cleanup OpenGL resources
     * - Destroy window
     */
    void cleanup();
    
    // Input handlers
    void handleKeyInput(int key, int action, int mods);
    void handleMouseButton(int button, int action, int mods, double x, double y);
    void handleMouseMove(double x, double y, double deltaX, double deltaY);
    void handleScroll(double xoffset, double yoffset);

    // FPS mode input handling
    void handleFPSKeyInput(int key, int action, int mods);
    bool isInFPSMode() const;
    void updateFPSMovement(float deltaTime);
    void captureFPSCursor();
    void releaseFPSCursor();
    
    // Astralis-style right-click context menu
    void showSpaceContextMenu(double x, double y);
    void showEntityContextMenu(const std::string& entityId, double x, double y);
    
    // Astralis-style movement commands
    void commandApproach(const std::string& entityId);
    void commandOrbit(const std::string& entityId, float distance = DEFAULT_ORBIT_DISTANCE);
    void commandKeepAtRange(const std::string& entityId, float distance = DEFAULT_KEEP_AT_RANGE);
    void commandAlignTo(const std::string& entityId);
    void commandWarpTo(const std::string& entityId);
    void commandJump(const std::string& entityId);
    void commandStopShip();
    
    // Spawn local player entity for offline/demo mode
    void spawnLocalPlayerEntity();
    void spawnDemoNPCEntities();
    void updateLocalMovement(float deltaTime);
    void updateTargetListUi(const glm::vec3& playerPosition);
    
    // Build info panel data for an entity
    void openInfoPanelForEntity(const std::string& entityId);

    static Application* s_instance;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<GameClient> m_gameClient;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputHandler> m_inputHandler;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<EmbeddedServer> m_embeddedServer;
    std::unique_ptr<SessionManager> m_sessionManager;
    std::unique_ptr<UI::EntityPicker> m_entityPicker;
    std::unique_ptr<SolarSystemScene> m_solarSystem;
    std::unique_ptr<ShipPhysics> m_shipPhysics;
    std::unique_ptr<atlas::AtlasContext> m_atlasCtx;
    std::unique_ptr<atlas::AtlasHUD> m_atlasHUD;
    std::unique_ptr<atlas::AtlasConsole> m_console;
    std::unique_ptr<atlas::AtlasPauseMenu> m_pauseMenu;
    std::unique_ptr<atlas::AtlasTitleScreen> m_titleScreen;
    std::unique_ptr<UI::ContextMenu> m_contextMenu;
    std::unique_ptr<UI::RadialMenu> m_radialMenu;

#ifdef NOVAFORGE_EDITOR_TOOLS
    std::unique_ptr<atlas::editor::EditorToolLayer> m_editorToolLayer;
#endif

    bool m_running;
    float m_lastFrameTime;
    float m_deltaTime = 0.016f;
    
    // Targeting state
    std::string m_currentTargetId;
    std::vector<std::string> m_targetList;
    int m_currentTargetIndex;
    
    // Astralis-style camera control state
    bool m_rightMouseDown = false;
    bool m_leftMouseDown = false;
    double m_lastMouseDragX = 0.0;
    double m_lastMouseDragY = 0.0;

    // Atlas UI mouse consumption — set after each Atlas frame, checked by
    // game-world interaction handlers to prevent click-through
    bool m_atlasConsumedMouse = false;
    
    // Astralis-style movement state
    enum class MoveCommand { None, Approach, Orbit, KeepAtRange, AlignTo, WarpTo };
    MoveCommand m_currentMoveCommand = MoveCommand::None;
    std::string m_moveTargetId;
    float m_orbitDistance = DEFAULT_ORBIT_DISTANCE;
    float m_keepAtRangeDistance = DEFAULT_KEEP_AT_RANGE;
    glm::vec3 m_playerVelocity{0.0f};
    float m_playerSpeed = 0.0f;
    float m_playerMaxSpeed = DEFAULT_PLAYER_MAX_SPEED;
    bool m_approachActive = false;
    bool m_orbitActive = false;
    bool m_keepRangeActive = false;
    
    // Context menu state
    bool m_showContextMenu = false;
    std::string m_contextMenuEntityId;
    double m_contextMenuX = 0.0;
    double m_contextMenuY = 0.0;
    
    // Radial menu state
    bool m_radialMenuOpen = false;
    double m_radialMenuStartX = 0.0;
    double m_radialMenuStartY = 0.0;
    double m_radialMenuHoldStartTime = 0.0;
    const double RADIAL_MENU_HOLD_TIME = 0.3;  // 300ms hold to open radial menu
    
    // D-key docking mode
    bool m_dockingModeActive = false;
    
    // S-key warp mode
    bool m_warpModeActive = false;
    
    // Active movement mode indicator text
    std::string m_activeModeText;
    
    // Local/demo mode
    bool m_localPlayerSpawned = false;
    std::string m_localPlayerId = "player_local";

    // ── Game-state tracking ────────────────────────────────────────
    GameState m_gameState = GameState::InSpace;

    // Station the player is currently docked at (empty when in space)
    std::string m_dockedStationId;

    // Docking animation timer (seconds remaining)
    float m_dockingTimer = 0.0f;
    static constexpr float DOCKING_ANIM_DURATION = 3.0f;

    // ── Camera defaults ────────────────────────────────────────────
    static constexpr float DEFAULT_CAMERA_FOV          = 45.0f;    // Degrees
    static constexpr float DEFAULT_CAMERA_NEAR_PLANE   = 0.1f;     // Meters
    static constexpr float DEFAULT_CAMERA_FAR_PLANE    = 10000.0f; // Meters
    static constexpr float DEFAULT_CAMERA_DISTANCE     = 200.0f;   // Initial orbit distance
    static constexpr float DEFAULT_CAMERA_PITCH        = 45.0f;    // Initial pitch degrees

    // ── Navigation defaults ────────────────────────────────────────
    static constexpr float DEFAULT_ORBIT_DISTANCE      = 500.0f;   // Meters
    static constexpr float DEFAULT_KEEP_AT_RANGE       = 2500.0f;  // Meters
    static constexpr float DEFAULT_PLAYER_MAX_SPEED    = 250.0f;   // m/s

    // ── Input thresholds ───────────────────────────────────────────
    static constexpr double MAX_DRAG_THRESHOLD_PX      = 10.0;     // Pixels — max mouse drift to still open radial menu
    static constexpr float  MAX_FPS_INTERACTION_RANGE   = 4.0f;    // Meters — max distance to interact on foot

    // ── FPS movement state ─────────────────────────────────────────
    bool  m_fpsCursorCaptured = false;    // Whether the cursor is hidden/locked for FPS look
    float m_fpsYaw   = 0.0f;             // Horizontal look angle (degrees)
    float m_fpsPitch = 0.0f;             // Vertical look angle (degrees, ±89 clamped)
    float m_fpsVelY  = 0.0f;             // Vertical velocity (jump/fall)
    bool  m_fpsGrounded = true;           // On the ground
    bool  m_fpsJumpRequested = false;     // Jump input this frame
    bool  m_fpsFlashlightOn = false;      // Flashlight toggle

    // ── FPS movement constants ─────────────────────────────────────
    static constexpr float FPS_WALK_SPEED       = 4.0f;     // m/s
    static constexpr float FPS_SPRINT_SPEED     = 7.0f;     // m/s
    static constexpr float FPS_CROUCH_SPEED     = 2.0f;     // m/s
    static constexpr float FPS_JUMP_IMPULSE     = 5.0f;     // m/s upward
    static constexpr float FPS_GRAVITY          = 9.81f;    // m/s²
    static constexpr float FPS_STAND_EYE_HEIGHT = 1.8f;     // metres
    static constexpr float FPS_CROUCH_EYE_HEIGHT= 1.0f;     // metres
};

} // namespace atlas
