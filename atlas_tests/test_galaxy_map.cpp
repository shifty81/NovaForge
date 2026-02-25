/**
 * Tests for GalaxyMapPanel — in-editor galaxy visualization and inspection.
 *
 * Validates:
 * - Default state before generation
 * - Galaxy generation and statistics
 * - Security zone filtering
 * - System selection and inspection
 * - Determinism
 * - Draw does not crash
 * - Log entries
 */

#include "tools/GalaxyMapPanel.h"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace atlas::editor;
using namespace atlas::pcg;

// ── Default state ────────────────────────────────────────────────────

void test_galaxy_panel_defaults() {
    GalaxyMapPanel panel;

    assert(!panel.HasGalaxy());
    assert(panel.TotalSystems()  == 0);
    assert(panel.HighSecCount()  == 0);
    assert(panel.LowSecCount()   == 0);
    assert(panel.NullSecCount()  == 0);
    assert(panel.ChokepointCount() == 0);
    assert(panel.SelectedSystemId() == 0);
    assert(panel.GetSelectedNode() == nullptr);
    assert(panel.GetSecurityFilter() == 0);
    assert(panel.FilteredNodeIndices().empty());
    assert(panel.GetSeed() == 42);
    assert(panel.GetSystemCount() == 100);
    assert(std::string(panel.Name()) == "Galaxy Map");
}

// ── Generation ───────────────────────────────────────────────────────

void test_galaxy_panel_generate() {
    GalaxyMapPanel panel;
    panel.SetSeed(12345);
    panel.SetSystemCount(50);
    panel.Generate();

    assert(panel.HasGalaxy());
    assert(panel.TotalSystems() == 50);
    assert(panel.HighSecCount() > 0);
    assert(panel.LowSecCount()  > 0);
    assert(panel.NullSecCount() > 0);
    int total = panel.HighSecCount() + panel.LowSecCount() + panel.NullSecCount();
    assert(total == 50);
}

void test_galaxy_panel_chokepoints() {
    GalaxyMapPanel panel;
    panel.SetSeed(99);
    panel.SetSystemCount(60);
    panel.Generate();

    assert(panel.HasGalaxy());
    assert(panel.ChokepointCount() > 0);
}

void test_galaxy_panel_system_count_clamped_low() {
    GalaxyMapPanel panel;
    panel.SetSystemCount(3);   // below minimum
    assert(panel.GetSystemCount() == 10);
}

void test_galaxy_panel_system_count_clamped_high() {
    GalaxyMapPanel panel;
    panel.SetSystemCount(9999); // above maximum
    assert(panel.GetSystemCount() == 500);
}

void test_galaxy_panel_log_after_generate() {
    GalaxyMapPanel panel;
    panel.SetSeed(1);
    panel.SetSystemCount(20);
    panel.Generate();

    assert(!panel.Log().empty());
    // Log should mention system count.
    bool found = false;
    for (const auto& entry : panel.Log()) {
        if (entry.find("20") != std::string::npos) { found = true; break; }
    }
    assert(found);
}

// ── Security filtering ───────────────────────────────────────────────

void test_galaxy_panel_filter_all() {
    GalaxyMapPanel panel;
    panel.SetSeed(10);
    panel.SetSystemCount(50);
    panel.Generate();

    panel.SetSecurityFilter(0); // All
    assert(static_cast<int>(panel.FilteredNodeIndices().size()) == 50);
}

void test_galaxy_panel_filter_highsec() {
    GalaxyMapPanel panel;
    panel.SetSeed(10);
    panel.SetSystemCount(50);
    panel.Generate();

    panel.SetSecurityFilter(1); // HighSec only
    int hs = static_cast<int>(panel.FilteredNodeIndices().size());
    assert(hs == panel.HighSecCount());
    assert(hs > 0);
}

void test_galaxy_panel_filter_lowsec() {
    GalaxyMapPanel panel;
    panel.SetSeed(10);
    panel.SetSystemCount(50);
    panel.Generate();

    panel.SetSecurityFilter(2); // LowSec only
    int ls = static_cast<int>(panel.FilteredNodeIndices().size());
    assert(ls == panel.LowSecCount());
}

