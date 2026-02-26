/**
 * Tests for Engine per-frame callback and EditorLayout serialisation.
 */

#include <cassert>
#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "../engine/core/Engine.h"
#include "../editor/ui/EditorLayout.h"
#include "../editor/ui/EditorPanel.h"

using namespace atlas;
using namespace atlas::editor;

// ── Helper panel ─────────────────────────────────────────────────

class CountingPanel : public EditorPanel {
public:
    explicit CountingPanel(const char* name) : m_name(name) {}
    const char* Name() const override { return m_name; }
    void Draw() override { ++drawCount; }
    int drawCount = 0;
private:
    const char* m_name;
};

// ══════════════════════════════════════════════════════════════════
// Engine frame callback tests
// ══════════════════════════════════════════════════════════════════

void test_engine_frame_callback_invoked() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Editor;
    cfg.maxTicks = 5;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    int callCount = 0;
    engine.SetFrameCallback([&callCount](float dt) {
        ++callCount;
    });

    engine.Run();
    assert(callCount == 5);
}

void test_engine_frame_callback_receives_dt() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Client;
    cfg.maxTicks = 1;
    cfg.tickRate = 20;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    float receivedDt = 0.0f;
    engine.SetFrameCallback([&receivedDt](float dt) {
        receivedDt = dt;
    });

    engine.Run();
    assert(std::fabs(receivedDt - 0.05f) < 0.001f); // 1/20 = 0.05
}

void test_engine_frame_callback_server_mode() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    int callCount = 0;
    engine.SetFrameCallback([&callCount](float) {
        ++callCount;
    });

    engine.Run();
    assert(callCount == 3);
}

void test_engine_no_callback_safe() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Editor;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    // No callback set — should not crash
    engine.Run();
    assert(engine.TickCount() == 3);
}

void test_engine_tick_count() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Editor;
    cfg.maxTicks = 10;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    assert(engine.TickCount() == 0);
    engine.Run();
    assert(engine.TickCount() == 10);
}

void test_engine_callback_replace() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Editor;
    cfg.maxTicks = 2;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    int first = 0;
    engine.SetFrameCallback([&first](float) { ++first; });
    // Replace
    int second = 0;
    engine.SetFrameCallback([&second](float) { ++second; });

    engine.Run();
    assert(first == 0);
    assert(second == 2);
}

// ══════════════════════════════════════════════════════════════════
// EditorLayout serialisation tests
// ══════════════════════════════════════════════════════════════════

void test_layout_serialize_empty() {
    EditorLayout layout;
    std::string json = layout.SerializeToJSON();
    assert(!json.empty());
    assert(json.find("\"layout\"") != std::string::npos);
}

void test_layout_serialize_single_panel() {
    CountingPanel panel("TestPanel");
    EditorLayout layout;
    layout.RegisterPanel(&panel);

    auto& root = layout.Root();
    root.split = DockSplit::None;
    root.panel = &panel;

    std::string json = layout.SerializeToJSON();
    assert(json.find("\"panel\": \"TestPanel\"") != std::string::npos);
    assert(json.find("\"split\": \"None\"") != std::string::npos);
}

void test_layout_serialize_split() {
    CountingPanel left("Left");
    CountingPanel right("Right");
    EditorLayout layout;
    layout.RegisterPanel(&left);
    layout.RegisterPanel(&right);

    auto& root = layout.Root();
    root.split = DockSplit::Horizontal;
    root.splitRatio = 0.6f;
    root.a = std::make_unique<DockNode>();
    root.b = std::make_unique<DockNode>();
    root.a->panel = &left;
    root.b->panel = &right;

    std::string json = layout.SerializeToJSON();
    assert(json.find("\"split\": \"Horizontal\"") != std::string::npos);
    assert(json.find("\"Left\"") != std::string::npos);
    assert(json.find("\"Right\"") != std::string::npos);
}

void test_layout_serialize_tabs() {
    CountingPanel a("TabA");
    CountingPanel b("TabB");
    EditorLayout layout;
    layout.RegisterPanel(&a);
    layout.RegisterPanel(&b);

    auto& root = layout.Root();
    root.split = DockSplit::Tab;
    root.tabs = {&a, &b};
    root.activeTab = 1;

    std::string json = layout.SerializeToJSON();
    assert(json.find("\"tabs\"") != std::string::npos);
    assert(json.find("\"TabA\"") != std::string::npos);
    assert(json.find("\"TabB\"") != std::string::npos);
    assert(json.find("\"activeTab\": 1") != std::string::npos);
}

void test_layout_roundtrip_single_panel() {
    CountingPanel panel("MyPanel");
    EditorLayout layout;
    layout.RegisterPanel(&panel);

    auto& root = layout.Root();
    root.split = DockSplit::None;
    root.panel = &panel;

    std::string json = layout.SerializeToJSON();

    // Deserialize into same layout
    bool ok = layout.DeserializeFromJSON(json);
    assert(ok);
    assert(layout.Root().split == DockSplit::None);
    assert(layout.Root().panel == &panel);
}

