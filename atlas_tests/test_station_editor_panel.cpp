/**
 * Tests for StationEditorPanel:
 *   - Construction & defaults
 *   - Identity (name, faction, security)
 *   - Segment CRUD
 *   - Docking port CRUD
 *   - Service CRUD (add, remove, toggle, cost multiplier)
 *   - Max limits enforcement
 *   - JSON export
 *   - Headless draw safety
 */

#include <cassert>
#include <string>
#include "../editor/tools/StationEditorPanel.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// StationEditorPanel tests
// ══════════════════════════════════════════════════════════════════

void test_station_editor_defaults() {
    StationEditorPanel panel;
    assert(std::string(panel.Name()) == "Station Editor");
    assert(panel.StationName() == "New Station");
    assert(panel.Faction() == "caldari");
    assert(panel.SecurityLevel() > 0.79f && panel.SecurityLevel() < 0.81f);
    assert(panel.SegmentCount() == 0);
    assert(panel.DockingPortCount() == 0);
    assert(panel.ServiceCount() == 0);
    assert(panel.MaxDockingCapacity() == 20);
    assert(panel.UndockClearanceRadius() > 499.0f);
    assert(panel.EnabledServiceCount() == 0);
}

void test_station_editor_identity() {
    StationEditorPanel panel;

    panel.SetStationName("Jita IV - Moon 4 - Caldari Navy Assembly Plant");
    assert(panel.StationName() == "Jita IV - Moon 4 - Caldari Navy Assembly Plant");

    panel.SetFaction("gallente");
    assert(panel.Faction() == "gallente");

    panel.SetSecurityLevel(0.5f);
    assert(panel.SecurityLevel() > 0.49f && panel.SecurityLevel() < 0.51f);

    // Clamped
    panel.SetSecurityLevel(1.5f);
    assert(panel.SecurityLevel() <= 1.0f);
    panel.SetSecurityLevel(-0.5f);
    assert(panel.SecurityLevel() >= 0.0f);

    // Empty name/faction rejected (no change)
    panel.SetStationName("");
    assert(panel.StationName() == "Jita IV - Moon 4 - Caldari Navy Assembly Plant");
    panel.SetFaction("");
    assert(panel.Faction() == "gallente");
}

void test_station_editor_add_segment() {
    StationEditorPanel panel;

    int id1 = panel.AddSegment("arm", 200.0f, 30.0f);
    assert(id1 > 0);
    assert(panel.SegmentCount() == 1);

    int id2 = panel.AddSegment("ring", 150.0f, 80.0f);
    assert(id2 > 0);
    assert(id2 != id1);
    assert(panel.SegmentCount() == 2);

    // Invalid type rejected
    assert(panel.AddSegment("invalid_type", 100.0f, 50.0f) == -1);

    // Invalid dimensions rejected
    assert(panel.AddSegment("arm", 0.0f, 50.0f) == -1);
    assert(panel.AddSegment("arm", 100.0f, -5.0f) == -1);
}

void test_station_editor_remove_segment() {
    StationEditorPanel panel;

    int id1 = panel.AddSegment("arm", 200.0f, 30.0f);
    panel.AddSegment("ring", 150.0f, 80.0f);

    assert(panel.RemoveSegment(static_cast<uint32_t>(id1)));
    assert(panel.SegmentCount() == 1);

    // Can't remove twice
    assert(!panel.RemoveSegment(static_cast<uint32_t>(id1)));
    // Can't remove nonexistent
    assert(!panel.RemoveSegment(99999));
}

void test_station_editor_segment_types() {
    StationEditorPanel panel;

    // All valid types
    assert(panel.AddSegment("arm", 100.0f, 30.0f) > 0);
    assert(panel.AddSegment("ring", 100.0f, 80.0f) > 0);
    assert(panel.AddSegment("hub", 50.0f, 50.0f) > 0);
    assert(panel.AddSegment("hangar", 200.0f, 40.0f) > 0);
    assert(panel.AddSegment("solar_panel", 300.0f, 10.0f) > 0);
    assert(panel.SegmentCount() == 5);

    // Verify segments data
    const auto& segs = panel.Segments();
    assert(segs[0].segmentType == "arm");
    assert(segs[1].segmentType == "ring");
    assert(segs[4].segmentType == "solar_panel");
}

