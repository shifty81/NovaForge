// Tests for: Ancient Tech Upgrade System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/ancient_tech_upgrade_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Ancient Tech Upgrade System Tests ====================

static void testAncientTechUpgradeDefaults() {
    std::cout << "\n=== Ancient Tech Upgrade Defaults ===" << std::endl;
    ecs::World world;
    systems::AncientTechUpgradeSystem upgSys(&world);
    assertTrue(upgSys.getUpgradingCount() == 0, "No modules upgrading initially");
    assertTrue(upgSys.getUpgradedCount() == 0, "No modules upgraded initially");
}

static void testAncientTechUpgradeStart() {
    std::cout << "\n=== Ancient Tech Upgrade Start ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient1");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;
    tech->tech_type = "shield";
    tech->power_multiplier = 1.8f;

    systems::AncientTechUpgradeSystem upgSys(&world);
    assertTrue(upgSys.startUpgrade("ancient1"), "Start upgrade succeeds on Repaired module");
    assertTrue(upgSys.getUpgradingCount() == 1, "One module upgrading");
    assertTrue(!upgSys.hasRuleBreakingBonuses("ancient1"), "Not yet upgraded");
}

static void testAncientTechUpgradeNotRepaired() {
    std::cout << "\n=== Ancient Tech Upgrade Not Repaired ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient2");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Broken;

    systems::AncientTechUpgradeSystem upgSys(&world);
    assertTrue(!upgSys.startUpgrade("ancient2"), "Cannot upgrade Broken module");
}

static void testAncientTechUpgradeComplete() {
    std::cout << "\n=== Ancient Tech Upgrade Complete ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient3");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;
    tech->tech_type = "weapon";
    tech->power_multiplier = 1.5f;
    auto* upg = addComp<components::AncientTechUpgradeState>(e);
    upg->upgrade_cost = 10.0f;

    systems::AncientTechUpgradeSystem upgSys(&world);
    assertTrue(upgSys.startUpgrade("ancient3"), "Start upgrade");

    upgSys.update(11.0f); // More than upgrade_cost
    assertTrue(tech->state == components::AncientTechModule::TechState::Upgraded, "Module is now Upgraded");
    assertTrue(upgSys.hasRuleBreakingBonuses("ancient3"), "Has rule-breaking bonuses");
    assertTrue(upgSys.getUpgradedCount() == 1, "One upgraded module");
    assertTrue(approxEqual(upgSys.getStatMultiplier("ancient3"), 1.5f), "Stat multiplier matches power_multiplier");
}

static void testAncientTechUpgradeCancel() {
    std::cout << "\n=== Ancient Tech Upgrade Cancel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient4");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;

    systems::AncientTechUpgradeSystem upgSys(&world);
    upgSys.startUpgrade("ancient4");
    assertTrue(upgSys.cancelUpgrade("ancient4"), "Cancel succeeds");
    assertTrue(upgSys.getUpgradingCount() == 0, "No modules upgrading after cancel");
    assertTrue(tech->state == components::AncientTechModule::TechState::Repaired, "State back to Repaired");
}

static void testAncientTechUpgradeProgress() {
    std::cout << "\n=== Ancient Tech Upgrade Progress ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient5");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;
    auto* upg = addComp<components::AncientTechUpgradeState>(e);
    upg->upgrade_cost = 100.0f;

    systems::AncientTechUpgradeSystem upgSys(&world);
    upgSys.startUpgrade("ancient5");
    upgSys.update(50.0f);
    assertTrue(approxEqual(upgSys.getUpgradeProgress("ancient5"), 0.5f), "50% progress after half time");
}

static void testAncientTechUpgradeBonusDescription() {
    std::cout << "\n=== Ancient Tech Upgrade Bonus Description ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient6");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Upgraded;
    tech->tech_type = "shield";
    tech->power_multiplier = 1.8f;

    systems::AncientTechUpgradeSystem upgSys(&world);
    std::string desc = upgSys.getBonusDescription("ancient6");
    assertTrue(!desc.empty(), "Bonus description not empty for upgraded module");
    assertTrue(desc.find("1.8") != std::string::npos, "Description contains multiplier value");
}


void run_ancient_tech_upgrade_system_tests() {
    testAncientTechUpgradeDefaults();
    testAncientTechUpgradeStart();
    testAncientTechUpgradeNotRepaired();
    testAncientTechUpgradeComplete();
    testAncientTechUpgradeCancel();
    testAncientTechUpgradeProgress();
    testAncientTechUpgradeBonusDescription();
}
