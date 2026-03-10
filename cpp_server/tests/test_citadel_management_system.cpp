// Tests for: CitadelManagementSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/citadel_management_system.h"

using namespace atlas;

// ==================== CitadelManagementSystem Tests ====================

static void testCitadelInit() {
    std::cout << "\n=== Citadel: Init ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    assertTrue(sys.initialize("c1", "corp_a", "Home Base",
               components::CitadelState::CitadelType::Astrahus),
               "Init Astrahus succeeds");
    assertTrue(sys.getState("c1") == components::CitadelState::StructureState::Online,
               "Initial state is Online");
    assertTrue(approxEqual(sys.getShieldHp("c1"), 10000.0f), "Astrahus shield HP");
    assertTrue(approxEqual(sys.getArmorHp("c1"), 10000.0f), "Astrahus armor HP");
    assertTrue(approxEqual(sys.getHullHp("c1"), 10000.0f), "Astrahus hull HP");
    assertTrue(sys.getServiceCount("c1") == 0, "Zero services initially");
    assertTrue(approxEqual(sys.getFuelRemaining("c1"), 3000.0f), "Astrahus fuel capacity");
    assertTrue(sys.getTotalReinforcements("c1") == 0, "Zero reinforcements initially");
}

static void testCitadelInitTypes() {
    std::cout << "\n=== Citadel: InitTypes ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    world.createEntity("c2");

    sys.initialize("c1", "corp_a", "Fort", components::CitadelState::CitadelType::Fortizar);
    assertTrue(approxEqual(sys.getShieldHp("c1"), 30000.0f), "Fortizar shield HP");

    sys.initialize("c2", "corp_a", "Keep", components::CitadelState::CitadelType::Keepstar);
    assertTrue(approxEqual(sys.getShieldHp("c2"), 100000.0f), "Keepstar shield HP");
}

static void testCitadelInitFails() {
    std::cout << "\n=== Citadel: InitFails ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "corp", "Name",
               components::CitadelState::CitadelType::Astrahus),
               "Init fails on missing entity");
    world.createEntity("c1");
    assertTrue(!sys.initialize("c1", "", "Name",
               components::CitadelState::CitadelType::Astrahus),
               "Init fails with empty corp");
    assertTrue(!sys.initialize("c1", "corp", "",
               components::CitadelState::CitadelType::Astrahus),
               "Init fails with empty name");
}

static void testCitadelSetOwner() {
    std::cout << "\n=== Citadel: SetOwner ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);

    assertTrue(sys.setOwner("c1", "corp_b"), "Transfer ownership");
    auto* comp = world.getEntity("c1")->getComponent<components::CitadelState>();
    assertTrue(comp->owner_corp_id == "corp_b", "Owner updated");
    assertTrue(!sys.setOwner("c1", ""), "Empty owner rejected");
}

static void testCitadelServices() {
    std::cout << "\n=== Citadel: Services ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);

    assertTrue(sys.addService("c1", "clone_bay", "Clone Bay", 10.0f), "Add clone bay");
    assertTrue(sys.addService("c1", "market", "Market Hub", 20.0f), "Add market");
    assertTrue(sys.getServiceCount("c1") == 2, "Two services installed");
    assertTrue(sys.getActiveServiceCount("c1") == 2, "Two services active");
    assertTrue(!sys.addService("c1", "clone_bay", "Duplicate", 5.0f), "Duplicate rejected");
    assertTrue(!sys.addService("c1", "", "Empty ID", 5.0f), "Empty ID rejected");
    assertTrue(!sys.addService("c1", "s3", "", 5.0f), "Empty name rejected");
    assertTrue(!sys.addService("c1", "s3", "Service", -1.0f), "Negative fuel rejected");

    assertTrue(sys.removeService("c1", "market"), "Remove market service");
    assertTrue(sys.getServiceCount("c1") == 1, "One service after removal");
    assertTrue(!sys.removeService("c1", "market"), "Remove nonexistent fails");
}

static void testCitadelServiceCapacity() {
    std::cout << "\n=== Citadel: ServiceCapacity ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);
    // Astrahus max_services = 3

    sys.addService("c1", "s1", "Svc1", 5.0f);
    sys.addService("c1", "s2", "Svc2", 5.0f);
    sys.addService("c1", "s3", "Svc3", 5.0f);
    assertTrue(sys.getServiceCount("c1") == 3, "Three services at capacity");
    assertTrue(!sys.addService("c1", "s4", "Svc4", 5.0f), "Fourth service rejected");
}