void test_station_editor_add_docking_port() {
    StationEditorPanel panel;

    int id1 = panel.AddDockingPort("Bay Alpha", "large");
    assert(id1 > 0);
    assert(panel.DockingPortCount() == 1);

    int id2 = panel.AddDockingPort("Dock 2", "small");
    assert(id2 > 0);
    assert(panel.DockingPortCount() == 2);

    // Invalid size rejected
    assert(panel.AddDockingPort("Bad", "huge") == -1);
    // Empty name rejected
    assert(panel.AddDockingPort("", "small") == -1);
}

void test_station_editor_remove_docking_port() {
    StationEditorPanel panel;

    int id1 = panel.AddDockingPort("Bay Alpha", "large");
    panel.AddDockingPort("Dock 2", "small");

    assert(panel.RemoveDockingPort(static_cast<uint32_t>(id1)));
    assert(panel.DockingPortCount() == 1);

    assert(!panel.RemoveDockingPort(static_cast<uint32_t>(id1)));
    assert(!panel.RemoveDockingPort(99999));
}

void test_station_editor_docking_port_sizes() {
    StationEditorPanel panel;

    assert(panel.AddDockingPort("Port S", "small") > 0);
    assert(panel.AddDockingPort("Port M", "medium") > 0);
    assert(panel.AddDockingPort("Port L", "large") > 0);
    assert(panel.AddDockingPort("Port C", "capital") > 0);
    assert(panel.DockingPortCount() == 4);

    const auto& ports = panel.DockingPorts();
    assert(ports[0].size == "small");
    assert(ports[3].size == "capital");
}

void test_station_editor_docking_capacity() {
    StationEditorPanel panel;

    panel.SetMaxDockingCapacity(50);
    assert(panel.MaxDockingCapacity() == 50);

    // Minimum 1
    panel.SetMaxDockingCapacity(0);
    assert(panel.MaxDockingCapacity() >= 1);
}

void test_station_editor_undock_clearance() {
    StationEditorPanel panel;

    panel.SetUndockClearanceRadius(1000.0f);
    assert(panel.UndockClearanceRadius() > 999.0f);

    // Minimum 50
    panel.SetUndockClearanceRadius(10.0f);
    assert(panel.UndockClearanceRadius() >= 50.0f);
}

void test_station_editor_add_service() {
    StationEditorPanel panel;

    assert(panel.AddService("repair", 1.0f));
    assert(panel.AddService("market", 1.5f));
    assert(panel.ServiceCount() == 2);
    assert(panel.EnabledServiceCount() == 2);

    // Duplicate type rejected
    assert(!panel.AddService("repair", 2.0f));

    // Invalid type rejected
    assert(!panel.AddService("teleporter", 1.0f));
}

void test_station_editor_remove_service() {
    StationEditorPanel panel;

    panel.AddService("repair", 1.0f);
    panel.AddService("market", 1.5f);

    // Get service IDs from the services list
    const auto& services = panel.Services();
    std::string repairId = services[0].serviceId;
    std::string marketId = services[1].serviceId;

    assert(panel.RemoveService(repairId));
    assert(panel.ServiceCount() == 1);

    assert(!panel.RemoveService(repairId));  // can't remove twice
    assert(!panel.RemoveService("nonexistent"));
}

void test_station_editor_toggle_service() {
    StationEditorPanel panel;

    panel.AddService("repair", 1.0f);
    const auto& services = panel.Services();
    std::string repairId = services[0].serviceId;

    // Initially enabled
    assert(panel.EnabledServiceCount() == 1);

    // Disable
    assert(panel.ToggleService(repairId, false));
    assert(panel.EnabledServiceCount() == 0);

    // Can't disable again
    assert(!panel.ToggleService(repairId, false));

    // Re-enable
    assert(panel.ToggleService(repairId, true));
    assert(panel.EnabledServiceCount() == 1);

    // Nonexistent service
    assert(!panel.ToggleService("nonexistent", true));
}

