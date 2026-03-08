// Tests for: EditorOverlaySystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/editor_overlay_system.h"

using namespace atlas;
using LM = components::EditorOverlayState::LayoutMode;

// ==================== EditorOverlaySystem Tests ====================

static void testOverlayDefaultState() {
    std::cout << "\n=== EditorOverlay: DefaultState ===" << std::endl;
    ecs::World world;
    systems::EditorOverlaySystem sys(&world);
    auto* e = world.createEntity("editor1");
    auto* state = addComp<components::EditorOverlayState>(e);

    assertTrue(state->layout_mode == LM::Hidden, "Default layout Hidden");
    assertTrue(approxEqual(state->overlay_opacity, 0.85f), "Default opacity 0.85");
    assertTrue(!state->captures_input, "Default no input capture");
    assertTrue(!state->show_hierarchy, "Default hierarchy hidden");
    assertTrue(!state->show_inspector, "Default inspector hidden");
    assertTrue(state->show_tools, "Default tools visible");
    assertTrue(!state->show_console, "Default console hidden");
    assertTrue(!state->show_profiler, "Default profiler hidden");
    assertTrue(approxEqual(state->hierarchy_width_pct, 0.20f), "Default hierarchy 20%");
    assertTrue(approxEqual(state->inspector_width_pct, 0.25f), "Default inspector 25%");
    assertTrue(state->toggle_count == 0, "Default toggle count 0");
    assertTrue(state->active, "Default active");
}

static void testOverlayCycleLayout() {
    std::cout << "\n=== EditorOverlay: CycleLayout ===" << std::endl;
    ecs::World world;
    systems::EditorOverlaySystem sys(&world);
    auto* e = world.createEntity("e1");
    auto* state = addComp<components::EditorOverlayState>(e);

    assertTrue(state->layout_mode == LM::Hidden, "Start Hidden");
    assertTrue(sys.cycleLayout("e1"), "Cycle 1 OK");
    assertTrue(state->layout_mode == LM::Minimal, "Now Minimal");
    assertTrue(sys.cycleLayout("e1"), "Cycle 2 OK");
    assertTrue(state->layout_mode == LM::Full, "Now Full");
    assertTrue(sys.cycleLayout("e1"), "Cycle 3 OK");
    assertTrue(state->layout_mode == LM::Hidden, "Back to Hidden");
    assertTrue(state->toggle_count == 3, "3 toggles");
}

static void testOverlaySetLayoutMode() {
    std::cout << "\n=== EditorOverlay: SetLayoutMode ===" << std::endl;
    ecs::World world;
    systems::EditorOverlaySystem sys(&world);
    auto* e = world.createEntity("e1");
    addComp<components::EditorOverlayState>(e);

    assertTrue(sys.setLayoutMode("e1", LM::Full), "Set Full OK");
    assertTrue(sys.getLayoutMode("e1") == LM::Full, "Is Full");
    assertTrue(sys.setLayoutMode("e1", LM::Minimal), "Set Minimal OK");
    assertTrue(sys.getLayoutMode("e1") == LM::Minimal, "Is Minimal");
    assertTrue(sys.setLayoutMode("e1", LM::Hidden), "Set Hidden OK");
    assertTrue(sys.getLayoutMode("e1") == LM::Hidden, "Is Hidden");
}

static void testOverlayToggleInputCapture() {
    std::cout << "\n=== EditorOverlay: ToggleInputCapture ===" << std::endl;
    ecs::World world;
    systems::EditorOverlaySystem sys(&world);
    auto* e = world.createEntity("e1");
    auto* state = addComp<components::EditorOverlayState>(e);

    assertTrue(!state->captures_input, "Starts false");
    assertTrue(sys.toggleInputCapture("e1"), "Toggle OK");
    assertTrue(state->captures_input, "Now true");
    assertTrue(sys.toggleInputCapture("e1"), "Toggle again OK");
    assertTrue(!state->captures_input, "Back to false");
}

