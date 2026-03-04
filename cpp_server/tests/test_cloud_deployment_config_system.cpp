// Tests for: CloudDeploymentConfig System Tests
#include "test_log.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/cloud_deployment_config_system.h"

using namespace atlas;

// ==================== CloudDeploymentConfig System Tests ====================

static void testCloudDeploymentCreate() {
    std::cout << "\n=== CloudDeploymentConfig: Create ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    auto* e = world.createEntity("cloud1");
    assertTrue(sys.createConfig("cloud1"), "Create config succeeds");
    auto* cfg = e->getComponent<components::CloudDeploymentConfig>();
    assertTrue(cfg != nullptr, "Component exists");
    assertTrue(cfg->active, "Config is active by default");
    assertTrue(cfg->provider == 0, "Default provider is AWS (0)");
    assertTrue(cfg->max_players == 20, "Default max players is 20");
    assertTrue(!cfg->deployed, "Not deployed by default");
}

static void testCloudDeploymentProvider() {
    std::cout << "\n=== CloudDeploymentConfig: Provider ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    auto* e = world.createEntity("cloud1");
    sys.createConfig("cloud1");
    assertTrue(sys.setProvider("cloud1", 1), "Set GCP succeeds");
    assertTrue(sys.getProvider("cloud1") == 1, "Provider is GCP (1)");
    auto* cfg = e->getComponent<components::CloudDeploymentConfig>();
    assertTrue(cfg->estimated_monthly_cost > 0.0f, "Cost calculated");
    assertTrue(!sys.setProvider("cloud1", 5), "Invalid provider rejected");
}

static void testCloudDeploymentRegion() {
    std::cout << "\n=== CloudDeploymentConfig: Region ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    assertTrue(sys.setRegion("cloud1", "us-east-1"), "Set region succeeds");
    assertTrue(sys.getRegion("cloud1") == "us-east-1", "Region stored");
    assertTrue(!sys.setRegion("cloud1", ""), "Empty region rejected");
}

static void testCloudDeploymentValidate() {
    std::cout << "\n=== CloudDeploymentConfig: Validate ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    assertTrue(!sys.validate("cloud1"), "Empty config fails validation");
    sys.setRegion("cloud1", "eu-west-1");
    assertTrue(!sys.validate("cloud1"), "Missing instance type fails");
    sys.setInstanceType("cloud1", "t3.medium");
    assertTrue(sys.validate("cloud1"), "Complete config validates");
}

static void testCloudDeploymentDeploy() {
    std::cout << "\n=== CloudDeploymentConfig: Deploy ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    assertTrue(!sys.deploy("cloud1"), "Deploy fails without valid config");
    sys.setRegion("cloud1", "us-west-2");
    sys.setInstanceType("cloud1", "c5.large");
    assertTrue(sys.deploy("cloud1"), "Deploy succeeds with valid config");
    assertTrue(sys.isDeployed("cloud1"), "Is deployed");
}

static void testCloudDeploymentUptime() {
    std::cout << "\n=== CloudDeploymentConfig: Uptime ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    sys.setRegion("cloud1", "us-east-1");
    sys.setInstanceType("cloud1", "t3.large");
    sys.deploy("cloud1");
    sys.update(10.0f);
    assertTrue(sys.getUptime("cloud1") > 9.0f, "Uptime tracked");
}

static void testCloudDeploymentHealthCheck() {
    std::cout << "\n=== CloudDeploymentConfig: HealthCheck ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    sys.setRegion("cloud1", "us-east-1");
    sys.setInstanceType("cloud1", "t3.large");
    sys.enableHealthCheck("cloud1", 10.0f);
    sys.deploy("cloud1");
    sys.update(25.0f);
    assertTrue(sys.getHealthCheckCount("cloud1") == 2, "2 health checks after 25s with 10s interval");
}

static void testCloudDeploymentMaxPlayers() {
    std::cout << "\n=== CloudDeploymentConfig: MaxPlayers ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    auto* e = world.createEntity("cloud1");
    sys.createConfig("cloud1");
    sys.setMaxPlayers("cloud1", 50);
    assertTrue(sys.getMaxPlayers("cloud1") == 50, "Max players set to 50");
    auto* cfg = e->getComponent<components::CloudDeploymentConfig>();
    assertTrue(cfg->estimated_monthly_cost > 0.0f, "Cost recalculated");
    sys.setMaxPlayers("cloud1", 200);
    assertTrue(sys.getMaxPlayers("cloud1") == 100, "Max clamped to 100");
}

static void testCloudDeploymentCost() {
    std::cout << "\n=== CloudDeploymentConfig: Cost ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    sys.setProvider("cloud1", 0);  // AWS
    sys.setMaxPlayers("cloud1", 20);
    float aws_cost = sys.getEstimatedMonthlyCost("cloud1");
    assertTrue(aws_cost > 0.0f, "AWS cost is positive");
    sys.setProvider("cloud1", 1);  // GCP
    float gcp_cost = sys.getEstimatedMonthlyCost("cloud1");
    assertTrue(gcp_cost > 0.0f, "GCP cost is positive");
    assertTrue(gcp_cost != aws_cost, "Different providers have different costs");
}

static void testCloudDeploymentMissing() {
    std::cout << "\n=== CloudDeploymentConfig: Missing ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    assertTrue(!sys.createConfig("nonexistent"), "Create fails on missing");
    assertTrue(!sys.setProvider("nonexistent", 0), "SetProvider fails on missing");
    assertTrue(!sys.setRegion("nonexistent", "us-east-1"), "SetRegion fails on missing");
    assertTrue(!sys.setInstanceType("nonexistent", "t3"), "SetInstanceType fails on missing");
    assertTrue(!sys.setMaxPlayers("nonexistent", 20), "SetMaxPlayers fails on missing");
    assertTrue(!sys.validate("nonexistent"), "Validate fails on missing");
    assertTrue(!sys.deploy("nonexistent"), "Deploy fails on missing");
    assertTrue(sys.getProvider("nonexistent") == -1, "-1 provider on missing");
    assertTrue(sys.getMaxPlayers("nonexistent") == 0, "0 max players on missing");
    assertTrue(!sys.isDeployed("nonexistent"), "Not deployed on missing");
}


void run_cloud_deployment_config_system_tests() {
    testCloudDeploymentCreate();
    testCloudDeploymentProvider();
    testCloudDeploymentRegion();
    testCloudDeploymentValidate();
    testCloudDeploymentDeploy();
    testCloudDeploymentUptime();
    testCloudDeploymentHealthCheck();
    testCloudDeploymentMaxPlayers();
    testCloudDeploymentCost();
    testCloudDeploymentMissing();
}
