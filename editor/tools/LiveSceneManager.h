#pragma once
#include "../ui/EditorPanel.h"
#include "ViewportPanel.h"
#include "PCGPreviewPanel.h"
#include "PCGOverrideStore.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * LiveSceneManager — bridges PCG generation and the viewport for live editing.
 *
 * This panel is the heart of the in-engine content-creation workflow:
 *
 *   1. **Populate** — generates ships, stations, interiors via PCG and
 *      loads them into the viewport so the designer sees a full game scene.
 *   2. **Live Edit** — the designer clicks and drags objects in the
 *      viewport.  After each manipulation the manager captures the delta
 *      and can optionally trigger a PCG re-generation to show how the
 *      change affects downstream content.
 *   3. **Save** — writes all manual overrides to a JSON file
 *      (`data/pcg_overrides.json`) that the game client reads at startup.
 *      No project rebuild is required.
 *   4. **Load** — reads an existing override file so the editor starts
 *      where the designer left off.
 *
 * Typical session:
 * @code
 *   LiveSceneManager liveScene(&viewport, &pcgPreview);
 *   liveScene.PopulateDefaultScene();          // generate & show
 *   // … designer drags objects in viewport …
 *   liveScene.CaptureViewportChanges();        // snapshot deltas
 *   liveScene.SaveOverrides("data/pcg_overrides.json");
 *   // open client → overrides are applied automatically
 * @endcode
 */
class LiveSceneManager : public EditorPanel {
public:
    LiveSceneManager(ViewportPanel* viewport, PCGPreviewPanel* pcgPreview);

    const char* Name() const override { return "Live Scene"; }
    void Draw() override;

    /** Called when an asset changes on disk (live-reload). */
    void OnAssetReloaded(const std::string& assetId,
                         const std::string& path) override;

    // ── Scene population ─────────────────────────────────────────

    /** Generate a default game scene (ship + station + interior) and
     *  load everything into the viewport. */
    void PopulateDefaultScene();

    /** Regenerate the current scene with the active PCG settings.
     *  Existing manual overrides are re-applied after regeneration. */
    void RegenerateScene();

    // ── Live editing ─────────────────────────────────────────────

    /** Snapshot any pending viewport changes into the override store.
     *  Call this after the user finishes a drag operation. */
    void CaptureViewportChanges();

    /** Returns true if the scene has been populated. */
    bool IsPopulated() const { return m_populated; }

    /** Returns true if there are unsaved changes. */
    bool HasUnsavedChanges() const { return m_overrides.IsDirty(); }

    // ── Persistence ──────────────────────────────────────────────

    /** Save all overrides to a JSON file.  Default path is
     *  `data/pcg_overrides.json`. */
    bool SaveOverrides(const std::string& path = "data/pcg_overrides.json");

    /** Load overrides from a JSON file.  Applies them to the current
     *  viewport objects and regenerates if needed. */
    bool LoadOverrides(const std::string& path = "data/pcg_overrides.json");

    // ── Accessors ────────────────────────────────────────────────

    const PCGOverrideStore& OverrideStore() const { return m_overrides; }
    const std::vector<std::string>& Log() const { return m_log; }

    uint64_t CurrentSeed()    const { return m_seed; }
    uint32_t CurrentVersion() const { return m_version; }

    void SetSeed(uint64_t seed)     { m_seed = seed; }
    void SetVersion(uint32_t ver)   { m_version = ver; }

    // ── Snapshot / Rollback ──────────────────────────────────────

    /** Capture the current scene overrides as a named snapshot.
     *  Returns the snapshot index (0-based). */
    size_t TakeSnapshot(const std::string& label = "");

    /** Rollback to a previously captured snapshot by index.
     *  Returns true if the rollback succeeded. */
    bool RollbackToSnapshot(size_t index);

    /** Number of snapshots currently stored. */
    size_t SnapshotCount() const { return m_snapshots.size(); }

    /** Label of a stored snapshot.
     *  Returns an empty string if index is out of bounds. */
    const std::string& SnapshotLabel(size_t index) const {
        static const std::string kEmpty;
        if (index >= m_snapshots.size()) return kEmpty;
        return m_snapshots[index].label;
    }

private:
    ViewportPanel*    m_viewport;
    PCGPreviewPanel*  m_pcgPreview;
    PCGOverrideStore  m_overrides;

    bool     m_populated = false;
    uint64_t m_seed      = 42;
    uint32_t m_version   = 1;

    /** A saved scene state for rollback. */
    struct Snapshot {
        std::string    label;
        PCGOverrideStore overrides;
        uint64_t seed    = 0;
        uint32_t version = 0;
    };
    std::vector<Snapshot> m_snapshots;

    std::vector<std::string> m_log;

    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;
    atlas::TextInputState m_snapshotLabelInput;

    /** Apply stored overrides to matching viewport objects. */
    void applyOverridesToViewport();
    void log(const std::string& msg);
};

} // namespace atlas::editor
