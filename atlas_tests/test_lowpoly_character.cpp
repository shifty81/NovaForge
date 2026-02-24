#include <iostream>
#include <cassert>
#include <string>
#include "../editor/tools/PCGPreviewPanel.h"
#include "../cpp_server/include/pcg/lowpoly_character_generator.h"
#include "../cpp_server/include/pcg/pcg_context.h"
#include "../cpp_server/include/pcg/pcg_manager.h"

// ── Helpers ─────────────────────────────────────────────────────────

static int lpc_passed = 0;

static void ok(const char* name) {
    // Output handled by RUN_TEST in main.cpp
    ++lpc_passed;
}

// ── Low-Poly Character Generator Tests ──────────────────────────────

void test_lowpoly_char_generate_default() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);

    auto ch = atlas::pcg::LowPolyCharacterGenerator::generate(ctx);

    assert(ch.characterId == ctx.seed);
    assert(ch.valid);
    assert(ch.flatShaded);
    assert(ch.useVertexColors);
    assert(ch.skeletonId == "base_humanoid");
    assert(!ch.bodyParts.empty());
    assert(!ch.clothing.empty());
    assert(!ch.palette.empty());
    assert(ch.maxTriCount == 6500);

    ok("test_lowpoly_char_generate_default");
}

void test_lowpoly_char_body_slot_count() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(100);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);

    auto ch = atlas::pcg::LowPolyCharacterGenerator::generate(ctx);

    // Should have exactly BODY_SLOT_COUNT (8) body parts:
    // Head, Torso, ArmL, ArmR, LegL, LegR, Hands, Feet
    assert(ch.bodyParts.size() == 8);

    ok("test_lowpoly_char_body_slot_count");
}

void test_lowpoly_char_override_archetype() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);

    auto ch = atlas::pcg::LowPolyCharacterGenerator::generate(
        ctx, atlas::pcg::CharacterArchetype::Militia);

    assert(ch.archetype == atlas::pcg::CharacterArchetype::Militia);
    assert(ch.valid);

    ok("test_lowpoly_char_override_archetype");
}

void test_lowpoly_char_override_gender() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);

    auto male = atlas::pcg::LowPolyCharacterGenerator::generate(
        ctx, atlas::pcg::CharacterArchetype::Survivor, true);
    auto female = atlas::pcg::LowPolyCharacterGenerator::generate(
        ctx, atlas::pcg::CharacterArchetype::Survivor, false);

    assert(male.isMale == true);
    assert(female.isMale == false);

    ok("test_lowpoly_char_override_gender");
}

void test_lowpoly_char_palette_regions() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);

    auto ch = atlas::pcg::LowPolyCharacterGenerator::generate(ctx);

    // Should have exactly 4 palette regions: skin, hair, shirt, pants
    assert(ch.palette.size() == 4);
    assert(ch.palette[0].regionName == "skin");
    assert(ch.palette[1].regionName == "hair");
    assert(ch.palette[2].regionName == "shirt");
    assert(ch.palette[3].regionName == "pants");

    // Each region should have a valid chosen index
    for (const auto& region : ch.palette) {
        assert(!region.colors.empty());
        assert(region.chosen >= 0);
        assert(region.chosen < static_cast<int>(region.colors.size()));
    }

    ok("test_lowpoly_char_palette_regions");
}

void test_lowpoly_char_flat_shading() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);

    auto ch = atlas::pcg::LowPolyCharacterGenerator::generate(ctx);

    // All body parts should be flat-shaded with vertex colors
    for (const auto& part : ch.bodyParts) {
        assert(part.flatShaded);
        assert(part.useVertexColors);
    }
    for (const auto& item : ch.clothing) {
        assert(item.flatShaded);
        assert(item.useVertexColors);
    }

    ok("test_lowpoly_char_flat_shading");
}

void test_lowpoly_char_fps_arms() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);

    auto ch = atlas::pcg::LowPolyCharacterGenerator::generate(ctx);

    // FPS arms should reference the body arm meshes with FPS_ prefix
    assert(!ch.fpsArms.leftArmMesh.empty());
    assert(!ch.fpsArms.rightArmMesh.empty());
    assert(ch.fpsArms.leftArmMesh.substr(0, 4) == "FPS_");
    assert(ch.fpsArms.rightArmMesh.substr(0, 4) == "FPS_");
    assert(ch.fpsArms.skinPaletteIndex == 0); // Skin palette

    ok("test_lowpoly_char_fps_arms");
}

void test_lowpoly_char_determinism() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);

    auto a = atlas::pcg::LowPolyCharacterGenerator::generate(ctx);
    auto b = atlas::pcg::LowPolyCharacterGenerator::generate(ctx);

    assert(a.archetype == b.archetype);
    assert(a.isMale == b.isMale);
    assert(a.bodyParts.size() == b.bodyParts.size());
    assert(a.clothing.size() == b.clothing.size());
    assert(a.palette.size() == b.palette.size());

    for (size_t i = 0; i < a.bodyParts.size(); ++i) {
        assert(a.bodyParts[i].meshFile == b.bodyParts[i].meshFile);
        assert(a.bodyParts[i].variant == b.bodyParts[i].variant);
    }

    for (size_t i = 0; i < a.palette.size(); ++i) {
        assert(a.palette[i].chosen == b.palette[i].chosen);
    }

    ok("test_lowpoly_char_determinism");
}

