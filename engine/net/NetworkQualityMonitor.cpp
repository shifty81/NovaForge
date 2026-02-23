#include "net/NetworkQualityMonitor.h"
#include <cmath>
#include <algorithm>

namespace atlas::net {

void NetworkQualityMonitor::recordRTT(float rttSeconds) {
    if (rttSeconds < 0.0f) return;

    if (m_firstRTT) {
        m_smoothedRTT = rttSeconds;
        m_jitter      = 0.0f;
        m_firstRTT    = false;
        return;
    }

    // RFC 6298-style EWMA
    float diff    = std::abs(rttSeconds - m_smoothedRTT);
    m_jitter      = m_jitter + kJitterAlpha * (diff - m_jitter);
    m_smoothedRTT = m_smoothedRTT + kRTTAlpha * (rttSeconds - m_smoothedRTT);
}

void NetworkQualityMonitor::recordPacketArrival(uint32_t seq) {
    if (!m_seqInitialized) {
        m_expectedSeq    = seq + 1;
        m_seqInitialized = true;
        m_seqWindow.push_back({seq, true});
        return;
    }

    // Fill in any gaps as "lost"
    while (m_expectedSeq < seq) {
        m_seqWindow.push_back({m_expectedSeq, false});
        ++m_expectedSeq;
        // trim window
        if (m_seqWindow.size() > kLossWindowSize)
            m_seqWindow.pop_front();
    }

    // Record this packet as received
    m_seqWindow.push_back({seq, true});
    if (m_seqWindow.size() > kLossWindowSize)
        m_seqWindow.pop_front();

    // Advance expected
    if (seq >= m_expectedSeq)
        m_expectedSeq = seq + 1;
}

float NetworkQualityMonitor::getPacketLossRate() const {
    if (m_seqWindow.empty()) return 0.0f;

    uint32_t lost = 0;
    for (const auto& r : m_seqWindow) {
        if (!r.received) ++lost;
    }
    return static_cast<float>(lost) / static_cast<float>(m_seqWindow.size());
}

float NetworkQualityMonitor::getAdaptiveInterpolationTime() const {
    // Base = half RTT (one-way) + jitter safety margin
    float base = m_smoothedRTT * 0.5f + m_jitter * kJitterMultiplier;

    // Add extra buffer when packets are being lost (up to +0.1s at 100% loss)
    float lossBonus = getPacketLossRate() * 0.1f;

    float result = base + lossBonus;
    return std::clamp(result, kMinInterpTime, kMaxInterpTime);
}

void NetworkQualityMonitor::reset() {
    m_smoothedRTT    = 0.0f;
    m_jitter         = 0.0f;
    m_firstRTT       = true;
    m_expectedSeq    = 0;
    m_seqInitialized = false;
    m_seqWindow.clear();
}

} // namespace atlas::net
