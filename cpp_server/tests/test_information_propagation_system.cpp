// Tests for: Information Propagation System tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/narrative_components.h"
#include "ecs/system.h"
#include "systems/information_propagation_system.h"

using namespace atlas;

// ==================== Information Propagation System tests ====================

static void testInfoPropReportAction() {
    std::cout << "\n=== Info Prop Report Action ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    sys1->addComponent(std::make_unique<components::InformationPropagation>());

    infoSys.reportPlayerAction("system1", "player1", "combat");
    assertTrue(infoSys.getRumorCount("system1") == 1, "One rumor reported");
    auto rumors = infoSys.getRumors("system1");
    assertTrue(rumors[0].player_id == "player1", "Rumor tracks correct player");
    assertTrue(rumors[0].action_type == "combat", "Rumor tracks correct action");
    assertTrue(rumors[0].personally_witnessed == true, "Rumor is witnessed");
}

static void testInfoPropDecay() {
    std::cout << "\n=== Info Prop Decay ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    auto infoProp = std::make_unique<components::InformationPropagation>();
    infoProp->decay_rate = 0.1f;
    infoProp->max_rumor_age = 300.0f;
    sys1->addComponent(std::move(infoProp));

    // Add a non-witnessed rumor
    auto* info = sys1->getComponent<components::InformationPropagation>();
    info->addRumor("rumor1", "player1", "piracy", "system1", false);
    float initial = info->rumors[0].belief_strength;

    infoSys.update(1.0f);  // 1 second
    assertTrue(info->rumors[0].belief_strength < initial, "Belief decayed for non-witnessed");
}

static void testInfoPropPropagation() {
    std::cout << "\n=== Info Prop Propagation ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    auto info1 = std::make_unique<components::InformationPropagation>();
    info1->neighbor_system_ids.push_back("system2");
    info1->propagation_interval = 1.0f;  // propagate every 1s for testing
    sys1->addComponent(std::move(info1));

    auto* sys2 = world.createEntity("system2");
    sys2->addComponent(std::make_unique<components::InformationPropagation>());

    infoSys.reportPlayerAction("system1", "player1", "combat");
    assertTrue(infoSys.getRumorCount("system1") == 1, "System1 has rumor");
    assertTrue(infoSys.getRumorCount("system2") == 0, "System2 has no rumor yet");

    infoSys.update(1.0f);  // trigger propagation
    assertTrue(infoSys.getRumorCount("system2") == 1, "Rumor propagated to system2");

    auto rumors2 = infoSys.getRumors("system2");
    assertTrue(rumors2[0].hops == 1, "Propagated rumor has hop count 1");
    assertTrue(rumors2[0].belief_strength < 1.0f, "Propagated rumor has reduced belief");
    assertTrue(rumors2[0].personally_witnessed == false, "Propagated rumor not witnessed");
}

static void testInfoPropPlayerNotoriety() {
    std::cout << "\n=== Info Prop Player Notoriety ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    sys1->addComponent(std::make_unique<components::InformationPropagation>());

    infoSys.reportPlayerAction("system1", "player1", "combat");
    infoSys.reportPlayerAction("system1", "player1", "piracy");
    float notoriety = infoSys.getPlayerNotoriety("system1", "player1");
    assertTrue(notoriety > 0.0f, "Player has notoriety");
    assertTrue(infoSys.getRumorCount("system1") == 2, "Two rumors about player");
}

static void testInfoPropMaxHops() {
    std::cout << "\n=== Info Prop Max Hops ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    auto info1 = std::make_unique<components::InformationPropagation>();
    info1->neighbor_system_ids.push_back("system2");
    info1->propagation_interval = 1.0f;
    info1->max_hops = 1;
    sys1->addComponent(std::move(info1));

    auto* sys2 = world.createEntity("system2");
    auto info2 = std::make_unique<components::InformationPropagation>();
    info2->neighbor_system_ids.push_back("system3");
    info2->propagation_interval = 1.0f;
    info2->max_hops = 1;
    sys2->addComponent(std::move(info2));

    auto* sys3 = world.createEntity("system3");
    sys3->addComponent(std::make_unique<components::InformationPropagation>());

    infoSys.reportPlayerAction("system1", "player1", "combat");
    infoSys.update(1.0f);  // propagate to system2
    assertTrue(infoSys.getRumorCount("system2") == 1, "Propagated to system2");

    infoSys.update(1.0f);  // try to propagate from system2 to system3
    assertTrue(infoSys.getRumorCount("system3") == 0, "Stopped at max hops");
}

static void testInfoPropExpiry() {
    std::cout << "\n=== Info Prop Expiry ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    auto infoProp = std::make_unique<components::InformationPropagation>();
    infoProp->max_rumor_age = 5.0f;
    sys1->addComponent(std::move(infoProp));

    auto* info = sys1->getComponent<components::InformationPropagation>();
    info->addRumor("rumor1", "player1", "combat", "system1", true);
    assertTrue(info->getRumorCount() == 1, "Rumor exists");

    infoSys.update(6.0f);  // age past expiry
    assertTrue(info->getRumorCount() == 0, "Rumor expired and removed");
}


void run_information_propagation_system_tests() {
    testInfoPropReportAction();
    testInfoPropDecay();
    testInfoPropPropagation();
    testInfoPropPlayerNotoriety();
    testInfoPropMaxHops();
    testInfoPropExpiry();
}
