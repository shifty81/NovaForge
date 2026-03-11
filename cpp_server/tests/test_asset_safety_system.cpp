// Tests for: AssetSafetySystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/asset_safety_system.h"

using namespace atlas;

// ==================== AssetSafetySystem Tests ====================

static void testSafetyInit() {
    std::cout << "\n=== Safety: Init ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initialize("player1", "owner1"), "Init succeeds");
    assertTrue(sys.getEntryCount("player1") == 0, "Zero entries initially");
    assertTrue(sys.getTotalTriggered("player1") == 0, "Zero triggered initially");
    assertTrue(sys.getTotalClaimed("player1") == 0, "Zero claimed initially");
    assertTrue(sys.getOwner("player1") == "owner1", "Owner stored");
}

static void testSafetyInitFails() {
    std::cout << "\n=== Safety: InitFails ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "owner1"), "Fails on missing entity");
    world.createEntity("player1");
    assertTrue(!sys.initialize("player1", ""), "Fails with empty owner_id");
}

static void testSafetyTrigger() {
    std::cout << "\n=== Safety: Trigger ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "owner1");

    assertTrue(sys.triggerSafety("player1", "str1", "Astrahus Alpha",
               "asset1", "Rifter", 1), "Trigger first asset");
    assertTrue(sys.getEntryCount("player1") == 1, "One entry");
    assertTrue(sys.getTotalTriggered("player1") == 1, "Total triggered = 1");
    assertTrue(sys.hasEntry("player1", "asset1"), "Entry present");
    assertTrue(sys.getUnclaimedCount("player1") == 1, "One unclaimed");

    assertTrue(sys.triggerSafety("player1", "str1", "Astrahus Alpha",
               "asset2", "Cargo Hold", 100), "Trigger second asset");
    assertTrue(sys.getEntryCount("player1") == 2, "Two entries");
}

static void testSafetyTriggerValidation() {
    std::cout << "\n=== Safety: TriggerValidation ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "owner1");

    assertTrue(!sys.triggerSafety("player1", "", "Astrahus", "a1", "Ship", 1),
               "Empty structure_id rejected");
    assertTrue(!sys.triggerSafety("player1", "str1", "", "a1", "Ship", 1),
               "Empty structure_name rejected");
    assertTrue(!sys.triggerSafety("player1", "str1", "Astrahus", "", "Ship", 1),
               "Empty asset_id rejected");
    assertTrue(!sys.triggerSafety("player1", "str1", "Astrahus", "a1", "", 1),
               "Empty asset_name rejected");
    assertTrue(!sys.triggerSafety("player1", "str1", "Astrahus", "a1", "Ship", 0),
               "Zero quantity rejected");
    assertTrue(!sys.triggerSafety("player1", "str1", "Astrahus", "a1", "Ship", -1),
               "Negative quantity rejected");

    // Duplicate asset_id
    sys.triggerSafety("player1", "str1", "Astrahus", "a1", "Rifter", 1);
    assertTrue(!sys.triggerSafety("player1", "str1", "Astrahus", "a1", "Rifter", 1),
               "Duplicate asset_id rejected");
}

static void testSafetyClaim() {
    std::cout << "\n=== Safety: Claim ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "owner1");
    sys.triggerSafety("player1", "str1", "Astrahus", "asset1", "Rifter", 1);
    sys.triggerSafety("player1", "str1", "Astrahus", "asset2", "Cargo", 50);

    assertTrue(sys.claimAsset("player1", "asset1"), "Claim first asset");
    assertTrue(sys.getTotalClaimed("player1") == 1, "Total claimed = 1");
    assertTrue(sys.getUnclaimedCount("player1") == 1, "One unclaimed remaining");

    assertTrue(!sys.claimAsset("player1", "asset1"), "Cannot claim twice");
    assertTrue(!sys.claimAsset("player1", "nonexistent"), "Claim unknown fails");
}

static void testSafetyClaimAll() {
    std::cout << "\n=== Safety: ClaimAll ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "owner1");
    sys.triggerSafety("player1", "str1", "Astrahus", "a1", "Ship1", 1);
    sys.triggerSafety("player1", "str1", "Astrahus", "a2", "Ship2", 1);
    sys.triggerSafety("player1", "str1", "Astrahus", "a3", "Ship3", 1);

    assertTrue(sys.claimAll("player1"), "Claim all succeeds");
    assertTrue(sys.getUnclaimedCount("player1") == 0, "Zero unclaimed after claim all");
    assertTrue(sys.getTotalClaimed("player1") == 3, "Three claimed");
    assertTrue(!sys.claimAll("player1"), "Claim all fails when nothing unclaimed");
}

