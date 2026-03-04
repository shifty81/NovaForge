// Tests for: Alliance System Tests
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/alliance_system.h"

using namespace atlas;

// ========== Alliance System Tests ==========

static void testAllianceCreate() {
    std::cout << "\n=== Alliance Create ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* corpEnt = world.createEntity("corp_alpha");
    auto* corp = addComp<components::Corporation>(corpEnt);
    corp->corp_id = "corp_alpha";
    corp->corp_name = "Alpha Corp";

    assertTrue(allianceSys.createAlliance("corp_alpha", "Test Alliance", "TSTA"),
               "Alliance created");

    auto* aEnt = world.getEntity("alliance_test_alliance");
    assertTrue(aEnt != nullptr, "Alliance entity exists");

    auto* a = aEnt->getComponent<components::Alliance>();
    assertTrue(a != nullptr, "Alliance component exists");
    assertTrue(a->executor_corp_id == "corp_alpha", "Executor is creator corp");
    assertTrue(a->alliance_name == "Test Alliance", "Alliance name set");
    assertTrue(a->ticker == "TSTA", "Ticker set");
    assertTrue(a->member_corp_ids.size() == 1, "One member corp");
}

static void testAllianceJoinCorp() {
    std::cout << "\n=== Alliance Join Corp ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    auto* c2 = world.createEntity("corp_b");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_b";

    allianceSys.createAlliance("corp_a", "Join Alliance", "JNAL");

    assertTrue(allianceSys.joinAlliance("corp_b", "alliance_join_alliance"),
               "Corp B joins alliance");
    assertTrue(allianceSys.getMemberCorpCount("alliance_join_alliance") == 2,
               "Two member corps");
    assertTrue(allianceSys.isCorpInAlliance("corp_b", "alliance_join_alliance"),
               "Corp B is in alliance");
    assertTrue(!allianceSys.joinAlliance("corp_b", "alliance_join_alliance"),
               "Duplicate join rejected");
}

static void testAllianceLeaveCorp() {
    std::cout << "\n=== Alliance Leave Corp ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    auto* c2 = world.createEntity("corp_b");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_b";

    allianceSys.createAlliance("corp_a", "Leave Alliance", "LVAL");
    allianceSys.joinAlliance("corp_b", "alliance_leave_alliance");

    assertTrue(allianceSys.leaveAlliance("corp_b", "alliance_leave_alliance"),
               "Corp B leaves alliance");
    assertTrue(allianceSys.getMemberCorpCount("alliance_leave_alliance") == 1,
               "One member corp after leave");
    assertTrue(!allianceSys.isCorpInAlliance("corp_b", "alliance_leave_alliance"),
               "Corp B no longer in alliance");
}

static void testAllianceExecutorCannotLeave() {
    std::cout << "\n=== Alliance Executor Cannot Leave ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    allianceSys.createAlliance("corp_a", "Exec Alliance", "EXAL");

    assertTrue(!allianceSys.leaveAlliance("corp_a", "alliance_exec_alliance"),
               "Executor corp cannot leave");
}

static void testAllianceSetExecutor() {
    std::cout << "\n=== Alliance Set Executor ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    auto* c2 = world.createEntity("corp_b");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_b";

    allianceSys.createAlliance("corp_a", "Exec Change", "EXCH");
    allianceSys.joinAlliance("corp_b", "alliance_exec_change");

    // Non-executor cannot change executor
    assertTrue(!allianceSys.setExecutor("alliance_exec_change", "corp_b", "corp_b"),
               "Non-executor cannot change executor");

    // Executor changes executor
    assertTrue(allianceSys.setExecutor("alliance_exec_change", "corp_b", "corp_a"),
               "Executor changed to corp_b");

    auto* a = world.getEntity("alliance_exec_change")->getComponent<components::Alliance>();
    assertTrue(a->executor_corp_id == "corp_b", "New executor is corp_b");
}

static void testAllianceDisbandAlliance() {
    std::cout << "\n=== Alliance Disband ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    auto* c2 = world.createEntity("corp_b");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_b";

    allianceSys.createAlliance("corp_a", "Disband Alliance", "DSBA");
    allianceSys.joinAlliance("corp_b", "alliance_disband_alliance");

    // Non-executor cannot disband
    assertTrue(!allianceSys.disbandAlliance("alliance_disband_alliance", "corp_b"),
               "Non-executor cannot disband");

    assertTrue(allianceSys.disbandAlliance("alliance_disband_alliance", "corp_a"),
               "Executor disbands alliance");
    assertTrue(world.getEntity("alliance_disband_alliance") == nullptr,
               "Alliance entity destroyed");
}

static void testAllianceMaxCorps() {
    std::cout << "\n=== Alliance Max Corps ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_founder");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_founder";

    allianceSys.createAlliance("corp_founder", "Max Alliance", "MXAL");

    // Set max_corps to 2 for testing
    auto* a = world.getEntity("alliance_max_alliance")->getComponent<components::Alliance>();
    a->max_corps = 2;

    auto* c2 = world.createEntity("corp_two");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_two";

    assertTrue(allianceSys.joinAlliance("corp_two", "alliance_max_alliance"),
               "Second corp joins");

    auto* c3 = world.createEntity("corp_three");
    auto* corp3 = addComp<components::Corporation>(c3);
    corp3->corp_id = "corp_three";

    assertTrue(!allianceSys.joinAlliance("corp_three", "alliance_max_alliance"),
               "Third corp rejected (max 2)");
}


void run_alliance_system_tests() {
    testAllianceCreate();
    testAllianceJoinCorp();
    testAllianceLeaveCorp();
    testAllianceExecutorCannotLeave();
    testAllianceSetExecutor();
    testAllianceDisbandAlliance();
    testAllianceMaxCorps();
}
