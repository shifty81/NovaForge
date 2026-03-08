// Tests for: TitanAssemblySystem
#include "test_log.h"
#include "systems/titan_assembly_system.h"

using namespace atlas;
using namespace atlas::systems;

// ==================== TitanAssemblySystem Tests ====================

static void testTitanInitialState() {
    std::cout << "\n=== TitanAssembly: InitialState ===" << std::endl;
    TitanAssemblyComponent comp;
    assertTrue(comp.progress == 0.0f, "Initial progress is 0");
    assertTrue(comp.phase == TitanPhase::Rumor, "Initial phase is Rumor");
    assertTrue(comp.resourceRate == 0.01f, "Default rate is 0.01");
    assertTrue(!comp.disrupted, "Not disrupted initially");
    assertTrue(comp.disruptCount == 0, "Zero disruptions initially");
}

static void testTitanTickAdvance() {
    std::cout << "\n=== TitanAssembly: TickAdvance ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;

    sys.tick(comp);
    assertTrue(approxEqual(comp.progress, 0.01f), "Progress +0.01 after 1 tick");
    assertTrue(comp.phase == TitanPhase::Rumor, "Still Rumor at 0.01");

    sys.tick(comp);
    assertTrue(approxEqual(comp.progress, 0.02f), "Progress +0.01 after 2 ticks");
}

static void testTitanPhaseRumorToUnease() {
    std::cout << "\n=== TitanAssembly: Rumor->Unease ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.resourceRate = 0.10f;

    // 2 ticks: 0.10, 0.20 → should reach Unease at 0.20
    sys.tick(comp);
    assertTrue(comp.phase == TitanPhase::Rumor, "Still Rumor at 0.10");
    sys.tick(comp);
    assertTrue(comp.phase == TitanPhase::Unease, "Unease at 0.20");
}

static void testTitanPhaseUneaseToFear() {
    std::cout << "\n=== TitanAssembly: Unease->Fear ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.progress = 0.49f;
    comp.resourceRate = 0.01f;

    sys.tick(comp);
    assertTrue(comp.phase == TitanPhase::Fear, "Fear at 0.50");
}

static void testTitanPhaseFearToAcceptance() {
    std::cout << "\n=== TitanAssembly: Fear->Acceptance ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.progress = 0.79f;
    comp.resourceRate = 0.01f;

    sys.tick(comp);
    assertTrue(comp.phase == TitanPhase::Acceptance, "Acceptance at 0.80");
}

static void testTitanProgressCap() {
    std::cout << "\n=== TitanAssembly: ProgressCap ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.progress = 0.97f;
    comp.resourceRate = 0.05f;

    sys.tick(comp);
    assertTrue(approxEqual(comp.progress, 1.0f), "Progress capped at 1.0");
    assertTrue(comp.phase == TitanPhase::Acceptance, "Acceptance at 1.0");
}

static void testTitanDisruptReducesProgress() {
    std::cout << "\n=== TitanAssembly: DisruptReduces ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.progress = 0.30f;
    comp.phase = TitanPhase::Unease;

    sys.disrupt(comp, 0.05f);
    assertTrue(approxEqual(comp.progress, 0.25f), "Progress reduced by disruption");
    assertTrue(comp.disrupted, "Disrupted flag set");
    assertTrue(comp.disruptCount == 1, "Disrupt count incremented");
    assertTrue(comp.phase == TitanPhase::Unease, "Still Unease at 0.25");
}

static void testTitanDisruptPhaseRegression() {
    std::cout << "\n=== TitanAssembly: DisruptPhaseRegression ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.progress = 0.21f;
    comp.phase = TitanPhase::Unease;

    sys.disrupt(comp, 0.05f);
    assertTrue(approxEqual(comp.progress, 0.16f), "Progress dropped to 0.16");
    assertTrue(comp.phase == TitanPhase::Rumor, "Regressed to Rumor from Unease");
}

