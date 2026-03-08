// Tests for: StationDeploymentSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/station_deployment_system.h"

using namespace atlas;

// ==================== StationDeploymentSystem Tests ====================

static void testStationDeployBegin() {
    std::cout << "\n=== StationDeployment: Begin ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.beginDeployment("ship1", "sol", 100.0f, 200.0f, 50.0f), "Begin deployment succeeds");
    assertTrue(sys.isDeploying("ship1"), "Ship is deploying");
    assertTrue(!sys.isDeployed("ship1"), "Ship is not yet deployed");
}

static void testStationDeployBeginMissing() {
    std::cout << "\n=== StationDeployment: BeginMissing ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);

    assertTrue(!sys.beginDeployment("missing", "sol", 0.0f, 0.0f, 0.0f), "Missing entity fails");
}

static void testStationDeployCompletion() {
    std::cout << "\n=== StationDeployment: Completion ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    sys.beginDeployment("ship1", "sol", 10.0f, 20.0f, 30.0f);
    assertTrue(sys.isDeploying("ship1"), "Deploying after begin");

    // Tick down the timer (default deploy_duration=300s)
    sys.update(299.0f);
    assertTrue(sys.isDeploying("ship1"), "Still deploying at 299s");

    sys.update(2.0f); // 301s total > 300s
    assertTrue(sys.isDeployed("ship1"), "Deployed after timer expires");
    assertTrue(!sys.isDeploying("ship1"), "No longer deploying");
}

static void testStationDeployCancel() {
    std::cout << "\n=== StationDeployment: Cancel ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    sys.beginDeployment("ship1", "sol", 0.0f, 0.0f, 0.0f);
    assertTrue(sys.isDeploying("ship1"), "Deploying after begin");

    sys.cancelDeployment("ship1");
    assertTrue(!sys.isDeploying("ship1"), "Not deploying after cancel");
    assertTrue(!sys.isDeployed("ship1"), "Not deployed after cancel");

    // Can begin again after cancel
    assertTrue(sys.beginDeployment("ship1", "alpha", 5.0f, 5.0f, 5.0f), "Re-deploy after cancel");
    assertTrue(sys.isDeploying("ship1"), "Deploying again");
}

static void testStationDeployDoubleBegin() {
    std::cout << "\n=== StationDeployment: DoubleBegin ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    assertTrue(sys.beginDeployment("ship1", "sol", 0.0f, 0.0f, 0.0f), "First begin succeeds");
    assertTrue(!sys.beginDeployment("ship1", "alpha", 1.0f, 1.0f, 1.0f), "Second begin fails (already deploying)");
}

static void testStationDeployAlreadyDeployed() {
    std::cout << "\n=== StationDeployment: AlreadyDeployed ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    sys.beginDeployment("ship1", "sol", 0.0f, 0.0f, 0.0f);
    sys.update(301.0f); // complete deployment
    assertTrue(sys.isDeployed("ship1"), "Deployed");
    assertTrue(!sys.beginDeployment("ship1", "alpha", 1.0f, 1.0f, 1.0f), "Cannot begin when already deployed");
}

static void testStationAttachModule() {
    std::cout << "\n=== StationDeployment: AttachModule ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    sys.beginDeployment("ship1", "sol", 0.0f, 0.0f, 0.0f);
    sys.update(301.0f);
    assertTrue(sys.isDeployed("ship1"), "Deployed");

    assertTrue(sys.attachModule("ship1", "security"), "Attach security module");
    assertTrue(sys.getAttachedModuleCount("ship1") == 1, "1 module attached");

    assertTrue(sys.attachModule("ship1", "market"), "Attach market module");
    assertTrue(sys.getAttachedModuleCount("ship1") == 2, "2 modules attached");

    assertTrue(sys.attachModule("ship1", "refinery"), "Attach refinery module");
    assertTrue(sys.getAttachedModuleCount("ship1") == 3, "3 modules attached (max)");

    // Max reached
    assertTrue(!sys.attachModule("ship1", "security"), "Cannot attach beyond max");
    assertTrue(sys.getAttachedModuleCount("ship1") == 3, "Still 3 modules");
}

