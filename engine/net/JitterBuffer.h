#pragma once
#include <cstdint>
#include <vector>
#include <deque>

namespace atlas::net {

/**
 * @brief Client-side jitter buffer for smoothing packet delivery timing.
 *
 * Network packets arrive with variable delay (jitter). This buffer
 * holds incoming packets for a configurable delay before releasing
 * them, so that downstream consumers (e.g. NetworkInterpolationBuffer)
 * see a smooth, steady stream of updates.
 *
 * Design:
 *   - Each buffered entry carries a server tick and arrival timestamp.
 *   - Entries are released in tick order once they have been held for
 *     at least `targetDelay` seconds.
 *   - Late packets (older than already-released ticks) are dropped.
 *   - The buffer adapts its target delay based on observed jitter
 *     using an exponential moving average.
 *
 * Workflow:
 *   1. On packet receipt, call Push() with the server tick and a
 *      monotonic local timestamp (seconds since some epoch).
 *   2. Each render/tick frame, call Flush() with the current time to
 *      retrieve entries that are ready for consumption.
 *
 * The buffer payload is a generic byte vector so it can carry any
 * serialised data (snapshots, compressed frames, etc.).
 */

struct JitterEntry {
    uint32_t             tick        = 0;
    float                arrivalTime = 0.0f;   // local monotonic time
    std::vector<uint8_t> payload;               // opaque data
};

class JitterBuffer {
public:
    /**
     * @param targetDelay   Initial target buffering delay (seconds).
     * @param maxBufferSize Maximum entries before oldest are dropped.
     * @param adaptive      If true, target delay auto-adjusts to jitter.
     */
    explicit JitterBuffer(float targetDelay   = 0.1f,
                          size_t maxBufferSize = 64,
                          bool   adaptive      = true);

    /**
     * Insert a new entry into the buffer.
     * @param tick        Server tick of the data.
     * @param arrivalTime Local monotonic time of arrival (seconds).
     * @param payload     Opaque serialised data.
     */
    void Push(uint32_t tick, float arrivalTime,
              const std::vector<uint8_t>& payload);

    /**
     * Retrieve entries that are ready for consumption.
     * Returns entries in tick order whose hold time has elapsed.
     * @param currentTime Local monotonic time (seconds).
     */
    std::vector<JitterEntry> Flush(float currentTime);

    /** Number of entries currently buffered. */
    size_t BufferedCount() const { return m_buffer.size(); }

    /** Current target delay (seconds). May change if adaptive. */
    float TargetDelay() const { return m_targetDelay; }

    /** Maximum buffer capacity. */
    size_t MaxBufferSize() const { return m_maxBufferSize; }

    /** Whether adaptive delay is enabled. */
    bool IsAdaptive() const { return m_adaptive; }

    /** Total entries pushed since construction or last Reset(). */
    uint64_t TotalPushed() const { return m_totalPushed; }

    /** Total entries dropped (late or overflow). */
    uint64_t TotalDropped() const { return m_totalDropped; }

    /** Reset all state. */
    void Reset();

    // Tuning constants (public for testability)
    static constexpr float kJitterAlpha     = 0.125f;   // EWMA weight
    static constexpr float kAdaptiveScale   = 2.0f;     // jitter → delay multiplier
    static constexpr float kMinTargetDelay  = 0.02f;    // 20 ms floor
    static constexpr float kMaxTargetDelay  = 0.50f;    // 500 ms ceiling

private:
    void adaptDelay(float interArrivalJitter);
    void trimOverflow();

    std::deque<JitterEntry> m_buffer;       // sorted by tick

    float    m_targetDelay;
    size_t   m_maxBufferSize;
    bool     m_adaptive;

    // Jitter estimation
    float    m_smoothedJitter   = 0.0f;
    float    m_lastArrivalTime  = -1.0f;
    float    m_lastInterArrival = 0.0f;
    bool     m_firstPush        = true;

    // Bookkeeping
    uint32_t m_lastReleasedTick = 0;
    uint64_t m_totalPushed      = 0;
    uint64_t m_totalDropped     = 0;
};

} // namespace atlas::net
