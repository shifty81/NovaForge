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
#include "tools/CharacterSelectPanel.h"
#include "tools/MissionEditorPanel.h"
#include "tools/SceneGraphPanel.h"
#include "tools/DataBrowserPanel.h"
#include "tools/ModuleEditorPanel.h"
#include "tools/NPCEditorPanel.h"
#include "tools/GalaxyMapPanel.h"
#include "tools/FleetFormationPanel.h"
#include "tools/SolarSystemEditorPanel.h"
#include "tools/ModelImportPanel.h"
#include "tools/StationEditorPanel.h"
#include "tools/TradeRoutePanel.h"
#include "tools/CombatLogPanel.h"
#include "ai/AIAggregator.h"
#include "ai/TemplateAIBackend.h"
#include "ui/KeybindManager.h"
#include "ui/UndoStack.h"
#include "assets/AssetRegistry.h"
#include "../cpp_client/include/ui/atlas/atlas_context.h"
#include "ui/input_handler.h"
#include "rendering/window.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <filesystem>

int main() {
    // ── Log working directory for diagnostics ────────────────────
    std::cout << "[Editor] Working directory: "
              << std::filesystem::current_path().string() << std::endl;
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
    atlas::editor::CharacterSelectPanel characterSelect;
    atlas::editor::MissionEditorPanel missionEditor;
    atlas::editor::SceneGraphPanel sceneGraph;
    atlas::editor::DataBrowserPanel dataBrowser;
    atlas::editor::ModuleEditorPanel moduleEditor;
    atlas::editor::NPCEditorPanel npcEditor;
    atlas::editor::GalaxyMapPanel galaxyMap;
    atlas::editor::FleetFormationPanel fleetFormation;
    atlas::editor::SolarSystemEditorPanel solarSystemEditor;
    atlas::editor::ModelImportPanel modelImport;
    atlas::editor::StationEditorPanel stationEditor;
    atlas::editor::TradeRoutePanel tradeRoute;
    atlas::editor::CombatLogPanel combatLog;

    // ── Keyboard shortcut manager ────────────────────────────────
    atlas::editor::KeybindManager keybinds;
    // Try to load saved keybind overrides; defaults remain if no file exists
    keybinds.LoadFromFile("data/editor_keybinds.json");

    // ── Undo/Redo stack ──────────────────────────────────────────
    atlas::editor::UndoStack undoStack;

    // Wire undo/redo to keyboard shortcuts
    keybinds.RegisterCallback("Undo", [&undoStack, &console]() {
        if (undoStack.CanUndo()) {
            std::string desc = undoStack.UndoDescription();
            undoStack.Undo();
            console.AddLine("[Undo] " + desc);
        }
    });
    keybinds.RegisterCallback("Redo", [&undoStack, &console]() {
        if (undoStack.CanRedo()) {
            std::string desc = undoStack.RedoDescription();
            undoStack.Redo();
            console.AddLine("[Redo] " + desc);
        }
    });

    // ── Viewport gizmo keybinds ──────────────────────────────────
    keybinds.RegisterCallback("Translate", [&viewport]() {
        viewport.SetGizmoMode(atlas::editor::GizmoMode::Translate);
    });
    keybinds.RegisterCallback("Rotate", [&viewport]() {
        viewport.SetGizmoMode(atlas::editor::GizmoMode::Rotate);
    });
    keybinds.RegisterCallback("Scale", [&viewport]() {
        viewport.SetGizmoMode(atlas::editor::GizmoMode::Scale);
    });
    keybinds.RegisterCallback("ToggleGrid", [&viewport]() {
        viewport.SetGridVisible(!viewport.IsGridVisible());
    });
    keybinds.RegisterCallback("FocusSelected", [&viewport]() {
        if (viewport.SelectedObjectId() != 0) {
            viewport.SetCameraDistance(200.0f);
        }
    });

    // ── Panel toggle keybinds ────────────────────────────────────
    keybinds.RegisterCallback("ToggleConsole", [&console]() {
        console.SetVisible(!console.IsVisible());
    });
    keybinds.RegisterCallback("ToggleInspector", [&ecsInspector]() {
        ecsInspector.SetVisible(!ecsInspector.IsVisible());
    });
    keybinds.RegisterCallback("ToggleSceneGraph", [&sceneGraph]() {
        sceneGraph.SetVisible(!sceneGraph.IsVisible());
    });

    // ── Delete keybind ───────────────────────────────────────────
    keybinds.RegisterCallback("Delete", [&viewport, &ecsInspector, &sceneGraph,
                                          &undoStack, &console]() {
        // Delete selected viewport object
        if (viewport.SelectedObjectId() != 0) {
            uint32_t id = viewport.SelectedObjectId();
            console.AddLine("[Delete] Viewport object " + std::to_string(id));
            viewport.DeselectAll();
        }
        // Delete selected ECS entity
        else if (ecsInspector.SelectedEntity() != 0) {
            atlas::ecs::EntityID eid = ecsInspector.SelectedEntity();
            undoStack.PushAction({
                "Delete entity " + std::to_string(eid),
                [&ecsInspector, &console, eid]() {
                    // Undo: entity already destroyed, cannot recreate — log the limitation
                    console.AddLine("[Undo] Entity " + std::to_string(eid) + " was destroyed (cannot recreate)");
                    ecsInspector.ClearSelection();
                },
                [&ecsInspector, &console, eid]() {
                    // Redo: entity already gone, log the operation
                    console.AddLine("[Redo] Entity " + std::to_string(eid) + " deletion confirmed");
                }
            });
            console.AddLine("[Delete] Entity " + std::to_string(eid));
            ecsInspector.DestroySelectedEntity();
        }
        // Delete selected scene graph node
        else if (sceneGraph.SelectedNode() != 0) {
            uint32_t nid = sceneGraph.SelectedNode();
            const auto* node = sceneGraph.GetNode(nid);
            std::string nodeName = node ? node->name : std::to_string(nid);
            std::string nodeType = node ? node->type : "Entity";
            uint32_t parentId = node ? node->parentId : 0;
            undoStack.PushAction({
                "Delete node '" + nodeName + "'",
                [&sceneGraph, nodeName, nodeType, parentId]() {
                    // Undo: re-add the node (new id, same name/type/parent)
                    sceneGraph.AddNode(nodeName, nodeType, parentId);
                },
                [&sceneGraph, nid]() {
                    // Redo: remove the specific node by its captured ID
                    sceneGraph.RemoveNode(nid);
                }
            });
            sceneGraph.RemoveNode(nid);
            console.AddLine("[Delete] Scene node '" + nodeName + "'");
        }
    });

    // ── AI backend: template-based offline suggestions ────────────
    atlas::ai::AIAggregator aiAggregator;
    atlas::ai::TemplateAIBackend templateAI;
    aiAggregator.RegisterBackend(&templateAI);

    // Connect AI to console so `ai.query` commands work
    console.SetAIAggregator(&aiAggregator);

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
    layout.RegisterPanel(&characterSelect);
    layout.RegisterPanel(&missionEditor);
    layout.RegisterPanel(&sceneGraph);
    layout.RegisterPanel(&liveScene);
    layout.RegisterPanel(&dataBrowser);
    layout.RegisterPanel(&moduleEditor);
    layout.RegisterPanel(&npcEditor);
    layout.RegisterPanel(&galaxyMap);
    layout.RegisterPanel(&fleetFormation);
    layout.RegisterPanel(&solarSystemEditor);
    layout.RegisterPanel(&modelImport);
    layout.RegisterPanel(&stationEditor);
    layout.RegisterPanel(&tradeRoute);
    layout.RegisterPanel(&combatLog);

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
    root.a->b->tabs = {&console, &ecsInspector, &netInspector, &sceneGraph,
                        &dataBrowser, &moduleEditor, &npcEditor, &fleetFormation};
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
    root.b->a->a->split = atlas::editor::DockSplit::Tab;
    root.b->a->a->tabs = {&pcgPreview, &characterSelect, &missionEditor, &galaxyMap,
                          &solarSystemEditor, &modelImport, &stationEditor,
                          &tradeRoute, &combatLog};
    root.b->a->a->activeTab = 0;
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
    std::cout << "[Editor] Panels: Viewport, PCG Preview, Character Select, "
              << "Mission Editor, Galaxy Map, Solar System Editor, Model Import, "
              << "Station Editor, Scene Graph, "
              << "Generation Style, Asset Style, Ship Archetype, ECS Inspector, "
              << "Net Inspector, Console, Game Packager, Live Scene Manager, "
              << "Data Browser, Module Editor, NPC Editor, Fleet Formation"
              << std::endl;

    // ── Save keybind (Ctrl+S) ────────────────────────────────────
    // Registered here (after layout & liveScene are constructed) so
    // the lambda captures are valid.
    keybinds.RegisterCallback("Save", [&layout, &liveScene, &keybinds, &console]() {
        layout.SaveToFile("data/editor_layout.json");
        liveScene.CaptureViewportChanges();
        liveScene.SaveOverrides("data/pcg_overrides.json");
        keybinds.SaveToFile("data/editor_keybinds.json");
        console.AddLine("[Editor] Layout, overrides, and keybinds saved");
    });

    // ── Build editor menu bar ────────────────────────────────────
    layout.MenuBar().Build();

    // Wire menu bar callbacks
    layout.MenuBar().onSaveLayout = [&layout]() {
        layout.SaveToFile("data/editor_layout.json");
        std::cout << "[Editor] Layout saved" << std::endl;
    };
    layout.MenuBar().onLoadLayout = [&layout]() {
        layout.LoadFromFile("data/editor_layout.json");
        std::cout << "[Editor] Layout loaded" << std::endl;
    };
    layout.MenuBar().onExit = [&engine]() {
        engine.Shutdown();
    };
    layout.MenuBar().onPCGContentSelected = [](const std::string& name) {
        std::cout << "[Editor] PCG panel toggled: " << name << std::endl;
    };

    std::cout << "[Editor] Menu bar initialized with "
              << layout.MenuBar().Menus().size() << " menus" << std::endl;

    // ── Atlas UI context for editor panels ────────────────────
    // Create the editor window (GLFW + OpenGL context) before UI init
    atlas::Window window("Atlas Editor", 1600, 900);

    atlas::AtlasContext uiContext;
    uiContext.init();

    // ── Load theme from JSON ─────────────────────────────────────
    const std::string themePath = "data/ui/novaforge_dark_theme.json";
    if (uiContext.loadThemeFromFile(themePath)) {
        std::cout << "[Editor] Theme loaded from " << themePath << std::endl;
    } else {
        std::cout << "[Editor] WARNING: Could not load theme from "
                  << themePath << " — using defaults" << std::endl;
    }

    // ── Apply DPI scaling ────────────────────────────────────────
    float dpiScale = window.getContentScale();
    if (dpiScale > 1.0f) {
        atlas::Theme t = uiContext.theme();
        t.applyDpiScale(dpiScale);
        // fontScale is independent — text rendering uses it separately
        t.fontScale = dpiScale;
        uiContext.setTheme(t);
        std::cout << "[Editor] DPI scale: " << dpiScale << "x" << std::endl;
    } else {
        std::cout << "[Editor] DPI scale: 1.0x (standard)" << std::endl;
    }

    layout.SetContext(&uiContext);

    // ── Wire GLFW input → InputHandler for mouse/keyboard ────────
    atlas::InputHandler inputHandler;
    window.setMouseCallback([&inputHandler](double x, double y) {
        inputHandler.handleMouse(x, y);
    });
    window.setMouseButtonCallback([&inputHandler](int button, int action, int mods) {
        inputHandler.handleMouseButton(button, action, mods,
                                        inputHandler.getMouseX(),
                                        inputHandler.getMouseY());
    });
    window.setScrollCallback([&inputHandler](double xoff, double yoff) {
        inputHandler.handleScroll(xoff, yoff);
    });
    window.setKeyCallback([&inputHandler, &keybinds](int key, int /*scancode*/, int action, int mods) {
        inputHandler.handleKey(key, action, mods);
        if (action == GLFW_PRESS) {
            atlas::editor::KeyMod km = atlas::editor::KeyMod::None;
            if (mods & GLFW_MOD_CONTROL) km = km | atlas::editor::KeyMod::Ctrl;
            if (mods & GLFW_MOD_SHIFT)   km = km | atlas::editor::KeyMod::Shift;
            if (mods & GLFW_MOD_ALT)     km = km | atlas::editor::KeyMod::Alt;
            keybinds.HandleKeyPress(key, km);
        }
    });

    std::cout << "[Editor] Input handler connected" << std::endl;

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

    // ── Scan reference models for the model import panel ─────────
    modelImport.ScanDirectory("cpp_client/assets/reference_models");
    std::cout << "[Editor] Model catalogue: "
              << modelImport.ModelCount() << " reference models" << std::endl;

    std::cout << "[Editor] Viewport populated with "
              << viewport.ObjectCount() << " objects" << std::endl;
    std::cout << "[Editor] Ready — drag objects in the viewport to edit, "
              << "then save overrides for the client" << std::endl;

    // ── Load saved dock layout (if any) ──────────────────────────
    if (layout.LoadFromFile("data/editor_layout.json")) {
        std::cout << "[Editor] Dock layout loaded from data/editor_layout.json"
                  << std::endl;
    } else {
        std::cout << "[Editor] No saved layout found at data/editor_layout.json"
                  << " — using built-in default layout" << std::endl;
    }

    // ── Per-frame callback: UI drawing, hot-reload, keybinds ────
    engine.SetFrameCallback([&](float /*dt*/) {
        // Close the editor when the window is closed
        if (window.shouldClose()) {
            engine.Shutdown();
            return;
        }

        // Reset per-frame transient input (clicked/released flags)
        inputHandler.beginFrame();

        // Poll window events (keyboard, mouse, resize)
        window.pollEvents();

        // Hot-reload assets
        assetRegistry.PollHotReload();

        // Clear the framebuffer
        glClearColor(0.05f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Build per-frame input state for the Atlas UI
        atlas::InputState input;
        input.windowW = window.getWidth();
        input.windowH = window.getHeight();
        input.mousePos = {static_cast<float>(inputHandler.getMouseX()),
                          static_cast<float>(inputHandler.getMouseY())};
        input.mouseDown[0]     = inputHandler.isMouseDown(0);
        input.mouseDown[1]     = inputHandler.isMouseDown(1);
        input.mouseDown[2]     = inputHandler.isMouseDown(2);
        input.mouseClicked[0]  = inputHandler.isMouseClicked(0);
        input.mouseClicked[1]  = inputHandler.isMouseClicked(1);
        input.mouseClicked[2]  = inputHandler.isMouseClicked(2);
        input.mouseReleased[0] = inputHandler.isMouseReleased(0);
        input.mouseReleased[1] = inputHandler.isMouseReleased(1);
        input.mouseReleased[2] = inputHandler.isMouseReleased(2);
        input.scrollY = inputHandler.getScrollDeltaY();

        // Begin UI frame, draw layout, end UI frame
        uiContext.beginFrame(input);
        layout.Draw();
        uiContext.endFrame();

        // Present the frame
        window.swapBuffers();
    });

    // ── Run editor loop ───────────────────────────────────────
    engine.Run();

    // ── Cleanup ───────────────────────────────────────────────
    layout.SetContext(nullptr);
    uiContext.shutdown();

    return 0;
}
