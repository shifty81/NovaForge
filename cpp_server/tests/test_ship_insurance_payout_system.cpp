// Tests for: Ship Insurance Payout System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/ship_insurance_payout_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Ship Insurance Payout System Tests ====================

static void testShipInsurancePayoutCreate() {
    std::cout << "\n=== ShipInsurancePayout: Create ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    world.createEntity("ins1");
    assertTrue(sys.initialize("ins1"), "Init succeeds");
    assertTrue(sys.getPolicyCount("ins1") == 0, "0 policies initially");
    assertTrue(sys.getTotalClaims("ins1") == 0, "0 claims initially");
    assertTrue(approxEqual(sys.getTotalPremiums("ins1"), 0.0f), "0 premiums initially");
    assertTrue(approxEqual(sys.getTotalPayouts("ins1"), 0.0f), "0 payouts initially");
}

static void testShipInsurancePayoutInitValidation() {
    std::cout << "\n=== ShipInsurancePayout: InitValidation ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Missing entity rejected");
}

static void testShipInsurancePayoutAddPolicy() {
    std::cout << "\n=== ShipInsurancePayout: AddPolicy ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    world.createEntity("ins1");
    sys.initialize("ins1");
    assertTrue(sys.addPolicy("ins1", "pol1", "ship_a", "basic", 1000000.0f), "Add basic policy");
    assertTrue(sys.getPolicyCount("ins1") == 1, "1 policy");
    assertTrue(sys.getActiveCount("ins1") == 1, "1 active");
    // Premium = 10% of 1M = 100K
    assertTrue(approxEqual(sys.getTotalPremiums("ins1"), 100000.0f), "Premium = 100K");
}

static void testShipInsurancePayoutPolicyValidation() {
    std::cout << "\n=== ShipInsurancePayout: PolicyValidation ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    world.createEntity("ins1");
    sys.initialize("ins1");
    assertTrue(!sys.addPolicy("ins1", "", "ship_a", "basic", 1000.0f), "Empty policy_id rejected");
    assertTrue(!sys.addPolicy("ins1", "p1", "", "basic", 1000.0f), "Empty ship_id rejected");
    assertTrue(!sys.addPolicy("ins1", "p1", "ship_a", "", 1000.0f), "Empty tier rejected");
    assertTrue(!sys.addPolicy("ins1", "p1", "ship_a", "basic", 0.0f), "Zero value rejected");
    assertTrue(!sys.addPolicy("ins1", "p1", "ship_a", "basic", -100.0f), "Negative value rejected");
}

static void testShipInsurancePayoutDuplicate() {
    std::cout << "\n=== ShipInsurancePayout: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    world.createEntity("ins1");
    sys.initialize("ins1");
    assertTrue(sys.addPolicy("ins1", "pol1", "ship_a", "basic", 1000.0f), "First policy");
    assertTrue(!sys.addPolicy("ins1", "pol1", "ship_b", "standard", 2000.0f), "Duplicate rejected");
}

static void testShipInsurancePayoutRemovePolicy() {
    std::cout << "\n=== ShipInsurancePayout: RemovePolicy ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    world.createEntity("ins1");
    sys.initialize("ins1");
    sys.addPolicy("ins1", "pol1", "ship_a", "basic", 1000.0f);
    sys.addPolicy("ins1", "pol2", "ship_b", "standard", 2000.0f);
    assertTrue(sys.removePolicy("ins1", "pol1"), "Remove pol1");
    assertTrue(sys.getPolicyCount("ins1") == 1, "1 policy left");
    assertTrue(!sys.removePolicy("ins1", "nonexistent"), "Cannot remove nonexistent");
}

static void testShipInsurancePayoutFileClaim() {
    std::cout << "\n=== ShipInsurancePayout: FileClaim ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    world.createEntity("ins1");
    sys.initialize("ins1");
    sys.addPolicy("ins1", "pol1", "ship_a", "basic", 1000000.0f);
    // Claim payout: 50% of 1M = 500K
    assertTrue(approxEqual(sys.getClaimPayout("ins1", "pol1"), 500000.0f), "Payout = 500K");
    assertTrue(sys.fileClaim("ins1", "pol1"), "File claim");
    assertTrue(sys.getTotalClaims("ins1") == 1, "1 claim");
    assertTrue(approxEqual(sys.getTotalPayouts("ins1"), 500000.0f), "Total payouts = 500K");
    assertTrue(!sys.fileClaim("ins1", "pol1"), "Cannot claim twice");
}

