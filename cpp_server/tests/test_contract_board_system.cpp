// Tests for: Contract Board System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/contract_board_system.h"

using namespace atlas;

// ==================== Contract Board System Tests ====================

static void testContractBoardCreate() {
    std::cout << "\n=== ContractBoard: Create ===" << std::endl;
    ecs::World world;
    systems::ContractBoardSystem sys(&world);
    world.createEntity("station1");
    assertTrue(sys.initialize("station1"), "Init succeeds");
    assertTrue(sys.getContractCount("station1") == 0, "0 contracts");
    assertTrue(sys.getOutstandingCount("station1") == 0, "0 outstanding");
    assertTrue(sys.getCompletedCount("station1") == 0, "0 completed");
    assertTrue(sys.getExpiredCount("station1") == 0, "0 expired");
}

static void testContractBoardPost() {
    std::cout << "\n=== ContractBoard: Post ===" << std::endl;
    ecs::World world;
    systems::ContractBoardSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    assertTrue(sys.postContract("station1", "c001", "npc_trader", "courier", 5000.0, 3.0f),
               "Post contract");
    assertTrue(sys.getContractCount("station1") == 1, "1 contract");
    assertTrue(sys.getOutstandingCount("station1") == 1, "1 outstanding");
    assertTrue(sys.getContractStatus("station1", "c001") == "outstanding", "Status outstanding");

    // Duplicate ID rejected
    assertTrue(!sys.postContract("station1", "c001", "npc_trader", "courier", 1000.0, 1.0f),
               "Duplicate rejected");
}

static void testContractBoardAcceptComplete() {
    std::cout << "\n=== ContractBoard: AcceptComplete ===" << std::endl;
    ecs::World world;
    systems::ContractBoardSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    sys.postContract("station1", "c001", "npc_trader", "item_exchange", 2000.0, 7.0f);
    assertTrue(sys.acceptContract("station1", "c001", "player1"), "Accept contract");
    assertTrue(sys.getContractStatus("station1", "c001") == "in_progress", "Status in_progress");
    assertTrue(sys.getOutstandingCount("station1") == 0, "0 outstanding after accept");

    // Can't accept again
    assertTrue(!sys.acceptContract("station1", "c001", "player2"),
               "Double accept rejected");

    assertTrue(sys.completeContract("station1", "c001"), "Complete contract");
    assertTrue(sys.getContractStatus("station1", "c001") == "completed", "Status completed");
    assertTrue(sys.getCompletedCount("station1") == 1, "1 completed");
}

static void testContractBoardFail() {
    std::cout << "\n=== ContractBoard: Fail ===" << std::endl;
    ecs::World world;
    systems::ContractBoardSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    sys.postContract("station1", "c001", "npc_miner", "courier", 1500.0, 2.0f);
    sys.acceptContract("station1", "c001", "player1");
    assertTrue(sys.failContract("station1", "c001"), "Fail contract");
    assertTrue(sys.getContractStatus("station1", "c001") == "failed", "Status failed");

    // Can't fail a contract that isn't in progress
    assertTrue(!sys.failContract("station1", "c001"), "Can't fail already failed");
}

static void testContractBoardExpiry() {
    std::cout << "\n=== ContractBoard: Expiry ===" << std::endl;
    ecs::World world;
    systems::ContractBoardSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1");

    // Post contract with 1 day duration (86400 seconds)
    sys.postContract("station1", "c001", "npc_trader", "auction", 10000.0, 1.0f);
    assertTrue(sys.getOutstandingCount("station1") == 1, "1 outstanding");

    // Tick 86400 seconds (1 day)
    for (int i = 0; i < 864; i++) sys.update(100.0f);
    assertTrue(sys.getExpiredCount("station1") == 1, "1 expired");
    assertTrue(sys.getOutstandingCount("station1") == 0, "0 outstanding after expiry");
    assertTrue(sys.getContractStatus("station1", "c001") == "expired", "Status expired");
}

static void testContractBoardMissing() {
    std::cout << "\n=== ContractBoard: Missing ===" << std::endl;
    ecs::World world;
    systems::ContractBoardSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.postContract("nonexistent", "c1", "npc", "courier", 100.0, 1.0f),
               "Post fails on missing");
    assertTrue(!sys.acceptContract("nonexistent", "c1", "p1"), "Accept fails on missing");
    assertTrue(!sys.completeContract("nonexistent", "c1"), "Complete fails on missing");
    assertTrue(!sys.failContract("nonexistent", "c1"), "Fail fails on missing");
    assertTrue(sys.getContractCount("nonexistent") == 0, "0 contracts on missing");
    assertTrue(sys.getOutstandingCount("nonexistent") == 0, "0 outstanding on missing");
    assertTrue(sys.getCompletedCount("nonexistent") == 0, "0 completed on missing");
    assertTrue(sys.getExpiredCount("nonexistent") == 0, "0 expired on missing");
    assertTrue(sys.getContractStatus("nonexistent", "c1") == "unknown", "Unknown status on missing");
}

void run_contract_board_system_tests() {
    testContractBoardCreate();
    testContractBoardPost();
    testContractBoardAcceptComplete();
    testContractBoardFail();
    testContractBoardExpiry();
    testContractBoardMissing();
}
