#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_server/include/pcg/ship_archetype.h"
#include "../../cpp_server/include/pcg/pcg_manager.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * @brief Editor panel for authoring per-class ship archetypes.
 *
 * This is the primary design tool for defining how each hull class
 * looks, what its interior layout is, where doors and hardpoints go,
 * and what subsystem slots are available.  PCG generates all in-game
 * ship variations from these archetypes.
 *
 * Designer workflow:
 *   1. Select a hull class (Frigate → Titan).
 *   2. Edit the hull shape profile (control points on the spine).
 *   3. Place interior rooms and doors.
 *   4. Define turret hardpoints (position, size, arc).
 *   5. Configure subsystem slots and their visual variants.
 *   6. Set PCG variation bounds (how much seeded variation is allowed).
 *   7. Preview — generate a variant to see the result.
 *   8. Save the archetype for the game to use.
 *
 * In-game, every ship is generated from its archetype.  When the
 * player fits modules, the ship's appearance morphs (like Astralis T3
 * ships) via applySubsystems() and applyModuleVisuals().
 */
class ShipArchetypePanel : public EditorPanel {
public:
    ShipArchetypePanel();

    const char* Name() const override { return "Ship Archetype"; }
    void Draw() override;

    // ── Hull class selection ────────────────────────────────────────

    /// Select a hull class and load its default archetype.
    void SelectHullClass(pcg::HullClass hull);

    /// Get the current hull class being edited.
    pcg::HullClass GetHullClass() const { return m_archetype.hullClass; }

    // ── Direct archetype access ─────────────────────────────────────

    const pcg::ShipArchetype& GetArchetype() const { return m_archetype; }
    void SetArchetype(const pcg::ShipArchetype& arch) { m_archetype = arch; }

    // ── Hull shape editing ──────────────────────────────────────────

    /// Add a shape control point to the hull profile.
    void AddHullControlPoint(const pcg::ShapeControlPoint& cp);

    /// Remove a hull control point by index.
    bool RemoveHullControlPoint(size_t index);

    /// Update a hull control point by index.
    bool UpdateHullControlPoint(size_t index,
                                const pcg::ShapeControlPoint& cp);

    size_t HullControlPointCount() const {
        return m_archetype.hullShape.controlPoints.size();
    }

    // ── Interior room editing ───────────────────────────────────────

    /// Add an interior room to the archetype.
    void AddRoom(const pcg::InteriorRoom& room);

    /// Remove a room by its roomId.
    bool RemoveRoom(int roomId);

    size_t RoomCount() const { return m_archetype.rooms.size(); }

    // ── Door placement ──────────────────────────────────────────────

    /// Add a door connecting two rooms.
    void AddDoor(const pcg::DoorPlacement& door);

    /// Remove a door by its doorId.
    bool RemoveDoor(uint32_t doorId);

    size_t DoorCount() const { return m_archetype.doors.size(); }

    // ── Hardpoint editing ───────────────────────────────────────────

    /// Add a turret/launcher hardpoint.
    void AddHardpoint(const pcg::HardpointDefinition& hp);

    /// Remove a hardpoint by its ID.
    bool RemoveHardpoint(uint32_t hardpointId);

    /// Update a hardpoint by its ID.
    bool UpdateHardpoint(uint32_t hardpointId,
                         const pcg::HardpointDefinition& hp);

    size_t HardpointCount() const { return m_archetype.hardpoints.size(); }

    // ── Subsystem slot editing ──────────────────────────────────────

    /// Add a subsystem slot.
    void AddSubsystemSlot(const pcg::SubsystemSlot& slot);

    /// Add a variant to an existing subsystem slot (by slot index).
    bool AddSubsystemVariant(size_t slotIndex,
                             const pcg::SubsystemVariant& variant);

    /// Set which variant is active for a slot (for preview).
    bool SetActiveVariant(size_t slotIndex, int variantIndex);

    size_t SubsystemSlotCount() const {
        return m_archetype.subsystems.size();
    }

    // ── Module visual rules ─────────────────────────────────────────

    /// Add a module visual rule.
    void AddModuleVisualRule(const pcg::ModuleVisualRule& rule);

    /// Remove a module visual rule by index.
    bool RemoveModuleVisualRule(size_t index);

    size_t ModuleVisualRuleCount() const {
        return m_archetype.moduleVisuals.size();
    }

    // ── Variation bounds ────────────────────────────────────────────

    /// Set the PCG variation bounds.
    void SetVariationBounds(float shape, float size, float detail);

    // ── Preview generation ──────────────────────────────────────────

    /// Generate a preview variant from the current archetype.
    void GeneratePreview();

    /// Generate with a specific subsystem configuration.
    void GeneratePreviewWithSubsystems(const std::vector<int>& activeVariants);

    /// Generate with specific module fittings.
    void GeneratePreviewWithModules(
        const std::vector<std::string>& fittedModules);

    /// Get the last generated preview.
    const pcg::ArchetypeVariant& GetPreview() const { return m_preview; }
    bool HasPreview() const { return m_hasPreview; }

    // ── Serialisation ───────────────────────────────────────────────

    std::string SaveToString() const;
    void LoadFromString(const std::string& data);

    bool SaveToFile(const std::string& path = "data/ship_archetype.json");
    bool LoadFromFile(const std::string& path = "data/ship_archetype.json");

    // ── Log ─────────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    pcg::ShipArchetype      m_archetype;
    pcg::PCGManager         m_pcgManager;
    pcg::ArchetypeVariant   m_preview;
    bool                    m_hasPreview = false;
    std::vector<std::string> m_log;

    atlas::PanelState m_panelState;
    bool m_hullDropdownOpen = false;
    float m_scrollOffset = 0.0f;
    int m_selectedRoomIndex = -1;
    int m_selectedHardpointIndex = -1;
    int m_selectedSubsystemIndex = -1;

    void log(const std::string& msg);
};

}
