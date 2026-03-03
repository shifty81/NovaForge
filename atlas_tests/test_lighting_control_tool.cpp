/**
 * Tests for LightingControlTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - SetProperty posts command, records DeltaEdit with light: prefix
 *   - SetProperty undo restores old value
 *   - ApplyPreset posts command, records multiple DeltaEdits
 *   - ApplyPreset undo restores old properties
 *   - Multiple property changes on same entity
 *   - Multiple entities with different lighting
 */

#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include "../cpp_client/include/editor/lighting_control_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// LightingControlTool tests
// ══════════════════════════════════════════════════════════════════

void test_light_tool_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    LightingControlTool tool(bus, store);
    assert(std::string(tool.Name()) == "Lighting Control");
}

void test_light_tool_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    LightingControlTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_light_tool_set_property() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    LightingControlTool tool(bus, store);

    tool.Activate();
    tool.SetProperty(1, "intensity", "0.5", "1.0");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "light:intensity");
    assert(edit.propertyValue == "1.0");
}

void test_light_tool_set_property_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    LightingControlTool tool(bus, store);

    tool.SetProperty(1, "color", "1,1,1", "1,0,0");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "1,0,0");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyName == "light:color");
    assert(store.Edits()[1].propertyValue == "1,1,1");
}

void test_light_tool_apply_preset() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    LightingControlTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"intensity", "0.5"}, {"color", "1,1,1"}, {"range", "10"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"intensity", "0.8"}, {"color", "1,0.2,0.2"}, {"range", "15"}
    };

    tool.ApplyPreset(1, "Combat Red Alert", oldProps, newProps);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].propertyName == "light:intensity");
    assert(store.Edits()[0].propertyValue == "0.8");
    assert(store.Edits()[1].propertyName == "light:color");
    assert(store.Edits()[1].propertyValue == "1,0.2,0.2");
    assert(store.Edits()[2].propertyName == "light:range");
    assert(store.Edits()[2].propertyValue == "15");
}

void test_light_tool_apply_preset_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    LightingControlTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"intensity", "0.5"}, {"color", "1,1,1"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"intensity", "0.3"}, {"color", "0.8,0.6,0.2"}
    };

    tool.ApplyPreset(1, "Docking Warm", oldProps, newProps);
    bus.ProcessCommands();
    assert(store.Count() == 2);

    bus.Undo();
    assert(store.Count() == 4);
    assert(store.Edits()[2].propertyName == "light:intensity");
    assert(store.Edits()[2].propertyValue == "0.5");
    assert(store.Edits()[3].propertyName == "light:color");
    assert(store.Edits()[3].propertyValue == "1,1,1");
}

void test_light_tool_multiple_properties() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    LightingControlTool tool(bus, store);

    tool.SetProperty(1, "intensity", "0.5", "1.0");
    tool.SetProperty(1, "color", "1,1,1", "0,1,0");
    tool.SetProperty(1, "range", "10", "20");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].propertyName == "light:intensity");
    assert(store.Edits()[1].propertyName == "light:color");
    assert(store.Edits()[2].propertyName == "light:range");
}

void test_light_tool_multiple_entities() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    LightingControlTool tool(bus, store);

    tool.SetProperty(1, "intensity", "0.5", "1.0");
    tool.SetProperty(2, "intensity", "0.3", "0.8");
    tool.SetProperty(3, "color", "1,1,1", "0,0,1");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[2].entityID == 3);
}
