#include "PhysicsTunerPanel.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

PhysicsTunerPanel::PhysicsTunerPanel() {
    // Zero-G
    PhysicsPreset zeroG;
    zeroG.name              = "Zero-G";
    zeroG.gravity           = 0.0f;
    zeroG.windStrength      = 0.0f;
    zeroG.windDirection[0]  = 0.0f;
    zeroG.windDirection[1]  = 0.0f;
    zeroG.windDirection[2]  = 0.0f;
    zeroG.atmosphereDensity = 0.0f;
    m_presets.push_back(zeroG);

    // Low-G Planet
    PhysicsPreset lowG;
    lowG.name              = "Low-G Planet";
    lowG.gravity           = 1.62f;
    lowG.windStrength      = 0.0f;
    lowG.windDirection[0]  = 0.0f;
    lowG.windDirection[1]  = 0.0f;
    lowG.windDirection[2]  = 0.0f;
    lowG.atmosphereDensity = 0.3f;
    m_presets.push_back(lowG);

    // Earth-Like
    PhysicsPreset earth;
    earth.name              = "Earth-Like";
    earth.gravity           = 9.81f;
    earth.windStrength      = 0.0f;
    earth.windDirection[0]  = 0.0f;
    earth.windDirection[1]  = 0.0f;
    earth.windDirection[2]  = 0.0f;
    earth.atmosphereDensity = 1.0f;
    m_presets.push_back(earth);

    // Windy
    PhysicsPreset windy;
    windy.name              = "Windy";
    windy.gravity           = 9.81f;
    windy.windStrength      = 15.0f;
    windy.windDirection[0]  = 1.0f;
    windy.windDirection[1]  = 0.0f;
    windy.windDirection[2]  = 0.3f;
    windy.atmosphereDensity = 1.2f;
    m_presets.push_back(windy);

    // Active environment defaults to Earth-Like (index 2)
    m_activeEnvironment = m_presets[2];

    log("Physics Tuner initialized");
}

// ── Draw ───────────────────────────────────────────────────────────

void PhysicsTunerPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Physics Tuner", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad  = ctx.theme().padding;
    const float rowH = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Preset buttons
    atlas::label(ctx, {b.x + pad, y}, "Presets:", ctx.theme().textPrimary);
    y += rowH;

    float btnX = b.x + pad;
    const float btnW = 100.0f;
    for (size_t i = 0; i < m_presets.size(); ++i) {
        if (btnX + btnW > b.x + b.w - pad) {
            btnX = b.x + pad;
            y += rowH + pad;
        }
        if (atlas::button(ctx, m_presets[i].name.c_str(), {btnX, y, btnW, rowH})) {
            ApplyPreset(i);
        }
        btnX += btnW + pad;
    }
    y += rowH + pad + pad;

    // Active environment parameters
    atlas::label(ctx, {b.x + pad, y},
        "Environment: " + m_activeEnvironment.name, ctx.theme().textPrimary);
    y += rowH + pad;

    atlas::label(ctx, {b.x + pad, y},
        "  Gravity: " + std::to_string(m_activeEnvironment.gravity),
        ctx.theme().textSecondary);
    y += rowH;
    atlas::label(ctx, {b.x + pad, y},
        "  Wind: " + std::to_string(m_activeEnvironment.windStrength),
        ctx.theme().textSecondary);
    y += rowH;
    atlas::label(ctx, {b.x + pad, y},
        "  Density: " + std::to_string(m_activeEnvironment.atmosphereDensity),
        ctx.theme().textSecondary);
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Object count + filter info
    std::string info = "Objects: " + std::to_string(m_objects.size());
    if (!m_typeFilter.empty()) {
        info += " (filtered: " + std::to_string(FilteredCount()) + ")";
    }
    atlas::label(ctx, {b.x + pad, y}, info, ctx.theme().textSecondary);
    y += rowH + pad;

    // Selected object details
    if (m_selectedIndex >= 0 &&
        m_selectedIndex < static_cast<int>(m_objects.size())) {
        const auto& obj = m_objects[m_selectedIndex];
        atlas::label(ctx, {b.x + pad, y},
            "Selected: " + obj.objectName, ctx.theme().textPrimary);
        y += rowH + pad;

        atlas::label(ctx, {b.x + pad, y},
            "  ID: " + obj.objectId, ctx.theme().textSecondary);
        y += rowH;
        atlas::label(ctx, {b.x + pad, y},
            "  Type: " + obj.objectType, ctx.theme().textSecondary);
        y += rowH;
        atlas::label(ctx, {b.x + pad, y},
            "  Mass: " + std::to_string(obj.mass), ctx.theme().textSecondary);
        y += rowH;
        atlas::label(ctx, {b.x + pad, y},
            "  Friction: " + std::to_string(obj.friction),
            ctx.theme().textSecondary);
        y += rowH;
        atlas::label(ctx, {b.x + pad, y},
            "  Restitution: " + std::to_string(obj.restitution),
            ctx.theme().textSecondary);
        y += rowH;
        atlas::label(ctx, {b.x + pad, y},
            std::string("  Kinematic: ") + (obj.isKinematic ? "yes" : "no"),
            ctx.theme().textSecondary);
        y += rowH;

        if (obj.objectType == "cloth") {
            atlas::label(ctx, {b.x + pad, y},
                "  Stiffness: " + std::to_string(obj.clothParams.stiffness),
                ctx.theme().textSecondary);
            y += rowH;
            atlas::label(ctx, {b.x + pad, y},
                "  Damping: " + std::to_string(obj.clothParams.damping),
                ctx.theme().textSecondary);
            y += rowH;
            atlas::label(ctx, {b.x + pad, y},
                "  GravityInfluence: "
                + std::to_string(obj.clothParams.gravityInfluence),
                ctx.theme().textSecondary);
            y += rowH;
            atlas::label(ctx, {b.x + pad, y},
                "  Drag: " + std::to_string(obj.clothParams.drag),
                ctx.theme().textSecondary);
            y += rowH;
        }
        y += pad;
    }

    // Simulation controls
    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    atlas::label(ctx, {b.x + pad, y},
        std::string("Simulation: ") + (m_simulationPaused ? "PAUSED" : "RUNNING")
        + "  Steps: " + std::to_string(m_stepCount),
        ctx.theme().textPrimary);
    y += rowH + pad;

    btnX = b.x + pad;
    const float ctrlW = 80.0f;
    if (atlas::button(ctx, m_simulationPaused ? "Resume" : "Pause",
                      {btnX, y, ctrlW, rowH})) {
        if (m_simulationPaused) ResumeSimulation();
        else                    PauseSimulation();
    }
    btnX += ctrlW + pad;
    if (atlas::button(ctx, "Step", {btnX, y, ctrlW, rowH})) {
        StepSimulation();
    }
    y += rowH + pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Object management ─────────────────────────────────────────────

size_t PhysicsTunerPanel::AddObject(const PhysicsObjectEntry& entry) {
    m_objects.push_back(entry);
    log("Added object: " + entry.objectName);
    return m_objects.size() - 1;
}

bool PhysicsTunerPanel::RemoveObject(size_t index) {
    if (index >= m_objects.size()) return false;
    std::string name = m_objects[index].objectName;
    m_objects.erase(m_objects.begin() + static_cast<ptrdiff_t>(index));

    if (m_selectedIndex == static_cast<int>(index)) {
        m_selectedIndex = -1;
    } else if (m_selectedIndex > static_cast<int>(index)) {
        --m_selectedIndex;
    }

    log("Removed object: " + name);
    return true;
}

bool PhysicsTunerPanel::UpdateObject(size_t index,
                                      const PhysicsObjectEntry& entry) {
    if (index >= m_objects.size()) return false;
    std::string oldName = m_objects[index].objectName;
    m_objects[index] = entry;
    log("Updated object: " + oldName + " -> " + entry.objectName);
    return true;
}

// ── Selection ─────────────────────────────────────────────────────

void PhysicsTunerPanel::SelectObject(int index) {
    if (index < 0 || index >= static_cast<int>(m_objects.size())) return;
    m_selectedIndex = index;
    log("Selected: " + m_objects[index].objectName);
}

void PhysicsTunerPanel::ClearSelection() {
    m_selectedIndex = -1;
}

// ── Filtering ─────────────────────────────────────────────────────

void PhysicsTunerPanel::SetTypeFilter(const std::string& filter) {
    m_typeFilter = filter;
}

size_t PhysicsTunerPanel::FilteredCount() const {
    size_t count = 0;
    for (const auto& obj : m_objects) {
        if (matchesFilter(obj)) ++count;
    }
    return count;
}

