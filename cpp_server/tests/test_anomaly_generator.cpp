// Tests for: Anomaly Generator Tests
#include "test_log.h"
#include "components/mission_components.h"
#include "pcg/pcg_context.h"
#include "systems/movement_system.h"
#include "pcg/anomaly_generator.h"

using namespace atlas;

// ==================== Anomaly Generator Tests ====================

static void testAnomalyGeneration() {
    std::cout << "\n=== PCG: AnomalyGenerator basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6000, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(ctx, 0.7f);
    assertTrue(!site.nodes.empty(), "Anomaly has nodes");
    assertTrue(site.siteRadius > 0.0f, "Anomaly has positive radius");
    assertTrue(site.totalValue > 0.0f, "Anomaly has positive value");
}

static void testAnomalyDeterminism() {
    std::cout << "\n=== PCG: AnomalyGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6100, 1 };
    auto s1 = atlas::pcg::AnomalyGenerator::generate(ctx, 0.5f);
    auto s2 = atlas::pcg::AnomalyGenerator::generate(ctx, 0.5f);
    assertTrue(s1.type == s2.type, "Same site type");
    assertTrue(s1.difficulty == s2.difficulty, "Same difficulty");
    assertTrue(s1.nodes.size() == s2.nodes.size(), "Same node count");
    bool allMatch = true;
    for (size_t i = 0; i < s1.nodes.size(); ++i) {
        if (s1.nodes[i].value != s2.nodes[i].value) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical anomaly");
}

static void testAnomalyTypeOverride() {
    std::cout << "\n=== PCG: AnomalyGenerator type override ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6200, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(
        ctx, atlas::pcg::AnomalySiteType::CombatSite, 0.5f);
    assertTrue(site.type == atlas::pcg::AnomalySiteType::CombatSite,
               "Type override applied");
    assertTrue(site.waveCount > 0, "Combat site has waves");
}

static void testAnomalyNonCombatNoWaves() {
    std::cout << "\n=== PCG: AnomalyGenerator non-combat no waves ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6300, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(
        ctx, atlas::pcg::AnomalySiteType::GasSite, 0.5f);
    assertTrue(site.type == atlas::pcg::AnomalySiteType::GasSite, "Gas site type");
    assertTrue(site.waveCount == 0, "Gas site has no waves");
}

static void testAnomalyLowSecRequiresScan() {
    std::cout << "\n=== PCG: AnomalyGenerator low-sec requires scan ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6400, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(ctx, 0.1f);
    assertTrue(site.requiresScan, "Low-sec anomaly requires scanning");
}

static void testAnomalyValueCalculation() {
    std::cout << "\n=== PCG: AnomalyGenerator value calculation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6500, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(ctx, 0.5f);
    float calculated = atlas::pcg::AnomalyGenerator::calculateTotalValue(site);
    float manual = 0.0f;
    for (const auto& n : site.nodes) manual += n.value;
    assertTrue(std::abs(calculated - manual) < 0.01f, "calculateTotalValue matches sum");
}


void run_anomaly_generator_tests() {
    testAnomalyGeneration();
    testAnomalyDeterminism();
    testAnomalyTypeOverride();
    testAnomalyNonCombatNoWaves();
    testAnomalyLowSecRequiresScan();
    testAnomalyValueCalculation();
}
