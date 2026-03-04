// Tests for: MissionEditor System Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/mission_editor_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== MissionEditor System Tests ====================

static void testMissionEditorCreate() {
    std::cout << "\n=== MissionEditor: Create ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    auto* e = world.createEntity("editor1");
    assertTrue(sys.createEditor("editor1"), "Create editor succeeds");
    auto* ed = e->getComponent<components::MissionEditor>();
    assertTrue(ed != nullptr, "Component exists");
    assertTrue(ed->active, "Editor is active by default");
    assertTrue(ed->mission_level == 1, "Default level is 1");
    assertTrue(ed->published_count == 0, "No published missions");
}

static void testMissionEditorSetName() {
    std::cout << "\n=== MissionEditor: SetName ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    assertTrue(sys.setMissionName("editor1", "Patrol Alpha"), "Set name succeeds");
    assertTrue(sys.setMissionLevel("editor1", 3), "Set level succeeds");
    assertTrue(sys.setMissionType("editor1", 2), "Set type succeeds");
}

static void testMissionEditorAddObjective() {
    std::cout << "\n=== MissionEditor: AddObjective ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    int id = sys.addObjective("editor1", "Destroy 5 pirates", 0);
    assertTrue(id > 0, "Objective ID is positive");
    assertTrue(sys.getObjectiveCount("editor1") == 1, "1 objective");
    int id2 = sys.addObjective("editor1", "Return to station", 1);
    assertTrue(id2 > id, "Second ID is higher");
    assertTrue(sys.getObjectiveCount("editor1") == 2, "2 objectives");
}

static void testMissionEditorRemoveObjective() {
    std::cout << "\n=== MissionEditor: RemoveObjective ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    int id = sys.addObjective("editor1", "Mine ore", 4);
    assertTrue(sys.removeObjective("editor1", id), "Remove succeeds");
    assertTrue(sys.getObjectiveCount("editor1") == 0, "0 objectives after remove");
    assertTrue(!sys.removeObjective("editor1", id), "Remove again fails");
}

static void testMissionEditorSetReward() {
    std::cout << "\n=== MissionEditor: SetReward ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    auto* e = world.createEntity("editor1");
    sys.createEditor("editor1");
    assertTrue(sys.setReward("editor1", 50000.0f, 0.5f), "Set reward succeeds");
    auto* ed = e->getComponent<components::MissionEditor>();
    assertTrue(ed->reward_credits == 50000.0f, "Credits set correctly");
    assertTrue(ed->reward_standing == 0.5f, "Standing set correctly");
}

static void testMissionEditorValidate() {
    std::cout << "\n=== MissionEditor: Validate ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    assertTrue(!sys.validate("editor1"), "Empty mission fails validation");
    assertTrue(sys.getValidationError("editor1") == "Mission name is required", "Error: name required");
    sys.setMissionName("editor1", "Test Mission");
    assertTrue(!sys.validate("editor1"), "No objectives fails validation");
    assertTrue(sys.getValidationError("editor1") == "At least one objective is required", "Error: objectives required");
}

static void testMissionEditorPublish() {
    std::cout << "\n=== MissionEditor: Publish ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    assertTrue(!sys.publish("editor1"), "Publish fails without valid mission");
    sys.setMissionName("editor1", "Patrol Beta");
    sys.addObjective("editor1", "Kill 3 enemies", 0);
    sys.setReward("editor1", 10000.0f, 0.1f);
    assertTrue(sys.publish("editor1"), "Publish succeeds with valid mission");
    assertTrue(sys.getPublishedCount("editor1") == 1, "Published count is 1");
}

static void testMissionEditorLevelClamp() {
    std::cout << "\n=== MissionEditor: LevelClamp ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    auto* e = world.createEntity("editor1");
    sys.createEditor("editor1");
    sys.setMissionLevel("editor1", 10);
    auto* ed = e->getComponent<components::MissionEditor>();
    assertTrue(ed->mission_level == 5, "Level clamped to 5");
    sys.setMissionLevel("editor1", -1);
    assertTrue(ed->mission_level == 1, "Level clamped to 1");
}

static void testMissionEditorEmptyObjective() {
    std::cout << "\n=== MissionEditor: EmptyObjective ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    int id = sys.addObjective("editor1", "", 0);
    assertTrue(id == -1, "Empty description rejected");
    assertTrue(sys.getObjectiveCount("editor1") == 0, "No objectives added");
}

static void testMissionEditorMissing() {
    std::cout << "\n=== MissionEditor: Missing ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    assertTrue(!sys.createEditor("nonexistent"), "Create fails on missing");
    assertTrue(!sys.setMissionName("nonexistent", "X"), "SetName fails on missing");
    assertTrue(!sys.setMissionLevel("nonexistent", 1), "SetLevel fails on missing");
    assertTrue(!sys.setMissionType("nonexistent", 0), "SetType fails on missing");
    assertTrue(sys.addObjective("nonexistent", "X", 0) == -1, "AddObjective fails on missing");
    assertTrue(!sys.removeObjective("nonexistent", 1), "RemoveObjective fails on missing");
    assertTrue(!sys.setReward("nonexistent", 100.0f, 0.0f), "SetReward fails on missing");
    assertTrue(!sys.validate("nonexistent"), "Validate fails on missing");
    assertTrue(!sys.publish("nonexistent"), "Publish fails on missing");
    assertTrue(sys.getObjectiveCount("nonexistent") == 0, "0 objectives on missing");
    assertTrue(sys.getPublishedCount("nonexistent") == 0, "0 published on missing");
}


void run_mission_editor_system_tests() {
    testMissionEditorCreate();
    testMissionEditorSetName();
    testMissionEditorAddObjective();
    testMissionEditorRemoveObjective();
    testMissionEditorSetReward();
    testMissionEditorValidate();
    testMissionEditorPublish();
    testMissionEditorLevelClamp();
    testMissionEditorEmptyObjective();
    testMissionEditorMissing();
}
