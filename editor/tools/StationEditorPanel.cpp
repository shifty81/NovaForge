#include "StationEditorPanel.h"

#include <algorithm>
#include <sstream>

namespace atlas::editor {

// ── Construction ─────────────────────────────────────────────────────

StationEditorPanel::StationEditorPanel() = default;

// ── Blueprint identity ───────────────────────────────────────────────

void StationEditorPanel::SetStationName(const std::string& name) {
    if (!name.empty()) {
        m_stationName = name;
        log("Station name: " + name);
    }
}

void StationEditorPanel::SetFaction(const std::string& faction) {
    if (!faction.empty()) {
        m_faction = faction;
        log("Faction: " + faction);
    }
}

void StationEditorPanel::SetSecurityLevel(float sec) {
    m_securityLevel = std::max(0.0f, std::min(1.0f, sec));
}

// ── Segment management ───────────────────────────────────────────────

int StationEditorPanel::AddSegment(const std::string& segmentType,
                                    float length, float radius) {
    if (static_cast<int>(m_segments.size()) >= kMaxSegments) return -1;
    if (length <= 0.0f || radius <= 0.0f) return -1;

    // Validate segment type
    if (segmentType != "arm" && segmentType != "ring" && segmentType != "hub" &&
        segmentType != "hangar" && segmentType != "solar_panel") return -1;

    StationSegment seg;
    seg.segmentId   = m_nextSegmentId++;
    seg.segmentType = segmentType;
    seg.length      = length;
    seg.radius      = radius;
    m_segments.push_back(seg);

    log("Added segment #" + std::to_string(seg.segmentId)
        + " type=" + segmentType + " L=" + std::to_string(static_cast<int>(length))
        + " R=" + std::to_string(static_cast<int>(radius)));
    return static_cast<int>(seg.segmentId);
}

bool StationEditorPanel::RemoveSegment(uint32_t segmentId) {
    auto it = std::find_if(m_segments.begin(), m_segments.end(),
        [segmentId](const StationSegment& s) { return s.segmentId == segmentId; });
    if (it == m_segments.end()) return false;
    log("Removed segment #" + std::to_string(segmentId));
    m_segments.erase(it);
    return true;
}

// ── Docking port management ──────────────────────────────────────────

int StationEditorPanel::AddDockingPort(const std::string& portName,
                                        const std::string& size) {
    if (static_cast<int>(m_dockingPorts.size()) >= kMaxDockingPorts) return -1;

    // Validate size
    if (size != "small" && size != "medium" && size != "large" && size != "capital") return -1;
    if (portName.empty()) return -1;

    StationDockingPort port;
    port.portId   = m_nextPortId++;
    port.portName = portName;
    port.size     = size;
    m_dockingPorts.push_back(port);

    log("Added dock port #" + std::to_string(port.portId)
        + " \"" + portName + "\" size=" + size);
    return static_cast<int>(port.portId);
}

bool StationEditorPanel::RemoveDockingPort(uint32_t portId) {
    auto it = std::find_if(m_dockingPorts.begin(), m_dockingPorts.end(),
        [portId](const StationDockingPort& p) { return p.portId == portId; });
    if (it == m_dockingPorts.end()) return false;
    log("Removed dock port #" + std::to_string(portId));
    m_dockingPorts.erase(it);
    return true;
}

void StationEditorPanel::SetMaxDockingCapacity(int capacity) {
    m_maxDockingCapacity = std::max(1, capacity);
}

void StationEditorPanel::SetUndockClearanceRadius(float radius) {
    m_undockClearanceRadius = std::max(50.0f, radius);
}

// ── Service management ───────────────────────────────────────────────

bool StationEditorPanel::AddService(const std::string& serviceType,
                                     float costMultiplier) {
    if (static_cast<int>(m_services.size()) >= kMaxServices) return false;

    // Validate service type
    if (serviceType != "repair" && serviceType != "market" &&
        serviceType != "manufacturing" && serviceType != "refining" &&
        serviceType != "clone_bay" && serviceType != "insurance" &&
        serviceType != "fitting" && serviceType != "bounty_office") return false;

    // No duplicate service types
    for (const auto& s : m_services) {
        if (s.serviceType == serviceType) return false;
    }

    StationService svc;
    svc.serviceId      = nextServiceId();
    svc.serviceType    = serviceType;
    svc.costMultiplier = std::max(0.1f, std::min(10.0f, costMultiplier));
    m_services.push_back(svc);

    log("Added service: " + serviceType + " cost×" +
        std::to_string(static_cast<int>(costMultiplier * 100)) + "%");
    return true;
}

bool StationEditorPanel::RemoveService(const std::string& serviceId) {
    auto it = std::find_if(m_services.begin(), m_services.end(),
        [&serviceId](const StationService& s) { return s.serviceId == serviceId; });
    if (it == m_services.end()) return false;
    log("Removed service: " + it->serviceType);
    m_services.erase(it);
    return true;
}

bool StationEditorPanel::ToggleService(const std::string& serviceId, bool enabled) {
    for (auto& s : m_services) {
        if (s.serviceId == serviceId) {
            if (s.enabled == enabled) return false;  // already in desired state
            s.enabled = enabled;
            log("Service " + s.serviceType + " " + (enabled ? "enabled" : "disabled"));
            return true;
        }
    }
    return false;
}

bool StationEditorPanel::SetServiceCostMultiplier(const std::string& serviceId,
                                                    float multiplier) {
    for (auto& s : m_services) {
        if (s.serviceId == serviceId) {
            s.costMultiplier = std::max(0.1f, std::min(10.0f, multiplier));
            return true;
        }
    }
    return false;
}

int StationEditorPanel::EnabledServiceCount() const {
    int count = 0;
    for (const auto& s : m_services) {
        if (s.enabled) ++count;
    }
    return count;
}

// ── Export ────────────────────────────────────────────────────────────

std::string StationEditorPanel::ExportJSON() const {
    std::ostringstream os;
    os << "{\n";
    os << "  \"stationName\": \"" << m_stationName << "\",\n";
    os << "  \"faction\": \"" << m_faction << "\",\n";
    os << "  \"securityLevel\": " << m_securityLevel << ",\n";
    os << "  \"maxDockingCapacity\": " << m_maxDockingCapacity << ",\n";
    os << "  \"undockClearanceRadius\": " << m_undockClearanceRadius << ",\n";

    os << "  \"segments\": [\n";
    for (size_t i = 0; i < m_segments.size(); ++i) {
        const auto& seg = m_segments[i];
        os << "    {\"id\": " << seg.segmentId
           << ", \"type\": \"" << seg.segmentType << "\""
           << ", \"length\": " << seg.length
           << ", \"radius\": " << seg.radius
           << ", \"rotationY\": " << seg.rotationY << "}";
        if (i + 1 < m_segments.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";

    os << "  \"dockingPorts\": [\n";
    for (size_t i = 0; i < m_dockingPorts.size(); ++i) {
        const auto& p = m_dockingPorts[i];
        os << "    {\"id\": " << p.portId
           << ", \"name\": \"" << p.portName << "\""
           << ", \"size\": \"" << p.size << "\""
           << ", \"pos\": [" << p.posX << ", " << p.posY << ", " << p.posZ << "]}";
        if (i + 1 < m_dockingPorts.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";

    os << "  \"services\": [\n";
    for (size_t i = 0; i < m_services.size(); ++i) {
        const auto& svc = m_services[i];
        os << "    {\"id\": \"" << svc.serviceId << "\""
           << ", \"type\": \"" << svc.serviceType << "\""
           << ", \"costMultiplier\": " << svc.costMultiplier
           << ", \"enabled\": " << (svc.enabled ? "true" : "false") << "}";
        if (i + 1 < m_services.size()) os << ",";
        os << "\n";
    }
    os << "  ]\n";

    os << "}";
    return os.str();
}

// ── Draw ──────────────────────────────────────────────────────────────

void StationEditorPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Station Editor", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad    = ctx.theme().padding;
    const float rowH   = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH  = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Station identity
    std::string identity = "Station: " + m_stationName + "  Faction: " + m_faction
        + "  Sec:" + std::to_string(static_cast<int>(m_securityLevel * 10)) + "/10";
    atlas::label(ctx, {b.x + pad, y}, identity, ctx.theme().textPrimary);
    y += rowH + pad;

    // Segment summary
    std::string segSummary = "Segments: " + std::to_string(SegmentCount())
        + "  Docking Ports: " + std::to_string(DockingPortCount())
        + "  Services: " + std::to_string(EnabledServiceCount())
        + "/" + std::to_string(ServiceCount());
    atlas::label(ctx, {b.x + pad, y}, segSummary, ctx.theme().textSecondary);
    y += rowH + pad;

    // Segment listing
    for (int i = 0; i < SegmentCount() && i < 10; ++i) {
        const auto& seg = m_segments[i];
        std::string info = "#" + std::to_string(seg.segmentId) + " " + seg.segmentType
            + " L:" + std::to_string(static_cast<int>(seg.length))
            + " R:" + std::to_string(static_cast<int>(seg.radius));
        atlas::label(ctx, {b.x + pad, y}, info, ctx.theme().textPrimary);
        y += rowH;
    }
    y += pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Helpers ───────────────────────────────────────────────────────────

void StationEditorPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

std::string StationEditorPanel::nextServiceId() {
    return "svc_" + std::to_string(m_nextServiceId++);
}

} // namespace atlas::editor