void test_galaxy_panel_filter_nullsec() {
    GalaxyMapPanel panel;
    panel.SetSeed(10);
    panel.SetSystemCount(50);
    panel.Generate();

    panel.SetSecurityFilter(3); // NullSec only
    int ns = static_cast<int>(panel.FilteredNodeIndices().size());
    assert(ns == panel.NullSecCount());
}

void test_galaxy_panel_filter_sum_equals_total() {
    GalaxyMapPanel panel;
    panel.SetSeed(77);
    panel.SetSystemCount(80);
    panel.Generate();

    panel.SetSecurityFilter(1);
    size_t hs = panel.FilteredNodeIndices().size();
    panel.SetSecurityFilter(2);
    size_t ls = panel.FilteredNodeIndices().size();
    panel.SetSecurityFilter(3);
    size_t ns = panel.FilteredNodeIndices().size();

    assert(static_cast<int>(hs + ls + ns) == panel.TotalSystems());
}

// ── Selection ────────────────────────────────────────────────────────

void test_galaxy_panel_select_system() {
    GalaxyMapPanel panel;
    panel.SetSeed(42);
    panel.SetSystemCount(30);
    panel.Generate();

    assert(panel.HasGalaxy());
    uint64_t firstId = panel.Galaxy().nodes[0].system_id;

    panel.SelectSystem(firstId);
    assert(panel.SelectedSystemId() == firstId);

    const GalaxyNode* node = panel.GetSelectedNode();
    assert(node != nullptr);
    assert(node->system_id == firstId);
}

void test_galaxy_panel_select_nonexistent_ignored() {
    GalaxyMapPanel panel;
    panel.SetSeed(42);
    panel.SetSystemCount(20);
    panel.Generate();

    panel.SelectSystem(0xDEADBEEFDEADBEEFull); // not in the galaxy
    assert(panel.SelectedSystemId() == 0);
    assert(panel.GetSelectedNode() == nullptr);
}

void test_galaxy_panel_clear_selection() {
    GalaxyMapPanel panel;
    panel.SetSeed(42);
    panel.SetSystemCount(20);
    panel.Generate();

    uint64_t id = panel.Galaxy().nodes[0].system_id;
    panel.SelectSystem(id);
    assert(panel.SelectedSystemId() != 0);

    panel.ClearSelection();
    assert(panel.SelectedSystemId() == 0);
    assert(panel.GetSelectedNode() == nullptr);
}

void test_galaxy_panel_generate_clears_selection() {
    GalaxyMapPanel panel;
    panel.SetSeed(42);
    panel.SetSystemCount(20);
    panel.Generate();

    uint64_t id = panel.Galaxy().nodes[0].system_id;
    panel.SelectSystem(id);
    assert(panel.SelectedSystemId() != 0);

    // Regenerating should clear the previous selection.
    panel.Generate();
    assert(panel.SelectedSystemId() == 0);
}

// ── Determinism ──────────────────────────────────────────────────────

void test_galaxy_panel_determinism() {
    GalaxyMapPanel p1, p2;
    p1.SetSeed(999); p1.SetSystemCount(40); p1.Generate();
    p2.SetSeed(999); p2.SetSystemCount(40); p2.Generate();

    assert(p1.TotalSystems()  == p2.TotalSystems());
    assert(p1.HighSecCount()  == p2.HighSecCount());
    assert(p1.LowSecCount()   == p2.LowSecCount());
    assert(p1.NullSecCount()  == p2.NullSecCount());
    assert(p1.ChokepointCount() == p2.ChokepointCount());
    assert(p1.Galaxy().nodes[0].system_id == p2.Galaxy().nodes[0].system_id);
}

void test_galaxy_panel_different_seeds_differ() {
    GalaxyMapPanel p1, p2;
    p1.SetSeed(1); p1.SetSystemCount(40); p1.Generate();
    p2.SetSeed(2); p2.SetSystemCount(40); p2.Generate();

    // The galaxy layouts must differ.
    assert(p1.Galaxy().nodes[0].system_id != p2.Galaxy().nodes[0].system_id);
}

// ── Draw ─────────────────────────────────────────────────────────────

void test_galaxy_panel_draw_does_not_crash() {
    GalaxyMapPanel panel;
    panel.Draw();   // before generation

    panel.SetSeed(42);
    panel.SetSystemCount(30);
    panel.Generate();

    uint64_t id = panel.Galaxy().nodes[0].system_id;
    panel.SelectSystem(id);

    panel.Draw();   // with generated galaxy + selected system
}