static void testShipInsurancePayoutTiers() {
    std::cout << "\n=== ShipInsurancePayout: Tiers ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    world.createEntity("ins1");
    sys.initialize("ins1");
    sys.addPolicy("ins1", "p1", "s1", "basic", 100000.0f);
    sys.addPolicy("ins1", "p2", "s2", "standard", 100000.0f);
    sys.addPolicy("ins1", "p3", "s3", "platinum", 100000.0f);
    // Basic: 50% = 50K, Standard: 75% = 75K, Platinum: 100% = 100K
    assertTrue(approxEqual(sys.getClaimPayout("ins1", "p1"), 50000.0f), "Basic payout 50K");
    assertTrue(approxEqual(sys.getClaimPayout("ins1", "p2"), 75000.0f), "Standard payout 75K");
    assertTrue(approxEqual(sys.getClaimPayout("ins1", "p3"), 100000.0f), "Platinum payout 100K");
    // Premiums: basic 10% = 10K, standard 20% = 20K, platinum 35% = 35K = 65K total
    assertTrue(approxEqual(sys.getTotalPremiums("ins1"), 65000.0f), "Total premiums = 65K");
}

static void testShipInsurancePayoutProfitLoss() {
    std::cout << "\n=== ShipInsurancePayout: ProfitLoss ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    world.createEntity("ins1");
    sys.initialize("ins1");
    sys.addPolicy("ins1", "p1", "s1", "basic", 100000.0f);
    // Premium = 10K, no claims yet -> P/L = 10K
    assertTrue(sys.getProfitLoss("ins1") > 0.0f, "Profit when no claims");
    sys.fileClaim("ins1", "p1");
    // Payout = 50K -> P/L = 10K - 50K = -40K
    assertTrue(sys.getProfitLoss("ins1") < 0.0f, "Loss after claim");
}

static void testShipInsurancePayoutMaxPolicies() {
    std::cout << "\n=== ShipInsurancePayout: MaxPolicies ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    world.createEntity("ins1");
    sys.initialize("ins1");
    for (int i = 0; i < 20; ++i) {
        assertTrue(sys.addPolicy("ins1", "pol_" + std::to_string(i),
                   "ship_" + std::to_string(i), "basic", 1000.0f),
                   ("Add policy " + std::to_string(i)).c_str());
    }
    assertTrue(!sys.addPolicy("ins1", "pol_overflow", "ship_x", "basic", 1000.0f),
               "21st policy rejected");
}

static void testShipInsurancePayoutMissing() {
    std::cout << "\n=== ShipInsurancePayout: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipInsurancePayoutSystem sys(&world);
    assertTrue(!sys.addPolicy("nonexistent", "p1", "s1", "basic", 1000.0f), "Add fails on missing");
    assertTrue(!sys.removePolicy("nonexistent", "p1"), "Remove fails on missing");
    assertTrue(!sys.fileClaim("nonexistent", "p1"), "Claim fails on missing");
    assertTrue(approxEqual(sys.getClaimPayout("nonexistent", "p1"), 0.0f), "0 payout on missing");
    assertTrue(sys.getPolicyCount("nonexistent") == 0, "0 policies on missing");
    assertTrue(sys.getActiveCount("nonexistent") == 0, "0 active on missing");
    assertTrue(approxEqual(sys.getTotalPremiums("nonexistent"), 0.0f), "0 premiums on missing");
    assertTrue(approxEqual(sys.getTotalPayouts("nonexistent"), 0.0f), "0 payouts on missing");
    assertTrue(approxEqual(sys.getProfitLoss("nonexistent"), 0.0f), "0 P/L on missing");
    assertTrue(sys.getTotalClaims("nonexistent") == 0, "0 claims on missing");
}

void run_ship_insurance_payout_system_tests() {
    testShipInsurancePayoutCreate();
    testShipInsurancePayoutInitValidation();
    testShipInsurancePayoutAddPolicy();
    testShipInsurancePayoutPolicyValidation();
    testShipInsurancePayoutDuplicate();
    testShipInsurancePayoutRemovePolicy();
    testShipInsurancePayoutFileClaim();
    testShipInsurancePayoutTiers();
    testShipInsurancePayoutProfitLoss();
    testShipInsurancePayoutMaxPolicies();
    testShipInsurancePayoutMissing();
}
