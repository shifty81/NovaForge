#include "systems/titan_assembly_system.h"
#include <algorithm>

namespace atlas {
namespace systems {

// ── Phase thresholds ───────────────────────────────────────────────
static constexpr float UNEASE_THRESHOLD     = 0.20f;
static constexpr float FEAR_THRESHOLD       = 0.50f;
static constexpr float ACCEPTANCE_THRESHOLD = 0.80f;

static TitanPhase phaseFromProgress(float p) {
    if (p >= ACCEPTANCE_THRESHOLD) return TitanPhase::Acceptance;
    if (p >= FEAR_THRESHOLD)       return TitanPhase::Fear;
    if (p >= UNEASE_THRESHOLD)     return TitanPhase::Unease;
    return TitanPhase::Rumor;
}

// ── TitanAssemblySystem ────────────────────────────────────────────

void TitanAssemblySystem::tick(TitanAssemblyComponent& comp) const {
    if (!comp.disrupted) {
        comp.progress += comp.resourceRate;
    } else {
        // Disrupted ticks still advance, but at 25% rate.
        comp.progress += comp.resourceRate * 0.25f;
        comp.disrupted = false; // reset flag after processing
    }
    comp.progress = std::min(comp.progress, 1.0f);
    comp.phase    = phaseFromProgress(comp.progress);
}

void TitanAssemblySystem::disrupt(TitanAssemblyComponent& comp,
                                   float amount) const {
    comp.disrupted = true;
    comp.disruptCount++;
    comp.progress -= amount;
    if (comp.progress < 0.0f) comp.progress = 0.0f;
    comp.phase = phaseFromProgress(comp.progress);
}

std::string TitanAssemblySystem::phaseName(TitanPhase phase) {
    switch (phase) {
        case TitanPhase::Rumor:      return "Rumor";
        case TitanPhase::Unease:     return "Unease";
        case TitanPhase::Fear:       return "Fear";
        case TitanPhase::Acceptance: return "Acceptance";
    }
    return "Unknown";
}

} // namespace systems
} // namespace atlas
