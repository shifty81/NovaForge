#include "pcg/constraint_solver.h"

namespace atlas {
namespace pcg {

// ── PowerGridConstraint ─────────────────────────────────────────────

PowerGridConstraint::PowerGridConstraint(float usedPower, float availablePower)
    : usedPower_(usedPower)
    , availablePower_(availablePower)
{}

bool PowerGridConstraint::evaluate() const {
    return usedPower_ <= availablePower_ * 0.85f;
}

const char* PowerGridConstraint::name() const {
    return "PowerGrid";
}

void PowerGridConstraint::setUsedPower(float p)      { usedPower_ = p; }
void PowerGridConstraint::setAvailablePower(float p)  { availablePower_ = p; }

// ── CPUConstraint ───────────────────────────────────────────────────

CPUConstraint::CPUConstraint(float usedCPU, float availableCPU)
    : usedCPU_(usedCPU)
    , availableCPU_(availableCPU)
{}

bool CPUConstraint::evaluate() const {
    return usedCPU_ <= availableCPU_;
}

const char* CPUConstraint::name() const {
    return "CPU";
}

void CPUConstraint::setUsedCPU(float c)      { usedCPU_ = c; }
void CPUConstraint::setAvailableCPU(float c)  { availableCPU_ = c; }

// ── MinDPSConstraint ────────────────────────────────────────────────

MinDPSConstraint::MinDPSConstraint(float currentDPS, float requiredDPS)
    : currentDPS_(currentDPS)
    , requiredDPS_(requiredDPS)
{}

bool MinDPSConstraint::evaluate() const {
    return currentDPS_ >= requiredDPS_;
}

const char* MinDPSConstraint::name() const {
    return "MinDPS";
}

void MinDPSConstraint::setCurrentDPS(float d)  { currentDPS_ = d; }
void MinDPSConstraint::setRequiredDPS(float d)  { requiredDPS_ = d; }

// ── ConstraintSolver ────────────────────────────────────────────────

ConstraintSolver::ConstraintSolver()
    : mutateFn_(nullptr)
    , fallbackFn_(nullptr)
{}

void ConstraintSolver::add(IConstraint* constraint) {
    if (constraint) {
        constraints_.push_back(constraint);
    }
}

void ConstraintSolver::setMutator(MutateFn fn) {
    mutateFn_ = fn;
}

void ConstraintSolver::setFallback(FallbackFn fn) {
    fallbackFn_ = fn;
}

bool ConstraintSolver::allPass() const {
    for (const auto* c : constraints_) {
        if (!c->evaluate()) return false;
    }
    return true;
}

ConstraintResult ConstraintSolver::solve(DeterministicRNG& rng,
                                          int maxRetries) {
    ConstraintResult result{ false, 0, {} };

    for (int i = 0; i <= maxRetries; ++i) {
        // Check all constraints.
        bool ok = true;
        for (const auto* c : constraints_) {
            if (!c->evaluate()) {
                result.failedName = c->name();
                ok = false;
                break;
            }
        }

        if (ok) {
            result.solved  = true;
            result.retries = i;
            result.failedName.clear();
            return result;
        }

        // Mutate and retry.
        if (mutateFn_ && i < maxRetries) {
            mutateFn_(rng);
        }
        result.retries = i + 1;
    }

    // All retries exhausted — apply fallback.
    if (fallbackFn_) {
        fallbackFn_();
    }

    return result;
}

} // namespace pcg
} // namespace atlas
