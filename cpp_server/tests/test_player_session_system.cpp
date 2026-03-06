// Tests for: Player Session System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/player_session_system.h"

using namespace atlas;

// ==================== Player Session System Tests ====================

static void testPlayerSessionCreate() {
    std::cout << "\n=== PlayerSession: Create ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_001", "TestPilot"), "Init succeeds");
    assertTrue(sys.getCharacterSlotCount("p1") == 0, "No character slots initially");
    assertTrue(sys.getSessionState("p1") == 0, "State is Disconnected");
    assertTrue(approxEqual(sys.getSessionDuration("p1"), 0.0f), "No session duration");
    assertTrue(approxEqual(sys.getTotalPlayTime("p1"), 0.0f), "No total play time");
    assertTrue(sys.getLoginCount("p1") == 0, "No logins");
    assertTrue(sys.getDisconnectCount("p1") == 0, "No disconnects");
    assertTrue(sys.getReconnectCount("p1") == 0, "No reconnects");
    assertTrue(sys.getSelectedCharacter("p1").empty(), "No selected character");
}

static void testPlayerSessionConnect() {
    std::cout << "\n=== PlayerSession: Connect ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001", "TestPilot");
    assertTrue(sys.connect("p1"), "Connect succeeds");
    assertTrue(sys.getSessionState("p1") == 2, "State is CharacterSelect");
    assertTrue(sys.getLoginCount("p1") == 1, "1 login");
    assertTrue(!sys.connect("p1"), "Double connect fails");
}

static void testPlayerSessionDisconnect() {
    std::cout << "\n=== PlayerSession: Disconnect ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001", "TestPilot");
    assertTrue(!sys.disconnect("p1"), "Disconnect while already disconnected fails");
    sys.connect("p1");
    assertTrue(sys.disconnect("p1"), "Disconnect succeeds");
    assertTrue(sys.getSessionState("p1") == 0, "State is Disconnected");
    assertTrue(sys.getDisconnectCount("p1") == 1, "1 disconnect");
}

static void testPlayerSessionCharacterSlots() {
    std::cout << "\n=== PlayerSession: CharacterSlots ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001", "TestPilot");
    assertTrue(sys.addCharacterSlot("p1", "char1", "Pilot Alpha", "Rifter", "Jita"), "Add slot 1");
    assertTrue(sys.addCharacterSlot("p1", "char2", "Pilot Beta", "Merlin", "Amarr"), "Add slot 2");
    assertTrue(sys.getCharacterSlotCount("p1") == 2, "2 slots");
    assertTrue(!sys.addCharacterSlot("p1", "char1", "Dup", "Dup", "Dup"), "Duplicate rejected");
    assertTrue(sys.removeCharacterSlot("p1", "char1"), "Remove slot");
    assertTrue(sys.getCharacterSlotCount("p1") == 1, "1 slot after remove");
    assertTrue(!sys.removeCharacterSlot("p1", "char1"), "Double remove fails");
}

static void testPlayerSessionCharacterSlotMax() {
    std::cout << "\n=== PlayerSession: CharacterSlotMax ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001", "TestPilot");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::PlayerSession>();
    state->max_character_slots = 2;
    sys.addCharacterSlot("p1", "c1", "A", "Rifter", "Jita");
    sys.addCharacterSlot("p1", "c2", "B", "Merlin", "Amarr");
    assertTrue(!sys.addCharacterSlot("p1", "c3", "C", "Kestrel", "Dodixie"), "Max slots enforced");
}

static void testPlayerSessionSelectCharacter() {
    std::cout << "\n=== PlayerSession: SelectCharacter ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001", "TestPilot");
    sys.addCharacterSlot("p1", "char1", "Pilot Alpha", "Rifter", "Jita");
    sys.connect("p1");
    assertTrue(sys.selectCharacter("p1", "char1"), "Select character succeeds");
    assertTrue(sys.getSelectedCharacter("p1") == "char1", "Character selected");
    assertTrue(sys.getSessionState("p1") == 3, "State is Loading");
    assertTrue(!sys.selectCharacter("p1", "char1"), "Select while Loading fails");
}

