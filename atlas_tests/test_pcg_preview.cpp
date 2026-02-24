#include <iostream>
#include <cassert>
#include <string>
#include "../editor/tools/PCGPreviewPanel.h"

// ── Helpers ─────────────────────────────────────────────────────────

static int passed = 0;

static void ok(const char* name) {
    // Output handled by RUN_TEST in main.cpp
    ++passed;
}

// ── Tests ───────────────────────────────────────────────────────────

void test_pcg_preview_defaults() {
    atlas::editor::PCGPreviewPanel panel;
    auto& s = panel.Settings();

    assert(s.mode         == atlas::editor::PCGPreviewMode::Ship);
    assert(s.seed         == 42);
    assert(s.version      == 1);
    assert(s.overrideHull == false);
    assert(s.overrideModuleCount == false);
    assert(s.shipClass    == 0);

    assert(std::string(panel.Name()) == "PCG Preview");
    assert(panel.GetShipPreview().populated     == false);
    assert(panel.GetStationPreview().populated  == false);
    assert(panel.GetInteriorPreview().populated == false);
    assert(panel.Log().empty());

    ok("test_pcg_preview_defaults");
}

void test_pcg_preview_generate_ship() {
    atlas::editor::PCGPreviewPanel panel;
    panel.Generate();  // defaults to Ship mode

    assert(panel.GetShipPreview().populated);
    auto& ship = panel.GetShipPreview().data;
    assert(!ship.shipName.empty());
    assert(ship.mass > 0.0f);
    assert(ship.thrust > 0.0f);
    assert(!panel.Log().empty());

    ok("test_pcg_preview_generate_ship");
}

void test_pcg_preview_generate_ship_override_hull() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.overrideHull = true;
    s.hullClass    = atlas::pcg::HullClass::Battleship;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetShipPreview().populated);
    assert(panel.GetShipPreview().data.hullClass ==
           atlas::pcg::HullClass::Battleship);

    ok("test_pcg_preview_generate_ship_override_hull");
}

void test_pcg_preview_generate_station() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode = atlas::editor::PCGPreviewMode::Station;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetStationPreview().populated);
    assert(!panel.GetStationPreview().data.modules.empty());
    assert(!panel.Log().empty());

    ok("test_pcg_preview_generate_station");
}

void test_pcg_preview_generate_station_override_count() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode                = atlas::editor::PCGPreviewMode::Station;
    s.overrideModuleCount = true;
    s.moduleCount         = 7;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetStationPreview().populated);
    assert(panel.GetStationPreview().data.modules.size() == 7);

    ok("test_pcg_preview_generate_station_override_count");
}

void test_pcg_preview_generate_interior() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode      = atlas::editor::PCGPreviewMode::Interior;
    s.shipClass = 2;  // Cruiser
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetInteriorPreview().populated);
    assert(!panel.GetInteriorPreview().data.rooms.empty());
    assert(!panel.GetInteriorPreview().data.corridors.empty());

    ok("test_pcg_preview_generate_interior");
}

void test_pcg_preview_determinism() {
    // Same seed + settings must produce identical output.
    atlas::editor::PCGPreviewPanel a;
    atlas::editor::PCGPreviewPanel b;
    a.Generate();
    b.Generate();

    assert(a.GetShipPreview().data.shipName ==
           b.GetShipPreview().data.shipName);
    assert(a.GetShipPreview().data.mass ==
           b.GetShipPreview().data.mass);
    assert(a.GetShipPreview().data.turretSlots ==
           b.GetShipPreview().data.turretSlots);

    ok("test_pcg_preview_determinism");
}

void test_pcg_preview_randomize_changes_seed() {
    atlas::editor::PCGPreviewPanel panel;
    uint64_t originalSeed = panel.Settings().seed;
    panel.Randomize();
    assert(panel.Settings().seed != originalSeed);
    assert(panel.GetShipPreview().populated);

    ok("test_pcg_preview_randomize_changes_seed");
}

void test_pcg_preview_clear() {
    atlas::editor::PCGPreviewPanel panel;
    panel.Generate();
    assert(panel.GetShipPreview().populated);
    assert(!panel.Log().empty());

    panel.ClearPreview();
    assert(!panel.GetShipPreview().populated);
    assert(!panel.GetStationPreview().populated);
    assert(!panel.GetInteriorPreview().populated);
    assert(panel.Log().empty());

    ok("test_pcg_preview_clear");
}

void test_pcg_preview_set_settings() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s;
    s.mode      = atlas::editor::PCGPreviewMode::Interior;
    s.seed      = 9999;
    s.shipClass = 4;  // Battleship-class interior
    panel.SetSettings(s);

    assert(panel.Settings().mode      == atlas::editor::PCGPreviewMode::Interior);
    assert(panel.Settings().seed      == 9999);
    assert(panel.Settings().shipClass == 4);

    ok("test_pcg_preview_set_settings");
}

void test_pcg_preview_different_seeds_differ() {
    atlas::editor::PCGPreviewPanel a;
    atlas::editor::PCGPreviewPanel b;

    atlas::editor::PCGPreviewSettings sa = a.Settings();
    sa.seed = 100;
    a.SetSettings(sa);

    atlas::editor::PCGPreviewSettings sb = b.Settings();
    sb.seed = 200;
    b.SetSettings(sb);

    a.Generate();
    b.Generate();

    // Different seeds should (almost certainly) produce different ships.
    assert(a.GetShipPreview().data.shipName !=
           b.GetShipPreview().data.shipName ||
           a.GetShipPreview().data.mass !=
           b.GetShipPreview().data.mass);

    ok("test_pcg_preview_different_seeds_differ");
}

void test_pcg_preview_draw_does_not_crash() {
    atlas::editor::PCGPreviewPanel panel;
    panel.Generate();
    panel.Draw();  // Should be a safe no-op stub
    ok("test_pcg_preview_draw_does_not_crash");
}
