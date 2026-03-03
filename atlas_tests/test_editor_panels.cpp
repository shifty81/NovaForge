/**
 * Tests for editor panel features: ECS Inspector, Network Inspector,
 * Game Packager, AI Aggregator, and Editor Layout / Dock tabs.
 */

#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include "../editor/panels/ECSInspectorPanel.h"
#include "../editor/panels/NetInspectorPanel.h"
#include "../editor/tools/GamePackagerPanel.h"
#include "../editor/tools/ViewportPanel.h"
#include "../editor/ai/AIAggregator.h"
#include "../editor/ui/EditorLayout.h"
#include "../editor/ui/KeybindManager.h"
#include "../editor/ui/UndoStack.h"
#include "../engine/ecs/ECS.h"
#include "../engine/net/NetContext.h"
#include "../cpp_server/include/pcg/ship_generator.h"

using namespace atlas::editor;
using namespace atlas::ecs;
using namespace atlas::net;
using namespace atlas::ai;

// ── Mock AI backend ───────────────────────────────────────────────

class MockBackend : public atlas::ai::AIBackend {
public:
    std::string response;
    float confidence;
    MockBackend(const std::string& r, float c) : response(r), confidence(c) {}
    atlas::ai::AIResponse Query(const std::string&, const atlas::ai::AIContext&) override {
        return {response, confidence};
    }
};

// ── Helper: trackable panel for dock tests ────────────────────────

class TrackablePanel : public EditorPanel {
public:
    explicit TrackablePanel(const char* name) : m_name(name) {}
    const char* Name() const override { return m_name; }
    void Draw() override { ++drawCount; }
    int drawCount = 0;
private:
    const char* m_name;
};

// ══════════════════════════════════════════════════════════════════
// ECS Inspector tests
// ══════════════════════════════════════════════════════════════════

void test_ecsi_defaults() {
    World w;
    ECSInspectorPanel panel(w);
    assert(panel.SelectedEntity() == 0);
    assert(panel.SearchFilter().empty());
    assert(panel.EntityCountVisible() == 0);
}

void test_ecsi_select_entity() {
    World w;
    ECSInspectorPanel panel(w);
    EntityID e = w.CreateEntity();
    panel.SelectEntity(e);
    assert(panel.SelectedEntity() == e);
}

void test_ecsi_select_dead_entity_ignored() {
    World w;
    ECSInspectorPanel panel(w);
    panel.SelectEntity(999);
    assert(panel.SelectedEntity() == 0);
}

void test_ecsi_destroy_selected() {
    World w;
    ECSInspectorPanel panel(w);
    EntityID e = w.CreateEntity();
    panel.SelectEntity(e);
    assert(panel.SelectedEntity() == e);
    panel.DestroySelectedEntity();
    assert(panel.SelectedEntity() == 0);
    assert(w.EntityCount() == 0);
}

void test_ecsi_clear_selection() {
    World w;
    ECSInspectorPanel panel(w);
    EntityID e = w.CreateEntity();
    panel.SelectEntity(e);
    assert(panel.SelectedEntity() == e);
    panel.ClearSelection();
    assert(panel.SelectedEntity() == 0);
}

void test_ecsi_search_filter_by_id() {
    World w;
    ECSInspectorPanel panel(w);
    w.CreateEntity(); // 1
    w.CreateEntity(); // 2
    w.CreateEntity(); // 3
    panel.SetSearchFilter("1");
    assert(panel.EntityCountVisible() == 1);
}

void test_ecsi_search_filter_empty_shows_all() {
    World w;
    ECSInspectorPanel panel(w);
    w.CreateEntity();
    w.CreateEntity();
    w.CreateEntity();
    panel.SetSearchFilter("");
    assert(panel.EntityCountVisible() == 3);
}

