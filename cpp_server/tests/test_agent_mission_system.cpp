// Tests for: AgentMissionSystem
#include "test_log.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/agent_mission_system.h"

using namespace atlas;
using MissionStatus = components::AgentMissionState::MissionStatus;
using MissionType   = components::AgentMissionState::MissionType;

static void testAgentMissionInit() {
    std::cout << "\n=== AgentMission: Init ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(sys.getMissionCount("e1") == 0, "No missions after init");
    assertTrue(sys.getActiveMissionCount("e1") == 0, "No active missions after init");
    assertTrue(approxEqual(sys.getTotalIskEarned("e1"), 0.0f), "No ISK earned after init");
    assertTrue(sys.getTotalLpEarned("e1") == 0, "No LP earned after init");
    assertTrue(sys.getTotalCompleted("e1") == 0, "No completed after init");
    assertTrue(sys.getTotalFailed("e1") == 0, "No failed after init");
    assertTrue(sys.getTotalExpired("e1") == 0, "No expired after init");
}

static void testAgentMissionOffer() {
    std::cout << "\n=== AgentMission: Offer ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    assertTrue(sys.offerMission("e1", "m1", "Kill pirates", "agent1", MissionType::Combat, 500000.0f, 100, 3600.0f), "Offer mission");
    assertTrue(sys.getMissionCount("e1") == 1, "1 mission count");
    assertTrue(sys.hasMission("e1", "m1"), "Has mission m1");
    assertTrue(sys.getMissionStatus("e1", "m1") == MissionStatus::Offered, "Status is Offered");
    assertTrue(!sys.offerMission("e1", "m1", "Dup", "agent1", MissionType::Combat, 100.0f, 10, 100.0f), "Duplicate rejected");
    assertTrue(!sys.offerMission("e1", "", "Name", "agent1", MissionType::Courier, 100.0f, 10, 100.0f), "Empty mission_id rejected");
    assertTrue(!sys.offerMission("e1", "m2", "", "agent1", MissionType::Courier, 100.0f, 10, 100.0f), "Empty mission_name rejected");
    assertTrue(!sys.offerMission("e1", "m2", "Name", "", MissionType::Courier, 100.0f, 10, 100.0f), "Empty agent_id rejected");
    assertTrue(!sys.offerMission("e1", "m2", "Name", "agent1", MissionType::Courier, 0.0f, 10, 100.0f), "Zero isk rejected");
    assertTrue(!sys.offerMission("e1", "m2", "Name", "agent1", MissionType::Courier, -1.0f, 10, 100.0f), "Negative isk rejected");
    assertTrue(!sys.offerMission("e1", "m2", "Name", "agent1", MissionType::Courier, 100.0f, 0, 100.0f), "Zero lp rejected");
    assertTrue(!sys.offerMission("e1", "m2", "Name", "agent1", MissionType::Courier, 100.0f, -1, 100.0f), "Negative lp rejected");
    assertTrue(!sys.offerMission("e1", "m2", "Name", "agent1", MissionType::Courier, 100.0f, 10, 0.0f), "Zero time_limit rejected");
    assertTrue(!sys.offerMission("e1", "m2", "Name", "agent1", MissionType::Courier, 100.0f, 10, -1.0f), "Negative time_limit rejected");
    assertTrue(sys.getMissionCount("e1") == 1, "Still 1 after invalid offers");
}

static void testAgentMissionAcceptComplete() {
    std::cout << "\n=== AgentMission: AcceptComplete ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.offerMission("e1", "m1", "Courier run", "agent1", MissionType::Courier, 200000.0f, 50, 1800.0f);
    assertTrue(sys.acceptMission("e1", "m1"), "Accept mission");
    assertTrue(sys.getMissionStatus("e1", "m1") == MissionStatus::Active, "Status Active");
    assertTrue(sys.getActiveMissionCount("e1") == 1, "1 active mission");
    assertTrue(!sys.acceptMission("e1", "m1"), "Can't accept active mission");
    assertTrue(sys.completeMission("e1", "m1"), "Complete mission");
    assertTrue(sys.getMissionStatus("e1", "m1") == MissionStatus::Completed, "Status Completed");
    assertTrue(sys.getActiveMissionCount("e1") == 0, "0 active after complete");
    assertTrue(approxEqual(sys.getTotalIskEarned("e1"), 200000.0f), "ISK earned");
    assertTrue(sys.getTotalLpEarned("e1") == 50, "LP earned");
    assertTrue(sys.getTotalCompleted("e1") == 1, "1 completed");
    assertTrue(!sys.completeMission("e1", "m1"), "Can't complete non-active");
}