static void testOverlayTogglePanels() {
    std::cout << "\n=== EditorOverlay: TogglePanels ===" << std::endl;
    ecs::World world;
    systems::EditorOverlaySystem sys(&world);
    auto* e = world.createEntity("e1");
    auto* state = addComp<components::EditorOverlayState>(e);

    assertTrue(sys.togglePanel("e1", "hierarchy"), "Toggle hierarchy OK");
    assertTrue(state->show_hierarchy, "Hierarchy now visible");
    assertTrue(sys.isPanelVisible("e1", "hierarchy"), "isPanelVisible hierarchy");

    assertTrue(sys.togglePanel("e1", "inspector"), "Toggle inspector OK");
    assertTrue(state->show_inspector, "Inspector now visible");

    assertTrue(sys.togglePanel("e1", "console"), "Toggle console OK");
    assertTrue(state->show_console, "Console now visible");

    assertTrue(sys.togglePanel("e1", "profiler"), "Toggle profiler OK");
    assertTrue(state->show_profiler, "Profiler now visible");

    // Tools starts true, toggle to false
    assertTrue(sys.togglePanel("e1", "tools"), "Toggle tools OK");
    assertTrue(!state->show_tools, "Tools now hidden");
    assertTrue(!sys.isPanelVisible("e1", "tools"), "isPanelVisible tools false");
}

static void testOverlaySetOpacity() {
    std::cout << "\n=== EditorOverlay: SetOpacity ===" << std::endl;
    ecs::World world;
    systems::EditorOverlaySystem sys(&world);
    auto* e = world.createEntity("e1");
    addComp<components::EditorOverlayState>(e);

    assertTrue(sys.setOpacity("e1", 0.5f), "Set 0.5 OK");
    assertTrue(approxEqual(sys.getOpacity("e1"), 0.5f), "Opacity 0.5");

    // Clamp low
    assertTrue(sys.setOpacity("e1", 0.0f), "Set 0.0 OK");
    assertTrue(approxEqual(sys.getOpacity("e1"), 0.1f), "Clamped to 0.1");

    // Clamp high
    assertTrue(sys.setOpacity("e1", 2.0f), "Set 2.0 OK");
    assertTrue(approxEqual(sys.getOpacity("e1"), 1.0f), "Clamped to 1.0");
}

static void testOverlayInvalidPanel() {
    std::cout << "\n=== EditorOverlay: InvalidPanel ===" << std::endl;
    ecs::World world;
    systems::EditorOverlaySystem sys(&world);
    auto* e = world.createEntity("e1");
    addComp<components::EditorOverlayState>(e);

    assertTrue(!sys.togglePanel("e1", "nonexistent"), "Invalid panel returns false");
    assertTrue(!sys.isPanelVisible("e1", "nonexistent"), "Invalid panel not visible");
}

static void testOverlayMissingEntity() {
    std::cout << "\n=== EditorOverlay: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::EditorOverlaySystem sys(&world);

    assertTrue(!sys.cycleLayout("x"), "Cycle on missing");
    assertTrue(!sys.setLayoutMode("x", LM::Full), "SetLayout on missing");
    assertTrue(sys.getLayoutMode("x") == LM::Hidden, "GetLayout on missing");
    assertTrue(!sys.toggleInputCapture("x"), "ToggleInput on missing");
    assertTrue(!sys.togglePanel("x", "hierarchy"), "TogglePanel on missing");
    assertTrue(!sys.isPanelVisible("x", "hierarchy"), "isPanelVisible on missing");
    assertTrue(approxEqual(sys.getOpacity("x"), 0.0f), "Opacity on missing");
    assertTrue(!sys.setOpacity("x", 0.5f), "SetOpacity on missing");
}

void run_editor_overlay_system_tests() {
    testOverlayDefaultState();
    testOverlayCycleLayout();
    testOverlaySetLayoutMode();
    testOverlayToggleInputCapture();
    testOverlayTogglePanels();
    testOverlaySetOpacity();
    testOverlayInvalidPanel();
    testOverlayMissingEntity();
}
