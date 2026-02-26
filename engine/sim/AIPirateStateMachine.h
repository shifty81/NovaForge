#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

/**
 * States for an autonomous AI pirate entity.
 */
enum class PirateState : uint8_t {
    Idle,            // Waiting, no activity
    Patrol,          // Traveling patrol route
    ScanForTargets,  // Scanning for nearby haulers / traders
    PursueTarget,    // Closing distance to target
    Attack,          // Actively attacking target
    LootCargo,       // Looting cargo from destroyed target
    Flee,            // Retreating from danger
    Dead             // Destroyed, awaiting respawn
};

/**
 * Configuration for a single AI pirate.
 */
struct PirateConfig {
    uint32_t entityId      = 0;
    float    attackPower    = 10.0f;    // damage per second
    float    hitPoints      = 100.0f;   // max HP
    float    travelSpeed    = 120.0f;   // units/s
    float    scanRange      = 300.0f;   // units
    float    respawnDelay   = 120.0f;   // seconds
    float    lootCapacity   = 200.0f;   // m³
};

/**
 * Runtime state for one AI pirate.
 */
struct PirateRuntime {
    PirateConfig config;
    PirateState  state          = PirateState::Idle;
    float        stateTimer     = 0.0f;
    float        currentHP      = 0.0f;
    uint32_t     targetEntityId = 0;
    float        travelProgress = 0.0f;
    float        lootFill       = 0.0f;      // current m³ looted
    uint32_t     killCount      = 0;
    float        respawnTimer   = 0.0f;
};

/**
 * @brief Autonomous AI pirate state machine.
 *
 * Manages multiple AI pirates through their lifecycle:
 *   Idle → Patrol → ScanForTargets → PursueTarget → Attack →
 *   LootCargo → (repeat)   or   Flee → Patrol
 *
 * Each tick advances timers, transitions states, and tracks kills/loot.
 * External systems provide patrol waypoints and scan results.
 *
 * Usage:
 *   AIPirateStateMachine sm;
 *   sm.AddPirate({1, 10.0f, 100.0f, 120.0f, 300.0f, 120.0f, 200.0f});
 *   sm.SetPatrolWaypoints({50, 51, 52});
 *   sm.Tick(deltaTime);
 */
class AIPirateStateMachine {
public:
    AIPirateStateMachine() = default;

    /** Add a new pirate. Returns false if entityId already tracked. */
    bool AddPirate(const PirateConfig& config);

    /** Remove a pirate by entity id. */
    bool RemovePirate(uint32_t entityId);

    /** Set available patrol waypoint entity ids. */
    void SetPatrolWaypoints(const std::vector<uint32_t>& waypoints);

    /** Set targets detected by scanning. */
    void SetTargetsInRange(const std::vector<uint32_t>& targets);

    /** Advance all pirates by deltaSeconds. */
    void Tick(float deltaSeconds);

    /** Apply damage to a pirate. Kills if HP <= 0. */
    bool DamagePirate(uint32_t entityId, float damage);

    /** Force-kill a pirate (simulates ship destruction). */
    bool KillPirate(uint32_t entityId);

    /** Get read-only runtime state. Returns nullptr if not found. */
    const PirateRuntime* GetPirate(uint32_t entityId) const;

    /** Number of tracked pirates. */
    size_t PirateCount() const { return m_pirates.size(); }

    /** Number of pirates in a given state. */
    size_t CountInState(PirateState state) const;

    /** Aggregate lifetime kills across all pirates. */
    uint32_t TotalKills() const;

    /** Aggregate looted cargo across all pirates. */
    float TotalLoot() const;

    /** Distance parameter for patrol travel time calculation. */
    void SetPatrolDistance(float distance) { m_patrolDistance = distance; }
    float PatrolDistance() const { return m_patrolDistance; }

private:
    void tickPirate(PirateRuntime& p, float dt);
    uint32_t pickTarget() const;

    std::vector<PirateRuntime> m_pirates;
    std::vector<uint32_t>      m_patrolWaypoints;
    std::vector<uint32_t>      m_targetsInRange;
    float                      m_patrolDistance = 600.0f; // units
};

} // namespace atlas::sim