void test_ecsi_draw_clears_dead_selection() {
    World w;
    ECSInspectorPanel panel(w);
    EntityID e = w.CreateEntity();
    panel.SelectEntity(e);
    w.DestroyEntity(e);
    panel.Draw();
    assert(panel.SelectedEntity() == 0);
}

void test_ecsi_name() {
    World w;
    ECSInspectorPanel panel(w);
    assert(std::string(panel.Name()) == "ECS Inspector");
}

void test_ecsi_visibility() {
    World w;
    ECSInspectorPanel panel(w);
    assert(panel.IsVisible() == true);
    panel.SetVisible(false);
    assert(panel.IsVisible() == false);
}

// ══════════════════════════════════════════════════════════════════
// Network Inspector tests
// ══════════════════════════════════════════════════════════════════

void test_neti_defaults() {
    NetContext net;
    NetInspectorPanel panel(net);
    assert(panel.SelectedPeer() == 0);
    auto stats = panel.GetStats();
    assert(stats.modeString == "Standalone");
    assert(stats.connectedPeerCount == 0);
}

void test_neti_mode_to_string() {
    assert(NetInspectorPanel::ModeToString(NetMode::Standalone) == "Standalone");
    assert(NetInspectorPanel::ModeToString(NetMode::Client)     == "Client");
    assert(NetInspectorPanel::ModeToString(NetMode::Server)     == "Server");
    assert(NetInspectorPanel::ModeToString(NetMode::P2P_Host)   == "P2P_Host");
    assert(NetInspectorPanel::ModeToString(NetMode::P2P_Peer)   == "P2P_Peer");
}

void test_neti_select_peer() {
    NetContext net;
    net.Init(NetMode::Server);
    uint32_t p1 = net.AddPeer();
    uint32_t p2 = net.AddPeer();
    NetInspectorPanel panel(net);
    panel.SelectPeer(p2);
    assert(panel.SelectedPeer() == p2);
    (void)p1;
    net.Shutdown();
}

void test_neti_select_nonexistent_peer_ignored() {
    NetContext net;
    net.Init(NetMode::Server);
    NetInspectorPanel panel(net);
    panel.SelectPeer(999);
    assert(panel.SelectedPeer() == 0);
    net.Shutdown();
}

void test_neti_clear_peer_selection() {
    NetContext net;
    net.Init(NetMode::Server);
    uint32_t p = net.AddPeer();
    NetInspectorPanel panel(net);
    panel.SelectPeer(p);
    assert(panel.SelectedPeer() == p);
    panel.ClearPeerSelection();
    assert(panel.SelectedPeer() == 0);
    net.Shutdown();
}

void test_neti_stats_with_peers() {
    NetContext net;
    net.Init(NetMode::Server);
    net.AddPeer(); // peer 1
    net.AddPeer(); // peer 2
    NetInspectorPanel panel(net);
    auto stats = panel.GetStats();
    assert(stats.connectedPeerCount == 2);
    assert(stats.modeString == "Server");
}

void test_neti_draw_clears_disconnected_peer() {
    NetContext net;
    net.Init(NetMode::Server);
    uint32_t p = net.AddPeer();
    NetInspectorPanel panel(net);
    panel.SelectPeer(p);
    assert(panel.SelectedPeer() == p);
    net.RemovePeer(p);
    panel.Draw();
    assert(panel.SelectedPeer() == 0);
    net.Shutdown();
}

void test_neti_name() {
    NetContext net;
    NetInspectorPanel panel(net);
    assert(std::string(panel.Name()) == "Network");
}

// ══════════════════════════════════════════════════════════════════
// Game Packager tests
// ══════════════════════════════════════════════════════════════════

void test_pkg_defaults() {
    GamePackagerPanel panel;
    assert(panel.Status().currentStep == PackageStep::Idle);
    assert(!panel.IsPackaging());
    assert(panel.Settings().target == BuildTarget::Client);
    assert(panel.Settings().mode == BuildMode::Debug);
}

