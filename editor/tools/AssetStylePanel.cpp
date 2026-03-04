#include "AssetStylePanel.h"
#include <sstream>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

AssetStylePanel::AssetStylePanel() {
    m_currentStyle        = pcg::AssetStyle{};
    m_currentStyle.name   = "Untitled";
    m_currentStyle.targetType = pcg::GenerationStyleType::ShipLayout;
    m_currentStyle.version = 1;
    m_currentStyle.valid   = false;
    m_pcgManager.initialize(42);
}

// ── Draw (stub) ────────────────────────────────────────────────────

void AssetStylePanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Asset Style", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    const float widgetW = b.w - 2.0f * pad;
    float y = b.y + headerH + pad;

    // Style name
    atlas::label(ctx, {b.x + pad, y}, "Style: " + m_currentStyle.name,
                 ctx.theme().textPrimary);
    y += rowH + pad;

    // Target type combo box
    static const std::vector<std::string> typeItems = {
        "ShipLayout", "StationLayout", "InteriorLayout",
        "StarSystem", "AsteroidField", "FleetComposition"
    };
    int typeIdx = static_cast<int>(m_currentStyle.targetType);
    if (atlas::comboBox(ctx, "Target Type", {b.x + pad, y, widgetW, rowH + pad},
                        typeItems, &typeIdx, &m_targetTypeDropdownOpen)) {
        m_currentStyle.targetType = static_cast<pcg::GenerationStyleType>(typeIdx);
    }
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, widgetW);
    y += pad;

    // ── Shape Profile section ──────────────────────────────────────
    atlas::label(ctx, {b.x + pad, y}, "Shape Profile",
                 ctx.theme().textPrimary);
    y += rowH + pad;

    atlas::label(ctx, {b.x + 2.0f * pad, y},
        "Name: " + m_currentStyle.shape.name, ctx.theme().textSecondary);
    y += rowH + pad;

    atlas::checkbox(ctx, "Mirror X",
        {b.x + pad, y, widgetW * 0.45f, rowH + pad},
        &m_currentStyle.shape.mirrorX);
    atlas::checkbox(ctx, "Mirror Y",
        {b.x + pad + widgetW * 0.5f, y, widgetW * 0.45f, rowH + pad},
        &m_currentStyle.shape.mirrorY);
    y += rowH + pad;

    atlas::slider(ctx, "Smoothing", {b.x + pad, y, widgetW, rowH + pad},
                  &m_currentStyle.shape.smoothing, 0.0f, 1.0f, "%.2f");
    y += rowH + pad;

    atlas::label(ctx, {b.x + 2.0f * pad, y},
        "Control Points: " + std::to_string(m_currentStyle.shape.controlPoints.size()),
        ctx.theme().textSecondary);
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, widgetW);
    y += pad;

    // ── Palette section ────────────────────────────────────────────
    atlas::label(ctx, {b.x + pad, y}, "Palette", ctx.theme().textPrimary);
    y += rowH + pad;

    atlas::label(ctx, {b.x + 2.0f * pad, y},
        "Name: " + m_currentStyle.palette.name, ctx.theme().textSecondary);
    y += rowH + pad;

    // Surface treatment combo box
    static const std::vector<std::string> treatmentItems = {
        "None", "PanelLines", "Greeble", "Weathered", "BattleScarred", "Pristine"
    };
    int treatIdx = static_cast<int>(m_currentStyle.palette.surfaceTreatment);
    if (atlas::comboBox(ctx, "Surface Treatment",
                        {b.x + pad, y, widgetW, rowH + pad},
                        treatmentItems, &treatIdx, &m_treatmentDropdownOpen)) {
        m_currentStyle.palette.surfaceTreatment =
            static_cast<pcg::SurfaceTreatment>(treatIdx);
    }
    y += rowH + pad;

    // Detail level slider
    atlas::slider(ctx, "Detail Level", {b.x + pad, y, widgetW, rowH + pad},
                  &m_currentStyle.palette.detailLevel, 0.0f, 1.0f, "%.2f");
    y += rowH + pad;

    // Color list
    atlas::label(ctx, {b.x + 2.0f * pad, y},
        "Colors: " + std::to_string(m_currentStyle.palette.colors.size()),
        ctx.theme().textSecondary);
    y += rowH + pad;
    {
        size_t count = m_currentStyle.palette.colors.size();
        size_t show = std::min(count, size_t(8));
        for (size_t i = 0; i < show; ++i) {
            const auto& c = m_currentStyle.palette.colors[i];
            atlas::label(ctx, {b.x + 3.0f * pad, y},
                c.regionName, ctx.theme().textSecondary);
            y += rowH + pad;
        }
        if (count > 8) {
            atlas::label(ctx, {b.x + 3.0f * pad, y},
                "... " + std::to_string(count - 8) + " more",
                ctx.theme().textSecondary);
            y += rowH + pad;
        }
    }

    // Material list
    atlas::label(ctx, {b.x + 2.0f * pad, y},
        "Materials: " + std::to_string(m_currentStyle.palette.materials.size()),
        ctx.theme().textSecondary);
    y += rowH + pad;
    {
        size_t count = m_currentStyle.palette.materials.size();
        size_t show = std::min(count, size_t(8));
        for (size_t i = 0; i < show; ++i) {
            const auto& m = m_currentStyle.palette.materials[i];
            atlas::label(ctx, {b.x + 3.0f * pad, y},
                m.name, ctx.theme().textSecondary);
            y += rowH + pad;
        }
        if (count > 8) {
            atlas::label(ctx, {b.x + 3.0f * pad, y},
                "... " + std::to_string(count - 8) + " more",
                ctx.theme().textSecondary);
            y += rowH + pad;
        }
    }

    atlas::separator(ctx, {b.x + pad, y}, widgetW);
    y += pad;

    // Apply & Preview button
    const float btnW = 130.0f;
    if (atlas::button(ctx, "Apply & Preview", {b.x + pad, y, btnW, rowH + pad})) {
        ApplyAndPreview();
    }
    y += rowH + pad + pad;

    // Save to Library button
    if (atlas::button(ctx, "Save to Library", {b.x + pad, y, btnW, rowH + pad})) {
        SaveToLibrary();
    }
    y += rowH + pad + pad;

    atlas::separator(ctx, {b.x + pad, y}, widgetW);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, widgetW, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Style management ───────────────────────────────────────────────

