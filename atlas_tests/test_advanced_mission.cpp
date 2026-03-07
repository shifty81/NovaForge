/**
 * Tests for AdvancedMissionGenerator — procedural mission generation
 * with templates, difficulty scaling, and branching mission chains.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/AdvancedMissionGenerator.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and template management
// ══════════════════════════════════════════════════════════════════

void test_mission_gen_defaults() {
    AdvancedMissionGenerator gen;
    assert(gen.TemplateCount() == 0);
    assert(gen.TotalGenerated() == 0);
}

void test_mission_gen_install_defaults() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    assert(gen.TemplateCount() == 20);
}

void test_mission_gen_add_template() {
    AdvancedMissionGenerator gen;
    MissionTemplate tpl;
    tpl.templateId = 100;
    tpl.namePattern = "Custom Mission";
    tpl.category = "Custom";
    assert(gen.AddTemplate(tpl));
    assert(gen.TemplateCount() == 1);
    assert(gen.GetTemplate(100) != nullptr);
    assert(gen.GetTemplate(100)->namePattern == "Custom Mission");
}

void test_mission_gen_add_duplicate_rejected() {
    AdvancedMissionGenerator gen;
    MissionTemplate tpl;
    tpl.templateId = 1;
    assert(gen.AddTemplate(tpl));
    assert(!gen.AddTemplate(tpl));
    assert(gen.TemplateCount() == 1);
}

void test_mission_gen_remove_template() {
    AdvancedMissionGenerator gen;
    MissionTemplate tpl;
    tpl.templateId = 1;
    gen.AddTemplate(tpl);
    assert(gen.RemoveTemplate(1));
    assert(gen.TemplateCount() == 0);
    assert(gen.GetTemplate(1) == nullptr);
}

void test_mission_gen_remove_nonexistent() {
    AdvancedMissionGenerator gen;
    assert(!gen.RemoveTemplate(999));
}

void test_mission_gen_get_by_category() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    auto combat = gen.GetTemplatesByCategory("Combat");
    assert(combat.size() == 4);
    auto mining = gen.GetTemplatesByCategory("Mining");
    assert(mining.size() == 4);
    auto courier = gen.GetTemplatesByCategory("Courier");
    assert(courier.size() == 4);
    auto exploration = gen.GetTemplatesByCategory("Exploration");
    assert(exploration.size() == 4);
    auto rescue = gen.GetTemplatesByCategory("Rescue");
    assert(rescue.size() == 4);
}

// ══════════════════════════════════════════════════════════════════
// Mission generation
// ══════════════════════════════════════════════════════════════════

void test_mission_gen_generate_for_system() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    auto missions = gen.GenerateForSystem(1, 5, 42, 3);
    assert(missions.size() == 3);
    for (auto& m : missions) {
        assert(m.missionId > 0);
        assert(m.systemId == 1);
        assert(m.playerLevel == 5);
        assert(m.rewardISC > 0.0f);
        assert(!m.objectives.empty());
        assert(!m.title.empty());
    }
    assert(gen.TotalGenerated() == 3);
}

void test_mission_gen_generate_empty_templates() {
    AdvancedMissionGenerator gen;
    auto missions = gen.GenerateForSystem(1, 5, 42, 3);
    assert(missions.empty());
}

void test_mission_gen_generate_zero_count() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    auto missions = gen.GenerateForSystem(1, 5, 42, 0);
    assert(missions.empty());
}

void test_mission_gen_from_template() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    auto m = gen.GenerateFromTemplate(1, 10, 3, 100);
    assert(m.missionId > 0);
    assert(m.title == "Strike: Pirate Outpost");
    assert(m.systemId == 10);
    assert(m.playerLevel == 3);
    assert(!m.objectives.empty());
    assert(m.objectives[0].type == ObjectiveType::DestroyTargets);
}

void test_mission_gen_from_nonexistent_template() {
    AdvancedMissionGenerator gen;
    auto m = gen.GenerateFromTemplate(999, 1, 1, 0);
    assert(m.missionId == 0);  // default-constructed
}

void test_mission_gen_difficulty_scaling() {
    // High level + low security → harder
    assert(AdvancedMissionGenerator::ComputeDifficulty(1, 1.0f) == DifficultyTier::Trivial);
    assert(AdvancedMissionGenerator::ComputeDifficulty(5, 0.5f) == DifficultyTier::Easy);
    assert(AdvancedMissionGenerator::ComputeDifficulty(8, 0.3f) == DifficultyTier::Normal);
    assert(AdvancedMissionGenerator::ComputeDifficulty(10, 0.1f) == DifficultyTier::Deadly);
    assert(AdvancedMissionGenerator::ComputeDifficulty(10, 0.0f) == DifficultyTier::Deadly);
    // Boundary: score=8.9 → Hard
    assert(AdvancedMissionGenerator::ComputeDifficulty(9, 0.01f) == DifficultyTier::Hard);
}

void test_mission_gen_reward_scaling() {
    float base = 1000.0f;
    assert(approxEq(AdvancedMissionGenerator::ScaleRewardByDifficulty(base, DifficultyTier::Trivial), 500.0f));
    assert(approxEq(AdvancedMissionGenerator::ScaleRewardByDifficulty(base, DifficultyTier::Easy), 800.0f));
    assert(approxEq(AdvancedMissionGenerator::ScaleRewardByDifficulty(base, DifficultyTier::Normal), 1000.0f));
    assert(approxEq(AdvancedMissionGenerator::ScaleRewardByDifficulty(base, DifficultyTier::Hard), 1500.0f));
    assert(approxEq(AdvancedMissionGenerator::ScaleRewardByDifficulty(base, DifficultyTier::Deadly), 2500.0f));
}

void test_mission_gen_branching() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    // Template 2 (Assault: Rogue Drone Hive) has branching
    auto m = gen.GenerateFromTemplate(2, 1, 5, 42);
    assert(m.branches.size() == 2);
    assert(!m.branches[0].label.empty());
    assert(!m.branches[1].label.empty());
    assert(m.branches[0].bonusReward > 0.0f);
}

void test_mission_gen_no_branching() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    // Template 1 (Strike: Pirate Outpost) has no branching
    auto m = gen.GenerateFromTemplate(1, 1, 5, 42);
    assert(m.branches.empty());
}

void test_mission_gen_level_filtering() {
    AdvancedMissionGenerator gen;
    MissionTemplate tpl;
    tpl.templateId = 100;
    tpl.minLevel = 5;
    tpl.maxLevel = 10;
    tpl.objectiveTypes = {ObjectiveType::DestroyTargets};
    gen.AddTemplate(tpl);

    // Player level 3 → no eligible templates
    auto missions = gen.GenerateForSystem(1, 3, 42, 5);
    assert(missions.empty());

    // Player level 7 → eligible
    missions = gen.GenerateForSystem(1, 7, 42, 5);
    assert(missions.size() == 5);
}

void test_mission_gen_deterministic() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    auto m1 = gen.GenerateForSystem(1, 5, 42, 3);
    // Reset generator for fresh IDs but same templates
    AdvancedMissionGenerator gen2;
    gen2.InstallDefaultTemplates();
    auto m2 = gen2.GenerateForSystem(1, 5, 42, 3);
    assert(m1.size() == m2.size());
    for (size_t i = 0; i < m1.size(); ++i) {
        assert(m1[i].title == m2[i].title);
        assert(m1[i].objectives.size() == m2[i].objectives.size());
    }
}

void test_mission_gen_unique_ids() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    auto missions = gen.GenerateForSystem(1, 5, 42, 5);
    for (size_t i = 0; i < missions.size(); ++i) {
        for (size_t j = i + 1; j < missions.size(); ++j) {
            assert(missions[i].missionId != missions[j].missionId);
        }
    }
}

void test_mission_gen_time_limit() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    auto m = gen.GenerateFromTemplate(1, 1, 5, 0);
    // Template 1: baseTimeLimit=600, timePerLevel=60, level=5 → 600+300=900
    assert(approxEq(m.timeLimitSec, 900.0f));
}

void test_mission_gen_standing_reward() {
    AdvancedMissionGenerator gen;
    gen.InstallDefaultTemplates();
    auto m = gen.GenerateFromTemplate(1, 1, 5, 0);
    // Template 1: baseStanding=0.05, standingPerLevel=0.01, level=5 → 0.10
    assert(approxEq(m.standingReward, 0.10f));
}
