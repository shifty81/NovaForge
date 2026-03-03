#pragma once
/**
 * @file editor_tool_layer.h
 * @brief Optional editor overlay that can be embedded in the game client.
 *
 * When the client is built with -DNOVAFORGE_EDITOR_TOOLS=ON the
 * EditorToolLayer provides all editor panels (viewport, PCG preview,
 * ship archetype, etc.) as a toggleable overlay on top of the running
 * game.  Pressing F12 activates the layer; pressing F12 again hides it.
 *
 * For release builds, simply turn the flag OFF — the tooling layer,
 * its headers, and every ITool implementation are compiled out entirely,
 * leaving zero overhead in the shipping client.
 *
 * Architecture (from editornewfeat.md):
 *   - ITool interface for individual tools
 *   - EditorCommandBus for decoupled command execution
 *   - EditorToolLayer manages panels, layout, keybinds, undo stack
 */

#ifdef NOVAFORGE_EDITOR_TOOLS

#include <memory>
#include <vector>
#include <string>
#include "editor/itool.h"
#include "editor/editor_command_bus.h"

namespace atlas {
class AtlasContext;
struct InputState;
} // namespace atlas

namespace atlas::editor {

// Forward declarations — keeps this header lightweight
class EditorLayout;
class EditorMenuBar;
class KeybindManager;
class UndoStack;
class ViewportPanel;
class PCGPreviewPanel;
class GenerationStylePanel;
class AssetStylePanel;
class ShipArchetypePanel;
class GamePackagerPanel;
class CharacterSelectPanel;
class MissionEditorPanel;
class SceneGraphPanel;
class DataBrowserPanel;
class ModuleEditorPanel;
class NPCEditorPanel;
class GalaxyMapPanel;
class FleetFormationPanel;
class LiveSceneManager;
class PCGOverrideStore;
class AssetPalettePanel;
class PhysicsTunerPanel;

namespace ai { class AIAggregator; class TemplateAIBackend; }

/**
 * Editor overlay that lives inside the game client.
 *
 * Lifecycle:
 *   1. Construct (lightweight — no panels yet).
 *   2. init()   — create all panels, layout, keybinds.
 *   3. Per frame while active:
 *        handleKeyPress() — forward key events
 *        draw()           — render the editor overlay
 *   4. shutdown() — tear down panels.
 */
class EditorToolLayer {
public:
    EditorToolLayer();
    ~EditorToolLayer();

    // Non-copyable
    EditorToolLayer(const EditorToolLayer&) = delete;
    EditorToolLayer& operator=(const EditorToolLayer&) = delete;

    // ── Lifecycle ───────────────────────────────────────────────────

    /** Create all editor panels and wire up the dock layout. */
    void init();

    /** Tear down all editor panels. */
    void shutdown();

    // ── Per-frame ───────────────────────────────────────────────────

    /**
     * Render the editor overlay.
     * Call between AtlasContext::beginFrame / endFrame.
     */
    void draw(atlas::AtlasContext& ctx);

    /** Forward a key press from the client input system. */
    void handleKeyPress(int key, int mods);

    // ── Activation ──────────────────────────────────────────────────

    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }
    void toggle() { m_active = !m_active; }

    // ── Tool management ─────────────────────────────────────────────

    /** Register a custom ITool with the layer. */
    void registerTool(std::unique_ptr<ITool> tool);

    /** Update all active tools (call once per frame). */
    void updateTools(float deltaTime);

    /** Access the command bus for posting deferred commands. */
    EditorCommandBus& commandBus() { return m_commandBus; }

    /** Number of panels registered in the layout. */
    size_t panelCount() const;

private:
    bool m_active = false;
    bool m_initialized = false;

    // ── Sub-systems ─────────────────────────────────────────────────
    EditorCommandBus m_commandBus;
    std::vector<std::unique_ptr<ITool>> m_tools;

    // ── Editor infrastructure (pimpl-style to keep header light) ────
    std::unique_ptr<EditorLayout>      m_layout;
    std::unique_ptr<KeybindManager>    m_keybinds;
    std::unique_ptr<UndoStack>         m_undoStack;

    // ── AI ──────────────────────────────────────────────────────────
    std::unique_ptr<ai::AIAggregator>      m_aiAggregator;
    std::unique_ptr<ai::TemplateAIBackend> m_templateAI;

    // ── Panels (owned here, registered with layout) ─────────────────
    std::unique_ptr<ViewportPanel>          m_viewport;
    std::unique_ptr<PCGPreviewPanel>        m_pcgPreview;
    std::unique_ptr<GenerationStylePanel>   m_genStyle;
    std::unique_ptr<AssetStylePanel>        m_assetStyle;
    std::unique_ptr<ShipArchetypePanel>     m_shipArchetype;
    std::unique_ptr<GamePackagerPanel>      m_packager;
    std::unique_ptr<CharacterSelectPanel>   m_characterSelect;
    std::unique_ptr<MissionEditorPanel>     m_missionEditor;
    std::unique_ptr<SceneGraphPanel>        m_sceneGraph;
    std::unique_ptr<DataBrowserPanel>       m_dataBrowser;
    std::unique_ptr<ModuleEditorPanel>      m_moduleEditor;
    std::unique_ptr<NPCEditorPanel>         m_npcEditor;
    std::unique_ptr<GalaxyMapPanel>         m_galaxyMap;
    std::unique_ptr<FleetFormationPanel>    m_fleetFormation;
    std::unique_ptr<AssetPalettePanel>      m_assetPalette;
    std::unique_ptr<PhysicsTunerPanel>      m_physicsTuner;

    // ── Live scene bridge ───────────────────────────────────────────
    std::unique_ptr<LiveSceneManager> m_liveScene;
};

} // namespace atlas::editor

#endif // NOVAFORGE_EDITOR_TOOLS
