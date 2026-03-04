// Tests for: Phase 11: Station Deployment System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/station_deployment_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Phase 11: Station Deployment System Tests ====================

static void testStationDeploymentBegin() {
    std::cout << "\n=== Station Deployment Begin ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    addComp<components::StationDeployment>(ship);
    bool ok = sys.beginDeployment("deployer1", "system_a", 100.0f, 200.0f, 300.0f);
    assertTrue(ok, "Deployment started successfully");
    assertTrue(sys.isDeploying("deployer1"), "Ship is deploying");
    assertTrue(!sys.isDeployed("deployer1"), "Ship is not yet deployed");
}

static void testStationDeploymentComplete() {
    std::cout << "\n=== Station Deployment Complete ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    auto* dep = addComp<components::StationDeployment>(ship);
    dep->deploy_duration = 10.0f; // short for testing
    sys.beginDeployment("deployer1", "system_a", 100.0f, 200.0f, 300.0f);
    sys.update(5.0f);
    assertTrue(sys.isDeploying("deployer1"), "Still deploying at 5s");
    sys.update(6.0f);
    assertTrue(sys.isDeployed("deployer1"), "Deployed after 11s total");
}

static void testStationDeploymentCancel() {
    std::cout << "\n=== Station Deployment Cancel ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    addComp<components::StationDeployment>(ship);
    sys.beginDeployment("deployer1", "system_a", 0, 0, 0);
    sys.cancelDeployment("deployer1");
    assertTrue(!sys.isDeploying("deployer1"), "Not deploying after cancel");
    assertTrue(!sys.isDeployed("deployer1"), "Not deployed after cancel");
}

static void testStationDeploymentAttachModule() {
    std::cout << "\n=== Station Deployment Attach Module ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    auto* dep = addComp<components::StationDeployment>(ship);
    dep->deploy_state = components::StationDeployment::DeployState::Deployed;
    bool ok = sys.attachModule("deployer1", "security");
    assertTrue(ok, "Module attached successfully");
    assertTrue(sys.getAttachedModuleCount("deployer1") == 1, "1 module attached");
}

static void testStationDeploymentModuleLimit() {
    std::cout << "\n=== Station Deployment Module Limit ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    auto* dep = addComp<components::StationDeployment>(ship);
    dep->deploy_state = components::StationDeployment::DeployState::Deployed;
    dep->max_module_slots = 2;
    sys.attachModule("deployer1", "security");
    sys.attachModule("deployer1", "market");
    bool ok = sys.attachModule("deployer1", "refinery");
    assertTrue(!ok, "Cannot attach beyond max module slots");
    assertTrue(sys.getAttachedModuleCount("deployer1") == 2, "Still 2 modules");
}

static void testStationDeploymentSystemBonuses() {
    std::cout << "\n=== Station Deployment System Bonuses ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    auto* dep = addComp<components::StationDeployment>(ship);
    dep->deploy_state = components::StationDeployment::DeployState::Deployed;
    sys.attachModule("deployer1", "security");
    sys.attachModule("deployer1", "market");
    float sec = 0.0f, econ = 0.0f, res = 0.0f;
    sys.getSystemBonuses("deployer1", sec, econ, res);
    assertTrue(approxEqual(sec, 0.05f), "Security bonus from security module");
    assertTrue(approxEqual(econ, 0.10f), "Economy bonus from market module");
    assertTrue(approxEqual(res, 0.0f), "No resource bonus without refinery");
}

static void testStationDeploymentNotDeployedCantAttach() {
    std::cout << "\n=== Station Deployment Not Deployed Can't Attach ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    addComp<components::StationDeployment>(ship);
    bool ok = sys.attachModule("deployer1", "security");
    assertTrue(!ok, "Cannot attach module to non-deployed station");
}


void run_phase_11_station_deployment_system_tests() {
    testStationDeploymentBegin();
    testStationDeploymentComplete();
    testStationDeploymentCancel();
    testStationDeploymentAttachModule();
    testStationDeploymentModuleLimit();
    testStationDeploymentSystemBonuses();
    testStationDeploymentNotDeployedCantAttach();
}
