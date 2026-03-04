// Tests for: ContractSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "data/world_persistence.h"
#include "systems/contract_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== ContractSystem Tests ====================

static void testContractCreate() {
    std::cout << "\n=== Contract Create ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    assertTrue(contractSys.createContract("station_1", "player_1", "item_exchange", 50000.0, 3600.0f),
               "Contract created successfully");
    assertTrue(contractSys.getActiveContractCount("station_1") == 1, "Active contract count is 1");
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 1,
               "Outstanding contract count is 1");
}

static void testContractAccept() {
    std::cout << "\n=== Contract Accept ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    contractSys.createContract("station_1", "player_1", "courier", 100000.0, -1.0f);
    auto* board = station->getComponent<components::ContractBoard>();
    std::string cid = board->contracts[0].contract_id;

    assertTrue(contractSys.acceptContract("station_1", cid, "player_2"),
               "Contract accepted");
    assertTrue(board->contracts[0].status == "in_progress", "Status changed to in_progress");
    assertTrue(board->contracts[0].assignee_id == "player_2", "Assignee set correctly");
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 0,
               "No outstanding contracts after accept");
    assertTrue(contractSys.getContractsByStatus("station_1", "in_progress") == 1,
               "One in_progress contract after accept");
}

static void testContractComplete() {
    std::cout << "\n=== Contract Complete ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    auto* acceptor = world.createEntity("player_2");
    auto* player = addComp<components::Player>(acceptor);
    player->credits = 10000.0;

    contractSys.createContract("station_1", "player_1", "item_exchange", 75000.0, -1.0f);
    auto* board = station->getComponent<components::ContractBoard>();
    std::string cid = board->contracts[0].contract_id;

    contractSys.acceptContract("station_1", cid, "player_2");
    assertTrue(contractSys.completeContract("station_1", cid), "Contract completed");
    assertTrue(board->contracts[0].status == "completed", "Status is completed");
    assertTrue(approxEqual(static_cast<float>(player->credits), 85000.0f), "Credits reward paid to acceptor");
}

static void testContractExpiry() {
    std::cout << "\n=== Contract Expiry ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    contractSys.createContract("station_1", "player_1", "auction", 0.0, 10.0f);

    contractSys.update(5.0f);
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 1,
               "Contract still outstanding at 5s");

    contractSys.update(6.0f);
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 0,
               "No outstanding contracts after 11s");
    assertTrue(contractSys.getContractsByStatus("station_1", "expired") == 1,
               "Contract expired after 11s");
}

static void testContractStatusQuery() {
    std::cout << "\n=== Contract Status Query ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    contractSys.createContract("station_1", "p1", "item_exchange", 1000.0, -1.0f);
    contractSys.createContract("station_1", "p2", "courier", 2000.0, 5.0f);
    contractSys.createContract("station_1", "p3", "auction", 3000.0, -1.0f);

    auto* board = station->getComponent<components::ContractBoard>();
    contractSys.acceptContract("station_1", board->contracts[0].contract_id, "buyer_1");
    contractSys.completeContract("station_1", board->contracts[0].contract_id);

    contractSys.update(6.0f); // expire the second contract

    assertTrue(contractSys.getContractsByStatus("station_1", "completed") == 1,
               "1 completed contract");
    assertTrue(contractSys.getContractsByStatus("station_1", "expired") == 1,
               "1 expired contract");
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 1,
               "1 outstanding contract");
    assertTrue(contractSys.getActiveContractCount("station_1") == 1,
               "1 active contract (outstanding only)");
}

static void testSerializeDeserializeContractBoard() {
    std::cout << "\n=== Serialize/Deserialize ContractBoard ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("board_test");
    auto* board = addComp<components::ContractBoard>(entity);

    components::ContractBoard::Contract c;
    c.contract_id = "contract_p1_0";
    c.issuer_id = "p1";
    c.assignee_id = "p2";
    c.type = "courier";
    c.status = "in_progress";
    c.isk_reward = 50000.0;
    c.isk_collateral = 10000.0;
    c.duration_remaining = 100.0f;
    c.days_to_complete = 7.0f;

    components::ContractBoard::ContractItem offered;
    offered.item_id = "trit"; offered.name = "Stellium";
    offered.quantity = 500; offered.volume = 0.01f;
    c.items_offered.push_back(offered);

    components::ContractBoard::ContractItem requested;
    requested.item_id = "pye"; requested.name = "Vanthium";
    requested.quantity = 100; requested.volume = 0.01f;
    c.items_requested.push_back(requested);

    board->contracts.push_back(c);

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "ContractBoard deserialization succeeds");

    auto* e2 = world2.getEntity("board_test");
    assertTrue(e2 != nullptr, "Board entity recreated");

    auto* board2 = e2->getComponent<components::ContractBoard>();
    assertTrue(board2 != nullptr, "ContractBoard component recreated");
    assertTrue(board2->contracts.size() == 1, "Contract count preserved");
    assertTrue(board2->contracts[0].contract_id == "contract_p1_0", "contract_id preserved");
    assertTrue(board2->contracts[0].issuer_id == "p1", "issuer_id preserved");
    assertTrue(board2->contracts[0].assignee_id == "p2", "assignee_id preserved");
    assertTrue(board2->contracts[0].type == "courier", "type preserved");
    assertTrue(board2->contracts[0].status == "in_progress", "status preserved");
    assertTrue(approxEqual(static_cast<float>(board2->contracts[0].isk_reward), 50000.0f), "isk_reward preserved");
    assertTrue(approxEqual(static_cast<float>(board2->contracts[0].isk_collateral), 10000.0f), "isk_collateral preserved");
    assertTrue(approxEqual(board2->contracts[0].duration_remaining, 100.0f), "duration_remaining preserved");
    assertTrue(approxEqual(board2->contracts[0].days_to_complete, 7.0f), "days_to_complete preserved");
    assertTrue(board2->contracts[0].items_offered.size() == 1, "items_offered count preserved");
    assertTrue(board2->contracts[0].items_offered[0].item_id == "trit", "offered item_id preserved");
    assertTrue(board2->contracts[0].items_offered[0].quantity == 500, "offered quantity preserved");
    assertTrue(board2->contracts[0].items_requested.size() == 1, "items_requested count preserved");
    assertTrue(board2->contracts[0].items_requested[0].item_id == "pye", "requested item_id preserved");
}


void run_contract_system_tests() {
    testContractCreate();
    testContractAccept();
    testContractComplete();
    testContractExpiry();
    testContractStatusQuery();
    testSerializeDeserializeContractBoard();
}
