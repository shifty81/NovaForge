#include "sim/AdvancedMissionGenerator.h"
#include <algorithm>
#include <cmath>

namespace atlas::sim {

// Simple deterministic hash for seeding
static uint32_t mixHash(uint32_t a, uint32_t b) {
    a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);
    return a;
}

// ─────────────────────────────────────────────────────────────────
// Default templates (20 templates across 5 categories)
// ─────────────────────────────────────────────────────────────────

void AdvancedMissionGenerator::InstallDefaultTemplates() {
    // Combat templates (4)
    AddTemplate({1,  "Strike: Pirate Outpost",       "Combat",
        {ObjectiveType::DestroyTargets},
        2000.0f, 800.0f, 0.05f, 0.01f, 600.0f, 60.0f, 1, 10, false});

    AddTemplate({2,  "Assault: Rogue Drone Hive",    "Combat",
        {ObjectiveType::DestroyTargets, ObjectiveType::HackDataSite},
        3500.0f, 1200.0f, 0.07f, 0.015f, 900.0f, 90.0f, 2, 10, true});

    AddTemplate({3,  "Defense: Station Under Siege",  "Combat",
        {ObjectiveType::DefendStation},
        5000.0f, 2000.0f, 0.10f, 0.02f, 1200.0f, 120.0f, 3, 10, false});

    AddTemplate({4,  "Extermination: Belt Rats",      "Combat",
        {ObjectiveType::DestroyTargets},
        1500.0f, 600.0f, 0.03f, 0.005f, 480.0f, 40.0f, 1, 5, false});

    // Mining templates (4)
    AddTemplate({5,  "Mining: Veldspar Requisition",  "Mining",
        {ObjectiveType::MineOre},
        1200.0f, 400.0f, 0.04f, 0.008f, 900.0f, 60.0f, 1, 8, false});

    AddTemplate({6,  "Mining: Rare Ore Survey",       "Mining",
        {ObjectiveType::MineOre, ObjectiveType::ScanAnomalies},
        2500.0f, 900.0f, 0.06f, 0.012f, 1200.0f, 90.0f, 3, 10, true});

    AddTemplate({7,  "Mining: Emergency Extraction",  "Mining",
        {ObjectiveType::MineOre, ObjectiveType::DestroyTargets},
        3000.0f, 1000.0f, 0.05f, 0.01f, 600.0f, 60.0f, 2, 7, false});

    AddTemplate({8,  "Mining: Ice Harvest",           "Mining",
        {ObjectiveType::MineOre},
        1800.0f, 700.0f, 0.04f, 0.01f, 1500.0f, 120.0f, 1, 10, false});

    // Courier templates (4)
    AddTemplate({9,  "Courier: Urgent Delivery",      "Courier",
        {ObjectiveType::DeliverCargo},
        1000.0f, 300.0f, 0.03f, 0.005f, 300.0f, 30.0f, 1, 5, false});

    AddTemplate({10, "Courier: High-Value Transport", "Courier",
        {ObjectiveType::DeliverCargo, ObjectiveType::EscortConvoy},
        4000.0f, 1500.0f, 0.08f, 0.02f, 1800.0f, 120.0f, 4, 10, true});

    AddTemplate({11, "Courier: Supply Run",           "Courier",
        {ObjectiveType::DeliverCargo},
        800.0f, 250.0f, 0.02f, 0.004f, 600.0f, 60.0f, 1, 4, false});

    AddTemplate({12, "Courier: Refugee Evacuation",   "Courier",
        {ObjectiveType::DeliverCargo, ObjectiveType::RescueSurvivors},
        3500.0f, 1200.0f, 0.07f, 0.015f, 900.0f, 90.0f, 3, 10, false});

    // Exploration templates (4)
    AddTemplate({13, "Exploration: Unknown Signal",   "Exploration",
        {ObjectiveType::ScanAnomalies},
        1500.0f, 500.0f, 0.04f, 0.008f, 1200.0f, 120.0f, 1, 10, false});

    AddTemplate({14, "Exploration: Ancient Relic",    "Exploration",
        {ObjectiveType::ScanAnomalies, ObjectiveType::HackDataSite},
        4500.0f, 1800.0f, 0.09f, 0.02f, 1800.0f, 180.0f, 4, 10, true});

    AddTemplate({15, "Exploration: Data Recovery",    "Exploration",
        {ObjectiveType::HackDataSite},
        2000.0f, 600.0f, 0.05f, 0.01f, 900.0f, 60.0f, 2, 8, false});

    AddTemplate({16, "Exploration: Wormhole Probe",   "Exploration",
        {ObjectiveType::ScanAnomalies, ObjectiveType::DestroyTargets},
        5500.0f, 2200.0f, 0.10f, 0.025f, 1500.0f, 120.0f, 5, 10, false});

    // Rescue templates (4)
    AddTemplate({17, "Rescue: Stranded Miners",       "Rescue",
        {ObjectiveType::RescueSurvivors},
        1800.0f, 600.0f, 0.05f, 0.01f, 600.0f, 60.0f, 1, 6, false});

    AddTemplate({18, "Rescue: Ambushed Convoy",       "Rescue",
        {ObjectiveType::RescueSurvivors, ObjectiveType::DestroyTargets},
        3000.0f, 1000.0f, 0.06f, 0.012f, 900.0f, 90.0f, 2, 8, true});

    AddTemplate({19, "Rescue: Station Evacuation",    "Rescue",
        {ObjectiveType::RescueSurvivors, ObjectiveType::EscortConvoy},
        4000.0f, 1400.0f, 0.08f, 0.015f, 1200.0f, 120.0f, 3, 10, false});

    AddTemplate({20, "Rescue: Downed Capsuleer",      "Rescue",
        {ObjectiveType::RescueSurvivors},
        1200.0f, 400.0f, 0.03f, 0.006f, 480.0f, 30.0f, 1, 4, false});
}

