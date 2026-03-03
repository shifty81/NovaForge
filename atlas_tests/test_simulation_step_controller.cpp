/**
 * Tests for SimulationStepController:
 *   - Default state (running)
 *   - Pause / resume
 *   - ShouldTick in running mode
 *   - ShouldTick in paused mode
 *   - StepOnce advances one tick then pauses
 *   - Multiple StepOnce accumulates
 *   - StepN advances N ticks
 *   - StepN with zero is no-op
 *   - Resume clears pending steps
 *   - TotalTicks counter
 *   - Reset
 */

#include <cassert>
#include "../cpp_client/include/editor/simulation_step_controller.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// SimulationStepController tests
// ══════════════════════════════════════════════════════════════════

void test_sim_ctrl_default_running() {
    SimulationStepController ctrl;
    assert(ctrl.IsRunning());
    assert(!ctrl.IsPaused());
    assert(ctrl.Mode() == SimulationMode::Running);
    assert(ctrl.TotalTicks() == 0);
    assert(ctrl.StepsRemaining() == 0);
}

void test_sim_ctrl_pause_resume() {
    SimulationStepController ctrl;
    ctrl.Pause();
    assert(ctrl.IsPaused());
    assert(!ctrl.IsRunning());
    assert(ctrl.Mode() == SimulationMode::Paused);

    ctrl.Resume();
    assert(ctrl.IsRunning());
    assert(!ctrl.IsPaused());
}

void test_sim_ctrl_should_tick_running() {
    SimulationStepController ctrl;
    assert(ctrl.ShouldTick());
    assert(ctrl.ShouldTick());
    assert(ctrl.ShouldTick());
    assert(ctrl.TotalTicks() == 3);
}

void test_sim_ctrl_should_tick_paused() {
    SimulationStepController ctrl;
    ctrl.Pause();
    assert(!ctrl.ShouldTick());
    assert(!ctrl.ShouldTick());
    assert(ctrl.TotalTicks() == 0);
}

void test_sim_ctrl_step_once() {
    SimulationStepController ctrl;
    ctrl.Pause();
    ctrl.StepOnce();

    assert(ctrl.Mode() == SimulationMode::Stepping);
    assert(ctrl.StepsRemaining() == 1);

    assert(ctrl.ShouldTick()); // tick fires
    assert(ctrl.TotalTicks() == 1);
    assert(ctrl.IsPaused());   // auto-paused after step
    assert(ctrl.StepsRemaining() == 0);

    assert(!ctrl.ShouldTick()); // stays paused
}

void test_sim_ctrl_step_once_accumulate() {
    SimulationStepController ctrl;
    ctrl.Pause();
    ctrl.StepOnce();
    ctrl.StepOnce();
    assert(ctrl.StepsRemaining() == 2);

    assert(ctrl.ShouldTick());
    assert(ctrl.StepsRemaining() == 1);
    assert(ctrl.Mode() == SimulationMode::Stepping);

    assert(ctrl.ShouldTick());
    assert(ctrl.StepsRemaining() == 0);
    assert(ctrl.IsPaused());
    assert(ctrl.TotalTicks() == 2);
}

void test_sim_ctrl_step_n() {
    SimulationStepController ctrl;
    ctrl.Pause();
    ctrl.StepN(3);
    assert(ctrl.StepsRemaining() == 3);

    assert(ctrl.ShouldTick());
    assert(ctrl.ShouldTick());
    assert(ctrl.ShouldTick());
    assert(ctrl.IsPaused());
    assert(ctrl.TotalTicks() == 3);
    assert(!ctrl.ShouldTick());
}

void test_sim_ctrl_step_n_zero() {
    SimulationStepController ctrl;
    ctrl.Pause();
    ctrl.StepN(0);
    assert(ctrl.IsPaused());
    assert(ctrl.StepsRemaining() == 0);
}

void test_sim_ctrl_resume_clears_steps() {
    SimulationStepController ctrl;
    ctrl.Pause();
    ctrl.StepN(5);
    assert(ctrl.StepsRemaining() == 5);

    ctrl.Resume();
    assert(ctrl.IsRunning());
    assert(ctrl.StepsRemaining() == 0);
}

void test_sim_ctrl_total_ticks() {
    SimulationStepController ctrl;
    ctrl.ShouldTick(); // 1
    ctrl.ShouldTick(); // 2
    ctrl.Pause();
    ctrl.ShouldTick(); // paused, no tick
    ctrl.StepOnce();
    ctrl.ShouldTick(); // 3
    assert(ctrl.TotalTicks() == 3);
}

void test_sim_ctrl_reset() {
    SimulationStepController ctrl;
    ctrl.Pause();
    ctrl.StepN(5);
    ctrl.ShouldTick();
    ctrl.ShouldTick();

    ctrl.Reset();
    assert(ctrl.IsRunning());
    assert(ctrl.StepsRemaining() == 0);
    assert(ctrl.TotalTicks() == 0);
}
