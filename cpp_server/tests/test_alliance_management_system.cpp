// Tests for: AllianceManagementSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/alliance_management_system.h"

using namespace atlas;

// ==================== AllianceManagementSystem Tests ====================

static void testAllianceInit() {
    std::cout << "\n=== Alliance: Init ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);
    world.createEntity("a1");
    assertTrue(sys.initialize("a1", "alliance_001", "Northern Coalition", "NC.",
               "corp_exec"), "Init succeeds");
    assertTrue(sys.getAllianceName("a1") == "Northern Coalition", "Name stored");
    assertTrue(sys.getTicker("a1") == "NC.", "Ticker stored");
    assertTrue(sys.getExecutorCorpId("a1") == "corp_exec", "Executor stored");
    assertTrue(sys.isActive("a1"), "Alliance is active");
    assertTrue(sys.getMemberCount("a1") == 1, "Executor auto-added as member");
    assertTrue(sys.hasMember("a1", "corp_exec"), "Executor is a member");
    assertTrue(sys.getTotalMembersJoined("a1") == 1, "One total joined");
}

static void testAllianceInitFails() {
    std::cout << "\n=== Alliance: InitFails ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "a", "N", "T", "e"),
               "Init fails on missing entity");
    world.createEntity("a1");
    assertTrue(!sys.initialize("a1", "", "Name", "TK", "exec"), "Empty alliance_id rejected");
    assertTrue(!sys.initialize("a1", "id", "", "TK", "exec"), "Empty name rejected");
    assertTrue(!sys.initialize("a1", "id", "Name", "", "exec"), "Empty ticker rejected");
    assertTrue(!sys.initialize("a1", "id", "Name", "TK", ""), "Empty executor rejected");
}

static void testAllianceAddMember() {
    std::cout << "\n=== Alliance: AddMember ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);
    world.createEntity("a1");
    sys.initialize("a1", "a001", "NC", "NC.", "exec_corp");

    assertTrue(sys.addMember("a1", "corp_a", "Alpha Corp"), "Add first member");
    assertTrue(sys.addMember("a1", "corp_b", "Beta Corp"), "Add second member");
    assertTrue(sys.getMemberCount("a1") == 3, "3 members (exec + 2)");
    assertTrue(sys.getTotalMembersJoined("a1") == 3, "3 total joined");
    assertTrue(!sys.addMember("a1", "corp_a", "Duplicate"), "Duplicate corp_id rejected");
    assertTrue(!sys.addMember("a1", "", "Empty ID"), "Empty corp_id rejected");
    assertTrue(!sys.addMember("a1", "c", ""), "Empty corp_name rejected");
    assertTrue(sys.getMemberCount("a1") == 3, "Count unchanged after rejections");
}

static void testAllianceRemoveMember() {
    std::cout << "\n=== Alliance: RemoveMember ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);
    world.createEntity("a1");
    sys.initialize("a1", "a001", "NC", "NC.", "exec_corp");
    sys.addMember("a1", "corp_a", "Alpha Corp");
    sys.addMember("a1", "corp_b", "Beta Corp");

    assertTrue(sys.removeMember("a1", "corp_a"), "Remove regular member");
    assertTrue(sys.getMemberCount("a1") == 2, "Count decremented");
    assertTrue(!sys.hasMember("a1", "corp_a"), "Removed member gone");
    assertTrue(!sys.removeMember("a1", "corp_a"), "Remove nonexistent fails");
    assertTrue(!sys.removeMember("a1", "exec_corp"), "Cannot remove executor");
    assertTrue(sys.getMemberCount("a1") == 2, "Executor still counted");
}

static void testAllianceSetExecutor() {
    std::cout << "\n=== Alliance: SetExecutor ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);
    world.createEntity("a1");
    sys.initialize("a1", "a001", "NC", "NC.", "exec_corp");
    sys.addMember("a1", "corp_a", "Alpha Corp");

    assertTrue(sys.setExecutor("a1", "corp_a"), "Set new executor");
    assertTrue(sys.getExecutorCorpId("a1") == "corp_a", "Executor updated");
    assertTrue(!sys.setExecutor("a1", "nonmember"), "Cannot set non-member as executor");
    assertTrue(sys.getExecutorCorpId("a1") == "corp_a", "Executor unchanged after failure");
}