static void testCitadelServiceOnlineToggle() {
    std::cout << "\n=== Citadel: ServiceOnline ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);
    sys.addService("c1", "clone_bay", "Clone Bay", 10.0f);

    assertTrue(sys.getActiveServiceCount("c1") == 1, "Service online by default");
    assertTrue(sys.setServiceOnline("c1", "clone_bay", false), "Take service offline");
    assertTrue(sys.getActiveServiceCount("c1") == 0, "Zero active services");
    assertTrue(sys.setServiceOnline("c1", "clone_bay", true), "Bring service online");
    assertTrue(sys.getActiveServiceCount("c1") == 1, "One active service");
    assertTrue(!sys.setServiceOnline("c1", "nonexistent", true), "Toggle unknown fails");
}

static void testCitadelFuel() {
    std::cout << "\n=== Citadel: Fuel ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);
    // Astrahus fuel_capacity = 3000

    assertTrue(!sys.addFuel("c1", 0.0f), "Zero fuel rejected");
    assertTrue(!sys.addFuel("c1", -10.0f), "Negative fuel rejected");
    assertTrue(sys.addFuel("c1", 100.0f), "Add fuel");
    // Already at capacity (3000), adding 100 should still cap at 3000
    assertTrue(approxEqual(sys.getFuelRemaining("c1"), 3000.0f), "Fuel capped at capacity");
}

static void testCitadelFuelConsumption() {
    std::cout << "\n=== Citadel: FuelConsumption ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);
    sys.addService("c1", "market", "Market Hub", 3600.0f); // 3600 fuel/hour = 1 fuel/second

    // Set fuel low to test depletion
    auto* comp = world.getEntity("c1")->getComponent<components::CitadelState>();
    comp->fuel_remaining = 5.0f;

    sys.update(3.0f);  // consume 3 fuel
    assertTrue(approxEqual(sys.getFuelRemaining("c1"), 2.0f), "Fuel decreased after tick");

    sys.update(3.0f);  // consume 3 more, exceeds remaining
    assertTrue(approxEqual(sys.getFuelRemaining("c1"), 0.0f), "Fuel depleted to zero");
    assertTrue(sys.getActiveServiceCount("c1") == 0, "Services offline when fuel depleted");
}

static void testCitadelDamage() {
    std::cout << "\n=== Citadel: Damage ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);
    sys.setVulnerable("c1");

    // Apply damage that takes out shield and some armor
    assertTrue(sys.applyDamage("c1", 12000.0f), "Apply 12k damage");
    assertTrue(approxEqual(sys.getShieldHp("c1"), 0.0f), "Shield depleted");
    assertTrue(approxEqual(sys.getArmorHp("c1"), 8000.0f), "Armor took 2k overflow");
    assertTrue(approxEqual(sys.getHullHp("c1"), 10000.0f), "Hull untouched");
    assertTrue(!sys.applyDamage("c1", 0.0f), "Zero damage rejected");
    assertTrue(!sys.applyDamage("c1", -100.0f), "Negative damage rejected");
}

static void testCitadelDestruction() {
    std::cout << "\n=== Citadel: Destruction ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);
    sys.setVulnerable("c1");

    // 30000 total HP (10k shield + 10k armor + 10k hull)
    assertTrue(sys.applyDamage("c1", 30000.0f), "Apply lethal damage");
    assertTrue(sys.getState("c1") == components::CitadelState::StructureState::Destroyed,
               "State is Destroyed");
    assertTrue(!sys.applyDamage("c1", 100.0f), "Cannot damage destroyed structure");
    assertTrue(!sys.addService("c1", "s", "S", 1.0f), "Cannot add service to destroyed");
    assertTrue(!sys.addFuel("c1", 100.0f), "Cannot add fuel to destroyed");
    assertTrue(!sys.setOwner("c1", "corp_b"), "Cannot transfer destroyed");
}

