// Tests for: SessionManagerSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/session_manager_system.h"

using namespace atlas;

// ==================== SessionManagerSystem Tests ====================

static void testSessionManagerInit() {
    std::cout << "\n=== SessionManager: Init ===" << std::endl;
    ecs::World world;
    systems::SessionManagerSystem sys(&world);
    world.createEntity("s1");
    assertTrue(sys.createSession("s1", "player_42", "SpacePilot"), "Create session succeeds");
    assertTrue(sys.getPhase("s1") == components::SessionState::Phase::Disconnected,
               "Initial phase is Disconnected");
    assertTrue(approxEqual(sys.getSessionDuration("s1"), 0.0f), "Duration starts at 0");
    assertTrue(sys.getLoginCount("s1") == 0, "Login count starts at 0");
    assertTrue(sys.getSpawnLocation("s1").empty(), "Spawn location starts empty");

    // Verify stored fields
    auto* e = world.getEntity("s1");
    auto* comp = e->getComponent<components::SessionState>();
    assertTrue(comp->player_id == "player_42", "player_id stored");
    assertTrue(comp->character_name == "SpacePilot", "character_name stored");
    assertTrue(!comp->is_new_player, "is_new_player defaults to false");
}

static void testSessionManagerAuthenticate() {
    std::cout << "\n=== SessionManager: Authenticate ===" << std::endl;
    ecs::World world;
    systems::SessionManagerSystem sys(&world);
    world.createEntity("s1");
    sys.createSession("s1", "player_1", "Ace");

    assertTrue(sys.authenticate("s1"), "Authenticate succeeds from Disconnected");
    assertTrue(sys.getPhase("s1") == components::SessionState::Phase::Loading,
               "Phase moves to Loading after authenticate");

    // Cannot authenticate again from Loading
    assertTrue(!sys.authenticate("s1"), "Double authenticate rejected");
    assertTrue(sys.getPhase("s1") == components::SessionState::Phase::Loading,
               "Phase unchanged after rejected authenticate");
}

static void testSessionManagerActivate() {
    std::cout << "\n=== SessionManager: Activate ===" << std::endl;
    ecs::World world;
    systems::SessionManagerSystem sys(&world);
    world.createEntity("s1");
    sys.createSession("s1", "player_1", "Ace");
    sys.authenticate("s1");

    assertTrue(sys.activateSession("s1"), "Activate succeeds from Loading");
    assertTrue(sys.getPhase("s1") == components::SessionState::Phase::Active,
               "Phase is Active");
    assertTrue(sys.getLoginCount("s1") == 1, "Login count incremented to 1");

    // Cannot activate again from Active
    assertTrue(!sys.activateSession("s1"), "Double activate rejected");
    assertTrue(sys.getLoginCount("s1") == 1, "Login count unchanged after rejected activate");
}

static void testSessionManagerDisconnect() {
    std::cout << "\n=== SessionManager: Disconnect ===" << std::endl;
    ecs::World world;
    systems::SessionManagerSystem sys(&world);
    world.createEntity("s1");
    sys.createSession("s1", "player_1", "Ace");
    sys.authenticate("s1");
    sys.activateSession("s1");

    assertTrue(sys.beginDisconnect("s1"), "Begin disconnect succeeds from Active");
    assertTrue(sys.getPhase("s1") == components::SessionState::Phase::Disconnecting,
               "Phase is Disconnecting");

    // Cannot disconnect again
    assertTrue(!sys.beginDisconnect("s1"), "Double disconnect rejected");

    // Cannot disconnect from non-Active phase
    world.createEntity("s2");
    sys.createSession("s2", "p2", "Bob");
    assertTrue(!sys.beginDisconnect("s2"), "Disconnect from Disconnected rejected");
}

static void testSessionManagerHeartbeat() {
    std::cout << "\n=== SessionManager: Heartbeat ===" << std::endl;
    ecs::World world;
    systems::SessionManagerSystem sys(&world);
    world.createEntity("s1");
    sys.createSession("s1", "player_1", "Ace");
    sys.authenticate("s1");
    sys.activateSession("s1");

    // Tick to accumulate idle time
    sys.update(60.0f);
    auto* comp = world.getEntity("s1")->getComponent<components::SessionState>();
    assertTrue(approxEqual(comp->idle_timer, 60.0f), "Idle timer at 60s before heartbeat");

    assertTrue(sys.heartbeat("s1"), "Heartbeat succeeds");
    assertTrue(approxEqual(comp->idle_timer, 0.0f), "Idle timer reset to 0 after heartbeat");
    assertTrue(approxEqual(comp->last_heartbeat, 60.0f), "last_heartbeat recorded");

    // Heartbeat on missing entity
    assertTrue(!sys.heartbeat("nonexistent"), "Heartbeat fails on missing entity");
}

