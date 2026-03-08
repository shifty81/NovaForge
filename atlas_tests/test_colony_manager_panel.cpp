/**
 * Tests for ColonyManagerPanel:
 *   - Construction & defaults
 *   - Add/remove buildings
 *   - Building validation
 *   - Toggle buildings on/off
 *   - Power budget enforcement
 *   - Goods management
 *   - Export goods
 *   - Max buildings
 *   - Export JSON
 *   - Headless draw
 */
#include <cassert>
#include <string>
#include <cmath>
#include "tools/ColonyManagerPanel.h"

using atlas::editor::ColonyManagerPanel;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::abs(a - b) < eps;
}

void test_colony_manager_defaults() {
    ColonyManagerPanel panel;
    assert(panel.BuildingCount() == 0);
    assert(panel.OnlineCount() == 0);
    assert(approxEq(panel.PowerUsed(), 0.0f));
    assert(panel.PowerCapacity() > 0.0f);
    assert(approxEq(panel.TotalExports(), 0.0f));
    assert(approxEq(panel.TotalExportValue(), 0.0f));
    assert(panel.GoodTypeCount() == 0);
}

void test_colony_manager_add_building() {
    ColonyManagerPanel panel;
    int id = panel.AddBuilding("Extractor A", "extractor", 1.0f, 10.0f);
    assert(id > 0);
    assert(panel.BuildingCount() == 1);
    assert(panel.OnlineCount() == 1);
    assert(approxEq(panel.PowerUsed(), 10.0f));
}

void test_colony_manager_building_validation() {
    ColonyManagerPanel panel;
    assert(panel.AddBuilding("", "extractor", 1.0f, 10.0f) == -1);
    assert(panel.AddBuilding("E1", "", 1.0f, 10.0f) == -1);
    assert(panel.BuildingCount() == 0);
}

void test_colony_manager_remove_building() {
    ColonyManagerPanel panel;
    int id1 = panel.AddBuilding("E1", "extractor", 1.0f, 10.0f);
    panel.AddBuilding("E2", "processor", 0.5f, 15.0f);
    assert(panel.RemoveBuilding(id1));
    assert(panel.BuildingCount() == 1);
    assert(!panel.RemoveBuilding(999));
}

void test_colony_manager_toggle_building() {
    ColonyManagerPanel panel;
    int id = panel.AddBuilding("E1", "extractor", 1.0f, 10.0f);
    assert(panel.OnlineCount() == 1);
    assert(panel.ToggleBuilding(id)); // offline
    assert(panel.OnlineCount() == 0);
    assert(panel.ToggleBuilding(id)); // online
    assert(panel.OnlineCount() == 1);
    assert(!panel.ToggleBuilding(999));
}

void test_colony_manager_power_budget() {
    ColonyManagerPanel panel;
    panel.SetPowerCapacity(50.0f);
    assert(panel.AddBuilding("E1", "extractor", 1.0f, 30.0f) > 0);
    assert(panel.AddBuilding("E2", "processor", 1.0f, 15.0f) > 0);
    assert(panel.AddBuilding("E3", "storage", 1.0f, 10.0f) == -1); // exceeds budget
    assert(approxEq(panel.RemainingPower(), 5.0f));
}

void test_colony_manager_add_goods() {
    ColonyManagerPanel panel;
    assert(panel.AddGoods("Tritanium", 500.0f, 1000.0f));
    assert(approxEq(panel.GetGoodsQuantity("Tritanium"), 500.0f));
    // Stack more
    assert(panel.AddGoods("Tritanium", 300.0f, 1000.0f));
    assert(approxEq(panel.GetGoodsQuantity("Tritanium"), 800.0f));
    // Cap at max
    panel.AddGoods("Tritanium", 500.0f, 1000.0f);
    assert(approxEq(panel.GetGoodsQuantity("Tritanium"), 1000.0f));
    assert(panel.GoodTypeCount() == 1);
}

void test_colony_manager_export_goods() {
    ColonyManagerPanel panel;
    panel.AddGoods("Tritanium", 500.0f, 1000.0f);
    assert(panel.ExportGoods("Tritanium", 200.0f, 10.0f));
    assert(approxEq(panel.GetGoodsQuantity("Tritanium"), 300.0f));
    assert(approxEq(panel.TotalExports(), 200.0f));
    assert(approxEq(panel.TotalExportValue(), 2000.0f));
    assert(!panel.ExportGoods("Tritanium", 500.0f, 10.0f)); // insufficient
    assert(!panel.ExportGoods("Nonexistent", 10.0f, 10.0f)); // unknown type
}

void test_colony_manager_goods_validation() {
    ColonyManagerPanel panel;
    assert(!panel.AddGoods("", 100.0f, 1000.0f));
    assert(!panel.ExportGoods("Tritanium", 0.0f, 10.0f));
    assert(!panel.ExportGoods("Tritanium", 10.0f, 0.0f));
}

void test_colony_manager_max_buildings() {
    ColonyManagerPanel panel;
    panel.SetPowerCapacity(10000.0f);
    for (int i = 0; i < 30; ++i) {
        assert(panel.AddBuilding("B_" + std::to_string(i), "extractor", 1.0f, 1.0f) > 0);
    }
    assert(panel.AddBuilding("Overflow", "extractor", 1.0f, 1.0f) == -1);
    assert(panel.BuildingCount() == 30);
}

void test_colony_manager_export_json() {
    ColonyManagerPanel panel;
    panel.AddBuilding("Extractor", "extractor", 1.5f, 20.0f);
    panel.AddGoods("Pyerite", 100.0f, 500.0f);
    std::string json = panel.ExportJSON();
    assert(json.find("Extractor") != std::string::npos);
    assert(json.find("extractor") != std::string::npos);
    assert(json.find("Pyerite") != std::string::npos);
    assert(json.find("totalExports") != std::string::npos);
    assert(json.find("powerCapacity") != std::string::npos);
}

void test_colony_manager_headless_draw() {
    ColonyManagerPanel panel;
    panel.AddBuilding("E1", "extractor", 1.0f, 10.0f);
    panel.Draw(); // should not crash without context
}
