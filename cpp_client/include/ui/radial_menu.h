#ifndef RADIAL_MENU_H
#define RADIAL_MENU_H

#include <string>
#include <functional>
#include <vector>
#include <cmath>

namespace atlas {
    class AtlasContext;
}

namespace UI {

/**
 * Astralis-style radial menu for in-space and FPS interaction.
 *
 * **Space mode** — Activated by holding left mouse button on an entity in
 * space.  Shows: Orbit, Approach, Warp To, Lock, Keep at Range, Look At,
 * Align, and Show Info.
 *
 * **FPS mode** — Activated by holding left mouse button while looking at
 * an interactable object inside a ship or station.  The menu adapts to
 * the type of object: doors, terminals, loot containers, fabricators,
 * medical bays, and airlocks each present their own relevant options.
 *
 * In both modes the player moves the mouse toward a segment to highlight
 * it, then releases to select.  Releasing in the dead zone cancels.
 */
class RadialMenu {
public:
    // ── Space-mode actions ─────────────────────────────────────────
    enum class Action {
        NONE = 0,
        ORBIT,
        APPROACH,
        WARP_TO,
        LOCK_TARGET,
        KEEP_AT_RANGE,
        LOOK_AT,
        SHOW_INFO,
        ALIGN_TO,

        // ── FPS-mode actions ───────────────────────────────────────
        FPS_USE,             // Generic "use" / activate
        FPS_OPEN,            // Open a door / container
        FPS_CLOSE,           // Close a door
        FPS_LOCK,            // Lock a door
        FPS_UNLOCK,          // Unlock a door
        FPS_HACK,            // Hack a terminal / security door
        FPS_LOOT_ALL,        // Loot all items from a container
        FPS_SEARCH,          // Search / examine an object
        FPS_REPAIR,          // Repair a system / hazard
        FPS_HEAL,            // Use medical bay for healing
        FPS_RESTOCK,         // Restock oxygen / supplies
        FPS_CRAFT,           // Begin crafting at fabricator
        FPS_EVA_BEGIN,       // Begin EVA sequence (airlock)
        FPS_EVA_ABORT,       // Abort EVA sequence
        FPS_ACCESS_TERMINAL, // Access a computer terminal
        FPS_EXAMINE          // Examine / inspect an interactable
    };

    /**
     * Interaction context — describes the type of FPS interactable the
     * player is targeting.  Determines which actions appear in the menu.
     */
    enum class InteractionContext {
        None = 0,           // No interactable (space mode or nothing targeted)
        Door,               // Standard interior door
        SecurityDoor,       // Access-restricted door
        Airlock,            // EVA airlock
        Terminal,           // Computer terminal
        LootContainer,      // Storage / loot box
        Fabricator,         // Crafting workbench
        MedicalBay          // Medical / oxygen station
    };

    // Callback for when an action is selected (with optional distance)
    using ActionCallback = std::function<void(Action action, const std::string& entityId)>;
    using RangedActionCallback = std::function<void(Action action, const std::string& entityId, int distance_m)>;

    RadialMenu();
    ~RadialMenu() = default;

    /**
     * Open the radial menu at screen position, targeting an entity.
     * Call when the user holds left-click on an entity in space.
     * @param distanceToTarget Distance in metres to the target entity (used to disable warp for nearby entities)
     */
    void Open(float screenX, float screenY, const std::string& entityId, float distanceToTarget = 0.0f);

    /**
     * Open the radial menu in FPS mode for an interactable object.
     * The menu segments are chosen based on the interaction context.
     *
     * @param screenX       Screen X position to center the menu
     * @param screenY       Screen Y position to center the menu
     * @param entityId      ID of the interactable entity
     * @param context       Type of interactable (door, terminal, loot, etc.)
     * @param displayName   Human-readable label for the target (e.g. "Airlock B-2")
     * @param isDoorOpen    Hint: true if a door/airlock is currently open
     * @param isLocked      Hint: true if the target is locked / access-restricted
     */
    void OpenFPS(float screenX, float screenY,
                 const std::string& entityId,
                 InteractionContext context,
                 const std::string& displayName = "",
                 bool isDoorOpen = false,
                 bool isLocked = false);

