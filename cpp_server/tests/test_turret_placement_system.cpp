// Tests for: Turret Placement System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "pcg/pcg_context.h"
#include "pcg/turret_placement_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Turret Placement System Tests ====================

static void testTurretPlacementBasic() {
    std::cout << "\n=== Turret Placement Basic ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto placement = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Cruiser, 4);
    assertTrue(static_cast<int>(placement.mounts.size()) == 4, "4 mounts placed");
    assertTrue(placement.coverage_score > 0.0f, "Has some coverage");
    assertTrue(placement.coverage_score <= 1.0f, "Coverage ≤ 1.0");
}

static void testTurretPlacementDeterminism() {
    std::cout << "\n=== Turret Placement Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto p1 = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Battleship, 6);
    auto p2 = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Battleship, 6);
    assertTrue(p1.mounts.size() == p2.mounts.size(), "Same mount count");
    assertTrue(approxEqual(p1.coverage_score, p2.coverage_score), "Same coverage");
    assertTrue(approxEqual(p1.max_overlap, p2.max_overlap), "Same overlap");
}

static void testTurretPlacementOverlapThreshold() {
    std::cout << "\n=== Turret Placement Overlap Threshold ===" << std::endl;
    // Moderate turret counts should pass the 30% overlap threshold.
    bool allValid = true;
    for (uint64_t i = 1; i <= 50; ++i) {
        pcg::PCGContext ctx{ i * 13, 1 };
        auto p = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Cruiser, 3);
        if (!p.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "3-turret cruisers all pass overlap threshold");
}

static void testTurretPlacementFaction() {
    std::cout << "\n=== Turret Placement Faction ===" << std::endl;
    pcg::PCGContext ctx{ 999, 1 };
    auto solari = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Cruiser, 4, "Solari");
    auto veyren = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Cruiser, 4, "Veyren");
    assertTrue(static_cast<int>(solari.mounts.size()) == 4, "Solari has 4 mounts");
    assertTrue(static_cast<int>(veyren.mounts.size()) == 4, "Veyren has 4 mounts");
    // Veyren should have wider arcs (utilitarian spread).
    float solariArcSum = 0.0f, veyrenArcSum = 0.0f;
    for (const auto& m : solari.mounts) solariArcSum += m.arc_deg;
    for (const auto& m : veyren.mounts) veyrenArcSum += m.arc_deg;
    assertTrue(veyrenArcSum > solariArcSum, "Veyren has wider total arcs than Solari");
}

static void testTurretPlacementZeroTurrets() {
    std::cout << "\n=== Turret Placement Zero Turrets ===" << std::endl;
    pcg::PCGContext ctx{ 0, 1 };
    auto p = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Frigate, 0);
    assertTrue(p.mounts.empty(), "Zero turrets produces no mounts");
    assertTrue(approxEqual(p.coverage_score, 0.0f), "No coverage with zero turrets");
}

static void testTurretPlacementCoverageComputation() {
    std::cout << "\n=== Turret Placement Coverage Computation ===" << std::endl;
    // A single turret with 360° arc should give 100% coverage.
    std::vector<pcg::TurretMount> mounts;
    pcg::TurretMount m{};
    m.direction_deg = 0.0f;
    m.arc_deg = 360.0f;
    mounts.push_back(m);
    float cov = pcg::TurretPlacementSystem::computeCoverage(mounts);
    assertTrue(approxEqual(cov, 1.0f), "360° arc gives 100% coverage");
}


void run_turret_placement_system_tests() {
    testTurretPlacementBasic();
    testTurretPlacementDeterminism();
    testTurretPlacementOverlapThreshold();
    testTurretPlacementFaction();
    testTurretPlacementZeroTurrets();
    testTurretPlacementCoverageComputation();
}
