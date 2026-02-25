#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_server/include/pcg/galaxy_generator.h"
#include "../../cpp_server/include/pcg/pcg_manager.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * GalaxyMapPanel — In-editor visualization and inspection of the procedural
 * galaxy graph.
 *
 * Designers can:
 *   - Generate a galaxy with a configurable system count and seed.
 *   - Filter the displayed nodes by security zone (High-Sec / Low-Sec /
 *     Null-Sec) or show all zones at once.
 *   - Inspect an individual system (neighbours, security level,
 *     constellation, region).
 *   - Re-roll the seed or fix a chosen seed for deterministic results.
 *   - View aggregate statistics (total systems, zone distribution,
 *     chokepoint count).
 *
 * The panel does not perform OpenGL rendering — it stores the galaxy data
 * and exposes it through accessor methods that the calling viewport or
 * 2-D overlay can use to draw the map.  In the test suite the Draw() call
 * is a no-op that merely validates state consistency.
 */
class GalaxyMapPanel : public EditorPanel {
public:
    GalaxyMapPanel();
    ~GalaxyMapPanel() override = default;

    const char* Name() const override { return "Galaxy Map"; }
    void Draw() override;

    // ── Generation ─────────────────────────────────────────────────

    /** Generate a new galaxy with the current settings. */
    void Generate();

    /** Change the generation seed and regenerate. */
    void SetSeed(uint64_t seed);

    /** Set the desired number of star systems (clamped to [10, 500]). */
    void SetSystemCount(int count);

    /** Returns true when a galaxy has been generated. */
    bool HasGalaxy() const { return m_galaxy.valid; }

    // ── Filtering / selection ──────────────────────────────────────

    /** Security-zone filter: 0 = All, 1 = HighSec, 2 = LowSec, 3 = NullSec. */
    void SetSecurityFilter(int filter);
    int  GetSecurityFilter() const { return m_securityFilter; }

    /** Select a system by its system_id.  Clears selection if id == 0 or
     *  the id is not present in the current galaxy. */
    void SelectSystem(uint64_t systemId);

    /** Deselect the currently inspected system. */
    void ClearSelection();

    /** Returns the system_id of the currently selected system (0 = none). */
    uint64_t SelectedSystemId() const { return m_selectedSystemId; }

    /** Returns a pointer to the selected GalaxyNode, or nullptr if none. */
    const pcg::GalaxyNode* GetSelectedNode() const;

    // ── Statistics ─────────────────────────────────────────────────

    /** Total number of generated systems (0 when no galaxy is present). */
    int TotalSystems()  const { return m_galaxy.total_systems; }
    int HighSecCount()  const { return m_galaxy.highsec_count; }
    int LowSecCount()   const { return m_galaxy.lowsec_count; }
    int NullSecCount()  const { return m_galaxy.nullsec_count; }

    /** Number of routes marked as chokepoints. */
    int ChokepointCount() const;

    // ── Filtered view ──────────────────────────────────────────────

    /** Returns a view (indices into m_galaxy.nodes) of systems that pass
     *  the current security filter.  Re-built on every call to Generate()
     *  or SetSecurityFilter(). */
    const std::vector<size_t>& FilteredNodeIndices() const { return m_filtered; }

    // ── Raw data access ────────────────────────────────────────────

    const pcg::GeneratedGalaxy& Galaxy() const { return m_galaxy; }

    // ── Settings ───────────────────────────────────────────────────

    uint64_t GetSeed()        const { return m_seed; }
    int      GetSystemCount() const { return m_systemCount; }

    // ── Log ────────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    void rebuildFilter();
    void log(const std::string& msg);

    pcg::PCGManager        m_mgr;
    pcg::GeneratedGalaxy   m_galaxy{};

    uint64_t m_seed        = 42;
    int      m_systemCount = 100;
    int      m_securityFilter = 0;   // 0 = All
    uint64_t m_selectedSystemId = 0;

    std::vector<size_t>      m_filtered;
    std::vector<std::string> m_log;

    float m_scrollOffset = 0.0f;
    atlas::PanelState m_panelState;
};

} // namespace atlas::editor
