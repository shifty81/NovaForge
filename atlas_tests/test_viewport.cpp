/**
 * Tests for ViewportPanel — visual PCG preview and manipulation.
 *
 * Validates:
 * - Scene loading from PCG-generated ships and stations
 * - Object selection and deselection
 * - Transform gizmo operations (translate, rotate, scale)
 * - Camera orbit controls
 * - Change tracking, commit, and discard
 * - Scene clearing
 */

#include "tools/ViewportPanel.h"
#include "../cpp_server/include/pcg/pcg_manager.h"
#include "../cpp_server/include/pcg/ship_generator.h"
#include "../cpp_server/include/pcg/station_generator.h"
#include "../cpp_server/include/pcg/spine_hull_generator.h"
#include "../cpp_server/include/pcg/turret_placement_system.h"
#include "../cpp_server/include/pcg/lowpoly_character_generator.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::editor;
using namespace atlas::pcg;

// ── Scene loading tests ───────────────────────────────────────────

void test_viewport_defaults() {
    ViewportPanel vp;
    assert(vp.ObjectCount() == 0);
    assert(vp.SelectedObjectId() == 0);
    assert(vp.GetGizmoMode() == GizmoMode::Translate);
    assert(vp.IsGridVisible() == true);
    assert(!vp.HasPendingChanges());
    assert(vp.GetCameraDistance() == 500.0f);

}

void test_viewport_load_ship() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);

    vp.LoadShip(ship, 12345);

    // Should have hull + turret objects
    assert(vp.ObjectCount() > 0);
    // First object should be the ship hull
    assert(vp.GetObject(0).type == "Ship");
    assert(!vp.GetObject(0).name.empty());

}

void test_viewport_load_station() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(99999);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Station, 1, 1);
    GeneratedStation station = StationGenerator::generate(ctx);

    vp.LoadStation(station, 99999);

    // Should have one object per module
    assert(vp.ObjectCount() == station.modules.size());
    if (vp.ObjectCount() > 0) {
        assert(vp.GetObject(0).type == "Module");
    }

}

void test_viewport_clear_scene() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);

    vp.LoadShip(ship, 12345);
    assert(vp.ObjectCount() > 0);

    vp.ClearScene();
    assert(vp.ObjectCount() == 0);
    assert(vp.SelectedObjectId() == 0);
    assert(!vp.HasPendingChanges());

}

// ── Selection tests ───────────────────────────────────────────────

void test_viewport_select_object() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    uint32_t firstId = vp.GetObject(0).id;
    vp.SelectObject(firstId);
    assert(vp.SelectedObjectId() == firstId);
    assert(vp.GetObject(0).selected);

}

void test_viewport_deselect_all() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    uint32_t firstId = vp.GetObject(0).id;
    vp.SelectObject(firstId);
    assert(vp.SelectedObjectId() == firstId);

    vp.DeselectAll();
    assert(vp.SelectedObjectId() == 0);
    assert(!vp.GetObject(0).selected);

}

// ── Transform gizmo tests ────────────────────────────────────────

void test_viewport_translate_selected() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    uint32_t id = vp.GetObject(0).id;
    vp.SelectObject(id);

    float origX = vp.GetTransform(id).posX;
    float origY = vp.GetTransform(id).posY;
    float origZ = vp.GetTransform(id).posZ;

    vp.TranslateSelected(10.0f, 20.0f, 30.0f);

    assert(std::abs(vp.GetTransform(id).posX - (origX + 10.0f)) < 0.01f);
    assert(std::abs(vp.GetTransform(id).posY - (origY + 20.0f)) < 0.01f);
    assert(std::abs(vp.GetTransform(id).posZ - (origZ + 30.0f)) < 0.01f);
    assert(vp.HasPendingChanges());

}

void test_viewport_rotate_selected() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    uint32_t id = vp.GetObject(0).id;
    vp.SelectObject(id);

    vp.RotateSelected(45.0f, 0.0f, 90.0f);

    assert(std::abs(vp.GetTransform(id).rotX - 45.0f) < 0.01f);
    assert(std::abs(vp.GetTransform(id).rotZ - 90.0f) < 0.01f);
    assert(vp.HasPendingChanges());

}

void test_viewport_scale_selected() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    uint32_t id = vp.GetObject(0).id;
    vp.SelectObject(id);

    float origSX = vp.GetTransform(id).scaleX;
    vp.ScaleSelected(5.0f, 5.0f, 5.0f);

    assert(vp.GetTransform(id).scaleX > origSX);
    assert(vp.HasPendingChanges());

}

void test_viewport_scale_clamps_positive() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    uint32_t id = vp.GetObject(0).id;
    vp.SelectObject(id);

    // Try to scale to negative — should be clamped to 0.01
    vp.ScaleSelected(-9999.0f, -9999.0f, -9999.0f);
    assert(vp.GetTransform(id).scaleX >= 0.01f);
    assert(vp.GetTransform(id).scaleY >= 0.01f);
    assert(vp.GetTransform(id).scaleZ >= 0.01f);

}