// ─────────────────────────────────────────────────────────────────
// Template management
// ─────────────────────────────────────────────────────────────────

bool AdvancedMissionGenerator::AddTemplate(const MissionTemplate& tpl) {
    for (const auto& existing : m_templates) {
        if (existing.templateId == tpl.templateId) return false;
    }
    m_templates.push_back(tpl);
    return true;
}

bool AdvancedMissionGenerator::RemoveTemplate(uint32_t templateId) {
    auto it = std::find_if(m_templates.begin(), m_templates.end(),
        [templateId](const MissionTemplate& t) { return t.templateId == templateId; });
    if (it == m_templates.end()) return false;
    m_templates.erase(it);
    return true;
}

const MissionTemplate* AdvancedMissionGenerator::GetTemplate(uint32_t templateId) const {
    for (const auto& t : m_templates) {
        if (t.templateId == templateId) return &t;
    }
    return nullptr;
}

std::vector<const MissionTemplate*> AdvancedMissionGenerator::GetTemplatesByCategory(
    const std::string& category) const {
    std::vector<const MissionTemplate*> result;
    for (const auto& t : m_templates) {
        if (t.category == category) result.push_back(&t);
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────
// Mission generation
// ─────────────────────────────────────────────────────────────────

std::vector<GeneratedMission> AdvancedMissionGenerator::GenerateForSystem(
    uint32_t systemId, uint32_t playerLevel, uint32_t seed, uint32_t count) const {

    std::vector<GeneratedMission> result;
    if (m_templates.empty() || count == 0) return result;

    // Filter templates eligible for this player level
    std::vector<const MissionTemplate*> eligible;
    for (const auto& t : m_templates) {
        if (playerLevel >= t.minLevel && playerLevel <= t.maxLevel) {
            eligible.push_back(&t);
        }
    }
    if (eligible.empty()) return result;

    result.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t localSeed = mixHash(seed, i);
        size_t tplIdx = localSeed % eligible.size();
        const MissionTemplate* tpl = eligible[tplIdx];
        result.push_back(buildMission(*tpl, systemId, playerLevel,
                                      mixHash(localSeed, tpl->templateId)));
    }
    return result;
}

GeneratedMission AdvancedMissionGenerator::GenerateFromTemplate(
    uint32_t templateId, uint32_t systemId,
    uint32_t playerLevel, uint32_t seed) const {
    const MissionTemplate* tpl = GetTemplate(templateId);
    if (!tpl) return {};
    return buildMission(*tpl, systemId, playerLevel, seed);
}

// ─────────────────────────────────────────────────────────────────
// Difficulty helpers
// ─────────────────────────────────────────────────────────────────

DifficultyTier AdvancedMissionGenerator::ComputeDifficulty(
    uint32_t playerLevel, float systemSecurity) {
    // Higher level + lower security → harder missions
    float score = static_cast<float>(playerLevel) * (1.0f - systemSecurity);
    if (score < 1.0f) return DifficultyTier::Trivial;
    if (score < 3.0f) return DifficultyTier::Easy;
    if (score < 6.0f) return DifficultyTier::Normal;
    if (score < 9.0f) return DifficultyTier::Hard;
    return DifficultyTier::Deadly;
}

float AdvancedMissionGenerator::ScaleRewardByDifficulty(
    float baseReward, DifficultyTier tier) {
    static const float multipliers[] = {0.5f, 0.8f, 1.0f, 1.5f, 2.5f};
    return baseReward * multipliers[static_cast<int>(tier)];
}

// ─────────────────────────────────────────────────────────────────
// Internal build
// ─────────────────────────────────────────────────────────────────

GeneratedMission AdvancedMissionGenerator::buildMission(
    const MissionTemplate& tpl, uint32_t systemId,
    uint32_t playerLevel, uint32_t seed) const {

    GeneratedMission m;
    m.missionId      = m_nextMissionId++;
    m.title          = tpl.namePattern;
    m.briefing       = "Mission briefing for " + tpl.namePattern;
    m.systemId       = systemId;
    m.playerLevel    = playerLevel;
    m.difficulty     = ComputeDifficulty(playerLevel, 0.5f);
    m.timeLimitSec   = tpl.baseTimeLimitSec + tpl.timePerLevel * playerLevel;

    float rawReward  = tpl.baseRewardISK + tpl.rewardPerLevel * playerLevel;
    m.rewardISK      = ScaleRewardByDifficulty(rawReward, m.difficulty);
    m.standingReward = tpl.baseStanding + tpl.standingPerLevel * playerLevel;

    // Build objectives, scaling target counts by level
    uint32_t objSeed = seed;
    for (auto objType : tpl.objectiveTypes) {
        MissionObjective obj;
        obj.type = objType;
        objSeed  = mixHash(objSeed, static_cast<uint32_t>(objType));
        // Base count 3-8, scaled by player level
        uint32_t base = 3 + (objSeed % 6);
        obj.targetCount = base + playerLevel;
        obj.bonusISK    = (objSeed % 2 == 0) ? tpl.rewardPerLevel * 0.25f : 0.0f;

        switch (objType) {
            case ObjectiveType::DestroyTargets:  obj.description = "Destroy hostile targets"; break;
            case ObjectiveType::MineOre:         obj.description = "Mine required ore";       break;
            case ObjectiveType::DeliverCargo:    obj.description = "Deliver cargo";            break;
            case ObjectiveType::ScanAnomalies:   obj.description = "Scan anomalies";           break;
            case ObjectiveType::RescueSurvivors: obj.description = "Rescue survivors";         break;
            case ObjectiveType::EscortConvoy:    obj.description = "Escort convoy safely";     break;
            case ObjectiveType::DefendStation:   obj.description = "Defend the station";       break;
            case ObjectiveType::HackDataSite:    obj.description = "Hack the data site";       break;
        }
        m.objectives.push_back(obj);
    }

    // Branching choices for templates that support them
    if (tpl.hasBranching && !m_templates.empty()) {
        BranchChoice a;
        a.label          = "Pursue the aggressors";
        a.standingChange = 0.02f;
        a.bonusReward    = rawReward * 0.2f;
        a.nextTemplateIdx = (tpl.templateId % m_templates.size());

        BranchChoice b;
        b.label          = "Protect the civilians";
        b.standingChange = 0.04f;
        b.bonusReward    = rawReward * 0.1f;
        b.nextTemplateIdx = ((tpl.templateId + 1) % m_templates.size());

        m.branches.push_back(a);
        m.branches.push_back(b);
    }

    ++m_totalGenerated;
    return m;
}

} // namespace atlas::sim
