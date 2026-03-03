/**
 * Tests for MaterialShaderTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - SetProperty posts command, records DeltaEdit with mat: prefix
 *   - SetProperty undo restores old value
 *   - ApplyPreset posts command, records multiple DeltaEdits
 *   - ApplyPreset undo restores old properties
 *   - Multiple property changes on same entity
 *   - Multiple entities with different materials
 */

#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include "../cpp_client/include/editor/material_shader_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// MaterialShaderTool tests
// ══════════════════════════════════════════════════════════════════

void test_mat_tool_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    MaterialShaderTool tool(bus, store);
    assert(std::string(tool.Name()) == "Material Shader");
}

void test_mat_tool_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    MaterialShaderTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_mat_tool_set_property() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    MaterialShaderTool tool(bus, store);

    tool.Activate();
    tool.SetProperty(1, "roughness", "0.5", "0.8");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "mat:roughness");
    assert(edit.propertyValue == "0.8");
}

void test_mat_tool_set_property_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    MaterialShaderTool tool(bus, store);

    tool.SetProperty(1, "color", "1,1,1,1", "0.2,0.5,0.8,1");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "0.2,0.5,0.8,1");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyName == "mat:color");
    assert(store.Edits()[1].propertyValue == "1,1,1,1");
}

void test_mat_tool_apply_preset() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    MaterialShaderTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"roughness", "0.5"}, {"metallic", "0.0"}, {"emission", "0,0,0"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"roughness", "0.2"}, {"metallic", "0.9"}, {"emission", "0.1,0.3,0.8"}
    };

    tool.ApplyPreset(1, "Shield Emissive", oldProps, newProps);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].propertyName == "mat:roughness");
    assert(store.Edits()[0].propertyValue == "0.2");
    assert(store.Edits()[1].propertyName == "mat:metallic");
    assert(store.Edits()[1].propertyValue == "0.9");
    assert(store.Edits()[2].propertyName == "mat:emission");
    assert(store.Edits()[2].propertyValue == "0.1,0.3,0.8");
}

void test_mat_tool_apply_preset_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    MaterialShaderTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"roughness", "0.5"}, {"metallic", "0.0"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"roughness", "0.1"}, {"metallic", "1.0"}
    };

    tool.ApplyPreset(1, "Hull Plating", oldProps, newProps);
    bus.ProcessCommands();
    assert(store.Count() == 2);

    bus.Undo();
    assert(store.Count() == 4);
    assert(store.Edits()[2].propertyName == "mat:roughness");
    assert(store.Edits()[2].propertyValue == "0.5");
    assert(store.Edits()[3].propertyName == "mat:metallic");
    assert(store.Edits()[3].propertyValue == "0.0");
}

void test_mat_tool_multiple_properties() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    MaterialShaderTool tool(bus, store);

    tool.SetProperty(1, "roughness", "0.5", "0.2");
    tool.SetProperty(1, "metallic", "0.0", "1.0");
    tool.SetProperty(1, "opacity", "1.0", "0.5");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].propertyName == "mat:roughness");
    assert(store.Edits()[1].propertyName == "mat:metallic");
    assert(store.Edits()[2].propertyName == "mat:opacity");
}

void test_mat_tool_multiple_entities() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    MaterialShaderTool tool(bus, store);

    tool.SetProperty(1, "roughness", "0.5", "0.2");
    tool.SetProperty(2, "metallic", "0.0", "1.0");
    tool.SetProperty(3, "color", "1,1,1,1", "0,0,1,1");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[2].entityID == 3);
}