void test_viewport_gizmo_mode() {
    ViewportPanel vp;
    assert(vp.GetGizmoMode() == GizmoMode::Translate);

    vp.SetGizmoMode(GizmoMode::Rotate);
    assert(vp.GetGizmoMode() == GizmoMode::Rotate);

    vp.SetGizmoMode(GizmoMode::Scale);
    assert(vp.GetGizmoMode() == GizmoMode::Scale);

    vp.SetGizmoMode(GizmoMode::None);
    assert(vp.GetGizmoMode() == GizmoMode::None);

}

// ── Camera tests ──────────────────────────────────────────────────

void test_viewport_camera_orbit() {
    ViewportPanel vp;
    float origYaw = vp.GetCameraYaw();
    float origPitch = vp.GetCameraPitch();

    vp.OrbitCamera(45.0f, -10.0f);

    assert(std::abs(vp.GetCameraYaw() - (origYaw + 45.0f)) < 0.01f);
    assert(std::abs(vp.GetCameraPitch() - (origPitch - 10.0f)) < 0.01f);

}

void test_viewport_camera_pitch_clamp() {
    ViewportPanel vp;
    // Try to pitch beyond limits
    vp.OrbitCamera(0.0f, 1000.0f);
    assert(vp.GetCameraPitch() <= 89.0f);

    vp.OrbitCamera(0.0f, -2000.0f);
    assert(vp.GetCameraPitch() >= -89.0f);

}

void test_viewport_camera_distance() {
    ViewportPanel vp;
    vp.SetCameraDistance(200.0f);
    assert(std::abs(vp.GetCameraDistance() - 200.0f) < 0.01f);

}

// ── Change tracking tests ─────────────────────────────────────────

void test_viewport_commit_changes() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    uint32_t id = vp.GetObject(0).id;
    vp.SelectObject(id);
    vp.TranslateSelected(10.0f, 0.0f, 0.0f);
    assert(vp.HasPendingChanges());

    auto changes = vp.CommitChanges();
    assert(!changes.empty());
    assert(changes[0].field == "position");
    assert(!vp.HasPendingChanges());

}

void test_viewport_discard_changes() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    uint32_t id = vp.GetObject(0).id;
    float origX = vp.GetTransform(id).posX;

    vp.SelectObject(id);
    vp.TranslateSelected(100.0f, 0.0f, 0.0f);
    assert(vp.HasPendingChanges());

    vp.DiscardChanges();
    assert(!vp.HasPendingChanges());
    // Transform should revert to original
    assert(std::abs(vp.GetTransform(id).posX - origX) < 0.01f);

}

// ── Misc tests ────────────────────────────────────────────────────

void test_viewport_grid_toggle() {
    ViewportPanel vp;
    assert(vp.IsGridVisible());

    vp.SetGridVisible(false);
    assert(!vp.IsGridVisible());

    vp.SetGridVisible(true);
    assert(vp.IsGridVisible());

}

void test_viewport_draw_does_not_crash() {
    ViewportPanel vp;
    vp.Draw(); // should not crash

    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);
    vp.Draw(); // should not crash with loaded scene

}

void test_viewport_log_entries() {
    ViewportPanel vp;
    assert(!vp.Log().empty()); // Should have init message

    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    size_t logSize = vp.Log().size();
    assert(logSize >= 2); // init + ship load

    vp.ClearScene();
    assert(vp.Log().size() > logSize); // clear also logs

}

void test_viewport_no_op_without_selection() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(12345);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedShip ship = ShipGenerator::generate(ctx);
    vp.LoadShip(ship, 12345);

    // No selection — transform should be a no-op
    vp.TranslateSelected(100.0f, 100.0f, 100.0f);
    vp.RotateSelected(90.0f, 0.0f, 0.0f);
    vp.ScaleSelected(5.0f, 5.0f, 5.0f);
    assert(!vp.HasPendingChanges());

}

// ── Spine Hull viewport tests ─────────────────────────────────────

void test_viewport_load_spine_hull() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(55555);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedSpineHull hull = SpineHullGenerator::generate(ctx, HullClass::Cruiser);

    vp.LoadSpineHull(hull, nullptr, 55555);

    // Should have one object per zone (3 zones).
    assert(vp.ObjectCount() == 3);
    assert(vp.GetObject(0).type == "HullZone");
    assert(vp.GetObject(1).type == "HullZone");
    assert(vp.GetObject(2).type == "HullZone");

}

void test_viewport_load_spine_hull_with_turrets() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(55555);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedSpineHull hull = SpineHullGenerator::generate(ctx, HullClass::Battleship);

    PCGContext ctx2 = mgr.makeRootContext(PCGDomain::Ship, 2, 1);
    TurretPlacement placement = TurretPlacementSystem::place(ctx2, hull.hull_class, 5);

    vp.LoadSpineHull(hull, &placement, 55555);

    // 3 zones + 5 turret mounts = 8 objects
    assert(vp.ObjectCount() == 8);

    // First 3 should be HullZone
    for (size_t i = 0; i < 3; ++i) {
        assert(vp.GetObject(i).type == "HullZone");
    }
    // Next 5 should be TurretMount
    for (size_t i = 3; i < 8; ++i) {
        assert(vp.GetObject(i).type == "TurretMount");
    }

}

