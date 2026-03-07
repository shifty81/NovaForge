#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

/**
 * Mission objective types.
 */
enum class ObjectiveType : uint8_t {
    DestroyTargets,     // Kill N hostiles
    MineOre,            // Mine N units of ore
    DeliverCargo,       // Deliver cargo to destination
    ScanAnomalies,      // Scan N anomalies
    RescueSurvivors,    // Rescue N NPCs from wrecks
    EscortConvoy,       // Escort convoy to destination
    DefendStation,      // Defend station from waves
    HackDataSite        // Hack a relic/data site
};

/**
 * Mission difficulty tier.
 */
enum class DifficultyTier : uint8_t {
    Trivial   = 0,
    Easy      = 1,
    Normal    = 2,
    Hard      = 3,
    Deadly    = 4
};

/**
 * A single objective within a mission.
 */
struct MissionObjective {
    ObjectiveType type       = ObjectiveType::DestroyTargets;
    uint32_t      targetCount = 1;         // how many to complete
    uint32_t      progress    = 0;         // current progress
    float         bonusISC    = 0.0f;      // optional bonus reward
    std::string   description;
};

/**
 * A branching choice presented after the mission completes.
 */
struct BranchChoice {
    std::string label;                     // e.g. "Help the miners"
    float       standingChange  = 0.0f;    // faction standing delta
    float       bonusReward     = 0.0f;    // ISC bonus for choosing this
    uint32_t    nextTemplateIdx = 0;       // follow-up mission template
};

/**
 * A generated mission instance.
 */
struct GeneratedMission {
    uint32_t                    missionId     = 0;
    std::string                 title;
    std::string                 briefing;
    DifficultyTier              difficulty    = DifficultyTier::Normal;
    float                       rewardISC     = 0.0f;
    float                       standingReward = 0.0f;
    float                       timeLimitSec  = 0.0f;
    std::vector<MissionObjective> objectives;
    std::vector<BranchChoice>   branches;     // empty = no branching
    uint32_t                    playerLevel   = 1;
    uint32_t                    systemId      = 0;
};

/**
 * A mission template used for procedural generation.
 */
struct MissionTemplate {
    uint32_t                    templateId    = 0;
    std::string                 namePattern;       // e.g. "Strike: {target}"
    std::string                 category;          // Combat, Mining, Courier, etc.
    std::vector<ObjectiveType>  objectiveTypes;    // objectives to generate
    float                       baseRewardISC     = 1000.0f;
    float                       rewardPerLevel    = 500.0f;
    float                       baseStanding      = 0.05f;
    float                       standingPerLevel  = 0.01f;
    float                       baseTimeLimitSec  = 600.0f;
    float                       timePerLevel      = 60.0f;
    uint32_t                    minLevel          = 1;
    uint32_t                    maxLevel          = 10;
    bool                        hasBranching      = false;
};

/**
 * @brief Procedural mission generation with templates, difficulty scaling,
 *        and branching mission chains.
 *
 * Supports multiple template categories (Combat, Mining, Courier, Exploration,
 * Rescue). Generates missions from templates with deterministic seeding,
 * scales difficulty and rewards to player level, and optionally attaches
 * branching choices for mission chains.
 *
 * Usage:
 *   AdvancedMissionGenerator gen;
 *   gen.InstallDefaultTemplates();
 *   auto missions = gen.GenerateForSystem(systemId, playerLevel, seed, count);
 */
class AdvancedMissionGenerator {
public:
    AdvancedMissionGenerator() = default;

    /** Install the built-in set of 20 templates. */
    void InstallDefaultTemplates();

    /** Add a custom template. Returns false if templateId already exists. */
    bool AddTemplate(const MissionTemplate& tpl);

    /** Remove a template by id. */
    bool RemoveTemplate(uint32_t templateId);

    /** Number of installed templates. */
    size_t TemplateCount() const { return m_templates.size(); }

    /** Get template by id. Returns nullptr if not found. */
    const MissionTemplate* GetTemplate(uint32_t templateId) const;

    /** Get templates filtered by category. */
    std::vector<const MissionTemplate*> GetTemplatesByCategory(
        const std::string& category) const;

    /**
     * Generate N missions for a system at a given player level.
     * Uses deterministic seed for reproducibility.
     */
    std::vector<GeneratedMission> GenerateForSystem(
        uint32_t systemId,
        uint32_t playerLevel,
        uint32_t seed,
        uint32_t count) const;

    /**
     * Generate a single mission from a specific template.
     */
    GeneratedMission GenerateFromTemplate(
        uint32_t templateId,
        uint32_t systemId,
        uint32_t playerLevel,
        uint32_t seed) const;

    /**
     * Compute the difficulty tier for a given player level and system.
     */
    static DifficultyTier ComputeDifficulty(uint32_t playerLevel,
                                            float systemSecurity);

    /**
     * Scale reward ISC by difficulty tier.
     */
    static float ScaleRewardByDifficulty(float baseReward,
                                         DifficultyTier tier);

    /** Total missions generated across all calls. */
    uint32_t TotalGenerated() const { return m_totalGenerated; }

private:
    GeneratedMission buildMission(const MissionTemplate& tpl,
                                  uint32_t systemId,
                                  uint32_t playerLevel,
                                  uint32_t seed) const;

    std::vector<MissionTemplate> m_templates;
    mutable uint32_t m_totalGenerated = 0;
    mutable uint32_t m_nextMissionId = 1;
};

} // namespace atlas::sim
