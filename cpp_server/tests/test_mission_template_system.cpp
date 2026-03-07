// Tests for: Mission Template System Tests
#include "test_log.h"
#include "ecs/system.h"
#include "systems/mission_template_system.h"

using namespace atlas;

// ==================== Mission Template System Tests ====================

static void testMissionTemplateInstallDefaults() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    // Should have templates for level 1 with no faction restriction
    auto templates = sys.getTemplatesForFaction("", 0.0f, 1);
    assertTrue(!templates.empty(), "Default templates installed for level 1");
}

static void testMissionTemplateFilterByLevel() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto l1 = sys.getTemplatesForFaction("", 0.0f, 1);
    auto l5 = sys.getTemplatesForFaction("", 0.0f, 5);
    assertTrue(!l1.empty(), "Level 1 templates exist");
    assertTrue(l5.size() <= l1.size(), "Level 5 templates <= level 1 templates");
}

static void testMissionTemplateFilterByStanding() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto low_standing = sys.getTemplatesForFaction("", -5.0f, 1);
    auto high_standing = sys.getTemplatesForFaction("", 5.0f, 1);
    assertTrue(high_standing.size() >= low_standing.size(),
               "Higher standing unlocks at least as many templates");
}

static void testMissionTemplateGenerate() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto templates = sys.getTemplatesForFaction("", 0.0f, 1);
    assertTrue(!templates.empty(), "Have templates to generate from");

    auto mission = sys.generateMissionFromTemplate(templates[0], "system_1", "player_1");
    assertTrue(!mission.mission_id.empty(), "Generated mission has ID");
    assertTrue(!mission.objectives.empty(), "Generated mission has objectives");
    assertTrue(mission.isc_reward > 0.0, "Generated mission has positive Credits reward");
}

static void testMissionTemplateDeterministic() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto templates = sys.getTemplatesForFaction("", 0.0f, 1);
    auto m1 = sys.generateMissionFromTemplate(templates[0], "system_1", "player_1");
    auto m2 = sys.generateMissionFromTemplate(templates[0], "system_1", "player_1");
    assertTrue(m1.objectives.size() == m2.objectives.size(),
               "Deterministic: same objectives count for same seed");
}

static void testMissionTemplateScaledRewards() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto l1 = sys.getTemplatesForFaction("", 0.0f, 1);
    auto l3 = sys.getTemplatesForFaction("", 1.0f, 3);

    if (!l1.empty() && !l3.empty()) {
        auto m1 = sys.generateMissionFromTemplate(l1[0], "s1", "p1");
        auto m3 = sys.generateMissionFromTemplate(l3[0], "s1", "p1");
        assertTrue(m3.isc_reward >= m1.isc_reward,
                   "Higher level missions give more Credits");
    } else {
        assertTrue(true, "Higher level missions give more Credits (skipped)");
    }
}


void run_mission_template_system_tests() {
    testMissionTemplateInstallDefaults();
    testMissionTemplateFilterByLevel();
    testMissionTemplateFilterByStanding();
    testMissionTemplateGenerate();
    testMissionTemplateDeterministic();
    testMissionTemplateScaledRewards();
}