    /**
     * Close/cancel the radial menu.
     */
    void Close();

    /**
     * Update mouse position while menu is open.
     * Determines which segment is highlighted.
     */
    void UpdateMousePosition(float mouseX, float mouseY);

    /**
     * Confirm selection (call on mouse release).
     * Returns the selected action.
     */
    Action Confirm();

    /**
     * Render the radial menu (legacy stub — no-op).
     */
    void Render();

    /**
     * Render the radial menu via Atlas (call between beginFrame/endFrame).
     */
    void RenderAtlas(atlas::AtlasContext& ctx);

    /**
     * Check if menu is currently open.
     */
    bool IsOpen() const { return m_open; }

    /**
     * Check if the menu is in FPS mode.
     */
    bool IsFPSMode() const { return m_fpsMode; }

    /**
     * Get the current interaction context (FPS mode only).
     */
    InteractionContext GetInteractionContext() const { return m_interactionContext; }

    /**
     * Set callback for action selection.
     */
    void SetActionCallback(ActionCallback cb) { m_onAction = std::move(cb); }

    /**
     * Set callback for ranged actions (Orbit, Keep at Range).
     * Distance is determined by how far the mouse is dragged from center.
     */
    void SetRangedActionCallback(RangedActionCallback cb) { m_onRangedAction = std::move(cb); }

    /**
     * Get the currently highlighted action.
     */
    Action GetHighlightedAction() const { return m_highlightedAction; }

    /**
     * Get targeted entity ID.
     */
    const std::string& GetTargetEntity() const { return m_entityId; }

    /**
     * Get the drag-to-range distance (metres) for the current selection.
     * Only meaningful when highlighted action is ORBIT or KEEP_AT_RANGE.
     */
    int GetRangeDistance() const { return m_rangeDistance; }

private:
    // Menu segment layout
    struct Segment {
        Action action;
        const char* label;
        const char* icon;
        float startAngle;  // In radians
        float endAngle;
    };

    void SetupSegments();
    void SetupFPSSegments(InteractionContext context, bool isDoorOpen, bool isLocked);
    int GetSegmentAtAngle(float angle) const;
    void UpdateRangeDistance(float dist);

    // Minimum warp distance in metres (matches ShipPhysics::MIN_WARP_DISTANCE)
    static constexpr float MIN_WARP_DISTANCE = 150000.0f;

    bool m_open;
    bool m_fpsMode = false;                // True when in FPS interaction mode
    InteractionContext m_interactionContext = InteractionContext::None;
    std::string m_displayName;             // FPS: human-readable target name
    float m_centerX, m_centerY;            // Screen center of the menu
    float m_mouseX, m_mouseY;             // Current mouse position
    std::string m_entityId;                // Target entity
    Action m_highlightedAction;            // Currently highlighted segment
    int m_rangeDistance = 0;               // Drag-to-range distance (metres)
    float m_distanceToTarget = 0.0f;       // Distance in metres to target entity

    /** Check if warp is disabled for the current target (too close). */
    bool isWarpDisabled() const {
        return m_distanceToTarget > 0.0f && m_distanceToTarget < MIN_WARP_DISTANCE;
    }

    std::vector<Segment> m_segments;

    ActionCallback m_onAction;
    RangedActionCallback m_onRangedAction;

    // Visual constants
    static constexpr float INNER_RADIUS = 30.0f;   // Dead zone radius
    static constexpr float OUTER_RADIUS = 100.0f;  // Menu outer radius
    static constexpr float ICON_RADIUS = 65.0f;    // Where icons/labels are drawn
    static constexpr float MAX_RANGE_RADIUS = 180.0f; // Max drag radius for range selection
};

} // namespace UI

#endif // RADIAL_MENU_H
