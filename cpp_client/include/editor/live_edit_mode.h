#pragma once
/**
 * @file live_edit_mode.h
 * @brief Live Edit Mode Controller — edit objects while the simulation runs.
 *
 * LiveEditMode is an ITool that allows designers to modify entities without
 * pausing the game simulation.  Three policies control how the simulation
 * behaves during an active edit session:
 *
 *   - PauseOnEdit  — traditional: freeze the simulation while editing.
 *   - SlowMotion   — reduce the tick rate so changes can be observed.
 *   - FullSpeed    — simulation continues unmodified.
 *
 * Policy and time-scale changes are undoable and recorded in the
 * DeltaEditStore as SetProperty edits on entityID 0.
 */

#include "editor/itool.h"
#include "editor/undoable_command_bus.h"
#include "editor/simulation_step_controller.h"
#include "../../engine/ecs/DeltaEditStore.h"
#include <string>
#include <cstdint>
#include <algorithm>

namespace atlas::editor {

/** Determines simulation behaviour while the designer is editing. */
enum class LiveEditPolicy : uint8_t {
    PauseOnEdit,  ///< Pause simulation when editing (default)
    SlowMotion,   ///< Slow the simulation during editing (reduced tick rate)
    FullSpeed     ///< Simulation continues at full speed during edits
};

/** Returns the string name for a LiveEditPolicy value. */
inline const char* LiveEditPolicyName(LiveEditPolicy p) {
    switch (p) {
        case LiveEditPolicy::PauseOnEdit: return "PauseOnEdit";
        case LiveEditPolicy::SlowMotion:  return "SlowMotion";
        case LiveEditPolicy::FullSpeed:   return "FullSpeed";
    }
    return "PauseOnEdit";
}

/** Parses a policy name string back to the enum value. */
inline LiveEditPolicy LiveEditPolicyFromName(const std::string& name) {
    if (name == "SlowMotion") return LiveEditPolicy::SlowMotion;
    if (name == "FullSpeed")  return LiveEditPolicy::FullSpeed;
    return LiveEditPolicy::PauseOnEdit;
}

// ─────────────────────────────────────────────────────────────────────────────
// Commands
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Undoable command: change the active LiveEditPolicy.
 *
 * Records a SetProperty DeltaEdit on entityID 0 with
 * propertyName = "live_edit_policy".
 */
class SetLiveEditPolicyCommand : public IUndoableCommand {
public:
    SetLiveEditPolicyCommand(atlas::ecs::DeltaEditStore& store,
                             LiveEditPolicy& policyRef,
                             LiveEditPolicy oldPolicy,
                             LiveEditPolicy newPolicy)
        : m_store(store), m_policyRef(policyRef),
          m_oldPolicy(oldPolicy), m_newPolicy(newPolicy) {}

    void Execute() override {
        m_policyRef = m_newPolicy;
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = 0;
        edit.propertyName  = "live_edit_policy";
        edit.propertyValue = LiveEditPolicyName(m_newPolicy);
        m_store.Record(edit);
    }

    void Undo() override {
        m_policyRef = m_oldPolicy;
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = 0;
        edit.propertyName  = "live_edit_policy";
        edit.propertyValue = LiveEditPolicyName(m_oldPolicy);
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set Live Edit Policy"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    LiveEditPolicy& m_policyRef;
    LiveEditPolicy  m_oldPolicy;
    LiveEditPolicy  m_newPolicy;
};

/**
 * Undoable command: change the time-scale factor.
 *
 * Records a SetProperty DeltaEdit on entityID 0 with
 * propertyName = "time_scale".
 */
class SetTimeScaleCommand : public IUndoableCommand {
public:
    SetTimeScaleCommand(atlas::ecs::DeltaEditStore& store,
                        float& timeScaleRef,
                        float oldScale,
                        float newScale)
        : m_store(store), m_timeScaleRef(timeScaleRef),
          m_oldScale(oldScale), m_newScale(newScale) {}

    void Execute() override {
        m_timeScaleRef = m_newScale;
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = 0;
        edit.propertyName  = "time_scale";
        edit.propertyValue = std::to_string(m_newScale);
        m_store.Record(edit);
    }