bool PhysicsTunerPanel::matchesFilter(const PhysicsObjectEntry& entry) const {
    if (m_typeFilter.empty()) return true;
    // Case-insensitive substring match on objectType
    std::string lowerType = entry.objectType;
    std::string lowerFilter = m_typeFilter;
    std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lowerType.find(lowerFilter) != std::string::npos;
}

// ── Preset management ─────────────────────────────────────────────

bool PhysicsTunerPanel::ApplyPreset(size_t index) {
    if (index >= m_presets.size()) return false;
    m_activeEnvironment  = m_presets[index];
    m_activePresetIndex  = static_cast<int>(index);
    log("Applied preset: " + m_presets[index].name);
    return true;
}

// ── Active environment ────────────────────────────────────────────

void PhysicsTunerPanel::SetGravity(float gravity) {
    m_activeEnvironment.gravity = gravity;
    m_activePresetIndex = -1;
    m_activeEnvironment.name = "Custom";
}

void PhysicsTunerPanel::SetWindStrength(float strength) {
    m_activeEnvironment.windStrength = strength;
    m_activePresetIndex = -1;
    m_activeEnvironment.name = "Custom";
}

void PhysicsTunerPanel::SetAtmosphereDensity(float density) {
    m_activeEnvironment.atmosphereDensity = density;
    m_activePresetIndex = -1;
    m_activeEnvironment.name = "Custom";
}

// ── Simulation control ────────────────────────────────────────────

void PhysicsTunerPanel::PauseSimulation() {
    m_simulationPaused = true;
    log("Simulation paused");
}

void PhysicsTunerPanel::ResumeSimulation() {
    m_simulationPaused = false;
    log("Simulation resumed");
}

void PhysicsTunerPanel::StepSimulation() {
    ++m_stepCount;
    log("Simulation step " + std::to_string(m_stepCount));
}

// ── Export / Import ───────────────────────────────────────────────

static std::string jsonEscapePT(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) { /* skip */ }
            else { out += c; }
            break;
        }
    }
    return out;
}

std::string PhysicsTunerPanel::ExportToJson() const {
    std::ostringstream os;
    os << "{ \"physicsTuner\": {\n";

    // Active environment
    os << "  \"environment\": {";
    os << " \"name\": \"" << jsonEscapePT(m_activeEnvironment.name) << "\",";
    os << " \"gravity\": " << m_activeEnvironment.gravity << ",";
    os << " \"windStrength\": " << m_activeEnvironment.windStrength << ",";
    os << " \"windDirection\": [" << m_activeEnvironment.windDirection[0]
       << ", " << m_activeEnvironment.windDirection[1]
       << ", " << m_activeEnvironment.windDirection[2] << "],";
    os << " \"atmosphereDensity\": " << m_activeEnvironment.atmosphereDensity;
    os << " },\n";

    // Objects
    os << "  \"objects\": {\n";
    for (size_t i = 0; i < m_objects.size(); ++i) {
        const auto& obj = m_objects[i];
        os << "    \"" << jsonEscapePT(obj.objectId) << "\": {";
        os << " \"objectName\": \"" << jsonEscapePT(obj.objectName) << "\",";
        os << " \"objectType\": \"" << jsonEscapePT(obj.objectType) << "\",";
        os << " \"mass\": " << obj.mass << ",";
        os << " \"friction\": " << obj.friction << ",";
        os << " \"restitution\": " << obj.restitution << ",";
        os << " \"isKinematic\": " << (obj.isKinematic ? "true" : "false") << ",";
        os << " \"simulationEnabled\": " << (obj.simulationEnabled ? "true" : "false");

        if (obj.objectType == "cloth") {
            os << ",";
            os << " \"stiffness\": " << obj.clothParams.stiffness << ",";
            os << " \"damping\": " << obj.clothParams.damping << ",";
            os << " \"gravityInfluence\": " << obj.clothParams.gravityInfluence << ",";
            os << " \"drag\": " << obj.clothParams.drag;
        }

        os << " }";
        if (i + 1 < m_objects.size()) os << ",";
        os << "\n";
    }
    os << "  }\n";

    os << "} }";
    return os.str();
}

