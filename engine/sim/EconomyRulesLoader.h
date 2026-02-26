#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace atlas::sim {

/**
 * Economy rules for a single star-system security band or global override.
 *
 * Every field carries a sensible default so the simulation runs without any
 * configuration file.  Mod authors override individual fields via
 * economy_rules.json (or the simple text format supported by LoadFromText).
 */
struct EconomyRules {
    std::string name;                          // human-readable label

    // NPC spawning
    float pirateSpawnRate       = 0.05f;       // spawns / second in low-sec
    float minerSpawnRate        = 0.10f;       // spawns / second in high-sec
    float haulerSpawnRate       = 0.08f;       // spawns / second
    float traderSpawnRate       = 0.06f;       // spawns / second

    // Resource
    float miningYieldMultiplier = 1.0f;        // multiplied into base yield
    float resourceRegenRate     = 0.001f;      // per second toward 1.0

    // Background sim drift rates
    float threatDecayRate       = 0.002f;      // per second toward 0
    float economicRecoveryRate  = 0.001f;      // per second toward 0.5
    float trafficDriftRate      = 0.0005f;     // per second toward baseline

    // Market
    float brokerFeePercent      = 3.0f;        // % of order value
    float salesTaxPercent       = 2.0f;        // % of sale price
    float priceFloor            = 0.5f;        // min price multiplier
    float priceCeiling          = 2.0f;        // max price multiplier

    // Security thresholds (system security 0-1)
    float highSecThreshold      = 0.5f;        // >= this = high-sec
    float lowSecThreshold       = 0.1f;        // >= this = low-sec (< highSec)
};

/**
 * @brief Data-driven economy rules loader.
 *
 * Manages named EconomyRules entries keyed by a unique string id (typically
 * "global", "highsec", "lowsec", "nullsec").  Rules can be installed
 * programmatically via AddRules() or loaded from a simple text
 * representation via LoadFromText().
 *
 * Text format (one rule-set per block):
 *
 *   [global]
 *   name = Global Defaults
 *   pirateSpawnRate = 0.05
 *   miningYieldMultiplier = 1.0
 *   ...
 *
 * Usage:
 *   EconomyRulesLoader loader;
 *   loader.InstallDefaults();                  // 4 built-in rule-sets
 *   loader.LoadFromText(fileContents);         // override / extend
 *   const EconomyRules* r = loader.GetRules("lowsec");
 */
class EconomyRulesLoader {
public:
    EconomyRulesLoader() = default;

    /** Install 4 built-in rule-sets: global, highsec, lowsec, nullsec. */
    void InstallDefaults();

    /** Add or replace a rule-set with the given id. */
    bool AddRules(const std::string& id, const EconomyRules& rules);

    /** Remove a rule-set by id. Returns false if not found. */
    bool RemoveRules(const std::string& id);

    /** Get a rule-set by id. Returns nullptr if not found. */
    const EconomyRules* GetRules(const std::string& id) const;

    /** Number of loaded rule-sets. */
    size_t RulesCount() const { return m_rules.size(); }

    /**
     * Load rule-sets from a simple text format.
     * Returns number of rule-sets parsed (0 if input is empty).
     */
    size_t LoadFromText(const std::string& text);

    /** Get all rule-set ids. */
    std::vector<std::string> RulesIds() const;

    /**
     * Select the appropriate rule-set for a system with the given security
     * level.  Falls back to "global" if no match is found.
     */
    const EconomyRules* GetRulesForSecurity(float security) const;

private:
    std::unordered_map<std::string, EconomyRules> m_rules;
};

} // namespace atlas::sim
