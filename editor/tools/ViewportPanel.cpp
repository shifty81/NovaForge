#include "ViewportPanel.h"
#include <algorithm>
#include <sstream>

namespace atlas::editor {

const ViewportTransform ViewportPanel::s_defaultTransform{};

ViewportPanel::ViewportPanel() {
    m_log.push_back("[Viewport] Initialized");
}

void ViewportPanel::Draw() {
    // Stub: real rendering handled by the host windowing system (OpenGL/Vulkan).
    // In headless / test mode this is a no-op.
}

// ── Scene management ──────────────────────────────────────────────

void ViewportPanel::LoadShip(const pcg::GeneratedShip& ship, uint64_t seed) {
    // Create a root object for the ship hull
    ViewportObject hull;
    hull.id = m_nextId++;
    hull.name = ship.shipName;
    hull.type = "Ship";
    // Use mass as a rough proxy for hull size
    float hullScale = std::cbrt(ship.mass) * 0.1f;
    hull.transform.scaleX = hullScale;
    hull.transform.scaleY = hullScale * 0.3f;
    hull.transform.scaleZ = hullScale * 0.5f;
    m_objects.push_back(hull);
    m_originalTransforms.push_back({hull.id, hull.transform});

    // Create child objects for each turret hardpoint
    for (int i = 0; i < ship.turretSlots; ++i) {
        ViewportObject turret;
        turret.id = m_nextId++;
        turret.name = ship.shipName + " Turret " + std::to_string(i + 1);
        turret.type = "Hardpoint";
        // Spread turrets along the ship spine
        float t = (ship.turretSlots > 1)
            ? static_cast<float>(i) / static_cast<float>(ship.turretSlots - 1)
            : 0.5f;
        turret.transform.posX = (t - 0.5f) * hull.transform.scaleX;
        turret.transform.posY = hull.transform.scaleY * 0.5f;
        m_objects.push_back(turret);
        m_originalTransforms.push_back({turret.id, turret.transform});
    }

    std::ostringstream oss;
    oss << "[Viewport] Loaded ship '" << ship.shipName << "' (seed=" << seed
        << ") — " << (1 + ship.turretSlots) << " objects";
    m_log.push_back(oss.str());
}

void ViewportPanel::LoadStation(const pcg::GeneratedStation& station, uint64_t seed) {
    std::string stationName = "Station_" + std::to_string(station.stationId);
    for (size_t i = 0; i < station.modules.size(); ++i) {
        const auto& mod = station.modules[i];
        ViewportObject obj;
        obj.id = m_nextId++;
        obj.name = stationName + " Module " + std::to_string(i + 1);
        obj.type = "Module";
        obj.transform.posX = mod.posX;
        obj.transform.posY = mod.posY;
        obj.transform.posZ = mod.posZ;
        obj.transform.scaleX = mod.dimX;
        obj.transform.scaleY = mod.dimY;
        obj.transform.scaleZ = mod.dimZ;
        m_objects.push_back(obj);
        m_originalTransforms.push_back({obj.id, obj.transform});
    }

    std::ostringstream oss;
    oss << "[Viewport] Loaded station '" << stationName << "' (seed=" << seed
        << ") — " << station.modules.size() << " modules";
    m_log.push_back(oss.str());
}

void ViewportPanel::LoadSpineHull(const pcg::GeneratedSpineHull& hull,
                                   const pcg::TurretPlacement* placement,
                                   uint64_t seed) {
    std::string hullName = pcg::SpineHullGenerator::spineTypeName(hull.spine)
                         + "_" + pcg::ShipGenerator::hullClassName(hull.hull_class);

    // Create zone objects along the spine.
    float zoneStart = -hull.profile.length * 0.5f;
    for (size_t i = 0; i < hull.zones.size(); ++i) {
        const auto& z = hull.zones[i];
        float zoneLen = z.length_fraction * hull.profile.length;

        const char* zoneName = "Zone";
        switch (z.zone) {
            case pcg::FunctionalZone::Command:     zoneName = "Command";     break;
            case pcg::FunctionalZone::MidHull:     zoneName = "MidHull";     break;
            case pcg::FunctionalZone::Engineering: zoneName = "Engineering"; break;
        }

        // Width at this zone position (interpolate from profile).
        float t = (static_cast<float>(i) + 0.5f) / static_cast<float>(hull.zones.size());
        float width = hull.profile.width_fwd * (1.0f - t) + hull.profile.width_aft * t;
        if (z.zone == pcg::FunctionalZone::MidHull) width = hull.profile.width_mid;

        ViewportObject obj;
        obj.id   = m_nextId++;
        obj.name = hullName + " " + zoneName;
        obj.type = "HullZone";
        obj.transform.posX   = zoneStart + zoneLen * 0.5f;
        obj.transform.scaleX = zoneLen;
        obj.transform.scaleY = width;
        obj.transform.scaleZ = width * 0.5f;
        m_objects.push_back(obj);
        m_originalTransforms.push_back({obj.id, obj.transform});

        zoneStart += zoneLen;
    }

    // Create turret mount objects if placement is provided.
    if (placement) {
        for (size_t i = 0; i < placement->mounts.size(); ++i) {
            const auto& m = placement->mounts[i];
            ViewportObject turret;
            turret.id   = m_nextId++;
            turret.name = hullName + " Turret " + std::to_string(m.socket_id);
            turret.type = "TurretMount";
            turret.transform.posX = m.x_offset;
            turret.transform.posY = m.y_offset;
            turret.transform.posZ = m.z_offset;
            turret.transform.rotY = m.direction_deg;
            m_objects.push_back(turret);
            m_originalTransforms.push_back({turret.id, turret.transform});
        }
    }

    size_t totalObjects = hull.zones.size()
                        + (placement ? placement->mounts.size() : 0);
    std::ostringstream oss;
    oss << "[Viewport] Loaded spine hull '" << hullName << "' (seed=" << seed
        << ") — " << totalObjects << " objects";
    m_log.push_back(oss.str());
}

void ViewportPanel::ClearScene() {
    m_objects.clear();
    m_originalTransforms.clear();
    m_pendingChanges.clear();
    m_selectedId = 0;
    m_nextId = 1;
    m_log.push_back("[Viewport] Scene cleared");
}

// ── Selection ─────────────────────────────────────────────────────

void ViewportPanel::SelectObject(uint32_t id) {
    DeselectAll();
    if (auto* obj = findObject(id)) {
        obj->selected = true;
        m_selectedId = id;
    }
}

void ViewportPanel::DeselectAll() {
    for (auto& obj : m_objects) {
        obj.selected = false;
    }
    m_selectedId = 0;
}

// ── Transform gizmo ───────────────────────────────────────────────

void ViewportPanel::TranslateSelected(float dx, float dy, float dz) {
    auto* obj = findObject(m_selectedId);
    if (!obj) return;

    float old3[3] = { obj->transform.posX, obj->transform.posY, obj->transform.posZ };
    obj->transform.posX += dx;
    obj->transform.posY += dy;
    obj->transform.posZ += dz;
    float new3[3] = { obj->transform.posX, obj->transform.posY, obj->transform.posZ };

    recordChange(obj->id, "position", old3, new3);
}

void ViewportPanel::RotateSelected(float dx, float dy, float dz) {
    auto* obj = findObject(m_selectedId);
    if (!obj) return;

    float old3[3] = { obj->transform.rotX, obj->transform.rotY, obj->transform.rotZ };
    obj->transform.rotX += dx;
    obj->transform.rotY += dy;
    obj->transform.rotZ += dz;
    float new3[3] = { obj->transform.rotX, obj->transform.rotY, obj->transform.rotZ };

    recordChange(obj->id, "rotation", old3, new3);
}

void ViewportPanel::ScaleSelected(float dx, float dy, float dz) {
    auto* obj = findObject(m_selectedId);
    if (!obj) return;

    float old3[3] = { obj->transform.scaleX, obj->transform.scaleY, obj->transform.scaleZ };
    obj->transform.scaleX += dx;
    obj->transform.scaleY += dy;
    obj->transform.scaleZ += dz;
    // Clamp scale to avoid zero/negative
    obj->transform.scaleX = std::max(obj->transform.scaleX, 0.01f);
    obj->transform.scaleY = std::max(obj->transform.scaleY, 0.01f);
    obj->transform.scaleZ = std::max(obj->transform.scaleZ, 0.01f);
    float new3[3] = { obj->transform.scaleX, obj->transform.scaleY, obj->transform.scaleZ };

    recordChange(obj->id, "scale", old3, new3);
}

const ViewportTransform& ViewportPanel::GetTransform(uint32_t id) const {
    const auto* obj = findObject(id);
    return obj ? obj->transform : s_defaultTransform;
}

// ── Camera ────────────────────────────────────────────────────────

void ViewportPanel::OrbitCamera(float deltaYaw, float deltaPitch) {
    m_cameraYaw += deltaYaw;
    m_cameraPitch += deltaPitch;
    // Clamp pitch to avoid gimbal lock
    if (m_cameraPitch > 89.0f) m_cameraPitch = 89.0f;
    if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;
}

// ── Change tracking ───────────────────────────────────────────────

std::vector<ViewportChange> ViewportPanel::CommitChanges() {
    std::vector<ViewportChange> committed = std::move(m_pendingChanges);
    m_pendingChanges.clear();

    // Update original transforms to match current state
    m_originalTransforms.clear();
    for (const auto& obj : m_objects) {
        m_originalTransforms.push_back({obj.id, obj.transform});
    }

    if (!committed.empty()) {
        std::ostringstream oss;
        oss << "[Viewport] Committed " << committed.size() << " changes";
        m_log.push_back(oss.str());
    }

    return committed;
}

void ViewportPanel::DiscardChanges() {
    // Revert all objects to their original transforms
    for (auto& obj : m_objects) {
        for (const auto& orig : m_originalTransforms) {
            if (orig.first == obj.id) {
                obj.transform = orig.second;
                break;
            }
        }
    }
    m_pendingChanges.clear();
    m_log.push_back("[Viewport] Discarded pending changes");
}

// ── Private helpers ───────────────────────────────────────────────

ViewportObject* ViewportPanel::findObject(uint32_t id) {
    for (auto& obj : m_objects) {
        if (obj.id == id) return &obj;
    }
    return nullptr;
}

const ViewportObject* ViewportPanel::findObject(uint32_t id) const {
    for (const auto& obj : m_objects) {
        if (obj.id == id) return &obj;
    }
    return nullptr;
}

void ViewportPanel::recordChange(uint32_t id, const std::string& field,
                                  const float old3[3], const float new3[3]) {
    ViewportChange change;
    change.objectId = id;
    change.field = field;
    for (int i = 0; i < 3; ++i) {
        change.oldValues[i] = old3[i];
        change.newValues[i] = new3[i];
    }
    m_pendingChanges.push_back(change);
}

void ViewportPanel::OnAssetReloaded(const std::string& assetId,
                                     const std::string& path) {
    std::ostringstream oss;
    oss << "[Viewport] Asset changed: " << assetId << " (" << path << ") — live reload";
    m_log.push_back(oss.str());
}

} // namespace atlas::editor
