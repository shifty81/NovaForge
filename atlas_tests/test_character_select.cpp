#ifndef ATLAS_NO_GLM

#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include "../editor/tools/CharacterSelectPanel.h"
#include "../cpp_server/include/pcg/lowpoly_character_generator.h"
#include "../cpp_client/include/characters/character_mesh_system.h"

// ── Helpers ─────────────────────────────────────────────────────────

static int csp_passed = 0;
static void ok(const char* /*name*/) { ++csp_passed; }

// ── CharacterSelectPanel tests ──────────────────────────────────────

void test_charsel_defaults() {
    atlas::editor::CharacterSelectPanel panel;
    auto s = panel.Settings();

    assert(s.seed == 42);
    assert(s.archetype == atlas::pcg::CharacterArchetype::Survivor);
    assert(s.isMale == true);
    assert(std::abs(s.height     - 1.0f) < 1e-5f);
    assert(std::abs(s.torsoWidth - 1.0f) < 1e-5f);
    assert(std::abs(s.armLength  - 1.0f) < 1e-5f);
    assert(std::abs(s.legLength  - 1.0f) < 1e-5f);
    assert(s.referenceMeshArchive == "human.zip");

    assert(!panel.GetPreview().populated);
    assert(!panel.Log().empty()); // Init message

    ok("test_charsel_defaults");
}

void test_charsel_generate() {
    atlas::editor::CharacterSelectPanel panel;
    panel.Generate();

    assert(panel.GetPreview().populated);
    assert(panel.GetPreview().generatedCharacter.valid);
    assert(!panel.GetPreview().generatedCharacter.bodyParts.empty());
    assert(!panel.GetPreview().generatedCharacter.clothing.empty());
    assert(panel.GetPreview().generatedCharacter.blenderSourceArchive == "human.zip");

    // Mesh character should have default sliders applied
    assert(!panel.GetPreview().meshCharacter.bodyMeshes.empty());
    assert(panel.GetPreview().meshCharacter.referenceMeshArchive == "human.zip");

    ok("test_charsel_generate");
}

void test_charsel_set_archetype() {
    atlas::editor::CharacterSelectPanel panel;
    auto s = panel.Settings();
    s.archetype = atlas::pcg::CharacterArchetype::Medic;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetPreview().populated);
    assert(panel.GetPreview().generatedCharacter.archetype ==
           atlas::pcg::CharacterArchetype::Medic);

    ok("test_charsel_set_archetype");
}

void test_charsel_set_gender() {
    atlas::editor::CharacterSelectPanel panel;
    auto s = panel.Settings();
    s.isMale = false;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetPreview().populated);
    assert(panel.GetPreview().generatedCharacter.isMale == false);

    ok("test_charsel_set_gender");
}

void test_charsel_body_sliders() {
    atlas::editor::CharacterSelectPanel panel;
    auto s = panel.Settings();
    s.height     = 1.10f;
    s.torsoWidth = 0.90f;
    s.armLength  = 1.05f;
    s.legLength  = 0.95f;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetPreview().populated);
    // Verify the mesh character has the slider values applied
    const auto& mc = panel.GetPreview().meshCharacter;
    bool foundHeight = false;
    for (const auto& sl : mc.sliders) {
        if (sl.name == "height") {
            assert(std::abs(sl.currentValue - 1.10f) < 1e-4f);
            foundHeight = true;
        }
    }
    assert(foundHeight);

    ok("test_charsel_body_sliders");
}

void test_charsel_randomize() {
    atlas::editor::CharacterSelectPanel panel;
    uint64_t oldSeed = panel.Settings().seed;
    panel.Randomize();

    assert(panel.Settings().seed != oldSeed);
    assert(panel.GetPreview().populated);

    ok("test_charsel_randomize");
}

void test_charsel_clear_preview() {
    atlas::editor::CharacterSelectPanel panel;
    panel.Generate();
    assert(panel.GetPreview().populated);

    panel.ClearPreview();
    assert(!panel.GetPreview().populated);

    ok("test_charsel_clear_preview");
}

void test_charsel_orbit_camera() {
    atlas::editor::CharacterSelectPanel panel;
    float yaw0   = panel.GetCameraYaw();
    float pitch0 = panel.GetCameraPitch();

    panel.OrbitCamera(45.0f, 10.0f);
    assert(std::abs(panel.GetCameraYaw()   - (yaw0   + 45.0f)) < 1e-5f);
    assert(std::abs(panel.GetCameraPitch()  - (pitch0 + 10.0f)) < 1e-5f);

    ok("test_charsel_orbit_camera");
}

void test_charsel_orbit_camera_pitch_clamp() {
    atlas::editor::CharacterSelectPanel panel;
    panel.OrbitCamera(0.0f, 200.0f);
    assert(panel.GetCameraPitch() <= 89.0f);

    panel.OrbitCamera(0.0f, -400.0f);
    assert(panel.GetCameraPitch() >= -89.0f);

    ok("test_charsel_orbit_camera_pitch_clamp");
}

void test_charsel_camera_distance() {
    atlas::editor::CharacterSelectPanel panel;
    panel.SetCameraDistance(5.0f);
    assert(std::abs(panel.GetCameraDistance() - 5.0f) < 1e-5f);

    ok("test_charsel_camera_distance");
}

void test_charsel_reference_mesh_archive() {
    atlas::editor::CharacterSelectPanel panel;
    auto s = panel.Settings();
    s.referenceMeshArchive = "custom.zip";
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetPreview().generatedCharacter.blenderSourceArchive == "custom.zip");
    assert(panel.GetPreview().meshCharacter.referenceMeshArchive == "custom.zip");

    ok("test_charsel_reference_mesh_archive");
}

void test_charsel_determinism() {
    atlas::editor::CharacterSelectPanel panelA;
    atlas::editor::CharacterSelectPanel panelB;

    panelA.Generate();
    panelB.Generate();

    const auto& a = panelA.GetPreview().generatedCharacter;
    const auto& b = panelB.GetPreview().generatedCharacter;

    assert(a.archetype == b.archetype);
    assert(a.isMale == b.isMale);
    assert(a.bodyParts.size() == b.bodyParts.size());
    assert(a.clothing.size() == b.clothing.size());

    for (size_t i = 0; i < a.bodyParts.size(); ++i) {
        assert(a.bodyParts[i].meshFile == b.bodyParts[i].meshFile);
    }

    ok("test_charsel_determinism");
}

void test_charsel_draw_does_not_crash() {
    atlas::editor::CharacterSelectPanel panel;
    panel.Generate();
    // Draw without context — should silently return.
    panel.Draw();

    ok("test_charsel_draw_does_not_crash");
}

void test_charsel_panel_name() {
    atlas::editor::CharacterSelectPanel panel;
    assert(std::string(panel.Name()) == "Character Select");

    ok("test_charsel_panel_name");
}

void test_charsel_log_entries() {
    atlas::editor::CharacterSelectPanel panel;
    size_t initLogSize = panel.Log().size();
    panel.Generate();
    assert(panel.Log().size() > initLogSize);

    ok("test_charsel_log_entries");
}

#endif // ATLAS_NO_GLM