static void testAllianceDisband() {
    std::cout << "\n=== Alliance: Disband ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);
    world.createEntity("a1");
    sys.initialize("a1", "a001", "NC", "NC.", "exec_corp");
    sys.addMember("a1", "corp_a", "Alpha Corp");

    assertTrue(sys.disbandAlliance("a1"), "Disband succeeds");
    assertTrue(!sys.isActive("a1"), "Alliance is disbanded");
    assertTrue(sys.getMemberCount("a1") == 0, "Members cleared on disband");
    assertTrue(sys.getExecutorCorpId("a1").empty(), "Executor cleared on disband");
    assertTrue(!sys.disbandAlliance("a1"), "Cannot disband twice");
    assertTrue(!sys.addMember("a1", "corp_b", "Beta"), "Cannot add to disbanded alliance");
    assertTrue(!sys.removeMember("a1", "corp_a"), "Cannot remove from disbanded alliance");
    assertTrue(!sys.setExecutor("a1", "corp_a"), "Cannot set executor on disbanded alliance");
}

static void testAllianceMaxMembers() {
    std::cout << "\n=== Alliance: MaxMembers ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);
    world.createEntity("a1");
    sys.initialize("a1", "a001", "NC", "NC.", "exec_corp");

    // Set small max for testing
    auto* comp = world.getEntity("a1")->getComponent<components::AllianceState>();
    comp->max_members = 3;

    sys.addMember("a1", "c1", "Corp1");
    sys.addMember("a1", "c2", "Corp2");
    assertTrue(sys.getMemberCount("a1") == 3, "3 members at capacity (exec+2)");
    assertTrue(!sys.addMember("a1", "c3", "Corp3"), "Fourth member rejected at capacity");
    assertTrue(sys.getMemberCount("a1") == 3, "Count still 3");
}

static void testAllianceHasMember() {
    std::cout << "\n=== Alliance: HasMember ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);
    world.createEntity("a1");
    sys.initialize("a1", "a001", "NC", "NC.", "exec_corp");
    sys.addMember("a1", "corp_a", "Alpha Corp");

    assertTrue(sys.hasMember("a1", "exec_corp"), "Has executor");
    assertTrue(sys.hasMember("a1", "corp_a"), "Has corp_a");
    assertTrue(!sys.hasMember("a1", "nonexistent"), "Does not have nonexistent");
    assertTrue(!sys.hasMember("nonexistent", "corp_a"), "Missing entity returns false");
}

static void testAllianceTick() {
    std::cout << "\n=== Alliance: Tick ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);
    world.createEntity("a1");
    sys.initialize("a1", "a001", "NC", "NC.", "exec_corp");

    sys.update(1.0f);
    sys.update(2.5f);
    // Just verify it doesn't crash — elapsed tracked internally
    assertTrue(sys.isActive("a1"), "Still active after ticks");
}

static void testAllianceMissing() {
    std::cout << "\n=== Alliance: Missing ===" << std::endl;
    ecs::World world;
    systems::AllianceManagementSystem sys(&world);

    assertTrue(!sys.addMember("nonexistent", "c", "C"), "AddMember fails on missing");
    assertTrue(!sys.removeMember("nonexistent", "c"), "RemoveMember fails on missing");
    assertTrue(!sys.setExecutor("nonexistent", "c"), "SetExecutor fails on missing");
    assertTrue(!sys.disbandAlliance("nonexistent"), "Disband fails on missing");
    assertTrue(sys.getMemberCount("nonexistent") == 0, "Zero members on missing");
    assertTrue(sys.getExecutorCorpId("nonexistent").empty(), "Empty executor on missing");
    assertTrue(sys.getAllianceName("nonexistent").empty(), "Empty name on missing");
    assertTrue(sys.getTicker("nonexistent").empty(), "Empty ticker on missing");
    assertTrue(!sys.isActive("nonexistent"), "Not active on missing");
    assertTrue(!sys.hasMember("nonexistent", "c"), "HasMember false on missing");
    assertTrue(sys.getTotalMembersJoined("nonexistent") == 0, "Zero total on missing");
}

void run_alliance_management_system_tests() {
    testAllianceInit();
    testAllianceInitFails();
    testAllianceAddMember();
    testAllianceRemoveMember();
    testAllianceSetExecutor();
    testAllianceDisband();
    testAllianceMaxMembers();
    testAllianceHasMember();
    testAllianceTick();
    testAllianceMissing();
}
