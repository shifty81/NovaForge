/**
 * Tests for EnvironmentControlTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - SetProperty posts command, records DeltaEdit
 *   - ApplyPreset posts command, records multiple DeltaEdits
 *   - Undo reverses both single properties and presets
 */

#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include "../cpp_client/include/editor/environment_control_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// EnvironmentControlTool tests
// ══════════════════════════════════════════════════════════════════

void test_env_tool_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    EnvironmentControlTool tool(bus, store);
    assert(std::string(tool.Name()) == "Environment Control");
}

void test_env_tool_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    EnvironmentControlTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_env_tool_set_property() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EnvironmentControlTool tool(bus, store);

    tool.Activate();
    tool.SetProperty(1, "gravity", "9.81", "0.0");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "gravity");
    assert(edit.propertyValue == "0.0");
}

void test_env_tool_set_property_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EnvironmentControlTool tool(bus, store);

    tool.SetProperty(1, "gravity", "9.81", "0.0");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "0.0");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyValue == "9.81");
}

void test_env_tool_apply_preset() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EnvironmentControlTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"gravity", "9.81"},
        {"wind_strength", "0.0"},
        {"atmosphere_density", "1.0"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"gravity", "0.0"},
        {"wind_strength", "0.0"},
        {"atmosphere_density", "0.0"}
    };

    tool.ApplyPreset(1, "Zero-G", oldProps, newProps);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(bus.PendingCount() == 0);
    // Three properties recorded
    assert(store.Count() == 3);

    assert(store.Edits()[0].propertyName == "gravity");
    assert(store.Edits()[0].propertyValue == "0.0");
    assert(store.Edits()[1].propertyName == "wind_strength");
    assert(store.Edits()[2].propertyName == "atmosphere_density");
}

void test_env_tool_apply_preset_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    EnvironmentControlTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"gravity", "9.81"},
        {"wind_strength", "0.0"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"gravity", "0.0"},
        {"wind_strength", "0.0"}
    };

    tool.ApplyPreset(1, "Zero-G", oldProps, newProps);
    bus.ProcessCommands();
    assert(store.Count() == 2); // 2 properties from new preset

    bus.Undo();
    assert(store.Count() == 4); // 2 more from old properties restored
    assert(store.Edits()[2].propertyName == "gravity");
    assert(store.Edits()[2].propertyValue == "9.81");
    assert(store.Edits()[3].propertyName == "wind_strength");
    assert(store.Edits()[3].propertyValue == "0.0");
}

void test_env_tool_multiple_properties() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    EnvironmentControlTool tool(bus, store);

    tool.SetProperty(1, "gravity", "9.81", "1.62");
    tool.SetProperty(1, "atmosphere_density", "1.0", "0.3");
    assert(bus.PendingCount() == 2);

    bus.ProcessCommands();
    assert(store.Count() == 2);
    assert(store.Edits()[0].propertyName == "gravity");
    assert(store.Edits()[1].propertyName == "atmosphere_density");
}
