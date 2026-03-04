// Tests for: CloneBay System Tests
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/clone_bay_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== CloneBay System Tests ====================

static void testCloneBayCreate() {
    std::cout << "\n=== CloneBay: Create ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    assertTrue(sys.initialize("bay1", "cb_01", "station_jita"), "Init succeeds");
    assertTrue(sys.getCloneCount("bay1") == 0, "No clones initially");
    assertTrue(sys.getImplantCount("bay1") == 0, "No implants initially");
    assertTrue(sys.getActiveClone("bay1") == 0, "No active clone");
    assertTrue(sys.getTotalDeaths("bay1") == 0, "No deaths initially");
}

static void testCloneBayAddClone() {
    std::cout << "\n=== CloneBay: AddClone ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    assertTrue(sys.addClone("bay1", "clone_a", 1), "Add Alpha clone");
    assertTrue(sys.addClone("bay1", "clone_b", 3), "Add Gamma clone");
    assertTrue(sys.getCloneCount("bay1") == 2, "2 clones");
}

static void testCloneBayDuplicate() {
    std::cout << "\n=== CloneBay: Duplicate ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 1);
    assertTrue(!sys.addClone("bay1", "clone_a", 2), "Duplicate clone rejected");
    sys.addClone("bay1", "clone_b", 2);
    sys.installImplant("bay1", "imp1", 1, "perception", 3.0f, "clone_b");
    assertTrue(!sys.installImplant("bay1", "imp1", 2, "memory", 3.0f, "clone_b"), "Duplicate implant rejected");
}

static void testCloneBayActivate() {
    std::cout << "\n=== CloneBay: Activate ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 1);
    sys.addClone("bay1", "clone_b", 3);
    assertTrue(sys.activateClone("bay1", "clone_a"), "Activate clone_a");
    assertTrue(sys.getActiveClone("bay1") == 1, "Active clone is grade 1");
    assertTrue(sys.activateClone("bay1", "clone_b"), "Activate clone_b");
    assertTrue(sys.getActiveClone("bay1") == 3, "Active clone is grade 3");
}

static void testCloneBayImplant() {
    std::cout << "\n=== CloneBay: Implant ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 3);
    assertTrue(sys.installImplant("bay1", "imp1", 1, "perception", 3.0f, "clone_a"), "Install implant");
    assertTrue(sys.getImplantCount("bay1") == 1, "1 implant");
    assertTrue(sys.removeImplant("bay1", "imp1"), "Remove implant");
    assertTrue(sys.getImplantCount("bay1") == 0, "0 implants after remove");
}

static void testCloneBayImplantSlot() {
    std::cout << "\n=== CloneBay: ImplantSlot ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 2);  // grade 2 = 2 implant slots
    assertTrue(sys.installImplant("bay1", "imp1", 1, "perception", 3.0f, "clone_a"), "Slot 1");
    assertTrue(sys.installImplant("bay1", "imp2", 2, "memory", 3.0f, "clone_a"), "Slot 2");
    assertTrue(!sys.installImplant("bay1", "imp3", 3, "willpower", 3.0f, "clone_a"), "Slot limit reached");
    assertTrue(sys.getImplantCount("bay1") == 2, "2 implants");
}

static void testCloneBayDeath() {
    std::cout << "\n=== CloneBay: Death ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 1);  // sp_limit = 900000
    sys.activateClone("bay1", "clone_a");
    float loss = sys.processDeath("bay1", 1500000.0f);
    assertTrue(loss > 500000.0f, "SP loss is above 500k");
    assertTrue(approxEqual(loss, 600000.0f), "SP loss is 600000");
    assertTrue(sys.getTotalDeaths("bay1") == 1, "1 death recorded");
}

static void testCloneBayGrades() {
    std::cout << "\n=== CloneBay: Grades ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 1);
    sys.addClone("bay1", "clone_b", 5);
    auto* entity = world.getEntity("bay1");
    auto* bay = entity->getComponent<components::CloneBay>();
    assertTrue(approxEqual(bay->clones[0].sp_limit, 900000.0f), "Grade 1 sp_limit 900k");
    assertTrue(approxEqual(bay->clones[1].sp_limit, 4500000.0f), "Grade 5 sp_limit 4500k");
    assertTrue(approxEqual(bay->clones[0].cost, 5000000.0f), "Grade 1 cost 5M");
    assertTrue(approxEqual(bay->clones[1].cost, 25000000.0f), "Grade 5 cost 25M");
}

static void testCloneBayMaxLimit() {
    std::cout << "\n=== CloneBay: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    auto* entity = world.getEntity("bay1");
    auto* bay = entity->getComponent<components::CloneBay>();
    bay->max_clones = 2;
    bay->max_implants = 1;
    sys.addClone("bay1", "c1", 3);
    sys.addClone("bay1", "c2", 3);
    assertTrue(!sys.addClone("bay1", "c3", 3), "Max clones enforced");
    sys.installImplant("bay1", "i1", 1, "perception", 3.0f, "c1");
    assertTrue(!sys.installImplant("bay1", "i2", 2, "memory", 3.0f, "c1"), "Max implants enforced");
}

static void testCloneBayMissing() {
    std::cout << "\n=== CloneBay: Missing ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "cb1", "s1"), "Init fails on missing");
    assertTrue(!sys.addClone("nonexistent", "c1", 1), "AddClone fails on missing");
    assertTrue(!sys.removeClone("nonexistent", "c1"), "RemoveClone fails on missing");
    assertTrue(!sys.activateClone("nonexistent", "c1"), "Activate fails on missing");
    assertTrue(!sys.installImplant("nonexistent", "i1", 1, "perception", 3.0f, "c1"), "InstallImplant fails on missing");
    assertTrue(!sys.removeImplant("nonexistent", "i1"), "RemoveImplant fails on missing");
    assertTrue(approxEqual(sys.processDeath("nonexistent", 1000.0f), 0.0f), "0 SP loss on missing");
    assertTrue(sys.getActiveClone("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getCloneCount("nonexistent") == 0, "0 clones on missing");
    assertTrue(sys.getImplantCount("nonexistent") == 0, "0 implants on missing");
    assertTrue(sys.getTotalDeaths("nonexistent") == 0, "0 deaths on missing");
    assertTrue(approxEqual(sys.getSkillPointsAtRisk("nonexistent", 1000.0f), 0.0f), "0 SP at risk on missing");
}


void run_clone_bay_system_tests() {
    testCloneBayCreate();
    testCloneBayAddClone();
    testCloneBayDuplicate();
    testCloneBayActivate();
    testCloneBayImplant();
    testCloneBayImplantSlot();
    testCloneBayDeath();
    testCloneBayGrades();
    testCloneBayMaxLimit();
    testCloneBayMissing();
}