void test_pkg_set_settings() {
    GamePackagerPanel panel;
    PackageSettings s;
    s.target = BuildTarget::Server;
    s.mode = BuildMode::Release;
    s.outputPath = "/tmp/out";
    panel.SetSettings(s);
    assert(panel.Settings().target == BuildTarget::Server);
    assert(panel.Settings().mode == BuildMode::Release);
    assert(panel.Settings().outputPath == "/tmp/out");
}

void test_pkg_start_package() {
    GamePackagerPanel panel;
    panel.StartPackage();
    assert(panel.Status().currentStep == PackageStep::Validate);
}

void test_pkg_advance_full_pipeline() {
    GamePackagerPanel panel;
    panel.StartPackage();
    panel.AdvanceStep(); // Validate -> CookAssets
    panel.AdvanceStep(); // CookAssets -> Compile
    panel.AdvanceStep(); // Compile -> Bundle
    panel.AdvanceStep(); // Bundle -> Complete
    assert(panel.Status().currentStep == PackageStep::Complete);
}

void test_pkg_cancel_package() {
    GamePackagerPanel panel;
    panel.StartPackage();
    panel.AdvanceStep();
    panel.CancelPackage();
    assert(panel.Status().currentStep == PackageStep::Idle);
}

void test_pkg_empty_output_path_fails() {
    GamePackagerPanel panel;
    PackageSettings s;
    s.outputPath = "";
    panel.SetSettings(s);
    panel.StartPackage();
    assert(panel.Status().currentStep == PackageStep::Failed);
    assert(panel.Status().hasErrors);
}

void test_pkg_is_packaging() {
    GamePackagerPanel panel;
    assert(!panel.IsPackaging()); // Idle

    panel.StartPackage();
    assert(panel.IsPackaging());  // Validate

    panel.AdvanceStep();
    assert(panel.IsPackaging());  // CookAssets

    panel.AdvanceStep();
    assert(panel.IsPackaging());  // Compile

    panel.AdvanceStep();
    assert(panel.IsPackaging());  // Bundle

    panel.AdvanceStep();
    assert(!panel.IsPackaging()); // Complete
}

void test_pkg_step_to_string() {
    assert(GamePackagerPanel::StepToString(PackageStep::Idle)       == "Idle");
    assert(GamePackagerPanel::StepToString(PackageStep::Validate)   == "Validate");
    assert(GamePackagerPanel::StepToString(PackageStep::CookAssets) == "Cook Assets");
    assert(GamePackagerPanel::StepToString(PackageStep::Compile)    == "Compile");
    assert(GamePackagerPanel::StepToString(PackageStep::Bundle)     == "Bundle");
    assert(GamePackagerPanel::StepToString(PackageStep::Complete)   == "Complete");
    assert(GamePackagerPanel::StepToString(PackageStep::Failed)     == "Failed");
}

void test_pkg_target_to_string() {
    assert(GamePackagerPanel::TargetToString(BuildTarget::Client) == "Client");
    assert(GamePackagerPanel::TargetToString(BuildTarget::Server) == "Server");
}

void test_pkg_mode_to_string() {
    assert(GamePackagerPanel::ModeToString(BuildMode::Debug)       == "Debug");
    assert(GamePackagerPanel::ModeToString(BuildMode::Development) == "Development");
    assert(GamePackagerPanel::ModeToString(BuildMode::Release)     == "Release");
}

void test_pkg_log_messages() {
    GamePackagerPanel panel;
    panel.StartPackage();
    panel.AdvanceStep();
    panel.AdvanceStep();
    panel.AdvanceStep();
    panel.AdvanceStep();
    auto& log = panel.Status().log;
    assert(log.size() >= 5);
    assert(log[0].find("Started packaging") != std::string::npos);
    assert(log[1] == "Validation passed");
    assert(log[2] == "Assets cooked");
    assert(log[3] == "Compilation finished");
    assert(log[4].find("Bundle created") != std::string::npos);
}

