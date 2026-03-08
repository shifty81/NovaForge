#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_server/include/pcg/generation_style.h"
#include "../../cpp_server/include/components/fleet_components.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <vector>
#include <string>
#include <cstdint>

namespace atlas::editor {

/**
 * Represents a single ship slot within the formation editor.
 */
struct FormationSlot {
    int         slotIndex = 0;
    std::string shipName;
    std::string role;       ///< "dps", "logistics", "ewar", "tackle", "command"
    pcg::HullClass hullClass = pcg::HullClass::Frigate;
    float       offsetX = 0.0f;   ///< metres from commander
    float       offsetY = 0.0f;
    float       offsetZ = 0.0f;
    float       spacingModifier = 1.0f;
};

/**
 * @brief FleetFormationPanel — editor panel for visualizing and editing
 *        fleet formation layouts.
 *
 * Allows designers to:
 *   - Choose from the standard formation types (Arrow, Line, Wedge, etc.)
 *   - Configure fleet size and spacing
 *   - See computed offsets for every slot
 *   - Import a fleet composition from the PCG generation pipeline
 *   - Export formation data for runtime use
 *
 * This panel is headless-safe: Draw() is a no-op without a UI context.
 */
class FleetFormationPanel : public EditorPanel {
public:
    FleetFormationPanel();
    ~FleetFormationPanel() override = default;

    const char* Name() const override { return "Fleet Formation"; }
    void Draw() override;

    // ── Formation type ────────────────────────────────────────────

    using FormationType = atlas::components::FleetFormation::FormationType;

    void SetFormationType(FormationType type);
    FormationType GetFormationType() const { return m_formationType; }

    static const char* FormationTypeName(FormationType type);
    static constexpr int kFormationTypeCount = 6; // None..Diamond

    // ── Fleet size / spacing ──────────────────────────────────────

    void SetFleetSize(int size);
    int  GetFleetSize() const { return m_fleetSize; }

    void  SetSpacing(float metres);
    float GetSpacing() const { return m_spacing; }

    // ── Slot access ───────────────────────────────────────────────

    /** Recompute all slot offsets for the current formation type. */
    void ComputeOffsets();

    /** Number of populated slots. */
    size_t SlotCount() const { return m_slots.size(); }

    /** Access a slot by index. */
    const FormationSlot& GetSlot(size_t index) const { return m_slots[index]; }

    /** Read-only access to all slots. */
    const std::vector<FormationSlot>& Slots() const { return m_slots; }

    // ── Import from PCG pipeline ─────────────────────────────────

    /** Populate the panel from a generated fleet composition. */
    void ImportFleet(const pcg::GeneratedFleetCompositionResult& fleet);

    // ── Selection ────────────────────────────────────────────────

    void SelectSlot(int slotIndex);
    void ClearSelection();
    int  SelectedSlot() const { return m_selectedSlot; }

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

    // ── Aggregate stats ──────────────────────────────────────────

    /** Bounding radius of the formation (max distance from commander). */
    float BoundingRadius() const;

    /** Centre-of-mass offset from the commander position. */
    void  CentreOfMass(float& cx, float& cy, float& cz) const;

private:
    void rebuildSlots();
    void log(const std::string& msg);

    // Arrow / Line / Wedge / Spread / Diamond offset computation
    void computeArrow(FormationSlot& slot) const;
    void computeLine(FormationSlot& slot) const;
    void computeWedge(FormationSlot& slot) const;
    void computeSpread(FormationSlot& slot) const;
    void computeDiamond(FormationSlot& slot) const;

    FormationType m_formationType = FormationType::Arrow;
    int           m_fleetSize     = 5;
    float         m_spacing       = 500.0f;

    std::vector<FormationSlot> m_slots;
    int m_selectedSlot = -1;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;
};

} // namespace atlas::editor
