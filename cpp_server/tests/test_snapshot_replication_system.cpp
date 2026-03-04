// Tests for: SnapshotReplicationSystem Tests, SnapshotReplication System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/snapshot_replication_system.h"
#include "systems/snapshot_replication_system2.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== SnapshotReplicationSystem Tests ====================

static void testSnapshotDeltaFirstSendFull() {
    std::cout << "\n=== Snapshot Delta: First Send is Full ===" << std::endl;

    ecs::World world;
    systems::SnapshotReplicationSystem srs(&world);

    auto* e = world.createEntity("ship_1");
    auto* pos = addComp<components::Position>(e);
    pos->x = 100.0f; pos->y = 200.0f; pos->z = 300.0f; pos->rotation = 1.0f;
    auto* vel = addComp<components::Velocity>(e);
    vel->vx = 10.0f; vel->vy = 0.0f; vel->vz = -5.0f;

    std::string msg = srs.buildDeltaUpdate(1, 1);

    // First send must include position data
    assertTrue(msg.find("\"ship_1\"") != std::string::npos,
               "First delta includes entity id");
    assertTrue(msg.find("\"pos\"") != std::string::npos,
               "First delta includes position");
    assertTrue(msg.find("\"vel\"") != std::string::npos,
               "First delta includes velocity");
    assertTrue(msg.find("\"delta\":true") != std::string::npos,
               "Message marked as delta");
}

static void testSnapshotDeltaNoChangeEmpty() {
    std::cout << "\n=== Snapshot Delta: No Change Yields Empty Entities ===" << std::endl;

    ecs::World world;
    systems::SnapshotReplicationSystem srs(&world);

    auto* e = world.createEntity("ship_1");
    auto* pos = addComp<components::Position>(e);
    pos->x = 100.0f; pos->y = 200.0f; pos->z = 300.0f;

    // First call — full
    srs.buildDeltaUpdate(1, 1);

    // Second call — nothing changed
    std::string msg = srs.buildDeltaUpdate(1, 2);

    // Should have empty entities array since nothing changed
    assertTrue(msg.find("\"entities\":[]") != std::string::npos,
               "No-change delta has empty entities");
}

static void testSnapshotDeltaPositionChange() {
    std::cout << "\n=== Snapshot Delta: Position Change Detected ===" << std::endl;

    ecs::World world;
    systems::SnapshotReplicationSystem srs(&world);

    auto* e = world.createEntity("ship_1");
    auto* pos = addComp<components::Position>(e);
    pos->x = 100.0f; pos->y = 200.0f; pos->z = 300.0f;
    auto* hp = addComp<components::Health>(e);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;

    srs.buildDeltaUpdate(1, 1);

    // Change only position
    pos->x = 150.0f;

    std::string msg = srs.buildDeltaUpdate(1, 2);

    assertTrue(msg.find("\"pos\"") != std::string::npos,
               "Delta includes changed position");
    // Health didn't change, should be omitted
    assertTrue(msg.find("\"health\"") == std::string::npos,
               "Delta omits unchanged health");
}

static void testSnapshotDeltaHealthChange() {
    std::cout << "\n=== Snapshot Delta: Health Change Only ===" << std::endl;

    ecs::World world;
    systems::SnapshotReplicationSystem srs(&world);

    auto* e = world.createEntity("ship_1");
    auto* pos = addComp<components::Position>(e);
    pos->x = 100.0f;
    auto* hp = addComp<components::Health>(e);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 50.0f; hp->armor_max = 50.0f;
    hp->hull_hp = 200.0f; hp->hull_max = 200.0f;

    srs.buildDeltaUpdate(1, 1);

    // Only change health
    hp->shield_hp = 80.0f;

    std::string msg = srs.buildDeltaUpdate(1, 2);

    assertTrue(msg.find("\"health\"") != std::string::npos,
               "Delta includes changed health");
    assertTrue(msg.find("\"pos\"") == std::string::npos,
               "Delta omits unchanged position");
}

static void testSnapshotFullUpdateResets() {
    std::cout << "\n=== Snapshot: Full Update Resets Tracking ===" << std::endl;

    ecs::World world;
    systems::SnapshotReplicationSystem srs(&world);

    auto* e = world.createEntity("ship_1");
    auto* pos = addComp<components::Position>(e);
    pos->x = 100.0f;

    srs.buildDeltaUpdate(1, 1);

    // Full update should resend everything
    std::string msg = srs.buildFullUpdate(1, 2);

    assertTrue(msg.find("\"pos\"") != std::string::npos,
               "Full update includes position even if unchanged");
}

