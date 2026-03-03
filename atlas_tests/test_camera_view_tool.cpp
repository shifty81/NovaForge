/**
 * Tests for CameraViewTool:
 *   - ITool interface compliance (name, activate, deactivate)
 *   - SetViewMode posts command, records DeltaEdit with camera: prefix
 *   - SetViewMode undo restores old mode
 *   - SetProperty posts command, records DeltaEdit with camera: prefix
 *   - SetProperty undo restores old value
 *   - SaveBookmark posts command, records multiple DeltaEdits
 *   - SaveBookmark undo restores old properties
 *   - Multiple property changes on same entity
 *   - Multiple entities with different camera settings
 */

#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include "../cpp_client/include/editor/camera_view_tool.h"

using namespace atlas::editor;
using namespace atlas::ecs;

// ══════════════════════════════════════════════════════════════════
// CameraViewTool tests
// ══════════════════════════════════════════════════════════════════

void test_camera_tool_name() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    CameraViewTool tool(bus, store);
    assert(std::string(tool.Name()) == "Camera View");
}

void test_camera_tool_activate_deactivate() {
    UndoableCommandBus bus;
    DeltaEditStore store;
    CameraViewTool tool(bus, store);

    assert(!tool.IsActive());
    tool.Activate();
    assert(tool.IsActive());
    tool.Deactivate();
    assert(!tool.IsActive());
}

void test_camera_tool_set_view_mode() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    CameraViewTool tool(bus, store);

    tool.Activate();
    tool.SetViewMode(1, CameraViewMode::FreeFly, CameraViewMode::Orbit);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "camera:view_mode");
    assert(edit.propertyValue == "Orbit");
}

void test_camera_tool_set_view_mode_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    CameraViewTool tool(bus, store);

    tool.SetViewMode(1, CameraViewMode::FreeFly, CameraViewMode::Orthographic);
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "Orthographic");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyName == "camera:view_mode");
    assert(store.Edits()[1].propertyValue == "FreeFly");
}

void test_camera_tool_set_property() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    CameraViewTool tool(bus, store);

    tool.SetProperty(1, "fov", "60", "90");
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 1);

    const auto& edit = store.Edits()[0];
    assert(edit.type == DeltaEditType::SetProperty);
    assert(edit.entityID == 1);
    assert(edit.propertyName == "camera:fov");
    assert(edit.propertyValue == "90");
}

void test_camera_tool_set_property_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    CameraViewTool tool(bus, store);

    tool.SetProperty(1, "position", "0,0,0", "10,20,30");
    bus.ProcessCommands();
    assert(store.Count() == 1);
    assert(store.Edits()[0].propertyValue == "10,20,30");

    bus.Undo();
    assert(store.Count() == 2);
    assert(store.Edits()[1].propertyName == "camera:position");
    assert(store.Edits()[1].propertyValue == "0,0,0");
}

void test_camera_tool_save_bookmark() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    CameraViewTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"position", "0,0,0"}, {"target", "0,0,1"}, {"mode", "FreeFly"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"position", "100,50,200"}, {"target", "0,0,0"}, {"mode", "Orbit"}
    };

    tool.SaveBookmark(1, "StationView", oldProps, newProps);
    assert(bus.PendingCount() == 1);

    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].propertyName == "camera:bookmark:StationView:position");
    assert(store.Edits()[0].propertyValue == "100,50,200");
    assert(store.Edits()[1].propertyName == "camera:bookmark:StationView:target");
    assert(store.Edits()[1].propertyValue == "0,0,0");
    assert(store.Edits()[2].propertyName == "camera:bookmark:StationView:mode");
    assert(store.Edits()[2].propertyValue == "Orbit");
}

void test_camera_tool_save_bookmark_undo() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    CameraViewTool tool(bus, store);

    std::vector<std::pair<std::string, std::string>> oldProps = {
        {"position", "0,0,0"}, {"target", "0,0,1"}
    };
    std::vector<std::pair<std::string, std::string>> newProps = {
        {"position", "50,25,100"}, {"target", "10,10,10"}
    };

    tool.SaveBookmark(1, "HangarCam", oldProps, newProps);
    bus.ProcessCommands();
    assert(store.Count() == 2);

    bus.Undo();
    assert(store.Count() == 4);
    assert(store.Edits()[2].propertyName == "camera:bookmark:HangarCam:position");
    assert(store.Edits()[2].propertyValue == "0,0,0");
    assert(store.Edits()[3].propertyName == "camera:bookmark:HangarCam:target");
    assert(store.Edits()[3].propertyValue == "0,0,1");
}

void test_camera_tool_multiple_properties() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    CameraViewTool tool(bus, store);

    tool.SetProperty(1, "fov", "60", "90");
    tool.SetProperty(1, "near_plane", "0.1", "0.5");
    tool.SetProperty(1, "far_plane", "1000", "5000");
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].propertyName == "camera:fov");
    assert(store.Edits()[1].propertyName == "camera:near_plane");
    assert(store.Edits()[2].propertyName == "camera:far_plane");
}

void test_camera_tool_multiple_entities() {
    UndoableCommandBus bus;
    DeltaEditStore store(42);
    CameraViewTool tool(bus, store);

    tool.SetProperty(1, "fov", "60", "90");
    tool.SetProperty(2, "fov", "45", "75");
    tool.SetViewMode(3, CameraViewMode::FreeFly, CameraViewMode::Orthographic);
    bus.ProcessCommands();
    assert(store.Count() == 3);

    assert(store.Edits()[0].entityID == 1);
    assert(store.Edits()[1].entityID == 2);
    assert(store.Edits()[2].entityID == 3);
}

void test_camera_view_mode_names() {
    assert(std::string(CameraViewModeName(CameraViewMode::FreeFly)) == "FreeFly");
    assert(std::string(CameraViewModeName(CameraViewMode::Orbit)) == "Orbit");
    assert(std::string(CameraViewModeName(CameraViewMode::Orthographic)) == "Orthographic");
}
