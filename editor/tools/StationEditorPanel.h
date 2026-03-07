#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * @brief Docking port definition for a station.
 */
struct StationDockingPort {
    uint32_t portId     = 0;
    std::string portName;        ///< e.g. "Bay Alpha", "Dock 3"
    std::string size;            ///< "small", "medium", "large", "capital"
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
};

/**
 * @brief Service slot available at a station.
 */
struct StationService {
    std::string serviceId;
    std::string serviceType;     ///< "repair", "market", "manufacturing", "refining",
                                 ///< "clone_bay", "insurance", "fitting", "bounty_office"
    float costMultiplier = 1.0f; ///< 1.0 = standard cost
    bool enabled = true;
};

/**
 * @brief Station arm/ring segment definition.
 */
struct StationSegment {
    uint32_t segmentId    = 0;
    std::string segmentType;     ///< "arm", "ring", "hub", "hangar", "solar_panel"
    float length    = 100.0f;
    float radius    = 50.0f;
    float rotationY = 0.0f;      ///< Rotation around Y axis (degrees)
};

/**
 * StationEditorPanel — Station blueprint authoring tool.
 *
 * Designers can:
 *   - Create a new station blueprint with name, faction, and security level.
 *   - Add structural segments (arms, rings, hubs, hangars, solar panels).
 *   - Configure docking ports with size class and position.
 *   - Enable/disable station services (repair, market, manufacturing, etc.).
 *   - Set cost multipliers for each service.
 *   - Define docking capacity and undock clearance radius.
 *   - Export the blueprint to JSON for the universe data pipeline.
 *
 * The panel is the station equivalent of ShipArchetypePanel.
 * Ships define hull shapes and hardpoints; stations define segments,
 * docking ports, and services.
 *
 * Headless-safe: Draw() is a no-op when no AtlasContext is set.
 */
class StationEditorPanel : public EditorPanel {
public:
    StationEditorPanel();
    ~StationEditorPanel() override = default;

    const char* Name() const override { return "Station Editor"; }
    void Draw() override;

    // ── Blueprint identity ───────────────────────────────────────

    void SetStationName(const std::string& name);
    const std::string& StationName() const { return m_stationName; }

    void SetFaction(const std::string& faction);
    const std::string& Faction() const { return m_faction; }

    void SetSecurityLevel(float sec);
    float SecurityLevel() const { return m_securityLevel; }

    // ── Segment management ───────────────────────────────────────

    int AddSegment(const std::string& segmentType, float length, float radius);
    bool RemoveSegment(uint32_t segmentId);
    int SegmentCount() const { return static_cast<int>(m_segments.size()); }
    const std::vector<StationSegment>& Segments() const { return m_segments; }

    // ── Docking port management ──────────────────────────────────

    int AddDockingPort(const std::string& portName, const std::string& size);
    bool RemoveDockingPort(uint32_t portId);
    int DockingPortCount() const { return static_cast<int>(m_dockingPorts.size()); }
    const std::vector<StationDockingPort>& DockingPorts() const { return m_dockingPorts; }

    void SetMaxDockingCapacity(int capacity);
    int MaxDockingCapacity() const { return m_maxDockingCapacity; }

    void SetUndockClearanceRadius(float radius);
    float UndockClearanceRadius() const { return m_undockClearanceRadius; }

    // ── Service management ───────────────────────────────────────

    bool AddService(const std::string& serviceType, float costMultiplier);
    bool RemoveService(const std::string& serviceId);
    bool ToggleService(const std::string& serviceId, bool enabled);
    bool SetServiceCostMultiplier(const std::string& serviceId, float multiplier);
    int ServiceCount() const { return static_cast<int>(m_services.size()); }
    const std::vector<StationService>& Services() const { return m_services; }

    int EnabledServiceCount() const;

    // ── Export ────────────────────────────────────────────────────

    std::string ExportJSON() const;

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::string m_stationName   = "New Station";
    std::string m_faction       = "caldari";
    float m_securityLevel       = 0.8f;
    int m_maxDockingCapacity    = 20;
    float m_undockClearanceRadius = 500.0f;

    std::vector<StationSegment>    m_segments;
    std::vector<StationDockingPort> m_dockingPorts;
    std::vector<StationService>    m_services;

    static constexpr int kMaxSegments     = 20;
    static constexpr int kMaxDockingPorts = 12;
    static constexpr int kMaxServices     = 10;

    uint32_t m_nextSegmentId = 1;
    uint32_t m_nextPortId    = 1;
    uint32_t m_nextServiceId = 1;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    std::string nextServiceId();
};

} // namespace atlas::editor
