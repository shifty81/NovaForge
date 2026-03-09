// Tests for: AncientTechSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/ancient_tech_system.h"

using namespace atlas;

// ==================== AncientTechSystem Tests ====================

static void testAncientTechInitialState() {
    std::cout << "\n=== AncientTech: InitialState ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);

    assertTrue(tech->state == components::AncientTechModule::TechState::Broken, "Initial state is Broken");
    assertTrue(tech->repair_progress == 0.0f, "Initial repair progress 0");
    assertTrue(!tech->reverse_engineered, "Not reverse engineered initially");
    assertTrue(!sys.isUsable("tech1"), "Broken tech is not usable");
}

static void testAncientTechStartRepair() {
    std::cout << "\n=== AncientTech: StartRepair ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);

    assertTrue(sys.startRepair("tech1"), "Start repair on broken tech");
    assertTrue(tech->state == components::AncientTechModule::TechState::Repairing, "State is Repairing");
    assertTrue(!sys.isUsable("tech1"), "Repairing tech is not usable");
}

static void testAncientTechRepairProgress() {
    std::cout << "\n=== AncientTech: RepairProgress ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->repair_cost = 100.0f;

    sys.startRepair("tech1");

    // Simulate partial update (repair_cost * 0.5 = 50s to complete)
    sys.update(10.0f);
    assertTrue(tech->repair_progress > 0.0f, "Progress advanced after update");
    assertTrue(tech->repair_progress < 1.0f, "Progress not yet complete");
    assertTrue(tech->state == components::AncientTechModule::TechState::Repairing, "Still repairing");
}

static void testAncientTechRepairCompletion() {
    std::cout << "\n=== AncientTech: RepairCompletion ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->repair_cost = 100.0f;

    sys.startRepair("tech1");

    // Simulate enough time to complete (repair_cost * 0.5 = 50s)
    sys.update(60.0f);
    assertTrue(tech->repair_progress >= 1.0f, "Progress reached 1.0");
    assertTrue(tech->state == components::AncientTechModule::TechState::Repaired, "State is Repaired");
    assertTrue(sys.isUsable("tech1"), "Repaired tech is usable");
}

static void testAncientTechCannotRepairNonBroken() {
    std::cout << "\n=== AncientTech: CannotRepairNonBroken ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;

    assertTrue(!sys.startRepair("tech1"), "Cannot repair already repaired tech");
    assertTrue(tech->state == components::AncientTechModule::TechState::Repaired, "State unchanged");
}

static void testAncientTechReverseEngineer() {
    std::cout << "\n=== AncientTech: ReverseEngineer ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;
    tech->blueprint_id = "ancient_laser_mk2";

    std::string bp = sys.reverseEngineer("tech1");
    assertTrue(bp == "ancient_laser_mk2", "Reverse engineer returns blueprint ID");
    assertTrue(tech->reverse_engineered, "Module marked as reverse engineered");
}

static void testAncientTechCannotReverseEngineerBroken() {
    std::cout << "\n=== AncientTech: CannotReverseEngineerBroken ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->blueprint_id = "ancient_shield_mk1";

    std::string bp = sys.reverseEngineer("tech1");
    assertTrue(bp.empty(), "Cannot reverse engineer broken tech");
    assertTrue(!tech->reverse_engineered, "Not marked as reverse engineered");
}

static void testAncientTechGetState() {
    std::cout << "\n=== AncientTech: GetState ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);

    assertTrue(sys.getState("tech1") == components::AncientTechModule::TechState::Broken, "Get state Broken");

    tech->state = components::AncientTechModule::TechState::Repaired;
    assertTrue(sys.getState("tech1") == components::AncientTechModule::TechState::Repaired, "Get state Repaired");

    tech->state = components::AncientTechModule::TechState::Upgraded;
    assertTrue(sys.getState("tech1") == components::AncientTechModule::TechState::Upgraded, "Get state Upgraded");
}

static void testAncientTechUpgradedIsUsable() {
    std::cout << "\n=== AncientTech: UpgradedIsUsable ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Upgraded;

    assertTrue(sys.isUsable("tech1"), "Upgraded tech is usable");
}

static void testAncientTechMissingEntity() {
    std::cout << "\n=== AncientTech: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);

    assertTrue(!sys.startRepair("ghost"), "Start repair fails for missing entity");
    assertTrue(sys.reverseEngineer("ghost").empty(), "Reverse engineer empty for missing");
    assertTrue(sys.getState("ghost") == components::AncientTechModule::TechState::Broken, "Default state for missing");
    assertTrue(!sys.isUsable("ghost"), "Not usable for missing entity");
}

static void testAncientTechMultipleEntities() {
    std::cout << "\n=== AncientTech: MultipleEntities ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);

    auto* e1 = world.createEntity("tech1");
    auto* e2 = world.createEntity("tech2");
    auto* t1 = addComp<components::AncientTechModule>(e1);
    auto* t2 = addComp<components::AncientTechModule>(e2);
    t1->repair_cost = 10.0f;
    t2->repair_cost = 10.0f;

    sys.startRepair("tech1");
    // Only tech1 repairing, tech2 still broken
    sys.update(6.0f);
    assertTrue(t1->state == components::AncientTechModule::TechState::Repaired, "tech1 repaired");
    assertTrue(t2->state == components::AncientTechModule::TechState::Broken, "tech2 still broken");
}

static void testAncientTechRepairDoesNotOvershoot() {
    std::cout << "\n=== AncientTech: RepairDoesNotOvershoot ===" << std::endl;
    ecs::World world;
    systems::AncientTechSystem sys(&world);
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->repair_cost = 10.0f;

    sys.startRepair("tech1");
    // Way more than needed
    sys.update(1000.0f);
    assertTrue(tech->repair_progress == 1.0f, "Progress clamped to 1.0");
    assertTrue(tech->state == components::AncientTechModule::TechState::Repaired, "Repaired after large delta");
}

void run_ancient_tech_system_tests() {
    testAncientTechInitialState();
    testAncientTechStartRepair();
    testAncientTechRepairProgress();
    testAncientTechRepairCompletion();
    testAncientTechCannotRepairNonBroken();
    testAncientTechReverseEngineer();
    testAncientTechCannotReverseEngineerBroken();
    testAncientTechGetState();
    testAncientTechUpgradedIsUsable();
    testAncientTechMissingEntity();
    testAncientTechMultipleEntities();
    testAncientTechRepairDoesNotOvershoot();
}
