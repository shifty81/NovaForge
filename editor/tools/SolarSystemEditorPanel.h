#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_server/include/pcg/star_system_generator.h"
#include "../../cpp_server/include/pcg/pcg_manager.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * SolarSystemEditorPanel — Interactive solar system authoring tool.
 *
 * Designers can:
 *   - Generate a star system from a seed and security level.
 *   - Override the star class (spectral type O–M).
 *   - Add, remove, and reorder orbit slots (planets, belts, moons).
 *   - Place stations at specific orbit indices with faction tags.
 *   - Add/remove jump gates with destination IDs.
 *   - Adjust the security level and see how it affects station/gate counts.
 *   - Export the authored system to JSON for the universe data pipeline.
 *   - Re-roll the seed for a fresh procedural base, then tweak.
 *
 * The panel works in two modes:
 *   1. **Generate** — procedural baseline from StarSystemGenerator.
 *   2. **Edit** — manual tweaks on top of the generated system.
 *
 * Like GalaxyMapPanel, the Draw() call is a no-op when running headless
 * (no AtlasContext), making the panel fully testable without OpenGL.
 */
class SolarSystemEditorPanel : public EditorPanel {
public:
    SolarSystemEditorPanel();
    ~SolarSystemEditorPanel() override = default;

    const char* Name() const override { return "Solar System Editor"; }
    void Draw() override;

    // ── Generation ─────────────────────────────────────────────────

    /** Generate a new star system from the current seed and security. */
    void Generate();

    /** Set the generation seed. */
    void SetSeed(uint64_t seed);

    /** Set the security level [0.0, 1.0]. */
    void SetSecurityLevel(float sec);

    /** Override the star class for the next generation. -1 = auto. */
    void SetStarClassOverride(int starClass);

    /** Returns true when a system has been generated or loaded. */
    bool HasSystem() const { return m_system.valid; }

    // ── Orbit editing ──────────────────────────────────────────────

    /** Add an orbit slot at the end.  Returns the new orbit index. */
    int AddOrbitSlot(pcg::OrbitSlotType type, pcg::SystemPlanetType planet,
                     float radiusAU);

    /** Remove an orbit slot by index.  Returns false if out of range. */
    bool RemoveOrbitSlot(int orbitIndex);

    /** Total orbit slot count. */
    int OrbitSlotCount() const { return static_cast<int>(m_system.orbitSlots.size()); }

    // ── Station editing ────────────────────────────────────────────

    /** Add a station at the given orbit index with a faction tag. */
    bool AddStation(int orbitIndex, const std::string& faction);

    /** Remove a station by its station ID. */
    bool RemoveStation(uint64_t stationId);

    /** Total station count. */
    int StationCount() const { return static_cast<int>(m_system.stations.size()); }

    // ── Gate editing ───────────────────────────────────────────────

    /** Add a jump gate at the given orbit, connecting to a destination system. */
    bool AddGate(int orbitIndex, uint64_t destinationSystemId);

    /** Remove a gate by its gate ID. */
    bool RemoveGate(uint64_t gateId);

    /** Total gate count. */
    int GateCount() const { return static_cast<int>(m_system.gates.size()); }

    // ── Queries ────────────────────────────────────────────────────

    float SecurityLevel() const { return m_system.securityLevel; }
    int   TotalPlanets()  const { return m_system.totalPlanets; }
    int   TotalBelts()    const { return m_system.totalBelts; }

    const pcg::GeneratedStarSystem& System() const { return m_system; }
    const pcg::StarData& Star()              const { return m_system.star; }

    // ── Settings ───────────────────────────────────────────────────

    uint64_t GetSeed()          const { return m_seed; }
    float    GetSecurityLevel() const { return m_securityLevel; }
    int      GetStarClassOverride() const { return m_starClassOverride; }

    // ── Export ──────────────────────────────────────────────────────

    /** Export the current system as a JSON string for the universe pipeline. */
    std::string ExportJSON() const;

    // ── Log ────────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    void log(const std::string& msg);
    void recountBodies();
    uint64_t nextId();

    pcg::PCGManager           m_mgr;
    pcg::GeneratedStarSystem  m_system{};

    uint64_t m_seed            = 42;
    float    m_securityLevel   = 0.8f;
    int      m_starClassOverride = -1;   // -1 = auto
    uint64_t m_nextId          = 10000;

    std::vector<std::string>  m_log;

    float m_scrollOffset = 0.0f;
    atlas::PanelState m_panelState;
};

} // namespace atlas::editor
