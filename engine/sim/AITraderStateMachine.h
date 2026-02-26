#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

/**
 * States for an autonomous AI trader entity.
 */
enum class TraderState : uint8_t {
    Idle,              // Waiting, evaluating opportunities
    EvaluateMarket,    // Scanning buy/sell price data
    TravelToBuy,       // Flying to low-price station
    BuyGoods,          // Purchasing goods at source station
    TravelToSell,      // Flying to high-price station
    SellGoods,         // Selling goods for profit
    Flee,              // Retreating from danger
    Dead               // Destroyed, awaiting respawn
};

/**
 * A market listing visible to the trader.
 */
struct MarketListing {
    uint32_t stationId   = 0;      // station offering goods
    float    buyPrice    = 0.0f;   // price to buy at this station
    float    sellPrice   = 0.0f;   // price to sell at this station
    float    available   = 0.0f;   // m³ available for purchase
    float    distance    = 0.0f;   // units to station
};

/**
 * Configuration for a single AI trader.
 */
struct TraderConfig {
    uint32_t entityId         = 0;
    float    cargoCapacity    = 500.0f;   // m³
    float    travelSpeed      = 90.0f;    // units/s
    float    startingCredits  = 5000.0f;  // initial wallet
    float    hitPoints        = 80.0f;    // max HP (traders are fragile)
    float    respawnDelay     = 90.0f;    // seconds
};

/**
 * Runtime state for one AI trader.
 */
struct TraderRuntime {
    TraderConfig config;
    TraderState  state           = TraderState::Idle;
    float        stateTimer      = 0.0f;
    float        credits         = 0.0f;       // current wallet
    float        cargoFill       = 0.0f;       // current m³ in hold
    float        cargoBuyPrice   = 0.0f;       // price paid per m³
    float        travelProgress  = 0.0f;       // 0-1
    float        totalProfit     = 0.0f;       // lifetime profit
    uint32_t     tradesCompleted = 0;
    uint32_t     buyStationId    = 0;          // station to buy from
    uint32_t     sellStationId   = 0;          // station to sell at
    float        currentHP       = 0.0f;
    float        respawnTimer    = 0.0f;
};

/**
 * @brief Autonomous AI trader state machine.
 *
 * Manages multiple AI traders that buy low and sell high across stations.
 *
 *   Idle → EvaluateMarket → TravelToBuy → BuyGoods →
 *   TravelToSell → SellGoods → (repeat)
 *
 * Traders analyse MarketListings for price differentials and execute
 * arbitrage trades, accumulating profit over time.
 *
 * Usage:
 *   AITraderStateMachine sm;
 *   sm.AddTrader({1, 500.0f, 90.0f, 5000.0f, 80.0f, 90.0f});
 *   sm.SetMarketListings({...});
 *   sm.Tick(dt);
 */
class AITraderStateMachine {
public:
    AITraderStateMachine() = default;

    /** Add a new trader. Returns false if entityId already tracked. */
    bool AddTrader(const TraderConfig& config);

    /** Remove a trader by entity id. */
    bool RemoveTrader(uint32_t entityId);

    /** Set available market listings. */
    void SetMarketListings(const std::vector<MarketListing>& listings);

    /** Advance all traders by deltaSeconds. */
    void Tick(float deltaSeconds);

    /** Apply damage to a trader. Kills if HP <= 0. */
    bool DamageTrader(uint32_t entityId, float damage);

    /** Force-kill a trader (simulates ship destruction). */
    bool KillTrader(uint32_t entityId);

    /** Get read-only runtime state. Returns nullptr if not found. */
    const TraderRuntime* GetTrader(uint32_t entityId) const;

    /** Number of tracked traders. */
    size_t TraderCount() const { return m_traders.size(); }

    /** Number of traders in a given state. */
    size_t CountInState(TraderState state) const;

    /** Aggregate lifetime profit across all traders. */
    float TotalProfit() const;

    /** Aggregate completed trades. */
    uint32_t TotalTrades() const;

    /** Default distance for sell-trip travel time. */
    void SetDefaultDistance(float distance) { m_defaultDistance = distance; }
    float DefaultDistance() const { return m_defaultDistance; }

private:
    void tickTrader(TraderRuntime& t, float dt);
    bool findBestTrade(uint32_t& outBuyStation, uint32_t& outSellStation,
                       float& outBuyPrice, float& outSellPrice) const;

    std::vector<TraderRuntime>  m_traders;
    std::vector<MarketListing>  m_marketListings;
    float                       m_defaultDistance = 500.0f;
};

} // namespace atlas::sim
