#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace atlas::sim {

/**
 * States for an autonomous AI miner entity.
 */
enum class MinerState : uint8_t {
    Idle,             // Waiting, no target
    SelectTarget,     // Choosing a mineral deposit
    TravelToField,    // Warping / flying to asteroid field
    Mining,           // Actively mining ore
    CargoFull,        // Cargo hold is full, need to return
    ReturnToStation,  // Traveling back to sell
    SellOre,          // Docked, selling ore at market
    Dead              // Destroyed, awaiting respawn logic
};

/**
 * Configuration for a single AI miner.
 */
struct MinerConfig {
    uint32_t entityId         = 0;
    float    cargoCapacity    = 400.0f;   // m³
    float    miningYieldPerSec = 2.0f;    // m³/s
    float    travelSpeed      = 100.0f;   // units/s
    float    sellPricePerUnit = 10.0f;    // credits per m³
    float    respawnDelay     = 60.0f;    // seconds
};

/**
 * Runtime state for one AI miner.
 */
struct MinerRuntime {
    MinerConfig config;
    MinerState  state          = MinerState::Idle;
    float       cargoFill      = 0.0f;      // current m³ in hold
    float       stateTimer     = 0.0f;      // seconds in current state
    float       totalEarnings  = 0.0f;      // lifetime credits earned
    float       travelProgress = 0.0f;      // 0-1 progress on current travel
    uint32_t    targetDepositId = 0;         // entity id of target deposit
    uint32_t    cyclesCompleted = 0;         // full mine→sell cycles
    float       respawnTimer   = 0.0f;       // countdown while dead
};

/**
 * @brief Autonomous AI miner state machine.
 *
 * Manages multiple AI miners through their lifecycle:
 *   Idle → SelectTarget → TravelToField → Mining → CargoFull →
 *   ReturnToStation → SellOre → (repeat)
 *
 * Each tick advances timers, transitions states, and accumulates earnings.
 * External systems provide deposit availability via SetAvailableDeposits().
 *
 * Usage:
 *   AIMinerStateMachine sm;
 *   sm.AddMiner({1, 400.0f, 2.0f, 100.0f, 10.0f, 60.0f});
 *   sm.SetAvailableDeposits({100, 101, 102});
 *   sm.Tick(deltaTime);
 *   auto& rt = sm.GetMiner(1);
 */
class AIMinerStateMachine {
public:
    AIMinerStateMachine() = default;

    /** Add a new miner. Returns false if entityId already tracked. */
    bool AddMiner(const MinerConfig& config);

    /** Remove a miner by entity id. */
    bool RemoveMiner(uint32_t entityId);

    /** Set the list of available mineral deposit entity ids. */
    void SetAvailableDeposits(const std::vector<uint32_t>& depositIds);

    /** Advance all miners by deltaSeconds. */
    void Tick(float deltaSeconds);

    /** Force-kill a miner (simulates ship destruction). */
    bool KillMiner(uint32_t entityId);

    /** Get read-only runtime state. Returns nullptr if not found. */
    const MinerRuntime* GetMiner(uint32_t entityId) const;

    /** Number of tracked miners. */
    size_t MinerCount() const { return m_miners.size(); }

    /** Number of miners in a given state. */
    size_t CountInState(MinerState state) const;

    /** Aggregate lifetime earnings across all miners. */
    float TotalEarnings() const;

    /** Aggregate completed mine→sell cycles. */
    uint32_t TotalCycles() const;

    /** Distance parameter for travel time calculation. */
    void SetFieldDistance(float distance) { m_fieldDistance = distance; }
    float FieldDistance() const { return m_fieldDistance; }

private:
    void tickMiner(MinerRuntime& m, float dt);
    uint32_t pickDeposit() const;

    std::vector<MinerRuntime> m_miners;
    std::vector<uint32_t>     m_availableDeposits;
    float                     m_fieldDistance = 500.0f; // units
};

} // namespace atlas::sim