static void testPlayerSessionEnterGame() {
    std::cout << "\n=== PlayerSession: EnterGame ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001", "TestPilot");
    sys.addCharacterSlot("p1", "char1", "Pilot Alpha", "Rifter", "Jita");
    sys.connect("p1");
    assertTrue(!sys.enterGame("p1"), "EnterGame before select fails");
    sys.selectCharacter("p1", "char1");
    assertTrue(sys.enterGame("p1"), "EnterGame succeeds");
    assertTrue(sys.getSessionState("p1") == 4, "State is InGame");
    assertTrue(!sys.enterGame("p1"), "Double enterGame fails");
}

static void testPlayerSessionReconnect() {
    std::cout << "\n=== PlayerSession: Reconnect ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001", "TestPilot");
    sys.addCharacterSlot("p1", "char1", "Pilot Alpha", "Rifter", "Jita");
    sys.connect("p1");
    sys.selectCharacter("p1", "char1");
    sys.enterGame("p1");
    sys.disconnect("p1");
    assertTrue(sys.reconnect("p1"), "Reconnect succeeds");
    assertTrue(sys.getSessionState("p1") == 5, "State is Reconnecting");
    assertTrue(sys.getReconnectCount("p1") == 1, "1 reconnect");
    assertTrue(sys.selectCharacter("p1", "char1"), "Select after reconnect works");
}

static void testPlayerSessionHeartbeat() {
    std::cout << "\n=== PlayerSession: Heartbeat ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001", "TestPilot");
    assertTrue(!sys.heartbeat("p1", 1.0f), "Heartbeat while disconnected fails");
    sys.connect("p1");
    assertTrue(sys.heartbeat("p1", 5.0f), "Heartbeat while connected succeeds");
}

static void testPlayerSessionUpdate() {
    std::cout << "\n=== PlayerSession: Update ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001", "TestPilot");
    sys.addCharacterSlot("p1", "char1", "Pilot Alpha", "Rifter", "Jita");
    sys.connect("p1");
    sys.selectCharacter("p1", "char1");
    sys.enterGame("p1");
    sys.update(1.0f);
    sys.update(2.5f);
    assertTrue(approxEqual(sys.getSessionDuration("p1"), 3.5f), "Session duration 3.5s");
    assertTrue(approxEqual(sys.getTotalPlayTime("p1"), 3.5f), "Total play time 3.5s");
}

static void testPlayerSessionMissing() {
    std::cout << "\n=== PlayerSession: Missing ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x", "x"), "Init fails on missing");
    assertTrue(!sys.connect("nonexistent"), "Connect fails on missing");
    assertTrue(!sys.disconnect("nonexistent"), "Disconnect fails on missing");
    assertTrue(!sys.reconnect("nonexistent"), "Reconnect fails on missing");
    assertTrue(!sys.addCharacterSlot("nonexistent", "c", "n", "s", "l"), "addCharacterSlot fails");
    assertTrue(!sys.removeCharacterSlot("nonexistent", "c"), "removeCharacterSlot fails");
    assertTrue(!sys.selectCharacter("nonexistent", "c"), "selectCharacter fails");
    assertTrue(!sys.enterGame("nonexistent"), "enterGame fails");
    assertTrue(!sys.heartbeat("nonexistent", 1.0f), "heartbeat fails");
    assertTrue(sys.getCharacterSlotCount("nonexistent") == 0, "0 slots");
    assertTrue(sys.getSessionState("nonexistent") == -1, "-1 state");
    assertTrue(approxEqual(sys.getSessionDuration("nonexistent"), 0.0f), "0 duration");
    assertTrue(approxEqual(sys.getTotalPlayTime("nonexistent"), 0.0f), "0 play time");
    assertTrue(sys.getLoginCount("nonexistent") == 0, "0 logins");
    assertTrue(sys.getDisconnectCount("nonexistent") == 0, "0 disconnects");
    assertTrue(sys.getReconnectCount("nonexistent") == 0, "0 reconnects");
    assertTrue(sys.getSelectedCharacter("nonexistent").empty(), "empty selected");
}

void run_player_session_system_tests() {
    testPlayerSessionCreate();
    testPlayerSessionConnect();
    testPlayerSessionDisconnect();
    testPlayerSessionCharacterSlots();
    testPlayerSessionCharacterSlotMax();
    testPlayerSessionSelectCharacter();
    testPlayerSessionEnterGame();
    testPlayerSessionReconnect();
    testPlayerSessionHeartbeat();
    testPlayerSessionUpdate();
    testPlayerSessionMissing();
}
