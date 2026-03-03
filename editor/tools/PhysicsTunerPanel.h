#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * @brief Environment preset for physics simulation.
 *
 * Defines global physics parameters such as gravity, wind, and atmosphere
 * density.  Built-in presets cover common scenarios from zero-gravity to
 * turbulent wind conditions.
 */
struct PhysicsPreset {
    std::string name;
    float gravity          = 9.81f;
    float windStrength     = 0.0f;
    float windDirection[3] = {0, 0, 0};
    float atmosphereDensity = 1.0f;
};

/**
 * @brief Parameters for cloth / cape / flexible-rig simulation.
 *
 * Only meaningful when the owning object has objectType == "cloth".
 */
struct ClothParams {
    float stiffness        = 0.8f;
    float damping          = 0.3f;
    float gravityInfluence = 1.0f;
    float drag             = 0.25f;
};

/**
 * @brief One physics object tracked by the tuner.
 *
 * Covers rigidbodies, cloth, joints, and colliders.  Cloth-specific
 * parameters live in @c clothParams and are only used when
 * @c objectType == "cloth".
 */
struct PhysicsObjectEntry {
    std::string objectId;
    std::string objectName;
    std::string objectType;          // "rigidbody", "cloth", "joint", "collider"
    float mass             = 1.0f;
    float friction         = 0.5f;
    float restitution      = 0.3f;
    bool  isKinematic      = false;
    ClothParams clothParams;         // only used when objectType == "cloth"
    bool  simulationEnabled = true;
};

/**
 * @brief PhysicsTunerPanel — real-time physics parameter adjustment.
 *
 * Provides live tuning of rigidbodies, joints, colliders, and cloth
 * parameters with toggleable gravity, wind, and drag.  Ships with four
 * built-in environment presets (Zero-G, Low-G Planet, Earth-Like, Windy)
 * and supports per-object override of physics properties.
 *
 * Usage workflow:
 *   1. AddObject() to register physics objects
 *   2. ApplyPreset() or tweak active environment parameters
 *   3. SelectObject() to inspect / edit individual objects
 *   4. Use simulation controls (pause / step / resume)
 *   5. ExportToJson() / ImportFromJson() for serialisation
 */
class PhysicsTunerPanel : public EditorPanel {
public:
    PhysicsTunerPanel();
    ~PhysicsTunerPanel() override = default;

    const char* Name() const override { return "Physics Tuner"; }
    void Draw() override;

    // ── Object management ────────────────────────────────────────

    size_t AddObject(const PhysicsObjectEntry& entry);
    bool   RemoveObject(size_t index);
    bool   UpdateObject(size_t index, const PhysicsObjectEntry& entry);
    size_t ObjectCount() const { return m_objects.size(); }

    const PhysicsObjectEntry& GetObject(size_t index) const { return m_objects[index]; }
    const std::vector<PhysicsObjectEntry>& Objects() const { return m_objects; }

    // ── Selection ────────────────────────────────────────────────

    void SelectObject(int index);
    void ClearSelection();
    int  SelectedObject() const { return m_selectedIndex; }

    // ── Type filter ──────────────────────────────────────────────

    void SetTypeFilter(const std::string& filter);
    const std::string& TypeFilter() const { return m_typeFilter; }
    size_t FilteredCount() const;

    // ── Preset management ────────────────────────────────────────

    size_t PresetCount() const { return m_presets.size(); }
    const PhysicsPreset& GetPreset(size_t index) const { return m_presets[index]; }
    const std::vector<PhysicsPreset>& Presets() const { return m_presets; }
    bool ApplyPreset(size_t index);
    int  ActivePreset() const { return m_activePresetIndex; }

    // ── Active environment ───────────────────────────────────────

    const PhysicsPreset& ActiveEnvironment() const { return m_activeEnvironment; }
    void SetGravity(float gravity);
    void SetWindStrength(float strength);
    void SetAtmosphereDensity(float density);

    // ── Simulation control ───────────────────────────────────────

    bool     IsSimulationPaused() const { return m_simulationPaused; }
    void     PauseSimulation();
    void     ResumeSimulation();
    void     StepSimulation();
    uint32_t SimulationStepCount() const { return m_stepCount; }

    // ── Export / Import ──────────────────────────────────────────

    std::string ExportToJson() const;
    size_t      ImportFromJson(const std::string& json);

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<PhysicsObjectEntry> m_objects;
    std::vector<PhysicsPreset> m_presets;
    PhysicsPreset m_activeEnvironment;
    int m_activePresetIndex = 2;        // Earth-Like by default
    int m_selectedIndex     = -1;
    std::string m_typeFilter;
    bool     m_simulationPaused = false;
    uint32_t m_stepCount        = 0;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    bool matchesFilter(const PhysicsObjectEntry& entry) const;
};

} // namespace atlas::editor
