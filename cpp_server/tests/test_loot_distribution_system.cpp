// Tests for: LootDistributionSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/loot_distribution_system.h"

using namespace atlas;

using LD = components::LootDistribution;

// ==================== LootDistributionSystem Tests ====================

static void testLootDistInit() {
    std::cout << "\n=== LootDist: Init ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    assertTrue(sys.initialize("wreck1"), "Init succeeds");
    assertTrue(sys.getState("wreck1") == LD::State::Idle, "Initial state Idle");
    assertTrue(sys.getParticipantCount("wreck1") == 0, "Zero participants initially");
    assertTrue(sys.getItemCount("wreck1") == 0, "Zero items initially");
    assertTrue(approxEqual(sys.getIskPool("wreck1"), 0.0f), "Zero ISK pool initially");
    assertTrue(sys.getTotalDistributions("wreck1") == 0, "Zero distributions initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testLootDistOpen() {
    std::cout << "\n=== LootDist: Open ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    sys.initialize("wreck1");

    assertTrue(sys.openDistribution("wreck1"), "Open succeeds");
    assertTrue(sys.getState("wreck1") == LD::State::Open, "State is Open");
    assertTrue(!sys.openDistribution("wreck1"), "Cannot re-open while already open");
}

static void testLootDistAddParticipant() {
    std::cout << "\n=== LootDist: AddParticipant ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    sys.initialize("wreck1");
    sys.openDistribution("wreck1");

    assertTrue(sys.addParticipant("wreck1", "pilot_alice", 3000.0f),
               "Add first participant");
    assertTrue(sys.addParticipant("wreck1", "pilot_bob", 1000.0f),
               "Add second participant");
    assertTrue(sys.getParticipantCount("wreck1") == 2, "2 participants stored");
    assertTrue(!sys.addParticipant("wreck1", "pilot_alice", 100.0f),
               "Duplicate pilot rejected");
    assertTrue(!sys.addParticipant("wreck1", "", 100.0f),
               "Empty pilot ID rejected");
    assertTrue(!sys.addParticipant("wreck1", "pilot_neg", -1.0f),
               "Negative damage rejected");
    assertTrue(sys.getParticipantCount("wreck1") == 2, "Count unchanged after rejections");
}

static void testLootDistRemoveParticipant() {
    std::cout << "\n=== LootDist: RemoveParticipant ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    sys.initialize("wreck1");
    sys.openDistribution("wreck1");
    sys.addParticipant("wreck1", "pilot_alice", 3000.0f);
    sys.addParticipant("wreck1", "pilot_bob",   1000.0f);

    assertTrue(sys.removeParticipant("wreck1", "pilot_alice"), "Remove existing participant");
    assertTrue(sys.getParticipantCount("wreck1") == 1, "1 participant remaining");
    assertTrue(!sys.removeParticipant("wreck1", "pilot_alice"), "Remove nonexistent fails");
    assertTrue(!sys.removeParticipant("wreck1", "pilot_charlie"), "Remove unknown fails");
}

static void testLootDistSetIskPool() {
    std::cout << "\n=== LootDist: SetIskPool ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    sys.initialize("wreck1");

    assertTrue(!sys.setIskPool("wreck1", 5000.0f), "SetIskPool fails when not open");
    sys.openDistribution("wreck1");
    assertTrue(sys.setIskPool("wreck1", 5000.0f), "SetIskPool succeeds when open");
    assertTrue(approxEqual(sys.getIskPool("wreck1"), 5000.0f), "ISK pool stored");
    assertTrue(!sys.setIskPool("wreck1", -1.0f), "Negative ISK rejected");
}

static void testLootDistAddItem() {
    std::cout << "\n=== LootDist: AddItem ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    sys.initialize("wreck1");

    assertTrue(!sys.addItem("wreck1", "tritanium", "Tritanium", 1000),
               "AddItem fails when not open");
    sys.openDistribution("wreck1");
    assertTrue(sys.addItem("wreck1", "tritanium", "Tritanium", 1000),
               "AddItem succeeds when open");
    assertTrue(sys.addItem("wreck1", "plex",      "PLEX",      1),
               "Add second item");
    assertTrue(sys.getItemCount("wreck1") == 2, "2 items stored");
    assertTrue(!sys.addItem("wreck1", "tritanium", "Dup", 500),
               "Duplicate item rejected");
    assertTrue(!sys.addItem("wreck1", "", "NoID", 1),
               "Empty item ID rejected");
    assertTrue(!sys.addItem("wreck1", "zero", "Zero", 0),
               "Zero quantity rejected");
    assertTrue(sys.getItemCount("wreck1") == 2, "Count unchanged after rejections");
}

static void testLootDistDistribute() {
    std::cout << "\n=== LootDist: Distribute ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    sys.initialize("wreck1");
    sys.openDistribution("wreck1");

    // alice dealt 75% of damage, bob 25%
    sys.addParticipant("wreck1", "pilot_alice", 3000.0f);
    sys.addParticipant("wreck1", "pilot_bob",   1000.0f);
    sys.setIskPool("wreck1", 10000.0f);

    assertTrue(sys.distribute("wreck1"), "Distribute succeeds");
    assertTrue(!sys.distribute("wreck1"), "Cannot distribute twice");
    assertTrue(sys.getState("wreck1") == LD::State::Distributed, "State is Distributed");
    assertTrue(sys.getTotalDistributions("wreck1") == 1, "1 distribution recorded");

    float alice_share = sys.getParticipantShare("wreck1", "pilot_alice");
    float bob_share   = sys.getParticipantShare("wreck1", "pilot_bob");
    assertTrue(approxEqual(alice_share, 7500.0f), "Alice gets 75% (7500 ISK)");
    assertTrue(approxEqual(bob_share,   2500.0f), "Bob gets 25% (2500 ISK)");
}

static void testLootDistItemAssignment() {
    std::cout << "\n=== LootDist: ItemAssignment ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    sys.initialize("wreck1");
    sys.openDistribution("wreck1");

    // alice has higher damage → gets first item
    sys.addParticipant("wreck1", "pilot_alice", 8000.0f);
    sys.addParticipant("wreck1", "pilot_bob",   2000.0f);
    sys.addItem("wreck1", "armor_plate", "Armor Plate", 1);
    sys.addItem("wreck1", "warp_drive",  "Warp Drive",  1);
    sys.addItem("wreck1", "cap_bat",     "Cap Battery", 2);

    sys.distribute("wreck1");

    auto* comp = world.getEntity("wreck1")->getComponent<components::LootDistribution>();
    assertTrue(comp->items[0].assigned_to == "pilot_alice", "Item 0 goes to alice (highest dmg)");
    assertTrue(comp->items[1].assigned_to == "pilot_bob",   "Item 1 goes to bob (round-robin)");
    assertTrue(comp->items[2].assigned_to == "pilot_alice", "Item 2 goes to alice (round-robin)");
}

static void testLootDistNoParticipants() {
    std::cout << "\n=== LootDist: NoParticipants ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    sys.initialize("wreck1");
    sys.openDistribution("wreck1");

    assertTrue(!sys.distribute("wreck1"), "Distribute fails with no participants");
    assertTrue(sys.getState("wreck1") == LD::State::Open, "State remains Open");
    assertTrue(sys.getTotalDistributions("wreck1") == 0, "No distributions counted");
}

static void testLootDistSingleParticipant() {
    std::cout << "\n=== LootDist: SingleParticipant ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);
    world.createEntity("wreck1");
    sys.initialize("wreck1");
    sys.openDistribution("wreck1");

    sys.addParticipant("wreck1", "solo_pilot", 5000.0f);
    sys.setIskPool("wreck1", 8000.0f);
    sys.addItem("wreck1", "module1", "Module Alpha", 1);

    assertTrue(sys.distribute("wreck1"), "Single-pilot distribution succeeds");
    assertTrue(approxEqual(sys.getParticipantShare("wreck1", "solo_pilot"), 8000.0f),
               "Solo pilot gets 100% ISK");

    auto* comp = world.getEntity("wreck1")->getComponent<components::LootDistribution>();
    assertTrue(comp->items[0].assigned_to == "solo_pilot", "Solo pilot gets the item");
}

static void testLootDistMissing() {
    std::cout << "\n=== LootDist: Missing ===" << std::endl;
    ecs::World world;
    systems::LootDistributionSystem sys(&world);

    assertTrue(!sys.openDistribution("nx"), "Open fails on missing entity");
    assertTrue(!sys.distribute("nx"), "Distribute fails on missing entity");
    assertTrue(!sys.addParticipant("nx", "p1", 100.0f), "AddParticipant fails on missing");
    assertTrue(!sys.removeParticipant("nx", "p1"), "RemoveParticipant fails on missing");
    assertTrue(!sys.setIskPool("nx", 1000.0f), "SetIskPool fails on missing");
    assertTrue(!sys.addItem("nx", "i1", "Item", 1), "AddItem fails on missing");
    assertTrue(sys.getState("nx") == LD::State::Idle, "Idle state on missing");
    assertTrue(sys.getParticipantCount("nx") == 0, "0 participants on missing");
    assertTrue(sys.getItemCount("nx") == 0, "0 items on missing");
    assertTrue(approxEqual(sys.getIskPool("nx"), 0.0f), "0 ISK on missing");
    assertTrue(approxEqual(sys.getParticipantShare("nx", "p1"), 0.0f), "0 share on missing");
    assertTrue(sys.getTotalDistributions("nx") == 0, "0 distributions on missing");
}

void run_loot_distribution_system_tests() {
    testLootDistInit();
    testLootDistOpen();
    testLootDistAddParticipant();
    testLootDistRemoveParticipant();
    testLootDistSetIskPool();
    testLootDistAddItem();
    testLootDistDistribute();
    testLootDistItemAssignment();
    testLootDistNoParticipants();
    testLootDistSingleParticipant();
    testLootDistMissing();
}
