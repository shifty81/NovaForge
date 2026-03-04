// Tests for: Star System Generator Tests
#include "test_log.h"
#include "ecs/system.h"
#include "pcg/pcg_context.h"
#include "pcg/star_system_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Star System Generator Tests ====================

static void testStarSystemGeneration() {
    std::cout << "\n=== PCG: StarSystemGenerator basic ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 12345, 1 };
    auto sys = StarSystemGenerator::generate(ctx, 0.8f);
    assertTrue(sys.valid, "Generated system is valid");
    assertTrue(sys.totalPlanets >= 3, "At least 3 planets");
    assertTrue(sys.securityLevel > 0.0f, "Security level positive");
    assertTrue(!sys.orbitSlots.empty(), "Has orbit slots");
}

static void testStarSystemDeterminism() {
    std::cout << "\n=== PCG: StarSystemGenerator determinism ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 54321, 1 };
    auto sys1 = StarSystemGenerator::generate(ctx, 0.5f);
    auto sys2 = StarSystemGenerator::generate(ctx, 0.5f);
    assertTrue(sys1.totalPlanets == sys2.totalPlanets, "Same planet count");
    assertTrue(sys1.totalBelts == sys2.totalBelts, "Same belt count");
    assertTrue(sys1.star.starClass == sys2.star.starClass, "Same star class");
}

static void testStarSystemSecurityAffectsStations() {
    std::cout << "\n=== PCG: StarSystem security affects stations ===" << std::endl;
    using namespace atlas::pcg;

    int highSecStations = 0, nullSecStations = 0;
    for (int i = 0; i < 20; ++i) {
        PCGContext ctx{ static_cast<uint64_t>(i * 137), 1 };
        auto hs = StarSystemGenerator::generate(ctx, 0.9f);
        highSecStations += static_cast<int>(hs.stations.size());
        auto ns = StarSystemGenerator::generate(ctx, 0.05f);
        nullSecStations += static_cast<int>(ns.stations.size());
    }
    assertTrue(highSecStations > nullSecStations,
               "High-sec has more stations than null-sec");
}

static void testStarSystemStarClassName() {
    std::cout << "\n=== PCG: StarSystem star class names ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(StarSystemGenerator::starClassName(StarClass::G) == "G", "G star name");
    assertTrue(StarSystemGenerator::starClassName(StarClass::M) == "M", "M star name");
    assertTrue(StarSystemGenerator::starClassName(StarClass::O) == "O", "O star name");
}


void run_star_system_generator_tests() {
    testStarSystemGeneration();
    testStarSystemDeterminism();
    testStarSystemSecurityAffectsStations();
    testStarSystemStarClassName();
}
