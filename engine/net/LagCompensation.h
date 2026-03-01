#pragma once
#include <cstdint>
#include <vector>
#include <deque>
#include <unordered_map>
#include <cmath>
#include "NetworkInterpolationBuffer.h"

namespace atlas::net {

/**
 * Result of a lag-compensated hit test.
 */
struct HitTestResult {
    bool     hit       = false;    ///< Whether the target was within range.
    uint32_t entityId  = 0;       ///< Target entity tested.
    uint32_t rewindTick = 0;      ///< Tick the state was rewound to.
    float    distance  = 0.0f;    ///< Distance from origin to rewound position.
};

/**
 * @brief Server-side lag compensation for hit detection.
 *
 * In a client-server architecture with non-trivial latency the shooting
 * player sees targets at positions that are slightly in the past.  The
 * server must rewind entity state to where the target was *when the
 * player pulled the trigger* (accounting for the client's round-trip
 * time) and test the hit against that historical position.
 *
 * This class stores a per-entity history of authoritative snapshots and
 * provides:
 *   - StoreSnapshot()  — record an authoritative entity state each tick.
 *   - GetStateAtTick() — retrieve interpolated state at any past tick.
 *   - HitTest()        — check if a shot from (origin) hits a target at
 *                         a rewound tick within a given radius.
 *
 * Configuration:
 *   - historyDuration: how many ticks of history to keep (default 64).
 *   - maxRewindTicks:  upper bound on how far back a client can rewind
 *                       (prevents abuse; default 40 ≈ 2 s at 20 Hz).
 *
 * Workflow:
 *   1. Each server tick, call StoreSnapshot() for every entity.
 *   2. When a client fires, compute the rewind tick:
 *          rewindTick = currentTick - RTT_in_ticks / 2
 *   3. Call HitTest() with the origin, target, rewindTick, and radius.
 *   4. The system returns whether the hit is valid.
 */
class LagCompensation {
public:
    explicit LagCompensation(size_t historyDuration = 64,
                              uint32_t maxRewindTicks = 40);

    /**
     * Record an authoritative snapshot for an entity at the current tick.
     * Old snapshots beyond historyDuration are automatically pruned.
     */
    void StoreSnapshot(const EntitySnapshot& snap);

    /**
     * Retrieve the interpolated state of an entity at a past tick.
     * If the tick falls between two stored snapshots, linear
     * interpolation is used.  Returns a state with valid=false if
     * the tick is outside the stored history.
     *
     * @param entityId   Entity to query.
     * @param tick       The (possibly fractional) tick to rewind to.
     */
    InterpolatedState GetStateAtTick(uint32_t entityId, float tick) const;

    /**
     * Perform a lag-compensated hit test.
     *
     * @param targetId    Entity to test against.
     * @param originX/Y/Z World-space origin of the shot.
     * @param rewindTick  Tick at which to evaluate target position.
     * @param hitRadius   Maximum distance for a hit (bounding sphere).
     * @return HitTestResult with hit=true if within radius.
     */
    HitTestResult HitTest(uint32_t targetId,
                          float originX, float originY, float originZ,
                          uint32_t rewindTick,
                          float hitRadius) const;

    /** Remove all history for an entity (e.g. when destroyed). */
    void RemoveEntity(uint32_t entityId);

    /** Clear all stored history. */
    void Clear();

    /** Number of entities currently tracked. */
    size_t TrackedEntityCount() const { return m_history.size(); }

    /** Number of snapshots stored for an entity. */
    size_t SnapshotCount(uint32_t entityId) const;

    /** Get history duration setting. */
    size_t HistoryDuration() const { return m_historyDuration; }

    /** Get max rewind ticks setting. */
    uint32_t MaxRewindTicks() const { return m_maxRewindTicks; }

    /** Current server tick (highest tick seen across all entities). */
    uint32_t CurrentTick() const { return m_currentTick; }

    // --- Tuning ---
    static constexpr float kDefaultHitRadius = 5.0f;

private:
    struct EntityHistory {
        uint32_t entityId = 0;
        std::deque<EntitySnapshot> snapshots; // oldest first
    };

    EntityHistory*       findEntity(uint32_t id);
    const EntityHistory* findEntity(uint32_t id) const;

    /** Prune snapshots older than historyDuration for an entity. */
    void prune(EntityHistory& eh) const;

    /** Clamp rewind tick to allowed range. */
    uint32_t clampRewindTick(uint32_t requested) const;

    std::unordered_map<uint32_t, EntityHistory> m_history;
    size_t   m_historyDuration;
    uint32_t m_maxRewindTicks;
    uint32_t m_currentTick = 0;
};

} // namespace atlas::net
