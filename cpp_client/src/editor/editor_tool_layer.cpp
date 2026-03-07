#ifdef NOVAFORGE_EDITOR_TOOLS

#include "editor/editor_tool_layer.h"

// Editor panel headers — resolved via include path to editor/
#include "ui/EditorLayout.h"
#include "ui/KeybindManager.h"
#include "ui/UndoStack.h"
#include "tools/ViewportPanel.h"
#include "tools/PCGPreviewPanel.h"
#include "tools/GenerationStylePanel.h"
#include "tools/AssetStylePanel.h"
#include "tools/ShipArchetypePanel.h"
#include "tools/GamePackagerPanel.h"
#include "tools/CharacterSelectPanel.h"
#include "tools/MissionEditorPanel.h"
#include "tools/SceneGraphPanel.h"
#include "tools/DataBrowserPanel.h"
#include "tools/ModuleEditorPanel.h"
#include "tools/NPCEditorPanel.h"
#include "tools/GalaxyMapPanel.h"
#include "tools/FleetFormationPanel.h"
#include "tools/LiveSceneManager.h"
#include "tools/AssetPalettePanel.h"
#include "tools/PhysicsTunerPanel.h"
#include "ai/AIAggregator.h"
#include "ai/TemplateAIBackend.h"
#include "editor/scene_bookmark_manager.h"
#include "editor/layer_tag_system.h"
#include "ui/atlas/atlas_context.h"

#include <iostream>