void test_lowpoly_char_different_seeds_differ() {
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);

    auto ctx1 = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);
    auto ctx2 = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 999, 1);

    auto a = atlas::pcg::LowPolyCharacterGenerator::generate(ctx1);
    auto b = atlas::pcg::LowPolyCharacterGenerator::generate(ctx2);

    // Different seeds should produce at least some variation
    bool anyDifference = (a.archetype != b.archetype) ||
                          (a.isMale != b.isMale) ||
                          (a.palette[0].chosen != b.palette[0].chosen) ||
                          (a.bodyParts[0].variant != b.bodyParts[0].variant);
    assert(anyDifference);

    ok("test_lowpoly_char_different_seeds_differ");
}

void test_lowpoly_char_clothing_always_has_basics() {
    // Shirt and pants should always be present
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);
    auto ctx = mgr.makeRootContext(atlas::pcg::PCGDomain::Character, 1, 1);

    auto ch = atlas::pcg::LowPolyCharacterGenerator::generate(ctx);

    bool hasShirt = false, hasPants = false;
    for (const auto& item : ch.clothing) {
        if (item.meshFile.find("Shirt") != std::string::npos) hasShirt = true;
        if (item.meshFile.find("Pants") != std::string::npos) hasPants = true;
    }
    assert(hasShirt);
    assert(hasPants);

    ok("test_lowpoly_char_clothing_always_has_basics");
}

void test_lowpoly_char_archetype_names() {
    assert(std::string(atlas::pcg::archetypeName(
        atlas::pcg::CharacterArchetype::Survivor)) == "Survivor");
    assert(std::string(atlas::pcg::archetypeName(
        atlas::pcg::CharacterArchetype::Militia)) == "Militia");
    assert(std::string(atlas::pcg::archetypeName(
        atlas::pcg::CharacterArchetype::Civilian)) == "Civilian");
    assert(std::string(atlas::pcg::archetypeName(
        atlas::pcg::CharacterArchetype::Scavenger)) == "Scavenger");
    assert(std::string(atlas::pcg::archetypeName(
        atlas::pcg::CharacterArchetype::Medic)) == "Medic");

    ok("test_lowpoly_char_archetype_names");
}

void test_lowpoly_char_body_slot_names() {
    assert(std::string(atlas::pcg::bodySlotName(
        atlas::pcg::BodySlot::Head)) == "Head");
    assert(std::string(atlas::pcg::bodySlotName(
        atlas::pcg::BodySlot::Feet)) == "Feet");

    ok("test_lowpoly_char_body_slot_names");
}

void test_lowpoly_char_clothing_slot_names() {
    assert(std::string(atlas::pcg::clothingSlotName(
        atlas::pcg::ClothingSlot::Hat)) == "Hat");
    assert(std::string(atlas::pcg::clothingSlotName(
        atlas::pcg::ClothingSlot::Backpack)) == "Backpack");

    ok("test_lowpoly_char_clothing_slot_names");
}

// ── PCG Preview Panel Character Mode Tests ──────────────────────────

void test_pcg_preview_generate_character() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode = atlas::editor::PCGPreviewMode::Character;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetCharacterPreview().populated);
    assert(panel.GetCharacterPreview().data.valid);
    assert(!panel.GetCharacterPreview().data.bodyParts.empty());
    assert(!panel.Log().empty());

    ok("test_pcg_preview_generate_character");
}

void test_pcg_preview_character_override_archetype() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode = atlas::editor::PCGPreviewMode::Character;
    s.overrideArchetype = true;
    s.characterArchetype = atlas::pcg::CharacterArchetype::Medic;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetCharacterPreview().populated);
    assert(panel.GetCharacterPreview().data.archetype ==
           atlas::pcg::CharacterArchetype::Medic);

    ok("test_pcg_preview_character_override_archetype");
}

void test_pcg_preview_character_override_gender() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode = atlas::editor::PCGPreviewMode::Character;
    s.overrideArchetype = true;
    s.characterArchetype = atlas::pcg::CharacterArchetype::Survivor;
    s.overrideGender = true;
    s.characterIsMale = false;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetCharacterPreview().populated);
    assert(panel.GetCharacterPreview().data.isMale == false);

    ok("test_pcg_preview_character_override_gender");
}

void test_pcg_preview_character_clear() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode = atlas::editor::PCGPreviewMode::Character;
    panel.SetSettings(s);
    panel.Generate();
    assert(panel.GetCharacterPreview().populated);

    panel.ClearPreview();
    assert(!panel.GetCharacterPreview().populated);

    ok("test_pcg_preview_character_clear");
}