void AssetStylePanel::NewStyle(const std::string& name,
                                pcg::GenerationStyleType targetType) {
    m_currentStyle            = pcg::AssetStyle{};
    m_currentStyle.name       = name;
    m_currentStyle.targetType = targetType;
    m_currentStyle.version    = 1;
    m_currentStyle.valid      = true;
    m_hasPreview              = false;
    m_log.clear();
    log("Created new asset style: " + name + " (target: "
        + pcg::GenerationStyleEngine::styleTypeName(targetType) + ")");
}

void AssetStylePanel::SetCurrentStyle(const pcg::AssetStyle& style) {
    m_currentStyle = style;
    m_hasPreview   = false;
    log("Loaded asset style: " + style.name);
}

// ── Shape editing ──────────────────────────────────────────────────

void AssetStylePanel::NewShapeProfile(const std::string& name) {
    m_currentStyle.shape = pcg::ShapeProfile{};
    m_currentStyle.shape.name      = name;
    m_currentStyle.shape.mirrorX   = false;
    m_currentStyle.shape.mirrorY   = false;
    m_currentStyle.shape.smoothing = 0.5f;
    log("Created shape profile: " + name);
}

void AssetStylePanel::AddControlPoint(const pcg::ShapeControlPoint& point) {
    m_currentStyle.shape.controlPoints.push_back(point);
    log("Added control point at ("
        + std::to_string(point.posX) + ", "
        + std::to_string(point.posY) + ", "
        + std::to_string(point.posZ) + ") weight="
        + std::to_string(point.weight));
}

bool AssetStylePanel::RemoveControlPoint(size_t index) {
    if (index >= m_currentStyle.shape.controlPoints.size()) return false;
    m_currentStyle.shape.controlPoints.erase(
        m_currentStyle.shape.controlPoints.begin() +
        static_cast<ptrdiff_t>(index));
    log("Removed control point " + std::to_string(index));
    return true;
}

bool AssetStylePanel::UpdateControlPoint(
        size_t index, const pcg::ShapeControlPoint& point) {
    if (index >= m_currentStyle.shape.controlPoints.size()) return false;
    m_currentStyle.shape.controlPoints[index] = point;
    log("Updated control point " + std::to_string(index));
    return true;
}

void AssetStylePanel::SetMirror(bool mirrorX, bool mirrorY) {
    m_currentStyle.shape.mirrorX = mirrorX;
    m_currentStyle.shape.mirrorY = mirrorY;
    log("Set mirror: X=" + std::string(mirrorX ? "on" : "off")
        + " Y=" + std::string(mirrorY ? "on" : "off"));
}

void AssetStylePanel::SetSmoothing(float smoothing) {
    m_currentStyle.shape.smoothing = smoothing;
    log("Set smoothing: " + std::to_string(smoothing));
}

// ── Palette editing ────────────────────────────────────────────────

void AssetStylePanel::NewPalette(const std::string& name) {
    m_currentStyle.palette = pcg::StylePalette{};
    m_currentStyle.palette.name             = name;
    m_currentStyle.palette.surfaceTreatment = pcg::SurfaceTreatment::None;
    m_currentStyle.palette.detailLevel      = 0.5f;
    log("Created palette: " + name);
}

