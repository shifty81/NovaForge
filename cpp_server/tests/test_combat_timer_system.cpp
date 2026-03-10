// Tests for: CombatTimerSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/combat_timer_system.h"

using namespace atlas;

using CT = components::CombatTimer;

// ==================== CombatTimerSystem Tests ====================

static void testCombatTimerInit() {
    std::cout << "\n=== CombatTimer: Init ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1"), "Init succeeds");
    assertTrue(!sys.isInCombat("p1"),   "No combat timers active initially");
    assertTrue(sys.canSafelyUndock("p1"), "Can safely undock initially");
    assertTrue(!sys.hasAggression("p1"), "No aggression timer initially");
    assertTrue(!sys.hasWeaponTimer("p1"), "No weapon timer initially");
    assertTrue(!sys.hasPodKillTimer("p1"), "No pod-kill timer initially");
    assertTrue(approxEqual(sys.getAggressionTimer("p1"), 0.0f), "Aggression timer is 0");
    assertTrue(approxEqual(sys.getWeaponTimer("p1"),     0.0f), "Weapon timer is 0");
    assertTrue(approxEqual(sys.getPodKillTimer("p1"),    0.0f), "Pod-kill timer is 0");
    assertTrue(sys.getTotalAggressions("p1") == 0, "Zero aggressions initially");
    assertTrue(sys.getTotalPodKills("p1")    == 0, "Zero pod kills initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testCombatTimerAggressionTrigger() {
    std::cout << "\n=== CombatTimer: AggressionTrigger ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.triggerAggression("p1"), "Trigger aggression succeeds");
    assertTrue(sys.hasAggression("p1"),     "Aggression timer active");
    assertTrue(sys.isInCombat("p1"),        "isInCombat true with aggression");
    assertTrue(approxEqual(sys.getAggressionTimer("p1"), 300.0f),
               "Aggression timer set to default 300 s");
    assertTrue(sys.getTotalAggressions("p1") == 1, "Aggression count incremented");
    assertTrue(sys.canSafelyUndock("p1"),    "Can still undock (no weapon timer)");
}

static void testCombatTimerWeaponTrigger() {
    std::cout << "\n=== CombatTimer: WeaponTrigger ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.triggerWeapon("p1"),    "Trigger weapon succeeds");
    assertTrue(sys.hasWeaponTimer("p1"),   "Weapon timer active");
    assertTrue(sys.isInCombat("p1"),       "isInCombat true with weapon timer");
    assertTrue(!sys.canSafelyUndock("p1"), "Cannot undock with weapon timer active");
    assertTrue(approxEqual(sys.getWeaponTimer("p1"), 60.0f),
               "Weapon timer set to default 60 s");
}

static void testCombatTimerPodKillTrigger() {
    std::cout << "\n=== CombatTimer: PodKillTrigger ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.triggerPodKill("p1"),    "Trigger pod kill succeeds");
    assertTrue(sys.hasPodKillTimer("p1"),   "Pod-kill timer active");
    assertTrue(sys.isInCombat("p1"),        "isInCombat true with pod-kill timer");
    assertTrue(approxEqual(sys.getPodKillTimer("p1"), 900.0f),
               "Pod-kill timer set to default 900 s");
    assertTrue(sys.getTotalPodKills("p1") == 1, "Pod-kill count incremented");
    assertTrue(sys.canSafelyUndock("p1"),   "Can undock (no weapon timer active)");
}

static void testCombatTimerCountdown() {
    std::cout << "\n=== CombatTimer: Countdown ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.triggerWeapon("p1");
    assertTrue(approxEqual(sys.getWeaponTimer("p1"), 60.0f), "Timer at 60 s before tick");

    sys.update(30.0f);
    assertTrue(approxEqual(sys.getWeaponTimer("p1"), 30.0f), "Timer at 30 s after 30 s tick");
    assertTrue(sys.hasWeaponTimer("p1"),  "Timer still active at 30 s");
    assertTrue(!sys.canSafelyUndock("p1"), "Still cannot undock");

    sys.update(30.0f);
    assertTrue(approxEqual(sys.getWeaponTimer("p1"), 0.0f), "Timer at 0 after full duration");
    assertTrue(!sys.hasWeaponTimer("p1"), "Timer expired");
    assertTrue(sys.canSafelyUndock("p1"), "Can undock after timer expires");
    assertTrue(!sys.isInCombat("p1"),     "No longer in combat");
}

static void testCombatTimerNoNegative() {
    std::cout << "\n=== CombatTimer: NoNegative ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.triggerWeapon("p1");
    // Overshoot the timer
    sys.update(1000.0f);
    assertTrue(approxEqual(sys.getWeaponTimer("p1"), 0.0f), "Timer never goes negative");
    assertTrue(!sys.isInCombat("p1"), "No longer in combat after overshoot");
}

