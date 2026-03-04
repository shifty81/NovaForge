// Tests for: Galaxy Generator Tests
#include "test_log.h"
#include "pcg/pcg_context.h"
#include "pcg/galaxy_generator.h"

using namespace atlas;

// ==================== Galaxy Generator Tests ====================

static void testGalaxyGeneration() {
    std::cout << "\n=== PCG: GalaxyGenerator basic ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 77777, 1 };
    auto galaxy = GalaxyGenerator::generate(ctx, 30);
    assertTrue(galaxy.valid, "Galaxy is valid");
    assertTrue(galaxy.total_systems == 30, "Correct system count");
    assertTrue(!galaxy.nodes.empty(), "Has nodes");
    assertTrue(!galaxy.routes.empty(), "Has routes");
    assertTrue(galaxy.highsec_count > 0, "Has high-sec systems");
}

static void testGalaxyDeterminism() {
    std::cout << "\n=== PCG: GalaxyGenerator determinism ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 88888, 1 };
    auto g1 = GalaxyGenerator::generate(ctx, 20);
    auto g2 = GalaxyGenerator::generate(ctx, 20);
    assertTrue(g1.total_systems == g2.total_systems, "Same system count");
    assertTrue(g1.highsec_count == g2.highsec_count, "Same highsec count");
    assertTrue(g1.routes.size() == g2.routes.size(), "Same route count");
}

static void testGalaxySecurityZones() {
    std::cout << "\n=== PCG: GalaxyGenerator security zones ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 99999, 1 };
    auto galaxy = GalaxyGenerator::generate(ctx, 50);
    assertTrue(galaxy.highsec_count + galaxy.lowsec_count + galaxy.nullsec_count
               == galaxy.total_systems, "Security counts sum to total");
    assertTrue(galaxy.highsec_count > 0, "Has high-sec");
    assertTrue(galaxy.lowsec_count > 0, "Has low-sec");
    assertTrue(galaxy.nullsec_count > 0, "Has null-sec");
}

static void testGalaxyConnectivity() {
    std::cout << "\n=== PCG: GalaxyGenerator all nodes connected ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 11111, 1 };
    auto galaxy = GalaxyGenerator::generate(ctx, 25);
    // Every node should have at least one neighbor.
    bool allConnected = true;
    for (const auto& node : galaxy.nodes) {
        if (node.neighbor_ids.empty()) { allConnected = false; break; }
    }
    assertTrue(allConnected, "All nodes have at least one connection");
}

static void testGalaxyZoneNames() {
    std::cout << "\n=== PCG: GalaxyGenerator zone names ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(GalaxyGenerator::securityZoneName(SecurityZone::HighSec) == "High-Sec", "HighSec name");
    assertTrue(GalaxyGenerator::securityZoneName(SecurityZone::LowSec) == "Low-Sec", "LowSec name");
    assertTrue(GalaxyGenerator::securityZoneName(SecurityZone::NullSec) == "Null-Sec", "NullSec name");
}


void run_galaxy_generator_tests() {
    testGalaxyGeneration();
    testGalaxyDeterminism();
    testGalaxySecurityZones();
    testGalaxyConnectivity();
    testGalaxyZoneNames();
}
