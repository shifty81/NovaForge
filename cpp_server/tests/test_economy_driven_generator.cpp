// Tests for: Economy-Driven Generator Tests
#include "test_log.h"
#include "components/core_components.h"
#include "pcg/pcg_context.h"
#include "systems/fleet_system.h"
#include "pcg/economy_driven_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Economy-Driven Generator Tests ====================

static void testEconomyFleetGeneration() {
    std::cout << "\n=== Economy Fleet Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto fleet = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::ResourceRich, 10);
    assertTrue(fleet.valid, "Economy fleet is valid");
    assertTrue(fleet.total_ships == 10, "Fleet has 10 ships");
    assertTrue(static_cast<int>(fleet.ships.size()) == 10, "Ships vector has 10 entries");
    assertTrue(fleet.economy == pcg::EconomyState::ResourceRich, "Economy state matches");
    assertTrue(fleet.average_equipment_quality > 0.0f, "Fleet has positive equipment quality");
}

static void testEconomyFleetDeterminism() {
    std::cout << "\n=== Economy Fleet Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto f1 = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::WarTorn, 5);
    auto f2 = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::WarTorn, 5);
    assertTrue(f1.total_ships == f2.total_ships, "Same seed same ship count");
    assertTrue(approxEqual(f1.average_equipment_quality, f2.average_equipment_quality),
               "Same seed same equipment quality");
    bool rolesMatch = true;
    for (int i = 0; i < f1.total_ships; ++i) {
        if (f1.ships[i].role != f2.ships[i].role) { rolesMatch = false; break; }
    }
    assertTrue(rolesMatch, "Same seed same roles");
}

static void testEconomyResourceRichHasMiners() {
    std::cout << "\n=== Economy ResourceRich Has Miners ===" << std::endl;
    pcg::PCGContext ctx{ 555, 1 };
    auto fleet = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::ResourceRich, 20);
    int minerCount = 0;
    for (const auto& ship : fleet.ships) {
        if (ship.role == pcg::EconomyShipRole::Miner) minerCount++;
    }
    assertTrue(minerCount > 0, "ResourceRich produces miners");
}

static void testEconomyWarTornHasDamage() {
    std::cout << "\n=== Economy WarTorn Has Damage ===" << std::endl;
    pcg::PCGContext ctx{ 888, 1 };
    auto fleet = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::WarTorn, 10);
    float totalWear = 0.0f;
    for (const auto& ship : fleet.ships) {
        totalWear += ship.damage_wear;
    }
    float avgWear = totalWear / static_cast<float>(fleet.total_ships);
    assertTrue(avgWear > 0.1f, "WarTorn ships have significant wear");
}

static void testEconomyProsperousHighQuality() {
    std::cout << "\n=== Economy Prosperous High Quality ===" << std::endl;
    pcg::PCGContext ctx{ 123, 1 };
    auto prosperous = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::Prosperous, 15);
    auto declining  = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::Declining, 15);
    assertTrue(prosperous.average_equipment_quality > declining.average_equipment_quality,
               "Prosperous has better equipment than Declining");
}

static void testEconomyLawlessHasPirates() {
    std::cout << "\n=== Economy Lawless Has Pirates ===" << std::endl;
    pcg::PCGContext ctx{ 444, 1 };
    auto fleet = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::Lawless, 20);
    int pirateCount = 0;
    for (const auto& ship : fleet.ships) {
        if (ship.role == pcg::EconomyShipRole::Pirate) pirateCount++;
    }
    assertTrue(pirateCount > 0, "Lawless produces pirates");
}

static void testEconomyStateNames() {
    std::cout << "\n=== Economy State Names ===" << std::endl;
    assertTrue(pcg::EconomyDrivenGenerator::economyStateName(pcg::EconomyState::Prosperous) == "Prosperous", "Prosperous name");
    assertTrue(pcg::EconomyDrivenGenerator::economyStateName(pcg::EconomyState::ResourceRich) == "ResourceRich", "ResourceRich name");
    assertTrue(pcg::EconomyDrivenGenerator::economyStateName(pcg::EconomyState::WarTorn) == "WarTorn", "WarTorn name");
    assertTrue(pcg::EconomyDrivenGenerator::shipRoleName(pcg::EconomyShipRole::Miner) == "Miner", "Miner role name");
    assertTrue(pcg::EconomyDrivenGenerator::shipRoleName(pcg::EconomyShipRole::Pirate) == "Pirate", "Pirate role name");
}


void run_economy_driven_generator_tests() {
    testEconomyFleetGeneration();
    testEconomyFleetDeterminism();
    testEconomyResourceRichHasMiners();
    testEconomyWarTornHasDamage();
    testEconomyProsperousHighQuality();
    testEconomyLawlessHasPirates();
    testEconomyStateNames();
}