void test_layout_roundtrip_horizontal_split() {
    CountingPanel left("Left");
    CountingPanel right("Right");
    EditorLayout layout;
    layout.RegisterPanel(&left);
    layout.RegisterPanel(&right);

    auto& root = layout.Root();
    root.split = DockSplit::Horizontal;
    root.splitRatio = 0.65f;
    root.a = std::make_unique<DockNode>();
    root.b = std::make_unique<DockNode>();
    root.a->panel = &left;
    root.b->panel = &right;

    std::string json = layout.SerializeToJSON();
    bool ok = layout.DeserializeFromJSON(json);
    assert(ok);
    assert(layout.Root().split == DockSplit::Horizontal);
    assert(std::fabs(layout.Root().splitRatio - 0.65f) < 0.01f);
    assert(layout.Root().a != nullptr);
    assert(layout.Root().b != nullptr);
    assert(layout.Root().a->panel == &left);
    assert(layout.Root().b->panel == &right);
}

void test_layout_roundtrip_tabs() {
    CountingPanel a("Alpha");
    CountingPanel b("Beta");
    CountingPanel c("Gamma");
    EditorLayout layout;
    layout.RegisterPanel(&a);
    layout.RegisterPanel(&b);
    layout.RegisterPanel(&c);

    auto& root = layout.Root();
    root.split = DockSplit::Tab;
    root.tabs = {&a, &b, &c};
    root.activeTab = 2;

    std::string json = layout.SerializeToJSON();
    bool ok = layout.DeserializeFromJSON(json);
    assert(ok);
    assert(layout.Root().split == DockSplit::Tab);
    assert(layout.Root().tabs.size() == 3);
    assert(layout.Root().tabs[0] == &a);
    assert(layout.Root().tabs[1] == &b);
    assert(layout.Root().tabs[2] == &c);
    assert(layout.Root().activeTab == 2);
}

void test_layout_deserialize_invalid() {
    EditorLayout layout;
    bool ok = layout.DeserializeFromJSON("not valid json");
    assert(!ok);
}

void test_layout_deserialize_empty_string() {
    EditorLayout layout;
    bool ok = layout.DeserializeFromJSON("");
    assert(!ok);
}

void test_layout_file_roundtrip() {
    CountingPanel vp("Viewport");
    CountingPanel con("Console");
    EditorLayout layout;
    layout.RegisterPanel(&vp);
    layout.RegisterPanel(&con);

    auto& root = layout.Root();
    root.split = DockSplit::Vertical;
    root.splitRatio = 0.7f;
    root.a = std::make_unique<DockNode>();
    root.b = std::make_unique<DockNode>();
    root.a->panel = &vp;
    root.b->panel = &con;

    std::string path = "/tmp/test_layout_roundtrip.json";
    bool saved = layout.SaveToFile(path);
    assert(saved);
    assert(std::filesystem::exists(path));

    // Load into fresh layout (with same panels registered)
    EditorLayout layout2;
    layout2.RegisterPanel(&vp);
    layout2.RegisterPanel(&con);
    bool loaded = layout2.LoadFromFile(path);
    assert(loaded);
    assert(layout2.Root().split == DockSplit::Vertical);
    assert(std::fabs(layout2.Root().splitRatio - 0.7f) < 0.01f);
    assert(layout2.Root().a != nullptr);
    assert(layout2.Root().b != nullptr);
    assert(layout2.Root().a->panel == &vp);
    assert(layout2.Root().b->panel == &con);

    std::filesystem::remove(path);
}

void test_layout_load_nonexistent_file() {
    EditorLayout layout;
    bool loaded = layout.LoadFromFile("/tmp/nonexistent_layout_file_12345.json");
    assert(!loaded);
}

void test_layout_roundtrip_nested() {
    CountingPanel vp("Viewport");
    CountingPanel con("Console");
    CountingPanel ecs("ECS");
    CountingPanel net("Net");
    EditorLayout layout;
    layout.RegisterPanel(&vp);
    layout.RegisterPanel(&con);
    layout.RegisterPanel(&ecs);
    layout.RegisterPanel(&net);

    // Build: root(H) -> left(V) -> top(vp) / bottom(tab: con,ecs) | right(net)
    auto& root = layout.Root();
    root.split = DockSplit::Horizontal;
    root.splitRatio = 0.65f;
    root.a = std::make_unique<DockNode>();
    root.b = std::make_unique<DockNode>();

    root.a->split = DockSplit::Vertical;
    root.a->splitRatio = 0.7f;
    root.a->a = std::make_unique<DockNode>();
    root.a->b = std::make_unique<DockNode>();
    root.a->a->panel = &vp;
    root.a->b->split = DockSplit::Tab;
    root.a->b->tabs = {&con, &ecs};
    root.a->b->activeTab = 1;

    root.b->panel = &net;

    std::string json = layout.SerializeToJSON();
    bool ok = layout.DeserializeFromJSON(json);
    assert(ok);

    // Verify nested structure
    assert(layout.Root().split == DockSplit::Horizontal);
    assert(layout.Root().a != nullptr);
    assert(layout.Root().b != nullptr);
    assert(layout.Root().a->split == DockSplit::Vertical);
    assert(layout.Root().a->a != nullptr);
    assert(layout.Root().a->a->panel == &vp);
    assert(layout.Root().a->b != nullptr);
    assert(layout.Root().a->b->split == DockSplit::Tab);
    assert(layout.Root().a->b->tabs.size() == 2);
    assert(layout.Root().a->b->tabs[0] == &con);
    assert(layout.Root().a->b->tabs[1] == &ecs);
    assert(layout.Root().a->b->activeTab == 1);
    assert(layout.Root().b->panel == &net);
}
