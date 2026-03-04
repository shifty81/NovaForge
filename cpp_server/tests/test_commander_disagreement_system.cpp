// Tests for: CommanderDisagreementSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/commander_disagreement_system.h"

using namespace atlas;

// ==================== CommanderDisagreementSystem Tests ====================

static void testDisagreementRaise() {
    std::cout << "\n=== Commander Disagreement: Raise ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    assertTrue(sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy"), "Disagreement raised");
    assertTrue(sys.getActiveCount("fleet1") == 1, "One active disagreement");
    assertTrue(sys.getTotalDisagreements("fleet1") == 1, "Total disagreements is 1");
    assertTrue(sys.getFleetTension("fleet1") > 0.0f, "Fleet tension increased");
}

static void testDisagreementResolveCompromise() {
    std::cout << "\n=== Commander Disagreement: Resolve Compromise ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Target");
    float tension_before = sys.getFleetTension("fleet1");
    assertTrue(sys.resolveDisagreement("fleet1", 0, "Compromise"), "Resolved by compromise");
    assertTrue(sys.getActiveCount("fleet1") == 0, "No active disagreements");
    assertTrue(sys.getTotalResolved("fleet1") == 1, "One resolved");
    assertTrue(sys.getResolution("fleet1", 0) == "Compromise", "Resolution is Compromise");
    assertTrue(sys.getFleetTension("fleet1") < tension_before, "Tension reduced after compromise");
}

static void testDisagreementResolveVote() {
    std::cout << "\n=== Commander Disagreement: Resolve Vote ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Formation");
    assertTrue(sys.resolveDisagreement("fleet1", 0, "Vote"), "Resolved by vote");
    assertTrue(sys.getResolution("fleet1", 0) == "Vote", "Resolution is Vote");
    assertTrue(sys.getTotalResolved("fleet1") == 1, "One resolved");
}

static void testDisagreementResolveAuthority() {
    std::cout << "\n=== Commander Disagreement: Resolve Authority ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Retreat");
    assertTrue(sys.resolveDisagreement("fleet1", 0, "AuthorityOverride"), "Resolved by authority");
    assertTrue(sys.getResolution("fleet1", 0) == "AuthorityOverride", "Resolution is AuthorityOverride");
    float impact = sys.getMoraleImpact("fleet1", 0);
    assertTrue(impact < -2.0f, "Authority override hurts morale more");
}

static void testDisagreementEscalation() {
    std::cout << "\n=== Commander Disagreement: Escalation ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    auto* cd = addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "LootSplit");
    assertTrue(sys.getSeverity("fleet1", 0) == "Minor", "Starts at Minor");

    // Set short escalation for testing
    cd->disagreements[0].escalation_threshold = 5.0f;
    sys.update(5.0f);
    assertTrue(sys.getSeverity("fleet1", 0) == "Moderate", "Escalated to Moderate");
    sys.update(5.0f);
    assertTrue(sys.getSeverity("fleet1", 0) == "Serious", "Escalated to Serious");
    sys.update(5.0f);
    assertTrue(sys.getSeverity("fleet1", 0) == "Critical", "Escalated to Critical");
    assertTrue(sys.getActiveCount("fleet1") == 0, "Auto-resolved at Critical (escalated)");
}

static void testDisagreementDismiss() {
    std::cout << "\n=== Commander Disagreement: Dismiss ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy");
    assertTrue(sys.dismissDisagreement("fleet1", 0), "Dismissed disagreement");
    assertTrue(sys.getActiveCount("fleet1") == 0, "No active after dismiss");
}

static void testDisagreementMultiple() {
    std::cout << "\n=== Commander Disagreement: Multiple ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy");
    sys.raiseDisagreement("fleet1", "cmd_b", "cmd_c", "Target");
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_c", "Retreat");
    assertTrue(sys.getActiveCount("fleet1") == 3, "Three active disagreements");
    assertTrue(sys.getTotalDisagreements("fleet1") == 3, "Three total");
    assertTrue(sys.getFleetTension("fleet1") >= 15.0f, "High fleet tension from three disagreements");
}

static void testDisagreementTensionCap() {
    std::cout << "\n=== Commander Disagreement: Tension Cap ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    auto* cd = addComp<components::CommanderDisagreement>(e);
    cd->fleet_tension = 98.0f;

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy");
    assertTrue(sys.getFleetTension("fleet1") <= 100.0f, "Tension capped at 100");
}

static void testDisagreementAlreadyResolved() {
    std::cout << "\n=== Commander Disagreement: Already Resolved ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy");
    sys.resolveDisagreement("fleet1", 0, "Vote");
    assertTrue(!sys.resolveDisagreement("fleet1", 0, "Compromise"), "Cannot resolve already resolved");
}

static void testDisagreementMissing() {
    std::cout << "\n=== Commander Disagreement: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CommanderDisagreementSystem sys(&world);
    assertTrue(sys.getActiveCount("nonexistent") == 0, "Default active count 0");
    assertTrue(approxEqual(sys.getFleetTension("nonexistent"), 0.0f), "Default tension 0");
    assertTrue(sys.getTotalDisagreements("nonexistent") == 0, "Default total 0");
    assertTrue(sys.getTotalResolved("nonexistent") == 0, "Default resolved 0");
    assertTrue(sys.getSeverity("nonexistent", 0) == "Unknown", "Default severity Unknown");
    assertTrue(sys.getResolution("nonexistent", 0) == "None", "Default resolution None");
    assertTrue(approxEqual(sys.getMoraleImpact("nonexistent", 0), 0.0f), "Default morale impact 0");
}


void run_commander_disagreement_system_tests() {
    testDisagreementRaise();
    testDisagreementResolveCompromise();
    testDisagreementResolveVote();
    testDisagreementResolveAuthority();
    testDisagreementEscalation();
    testDisagreementDismiss();
    testDisagreementMultiple();
    testDisagreementTensionCap();
    testDisagreementAlreadyResolved();
    testDisagreementMissing();
}
