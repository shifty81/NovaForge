#include "CharacterSelectPanel.h"
#include <algorithm>
#include <sstream>
#include <cstdlib>

namespace atlas::editor {

// ── Construction ──────────────────────────────────────────────────────

CharacterSelectPanel::CharacterSelectPanel() {
    m_pcgManager.initialize(m_settings.seed);
    m_settings.referenceMeshArchive = "human.zip";
    m_meshSystem.setReferenceMeshArchive(m_settings.referenceMeshArchive);
    log("[CharacterSelect] Initialized (reference mesh: human.zip)");
}

// ── Draw ──────────────────────────────────────────────────────────────

void CharacterSelectPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    if (!atlas::panelBeginStateful(ctx, "Character Select", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // ── 3D preview placeholder ────────────────────────────────────
    // In a full GPU build the orbit camera would render a live model
    // here.  In headless / editor-only mode we show camera state.
    {
        std::ostringstream oss;
        oss << "Orbit Yaw: " << static_cast<int>(m_cameraYaw)
            << "  Pitch: " << static_cast<int>(m_cameraPitch)
            << "  Dist: " << m_cameraDistance;
        atlas::label(ctx, {b.x + pad, y}, oss.str(), ctx.theme().textPrimary);
        y += rowH + pad;
    }

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // ── Seed ──────────────────────────────────────────────────────
    atlas::label(ctx, {b.x + pad, y},
        "Seed: " + std::to_string(m_settings.seed), ctx.theme().textSecondary);
    y += rowH + pad;

    // ── Archetype dropdown ────────────────────────────────────────
    {
        static const std::vector<std::string> archetypes = {
            "Survivor", "Militia", "Civilian", "Scavenger", "Medic"
        };
        int selected = static_cast<int>(m_settings.archetype);
        if (atlas::comboBox(ctx, "Archetype",
                            {b.x + pad, y, b.w - 2.0f * pad, rowH + pad},
                            archetypes, &selected, &m_archetypeDropdownOpen)) {
            m_settings.archetype = static_cast<pcg::CharacterArchetype>(selected);
        }
        y += rowH + pad + pad;
    }

    // ── Gender toggle ─────────────────────────────────────────────
    {
        bool female = !m_settings.isMale;
        if (atlas::checkbox(ctx, "Female", {b.x + pad, y, b.w - 2.0f * pad, rowH + pad}, &female)) {
            m_settings.isMale = !female;
        }
        y += rowH + pad + pad;
    }

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // ── Body sliders ──────────────────────────────────────────────
    atlas::slider(ctx, "Height",
                  {b.x + pad, y, b.w - 2.0f * pad, rowH + pad},
                  &m_settings.height, 0.85f, 1.15f, "%.2f");
    y += rowH + pad + pad;

    atlas::slider(ctx, "Torso Width",
                  {b.x + pad, y, b.w - 2.0f * pad, rowH + pad},
                  &m_settings.torsoWidth, 0.80f, 1.20f, "%.2f");
    y += rowH + pad + pad;

    atlas::slider(ctx, "Arm Length",
                  {b.x + pad, y, b.w - 2.0f * pad, rowH + pad},
                  &m_settings.armLength, 0.85f, 1.15f, "%.2f");
    y += rowH + pad + pad;

    atlas::slider(ctx, "Leg Length",
                  {b.x + pad, y, b.w - 2.0f * pad, rowH + pad},
                  &m_settings.legLength, 0.85f, 1.15f, "%.2f");
    y += rowH + pad + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // ── Reference mesh archive ────────────────────────────────────
    atlas::label(ctx, {b.x + pad, y},
        "Mesh: " + m_settings.referenceMeshArchive, ctx.theme().textMuted);
    y += rowH + pad;

    // ── Action buttons ────────────────────────────────────────────
    float btnW = (b.w - 4.0f * pad) / 3.0f;
    if (atlas::button(ctx, "Generate", {b.x + pad, y, btnW, rowH + pad})) {
        Generate();
    }
    if (atlas::button(ctx, "Randomize", {b.x + 2.0f * pad + btnW, y, btnW, rowH + pad})) {
        Randomize();
    }
    if (atlas::button(ctx, "Clear", {b.x + 3.0f * pad + 2.0f * btnW, y, btnW, rowH + pad})) {
        ClearPreview();
    }

    atlas::panelEnd(ctx);
}

// ── Generation ────────────────────────────────────────────────────────

void CharacterSelectPanel::Generate() {
    m_pcgManager.initialize(m_settings.seed);
    auto ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Character, m_settings.seed, 1);

    // 1. Low-poly character via PCG
    m_preview.generatedCharacter =
        pcg::LowPolyCharacterGenerator::generate(
            ctx, m_settings.archetype, m_settings.isMale);

    // Attach the reference mesh archive from human.zip
    m_preview.generatedCharacter.blenderSourceArchive =
        m_settings.referenceMeshArchive;

    // 2. Character mesh system for body sliders
    m_meshSystem.setReferenceMeshArchive(m_settings.referenceMeshArchive);
    m_preview.meshCharacter =
        m_meshSystem.generateCharacter(
            static_cast<int>(m_settings.seed), !m_settings.isMale);

    // Apply slider values
    m_meshSystem.applySlider(m_preview.meshCharacter, "height",     m_settings.height);
    m_meshSystem.applySlider(m_preview.meshCharacter, "torsoWidth", m_settings.torsoWidth);
    m_meshSystem.applySlider(m_preview.meshCharacter, "armLength",  m_settings.armLength);
    m_meshSystem.applySlider(m_preview.meshCharacter, "legLength",  m_settings.legLength);

    m_preview.populated = true;

    std::ostringstream oss;
    oss << "[CharacterSelect] Generated "
        << pcg::archetypeName(m_preview.generatedCharacter.archetype)
        << " (" << (m_settings.isMale ? "Male" : "Female")
        << ") seed=" << m_settings.seed
        << " parts=" << m_preview.generatedCharacter.bodyParts.size()
        << " clothing=" << m_preview.generatedCharacter.clothing.size()
        << " ref=" << m_settings.referenceMeshArchive;
    log(oss.str());
}

void CharacterSelectPanel::Randomize() {
    m_settings.seed = static_cast<uint64_t>(std::rand()) * 31 + 7;
    Generate();
    log("[CharacterSelect] Randomized — new seed: "
        + std::to_string(m_settings.seed));
}

void CharacterSelectPanel::ClearPreview() {
    m_preview = CharacterSelectPreview{};
    log("[CharacterSelect] Preview cleared");
}

// ── Orbit camera ──────────────────────────────────────────────────────

void CharacterSelectPanel::OrbitCamera(float deltaYaw, float deltaPitch) {
    m_cameraYaw += deltaYaw;
    m_cameraPitch += deltaPitch;
    // Clamp pitch to avoid gimbal lock.
    m_cameraPitch = std::max(-89.0f, std::min(89.0f, m_cameraPitch));
}

// ── Logging ───────────────────────────────────────────────────────────

void CharacterSelectPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
