/**
 * Tests for editor panel features: ECS Inspector, Network Inspector,
 * Game Packager, AI Aggregator, and Editor Layout / Dock tabs.
 */

#include <iostream>
#include <cassert>
#include <string>
#include "../editor/panels/ECSInspectorPanel.h"
#include "../editor/panels/NetInspectorPanel.h"
#include "../editor/tools/GamePackagerPanel.h"
#include "../editor/ai/AIAggregator.h"
#include "../editor/ui/EditorLayout.h"
#include "../engine/ecs/ECS.h"
#include "../engine/net/NetContext.h"

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