static void testSnapshotClearClient() {
    std::cout << "\n=== Snapshot: Clear Client ===" << std::endl;

    ecs::World world;
    systems::SnapshotReplicationSystem srs(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::Position>(e);

    srs.buildDeltaUpdate(1, 1);
    assertTrue(srs.getTrackedClientCount() == 1, "One client tracked");
    assertTrue(srs.getTrackedEntityCount(1) == 1, "One entity tracked for client");

    srs.clearClient(1);
    assertTrue(srs.getTrackedClientCount() == 0, "Client cleared");
    assertTrue(srs.getTrackedEntityCount(1) == 0, "No entities after clear");
}

static void testSnapshotEpsilonFiltering() {
    std::cout << "\n=== Snapshot: Epsilon Filtering ===" << std::endl;

    ecs::World world;
    systems::SnapshotReplicationSystem srs(&world);
    srs.setPositionEpsilon(1.0f);

    auto* e = world.createEntity("ship_1");
    auto* pos = addComp<components::Position>(e);
    pos->x = 100.0f; pos->y = 200.0f; pos->z = 300.0f;

    srs.buildDeltaUpdate(1, 1);

    // Move by less than epsilon
    pos->x = 100.5f;
    std::string msg = srs.buildDeltaUpdate(1, 2);
    assertTrue(msg.find("\"entities\":[]") != std::string::npos,
               "Sub-epsilon change filtered out");

    // Move beyond epsilon
    pos->x = 102.0f;
    msg = srs.buildDeltaUpdate(1, 3);
    assertTrue(msg.find("\"pos\"") != std::string::npos,
               "Super-epsilon change included");
}

static void testSnapshotMultipleClients() {
    std::cout << "\n=== Snapshot: Multiple Clients Independent ===" << std::endl;

    ecs::World world;
    systems::SnapshotReplicationSystem srs(&world);

    auto* e = world.createEntity("ship_1");
    auto* pos = addComp<components::Position>(e);
    pos->x = 100.0f;

    // Client 1 gets first update
    srs.buildDeltaUpdate(1, 1);

    // Client 2 has never seen the entity → should get full
    std::string msg2 = srs.buildDeltaUpdate(2, 1);
    assertTrue(msg2.find("\"pos\"") != std::string::npos,
               "Client 2 gets full state for unseen entity");

    // Client 1 gets no change
    std::string msg1 = srs.buildDeltaUpdate(1, 2);
    assertTrue(msg1.find("\"entities\":[]") != std::string::npos,
               "Client 1 gets empty delta (no change)");

    assertTrue(srs.getTrackedClientCount() == 2, "Two clients tracked");
}


// ==================== SnapshotReplication System Tests ====================

static void testSnapshotRepCreate() {
    std::cout << "\n=== SnapshotReplication: Create ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("rep1");
    assertTrue(sys.initialize("rep1", "server_main"), "Init succeeds");
    assertTrue(sys.getCurrentFrame("rep1") == 0, "Frame 0 initially");
    assertTrue(sys.getHistorySize("rep1") == 0, "Empty history");
    assertTrue(sys.getClientCount("rep1") == 0, "No clients");
}

static void testSnapshotRepCapture() {
    std::cout << "\n=== SnapshotReplication: Capture ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("rep1");
    sys.initialize("rep1", "server_main");
    assertTrue(sys.captureSnapshot("rep1"), "Capture succeeds");
    assertTrue(sys.getCurrentFrame("rep1") == 1, "Frame advanced to 1");
    assertTrue(sys.getHistorySize("rep1") == 1, "1 frame in history");
    assertTrue(sys.getTotalSnapshotsSent("rep1") == 1, "1 snapshot sent");
}

static void testSnapshotRepAddEntity() {
    std::cout << "\n=== SnapshotReplication: AddEntity ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("rep1");
    sys.initialize("rep1", "server_main");
    sys.captureSnapshot("rep1");
    assertTrue(sys.addEntityToSnapshot("rep1", "ship1", 100.0f, 200.0f, 300.0f, 100.0f, 100.0f, 50.0f), "Add entity succeeds");
    assertTrue(!sys.addEntityToSnapshot("rep1", "ship1", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), "Duplicate rejected");
}

static void testSnapshotRepRegisterClient() {
    std::cout << "\n=== SnapshotReplication: RegisterClient ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("rep1");
    sys.initialize("rep1", "server_main");
    assertTrue(sys.registerClient("rep1", "client1"), "Register client succeeds");
    assertTrue(sys.getClientCount("rep1") == 1, "1 client");
    assertTrue(!sys.registerClient("rep1", "client1"), "Duplicate client rejected");
    assertTrue(sys.registerClient("rep1", "client2"), "Second client succeeds");
    assertTrue(sys.getClientCount("rep1") == 2, "2 clients");
}

static void testSnapshotRepUnregisterClient() {
    std::cout << "\n=== SnapshotReplication: UnregisterClient ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("rep1");
    sys.initialize("rep1", "server_main");
    sys.registerClient("rep1", "client1");
    assertTrue(sys.unregisterClient("rep1", "client1"), "Unregister succeeds");
    assertTrue(sys.getClientCount("rep1") == 0, "0 clients after unregister");
    assertTrue(!sys.unregisterClient("rep1", "client1"), "Unregister nonexistent fails");
}

static void testSnapshotRepAcknowledge() {
    std::cout << "\n=== SnapshotReplication: Acknowledge ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("rep1");
    sys.initialize("rep1", "server_main");
    sys.registerClient("rep1", "client1");
    sys.captureSnapshot("rep1");
    assertTrue(sys.acknowledgeFrame("rep1", "client1", 1), "Ack succeeds");
    assertTrue(sys.getClientLastAck("rep1", "client1") == 1, "Last ack is frame 1");
    // Old frame ack should not regress
    assertTrue(sys.acknowledgeFrame("rep1", "client1", 0), "Old ack accepted");
    assertTrue(sys.getClientLastAck("rep1", "client1") == 1, "Last ack still frame 1");
}

static void testSnapshotRepDelta() {
    std::cout << "\n=== SnapshotReplication: Delta ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("rep1");
    sys.initialize("rep1", "server_main");
    sys.registerClient("rep1", "client1");
    // Frame 1: add ship1
    sys.captureSnapshot("rep1");
    sys.addEntityToSnapshot("rep1", "ship1", 100.0f, 200.0f, 300.0f, 100.0f, 100.0f, 50.0f);
    sys.acknowledgeFrame("rep1", "client1", 1);
    // Frame 2: ship1 moves
    sys.captureSnapshot("rep1");
    sys.addEntityToSnapshot("rep1", "ship1", 110.0f, 210.0f, 310.0f, 100.0f, 100.0f, 50.0f);
    int delta = sys.getDeltaEntityCount("rep1", "client1");
    assertTrue(delta == 1, "1 entity changed in delta");
}

static void testSnapshotRepAutoCapture() {
    std::cout << "\n=== SnapshotReplication: AutoCapture ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("rep1");
    sys.initialize("rep1", "server_main");
    // Default interval 0.05s (20Hz)
    sys.update(0.05f);
    assertTrue(sys.getCurrentFrame("rep1") == 1, "Auto-captured frame 1");
    sys.update(0.05f);
    assertTrue(sys.getCurrentFrame("rep1") == 2, "Auto-captured frame 2");
    assertTrue(sys.getTotalSnapshotsSent("rep1") == 2, "2 snapshots sent");
}

static void testSnapshotRepHistoryLimit() {
    std::cout << "\n=== SnapshotReplication: HistoryLimit ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("rep1");
    sys.initialize("rep1", "server_main");
    auto* entity = world.getEntity("rep1");
    auto* sr = entity->getComponent<components::SnapshotReplication>();
    sr->max_history = 3;
    for (int i = 0; i < 5; i++) sys.captureSnapshot("rep1");
    assertTrue(sys.getHistorySize("rep1") == 3, "History trimmed to max");
    assertTrue(sys.getCurrentFrame("rep1") == 5, "Frame counter at 5");
}

static void testSnapshotRepMissing() {
    std::cout << "\n=== SnapshotReplication: Missing ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    assertTrue(!sys.initialize("nonexistent", "s1"), "Init fails on missing");
    assertTrue(!sys.captureSnapshot("nonexistent"), "Capture fails on missing");
    assertTrue(!sys.addEntityToSnapshot("nonexistent", "e1", 0, 0, 0, 0, 0, 0), "Add entity fails on missing");
    assertTrue(!sys.registerClient("nonexistent", "c1"), "Register fails on missing");
    assertTrue(!sys.unregisterClient("nonexistent", "c1"), "Unregister fails on missing");
    assertTrue(!sys.acknowledgeFrame("nonexistent", "c1", 1), "Ack fails on missing");
    assertTrue(sys.getCurrentFrame("nonexistent") == 0, "0 frame on missing");
    assertTrue(sys.getHistorySize("nonexistent") == 0, "0 history on missing");
    assertTrue(sys.getClientCount("nonexistent") == 0, "0 clients on missing");
    assertTrue(sys.getTotalSnapshotsSent("nonexistent") == 0, "0 sent on missing");
}


void run_snapshot_replication_system_tests() {
    testSnapshotDeltaFirstSendFull();
    testSnapshotDeltaNoChangeEmpty();
    testSnapshotDeltaPositionChange();
    testSnapshotDeltaHealthChange();
    testSnapshotFullUpdateResets();
    testSnapshotClearClient();
    testSnapshotEpsilonFiltering();
    testSnapshotMultipleClients();
    testSnapshotRepCreate();
    testSnapshotRepCapture();
    testSnapshotRepAddEntity();
    testSnapshotRepRegisterClient();
    testSnapshotRepUnregisterClient();
    testSnapshotRepAcknowledge();
    testSnapshotRepDelta();
    testSnapshotRepAutoCapture();
    testSnapshotRepHistoryLimit();
    testSnapshotRepMissing();
}
