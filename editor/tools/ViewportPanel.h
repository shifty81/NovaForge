#pragma once
#include "../ui/EditorPanel.h"
#include "PCGPreviewPanel.h"
#include "../../cpp_server/include/pcg/pcg_manager.h"
#include "../../cpp_server/include/pcg/ship_generator.h"
#include "../../cpp_server/include/pcg/station_generator.h"
#include "../../cpp_server/include/pcg/lowpoly_character_generator.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <vector>
#include <string>
#include <cmath>

namespace atlas::editor {

/**
 * Transform gizmo mode for viewport manipulation.
 */
enum class GizmoMode {
    None,
    Translate,
    Rotate,
    Scale
};

/**
 * Represents a 3D transform (position, rotation, scale)
 * used by viewport objects for visual manipulation.
 */
struct ViewportTransform {
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;  // degrees
    float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;
};

/**
 * A single selectable object displayed in the viewport.
 * Wraps a PCG-generated entity with a transform for manipulation.
 */
struct ViewportObject {
    uint32_t id = 0;
    std::string name;
    std::string type;   // "Ship", "Station", "Module", "Room"
    ViewportTransform transform;
    bool selected = false;
    bool visible = true;
};

/**
 * Records a permanent change made via viewport manipulation
 * that should be written back to PCG parameters.
 */
struct ViewportChange {
    uint32_t objectId = 0;
    std::string field;      // e.g. "position", "rotation", "scale"
    float oldValues[3] = {};
    float newValues[3] = {};
};

/**
 * ViewportPanel — Visual 3D preview and manipulation for PCG content.
 *
 * This panel provides:
 * - Visual representation of PCG-generated ships, stations, interiors
 * - Transform gizmo for translate/rotate/scale manipulation
 * - Selection of individual components (modules, rooms, hardpoints)
 * - Saving viewport changes back to PCG parameters permanently
 * - Orbit camera with zoom/pan for navigation
 *
 * The viewport acts as the bridge between the PCG generation system
 * and the designer's visual authoring workflow.
 */
class ViewportPanel : public EditorPanel {
public:
    ViewportPanel();
    ~ViewportPanel() override = default;

    const char* Name() const override { return "Viewport"; }
    void Draw() override;

    // ── Scene management ──────────────────────────────────────

    /** Load a generated ship into the viewport for manipulation. */
    void LoadShip(const pcg::GeneratedShip& ship, uint64_t seed);

    /** Load a generated station into the viewport for manipulation. */
    void LoadStation(const pcg::GeneratedStation& station, uint64_t seed);

    /** Load a spine hull with optional turret placement into the viewport. */
    void LoadSpineHull(const pcg::GeneratedSpineHull& hull,
                       const pcg::TurretPlacement* placement,
                       uint64_t seed);

    /** Load a generated low-poly character into the viewport for inspection
     *  and placement.  Creates one object per body part and one per clothing
     *  item so each piece can be individually selected and transformed. */
    void LoadCharacter(const pcg::GeneratedLowPolyCharacter& character,
                       uint64_t seed);

    /** Clear all objects from the viewport. */
    void ClearScene();

    /** Get the number of objects in the viewport. */
    size_t ObjectCount() const { return m_objects.size(); }

    /** Get a viewport object by index. */
    const ViewportObject& GetObject(size_t index) const { return m_objects[index]; }

    // ── Selection ─────────────────────────────────────────────

    /** Select an object by its ID. */
    void SelectObject(uint32_t id);

    /** Deselect all objects. */
    void DeselectAll();

    /** Get the currently selected object ID (0 = none). */
    uint32_t SelectedObjectId() const { return m_selectedId; }

    // ── Transform gizmo ───────────────────────────────────────

    /** Set the active gizmo mode. */
    void SetGizmoMode(GizmoMode mode) { m_gizmoMode = mode; }

    /** Get the active gizmo mode. */
    GizmoMode GetGizmoMode() const { return m_gizmoMode; }

    /** Apply a translation delta to the selected object. */
    void TranslateSelected(float dx, float dy, float dz);

    /** Apply a rotation delta (degrees) to the selected object. */
    void RotateSelected(float dx, float dy, float dz);

    /** Apply a scale delta to the selected object. */
    void ScaleSelected(float dx, float dy, float dz);

    /** Get the transform of a specific object. */
    const ViewportTransform& GetTransform(uint32_t id) const;

    // ── Camera ────────────────────────────────────────────────

    /** Set camera orbit distance. */
    void SetCameraDistance(float dist) { m_cameraDistance = dist; }

    /** Get camera orbit distance. */
    float GetCameraDistance() const { return m_cameraDistance; }

    /** Orbit camera by delta yaw/pitch. */
    void OrbitCamera(float deltaYaw, float deltaPitch);

    /** Get camera yaw. */
    float GetCameraYaw() const { return m_cameraYaw; }

    /** Get camera pitch. */
    float GetCameraPitch() const { return m_cameraPitch; }

    // ── Change tracking ───────────────────────────────────────

    /** Get the list of uncommitted changes. */
    const std::vector<ViewportChange>& PendingChanges() const { return m_pendingChanges; }

    /** Commit all pending changes and return them for PCG parameter update. */
    std::vector<ViewportChange> CommitChanges();

    /** Discard all pending changes and revert transforms. */
    void DiscardChanges();

    /** Check if there are unsaved viewport changes. */
    bool HasPendingChanges() const { return !m_pendingChanges.empty(); }

    // ── Grid / helpers ────────────────────────────────────────

    /** Toggle grid visibility. */
    void SetGridVisible(bool v) { m_gridVisible = v; }
    bool IsGridVisible() const { return m_gridVisible; }

    /** Get the viewport log (for debug/status display). */
    const std::vector<std::string>& Log() const { return m_log; }

    // ── Live Reload ───────────────────────────────────────────
    /** Called when an asset changes on disk.  Logs the event so the
     *  designer sees that the viewport is aware of file changes. */
    void OnAssetReloaded(const std::string& assetId,
                         const std::string& path) override;

private:
    ViewportObject* findObject(uint32_t id);
    const ViewportObject* findObject(uint32_t id) const;
    void recordChange(uint32_t id, const std::string& field,
                      const float old3[3], const float new3[3]);

    std::vector<ViewportObject> m_objects;
    uint32_t m_nextId = 1;
    uint32_t m_selectedId = 0;

    GizmoMode m_gizmoMode = GizmoMode::Translate;

    // Camera state
    float m_cameraDistance = 500.0f;
    float m_cameraYaw = 0.0f;
    float m_cameraPitch = 30.0f;

    bool m_gridVisible = true;

    // Change tracking
    std::vector<ViewportChange> m_pendingChanges;
    std::vector<std::string> m_log;

    // Original transforms for revert
    std::vector<std::pair<uint32_t, ViewportTransform>> m_originalTransforms;

    static const ViewportTransform s_defaultTransform;

    atlas::PanelState m_viewportPanelState;
};

} // namespace atlas::editor