void test_station_editor_service_cost() {
    StationEditorPanel panel;

    panel.AddService("repair", 1.0f);
    const auto& services = panel.Services();
    std::string repairId = services[0].serviceId;

    assert(panel.SetServiceCostMultiplier(repairId, 2.5f));
    assert(services[0].costMultiplier > 2.4f && services[0].costMultiplier < 2.6f);

    // Clamped to 0.1–10.0
    panel.SetServiceCostMultiplier(repairId, 0.01f);
    assert(services[0].costMultiplier >= 0.1f);
    panel.SetServiceCostMultiplier(repairId, 100.0f);
    assert(services[0].costMultiplier <= 10.0f);

    // Nonexistent
    assert(!panel.SetServiceCostMultiplier("nonexistent", 1.0f));
}

void test_station_editor_service_types() {
    StationEditorPanel panel;

    // All valid service types
    assert(panel.AddService("repair", 1.0f));
    assert(panel.AddService("market", 1.0f));
    assert(panel.AddService("manufacturing", 1.0f));
    assert(panel.AddService("refining", 1.0f));
    assert(panel.AddService("clone_bay", 1.0f));
    assert(panel.AddService("insurance", 1.0f));
    assert(panel.AddService("fitting", 1.0f));
    assert(panel.AddService("bounty_office", 1.0f));
    assert(panel.ServiceCount() == 8);
}

void test_station_editor_max_segments() {
    StationEditorPanel panel;

    for (int i = 0; i < 20; ++i) {
        assert(panel.AddSegment("arm", 100.0f, 30.0f) > 0);
    }
    assert(panel.SegmentCount() == 20);
    assert(panel.AddSegment("arm", 100.0f, 30.0f) == -1);  // 21st rejected
}

void test_station_editor_max_docking_ports() {
    StationEditorPanel panel;

    for (int i = 0; i < 12; ++i) {
        assert(panel.AddDockingPort("Port " + std::to_string(i), "small") > 0);
    }
    assert(panel.DockingPortCount() == 12);
    assert(panel.AddDockingPort("Port 12", "small") == -1);  // 13th rejected
}

void test_station_editor_max_services() {
    StationEditorPanel panel;

    assert(panel.AddService("repair", 1.0f));
    assert(panel.AddService("market", 1.0f));
    assert(panel.AddService("manufacturing", 1.0f));
    assert(panel.AddService("refining", 1.0f));
    assert(panel.AddService("clone_bay", 1.0f));
    assert(panel.AddService("insurance", 1.0f));
    assert(panel.AddService("fitting", 1.0f));
    assert(panel.AddService("bounty_office", 1.0f));
    // 8 unique types is the max (there are only 8 valid types)
    assert(panel.ServiceCount() == 8);
}

void test_station_editor_export_json() {
    StationEditorPanel panel;

    panel.SetStationName("Amarr Trade Hub");
    panel.SetFaction("amarr");
    panel.SetSecurityLevel(1.0f);
    panel.AddSegment("hub", 100.0f, 100.0f);
    panel.AddSegment("arm", 200.0f, 30.0f);
    panel.AddDockingPort("Bay Alpha", "large");
    panel.AddService("repair", 1.0f);
    panel.AddService("market", 0.8f);

    std::string json = panel.ExportJSON();
    assert(!json.empty());
    assert(json.find("Amarr Trade Hub") != std::string::npos);
    assert(json.find("amarr") != std::string::npos);
    assert(json.find("hub") != std::string::npos);
    assert(json.find("arm") != std::string::npos);
    assert(json.find("Bay Alpha") != std::string::npos);
    assert(json.find("repair") != std::string::npos);
    assert(json.find("market") != std::string::npos);
}

void test_station_editor_headless_draw() {
    StationEditorPanel panel;
    // Draw without context should not crash
    panel.Draw();
    assert(panel.SegmentCount() == 0);  // no side effects
}