static void testStationAttachBeforeDeployed() {
    std::cout << "\n=== StationDeployment: AttachBeforeDeployed ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    sys.beginDeployment("ship1", "sol", 0.0f, 0.0f, 0.0f);
    assertTrue(!sys.attachModule("ship1", "security"), "Cannot attach while deploying");

    // Also try on Mobile state
    sys.cancelDeployment("ship1");
    assertTrue(!sys.attachModule("ship1", "security"), "Cannot attach in Mobile state");
}

static void testStationAttachSameType() {
    std::cout << "\n=== StationDeployment: AttachSameType ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    sys.beginDeployment("ship1", "sol", 0.0f, 0.0f, 0.0f);
    sys.update(301.0f);

    assertTrue(sys.attachModule("ship1", "security"), "Attach 1st security");
    assertTrue(sys.attachModule("ship1", "security"), "Attach 2nd security");
    assertTrue(sys.getAttachedModuleCount("ship1") == 2, "2 same-type modules");
}

static void testStationModuleBonuses() {
    std::cout << "\n=== StationDeployment: ModuleBonuses ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    sys.beginDeployment("ship1", "sol", 0.0f, 0.0f, 0.0f);
    sys.update(301.0f);

    float sec = 0, eco = 0, res = 0;
    sys.getSystemBonuses("ship1", sec, eco, res);
    assertTrue(sec == 0.0f, "Initial security bonus is 0");
    assertTrue(eco == 0.0f, "Initial economy bonus is 0");
    assertTrue(res == 0.0f, "Initial resource bonus is 0");

    sys.attachModule("ship1", "security");
    sys.getSystemBonuses("ship1", sec, eco, res);
    assertTrue(approxEqual(sec, 0.05f), "Security bonus after module");

    sys.attachModule("ship1", "market");
    sys.getSystemBonuses("ship1", sec, eco, res);
    assertTrue(approxEqual(eco, 0.10f), "Economy bonus after market");

    sys.attachModule("ship1", "refinery");
    sys.getSystemBonuses("ship1", sec, eco, res);
    assertTrue(approxEqual(res, 0.08f), "Resource bonus after refinery");
}

static void testStationCancelWhileDeployed() {
    std::cout << "\n=== StationDeployment: CancelWhileDeployed ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    world.createEntity("ship1");

    sys.beginDeployment("ship1", "sol", 0.0f, 0.0f, 0.0f);
    sys.update(301.0f);
    assertTrue(sys.isDeployed("ship1"), "Deployed");

    // Cancel on deployed should not revert (only works during Deploying)
    sys.cancelDeployment("ship1");
    assertTrue(sys.isDeployed("ship1"), "Still deployed after cancel attempt");
}

static void testStationMissingEntity() {
    std::cout << "\n=== StationDeployment: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);

    assertTrue(!sys.isDeployed("x"), "isDeployed false on missing");
    assertTrue(!sys.isDeploying("x"), "isDeploying false on missing");
    assertTrue(!sys.attachModule("x", "security"), "attachModule fails on missing");
    assertTrue(sys.getAttachedModuleCount("x") == 0, "Module count 0 on missing");

    float s = -1, e = -1, r = -1;
    sys.getSystemBonuses("x", s, e, r);
    assertTrue(s == 0.0f, "Security bonus 0 on missing");
    assertTrue(e == 0.0f, "Economy bonus 0 on missing");
    assertTrue(r == 0.0f, "Resource bonus 0 on missing");
}

void run_station_deployment_system_tests() {
    testStationDeployBegin();
    testStationDeployBeginMissing();
    testStationDeployCompletion();
    testStationDeployCancel();
    testStationDeployDoubleBegin();
    testStationDeployAlreadyDeployed();
    testStationAttachModule();
    testStationAttachBeforeDeployed();
    testStationAttachSameType();
    testStationModuleBonuses();
    testStationCancelWhileDeployed();
    testStationMissingEntity();
}