    void Undo() override {
        m_timeScaleRef = m_oldScale;
        atlas::ecs::DeltaEdit edit{};
        edit.type          = atlas::ecs::DeltaEditType::SetProperty;
        edit.entityID      = 0;
        edit.propertyName  = "time_scale";
        edit.propertyValue = std::to_string(m_oldScale);
        m_store.Record(edit);
    }

    const char* Description() const override { return "Set Time Scale"; }

private:
    atlas::ecs::DeltaEditStore& m_store;
    float& m_timeScaleRef;
    float  m_oldScale;
    float  m_newScale;
};

// ─────────────────────────────────────────────────────────────────────────────
// Tool
// ─────────────────────────────────────────────────────────────────────────────

/**
 * LiveEditMode — concrete ITool for editing while the simulation runs.
 *
 * Manages an edit session lifecycle (BeginEdit / EndEdit) and adjusts
 * the SimulationStepController according to the active LiveEditPolicy.
 * Policy and time-scale changes are posted as undoable commands and
 * recorded in the DeltaEditStore.
 */
class LiveEditMode : public ITool {
public:
    LiveEditMode(UndoableCommandBus& bus,
                 atlas::ecs::DeltaEditStore& store,
                 SimulationStepController& simCtrl)
        : m_bus(bus), m_store(store), m_simCtrl(simCtrl) {}

    // ── ITool interface ─────────────────────────────────────────────
    const char* Name() const override { return "Live Edit Mode"; }

    void Activate() override   { m_active = true; }
    void Deactivate() override { m_active = false; }
    void Update(float /*dt*/) override {}
    bool IsActive() const override { return m_active; }

    // ── Policy ──────────────────────────────────────────────────────

    LiveEditPolicy Policy() const { return m_policy; }

    void SetPolicy(LiveEditPolicy newPolicy) {
        if (newPolicy == m_policy) return;
        m_bus.PostCommand(std::make_unique<SetLiveEditPolicyCommand>(
            m_store, m_policy, m_policy, newPolicy));
    }

    // ── Time scale ──────────────────────────────────────────────────

    float TimeScale() const { return m_timeScale; }

    void SetTimeScale(float scale) {
        scale = std::clamp(scale, 0.0f, 1.0f);
        if (scale == m_timeScale) return;
        m_bus.PostCommand(std::make_unique<SetTimeScaleCommand>(
            m_store, m_timeScale, m_timeScale, scale));
    }

    // ── Edit session ────────────────────────────────────────────────

    bool IsEditing() const { return m_active && m_editing; }

    void BeginEdit() {
        m_editing = true;
        switch (m_policy) {
            case LiveEditPolicy::PauseOnEdit:
                m_simCtrl.Pause();
                break;
            case LiveEditPolicy::SlowMotion:
                SetTimeScale(0.25f);
                break;
            case LiveEditPolicy::FullSpeed:
                break;
        }
        ++m_editCount;
    }

    void EndEdit() {
        m_editing = false;
        switch (m_policy) {
            case LiveEditPolicy::PauseOnEdit:
                m_simCtrl.Resume();
                break;
            case LiveEditPolicy::SlowMotion:
                SetTimeScale(1.0f);
                break;
            case LiveEditPolicy::FullSpeed:
                break;
        }
    }

    /**
     * Effective time-scale accounting for the current edit state:
     *   - Editing + PauseOnEdit → 0.0
     *   - Editing + SlowMotion  → m_timeScale * 0.25
     *   - Otherwise             → m_timeScale
     */
    float EffectiveTimeScale() const {
        if (m_editing) {
            if (m_policy == LiveEditPolicy::PauseOnEdit) return 0.0f;
            if (m_policy == LiveEditPolicy::SlowMotion)  return m_timeScale * 0.25f;
        }
        return m_timeScale;
    }

    /** Number of completed BeginEdit/EndEdit cycles. */
    uint32_t EditCount() const { return m_editCount; }

private:
    UndoableCommandBus&          m_bus;
    atlas::ecs::DeltaEditStore&  m_store;
    SimulationStepController&    m_simCtrl;

    LiveEditPolicy m_policy    = LiveEditPolicy::PauseOnEdit;
    float          m_timeScale = 1.0f;
    bool           m_editing   = false;
    bool           m_active    = false;
    uint32_t       m_editCount = 0;
};

} // namespace atlas::editor
