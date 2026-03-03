#pragma once
/**
 * @file simulation_step_controller.h
 * @brief Pause / resume / single-step control for the simulation loop.
 *
 * SimulationStepController sits between the editor and the tick
 * scheduler, allowing the designer to freeze the simulation mid-frame,
 * advance one tick at a time, or resume normal execution.  This is
 * critical for debugging physics interactions, animation blending, and
 * AI behaviour in the live editor.
 *
 * It does not modify the DeltaEditStore — it only gates whether the
 * simulation tick fires.
 */

#include <cstdint>

namespace atlas::editor {

/** Simulation execution mode. */
enum class SimulationMode : uint8_t {
    Running,   ///< Normal continuous execution
    Paused,    ///< Simulation frozen
    Stepping   ///< Advancing exactly one tick then re-pausing
};

/**
 * SimulationStepController — manages pause / resume / step workflow.
 *
 * Usage:
 *   SimulationStepController ctrl;
 *   ctrl.Pause();                    // freeze
 *   ctrl.StepOnce();                 // advance one tick
 *   if (ctrl.ShouldTick()) { ... }   // returns true once then re-pauses
 *   ctrl.Resume();                   // back to continuous
 */
class SimulationStepController {
public:
    /** Current mode. */
    SimulationMode Mode() const { return m_mode; }

    /** Whether the simulation is currently running (not paused). */
    bool IsRunning() const { return m_mode == SimulationMode::Running; }

    /** Whether the simulation is paused. */
    bool IsPaused() const { return m_mode == SimulationMode::Paused; }

    /** Pause the simulation. */
    void Pause() {
        m_mode = SimulationMode::Paused;
    }

    /** Resume continuous execution. */
    void Resume() {
        m_mode = SimulationMode::Running;
        m_stepsRemaining = 0;
    }

    /**
     * Request a single tick advance.  The simulation will execute
     * exactly one tick and then return to the Paused state.
     * Multiple calls accumulate: StepOnce(); StepOnce(); → 2 ticks.
     */
    void StepOnce() {
        m_stepsRemaining++;
        m_mode = SimulationMode::Stepping;
    }

    /**
     * Request N ticks of advance.  Accumulates with any pending steps.
     */
    void StepN(uint32_t n) {
        if (n == 0) return;
        m_stepsRemaining += n;
        m_mode = SimulationMode::Stepping;
    }

    /** Number of ticks remaining in a step sequence. */
    uint32_t StepsRemaining() const { return m_stepsRemaining; }

    /**
     * Called by the tick scheduler each frame.  Returns true if the
     * simulation should execute this tick.
     *
     * - Running → always returns true.
     * - Paused  → always returns false.
     * - Stepping → returns true and decrements step count; when
     *   count reaches zero, transitions to Paused.
     */
    bool ShouldTick() {
        switch (m_mode) {
            case SimulationMode::Running:
                m_totalTicks++;
                return true;
            case SimulationMode::Paused:
                return false;
            case SimulationMode::Stepping:
                if (m_stepsRemaining > 0) {
                    --m_stepsRemaining;
                    m_totalTicks++;
                    if (m_stepsRemaining == 0)
                        m_mode = SimulationMode::Paused;
                    return true;
                }
                m_mode = SimulationMode::Paused;
                return false;
        }
        return false;
    }

    /** Total ticks that have been allowed through. */
    uint64_t TotalTicks() const { return m_totalTicks; }

    /** Reset to initial running state with zero counters. */
    void Reset() {
        m_mode = SimulationMode::Running;
        m_stepsRemaining = 0;
        m_totalTicks = 0;
    }

private:
    SimulationMode m_mode = SimulationMode::Running;
    uint32_t m_stepsRemaining = 0;
    uint64_t m_totalTicks = 0;
};

} // namespace atlas::editor