// ══════════════════════════════════════════════════════════════════
// AI Aggregator tests
// ══════════════════════════════════════════════════════════════════

void test_ai_no_backends() {
    AIAggregator agg;
    AIContext ctx;
    auto resp = agg.Execute(AIRequestType::CodeAssist, "hello", ctx);
    assert(resp.content.empty());
    assert(resp.confidence == 0.0f);
}

void test_ai_register_null_ignored() {
    AIAggregator agg;
    agg.RegisterBackend(nullptr);
    AIContext ctx;
    auto resp = agg.Execute(AIRequestType::CodeAssist, "hello", ctx);
    assert(resp.content.empty());
}

void test_ai_single_backend() {
    AIAggregator agg;
    MockBackend mb("test response", 0.9f);
    agg.RegisterBackend(&mb);
    AIContext ctx;
    auto resp = agg.Execute(AIRequestType::Analysis, "prompt", ctx);
    assert(resp.content == "test response");
    assert(resp.confidence == 0.9f);
}

void test_ai_best_confidence_wins() {
    AIAggregator agg;
    MockBackend low("low quality", 0.3f);
    MockBackend high("high quality", 0.95f);
    agg.RegisterBackend(&low);
    agg.RegisterBackend(&high);
    AIContext ctx;
    auto resp = agg.Execute(AIRequestType::GraphGeneration, "gen", ctx);
    assert(resp.content == "high quality");
    assert(resp.confidence == 0.95f);
}

// ══════════════════════════════════════════════════════════════════
// Editor Layout / Dock tab tests
// ══════════════════════════════════════════════════════════════════

void test_dock_tab_draw() {
    TrackablePanel a("PanelA");
    TrackablePanel b("PanelB");

    DockNode node;
    node.split = DockSplit::Tab;
    node.tabs = {&a, &b};
    node.activeTab = 0;

    EditorLayout layout;
    layout.Root() = std::move(node);
    layout.Draw();

    assert(a.drawCount == 1);
    assert(b.drawCount == 0);
}

void test_dock_tab_switch() {
    TrackablePanel a("PanelA");
    TrackablePanel b("PanelB");

    DockNode node;
    node.split = DockSplit::Tab;
    node.tabs = {&a, &b};
    node.activeTab = 1;

    EditorLayout layout;
    layout.Root() = std::move(node);
    layout.Draw();

    assert(a.drawCount == 0);
    assert(b.drawCount == 1);
}

void test_dock_register_panels() {
    TrackablePanel a("A");
    TrackablePanel b("B");
    TrackablePanel c("C");

    EditorLayout layout;
    layout.RegisterPanel(&a);
    layout.RegisterPanel(&b);
    layout.RegisterPanel(&c);

    assert(layout.Panels().size() == 3);
    assert(layout.Panels()[0] == &a);
    assert(layout.Panels()[1] == &b);
    assert(layout.Panels()[2] == &c);
}

// ══════════════════════════════════════════════════════════════════
// GamePackager settings interaction tests
// ══════════════════════════════════════════════════════════════════

void test_pkg_settings_target_toggle() {
    GamePackagerPanel panel;
    assert(panel.Settings().target == BuildTarget::Client);
    PackageSettings s = panel.Settings();
    s.target = BuildTarget::Server;
    panel.SetSettings(s);
    assert(panel.Settings().target == BuildTarget::Server);
    s.target = BuildTarget::Client;
    panel.SetSettings(s);
    assert(panel.Settings().target == BuildTarget::Client);
}

void test_pkg_settings_mode_cycle() {
    GamePackagerPanel panel;
    PackageSettings s = panel.Settings();
    s.mode = BuildMode::Debug;
    panel.SetSettings(s);
    assert(panel.Settings().mode == BuildMode::Debug);
    s.mode = BuildMode::Development;
    panel.SetSettings(s);
    assert(panel.Settings().mode == BuildMode::Development);
    s.mode = BuildMode::Release;
    panel.SetSettings(s);
    assert(panel.Settings().mode == BuildMode::Release);
}

