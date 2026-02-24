#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_server/include/pcg/pcg_context.h"
#include "../../cpp_server/include/pcg/pcg_manager.h"
#include "../../cpp_server/include/pcg/ship_generator.h"
#include "../../cpp_server/include/pcg/station_generator.h"
#include "../../cpp_server/include/pcg/interior_generator.h"
#include "../../cpp_server/include/pcg/lowpoly_character_generator.h"
#include "../../cpp_server/include/pcg/spine_hull_generator.h"
#include "../../cpp_server/include/pcg/turret_placement_system.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/// Generator domain the user can select in the panel.
enum class PCGPreviewMode : int {
    Ship            = 0,
    Station         = 1,
    Interior        = 2,
    Character       = 3,
    SpineHull       = 4,
    TurretPlacement = 5,
};

/// Editable parameters exposed through the panel UI.
struct PCGPreviewSettings {
    PCGPreviewMode mode         = PCGPreviewMode::Ship;
    uint64_t       seed         = 42;
    uint32_t       version      = 1;

    // Ship-specific
    pcg::HullClass hullClass    = pcg::HullClass::Frigate;
    bool           overrideHull = false;

    // Station-specific
    int            moduleCount  = 0;  ///< 0 = let generator decide
    bool           overrideModuleCount = false;

    // Interior-specific
    int            shipClass    = 0;  ///< 0=Frigate .. 5=Capital

    // Character-specific
    pcg::CharacterArchetype characterArchetype = pcg::CharacterArchetype::Survivor;
    bool           overrideArchetype = false;
    bool           characterIsMale   = true;
    bool           overrideGender    = false;

    // SpineHull-specific
    std::string    faction;             ///< "" = no faction style
    bool           overrideFaction = false;

    // TurretPlacement-specific
    int            turretSlots     = 0; ///< 0 = derive from ship generator
    bool           overrideTurretSlots = false;
};

/// Snapshot of a generated ship shown in the preview area.
struct ShipPreview {
    pcg::GeneratedShip data{};
    bool               populated = false;
};

/// Snapshot of a generated station shown in the preview area.
struct StationPreview {
    pcg::GeneratedStation data{};
    bool                  populated = false;
};

/// Snapshot of a generated interior shown in the preview area.
struct InteriorPreview {
    pcg::GeneratedInterior data{};
    bool                   populated = false;
};

/// Snapshot of a generated low-poly character shown in the preview area.
struct CharacterPreview {
    pcg::GeneratedLowPolyCharacter data{};
    bool                           populated = false;
};

/// Snapshot of a generated spine hull shown in the preview area.
struct SpineHullPreview {
    pcg::GeneratedSpineHull data{};
    bool                    populated = false;
};

/// Snapshot of a turret placement shown in the preview area.
struct TurretPlacementPreview {
    pcg::TurretPlacement data{};
    pcg::GeneratedSpineHull hull{};  ///< Spine hull used as placement surface.
    bool                 populated = false;
};

/**
 * @brief Editor panel for visually generating and manipulating PCG items.
 *
 * Gives designers a way to explore the procedural content generation
 * pipeline outside of the running game.  Supports ships, stations, and
 * ship interiors with full parameter control and deterministic output.
 */
class PCGPreviewPanel : public EditorPanel {
public:
    PCGPreviewPanel();

    const char* Name() const override { return "PCG Preview"; }
    void Draw() override;

    // ── Public API for programmatic / test access ────────────────────

    /// Read / write settings.
    const PCGPreviewSettings& Settings() const { return m_settings; }
    void SetSettings(const PCGPreviewSettings& s) { m_settings = s; }

    /// Trigger generation with the current settings.
    void Generate();

    /// Randomize the seed and regenerate.
    void Randomize();

    /// Clear all preview data.
    void ClearPreview();

    // ── Accessors for generated previews ─────────────────────────────

    const ShipPreview&             GetShipPreview()             const { return m_shipPreview; }
    const StationPreview&          GetStationPreview()          const { return m_stationPreview; }
    const InteriorPreview&         GetInteriorPreview()         const { return m_interiorPreview; }
    const CharacterPreview&        GetCharacterPreview()        const { return m_characterPreview; }
    const SpineHullPreview&        GetSpineHullPreview()        const { return m_spineHullPreview; }
    const TurretPlacementPreview&  GetTurretPlacementPreview()  const { return m_turretPlacementPreview; }

    /// Status / log lines produced during the last generation.
    const std::vector<std::string>& Log() const { return m_log; }

private:
    PCGPreviewSettings m_settings;
    pcg::PCGManager    m_pcgManager;

    ShipPreview             m_shipPreview;
    StationPreview          m_stationPreview;
    InteriorPreview         m_interiorPreview;
    CharacterPreview        m_characterPreview;
    SpineHullPreview        m_spineHullPreview;
    TurretPlacementPreview  m_turretPlacementPreview;

    std::vector<std::string> m_log;

    atlas::PanelState m_panelState;
    bool m_modeDropdownOpen = false;
    float m_scrollOffset = 0.0f;

    void generateShip();
    void generateStation();
    void generateInterior();
    void generateCharacter();
    void generateSpineHull();
    void generateTurretPlacement();
    void log(const std::string& msg);
};

}