void test_viewport_spine_hull_zone_names() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(77777);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedSpineHull hull = SpineHullGenerator::generate(ctx, HullClass::Frigate, "Solari");

    vp.LoadSpineHull(hull, nullptr, 77777);

    assert(vp.ObjectCount() == 3);
    // Zone names should contain the functional zone name
    std::string n0 = vp.GetObject(0).name;
    std::string n1 = vp.GetObject(1).name;
    std::string n2 = vp.GetObject(2).name;
    assert(n0.find("Command") != std::string::npos);
    assert(n1.find("MidHull") != std::string::npos);
    assert(n2.find("Engineering") != std::string::npos);

}

void test_viewport_spine_hull_select_and_transform() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(55555);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Ship, 1, 1);
    GeneratedSpineHull hull = SpineHullGenerator::generate(ctx, HullClass::Cruiser);

    vp.LoadSpineHull(hull, nullptr, 55555);

    uint32_t zoneId = vp.GetObject(0).id;
    vp.SelectObject(zoneId);
    assert(vp.SelectedObjectId() == zoneId);

    float origX = vp.GetTransform(zoneId).posX;
    vp.TranslateSelected(10.0f, 0.0f, 0.0f);
    assert(std::abs(vp.GetTransform(zoneId).posX - (origX + 10.0f)) < 0.01f);
    assert(vp.HasPendingChanges());

}

// ── Character viewport tests ──────────────────────────────────────

void test_viewport_load_character() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(11111);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Character, 1, 1);
    GeneratedLowPolyCharacter ch = LowPolyCharacterGenerator::generate(ctx);

    vp.LoadCharacter(ch, 11111);

    // Should have body parts + clothing as separate objects.
    size_t expected = ch.bodyParts.size() + ch.clothing.size();
    assert(vp.ObjectCount() == expected);

}

void test_viewport_load_character_body_part_type() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(22222);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Character, 1, 1);
    GeneratedLowPolyCharacter ch = LowPolyCharacterGenerator::generate(ctx);

    vp.LoadCharacter(ch, 22222);

    // First objects (body parts) must have type "BodyPart".
    for (size_t i = 0; i < ch.bodyParts.size(); ++i) {
        assert(vp.GetObject(i).type == "BodyPart");
    }

}

void test_viewport_load_character_clothing_type() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(33333);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Character, 1, 1);
    GeneratedLowPolyCharacter ch = LowPolyCharacterGenerator::generate(ctx);

    vp.LoadCharacter(ch, 33333);

    // Objects after body parts must have type "Clothing".
    size_t bodyCount = ch.bodyParts.size();
    for (size_t i = bodyCount; i < vp.ObjectCount(); ++i) {
        assert(vp.GetObject(i).type == "Clothing");
    }

}

void test_viewport_load_character_select_and_transform() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(44444);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Character, 1, 1);
    GeneratedLowPolyCharacter ch = LowPolyCharacterGenerator::generate(ctx);

    vp.LoadCharacter(ch, 44444);
    assert(vp.ObjectCount() > 0);

    uint32_t partId = vp.GetObject(0).id;
    vp.SelectObject(partId);
    assert(vp.SelectedObjectId() == partId);

    float origX = vp.GetTransform(partId).posX;
    vp.TranslateSelected(5.0f, 0.0f, 0.0f);
    assert(std::abs(vp.GetTransform(partId).posX - (origX + 5.0f)) < 0.01f);
    assert(vp.HasPendingChanges());

}

void test_viewport_load_character_log_entry() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(55555);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Character, 1, 1);
    GeneratedLowPolyCharacter ch = LowPolyCharacterGenerator::generate(ctx);

    size_t logBefore = vp.Log().size();
    vp.LoadCharacter(ch, 55555);

    assert(vp.Log().size() > logBefore);

}

void test_viewport_load_character_female() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(66666);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Character, 1, 1);
    GeneratedLowPolyCharacter ch = LowPolyCharacterGenerator::generate(
        ctx, CharacterArchetype::Medic, /*isMale=*/false);

    vp.LoadCharacter(ch, 66666);

    size_t expected = ch.bodyParts.size() + ch.clothing.size();
    assert(vp.ObjectCount() == expected);
    // All body-part objects present.
    for (size_t i = 0; i < ch.bodyParts.size(); ++i) {
        assert(vp.GetObject(i).type == "BodyPart");
    }

}

void test_viewport_load_character_clear_scene() {
    ViewportPanel vp;
    PCGManager mgr;
    mgr.initialize(77777);
    PCGContext ctx = mgr.makeRootContext(PCGDomain::Character, 1, 1);
    GeneratedLowPolyCharacter ch = LowPolyCharacterGenerator::generate(ctx);

    vp.LoadCharacter(ch, 77777);
    assert(vp.ObjectCount() > 0);

    vp.ClearScene();
    assert(vp.ObjectCount() == 0);
    assert(vp.SelectedObjectId() == 0);
    assert(!vp.HasPendingChanges());

}