static void testCitadelReinforcement() {
    std::cout << "\n=== Citadel: Reinforcement ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);

    assertTrue(!sys.triggerReinforcement("c1"), "Cannot reinforce from Online");
    sys.setVulnerable("c1");
    assertTrue(sys.triggerReinforcement("c1"), "Reinforce from Vulnerable");
    assertTrue(sys.getState("c1") == components::CitadelState::StructureState::Reinforced,
               "State is Reinforced");
    assertTrue(sys.getTotalReinforcements("c1") == 1, "Reinforcement counter = 1");
    assertTrue(sys.getReinforcementTimer("c1") > 0.0f, "Timer is positive");

    // Damage blocked during reinforcement
    assertTrue(!sys.applyDamage("c1", 1000.0f), "Damage blocked during reinforcement");

    // Timer counts down
    sys.update(86400.0f); // 24h default reinforcement duration
    assertTrue(sys.getState("c1") == components::CitadelState::StructureState::Vulnerable,
               "Returns to Vulnerable after timer");
    assertTrue(approxEqual(sys.getReinforcementTimer("c1"), 0.0f), "Timer at zero");
}

static void testCitadelRepair() {
    std::cout << "\n=== Citadel: Repair ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);
    sys.setVulnerable("c1");
    sys.applyDamage("c1", 15000.0f);

    assertTrue(sys.repairStructure("c1"), "Repair succeeds");
    assertTrue(approxEqual(sys.getShieldHp("c1"), 10000.0f), "Shield fully restored");
    assertTrue(approxEqual(sys.getArmorHp("c1"), 10000.0f), "Armor fully restored");
    assertTrue(approxEqual(sys.getHullHp("c1"), 10000.0f), "Hull fully restored");
    assertTrue(sys.getState("c1") == components::CitadelState::StructureState::Online,
               "State returned to Online");
}

static void testCitadelCannotRepairDestroyed() {
    std::cout << "\n=== Citadel: CannotRepairDestroyed ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1", "corp_a", "Base", components::CitadelState::CitadelType::Astrahus);
    sys.setVulnerable("c1");
    sys.applyDamage("c1", 30000.0f);
    assertTrue(sys.getState("c1") == components::CitadelState::StructureState::Destroyed,
               "Destroyed");
    assertTrue(!sys.repairStructure("c1"), "Cannot repair destroyed");
}

static void testCitadelMissing() {
    std::cout << "\n=== Citadel: Missing ===" << std::endl;
    ecs::World world;
    systems::CitadelManagementSystem sys(&world);

    assertTrue(!sys.setOwner("x", "corp"), "SetOwner fails on missing");
    assertTrue(!sys.addService("x", "s", "S", 1.0f), "AddService fails on missing");
    assertTrue(!sys.removeService("x", "s"), "RemoveService fails on missing");
    assertTrue(!sys.setServiceOnline("x", "s", true), "SetServiceOnline fails on missing");
    assertTrue(!sys.addFuel("x", 100.0f), "AddFuel fails on missing");
    assertTrue(!sys.setVulnerable("x"), "SetVulnerable fails on missing");
    assertTrue(!sys.triggerReinforcement("x"), "TriggerReinforcement fails on missing");
    assertTrue(!sys.repairStructure("x"), "RepairStructure fails on missing");
    assertTrue(!sys.applyDamage("x", 100.0f), "ApplyDamage fails on missing");
    assertTrue(sys.getState("x") == components::CitadelState::StructureState::Online,
               "Default state on missing");
    assertTrue(approxEqual(sys.getFuelRemaining("x"), 0.0f), "Zero fuel on missing");
    assertTrue(sys.getActiveServiceCount("x") == 0, "Zero active services on missing");
    assertTrue(sys.getServiceCount("x") == 0, "Zero services on missing");
    assertTrue(approxEqual(sys.getShieldHp("x"), 0.0f), "Zero shield on missing");
    assertTrue(approxEqual(sys.getArmorHp("x"), 0.0f), "Zero armor on missing");
    assertTrue(approxEqual(sys.getHullHp("x"), 0.0f), "Zero hull on missing");
    assertTrue(sys.getTotalReinforcements("x") == 0, "Zero reinforcements on missing");
    assertTrue(approxEqual(sys.getReinforcementTimer("x"), 0.0f), "Zero timer on missing");
}

void run_citadel_management_system_tests() {
    testCitadelInit();
    testCitadelInitTypes();
    testCitadelInitFails();
    testCitadelSetOwner();
    testCitadelServices();
    testCitadelServiceCapacity();
    testCitadelServiceOnlineToggle();
    testCitadelFuel();
    testCitadelFuelConsumption();
    testCitadelDamage();
    testCitadelDestruction();
    testCitadelReinforcement();
    testCitadelRepair();
    testCitadelCannotRepairDestroyed();
    testCitadelMissing();
}
