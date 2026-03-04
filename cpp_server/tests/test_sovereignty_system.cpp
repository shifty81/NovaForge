// Tests for: Sovereignty System Tests
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/sovereignty_system.h"

using namespace atlas;

// ========== Sovereignty System Tests ==========

static void testSovereigntyClaim() {
    std::cout << "\n=== Sovereignty Claim ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    assertTrue(sovSys.claimSovereignty("system_alpha", "alliance_1", "Alpha System"),
               "Sovereignty claimed");
    assertTrue(sovSys.getOwner("system_alpha") == "alliance_1",
               "Owner is alliance_1");
    assertTrue(sovSys.getControlLevel("system_alpha") > 0.0f,
               "Control level > 0");
}

static void testSovereigntyRelinquish() {
    std::cout << "\n=== Sovereignty Relinquish ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_beta", "alliance_2", "Beta System");

    assertTrue(!sovSys.relinquishSovereignty("system_beta", "wrong_owner"),
               "Non-owner cannot relinquish");
    assertTrue(sovSys.relinquishSovereignty("system_beta", "alliance_2"),
               "Owner relinquishes");
    assertTrue(sovSys.getOwner("system_beta").empty(),
               "No owner after relinquish");
}

static void testSovereigntyContest() {
    std::cout << "\n=== Sovereignty Contest ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_gamma", "alliance_3", "Gamma System");

    assertTrue(sovSys.contestSovereignty("system_gamma"),
               "System contested");

    auto* sov = world.getEntity("system_gamma")->getComponent<components::Sovereignty>();
    assertTrue(sov->is_contested, "System is contested");
}

static void testSovereigntyUpdateIndices() {
    std::cout << "\n=== Sovereignty Update Indices ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_delta", "alliance_4", "Delta System");

    assertTrue(sovSys.updateIndices("system_delta", 2.0f, 3.0f),
               "Indices updated");

    auto* sov = world.getEntity("system_delta")->getComponent<components::Sovereignty>();
    assertTrue(approxEqual(sov->military_index, 2.0f), "Military index is 2.0");
    assertTrue(approxEqual(sov->industrial_index, 3.0f), "Industrial index is 3.0");

    // Clamp to max
    sovSys.updateIndices("system_delta", 10.0f, 10.0f);
    assertTrue(approxEqual(sov->military_index, 5.0f), "Military index clamped to 5.0");
    assertTrue(approxEqual(sov->industrial_index, 5.0f), "Industrial index clamped to 5.0");
}

static void testSovereigntyUpgrade() {
    std::cout << "\n=== Sovereignty Upgrade ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_eps", "alliance_5", "Epsilon System");

    assertTrue(!sovSys.upgradeInfrastructure("system_eps", "wrong_owner"),
               "Non-owner cannot upgrade");
    assertTrue(sovSys.upgradeInfrastructure("system_eps", "alliance_5"),
               "Owner upgrades infrastructure");

    auto* sov = world.getEntity("system_eps")->getComponent<components::Sovereignty>();
    assertTrue(sov->upgrade_level == 1, "Upgrade level is 1");
}

static void testSovereigntyMaxUpgrade() {
    std::cout << "\n=== Sovereignty Max Upgrade ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_zeta", "alliance_6", "Zeta System");

    for (int i = 0; i < 5; i++) {
        assertTrue(sovSys.upgradeInfrastructure("system_zeta", "alliance_6"),
                   "Upgrade succeeds");
    }

    assertTrue(!sovSys.upgradeInfrastructure("system_zeta", "alliance_6"),
               "Cannot upgrade beyond 5");

    auto* sov = world.getEntity("system_zeta")->getComponent<components::Sovereignty>();
    assertTrue(sov->upgrade_level == 5, "Upgrade level capped at 5");
}

static void testSovereigntyUpdateDecay() {
    std::cout << "\n=== Sovereignty Update Decay ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_eta", "alliance_7", "Eta System");

    auto* sov = world.getEntity("system_eta")->getComponent<components::Sovereignty>();
    sov->control_level = 0.5f;
    sov->is_contested = true;
    sov->military_index = 3.0f;
    sov->industrial_index = 2.0f;

    // Simulate 1 hour (3600 seconds)
    sovSys.update(3600.0f);

    assertTrue(sov->control_level < 0.5f, "Contested control level decayed");
    assertTrue(sov->military_index < 3.0f, "Military index decayed");
    assertTrue(sov->industrial_index < 2.0f, "Industrial index decayed");
}

static void testSovereigntyCannotClaimOwned() {
    std::cout << "\n=== Sovereignty Cannot Claim Owned ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_theta", "alliance_8", "Theta System");

    assertTrue(!sovSys.claimSovereignty("system_theta", "alliance_9", "Theta System"),
               "Cannot claim already-owned system");
}


void run_sovereignty_system_tests() {
    testSovereigntyClaim();
    testSovereigntyRelinquish();
    testSovereigntyContest();
    testSovereigntyUpdateIndices();
    testSovereigntyUpgrade();
    testSovereigntyMaxUpgrade();
    testSovereigntyUpdateDecay();
    testSovereigntyCannotClaimOwned();
}