static void testAgentMissionFail() {
    std::cout << "\n=== AgentMission: Fail ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.offerMission("e1", "m1", "Mining run", "agent1", MissionType::Mining, 100000.0f, 25, 7200.0f);
    sys.acceptMission("e1", "m1");
    assertTrue(sys.failMission("e1", "m1"), "Fail active mission");
    assertTrue(sys.getMissionStatus("e1", "m1") == MissionStatus::Failed, "Status Failed");
    assertTrue(sys.getTotalFailed("e1") == 1, "1 failed");
    assertTrue(approxEqual(sys.getTotalIskEarned("e1"), 0.0f), "No ISK for failed");
    sys.offerMission("e1", "m2", "Explore", "agent2", MissionType::Exploration, 150000.0f, 30, 3600.0f);
    assertTrue(sys.failMission("e1", "m2"), "Fail offered mission");
    assertTrue(sys.getMissionStatus("e1", "m2") == MissionStatus::Failed, "Status Failed");
    assertTrue(sys.getTotalFailed("e1") == 2, "2 failed");
}

static void testAgentMissionExpire() {
    std::cout << "\n=== AgentMission: Expire ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.offerMission("e1", "m1", "Old mission", "agent1", MissionType::Combat, 50000.0f, 10, 100.0f);
    assertTrue(sys.expireMission("e1", "m1"), "Expire offered mission");
    assertTrue(sys.getMissionStatus("e1", "m1") == MissionStatus::Expired, "Status Expired");
    assertTrue(sys.getTotalExpired("e1") == 1, "1 expired");
    sys.offerMission("e1", "m2", "Active mission", "agent2", MissionType::Courier, 50000.0f, 10, 100.0f);
    sys.acceptMission("e1", "m2");
    assertTrue(!sys.expireMission("e1", "m2"), "Can't expire active mission");
}

static void testAgentMissionTimeLimitFail() {
    std::cout << "\n=== AgentMission: TimeLimitFail ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.offerMission("e1", "m1", "Timed mission", "agent1", MissionType::Combat, 100000.0f, 20, 5.0f);
    sys.acceptMission("e1", "m1");
    sys.update(3.0f);
    assertTrue(sys.getMissionStatus("e1", "m1") == MissionStatus::Active, "Still active at 3s");
    sys.update(3.0f);
    assertTrue(sys.getMissionStatus("e1", "m1") == MissionStatus::Failed, "Failed at 6s (limit 5s)");
    assertTrue(sys.getTotalFailed("e1") == 1, "1 auto-failed");
}

static void testAgentMissionRemove() {
    std::cout << "\n=== AgentMission: Remove ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.offerMission("e1", "m1", "Mission1", "a1", MissionType::Combat, 100000.0f, 10, 100.0f);
    sys.offerMission("e1", "m2", "Mission2", "a1", MissionType::Courier, 200000.0f, 20, 200.0f);
    assertTrue(sys.getMissionCount("e1") == 2, "2 missions");
    assertTrue(sys.removeMission("e1", "m1"), "Remove m1");
    assertTrue(sys.getMissionCount("e1") == 1, "1 mission after remove");
    assertTrue(!sys.hasMission("e1", "m1"), "m1 gone");
    assertTrue(sys.hasMission("e1", "m2"), "m2 still present");
    assertTrue(!sys.removeMission("e1", "m1"), "Remove missing mission fails");
    assertTrue(!sys.removeMission("nonexistent", "m2"), "Remove on missing entity fails");
}

static void testAgentMissionClear() {
    std::cout << "\n=== AgentMission: Clear ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.offerMission("e1", "m1", "M1", "a1", MissionType::Mining, 100000.0f, 10, 100.0f);
    sys.offerMission("e1", "m2", "M2", "a1", MissionType::Exploration, 200000.0f, 20, 200.0f);
    assertTrue(sys.getMissionCount("e1") == 2, "2 missions before clear");
    assertTrue(sys.clearMissions("e1"), "Clear succeeds");
    assertTrue(sys.getMissionCount("e1") == 0, "0 missions after clear");
    assertTrue(!sys.clearMissions("nonexistent"), "Clear on missing entity fails");
}

