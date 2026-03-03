#include "LiveSceneManager.h"
#include <sstream>
#include <iostream>

namespace atlas::editor {

LiveSceneManager::LiveSceneManager(ViewportPanel* viewport,
                                    PCGPreviewPanel* pcgPreview)
    : m_viewport(viewport), m_pcgPreview(pcgPreview) {
    log("[LiveScene] Initialized");
}

// ── Draw (stub — real UI rendered via the editor dock layout) ────

void LiveSceneManager::Draw() {
    // The LiveSceneManager's Draw() is a no-op when headless.
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Live Scene", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Scene status
    atlas::label(ctx, {b.x + pad, y},
        m_populated ? "Scene: Populated" : "Scene: Not populated",
        m_populated ? ctx.theme().success : ctx.theme().textSecondary);
    y += rowH + pad;

    // Seed and version
    atlas::label(ctx, {b.x + pad, y},
        "Seed: " + std::to_string(m_seed) + "  Version: " + std::to_string(m_version),
        ctx.theme().textPrimary);
    y += rowH + pad;

    // Object count from viewport
    size_t objCount = m_viewport ? m_viewport->ObjectCount() : 0;
    atlas::label(ctx, {b.x + pad, y},
        "Objects: " + std::to_string(objCount), ctx.theme().textSecondary);
    y += rowH + pad;

    // Unsaved changes indicator
    if (m_overrides.IsDirty()) {
        atlas::label(ctx, {b.x + pad, y}, "* Unsaved changes", ctx.theme().warning);
        y += rowH + pad;
    }

    // Save / Load Overrides buttons
    const float btnW = 130.0f;
    if (atlas::button(ctx, "Save Overrides", {b.x + pad, y, btnW, rowH + pad})) {
        SaveOverrides();
    }
    if (atlas::button(ctx, "Load Overrides", {b.x + 2.0f * pad + btnW, y, btnW, rowH + pad})) {
        LoadOverrides();
    }
    y += rowH + pad + pad;

    // Regenerate button
    if (atlas::button(ctx, "Regenerate", {b.x + pad, y, 100.0f, rowH + pad})) {
        RegenerateScene();
    }
    y += rowH + pad + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

void LiveSceneManager::OnAssetReloaded(const std::string& assetId,
                                        const std::string& path) {
    log("[LiveScene] Asset changed: " + assetId + " — triggering regeneration");
    RegenerateScene();
}

// ── Scene population ─────────────────────────────────────────────

void LiveSceneManager::PopulateDefaultScene() {
    if (!m_viewport || !m_pcgPreview) return;

    m_viewport->ClearScene();

    // Start from clean default settings so that previous overrides
    // (e.g. overrideHull from a prior SpineHull step) do not leak into
    // subsequent calls and produce a different object count.
    PCGPreviewSettings settings{};
    settings.seed    = m_seed;
    settings.version = m_version;

    // 1. Generate a ship and load into viewport
    settings.mode = PCGPreviewMode::Ship;
    m_pcgPreview->SetSettings(settings);
    m_pcgPreview->Generate();
    if (m_pcgPreview->GetShipPreview().populated) {
        m_viewport->LoadShip(m_pcgPreview->GetShipPreview().data, m_seed);
        log("[LiveScene] Ship loaded into viewport: "
            + m_pcgPreview->GetShipPreview().data.shipName);
    }

    // 2. Generate a station and load into viewport
    settings.mode = PCGPreviewMode::Station;
    m_pcgPreview->SetSettings(settings);
    m_pcgPreview->Generate();
    if (m_pcgPreview->GetStationPreview().populated) {
        m_viewport->LoadStation(m_pcgPreview->GetStationPreview().data, m_seed);
        std::ostringstream oss;
        oss << "[LiveScene] Station loaded into viewport: "
            << m_pcgPreview->GetStationPreview().data.modules.size()
            << " modules";
        log(oss.str());
    }

    // 3. Generate a spine hull with turret placement
    settings.mode         = PCGPreviewMode::SpineHull;
    settings.overrideHull = true;
    settings.hullClass    = pcg::HullClass::Cruiser;
    m_pcgPreview->SetSettings(settings);
    m_pcgPreview->Generate();
    if (m_pcgPreview->GetSpineHullPreview().populated) {
        // Generate turret placement on top of the hull
        settings.mode = PCGPreviewMode::TurretPlacement;
        m_pcgPreview->SetSettings(settings);
        m_pcgPreview->Generate();

        const pcg::TurretPlacement* placement = nullptr;
        if (m_pcgPreview->GetTurretPlacementPreview().populated) {
            placement = &m_pcgPreview->GetTurretPlacementPreview().data;
        }
        m_viewport->LoadSpineHull(
            m_pcgPreview->GetSpineHullPreview().data, placement, m_seed);
        log("[LiveScene] SpineHull + turrets loaded into viewport");
    }

    // Apply any previously saved overrides
    applyOverridesToViewport();

    m_populated = true;

    std::ostringstream oss;
    oss << "[LiveScene] Scene populated: "
        << m_viewport->ObjectCount() << " objects (seed="
        << m_seed << " v" << m_version << ")";
    log(oss.str());
    std::cout << oss.str() << std::endl;
}

void LiveSceneManager::RegenerateScene() {
    if (!m_populated) return;

    // Capture any pending changes before regenerating
    CaptureViewportChanges();

    // Re-populate (this clears and reloads from PCG)
    PopulateDefaultScene();

    log("[LiveScene] Scene regenerated (live reload)");
}

// ── Live editing ─────────────────────────────────────────────────

void LiveSceneManager::CaptureViewportChanges() {
    if (!m_viewport) return;
    if (!m_viewport->HasPendingChanges()) return;

    // Collect viewport objects for name/type resolution
    std::vector<ViewportObject> objects;
    for (size_t i = 0; i < m_viewport->ObjectCount(); ++i) {
        objects.push_back(m_viewport->GetObject(i));
    }

    auto changes = m_viewport->CommitChanges();
    m_overrides.ImportFromViewport(changes, objects, m_seed, m_version);

    std::ostringstream oss;
    oss << "[LiveScene] Captured " << changes.size() << " viewport change(s)";
    log(oss.str());
}

// ── Persistence ──────────────────────────────────────────────────

bool LiveSceneManager::SaveOverrides(const std::string& path) {
    // Always capture latest viewport changes before saving
    CaptureViewportChanges();

    bool ok = m_overrides.SaveToFile(path);
    if (ok) {
        log("[LiveScene] Overrides saved to " + path
            + " — open the client to see changes");
    }
    return ok;
}

bool LiveSceneManager::LoadOverrides(const std::string& path) {
    bool ok = m_overrides.LoadFromFile(path);
    if (ok && m_populated) {
        applyOverridesToViewport();
        log("[LiveScene] Overrides loaded and applied from " + path);
    }
    return ok;
}

// ── Private helpers ──────────────────────────────────────────────

void LiveSceneManager::applyOverridesToViewport() {
    if (!m_viewport) return;

    for (const auto& ov : m_overrides.Overrides()) {
        // Find the viewport object by name (IDs may differ across sessions)
        for (size_t i = 0; i < m_viewport->ObjectCount(); ++i) {
            const auto& obj = m_viewport->GetObject(i);
            if (obj.name == ov.objectName && obj.type == ov.objectType) {
                m_viewport->SelectObject(obj.id);
                if (ov.field == "position") {
                    // Set absolute position by computing delta from current
                    auto& t = m_viewport->GetTransform(obj.id);
                    m_viewport->TranslateSelected(
                        ov.values[0] - t.posX,
                        ov.values[1] - t.posY,
                        ov.values[2] - t.posZ);
                } else if (ov.field == "rotation") {
                    auto& t = m_viewport->GetTransform(obj.id);
                    m_viewport->RotateSelected(
                        ov.values[0] - t.rotX,
                        ov.values[1] - t.rotY,
                        ov.values[2] - t.rotZ);
                } else if (ov.field == "scale") {
                    auto& t = m_viewport->GetTransform(obj.id);
                    m_viewport->ScaleSelected(
                        ov.values[0] - t.scaleX,
                        ov.values[1] - t.scaleY,
                        ov.values[2] - t.scaleZ);
                }
                break;
            }
        }
    }

    // Clear the changes generated by applying overrides so they don't
    // count as new user edits.
    m_viewport->CommitChanges();
    m_viewport->DeselectAll();
}

void LiveSceneManager::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