namespace atlas::editor {

// ─────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────

EditorToolLayer::EditorToolLayer() = default;

EditorToolLayer::~EditorToolLayer() {
    if (m_initialized) {
        shutdown();
    }
}

// ─────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────

void EditorToolLayer::init() {
    if (m_initialized) return;

    std::cout << "[ToolLayer] Initializing editor tool layer..." << std::endl;

    // ── Create infrastructure ────────────────────────────────────
    m_layout    = std::make_unique<EditorLayout>();
    m_keybinds  = std::make_unique<KeybindManager>();
    m_undoStack = std::make_unique<UndoStack>();

    // Try to load saved keybind overrides
    if (m_keybinds->LoadFromFile("data/editor_keybinds.json")) {
        std::cout << "[ToolLayer] Custom keybinds loaded" << std::endl;
    } else {
        std::cout << "[ToolLayer] Using default keybinds" << std::endl;
    }

    // ── AI backend ───────────────────────────────────────────────
    m_aiAggregator = std::make_unique<::atlas::ai::AIAggregator>();
    m_templateAI   = std::make_unique<::atlas::ai::TemplateAIBackend>();
    m_aiAggregator->RegisterBackend(m_templateAI.get());

    // ── Create standalone editor panels ──────────────────────────
    m_viewport        = std::make_unique<ViewportPanel>();
    m_pcgPreview      = std::make_unique<PCGPreviewPanel>();
    m_genStyle        = std::make_unique<GenerationStylePanel>();
    m_assetStyle      = std::make_unique<AssetStylePanel>();
    m_shipArchetype   = std::make_unique<ShipArchetypePanel>();
    m_packager        = std::make_unique<GamePackagerPanel>();
    m_characterSelect = std::make_unique<CharacterSelectPanel>();
    m_missionEditor   = std::make_unique<MissionEditorPanel>();
    m_sceneGraph      = std::make_unique<SceneGraphPanel>();
    m_dataBrowser     = std::make_unique<DataBrowserPanel>();
    m_moduleEditor    = std::make_unique<ModuleEditorPanel>();
    m_npcEditor       = std::make_unique<NPCEditorPanel>();
    m_galaxyMap       = std::make_unique<GalaxyMapPanel>();
    m_fleetFormation  = std::make_unique<FleetFormationPanel>();
    m_assetPalette    = std::make_unique<AssetPalettePanel>();
    m_physicsTuner    = std::make_unique<PhysicsTunerPanel>();

    // ── Live scene manager ───────────────────────────────────────
    m_liveScene = std::make_unique<LiveSceneManager>(
        m_viewport.get(), m_pcgPreview.get());

    // ── Editor utilities ─────────────────────────────────────────
    m_bookmarkManager = std::make_unique<SceneBookmarkManager>();
    m_layerTagSystem  = std::make_unique<LayerTagSystem>();

    // ── Register panels with layout ──────────────────────────────
    m_layout->RegisterPanel(m_viewport.get());
    m_layout->RegisterPanel(m_pcgPreview.get());
    m_layout->RegisterPanel(m_genStyle.get());
    m_layout->RegisterPanel(m_assetStyle.get());
    m_layout->RegisterPanel(m_shipArchetype.get());
    m_layout->RegisterPanel(m_packager.get());
    m_layout->RegisterPanel(m_characterSelect.get());
    m_layout->RegisterPanel(m_missionEditor.get());
    m_layout->RegisterPanel(m_sceneGraph.get());
    m_layout->RegisterPanel(m_liveScene.get());
    m_layout->RegisterPanel(m_dataBrowser.get());
    m_layout->RegisterPanel(m_moduleEditor.get());
    m_layout->RegisterPanel(m_npcEditor.get());
    m_layout->RegisterPanel(m_galaxyMap.get());
    m_layout->RegisterPanel(m_fleetFormation.get());
    m_layout->RegisterPanel(m_assetPalette.get());
    m_layout->RegisterPanel(m_physicsTuner.get());

    // ── Build dock layout ────────────────────────────────────────
    //
    //  ┌─────────────────────────────┬──────────────┐
    //  │                             │  PCG Preview  │
    //  │         Viewport            ├──────────────┤
    //  │                             │  Ship Arch.  │
    //  ├─────────────────────────────┼──────────────┤
    //  │  Scene Graph / Data Browser │ Asset Style  │
    //  │  Module / NPC / Fleet       │ Packager     │
    //  └─────────────────────────────┴──────────────┘
    //
    auto& root = m_layout->Root();
    root.split = DockSplit::Horizontal;
    root.splitRatio = 0.65f;

    root.a = std::make_unique<DockNode>();
    root.b = std::make_unique<DockNode>();

    // Left: viewport on top, tool tabs on bottom
    root.a->split = DockSplit::Vertical;
    root.a->splitRatio = 0.70f;
    root.a->a = std::make_unique<DockNode>();
    root.a->b = std::make_unique<DockNode>();
    root.a->a->panel = m_viewport.get();
    root.a->b->split = DockSplit::Tab;
    root.a->b->tabs  = {m_sceneGraph.get(), m_dataBrowser.get(),
                         m_moduleEditor.get(), m_npcEditor.get(),
                         m_fleetFormation.get(), m_assetPalette.get(),
                         m_physicsTuner.get()};
    root.a->b->activeTab = 0;

    // Right: PCG panels on top, asset style + packager on bottom
    root.b->split = DockSplit::Vertical;
    root.b->splitRatio = 0.60f;
    root.b->a = std::make_unique<DockNode>();
    root.b->b = std::make_unique<DockNode>();

    root.b->a->split = DockSplit::Vertical;
    root.b->a->splitRatio = 0.33f;
    root.b->a->a = std::make_unique<DockNode>();
    root.b->a->b = std::make_unique<DockNode>();
    root.b->a->a->split = DockSplit::Tab;
    root.b->a->a->tabs  = {m_pcgPreview.get(), m_characterSelect.get(),
                            m_missionEditor.get(), m_galaxyMap.get()};
    root.b->a->a->activeTab = 0;
    root.b->a->b->split = DockSplit::Vertical;
    root.b->a->b->splitRatio = 0.50f;
    root.b->a->b->a = std::make_unique<DockNode>();
    root.b->a->b->b = std::make_unique<DockNode>();
    root.b->a->b->a->panel = m_shipArchetype.get();
    root.b->a->b->b->panel = m_genStyle.get();

    root.b->b->split = DockSplit::Vertical;
    root.b->b->splitRatio = 0.50f;
    root.b->b->a = std::make_unique<DockNode>();
    root.b->b->b = std::make_unique<DockNode>();
    root.b->b->a->panel = m_assetStyle.get();
    root.b->b->b->panel = m_packager.get();

    // ── Wire undo/redo keybinds ──────────────────────────────────
    m_keybinds->RegisterCallback("Undo", [this]() {
        if (m_undoStack->CanUndo()) {
            m_undoStack->Undo();
        }
    });
    m_keybinds->RegisterCallback("Redo", [this]() {
        if (m_undoStack->CanRedo()) {
            m_undoStack->Redo();
        }
    });

    // ── Viewport gizmo keybinds ──────────────────────────────────
    m_keybinds->RegisterCallback("Translate", [this]() {
        m_viewport->SetGizmoMode(GizmoMode::Translate);
    });
    m_keybinds->RegisterCallback("Rotate", [this]() {
        m_viewport->SetGizmoMode(GizmoMode::Rotate);
    });
    m_keybinds->RegisterCallback("Scale", [this]() {
        m_viewport->SetGizmoMode(GizmoMode::Scale);
    });
    m_keybinds->RegisterCallback("ToggleGrid", [this]() {
        m_viewport->SetGridVisible(!m_viewport->IsGridVisible());
    });

    // ── Save keybind ─────────────────────────────────────────────
    m_keybinds->RegisterCallback("Save", [this]() {
        m_layout->SaveToFile("data/editor_layout.json");
        m_liveScene->CaptureViewportChanges();
        m_liveScene->SaveOverrides("data/pcg_overrides.json");
        m_keybinds->SaveToFile("data/editor_keybinds.json");
        m_deltaEditStore.SaveToFile("data/delta_edits.json");
        m_bookmarkManager->SaveToFile("data/editor_bookmarks.json");
        std::cout << "[ToolLayer] Layout, overrides, keybinds, delta edits, and bookmarks saved"
                  << std::endl;
    });

    // ── Build menu bar ───────────────────────────────────────────
    m_layout->MenuBar().Build();

    m_layout->MenuBar().onSaveLayout = [this]() {
        m_layout->SaveToFile("data/editor_layout.json");
    };
    m_layout->MenuBar().onLoadLayout = [this]() {
        m_layout->LoadFromFile("data/editor_layout.json");
    };
    m_layout->MenuBar().onPCGContentSelected = [](const std::string& name) {
        std::cout << "[ToolLayer] PCG panel toggled: " << name << std::endl;
    };

    // ── Load saved dock layout (if any) ──────────────────────────
    if (m_layout->LoadFromFile("data/editor_layout.json")) {
        std::cout << "[ToolLayer] Dock layout loaded" << std::endl;
    }

    // ── Load saved PCG overrides ─────────────────────────────────
    m_liveScene->LoadOverrides("data/pcg_overrides.json");
    m_liveScene->PopulateDefaultScene();

    // ── Load saved delta edits ───────────────────────────────────
    if (m_deltaEditStore.LoadFromFile("data/delta_edits.json")) {
        std::cout << "[ToolLayer] Delta edits loaded (" << m_deltaEditStore.Count()
                  << " edits)" << std::endl;
    }

    // ── Load saved bookmarks ─────────────────────────────────────
    if (m_bookmarkManager->LoadFromFile("data/editor_bookmarks.json")) {
        std::cout << "[ToolLayer] Bookmarks loaded (" << m_bookmarkManager->Count()
                  << " bookmarks)" << std::endl;
    }

    m_initialized = true;

    std::cout << "[ToolLayer] Initialized with "
              << m_layout->Panels().size() << " panels" << std::endl;
    std::cout << "[ToolLayer] Press F12 to toggle the editor overlay"
              << std::endl;
}

void EditorToolLayer::shutdown() {
    if (!m_initialized) return;

    std::cout << "[ToolLayer] Shutting down..." << std::endl;

    m_layout->SetContext(nullptr);

    m_liveScene.reset();
    m_layerTagSystem.reset();
    m_bookmarkManager.reset();
    m_physicsTuner.reset();
    m_assetPalette.reset();
    m_fleetFormation.reset();
    m_galaxyMap.reset();
    m_npcEditor.reset();
    m_moduleEditor.reset();
    m_dataBrowser.reset();
    m_sceneGraph.reset();
    m_missionEditor.reset();
    m_characterSelect.reset();
    m_packager.reset();
    m_shipArchetype.reset();
    m_assetStyle.reset();
    m_genStyle.reset();
    m_pcgPreview.reset();
    m_viewport.reset();

    m_templateAI.reset();
    m_aiAggregator.reset();
    m_undoStack.reset();
    m_keybinds.reset();
    m_layout.reset();

    m_tools.clear();
    m_initialized = false;

    std::cout << "[ToolLayer] Shutdown complete" << std::endl;
}

// ─────────────────────────────────────────────────────────────────
// Per-frame
// ─────────────────────────────────────────────────────────────────

void EditorToolLayer::draw(atlas::AtlasContext& ctx) {
    if (!m_active || !m_initialized) return;

    m_layout->SetContext(&ctx);

    // Process any queued commands
    m_commandBus.ProcessCommands();
    m_undoableCommandBus.ProcessCommands();

    // Draw the full editor layout (menu bar + dock tree)
    m_layout->Draw();
}

void EditorToolLayer::handleKeyPress(int key, int mods) {
    if (!m_active || !m_initialized) return;

    KeyMod km = KeyMod::None;
    // GLFW modifier bit masks (from glfw3.h)
    constexpr int GLFW_MOD_SHIFT_BIT   = 0x0001;
    constexpr int GLFW_MOD_CONTROL_BIT = 0x0002;
    constexpr int GLFW_MOD_ALT_BIT     = 0x0004;
    if (mods & GLFW_MOD_CONTROL_BIT) km = km | KeyMod::Ctrl;
    if (mods & GLFW_MOD_SHIFT_BIT)   km = km | KeyMod::Shift;
    if (mods & GLFW_MOD_ALT_BIT)     km = km | KeyMod::Alt;
    m_keybinds->HandleKeyPress(key, km);
}

// ─────────────────────────────────────────────────────────────────
// Tool management
// ─────────────────────────────────────────────────────────────────

void EditorToolLayer::registerTool(std::unique_ptr<ITool> tool) {
    if (tool) {
        m_tools.push_back(std::move(tool));
    }
}

void EditorToolLayer::updateTools(float deltaTime) {
    for (auto& tool : m_tools) {
        if (tool->IsActive()) {
            tool->Update(deltaTime);
        }
    }
}

size_t EditorToolLayer::panelCount() const {
    return m_layout ? m_layout->Panels().size() : 0;
}

SceneBookmarkManager& EditorToolLayer::bookmarkManager() {
    return *m_bookmarkManager;
}

LayerTagSystem& EditorToolLayer::layerTagSystem() {
    return *m_layerTagSystem;
}

} // namespace atlas::editor

#endif // NOVAFORGE_EDITOR_TOOLS
