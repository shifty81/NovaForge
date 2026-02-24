#include "core/Engine.h"
#include "ui/EditorLayout.h"
#include "panels/ECSInspectorPanel.h"
#include "panels/NetInspectorPanel.h"
#include "panels/ConsolePanel.h"
#include "tools/PCGPreviewPanel.h"
#include "tools/GenerationStylePanel.h"
#include "tools/AssetStylePanel.h"
#include "tools/ShipArchetypePanel.h"
#include "tools/GamePackagerPanel.h"
#include "tools/ViewportPanel.h"
#include "tools/LiveSceneManager.h"
#include "assets/AssetRegistry.h"
#include "../cpp_client/include/ui/atlas/atlas_context.h"
#include <iostream>
#include <memory>

int main() {
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Editor;

    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();
    engine.InitEditor();

    // ── Create editor panels ──────────────────────────────────
    atlas::editor::ViewportPanel viewport;
    atlas::editor::PCGPreviewPanel pcgPreview;
    atlas::editor::GenerationStylePanel genStyle;
    atlas::editor::AssetStylePanel assetStyle;
    atlas::editor::ShipArchetypePanel shipArchetype;
    atlas::editor::ECSInspectorPanel ecsInspector(engine.GetWorld());
    atlas::editor::NetInspectorPanel netInspector(engine.GetNet());
    atlas::editor::ConsolePanel console(engine.GetWorld(), engine.GetNet(),
                                        engine.GetScheduler());
    atlas::editor::GamePackagerPanel packager;

    // ── Live scene manager: connects viewport ↔ PCG ──────────
    // This is the core of the in-engine content creation workflow.
    // It populates the viewport with PCG content, captures viewport
    // edits as overrides, and saves them to a file the client reads.
    atlas::editor::LiveSceneManager liveScene(&viewport, &pcgPreview);

    // ── Build dock layout ─────────────────────────────────────
    //
    //  ┌─────────────────────────────┬──────────────┐
    //  │                             │  PCG Preview  │
    //  │         Viewport            ├──────────────┤
    //  │   (live game scene with     │  Ship Arch.  │
    //  │    click-drag editing)      ├──────────────┤
    //  │                             │  Gen Style   │
    //  ├─────────────────────────────┼──────────────┤
    //  │  Console / ECS / Net /      │ Asset Style  │
    //  │  Live Scene                 │ Packager     │
    //  └─────────────────────────────┴──────────────┘
    //
    atlas::editor::EditorLayout layout;
    layout.RegisterPanel(&viewport);
    layout.RegisterPanel(&pcgPreview);
    layout.RegisterPanel(&genStyle);
    layout.RegisterPanel(&assetStyle);
    layout.RegisterPanel(&shipArchetype);
    layout.RegisterPanel(&ecsInspector);
    layout.RegisterPanel(&netInspector);
    layout.RegisterPanel(&console);
    layout.RegisterPanel(&packager);
    layout.RegisterPanel(&liveScene);

    // Root: horizontal split — left (viewport area) | right (tool panels)
    auto& root = layout.Root();
    root.split = atlas::editor::DockSplit::Horizontal;
    root.splitRatio = 0.65f;

    root.a = std::make_unique<atlas::editor::DockNode>();
    root.b = std::make_unique<atlas::editor::DockNode>();

    // Left side: vertical split — top (viewport) | bottom (console/inspectors)
    root.a->split = atlas::editor::DockSplit::Vertical;
    root.a->splitRatio = 0.70f;
    root.a->a = std::make_unique<atlas::editor::DockNode>();
    root.a->b = std::make_unique<atlas::editor::DockNode>();
    root.a->a->panel = &viewport;
    root.a->b->split = atlas::editor::DockSplit::Tab;
    root.a->b->tabs = {&console, &ecsInspector, &netInspector};
    root.a->b->activeTab = 0;

    // Right side: vertical split — top (PCG preview + tools) | bottom (asset/packager)
    root.b->split = atlas::editor::DockSplit::Vertical;
    root.b->splitRatio = 0.60f;
    root.b->a = std::make_unique<atlas::editor::DockNode>();
    root.b->b = std::make_unique<atlas::editor::DockNode>();

    // Right-top: vertical chain of PCG panels
    root.b->a->split = atlas::editor::DockSplit::Vertical;
    root.b->a->splitRatio = 0.33f;
    root.b->a->a = std::make_unique<atlas::editor::DockNode>();
    root.b->a->b = std::make_unique<atlas::editor::DockNode>();
    root.b->a->a->panel = &pcgPreview;
    root.b->a->b->split = atlas::editor::DockSplit::Vertical;
    root.b->a->b->splitRatio = 0.50f;
    root.b->a->b->a = std::make_unique<atlas::editor::DockNode>();
    root.b->a->b->b = std::make_unique<atlas::editor::DockNode>();
    root.b->a->b->a->panel = &shipArchetype;
    root.b->a->b->b->panel = &genStyle;

    // Right-bottom: asset style + packager
    root.b->b->split = atlas::editor::DockSplit::Vertical;
    root.b->b->splitRatio = 0.50f;
    root.b->b->a = std::make_unique<atlas::editor::DockNode>();
    root.b->b->b = std::make_unique<atlas::editor::DockNode>();
    root.b->b->a->panel = &assetStyle;
    root.b->b->b->panel = &packager;

    std::cout << "[Editor] Layout built with " << layout.Panels().size()
              << " panels" << std::endl;
    std::cout << "[Editor] Panels: Viewport, PCG Preview, Generation Style, "
              << "Asset Style, Ship Archetype, ECS Inspector, Net Inspector, "
              << "Console, Game Packager, Live Scene Manager" << std::endl;

    // ── Atlas UI context for editor panels ────────────────────
    atlas::AtlasContext uiContext;
    uiContext.init();
    layout.SetContext(&uiContext);

    std::cout << "[Editor] Atlas UI context initialized for "
              << layout.Panels().size() << " panels" << std::endl;

    // ── Live-reload: watch asset files for changes ────────────
    atlas::asset::AssetRegistry assetRegistry;
    assetRegistry.Scan(engine.Config().assetRoot);
    assetRegistry.Scan("data");
    assetRegistry.SetReloadCallback(
        [&layout](const atlas::asset::AssetEntry& entry) {
            std::cout << "[HotReload] " << entry.id
                      << " changed (v" << entry.version << ") — "
                      << entry.path << std::endl;
            layout.BroadcastAssetReload(entry.id, entry.path);
        });

    std::cout << "[Editor] Live-reload enabled — watching "
              << assetRegistry.Count() << " assets" << std::endl;

    // ── Auto-populate the viewport with a default game scene ──
    // Load any previously saved overrides so the designer picks up
    // where they left off.
    liveScene.LoadOverrides("data/pcg_overrides.json");
    liveScene.PopulateDefaultScene();

    std::cout << "[Editor] Viewport populated with "
              << viewport.ObjectCount() << " objects" << std::endl;
    std::cout << "[Editor] Ready — drag objects in the viewport to edit, "
              << "then save overrides for the client" << std::endl;

    // ── Run editor loop ───────────────────────────────────────
    engine.Run();

    // ── Cleanup ───────────────────────────────────────────────
    layout.SetContext(nullptr);
    uiContext.shutdown();

    return 0;
}