static void testCombatTimerMultipleActive() {
    std::cout << "\n=== CombatTimer: MultipleActive ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.triggerAggression("p1");
    sys.triggerWeapon("p1");
    sys.triggerPodKill("p1");
    assertTrue(sys.hasAggression("p1"),   "Aggression timer active");
    assertTrue(sys.hasWeaponTimer("p1"),  "Weapon timer active");
    assertTrue(sys.hasPodKillTimer("p1"), "Pod-kill timer active");
    assertTrue(sys.isInCombat("p1"),      "All three timers active");
    assertTrue(!sys.canSafelyUndock("p1"), "Cannot undock with weapon timer");

    // Let weapon timer expire; aggression and pod-kill remain
    sys.update(60.0f);
    assertTrue(!sys.hasWeaponTimer("p1"), "Weapon timer expired");
    assertTrue(sys.hasAggression("p1"),   "Aggression still running");
    assertTrue(sys.hasPodKillTimer("p1"), "Pod-kill still running");
    assertTrue(sys.isInCombat("p1"),      "Still in combat");
    assertTrue(sys.canSafelyUndock("p1"), "Can undock (weapon expired)");
}

static void testCombatTimerConfiguration() {
    std::cout << "\n=== CombatTimer: Configuration ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.setAggressionDuration("p1", 600.0f), "Set aggression duration");
    assertTrue(sys.setWeaponDuration("p1",     120.0f), "Set weapon duration");
    assertTrue(sys.setPodKillDuration("p1",   1800.0f), "Set pod-kill duration");

    sys.triggerAggression("p1");
    sys.triggerWeapon("p1");
    sys.triggerPodKill("p1");

    assertTrue(approxEqual(sys.getAggressionTimer("p1"), 600.0f),
               "Custom aggression duration applied");
    assertTrue(approxEqual(sys.getWeaponTimer("p1"),     120.0f),
               "Custom weapon duration applied");
    assertTrue(approxEqual(sys.getPodKillTimer("p1"),   1800.0f),
               "Custom pod-kill duration applied");

    // Invalid configurations rejected
    assertTrue(!sys.setAggressionDuration("p1", 0.0f),  "Zero aggression duration rejected");
    assertTrue(!sys.setWeaponDuration("p1",    -5.0f),  "Negative weapon duration rejected");
}

static void testCombatTimerRetrigger() {
    std::cout << "\n=== CombatTimer: Retrigger ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.triggerWeapon("p1");
    sys.update(30.0f); // half expired
    sys.triggerWeapon("p1"); // re-trigger resets to full duration
    assertTrue(approxEqual(sys.getWeaponTimer("p1"), 60.0f),
               "Re-trigger resets weapon timer to full duration");
    assertTrue(sys.hasWeaponTimer("p1"), "Timer still active after re-trigger");
}

static void testCombatTimerCounters() {
    std::cout << "\n=== CombatTimer: Counters ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.triggerAggression("p1");
    sys.triggerAggression("p1");
    sys.triggerAggression("p1");
    assertTrue(sys.getTotalAggressions("p1") == 3,
               "Aggression counter increments on each trigger");

    sys.triggerPodKill("p1");
    sys.triggerPodKill("p1");
    assertTrue(sys.getTotalPodKills("p1") == 2,
               "Pod-kill counter increments on each trigger");
}

static void testCombatTimerMissing() {
    std::cout << "\n=== CombatTimer: Missing ===" << std::endl;
    ecs::World world;
    systems::CombatTimerSystem sys(&world);

    assertTrue(!sys.triggerAggression("nx"), "TriggerAggression fails on missing");
    assertTrue(!sys.triggerWeapon("nx"),     "TriggerWeapon fails on missing");
    assertTrue(!sys.triggerPodKill("nx"),    "TriggerPodKill fails on missing");
    assertTrue(!sys.setAggressionDuration("nx", 100.0f), "SetAggDuration fails on missing");
    assertTrue(!sys.setWeaponDuration("nx",     100.0f), "SetWeaponDuration fails on missing");
    assertTrue(!sys.setPodKillDuration("nx",    100.0f), "SetPodKillDuration fails on missing");
    assertTrue(!sys.isInCombat("nx"),        "isInCombat false on missing");
    assertTrue(sys.canSafelyUndock("nx"),    "canSafelyUndock true on missing");
    assertTrue(!sys.hasAggression("nx"),     "No aggression on missing");
    assertTrue(!sys.hasWeaponTimer("nx"),    "No weapon timer on missing");
    assertTrue(!sys.hasPodKillTimer("nx"),   "No pod-kill timer on missing");
    assertTrue(approxEqual(sys.getAggressionTimer("nx"), 0.0f), "0 aggression on missing");
    assertTrue(approxEqual(sys.getWeaponTimer("nx"),     0.0f), "0 weapon on missing");
    assertTrue(approxEqual(sys.getPodKillTimer("nx"),    0.0f), "0 pod-kill on missing");
    assertTrue(sys.getTotalAggressions("nx") == 0, "0 aggressions on missing");
    assertTrue(sys.getTotalPodKills("nx")    == 0, "0 pod kills on missing");
}

void run_combat_timer_system_tests() {
    testCombatTimerInit();
    testCombatTimerAggressionTrigger();
    testCombatTimerWeaponTrigger();
    testCombatTimerPodKillTrigger();
    testCombatTimerCountdown();
    testCombatTimerNoNegative();
    testCombatTimerMultipleActive();
    testCombatTimerConfiguration();
    testCombatTimerRetrigger();
    testCombatTimerCounters();
    testCombatTimerMissing();
}