void test_pkg_settings_options() {
    GamePackagerPanel panel;
    PackageSettings s = panel.Settings();
    assert(s.singleExe == false);
    assert(s.includeMods == false);
    assert(s.stripEditorData == true);
    s.singleExe = true;
    s.includeMods = true;
    s.stripEditorData = false;
    panel.SetSettings(s);
    assert(panel.Settings().singleExe == true);
    assert(panel.Settings().includeMods == true);
    assert(panel.Settings().stripEditorData == false);
}

// ══════════════════════════════════════════════════════════════════
// Keybind wiring integration tests
// ══════════════════════════════════════════════════════════════════

void test_keybind_viewport_gizmo_wiring() {
    KeybindManager keybinds;
    // Verify viewport keybinds exist
    assert(keybinds.FindBinding("Translate") != nullptr);
    assert(keybinds.FindBinding("Rotate") != nullptr);
    assert(keybinds.FindBinding("Scale") != nullptr);
    assert(keybinds.FindBinding("ToggleGrid") != nullptr);
    assert(keybinds.FindBinding("FocusSelected") != nullptr);

    // Test that callbacks fire via HandleKeyPress
    GizmoMode mode = GizmoMode::None;
    keybinds.RegisterCallback("Translate", [&mode]() { mode = GizmoMode::Translate; });
    keybinds.RegisterCallback("Rotate", [&mode]() { mode = GizmoMode::Rotate; });
    keybinds.RegisterCallback("Scale", [&mode]() { mode = GizmoMode::Scale; });

    keybinds.HandleKeyPress('W', KeyMod::None);
    assert(mode == GizmoMode::Translate);
    keybinds.HandleKeyPress('E', KeyMod::None);
    assert(mode == GizmoMode::Rotate);
    keybinds.HandleKeyPress('R', KeyMod::None);
    assert(mode == GizmoMode::Scale);
}

void test_keybind_panel_toggle_wiring() {
    KeybindManager keybinds;
    assert(keybinds.FindBinding("ToggleConsole") != nullptr);
    assert(keybinds.FindBinding("ToggleInspector") != nullptr);
    assert(keybinds.FindBinding("ToggleSceneGraph") != nullptr);

    bool consoleToggled = false;
    keybinds.RegisterCallback("ToggleConsole", [&consoleToggled]() { consoleToggled = true; });
    keybinds.HandleKeyPress('`', KeyMod::None);
    assert(consoleToggled);
}

void test_keybind_save_wiring() {
    KeybindManager keybinds;
    assert(keybinds.FindBinding("Save") != nullptr);

    bool saved = false;
    keybinds.RegisterCallback("Save", [&saved]() { saved = true; });
    keybinds.HandleKeyPress('S', KeyMod::Ctrl);
    assert(saved);
}

void test_keybind_delete_wiring() {
    KeybindManager keybinds;
    assert(keybinds.FindBinding("Delete") != nullptr);

    bool deleted = false;
    keybinds.RegisterCallback("Delete", [&deleted]() { deleted = true; });
    keybinds.HandleKeyPress(127, KeyMod::None);
    assert(deleted);
}

// ══════════════════════════════════════════════════════════════════
// ECS Inspector component display test
// ══════════════════════════════════════════════════════════════════

void test_ecsi_component_types_visible() {
    World w;
    ECSInspectorPanel panel(w);
    EntityID e = w.CreateEntity();
    // Add a component (just int for testing)
    w.AddComponent<int>(e, 42);
    panel.SelectEntity(e);
    assert(panel.SelectedEntity() == e);

    // Verify that the entity has components
    auto types = w.GetComponentTypes(e);
    assert(types.size() == 1);
}

