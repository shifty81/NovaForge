// Tests for: SnapshotReplicationSystem2
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/snapshot_replication_system2.h"

using namespace atlas;

// ==================== SnapshotReplicationSystem2 Tests ====================

static void testSnapshotReplCreate() {
    std::cout << "\n=== SnapshotReplication2: Create ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("srv1");
    assertTrue(sys.initialize("srv1", "gameserver_01"), "Init succeeds");
    assertTrue(sys.getCurrentFrame("srv1") == 0, "Frame 0 initially");
    assertTrue(sys.getHistorySize("srv1") == 0, "Empty history");
    assertTrue(sys.getClientCount("srv1") == 0, "No clients");
    assertTrue(sys.getTotalSnapshotsSent("srv1") == 0, "Zero snapshots sent");
}

static void testSnapshotReplInvalidInit() {
    std::cout << "\n=== SnapshotReplication2: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    assertTrue(!sys.initialize("nonexistent", "server1"), "Missing entity fails");
}

static void testSnapshotReplCapture() {
    std::cout << "\n=== SnapshotReplication2: Capture ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", "server1");

    assertTrue(sys.captureSnapshot("srv1"), "Capture snapshot 1");
    assertTrue(sys.getCurrentFrame("srv1") == 1, "Frame 1");
    assertTrue(sys.getHistorySize("srv1") == 1, "1 in history");
    assertTrue(sys.getTotalSnapshotsSent("srv1") == 1, "1 snapshot sent");

    assertTrue(sys.captureSnapshot("srv1"), "Capture snapshot 2");
    assertTrue(sys.getCurrentFrame("srv1") == 2, "Frame 2");
    assertTrue(sys.getHistorySize("srv1") == 2, "2 in history");
}

static void testSnapshotReplAddEntity() {
    std::cout << "\n=== SnapshotReplication2: AddEntity ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", "server1");

    sys.captureSnapshot("srv1");
    assertTrue(sys.addEntityToSnapshot("srv1", "ship1", 10.0f, 20.0f, 30.0f, 100.0f, 50.0f, 5.0f),
               "Add ship1 to snapshot");
    assertTrue(sys.addEntityToSnapshot("srv1", "ship2", 40.0f, 50.0f, 60.0f, 80.0f, 30.0f, 10.0f),
               "Add ship2 to snapshot");

    // Duplicate rejected
    assertTrue(!sys.addEntityToSnapshot("srv1", "ship1", 0, 0, 0, 0, 0, 0),
               "Duplicate entity rejected");
}

static void testSnapshotReplAddEntityNoHistory() {
    std::cout << "\n=== SnapshotReplication2: AddEntityNoHistory ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", "server1");

    assertTrue(!sys.addEntityToSnapshot("srv1", "ship1", 0, 0, 0, 100, 50, 5),
               "Add fails with no history");
}

static void testSnapshotReplClientReg() {
    std::cout << "\n=== SnapshotReplication2: ClientReg ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", "server1");

    assertTrue(sys.registerClient("srv1", "client_a"), "Register client A");
    assertTrue(sys.registerClient("srv1", "client_b"), "Register client B");
    assertTrue(sys.getClientCount("srv1") == 2, "2 clients");

    // Duplicate rejected
    assertTrue(!sys.registerClient("srv1", "client_a"), "Duplicate client rejected");

    // Unregister
    assertTrue(sys.unregisterClient("srv1", "client_a"), "Unregister client A");
    assertTrue(sys.getClientCount("srv1") == 1, "1 client left");
    assertTrue(!sys.unregisterClient("srv1", "client_a"), "Double unregister fails");
}

