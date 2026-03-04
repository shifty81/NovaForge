// Tests for: NPC Encounter Generator Tests
#include "test_log.h"
#include "pcg/npc_encounter_generator.h"
#include "pcg/pcg_context.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== NPC Encounter Generator Tests ====================

static void testNPCEncounterGeneration() {
    std::cout << "\n=== PCG: NPCEncounterGenerator basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7000, 1 };
    auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.6f);
    assertTrue(enc.valid, "Encounter is valid");
    assertTrue(!enc.waves.empty(), "Encounter has waves");
    assertTrue(enc.totalShips > 0, "Encounter has ships");
    assertTrue(enc.estimatedBounty > 0.0f, "Encounter has bounty");
}

static void testNPCEncounterDeterminism() {
    std::cout << "\n=== PCG: NPCEncounterGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7100, 1 };
    auto e1 = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.5f);
    auto e2 = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.5f);
    assertTrue(e1.faction == e2.faction, "Same faction");
    assertTrue(e1.totalShips == e2.totalShips, "Same ship count");
    assertTrue(e1.waves.size() == e2.waves.size(), "Same wave count");
    bool allMatch = true;
    for (size_t w = 0; w < e1.waves.size(); ++w) {
        if (e1.waves[w].ships.size() != e2.waves[w].ships.size()) {
            allMatch = false; break;
        }
    }
    assertTrue(allMatch, "Same seed → identical encounter");
}

static void testNPCEncounterExplicitWaves() {
    std::cout << "\n=== PCG: NPCEncounterGenerator explicit waves ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7200, 1 };
    auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 3, 0.5f);
    assertTrue(static_cast<int>(enc.waves.size()) == 3, "Explicit 3 waves");
}

static void testNPCEncounterBountyCalculation() {
    std::cout << "\n=== PCG: NPCEncounterGenerator bounty calculation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7300, 1 };
    auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.5f);
    float calculated = atlas::pcg::NPCEncounterGenerator::calculateBounty(enc);
    assertTrue(calculated > 0.0f, "Calculated bounty is positive");
}

static void testNPCEncounterDifficultyScaling() {
    std::cout << "\n=== PCG: NPCEncounterGenerator difficulty scaling ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7400, 1 };
    auto highSec = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.9f);
    auto nullSec = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.1f);
    assertTrue(nullSec.difficultyRating > highSec.difficultyRating,
               "Null-sec harder than high-sec");
}

static void testNPCEncounterAllValid() {
    std::cout << "\n=== PCG: NPCEncounterGenerator all valid ===" << std::endl;
    bool allValid = true;
    for (uint64_t i = 1; i <= 50; ++i) {
        atlas::pcg::PCGContext ctx{ i * 83, 1 };
        auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.5f);
        if (!enc.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All 50 encounters are valid");
}

static void testNPCEncounterWaveEscalation() {
    std::cout << "\n=== PCG: NPCEncounterGenerator wave escalation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7500, 1 };
    auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 5, 0.3f);
    // First wave delay should be 0.
    assertTrue(enc.waves[0].triggerDelay == 0.0f, "First wave has no delay");
    // Subsequent waves should have positive delay.
    if (enc.waves.size() > 1) {
        assertTrue(enc.waves[1].triggerDelay > 0.0f, "Second wave has delay");
    }
}


void run_npc_encounter_generator_tests() {
    testNPCEncounterGeneration();
    testNPCEncounterDeterminism();
    testNPCEncounterExplicitWaves();
    testNPCEncounterBountyCalculation();
    testNPCEncounterDifficultyScaling();
    testNPCEncounterAllValid();
    testNPCEncounterWaveEscalation();
}