// ══════════════════════════════════════════════════════════════════
// Undo integration test
// ══════════════════════════════════════════════════════════════════

void test_undo_viewport_transform() {
    // Test that UndoStack can revert viewport transforms
    atlas::editor::UndoStack undoStack;
    atlas::editor::ViewportPanel viewport;

    atlas::pcg::GeneratedShip ship;
    ship.shipName = "TestShip";
    ship.mass = 1000.0f;
    ship.turretSlots = 0;
    viewport.LoadShip(ship, 42);
    assert(viewport.ObjectCount() == 1);

    uint32_t objId = viewport.GetObject(0).id;
    viewport.SelectObject(objId);

    auto oldTransform = viewport.GetTransform(objId);
    float oldX = oldTransform.posX;

    // Record undo action for translate
    float dx = 10.0f, dy = 0.0f, dz = 0.0f;
    viewport.TranslateSelected(dx, dy, dz);
    float newX = viewport.GetTransform(objId).posX;
    assert(newX != oldX);

    undoStack.PushAction({
        "Translate object",
        [&viewport, objId, dx, dy, dz]() {
            viewport.SelectObject(objId);
            viewport.TranslateSelected(-dx, -dy, -dz);
        },
        [&viewport, objId, dx, dy, dz]() {
            viewport.SelectObject(objId);
            viewport.TranslateSelected(dx, dy, dz);
        }
    });

    // Undo should revert
    undoStack.Undo();
    // The position should be close to original + 2 changes (translate + undo translate)
    // Actually undo calls TranslateSelected(-dx,...) which adds -dx, so posX should be ~oldX
    float afterUndo = viewport.GetTransform(objId).posX;
    assert(std::abs(afterUndo - oldX) < 0.01f);

    // Redo should re-apply
    undoStack.Redo();
    float afterRedo = viewport.GetTransform(objId).posX;
    assert(std::abs(afterRedo - newX) < 0.01f);
}

// ══════════════════════════════════════════════════════════════════
// Dock layout bounds computation tests
// ══════════════════════════════════════════════════════════════════

void test_dock_layout_single_panel_bounds() {
    TrackablePanel a("A");

    EditorLayout layout;
    layout.RegisterPanel(&a);
    layout.Root().panel = &a;

    // Draw triggers DrawNode which should assign dock bounds
    layout.Draw();

    assert(a.HasDockBounds());
    // Default window is 1600×900; menu bar is 22px high
    // So dock area is {0, 22, 1600, 878}
    const auto& b = a.DockBounds();
    assert(b.x == 0.0f);
    assert(b.y == 22.0f);
    assert(b.w == 1600.0f);
    assert(b.h == 878.0f);
}

void test_dock_layout_horizontal_split_bounds() {
    TrackablePanel left("Left");
    TrackablePanel right("Right");

    EditorLayout layout;
    layout.RegisterPanel(&left);
    layout.RegisterPanel(&right);

    auto& root = layout.Root();
    root.split = DockSplit::Horizontal;
    root.splitRatio = 0.60f;
    root.a = std::make_unique<DockNode>();
    root.b = std::make_unique<DockNode>();
    root.a->panel = &left;
    root.b->panel = &right;

    layout.Draw();

    assert(left.HasDockBounds());
    assert(right.HasDockBounds());

    const auto& lb = left.DockBounds();
    const auto& rb = right.DockBounds();

    // Left panel: 60% of 1600 = 960 wide
    assert(lb.x == 0.0f);
    assert(lb.w == 960.0f);
    // Right panel: starts at 960, width = 640
    assert(rb.x == 960.0f);
    assert(rb.w == 640.0f);
    // Both full height (below menu bar)
    assert(lb.h == 878.0f);
    assert(rb.h == 878.0f);
}