void AssetStylePanel::AddColor(const pcg::StyleColor& color) {
    m_currentStyle.palette.colors.push_back(color);
    log("Added color to region: " + color.regionName);
}

bool AssetStylePanel::SetColor(size_t index, float r, float g,
                                float b, float a) {
    if (index >= m_currentStyle.palette.colors.size()) return false;
    auto& c = m_currentStyle.palette.colors[index];
    c.r = r; c.g = g; c.b = b; c.a = a;
    log("Updated color " + std::to_string(index));
    return true;
}

bool AssetStylePanel::RemoveColor(size_t index) {
    if (index >= m_currentStyle.palette.colors.size()) return false;
    m_currentStyle.palette.colors.erase(
        m_currentStyle.palette.colors.begin() +
        static_cast<ptrdiff_t>(index));
    log("Removed color " + std::to_string(index));
    return true;
}

void AssetStylePanel::AddMaterial(const pcg::StyleMaterial& material) {
    m_currentStyle.palette.materials.push_back(material);
    log("Added material: " + material.name);
}

bool AssetStylePanel::SetMaterial(size_t index,
                                   const pcg::StyleMaterial& material) {
    if (index >= m_currentStyle.palette.materials.size()) return false;
    m_currentStyle.palette.materials[index] = material;
    log("Updated material " + std::to_string(index) + ": " + material.name);
    return true;
}

bool AssetStylePanel::RemoveMaterial(size_t index) {
    if (index >= m_currentStyle.palette.materials.size()) return false;
    m_currentStyle.palette.materials.erase(
        m_currentStyle.palette.materials.begin() +
        static_cast<ptrdiff_t>(index));
    log("Removed material " + std::to_string(index));
    return true;
}

void AssetStylePanel::SetSurfaceTreatment(pcg::SurfaceTreatment treatment) {
    m_currentStyle.palette.surfaceTreatment = treatment;
    log("Set surface treatment: " +
        std::string(pcg::AssetStyleLibrary::surfaceTreatmentName(treatment)));
}

void AssetStylePanel::SetDetailLevel(float level) {
    m_currentStyle.palette.detailLevel = level;
    log("Set detail level: " + std::to_string(level));
}

// ── Preview ────────────────────────────────────────────────────────

void AssetStylePanel::ApplyAndPreview() {
    m_log.clear();

    m_pcgManager.initialize(42);
    pcg::PCGContext ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Ship, /*objectId=*/1, m_currentStyle.version);

    log("Generating preview for asset style: " + m_currentStyle.name);

    if (m_currentStyle.targetType == pcg::GenerationStyleType::ShipLayout) {
        m_previewShip = pcg::ShipGenerator::generate(ctx);
        pcg::AssetStyleLibrary::applyToShip(m_previewShip, m_currentStyle);

        std::ostringstream os;
        os << "Ship preview: " << m_previewShip.shipName
           << " | Mass: " << m_previewShip.mass
           << " | SigRadius: " << m_previewShip.signatureRadius;
        log(os.str());
        m_hasPreview = true;

    } else if (m_currentStyle.targetType ==
               pcg::GenerationStyleType::StationLayout) {
        pcg::PCGContext stCtx = m_pcgManager.makeRootContext(
            pcg::PCGDomain::Station, /*objectId=*/1,
            m_currentStyle.version);
        m_previewStation = pcg::StationGenerator::generate(stCtx);
        pcg::AssetStyleLibrary::applyToStation(
            m_previewStation, m_currentStyle);

        std::ostringstream os;
        os << "Station preview: " << m_previewStation.modules.size()
           << " modules";
        log(os.str());
        m_hasPreview = true;

    } else {
        log("Preview not yet supported for this target type");
        m_hasPreview = false;
    }
}

// ── Library ────────────────────────────────────────────────────────

void AssetStylePanel::SaveToLibrary() {
    m_currentStyle.valid = true;
    m_library.addStyle(m_currentStyle);
    log("Saved '" + m_currentStyle.name + "' to library ("
        + std::to_string(m_library.size()) + " styles total)");
}

bool AssetStylePanel::LoadFromLibrary(const std::string& name) {
    const pcg::AssetStyle* s = m_library.findStyle(name);
    if (!s) {
        log("Style '" + name + "' not found in library");
        return false;
    }
    m_currentStyle = *s;
    m_hasPreview   = false;
    log("Loaded '" + name + "' from library");
    return true;
}

// ── Serialisation ──────────────────────────────────────────────────

std::string AssetStylePanel::SaveToString() const {
    return pcg::AssetStyleLibrary::serialize(m_currentStyle);
}

void AssetStylePanel::LoadFromString(const std::string& data) {
    m_currentStyle = pcg::AssetStyleLibrary::deserialize(data);
    m_hasPreview   = false;
    log("Loaded asset style from string: " + m_currentStyle.name);
}

// ── Logging ────────────────────────────────────────────────────────

void AssetStylePanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

}