static void testTitanDisruptProgressFloor() {
    std::cout << "\n=== TitanAssembly: DisruptProgressFloor ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.progress = 0.03f;

    sys.disrupt(comp, 0.10f);
    assertTrue(comp.progress == 0.0f, "Progress clamped to 0");
    assertTrue(comp.phase == TitanPhase::Rumor, "Phase stays Rumor");
}

static void testTitanDisruptedTick() {
    std::cout << "\n=== TitanAssembly: DisruptedTick ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.resourceRate = 0.10f;

    // Set disrupted flag (simulating disrupt was called before tick)
    comp.disrupted = true;

    float before = comp.progress;
    sys.tick(comp);
    float after = comp.progress;
    float expected = before + 0.10f * 0.25f; // 25% rate when disrupted
    assertTrue(approxEqual(after, expected), "Disrupted tick advances at 25% rate");
    assertTrue(!comp.disrupted, "Disrupted flag cleared after tick");
}

static void testTitanMultipleDisruptions() {
    std::cout << "\n=== TitanAssembly: MultipleDisruptions ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.progress = 0.50f;
    comp.phase = TitanPhase::Fear;

    sys.disrupt(comp, 0.10f);
    assertTrue(comp.disruptCount == 1, "1st disruption counted");
    sys.disrupt(comp, 0.10f);
    assertTrue(comp.disruptCount == 2, "2nd disruption counted");
    assertTrue(approxEqual(comp.progress, 0.30f), "Progress after 2 disruptions");
    assertTrue(comp.phase == TitanPhase::Unease, "Regressed to Unease");
}

static void testTitanZeroDisruptAmount() {
    std::cout << "\n=== TitanAssembly: ZeroDisruptAmount ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.progress = 0.50f;

    sys.disrupt(comp, 0.0f);
    assertTrue(approxEqual(comp.progress, 0.50f), "No change on zero disruption");
    assertTrue(comp.disruptCount == 1, "Still counted as disruption");
}

static void testTitanZeroResourceRate() {
    std::cout << "\n=== TitanAssembly: ZeroResourceRate ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.resourceRate = 0.0f;

    sys.tick(comp);
    assertTrue(comp.progress == 0.0f, "No progress with zero rate");
}

static void testTitanPhaseNames() {
    std::cout << "\n=== TitanAssembly: PhaseNames ===" << std::endl;
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Rumor) == "Rumor", "Rumor name");
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Unease) == "Unease", "Unease name");
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Fear) == "Fear", "Fear name");
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Acceptance) == "Acceptance", "Acceptance name");
}

static void testTitanFullProgression() {
    std::cout << "\n=== TitanAssembly: FullProgression ===" << std::endl;
    TitanAssemblySystem sys;
    TitanAssemblyComponent comp;
    comp.resourceRate = 0.10f;

    // Tick 10 times to reach 1.0
    for (int i = 0; i < 10; i++) {
        sys.tick(comp);
    }
    assertTrue(approxEqual(comp.progress, 1.0f), "Full progress after 10 ticks at 0.10");
    assertTrue(comp.phase == TitanPhase::Acceptance, "Final phase is Acceptance");

    // Extra tick does not exceed 1.0
    sys.tick(comp);
    assertTrue(approxEqual(comp.progress, 1.0f), "Still 1.0 after extra tick");
}

void run_titan_assembly_system_tests() {
    testTitanInitialState();
    testTitanTickAdvance();
    testTitanPhaseRumorToUnease();
    testTitanPhaseUneaseToFear();
    testTitanPhaseFearToAcceptance();
    testTitanProgressCap();
    testTitanDisruptReducesProgress();
    testTitanDisruptPhaseRegression();
    testTitanDisruptProgressFloor();
    testTitanDisruptedTick();
    testTitanMultipleDisruptions();
    testTitanZeroDisruptAmount();
    testTitanZeroResourceRate();
    testTitanPhaseNames();
    testTitanFullProgression();
}