void test_dock_layout_vertical_split_bounds() {
    TrackablePanel top("Top");
    TrackablePanel bottom("Bottom");

    EditorLayout layout;
    layout.RegisterPanel(&top);
    layout.RegisterPanel(&bottom);

    auto& root = layout.Root();
    root.split = DockSplit::Vertical;
    root.splitRatio = 0.70f;
    root.a = std::make_unique<DockNode>();
    root.b = std::make_unique<DockNode>();
    root.a->panel = &top;
    root.b->panel = &bottom;

    layout.Draw();

    const auto& tb = top.DockBounds();
    const auto& bb = bottom.DockBounds();

    // Top panel: 70% of 878 ≈ 614.6
    assert(std::abs(tb.h - 878.0f * 0.70f) < 0.1f);
    // Bottom panel: 30% of 878 ≈ 263.4
    assert(std::abs(bb.h - 878.0f * 0.30f) < 0.1f);
    // Bottom panel starts after top
    assert(std::abs(bb.y - (22.0f + 878.0f * 0.70f)) < 0.1f);
}

void test_dock_layout_tab_bounds() {
    TrackablePanel a("A");
    TrackablePanel b("B");

    EditorLayout layout;
    layout.RegisterPanel(&a);
    layout.RegisterPanel(&b);

    auto& root = layout.Root();
    root.split = DockSplit::Tab;
    root.tabs = {&a, &b};
    root.activeTab = 0;

    layout.Draw();

    // Active tab should get dock bounds
    assert(a.HasDockBounds());
    const auto& ab = a.DockBounds();
    assert(ab.x == 0.0f);
    assert(ab.y == 22.0f);
    assert(ab.w == 1600.0f);
    assert(ab.h == 878.0f);
}

void test_dock_layout_nested_split_bounds() {
    // Mimics the editor layout: Horizontal root, left has vertical split
    TrackablePanel viewport("Viewport");
    TrackablePanel console("Console");
    TrackablePanel tools("Tools");

    EditorLayout layout;
    layout.RegisterPanel(&viewport);
    layout.RegisterPanel(&console);
    layout.RegisterPanel(&tools);

    auto& root = layout.Root();
    root.split = DockSplit::Horizontal;
    root.splitRatio = 0.65f;

    root.a = std::make_unique<DockNode>();
    root.b = std::make_unique<DockNode>();

    // Left: vertical split (viewport on top, console on bottom)
    root.a->split = DockSplit::Vertical;
    root.a->splitRatio = 0.70f;
    root.a->a = std::make_unique<DockNode>();
    root.a->b = std::make_unique<DockNode>();
    root.a->a->panel = &viewport;
    root.a->b->panel = &console;

    // Right: single panel
    root.b->panel = &tools;

    layout.Draw();

    const auto& vb = viewport.DockBounds();
    const auto& cb = console.DockBounds();
    const auto& tb = tools.DockBounds();

    // Left column is 65% of 1600 = 1040
    assert(std::abs(vb.w - 1040.0f) < 0.1f);
    assert(std::abs(cb.w - 1040.0f) < 0.1f);
    // Viewport is 70% of left's height
    float leftH = 878.0f;
    assert(std::abs(vb.h - leftH * 0.70f) < 0.1f);
    // Console is 30% of left's height
    assert(std::abs(cb.h - leftH * 0.30f) < 0.1f);
    // Right column starts at x=1040, width=560
    assert(std::abs(tb.x - 1040.0f) < 0.1f);
    assert(std::abs(tb.w - 560.0f) < 0.1f);
    assert(std::abs(tb.h - 878.0f) < 0.1f);
}

void test_dock_set_dock_bounds() {
    TrackablePanel a("A");
    assert(!a.HasDockBounds());

    a.SetDockBounds({100, 200, 300, 400});
    assert(a.HasDockBounds());
    assert(a.DockBounds().x == 100.0f);
    assert(a.DockBounds().y == 200.0f);
    assert(a.DockBounds().w == 300.0f);
    assert(a.DockBounds().h == 400.0f);
}