static void testSnapshotReplAcknowledge() {
    std::cout << "\n=== SnapshotReplication2: Acknowledge ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", "server1");

    sys.registerClient("srv1", "client_a");
    sys.captureSnapshot("srv1");

    assertTrue(sys.acknowledgeFrame("srv1", "client_a", 1), "Ack frame 1");
    assertTrue(sys.getClientLastAck("srv1", "client_a") == 1, "Last ack = 1");

    // Older frame doesn't go backward
    assertTrue(sys.acknowledgeFrame("srv1", "client_a", 0), "Ack older frame (no-op)");
    assertTrue(sys.getClientLastAck("srv1", "client_a") == 1, "Last ack still 1");

    // Unknown client fails
    assertTrue(!sys.acknowledgeFrame("srv1", "unknown_client", 1), "Unknown client fails");
}

static void testSnapshotReplDeltaCount() {
    std::cout << "\n=== SnapshotReplication2: DeltaCount ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", "server1");
    sys.registerClient("srv1", "client_a");

    // Frame 1: ship1 at (0,0,0)
    sys.captureSnapshot("srv1");
    sys.addEntityToSnapshot("srv1", "ship1", 0, 0, 0, 100, 100, 0);
    sys.acknowledgeFrame("srv1", "client_a", 1);

    // Frame 2: ship1 moved
    sys.captureSnapshot("srv1");
    sys.addEntityToSnapshot("srv1", "ship1", 10, 0, 0, 100, 100, 5);

    int delta = sys.getDeltaEntityCount("srv1", "client_a");
    assertTrue(delta == 1, "1 entity changed (ship1 moved)");
}

static void testSnapshotReplDeltaNoChange() {
    std::cout << "\n=== SnapshotReplication2: DeltaNoChange ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", "server1");
    sys.registerClient("srv1", "client_a");

    // Frame 1: ship1 at (0,0,0)
    sys.captureSnapshot("srv1");
    sys.addEntityToSnapshot("srv1", "ship1", 0, 0, 0, 100, 100, 0);
    sys.acknowledgeFrame("srv1", "client_a", 1);

    // Frame 2: ship1 same position
    sys.captureSnapshot("srv1");
    sys.addEntityToSnapshot("srv1", "ship1", 0, 0, 0, 100, 100, 0);

    int delta = sys.getDeltaEntityCount("srv1", "client_a");
    assertTrue(delta == 0, "0 entities changed (no movement)");
}

static void testSnapshotReplUpdate() {
    std::cout << "\n=== SnapshotReplication2: Update ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    world.createEntity("srv1");
    sys.initialize("srv1", "server1");

    // Update should auto-capture at intervals (50ms = 20Hz)
    sys.update(0.06f); // 60ms > 50ms interval
    assertTrue(sys.getCurrentFrame("srv1") == 1, "Auto-captured frame 1");
    assertTrue(sys.getHistorySize("srv1") == 1, "1 in history after auto-capture");
}

static void testSnapshotReplMissing() {
    std::cout << "\n=== SnapshotReplication2: Missing ===" << std::endl;
    ecs::World world;
    systems::SnapshotReplicationSystem2 sys(&world);
    assertTrue(sys.getCurrentFrame("x") == 0, "Default frame on missing");
    assertTrue(sys.getHistorySize("x") == 0, "Default history on missing");
    assertTrue(sys.getClientCount("x") == 0, "Default clients on missing");
    assertTrue(sys.getTotalSnapshotsSent("x") == 0, "Default snapshots on missing");
    assertTrue(sys.getClientLastAck("x", "c") == 0, "Default ack on missing");
    assertTrue(!sys.captureSnapshot("x"), "Capture fails on missing");
}

void run_snapshot_replication_system2_tests() {
    testSnapshotReplCreate();
    testSnapshotReplInvalidInit();
    testSnapshotReplCapture();
    testSnapshotReplAddEntity();
    testSnapshotReplAddEntityNoHistory();
    testSnapshotReplClientReg();
    testSnapshotReplAcknowledge();
    testSnapshotReplDeltaCount();
    testSnapshotReplDeltaNoChange();
    testSnapshotReplUpdate();
    testSnapshotReplMissing();
}
