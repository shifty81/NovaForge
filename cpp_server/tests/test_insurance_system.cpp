// Tests for: Insurance System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/insurance_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Insurance System Tests ====================

static void testInsurancePurchase() {
    std::cout << "\n=== Insurance Purchase ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 1000000.0;

    assertTrue(insSys.purchaseInsurance("player_ship", "basic", 500000.0),
               "Basic insurance purchased");
    auto* policy = ship->getComponent<components::InsurancePolicy>();
    assertTrue(policy != nullptr, "InsurancePolicy component created");
    assertTrue(policy->tier == "basic", "Policy tier is basic");
    assertTrue(approxEqual(static_cast<float>(policy->coverage_fraction), 0.5f), "Basic coverage is 50%");
    assertTrue(approxEqual(static_cast<float>(policy->payout_value), 250000.0f), "Payout is 50% of ship value");
    assertTrue(player->credits < 1000000.0, "Premium deducted from Credits");
    assertTrue(policy->active, "Policy is active");
}

static void testInsuranceClaim() {
    std::cout << "\n=== Insurance Claim ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 1000000.0;

    insSys.purchaseInsurance("player_ship", "standard", 500000.0);
    double isc_after_purchase = player->credits;

    double payout = insSys.claimInsurance("player_ship");
    assertTrue(payout > 0.0, "Claim returns positive payout");
    assertTrue(approxEqual(static_cast<float>(payout), 350000.0f), "Standard pays 70% of ship value");
    assertTrue(approxEqual(static_cast<float>(player->credits), static_cast<float>(isc_after_purchase + payout)),
               "Credits increased by payout");

    auto* policy = ship->getComponent<components::InsurancePolicy>();
    assertTrue(policy->claimed, "Policy marked as claimed");

    double second_claim = insSys.claimInsurance("player_ship");
    assertTrue(approxEqual(static_cast<float>(second_claim), 0.0f), "Double claim returns 0");
}

static void testInsurancePlatinum() {
    std::cout << "\n=== Insurance Platinum ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 1000000.0;

    assertTrue(insSys.purchaseInsurance("player_ship", "platinum", 500000.0),
               "Platinum insurance purchased");
    auto* policy = ship->getComponent<components::InsurancePolicy>();
    assertTrue(approxEqual(static_cast<float>(policy->coverage_fraction), 1.0f), "Platinum coverage is 100%");
    assertTrue(approxEqual(static_cast<float>(policy->payout_value), 500000.0f), "Platinum payout is full value");
}

static void testInsuranceExpiry() {
    std::cout << "\n=== Insurance Expiry ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 1000000.0;

    insSys.purchaseInsurance("player_ship", "basic", 500000.0);
    auto* policy = ship->getComponent<components::InsurancePolicy>();
    policy->duration_remaining = 10.0f; // 10 seconds

    insSys.update(5.0f);
    assertTrue(policy->active, "Policy still active at 5s");
    assertTrue(insSys.hasActivePolicy("player_ship"), "hasActivePolicy returns true");

    insSys.update(6.0f);
    assertTrue(!policy->active, "Policy expired after 11s");
    assertTrue(!insSys.hasActivePolicy("player_ship"), "hasActivePolicy returns false after expiry");
}

static void testInsuranceInsufficientFunds() {
    std::cout << "\n=== Insurance Insufficient Funds ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 100.0; // Not enough

    assertTrue(!insSys.purchaseInsurance("player_ship", "basic", 500000.0),
               "Insurance rejected with insufficient funds");
    assertTrue(ship->getComponent<components::InsurancePolicy>() == nullptr,
               "No policy created on failure");
}


void run_insurance_system_tests() {
    testInsurancePurchase();
    testInsuranceClaim();
    testInsurancePlatinum();
    testInsuranceExpiry();
    testInsuranceInsufficientFunds();
}
