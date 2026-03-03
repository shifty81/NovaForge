/**
 * Tests for AssetStatsPanel:
 *   - Default state (empty)
 *   - Add/remove/update assets
 *   - Get by entity ID
 *   - Total memory, collisions, physics bodies
 *   - Filter by type
 *   - Sorted by memory
 *   - Unique types
 *   - Clear
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/editor/asset_stats_panel.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// AssetStatsPanel tests
// ══════════════════════════════════════════════════════════════════

void test_stats_panel_defaults() {
    AssetStatsPanel panel;
    assert(panel.Count() == 0);
    assert(panel.TotalMemory() == 0);
    assert(panel.TotalCollisions() == 0);
    assert(panel.TotalPhysicsBodies() == 0);
}

void test_stats_panel_add_and_count() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Frigate", 102400, 12, 3, 5});
    panel.AddAsset({2, "prop", "Crate", 4096, 1, 1, 0});
    assert(panel.Count() == 2);
}

void test_stats_panel_get() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Frigate", 102400, 12, 3, 5});
    const auto* s = panel.Get(1);
    assert(s != nullptr);
    assert(s->name == "Frigate");
    assert(s->memoryBytes == 102400);
    assert(panel.Get(99) == nullptr);
}

void test_stats_panel_remove() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Frigate", 102400, 12, 3, 5});
    assert(panel.RemoveAsset(1));
    assert(panel.Count() == 0);
    assert(!panel.RemoveAsset(1));
}

void test_stats_panel_update() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Frigate", 102400, 12, 3, 5});
    assert(panel.UpdateAsset({1, "ship", "Frigate MkII", 204800, 24, 6, 10}));
    const auto* s = panel.Get(1);
    assert(s->name == "Frigate MkII");
    assert(s->memoryBytes == 204800);
    assert(!panel.UpdateAsset({99, "ship", "Ghost", 0, 0, 0, 0}));
}

void test_stats_panel_total_memory() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Frigate", 100000, 0, 0, 0});
    panel.AddAsset({2, "prop", "Crate", 5000, 0, 0, 0});
    assert(panel.TotalMemory() == 105000);
}

void test_stats_panel_total_collisions() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Frigate", 0, 10, 0, 0});
    panel.AddAsset({2, "prop", "Crate", 0, 3, 0, 0});
    assert(panel.TotalCollisions() == 13);
}

void test_stats_panel_total_physics() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Frigate", 0, 0, 5, 0});
    panel.AddAsset({2, "prop", "Crate", 0, 0, 2, 0});
    assert(panel.TotalPhysicsBodies() == 7);
}

void test_stats_panel_filter_by_type() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Frigate", 0, 0, 0, 0});
    panel.AddAsset({2, "prop", "Crate", 0, 0, 0, 0});
    panel.AddAsset({3, "ship", "Destroyer", 0, 0, 0, 0});
    auto ships = panel.FilterByType("ship");
    assert(ships.size() == 2);
    auto chars = panel.FilterByType("character");
    assert(chars.empty());
}

void test_stats_panel_sorted_by_memory() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Small", 1000, 0, 0, 0});
    panel.AddAsset({2, "ship", "Large", 50000, 0, 0, 0});
    panel.AddAsset({3, "prop", "Medium", 10000, 0, 0, 0});
    auto sorted = panel.SortedByMemory();
    assert(sorted.size() == 3);
    assert(sorted[0]->memoryBytes >= sorted[1]->memoryBytes);
    assert(sorted[1]->memoryBytes >= sorted[2]->memoryBytes);
}

void test_stats_panel_unique_types() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "A", 0, 0, 0, 0});
    panel.AddAsset({2, "prop", "B", 0, 0, 0, 0});
    panel.AddAsset({3, "ship", "C", 0, 0, 0, 0});
    auto types = panel.UniqueTypes();
    assert(types.size() == 2);
}

void test_stats_panel_clear() {
    AssetStatsPanel panel;
    panel.AddAsset({1, "ship", "Frigate", 102400, 12, 3, 5});
    panel.Clear();
    assert(panel.Count() == 0);
    assert(panel.TotalMemory() == 0);
}
