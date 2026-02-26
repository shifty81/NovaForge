#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

/**
 * States for an autonomous AI industrialist entity.
 */
enum class IndustrialistState : uint8_t {
    Idle,                // Waiting for work
    SelectBlueprint,     // Choosing what to manufacture
    AcquireMaterials,    // Purchasing / gathering raw materials
    Manufacturing,       // Running the production job
    DeliverGoods,        // Transporting finished goods to market
    SellGoods,           // Listing and selling manufactured goods
    Dead                 // Destroyed, awaiting respawn
};

/**
 * A blueprint available for manufacturing.
 */
struct BlueprintEntry {
    uint32_t blueprintId     = 0;
    float    materialCost    = 0.0f;   // credits worth of materials per unit
    float    outputValue     = 0.0f;   // sell price per unit of output
    float    buildTime       = 10.0f;  // seconds per production run
    float    outputVolume    = 1.0f;   // m³ per unit of output
};

/**
 * Configuration for a single AI industrialist.
 */
struct IndustrialistConfig {
    uint32_t entityId           = 0;
    float    productionCapacity = 200.0f;  // m³ per production run
    float    travelSpeed        = 70.0f;   // units/s
    float    startingCredits    = 10000.0f;
    float    respawnDelay       = 120.0f;  // seconds
};

/**
 * Runtime state for one AI industrialist.
 */
struct IndustrialistRuntime {
    IndustrialistConfig config;
    IndustrialistState  state              = IndustrialistState::Idle;
    float               stateTimer         = 0.0f;
    float               credits            = 0.0f;
    float               outputFill         = 0.0f;      // m³ of finished goods
    float               buildProgress      = 0.0f;      // 0-1
    float               travelProgress     = 0.0f;      // 0-1
    float               totalProfit        = 0.0f;
    uint32_t            jobsCompleted      = 0;
    uint32_t            activeBlueprintId  = 0;
    float               activeOutputValue  = 0.0f;
    float               activeMaterialCost = 0.0f;
    float               activeBuildTime    = 0.0f;
    float               respawnTimer       = 0.0f;
};

/**
 * @brief Autonomous AI industrialist state machine.
 *
 * Manages multiple NPC industrialists that manufacture goods from
 * blueprints and sell them for profit.
 *
 *   Idle → SelectBlueprint → AcquireMaterials → Manufacturing →
 *   DeliverGoods → SellGoods → (repeat)
 *
 * Industrialists choose blueprints with the best profit margin,
 * spend credits on materials, run timed production, then sell output.
 *
 * Usage:
 *   AIIndustrialistStateMachine sm;
 *   sm.AddIndustrialist({1, 200.0f, 70.0f, 10000.0f, 120.0f});
 *   sm.SetAvailableBlueprints({{1, 50.0f, 120.0f, 10.0f, 1.0f}});
 *   sm.Tick(dt);
 */
class AIIndustrialistStateMachine {
public:
    AIIndustrialistStateMachine() = default;

    /** Add a new industrialist. Returns false if entityId already tracked. */
    bool AddIndustrialist(const IndustrialistConfig& config);

    /** Remove an industrialist by entity id. */
    bool RemoveIndustrialist(uint32_t entityId);

    /** Set available blueprints for manufacturing. */
    void SetAvailableBlueprints(const std::vector<BlueprintEntry>& blueprints);

    /** Advance all industrialists by deltaSeconds. */
    void Tick(float deltaSeconds);

    /** Force-kill an industrialist. */
    bool KillIndustrialist(uint32_t entityId);

    /** Get read-only runtime state. Returns nullptr if not found. */
    const IndustrialistRuntime* GetIndustrialist(uint32_t entityId) const;

    /** Number of tracked industrialists. */
    size_t IndustrialistCount() const { return m_industrialists.size(); }

    /** Number of industrialists in a given state. */
    size_t CountInState(IndustrialistState state) const;

    /** Aggregate lifetime profit across all industrialists. */
    float TotalProfit() const;

    /** Aggregate completed manufacturing jobs. */
    uint32_t TotalJobs() const;

    /** Distance parameter for delivery travel time. */
    void SetMarketDistance(float distance) { m_marketDistance = distance; }
    float MarketDistance() const { return m_marketDistance; }

private:
    void tickIndustrialist(IndustrialistRuntime& ind, float dt);
    const BlueprintEntry* pickBestBlueprint(float availableCredits) const;

    std::vector<IndustrialistRuntime> m_industrialists;
    std::vector<BlueprintEntry>       m_blueprints;
    float                             m_marketDistance = 400.0f;
};

} // namespace atlas::sim
