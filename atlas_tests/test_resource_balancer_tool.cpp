/**
 * Tests for ResourceBalancerTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - SetProperty posts command, records DeltaEdit
 *   - ApplyPreset posts command, records multiple DeltaEdits
 *   - Undo reverses both single properties and presets
 */

#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include "../cpp_client/include/editor/resource_balancer_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// ResourceBalancerTool tests
// ══════════════════════════════════════════════════════════════════

void test_res_balancer_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ResourceBalancerTool tool(bus, store);
    assert(std::string(tool.Name()) == "Resource Balancer");
}

void test_res_balancer_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ResourceBalancerTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_res_balancer_set_property() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    ResourceBalancerTool tool(bus, store);

    tool.Activate();
    tool.SetProperty(1, "base_price", "100.0", "150.0");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "base_price");
    assert(edit.propertyValue == "150.0");
}

void test_res_balancer_set_property_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    ResourceBalancerTool tool(bus, store);

    tool.SetProperty(1, "base_price", "100.0", "150.0");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "150.0");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyValue == "100.0");
}

void test_res_balancer_apply_preset() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    ResourceBalancerTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"base_price", "100.0"},
        {"spawn_rate", "1.0"},
        {"refine_yield", "0.8"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"base_price", "50.0"},
        {"spawn_rate", "2.0"},
        {"refine_yield", "0.9"}
    };

    tool.ApplyPreset(1, "Abundant", oldProps, newProps);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);
    // Three properties recorded
    assert(store.Count() == 3);

    assert(store.Edits()[0].propertyName == "base_price");
    assert(store.Edits()[0].propertyValue == "50.0");
    assert(store.Edits()[1].propertyName == "spawn_rate");
    assert(store.Edits()[1].propertyValue == "2.0");
    assert(store.Edits()[2].propertyName == "refine_yield");
    assert(store.Edits()[2].propertyValue == "0.9");
}

void test_res_balancer_apply_preset_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    ResourceBalancerTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"base_price", "100.0"},
        {"spawn_rate", "1.0"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"base_price", "50.0"},
        {"spawn_rate", "2.0"}
    };

    tool.ApplyPreset(1, "Abundant", oldProps, newProps);
    bus.ProcessCommands();
    assert(store.Count() == 2);

    bus.Undo();
    assert(store.Count() == 4);
    assert(store.Edits()[2].propertyName == "base_price");
    assert(store.Edits()[2].propertyValue == "100.0");
    assert(store.Edits()[3].propertyName == "spawn_rate");
    assert(store.Edits()[3].propertyValue == "1.0");
}

void test_res_balancer_multiple_properties() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    ResourceBalancerTool tool(bus, store);

    tool.SetProperty(1, "base_price", "100.0", "200.0");
    tool.SetProperty(1, "spawn_rate", "1.0", "0.5");
    assert(bus.PendingCount() == 2);

    bus.ProcessCommands();
    assert(store.Count() == 2);
    assert(store.Edits()[0].propertyName == "base_price");
    assert(store.Edits()[1].propertyName == "spawn_rate");
}
