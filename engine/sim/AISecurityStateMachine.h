#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::sim {

/**
 * States for an autonomous AI security guard entity.
 */
enum class SecurityState : uint8_t {
    Idle,            // Waiting, no activity
    Standby,         // On standby at post
    ScanForThreats,  // Scanning for nearby threats
    WarpToThreat,    // Warping to engage a threat
    EngageThreat,    // Actively fighting a threat
    ReturnToPost,    // Returning to guard post
    Dead             // Destroyed, awaiting respawn
};

/**
 * Configuration for a single AI security guard.
 */
struct SecurityConfig {
    uint32_t entityId      = 0;
    float    attackPower    = 15.0f;    // damage per second
    float    hitPoints      = 200.0f;   // max HP
    float    travelSpeed    = 150.0f;   // units/s
    float    scanRange      = 500.0f;   // units
    float    respawnDelay   = 90.0f;    // seconds
    float    responseTime   = 5.0f;     // seconds before scanning
};

/**
 * Runtime state for one AI security guard.
 */
struct SecurityRuntime {
    SecurityConfig config;
    SecurityState  state              = SecurityState::Idle;
    float          stateTimer         = 0.0f;
    float          currentHP          = 0.0f;
    uint32_t       targetThreatId     = 0;
    float          travelProgress     = 0.0f;
    uint32_t       threatsNeutralized = 0;
    float          respawnTimer       = 0.0f;
};

/**
 * @brief Autonomous AI security state machine.
 *
 * Manages multiple security guard NPCs that respond to threats
 * (pirates, hostile ships) by warping to engage them.
 *
 *   Idle → Standby → ScanForThreats → WarpToThreat → EngageThreat →
 *   ReturnToPost → (repeat)
 *
 * Usage:
 *   AISecurityStateMachine sm;
 *   sm.AddGuard({1, 15.0f, 200.0f, 150.0f, 500.0f, 90.0f, 5.0f});
 *   sm.SetActiveThreats({200, 201});
 *   sm.Tick(deltaTime);
 */
class AISecurityStateMachine {
public:
    AISecurityStateMachine() = default;

    /** Add a new guard. Returns false if entityId already tracked. */
    bool AddGuard(const SecurityConfig& config);

    /** Remove a guard by entity id. */
    bool RemoveGuard(uint32_t entityId);

    /** Set known active threat entity ids. */
    void SetActiveThreats(const std::vector<uint32_t>& threats);

    /** Advance all guards by deltaSeconds. */
    void Tick(float deltaSeconds);

    /** Apply damage to a guard. Kills if HP <= 0. */
    bool DamageGuard(uint32_t entityId, float damage);

    /** Force-kill a guard (simulates ship destruction). */
    bool KillGuard(uint32_t entityId);

    /** Get read-only runtime state. Returns nullptr if not found. */
    const SecurityRuntime* GetGuard(uint32_t entityId) const;

    /** Number of tracked guards. */
    size_t GuardCount() const { return m_guards.size(); }

    /** Number of guards in a given state. */
    size_t CountInState(SecurityState state) const;

    /** Aggregate lifetime threats neutralized across all guards. */
    uint32_t TotalNeutralized() const;

    /** Post distance for warp travel time calculation. */
    void SetPostDistance(float distance) { m_postDistance = distance; }
    float PostDistance() const { return m_postDistance; }

private:
    void tickGuard(SecurityRuntime& g, float dt);
    uint32_t pickThreat() const;

    std::vector<SecurityRuntime> m_guards;
    std::vector<uint32_t>        m_activeThreats;
    float                        m_postDistance = 400.0f; // units
};

} // namespace atlas::sim