size_t PhysicsTunerPanel::ImportFromJson(const std::string& json) {
    size_t imported = 0;
    size_t pos = 0;

    // Skip to the "objects" sub-object by finding three opening braces
    size_t firstBrace = json.find('{');
    if (firstBrace == std::string::npos) return 0;
    size_t secondBrace = json.find('{', firstBrace + 1);
    if (secondBrace == std::string::npos) return 0;

    // Look for the "objects" key to find the right sub-object
    size_t objectsKey = json.find("\"objects\"", secondBrace);
    if (objectsKey == std::string::npos) {
        // Fallback: treat second brace as the objects container
        objectsKey = secondBrace;
    }
    size_t objectsBrace = json.find('{', objectsKey);
    if (objectsBrace == std::string::npos) return 0;

    pos = objectsBrace + 1;
    while (pos < json.size()) {
        // Find next quoted key (object id)
        size_t qStart = json.find('"', pos);
        if (qStart == std::string::npos) break;
        size_t qEnd = json.find('"', qStart + 1);
        if (qEnd == std::string::npos) break;

        std::string objectId = json.substr(qStart + 1, qEnd - qStart - 1);
        if (objectId.empty()) { pos = qEnd + 1; continue; }

        // Find the object body
        size_t objStart = json.find('{', qEnd);
        if (objStart == std::string::npos) break;

        // Simple brace matching
        int depth = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < json.size() && depth > 0) {
            if (json[objEnd] == '{') ++depth;
            else if (json[objEnd] == '}') --depth;
            ++objEnd;
        }

        std::string body = json.substr(objStart + 1, objEnd - objStart - 2);

        PhysicsObjectEntry entry;
        entry.objectId = objectId;

        // Extract known fields from the object body
        size_t fpos = 0;
        while (fpos < body.size()) {
            size_t fks = body.find('"', fpos);
            if (fks == std::string::npos) break;
            size_t fke = body.find('"', fks + 1);
            if (fke == std::string::npos) break;
            std::string fkey = body.substr(fks + 1, fke - fks - 1);

            size_t colon = body.find(':', fke);
            if (colon == std::string::npos) break;

            size_t vs = colon + 1;
            while (vs < body.size() && body[vs] == ' ') ++vs;

            if (vs < body.size() && body[vs] == '"') {
                // String value
                size_t ve = body.find('"', vs + 1);
                if (ve == std::string::npos) break;
                std::string val = body.substr(vs + 1, ve - vs - 1);

                if (fkey == "objectName")       entry.objectName = val;
                else if (fkey == "objectType")  entry.objectType = val;

                fpos = ve + 1;
            } else {
                // Number or boolean
                size_t ve = vs;
                while (ve < body.size() && body[ve] != ',' && body[ve] != '}')
                    ++ve;
                std::string raw = body.substr(vs, ve - vs);
                while (!raw.empty() && (raw.back() == ' ' || raw.back() == '\n'))
                    raw.pop_back();

                if (fkey == "mass") {
                    try { entry.mass = std::stof(raw); }
                    catch (...) { entry.mass = 1.0f; }
                } else if (fkey == "friction") {
                    try { entry.friction = std::stof(raw); }
                    catch (...) { entry.friction = 0.5f; }
                } else if (fkey == "restitution") {
                    try { entry.restitution = std::stof(raw); }
                    catch (...) { entry.restitution = 0.3f; }
                } else if (fkey == "isKinematic") {
                    entry.isKinematic = (raw == "true");
                } else if (fkey == "simulationEnabled") {
                    entry.simulationEnabled = (raw == "true");
                } else if (fkey == "stiffness") {
                    try { entry.clothParams.stiffness = std::stof(raw); }
                    catch (...) { entry.clothParams.stiffness = 0.8f; }
                } else if (fkey == "damping") {
                    try { entry.clothParams.damping = std::stof(raw); }
                    catch (...) { entry.clothParams.damping = 0.3f; }
                } else if (fkey == "gravityInfluence") {
                    try { entry.clothParams.gravityInfluence = std::stof(raw); }
                    catch (...) { entry.clothParams.gravityInfluence = 1.0f; }
                } else if (fkey == "drag") {
                    try { entry.clothParams.drag = std::stof(raw); }
                    catch (...) { entry.clothParams.drag = 0.25f; }
                }

                fpos = ve;
            }
        }

        m_objects.push_back(entry);
        ++imported;
        pos = objEnd;
    }

    if (imported > 0) {
        log("Imported " + std::to_string(imported) + " objects from JSON");
    }
    return imported;
}

// ── Logging ───────────────────────────────────────────────────────

void PhysicsTunerPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
