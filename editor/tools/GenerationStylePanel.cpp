#include "GenerationStylePanel.h"
#include <sstream>
#include <fstream>
#include <filesystem>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

GenerationStylePanel::GenerationStylePanel() {
    // Start with a default ship layout style.
    m_style = pcg::GenerationStyleEngine::createDefaultStyle(
        pcg::GenerationStyleType::ShipLayout);
    m_pcgManager.initialize(m_style.baseSeed);
}

// ── Draw (stub — real UI via Atlas UI) ─────────────────────────────

void GenerationStylePanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Generation Style", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Style name
    atlas::label(ctx, {b.x + pad, y}, "Style: " + m_style.name, ctx.theme().textPrimary);
    y += rowH + pad;

    // Placement count
    atlas::label(ctx, {b.x + pad, y},
        "Placements: " + std::to_string(m_style.placements.size()), ctx.theme().textSecondary);
    y += rowH + pad;

    // Parameter count
    atlas::label(ctx, {b.x + pad, y},
        "Parameters: " + std::to_string(m_style.parameters.size()), ctx.theme().textSecondary);
    y += rowH + pad;

    // Generate button
    const float btnW = 100.0f;
    if (atlas::button(ctx, "Generate", {b.x + pad, y, btnW, rowH + pad})) {
        Generate();
    }
    y += rowH + pad + pad;

    // Save / Load buttons
    float halfW = (b.w - 3.0f * pad) * 0.5f;
    if (atlas::button(ctx, "Save", {b.x + pad, y, halfW, rowH + pad})) {
        SaveStyleToFile();
    }
    if (atlas::button(ctx, "Load", {b.x + 2.0f * pad + halfW, y, halfW, rowH + pad})) {
        LoadStyleFromFile();
    }
    y += rowH + pad + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Style management ───────────────────────────────────────────────

void GenerationStylePanel::NewStyle(pcg::GenerationStyleType type,
                                     const std::string& name) {
    m_style = pcg::GenerationStyleEngine::createDefaultStyle(type, name);
    m_hasAssetStyle = false;
    m_assetStyle    = pcg::AssetStyle{};
    m_result        = pcg::StyleGenerationResult{};
    m_log.clear();
    log("Created new " + std::string(
        pcg::GenerationStyleEngine::styleTypeName(type)) + " style"
        + (name.empty() ? "" : (": " + name)));
}

// ── Placement editing ──────────────────────────────────────────────

void GenerationStylePanel::AddPlacement(const pcg::PlacementEntry& entry) {
    // Replace if slot already exists.
    for (auto& pe : m_style.placements) {
        if (pe.slotIndex == entry.slotIndex) {
            pe = entry;
            log("Updated placement at slot " +
                std::to_string(entry.slotIndex));
            return;
        }
    }
    m_style.placements.push_back(entry);
    log("Added placement at slot " + std::to_string(entry.slotIndex)
        + ": " + entry.label);
}

bool GenerationStylePanel::RemovePlacement(uint32_t slotIndex) {
    for (auto it = m_style.placements.begin();
         it != m_style.placements.end(); ++it) {
        if (it->slotIndex == slotIndex) {
            m_style.placements.erase(it);
            log("Removed placement at slot " + std::to_string(slotIndex));
            return true;
        }
    }
    return false;
}

// ── Parameter editing ──────────────────────────────────────────────

bool GenerationStylePanel::SetParameter(const std::string& name,
                                          float value) {
    for (auto& p : m_style.parameters) {
        if (p.name == name) {
            p.value = value;
            log("Set parameter " + name + " = " + std::to_string(value));
            return true;
        }
    }
    return false;
}

bool GenerationStylePanel::EnableParameter(const std::string& name,
                                             bool enabled) {
    for (auto& p : m_style.parameters) {
        if (p.name == name) {
            p.enabled = enabled;
            log(std::string(enabled ? "Enabled" : "Disabled")
                + " parameter " + name);
            return true;
        }
    }
    return false;
}

// ── Asset style attachment ─────────────────────────────────────────

void GenerationStylePanel::AttachAssetStyle(const pcg::AssetStyle& style) {
    m_assetStyle    = style;
    m_hasAssetStyle = true;
    log("Attached asset style: " + style.name);
}

void GenerationStylePanel::DetachAssetStyle() {
    m_hasAssetStyle = false;
    m_assetStyle    = pcg::AssetStyle{};
    log("Detached asset style");
}

// ── Generation ─────────────────────────────────────────────────────

void GenerationStylePanel::Generate() {
    m_log.clear();

    // Validate the style first.
    if (!pcg::GenerationStyleEngine::validate(m_style)) {
        log("ERROR: Style validation failed");
        m_result          = pcg::StyleGenerationResult{};
        m_result.success  = false;
        m_result.errorMessage = "Style validation failed";
        return;
    }

    // Re-initialise the manager with the style's base seed.
    m_pcgManager.initialize(m_style.baseSeed);

    pcg::PCGContext ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Ship, /*objectId=*/1, m_style.version);

    log("Generating " +
        std::string(pcg::GenerationStyleEngine::styleTypeName(m_style.type))
        + " with seed " + std::to_string(m_style.baseSeed)
        + " (v" + std::to_string(m_style.version) + ")");
    log("  Placements: " + std::to_string(m_style.placements.size()));
    log("  Parameters: " + std::to_string(m_style.parameters.size()));

    m_result = pcg::GenerationStyleEngine::generate(ctx, m_style);

    // Apply asset style if attached.
    if (m_hasAssetStyle) {
        log("Applying asset style: " + m_assetStyle.name);

        if (m_style.type == pcg::GenerationStyleType::ShipLayout) {
            pcg::AssetStyleLibrary::applyToShip(
                m_result.shipResult, m_assetStyle);
        } else if (m_style.type == pcg::GenerationStyleType::StationLayout) {
            pcg::AssetStyleLibrary::applyToStation(
                m_result.stationResult, m_assetStyle);
        }
    }

    // Summary log.
    std::ostringstream os;
    os << "Result: " << (m_result.success ? "SUCCESS" : "FAILED");
    if (!m_result.errorMessage.empty())
        os << " (" << m_result.errorMessage << ")";
    os << " | Placements applied: " << m_result.placementsApplied
       << " | Parameters applied: " << m_result.parametersApplied;
    log(os.str());
}

// ── Serialisation ──────────────────────────────────────────────────

std::string GenerationStylePanel::SaveStyleToString() const {
    return pcg::GenerationStyleEngine::serialize(m_style);
}

void GenerationStylePanel::LoadStyleFromString(const std::string& data) {
    m_style = pcg::GenerationStyleEngine::deserialize(data);
    m_log.clear();
    log("Loaded style: " + m_style.name);
}

bool GenerationStylePanel::SaveStyleToFile(const std::string& path) {
    std::filesystem::path fspath(path);
    if (fspath.has_parent_path()) {
        std::filesystem::create_directories(fspath.parent_path());
    }

    std::ofstream out(path);
    if (!out.is_open()) {
        log("ERROR: Could not open " + path + " for writing");
        return false;
    }

    out << SaveStyleToString();
    out.close();
    log("Saved style to " + path);
    return true;
}

bool GenerationStylePanel::LoadStyleFromFile(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        log("File not found: " + path);
        return false;
    }

    std::ifstream in(path);
    if (!in.is_open()) {
        log("ERROR: Could not open " + path + " for reading");
        return false;
    }

    std::string data((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    in.close();
    LoadStyleFromString(data);
    return true;
}

// ── Logging ────────────────────────────────────────────────────────

void GenerationStylePanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

}
