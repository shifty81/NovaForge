// Tests for: Spine Hull Generator Tests
#include "test_log.h"
#include "components/core_components.h"
#include "pcg/pcg_context.h"
#include "systems/movement_system.h"
#include "pcg/spine_hull_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Spine Hull Generator Tests ====================

static void testSpineHullGeneration() {
    std::cout << "\n=== Spine Hull Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto hull = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Cruiser);
    assertTrue(hull.valid, "Generated spine hull is valid");
    assertTrue(hull.profile.length > 0.0f, "Hull has positive length");
    assertTrue(hull.profile.width_mid > 0.0f, "Hull has positive mid-width");
    assertTrue(hull.engine_cluster_count > 0, "Hull has engines");
    assertTrue(static_cast<int>(hull.zones.size()) == 3, "Hull has 3 functional zones");
    assertTrue(hull.bilateral_symmetry, "Ships have bilateral symmetry");
}

static void testSpineHullDeterminism() {
    std::cout << "\n=== Spine Hull Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto h1 = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Battleship);
    auto h2 = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Battleship);
    assertTrue(h1.spine == h2.spine, "Same seed same spine type");
    assertTrue(approxEqual(h1.profile.length, h2.profile.length), "Same seed same length");
    assertTrue(approxEqual(h1.profile.width_mid, h2.profile.width_mid), "Same seed same width");
    assertTrue(h1.engine_cluster_count == h2.engine_cluster_count, "Same seed same engines");
    assertTrue(h1.total_greeble_count == h2.total_greeble_count, "Same seed same greebles");
}

static void testSpineHullZoneOrdering() {
    std::cout << "\n=== Spine Hull Zone Ordering ===" << std::endl;
    pcg::PCGContext ctx{ 555, 1 };
    auto hull = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Frigate);
    assertTrue(hull.zones[0].zone == pcg::FunctionalZone::Command, "Zone 0 is Command");
    assertTrue(hull.zones[1].zone == pcg::FunctionalZone::MidHull, "Zone 1 is MidHull");
    assertTrue(hull.zones[2].zone == pcg::FunctionalZone::Engineering, "Zone 2 is Engineering");

    // Zone fractions must sum to 1.0.
    float total = hull.zones[0].length_fraction + hull.zones[1].length_fraction
                + hull.zones[2].length_fraction;
    assertTrue(std::fabs(total - 1.0f) < 0.02f, "Zone fractions sum to 1.0");
}

static void testSpineHullAspectRatio() {
    std::cout << "\n=== Spine Hull Aspect Ratio ===" << std::endl;
    // Generate many hulls and verify aspect ratio clamping.
    bool allClamped = true;
    for (uint64_t i = 1; i <= 100; ++i) {
        pcg::PCGContext ctx{ i * 31, 1 };
        auto hull = pcg::SpineHullGenerator::generate(ctx);
        if (hull.aspect_ratio < 1.5f || hull.aspect_ratio > 15.0f) {
            allClamped = false;
            break;
        }
    }
    assertTrue(allClamped, "All hulls have aspect ratio in [1.5, 15]");
}

static void testSpineHullFactionStyle() {
    std::cout << "\n=== Spine Hull Faction Style ===" << std::endl;
    pcg::PCGContext ctx{ 999, 1 };
    auto solari = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Cruiser, "Solari");
    auto veyren = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    assertTrue(solari.faction_style == "Solari", "Solari faction tag set");
    assertTrue(veyren.faction_style == "Veyren", "Veyren faction tag set");
    // Solari hulls should be narrower (elegant) vs Veyren (wider, angular).
    assertTrue(solari.profile.width_mid < veyren.profile.width_mid,
               "Solari narrower than Veyren");
}

static void testSpineHullAllClassesValid() {
    std::cout << "\n=== Spine Hull All Classes Valid ===" << std::endl;
    pcg::HullClass classes[] = {
        pcg::HullClass::Frigate, pcg::HullClass::Destroyer,
        pcg::HullClass::Cruiser, pcg::HullClass::Battlecruiser,
        pcg::HullClass::Battleship, pcg::HullClass::Capital
    };
    bool allValid = true;
    for (auto hc : classes) {
        pcg::PCGContext ctx{ static_cast<uint64_t>(hc) * 100 + 7, 1 };
        auto hull = pcg::SpineHullGenerator::generate(ctx, hc);
        if (!hull.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All hull classes produce valid spine hulls");
}

static void testSpineHullCapitalLarger() {
    std::cout << "\n=== Spine Hull Capital Larger ===" << std::endl;
    pcg::PCGContext ctx{ 123, 1 };
    auto frigate = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Frigate);
    auto capital = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Capital);
    assertTrue(capital.profile.length > frigate.profile.length,
               "Capital hull longer than frigate");
    assertTrue(capital.engine_cluster_count > frigate.engine_cluster_count,
               "Capital has more engines than frigate");
}

static void testSpineTypeName() {
    std::cout << "\n=== Spine Type Names ===" << std::endl;
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Needle) == "Needle", "Needle name");
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Wedge) == "Wedge", "Wedge name");
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Hammer) == "Hammer", "Hammer name");
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Slab) == "Slab", "Slab name");
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Ring) == "Ring", "Ring name");
}


void run_spine_hull_generator_tests() {
    testSpineHullGeneration();
    testSpineHullDeterminism();
    testSpineHullZoneOrdering();
    testSpineHullAspectRatio();
    testSpineHullFactionStyle();
    testSpineHullAllClassesValid();
    testSpineHullCapitalLarger();
    testSpineTypeName();
}