static void testAgentMissionGetCountByType() {
    std::cout << "\n=== AgentMission: GetCountByType ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.offerMission("e1", "m1", "Combat1", "a1", MissionType::Combat,       100000.0f, 10, 100.0f);
    sys.offerMission("e1", "m2", "Combat2", "a1", MissionType::Combat,       100000.0f, 10, 100.0f);
    sys.offerMission("e1", "m3", "Courier1", "a1", MissionType::Courier,     100000.0f, 10, 100.0f);
    sys.offerMission("e1", "m4", "Mining1", "a1", MissionType::Mining,       100000.0f, 10, 100.0f);
    sys.offerMission("e1", "m5", "Explore1", "a1", MissionType::Exploration, 100000.0f, 10, 100.0f);
    assertTrue(sys.getCountByType("e1", MissionType::Combat) == 2, "2 combat");
    assertTrue(sys.getCountByType("e1", MissionType::Courier) == 1, "1 courier");
    assertTrue(sys.getCountByType("e1", MissionType::Mining) == 1, "1 mining");
    assertTrue(sys.getCountByType("e1", MissionType::Exploration) == 1, "1 exploration");
    assertTrue(sys.getCountByType("nonexistent", MissionType::Combat) == 0, "0 for missing entity");
}

static void testAgentMissionMaxCap() {
    std::cout << "\n=== AgentMission: MaxCap ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    for (int i = 0; i < 20; i++) {
        std::string id = "m" + std::to_string(i);
        std::string name = "Mission" + std::to_string(i);
        assertTrue(sys.offerMission("e1", id, name, "agent1", MissionType::Combat, 100000.0f, 10, 100.0f), "Offer " + id);
    }
    assertTrue(sys.getMissionCount("e1") == 20, "20 missions at cap");
    assertTrue(!sys.offerMission("e1", "m_over", "Over", "agent1", MissionType::Combat, 100000.0f, 10, 100.0f), "21st mission rejected");
}

static void testAgentMissionMissing() {
    std::cout << "\n=== AgentMission: Missing entity ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    const std::string missing = "nonexistent";
    assertTrue(!sys.offerMission(missing, "m1", "M1", "a1", MissionType::Combat, 100000.0f, 10, 100.0f), "offerMission missing entity");
    assertTrue(!sys.acceptMission(missing, "m1"), "acceptMission missing entity");
    assertTrue(!sys.completeMission(missing, "m1"), "completeMission missing entity");
    assertTrue(!sys.failMission(missing, "m1"), "failMission missing entity");
    assertTrue(!sys.expireMission(missing, "m1"), "expireMission missing entity");
    assertTrue(!sys.removeMission(missing, "m1"), "removeMission missing entity");
    assertTrue(!sys.clearMissions(missing), "clearMissions missing entity");
    assertTrue(sys.getMissionCount(missing) == 0, "getMissionCount missing entity");
    assertTrue(sys.getActiveMissionCount(missing) == 0, "getActiveMissionCount missing entity");
    assertTrue(!sys.hasMission(missing, "m1"), "hasMission missing entity");
    assertTrue(approxEqual(sys.getTotalIskEarned(missing), 0.0f), "getTotalIskEarned missing entity");
    assertTrue(sys.getTotalLpEarned(missing) == 0, "getTotalLpEarned missing entity");
    assertTrue(sys.getTotalCompleted(missing) == 0, "getTotalCompleted missing entity");
    assertTrue(sys.getTotalFailed(missing) == 0, "getTotalFailed missing entity");
    assertTrue(sys.getTotalExpired(missing) == 0, "getTotalExpired missing entity");
    assertTrue(sys.getCountByType(missing, MissionType::Combat) == 0, "getCountByType missing entity");
}

static void testAgentMissionInactiveSkipsUpdate() {
    std::cout << "\n=== AgentMission: InactiveSkips ===" << std::endl;
    ecs::World world;
    systems::AgentMissionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.offerMission("e1", "m1", "Timed", "agent1", MissionType::Combat, 100000.0f, 10, 5.0f);
    sys.acceptMission("e1", "m1");
    auto* comp = world.getEntity("e1")->getComponent<components::AgentMissionState>();
    comp->active = false;
    sys.update(10.0f);
    assertTrue(sys.getMissionStatus("e1", "m1") == MissionStatus::Active, "Still active when system inactive");
    assertTrue(sys.getTotalFailed("e1") == 0, "No auto-fails when inactive");
}

void run_agent_mission_system_tests() {
    testAgentMissionInit();
    testAgentMissionOffer();
    testAgentMissionAcceptComplete();
    testAgentMissionFail();
    testAgentMissionExpire();
    testAgentMissionTimeLimitFail();
    testAgentMissionRemove();
    testAgentMissionClear();
    testAgentMissionGetCountByType();
    testAgentMissionMaxCap();
    testAgentMissionMissing();
    testAgentMissionInactiveSkipsUpdate();
}