static void testSessionManagerIdleTimeout() {
    std::cout << "\n=== SessionManager: IdleTimeout ===" << std::endl;
    ecs::World world;
    systems::SessionManagerSystem sys(&world);
    world.createEntity("s1");
    sys.createSession("s1", "player_1", "Ace");
    sys.authenticate("s1");
    sys.activateSession("s1");

    // Set a short timeout for testing
    auto* comp = world.getEntity("s1")->getComponent<components::SessionState>();
    comp->idle_timeout = 10.0f;

    assertTrue(!sys.isIdle("s1"), "Not idle initially");

    sys.update(5.0f);
    assertTrue(!sys.isIdle("s1"), "Not idle after 5s");
    assertTrue(sys.getPhase("s1") == components::SessionState::Phase::Active,
               "Still Active after 5s");

    // Tick past timeout — auto-disconnect
    sys.update(6.0f);
    assertTrue(sys.getPhase("s1") == components::SessionState::Phase::Disconnecting,
               "Auto-disconnect after idle timeout");
    assertTrue(approxEqual(sys.getSessionDuration("s1"), 11.0f), "Duration accumulated correctly");
}

static void testSessionManagerSpawnLocation() {
    std::cout << "\n=== SessionManager: SpawnLocation ===" << std::endl;
    ecs::World world;
    systems::SessionManagerSystem sys(&world);
    world.createEntity("s1");
    sys.createSession("s1", "player_1", "Ace");

    assertTrue(sys.setSpawnLocation("s1", "station"), "Set spawn location to station");
    assertTrue(sys.getSpawnLocation("s1") == "station", "Spawn location is station");

    assertTrue(sys.setSpawnLocation("s1", "space"), "Update spawn location to space");
    assertTrue(sys.getSpawnLocation("s1") == "space", "Spawn location is space");

    // Empty location rejected
    assertTrue(!sys.setSpawnLocation("s1", ""), "Empty location rejected");
    assertTrue(sys.getSpawnLocation("s1") == "space", "Spawn location unchanged after rejection");

    // Missing entity
    assertTrue(sys.getSpawnLocation("nonexistent").empty(), "Empty location for missing entity");
}

static void testSessionManagerMissingEntity() {
    std::cout << "\n=== SessionManager: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::SessionManagerSystem sys(&world);

    assertTrue(!sys.createSession("nonexistent", "p1", "Bob"), "Create fails on missing entity");
    assertTrue(!sys.authenticate("nonexistent"), "Authenticate fails on missing");
    assertTrue(!sys.activateSession("nonexistent"), "Activate fails on missing");
    assertTrue(!sys.beginDisconnect("nonexistent"), "Disconnect fails on missing");
    assertTrue(sys.getPhase("nonexistent") == components::SessionState::Phase::Disconnected,
               "Phase Disconnected on missing");
    assertTrue(approxEqual(sys.getSessionDuration("nonexistent"), 0.0f), "Duration 0 on missing");
    assertTrue(sys.getLoginCount("nonexistent") == 0, "Login count 0 on missing");
    assertTrue(!sys.heartbeat("nonexistent"), "Heartbeat fails on missing");
    assertTrue(!sys.isIdle("nonexistent"), "Not idle on missing");
    assertTrue(!sys.setSpawnLocation("nonexistent", "station"), "Set spawn fails on missing");
    assertTrue(sys.getSpawnLocation("nonexistent").empty(), "Empty spawn on missing");
}

void run_session_manager_system_tests() {
    testSessionManagerInit();
    testSessionManagerAuthenticate();
    testSessionManagerActivate();
    testSessionManagerDisconnect();
    testSessionManagerHeartbeat();
    testSessionManagerIdleTimeout();
    testSessionManagerSpawnLocation();
    testSessionManagerMissingEntity();
}
