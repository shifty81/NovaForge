// Tests for: Procedural Texture Generator Tests
#include "test_log.h"
#include "components/core_components.h"
#include "pcg/pcg_context.h"
#include "systems/movement_system.h"
#include "pcg/procedural_texture_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Procedural Texture Generator Tests ====================

static void testTextureGeneration() {
    std::cout << "\n=== Procedural Texture Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto tex = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    assertTrue(tex.valid, "Texture params valid");
    assertTrue(tex.faction == "Veyren", "Faction stored");
    assertTrue(tex.hull_class == pcg::HullClass::Cruiser, "Hull class stored");
    assertTrue(!tex.markings.empty(), "Has hull markings");
    assertTrue(!tex.window_lights.empty(), "Has window lights");
    assertTrue(tex.panel_tile_count > 0, "Has panel tiling");
}

static void testTextureDeterminism() {
    std::cout << "\n=== Procedural Texture Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto t1 = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Battleship, "Solari");
    auto t2 = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Battleship, "Solari");
    assertTrue(approxEqual(t1.palette.primary_r, t2.palette.primary_r), "Same seed same palette R");
    assertTrue(approxEqual(t1.palette.primary_g, t2.palette.primary_g), "Same seed same palette G");
    assertTrue(approxEqual(t1.material.metalness, t2.material.metalness), "Same seed same metalness");
    assertTrue(static_cast<int>(t1.markings.size()) == static_cast<int>(t2.markings.size()),
               "Same seed same marking count");
    assertTrue(static_cast<int>(t1.window_lights.size()) == static_cast<int>(t2.window_lights.size()),
               "Same seed same window count");
}

static void testTextureFactionPalettes() {
    std::cout << "\n=== Texture Faction Palettes ===" << std::endl;
    auto solari  = pcg::ProceduralTextureGenerator::basePalette("Solari");
    auto veyren  = pcg::ProceduralTextureGenerator::basePalette("Veyren");
    auto aurelian = pcg::ProceduralTextureGenerator::basePalette("Aurelian");
    auto keldari = pcg::ProceduralTextureGenerator::basePalette("Keldari");
    // Solari is golden (R > G > B).
    assertTrue(solari.primary_r > solari.primary_b, "Solari primary is warm (R > B)");
    // Veyren is blue-grey (B > R).
    assertTrue(veyren.primary_b > veyren.primary_r, "Veyren primary is cool (B > R)");
    // Aurelian is green-tinted (G > R, G > B).
    assertTrue(aurelian.primary_g > aurelian.primary_r, "Aurelian primary green (G > R)");
    // Keldari is brown (R > G > B).
    assertTrue(keldari.primary_r > keldari.primary_g, "Keldari primary warm (R > G)");
    assertTrue(keldari.primary_g > keldari.primary_b, "Keldari primary brown (G > B)");
}

static void testTextureMaterialByFaction() {
    std::cout << "\n=== Texture Material By Faction ===" << std::endl;
    pcg::PCGContext ctx{ 500, 1 };
    auto solari  = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Cruiser, "Solari");
    auto keldari = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Cruiser, "Keldari");
    // Solari ships are more polished (lower roughness).
    assertTrue(solari.material.roughness < keldari.material.roughness,
               "Solari smoother than Keldari");
}

static void testTextureScalesWithClass() {
    std::cout << "\n=== Texture Scales With Class ===" << std::endl;
    pcg::PCGContext ctx{ 600, 1 };
    auto frigate = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Frigate, "Veyren");
    auto capital = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Capital, "Veyren");
    assertTrue(static_cast<int>(capital.window_lights.size()) > static_cast<int>(frigate.window_lights.size()),
               "Capital has more windows than frigate");
    assertTrue(capital.panel_tile_count > frigate.panel_tile_count,
               "Capital has more panel tiling than frigate");
    assertTrue(static_cast<int>(capital.markings.size()) > static_cast<int>(frigate.markings.size()),
               "Capital has more markings than frigate");
}

static void testTextureEngineGlowFaction() {
    std::cout << "\n=== Texture Engine Glow By Faction ===" << std::endl;
    pcg::PCGContext ctx{ 700, 1 };
    auto veyren = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Frigate, "Veyren");
    auto keldari = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Frigate, "Keldari");
    // Veyren engine glow is blue (B channel dominant).
    assertTrue(veyren.engine_glow.color_b > veyren.engine_glow.color_r,
               "Veyren engine glow is blue");
    // Keldari engine glow is orange/red (R channel dominant).
    assertTrue(keldari.engine_glow.color_r > keldari.engine_glow.color_b,
               "Keldari engine glow is warm");
    assertTrue(veyren.engine_glow.intensity > 0.0f, "Engine glow intensity positive");
}

static void testTextureMarkingTypeNames() {
    std::cout << "\n=== Texture Marking Type Names ===" << std::endl;
    assertTrue(pcg::ProceduralTextureGenerator::markingTypeName(pcg::MarkingType::StripeHorizontal) == "StripeHorizontal",
               "StripeHorizontal name");
    assertTrue(pcg::ProceduralTextureGenerator::markingTypeName(pcg::MarkingType::FactionInsignia) == "FactionInsignia",
               "FactionInsignia name");
    assertTrue(pcg::ProceduralTextureGenerator::markingTypeName(pcg::MarkingType::WarningHazard) == "WarningHazard",
               "WarningHazard name");
}

static void testTextureAllClassesValid() {
    std::cout << "\n=== Texture All Classes Valid ===" << std::endl;
    std::vector<pcg::HullClass> classes = {
        pcg::HullClass::Frigate, pcg::HullClass::Destroyer,
        pcg::HullClass::Cruiser, pcg::HullClass::Battlecruiser,
        pcg::HullClass::Battleship, pcg::HullClass::Capital
    };
    bool allValid = true;
    for (auto hc : classes) {
        pcg::PCGContext ctx{ static_cast<uint64_t>(hc) + 1000, 1 };
        auto tex = pcg::ProceduralTextureGenerator::generate(ctx, hc, "Veyren");
        if (!tex.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All hull classes produce valid texture params");
}


void run_procedural_texture_generator_tests() {
    testTextureGeneration();
    testTextureDeterminism();
    testTextureFactionPalettes();
    testTextureMaterialByFaction();
    testTextureScalesWithClass();
    testTextureEngineGlowFaction();
    testTextureMarkingTypeNames();
    testTextureAllClassesValid();
}
