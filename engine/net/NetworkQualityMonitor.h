#pragma once
#include <cstdint>
#include <deque>

namespace atlas::net {

/// Tracks network quality metrics: RTT, jitter, packet loss rate.
/// Recommends an adaptive interpolation window for smooth rendering.
class NetworkQualityMonitor {
public:
    /// Record a round-trip time sample (seconds).
    void recordRTT(float rttSeconds);

    /// Record arrival of a packet with the given sequence number.
    /// Detects gaps and out-of-order delivery.
    void recordPacketArrival(uint32_t sequenceNumber);

    /// Smoothed RTT estimate (exponential moving average).
    float getSmoothedRTT() const { return m_smoothedRTT; }

    /// Jitter estimate — mean deviation of RTT samples (RFC 3550 style).
    float getJitter() const { return m_jitter; }

    /// Fraction of packets lost in the observation window [0, 1].
    float getPacketLossRate() const;

    /// Recommended interpolation buffer duration (seconds).
    /// Adapts to current RTT + jitter so the client always has a snapshot
    /// to interpolate toward, even under moderate packet loss.
    float getAdaptiveInterpolationTime() const;

    /// Reset all state.
    void reset();

    // --- Tuning constants (public for testability) ---
    static constexpr float kRTTAlpha          = 0.125f;   // EWMA weight for new RTT
    static constexpr float kJitterAlpha        = 0.0625f;  // EWMA weight for jitter
    static constexpr float kMinInterpTime      = 0.05f;    // 50 ms floor
    static constexpr float kMaxInterpTime      = 0.30f;    // 300 ms ceiling
    static constexpr float kJitterMultiplier   = 2.0f;     // safety margin on jitter
    static constexpr uint32_t kLossWindowSize  = 100;      // sequence window for loss calc

private:
    // RTT / jitter
    float m_smoothedRTT = 0.0f;
    float m_jitter      = 0.0f;
    bool  m_firstRTT    = true;

    // Packet loss tracking
    uint32_t m_expectedSeq     = 0;   // next expected sequence number
    uint32_t m_packetsReceived = 0;
    uint32_t m_packetsExpected = 0;
    bool     m_seqInitialized  = false;

    // Sliding window for loss rate
    struct SeqRecord { uint32_t seq; bool received; };
    std::deque<SeqRecord> m_seqWindow;
};

} // namespace atlas::net
