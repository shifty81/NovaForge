#ifndef EVE_PCG_CONSTRAINT_SOLVER_H
#define EVE_PCG_CONSTRAINT_SOLVER_H

#include "deterministic_rng.h"
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace atlas {
namespace pcg {

/**
 * @brief Abstract constraint evaluated during PCG assembly.
 *
 * Concrete constraints (PowerGrid, CPU, FireArc …) inherit from this
 * and implement Evaluate().  The ConstraintSolver retries with derived
 * seeds when constraints fail, and falls back to a safe default if
 * all retries are exhausted.
 */
class IConstraint {
public:
    virtual ~IConstraint() = default;

    /** @return true when the constraint is satisfied. */
    virtual bool evaluate() const = 0;

    /** Human-readable constraint name (for debug traces). */
    virtual const char* name() const = 0;
};

// ── Concrete constraints ────────────────────────────────────────────

/**
 * @brief Ensures total weapon power draw ≤ 85 % of powergrid output.
 */
class PowerGridConstraint : public IConstraint {
public:
    PowerGridConstraint(float usedPower, float availablePower);

    bool        evaluate() const override;
    const char* name()     const override;

    void setUsedPower(float p);
    void setAvailablePower(float p);

private:
    float usedPower_;
    float availablePower_;
};

/**
 * @brief Ensures total CPU usage ≤ available CPU.
 */
class CPUConstraint : public IConstraint {
public:
    CPUConstraint(float usedCPU, float availableCPU);

    bool        evaluate() const override;
    const char* name()     const override;

    void setUsedCPU(float c);
    void setAvailableCPU(float c);

private:
    float usedCPU_;
    float availableCPU_;
};

/**
 * @brief Ensures minimum forward DPS meets ship-class requirement.
 */
class MinDPSConstraint : public IConstraint {
public:
    MinDPSConstraint(float currentDPS, float requiredDPS);

    bool        evaluate() const override;
    const char* name()     const override;

    void setCurrentDPS(float d);
    void setRequiredDPS(float d);

private:
    float currentDPS_;
    float requiredDPS_;
};

// ── Solver ──────────────────────────────────────────────────────────

/**
 * @brief Result of a solver pass.
 */
struct ConstraintResult {
    bool        solved;     ///< true if all constraints passed.
    int         retries;    ///< Number of retries consumed.
    std::string failedName; ///< Name of last failing constraint (if any).
};

/**
 * @brief Iterative constraint solver with deterministic retries.
 *
 * Usage:
 *   1. Register constraints via add().
 *   2. Call solve() with an RNG — the solver will mutate state and
 *      retry up to maxRetries times.
 *   3. If all retries fail the solver calls applyFallback().
 *
 * Mutation strategy (caller-provided callback):
 *   - Remove highest-cost weapon
 *   - Downgrade module tier
 *   - Add auxiliary power
 *   - Reduce turret count
 */
class ConstraintSolver {
public:
    /** Callback invoked on each failed retry to mutate the fitting. */
    using MutateFn   = void(*)(DeterministicRNG& rng);
    /** Callback invoked when all retries are exhausted. */
    using FallbackFn = void(*)();

    ConstraintSolver();

    /** Add a constraint (solver does NOT take ownership). */
    void add(IConstraint* constraint);

    /** Set the mutation function called on each retry. */
    void setMutator(MutateFn fn);

    /** Set the fallback function called when all retries fail. */
    void setFallback(FallbackFn fn);

    /**
     * @brief Run the solver.
     * @param rng         Deterministic RNG for mutation decisions.
     * @param maxRetries  Maximum number of retry iterations (default 8).
     * @return Result indicating success, retry count, and any failure.
     */
    ConstraintResult solve(DeterministicRNG& rng, int maxRetries = 8);

    /** @return true if every registered constraint passes right now. */
    bool allPass() const;

private:
    std::vector<IConstraint*> constraints_;
    MutateFn                  mutateFn_;
    FallbackFn                fallbackFn_;
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_CONSTRAINT_SOLVER_H