static void testSafetyExpiry() {
    std::cout << "\n=== Safety: Expiry ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "owner1");
    sys.triggerSafety("player1", "str1", "Astrahus", "a1", "Ship", 1);

    // Set a short safety duration
    auto* comp = world.getEntity("player1")->getComponent<components::AssetSafetyState>();
    comp->entries[0].expires_in = 10.0f;

    assertTrue(sys.getUnclaimedCount("player1") == 1, "Unclaimed before expiry");

    sys.update(5.0f);
    assertTrue(sys.getUnclaimedCount("player1") == 1, "Still unclaimed at 5s");
    assertTrue(sys.getExpiredCount("player1") == 0, "Not expired at 5s");

    sys.update(6.0f);
    assertTrue(sys.getExpiredCount("player1") == 1, "Expired after 11s total");
    assertTrue(sys.getUnclaimedCount("player1") == 0, "No longer unclaimed after expiry");
}

static void testSafetyExpiredCantClaim() {
    std::cout << "\n=== Safety: ExpiredCantClaim ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "owner1");
    sys.triggerSafety("player1", "str1", "Astrahus", "a1", "Ship", 1);

    auto* comp = world.getEntity("player1")->getComponent<components::AssetSafetyState>();
    comp->entries[0].expires_in = 1.0f;
    sys.update(2.0f);

    assertTrue(comp->entries[0].expired, "Entry expired");
    assertTrue(!sys.claimAsset("player1", "a1"), "Cannot claim expired asset");
}

static void testSafetyCapacity() {
    std::cout << "\n=== Safety: Capacity ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "owner1");
    auto* comp = world.getEntity("player1")->getComponent<components::AssetSafetyState>();
    comp->max_entries = 2;

    assertTrue(sys.triggerSafety("player1", "s1", "S1", "a1", "Item1", 1), "First ok");
    assertTrue(sys.triggerSafety("player1", "s1", "S1", "a2", "Item2", 1), "Second ok");
    assertTrue(!sys.triggerSafety("player1", "s1", "S1", "a3", "Item3", 1), "Third rejected");
    assertTrue(sys.getEntryCount("player1") == 2, "Still at 2");
}

static void testSafetyTick() {
    std::cout << "\n=== Safety: Tick ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", "owner1");
    sys.triggerSafety("player1", "s1", "S1", "a1", "Item", 1);

    auto* comp = world.getEntity("player1")->getComponent<components::AssetSafetyState>();
    float initial_expires = comp->entries[0].expires_in;
    sys.update(100.0f);
    assertTrue(approxEqual(comp->entries[0].expires_in, initial_expires - 100.0f),
               "Expires_in decremented");
    assertTrue(approxEqual(comp->elapsed, 100.0f), "Elapsed updated");
}

static void testSafetyMissing() {
    std::cout << "\n=== Safety: Missing ===" << std::endl;
    ecs::World world;
    systems::AssetSafetySystem sys(&world);

    assertTrue(!sys.triggerSafety("nonexistent", "s1", "S1", "a1", "Item", 1),
               "Trigger fails on missing");
    assertTrue(!sys.claimAsset("nonexistent", "a1"), "Claim fails on missing");
    assertTrue(!sys.claimAll("nonexistent"), "ClaimAll fails on missing");
    assertTrue(sys.getEntryCount("nonexistent") == 0, "Zero entries on missing");
    assertTrue(sys.getUnclaimedCount("nonexistent") == 0, "Zero unclaimed on missing");
    assertTrue(sys.getExpiredCount("nonexistent") == 0, "Zero expired on missing");
    assertTrue(sys.getTotalTriggered("nonexistent") == 0, "Zero triggered on missing");
    assertTrue(sys.getTotalClaimed("nonexistent") == 0, "Zero claimed on missing");
    assertTrue(!sys.hasEntry("nonexistent", "a1"), "HasEntry false on missing");
    assertTrue(sys.getOwner("nonexistent").empty(), "Empty owner on missing");
}

void run_asset_safety_system_tests() {
    testSafetyInit();
    testSafetyInitFails();
    testSafetyTrigger();
    testSafetyTriggerValidation();
    testSafetyClaim();
    testSafetyClaimAll();
    testSafetyExpiry();
    testSafetyExpiredCantClaim();
    testSafetyCapacity();
    testSafetyTick();
    testSafetyMissing();
}
