#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_server/include/pcg/pcg_context.h"
#include "../../cpp_server/include/pcg/pcg_manager.h"
#include "../../cpp_server/include/pcg/lowpoly_character_generator.h"
#include "../../cpp_client/include/characters/character_mesh_system.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * @brief Editable character attributes exposed through the editor UI.
 *
 * Aggregates the customisation values that drive the CharacterMeshSystem
 * body sliders and the LowPolyCharacterGenerator archetype / gender
 * selection.  Also stores the orbit camera state for the rotatable 3D
 * character preview.
 */
struct CharacterSelectSettings {
    // ── Character identity ────────────────────────────────────
    uint64_t seed = 42;

    // ── Archetype / gender ────────────────────────────────────
    pcg::CharacterArchetype archetype = pcg::CharacterArchetype::Survivor;
    bool isMale = true;

    // ── Body sliders (from CharacterMeshSystem) ───────────────
    float height     = 1.0f;   ///< 0.85 – 1.15
    float torsoWidth = 1.0f;   ///< 0.80 – 1.20
    float armLength  = 1.0f;   ///< 0.85 – 1.15
    float legLength  = 1.0f;   ///< 0.85 – 1.15

    // ── Reference mesh archive ────────────────────────────────
    std::string referenceMeshArchive;  ///< Path to human.zip or similar.
};

/**
 * @brief Snapshot of the generated character shown in the preview area.
 */
struct CharacterSelectPreview {
    pcg::GeneratedLowPolyCharacter generatedCharacter{};
    atlas::AstronautCharacter      meshCharacter{};
    bool populated = false;
};

/**
 * @brief Editor panel providing a rotatable 3D character preview and an
 *        attribute editor for character creation / selection.
 *
 * Features:
 *   - Orbit camera (yaw / pitch) for rotating the 3D character render.
 *   - Body customisation sliders (height, torso width, arm/leg length).
 *   - Archetype and gender dropdowns.
 *   - Seed control for deterministic character generation.
 *   - Integration with human.zip as the reference mesh archive.
 *   - Connects CharacterMeshSystem (body sliders) and
 *     LowPolyCharacterGenerator (PCG assembly).
 */
class CharacterSelectPanel : public EditorPanel {
public:
    CharacterSelectPanel();

    const char* Name() const override { return "Character Select"; }
    void Draw() override;

    // ── Settings access ──────────────────────────────────────────
    const CharacterSelectSettings& Settings() const { return m_settings; }
    void SetSettings(const CharacterSelectSettings& s) { m_settings = s; }

    // ── Generation ───────────────────────────────────────────────
    /** Generate (or regenerate) the character with current settings. */
    void Generate();

    /** Randomize the seed and regenerate. */
    void Randomize();

    /** Clear the preview. */
    void ClearPreview();

    // ── Preview access ───────────────────────────────────────────
    const CharacterSelectPreview& GetPreview() const { return m_preview; }

    // ── Orbit camera ─────────────────────────────────────────────
    void  OrbitCamera(float deltaYaw, float deltaPitch);
    float GetCameraYaw()   const { return m_cameraYaw; }
    float GetCameraPitch() const { return m_cameraPitch; }
    void  SetCameraDistance(float d) { m_cameraDistance = d; }
    float GetCameraDistance() const { return m_cameraDistance; }

    // ── Log ──────────────────────────────────────────────────────
    const std::vector<std::string>& Log() const { return m_log; }

private:
    CharacterSelectSettings m_settings;
    CharacterSelectPreview  m_preview;

    pcg::PCGManager          m_pcgManager;
    atlas::CharacterMeshSystem m_meshSystem;

    // Orbit camera state
    float m_cameraYaw      = 0.0f;
    float m_cameraPitch    = 15.0f;
    float m_cameraDistance  = 3.0f;

    // UI state
    atlas::PanelState m_panelState;
    bool  m_archetypeDropdownOpen = false;
    float m_scrollOffset = 0.0f;

    std::vector<std::string> m_log;

    void log(const std::string& msg);
};

} // namespace atlas::editor
